#include "tcg.h"
#include "../../include/exec/cpu-all.h"
#include "../../include/exec/cpu-defs.h"
#include "../../include/exec/exec-all.h"
#include "../../include/exec/tb-hash.h"
#include "../../include/exec/tb-lookup.h"
#include "../../include/hw/core/cpu.h"
#include "../../include/qemu/atomic.h"
#include "../../include/qemu/osdep.h"
#include "../../include/sysemu/cpus.h"
#include "../../include/sysemu/replay.h"
#include "../../include/types.h"
#include "../../include/qemu/thread-posix.h"
#include <stdbool.h>
#include <stddef.h>

// FIXME we define similar functios at head of cpu-tlb.c
#define qemu_mutex_lock(m) ({})
#define qemu_mutex_unlock(m) ({})

#define TCG_HIGHWATER 1024

#include <glib-2.0/glib/gtree.h> // remove glib
struct tcg_region_tree {
  QemuMutex lock;
  GTree *tree;
  /* padding to avoid false sharing is computed at run-time */
};

/*
 * We divide code_gen_buffer into equally-sized "regions" that TCG threads
 * dynamically allocate from as demand dictates. Given appropriate region
 * sizing, this minimizes flushes even when some TCG threads generate a lot
 * more code than others.
 */
struct tcg_region_state {
  QemuMutex lock;

  /* fields set at init time */
  void *start;
  void *start_aligned;
  void *end;
  size_t n;
  size_t size;   /* size of one region */
  size_t stride; /* .size + guard size */

  /* fields protected by the lock */
  size_t current;       /* current region index */
  size_t agg_size_full; /* aggregate size of full regions */
};

typedef struct TCGLabelPoolData {
    struct TCGLabelPoolData *next;
    tcg_insn_unit *label;
    intptr_t addend;
    int rtype;
    unsigned nlong;
    tcg_target_ulong data[];
} TCGLabelPoolData;

static struct tcg_region_state region;
/*
 * This is an array of struct tcg_region_tree's, with padding.
 * We use void * to simplify the computation of region_trees[i]; each
 * struct is found every tree_size bytes.
 */
static void *region_trees;
static size_t tree_size;
static TCGRegSet tcg_target_available_regs[TCG_TYPE_COUNT];
static TCGRegSet tcg_target_call_clobber_regs;

static void tcg_region_bounds(size_t curr_region, void **pstart, void **pend) {
  void *start, *end;

  start = region.start_aligned + curr_region * region.stride;
  end = start + region.size;

  if (curr_region == 0) {
    start = region.start;
  }
  if (curr_region == region.n - 1) {
    end = region.end;
  }

  *pstart = start;
  *pend = end;
}

static void tcg_region_assign(TCGContext *s, size_t curr_region) {
  void *start, *end;

  tcg_region_bounds(curr_region, &start, &end);

  s->code_gen_buffer = start;
  s->code_gen_ptr = start;
  s->code_gen_buffer_size = end - start;
  s->code_gen_highwater = end - TCG_HIGHWATER;
}

static bool tcg_region_alloc__locked(TCGContext *s) {
  if (region.current == region.n) {
    return true;
  }
  tcg_region_assign(s, region.current);
  region.current++;
  return false;
}

/*
 * Request a new region once the one in use has filled up.
 * Returns true on error.
 */
static bool tcg_region_alloc(TCGContext *s) {
  bool err;
  /* read the region size now; alloc__locked will overwrite it on success */
  size_t size_full = s->code_gen_buffer_size;

  qemu_mutex_lock(&region.lock);
  err = tcg_region_alloc__locked(s);
  if (!err) {
    region.agg_size_full += size_full - TCG_HIGHWATER;
  }
  qemu_mutex_unlock(&region.lock);
  return err;
}

static struct tcg_region_tree *tc_ptr_to_region_tree(void *p) {
  size_t region_idx;

  if (p < region.start_aligned) {
    region_idx = 0;
  } else {
    ptrdiff_t offset = p - region.start_aligned;

    if (offset > region.stride * (region.n - 1)) {
      region_idx = region.n - 1;
    } else {
      region_idx = offset / region.stride;
    }
  }
  return region_trees + region_idx * tree_size;
}

#ifndef CONFIG_DIRECT_REGS
static
#endif
TCGTemp *tcg_global_reg_new_internal(TCGContext *s, TCGType type,
                                            TCGReg reg, const char *name)
{
    TCGTemp *ts;

    if (TCG_TARGET_REG_BITS == 32 && type != TCG_TYPE_I32) {
        tcg_abort();
    }

    ts = tcg_global_alloc(s);
    ts->base_type = type;
    ts->type = type;
    ts->fixed_reg = 1;
    ts->reg = reg;
    ts->name = name;
    tcg_regset_set_reg(s->reserved_regs, reg);

    return ts;
}

void tcg_set_frame(TCGContext *s, TCGReg reg, intptr_t start, intptr_t size)
{
    s->frame_start = start;
    s->frame_end = start + size;
    s->frame_temp
        = tcg_global_reg_new_internal(s, TCG_TYPE_PTR, reg, "_frame");
}

// FIXME review this functions later, so many macros
/* Generate global QEMU prologue and epilogue code */
static void tcg_target_qemu_prologue(TCGContext *s)
{
    int i;

    #ifdef LOONGARCH_DEBUG
    printf("Start tcg_target_qemu_prologue.\n");
    #endif

    tcg_set_frame(s, TCG_REG_SP, TCG_STATIC_CALL_ARGS_SIZE, TEMP_SIZE);

#ifdef CONFIG_LATX
    i = target_x86_to_mips_static_codes(s->code_ptr);
    s->code_ptr += i;
    s->code_gen_prologue = (void*)context_switch_bt_to_native;
    s->code_gen_epilogue = (void*)context_switch_native_to_bt;
#else
    /* TB prologue */
    tcg_out_opc_imm(s, ALIAS_PADDI, TCG_REG_SP, TCG_REG_SP, -FRAME_SIZE);
    for (i = 0; i < ARRAY_SIZE(tcg_target_callee_save_regs); i++) {
        tcg_out_st(s, TCG_TYPE_REG, tcg_target_callee_save_regs[i],
                   TCG_REG_SP, SAVE_OFS + i * REG_SIZE);
    }

#ifndef CONFIG_SOFTMMU
    if (guest_base) {
        tcg_out_movi(s, TCG_TYPE_PTR, TCG_GUEST_BASE_REG, guest_base);
        tcg_regset_set_reg(s->reserved_regs, TCG_GUEST_BASE_REG);
    }
#endif

    /* Call generated code */
    tcg_out_mov(s, TCG_TYPE_PTR, TCG_AREG0, tcg_target_call_iarg_regs[0]);
    //LoongArch
    //tcg_out_opc_imm(s, OPC_JALR, TCG_REG_ZERO, tcg_target_call_iarg_regs[1], 0);
    tcg_out_opc_jirl(s, TCG_REG_ZERO, tcg_target_call_iarg_regs[1], 0);
    /* Return path for goto_ptr. Set return value to 0 */
    s->code_gen_epilogue = s->code_ptr;
    tcg_out_mov(s, TCG_TYPE_REG, TCG_REG_A0, TCG_REG_ZERO);

    /* TB epilogue */
    tb_ret_addr = s->code_ptr;
    for (i = 0; i < ARRAY_SIZE(tcg_target_callee_save_regs); i++) {
        tcg_out_ld(s, TCG_TYPE_REG, tcg_target_callee_save_regs[i],
                   TCG_REG_SP, SAVE_OFS + i * REG_SIZE);
    }

    tcg_out_opc_imm(s, ALIAS_PADDI, TCG_REG_SP, TCG_REG_SP, FRAME_SIZE);
    //LoongArch
    //tcg_out_opc_imm(s, OPC_JALR, TCG_REG_ZERO, TCG_REG_RA, 0);
    tcg_out_opc_jirl(s, TCG_REG_ZERO, TCG_REG_RA, 0);
#endif

    #ifdef LOONGARCH_DEBUG
    printf("End tcg_target_qemu_prologue.\n");
    #endif
}

// FIXME wow, a huge function
static void tcg_register_jit_int(void *buf_ptr, size_t buf_size,
                                 const void *debug_frame,
                                 size_t debug_frame_size);

void tcg_register_jit(void *buf, size_t buf_size)
{
    tcg_register_jit_int(buf, buf_size, &debug_frame, sizeof(debug_frame));
}


// FIXME review this function
static int tcg_out_pool_finalize(TCGContext *s)
{
    TCGLabelPoolData *p = s->pool_labels;
    TCGLabelPoolData *l = NULL;
    void *a;

    if (p == NULL) {
        return 0;
    }

    /* ??? Round up to qemu_icache_linesize, but then do not round
       again when allocating the next TranslationBlock structure.  */
    a = (void *)ROUND_UP((uintptr_t)s->code_ptr,
                         sizeof(tcg_target_ulong) * p->nlong);
    tcg_out_nop_fill(s->code_ptr, (tcg_insn_unit *)a - s->code_ptr);
    s->data_gen_ptr = a;

    for (; p != NULL; p = p->next) {
        size_t size = sizeof(tcg_target_ulong) * p->nlong;
        if (!l || l->nlong != p->nlong || memcmp(l->data, p->data, size)) {
            if (unlikely(a > s->code_gen_highwater)) {
                return -1;
            }
            memcpy(a, p->data, size);
            a += size;
            l = p;
        }
        if (!patch_reloc(p->label, p->rtype, (intptr_t)a - size, p->addend)) {
            return -2;
        }
    }

    s->code_ptr = a;
    return 0;
}

/*
 * Allocate TBs right before their corresponding translated code, making
 * sure that TBs and code are on different cache lines.
 */
TranslationBlock *tcg_tb_alloc(TCGContext *s) {
  uintptr_t align = qemu_icache_linesize;
  TranslationBlock *tb;
  void *next;

retry:
  tb = (void *)ROUND_UP((uintptr_t)s->code_gen_ptr, align);
  next = (void *)ROUND_UP((uintptr_t)(tb + 1), align);

  if (unlikely(next > s->code_gen_highwater)) {
    if (tcg_region_alloc(s)) {
      return NULL;
    }
    goto retry;
  }
  atomic_set(&s->code_gen_ptr, next);
  s->data_gen_ptr = NULL;
  return tb;
}

void tcg_tb_insert(TranslationBlock *tb) {
  struct tcg_region_tree *rt = tc_ptr_to_region_tree(tb->tc.ptr);

  qemu_mutex_lock(&rt->lock);
  // FIXME of course, it's impossible to include glib
  // but, understand how this file work and make a reconstruction
  g_tree_insert(rt->tree, &tb->tc, tb);
  qemu_mutex_unlock(&rt->lock);
}

void tcg_tb_remove(TranslationBlock *tb)
{
    struct tcg_region_tree *rt = tc_ptr_to_region_tree(tb->tc.ptr);

    qemu_mutex_lock(&rt->lock);
    g_tree_remove(rt->tree, &tb->tc);
    qemu_mutex_unlock(&rt->lock);
}

/*
 * Find the TB 'tb' such that
 * tb->tc.ptr <= tc_ptr < tb->tc.ptr + tb->tc.size
 * Return NULL if not found.
 */
TranslationBlock *tcg_tb_lookup(uintptr_t tc_ptr)
{
    struct tcg_region_tree *rt = tc_ptr_to_region_tree((void *)tc_ptr);
    TranslationBlock *tb;
    struct tb_tc s = { .ptr = (void *)tc_ptr };

    qemu_mutex_lock(&rt->lock);
    tb = g_tree_lookup(rt->tree, &s);
    qemu_mutex_unlock(&rt->lock);
    return tb;
}

void tcg_prologue_init(TCGContext *s)
{
    size_t prologue_size, total_size;
    void *buf0, *buf1;

    /* Put the prologue at the beginning of code_gen_buffer.  */
    buf0 = s->code_gen_buffer;
    total_size = s->code_gen_buffer_size;
    s->code_ptr = buf0;
    s->code_buf = buf0;
    s->data_gen_ptr = NULL;
    s->code_gen_prologue = buf0;

    /* Compute a high-water mark, at which we voluntarily flush the buffer
       and start over.  The size here is arbitrary, significantly larger
       than we expect the code generation for any one opcode to require.  */
    s->code_gen_highwater = s->code_gen_buffer + (total_size - TCG_HIGHWATER);

#ifdef TCG_TARGET_NEED_POOL_LABELS
    s->pool_labels = NULL;
#endif

    /* Generate the prologue.  */
    tcg_target_qemu_prologue(s);

    // FIXME what does this macro mean ?
#ifdef TCG_TARGET_NEED_POOL_LABELS
    /* Allow the prologue to put e.g. guest_base into a pool entry.  */
    {
        int result = tcg_out_pool_finalize(s);
        tcg_debug_assert(result == 0);
    }
#endif

    buf1 = s->code_ptr;
    flush_icache_range((uintptr_t)buf0, (uintptr_t)buf1);

    /* Deduct the prologue from the buffer.  */
    prologue_size = tcg_current_code_size(s);
    s->code_gen_ptr = buf1;
    s->code_gen_buffer = buf1;
    s->code_buf = buf1;
    total_size -= prologue_size;
    s->code_gen_buffer_size = total_size;

    tcg_register_jit(s->code_gen_buffer, total_size);

#ifdef DEBUG_DISAS
    if (qemu_loglevel_mask(CPU_LOG_TB_OUT_ASM)) {
        qemu_log_lock();
        qemu_log("PROLOGUE: [size=%zu]\n", prologue_size);
        if (s->data_gen_ptr) {
            size_t code_size = s->data_gen_ptr - buf0;
            size_t data_size = prologue_size - code_size;
            size_t i;

            log_disas(buf0, code_size);

            for (i = 0; i < data_size; i += sizeof(tcg_target_ulong)) {
                if (sizeof(tcg_target_ulong) == 8) {
                    qemu_log("0x%08" PRIxPTR ":  .quad  0x%016" PRIx64 "\n",
                             (uintptr_t)s->data_gen_ptr + i,
                             *(uint64_t *)(s->data_gen_ptr + i));
                } else {
                    qemu_log("0x%08" PRIxPTR ":  .long  0x%08x\n",
                             (uintptr_t)s->data_gen_ptr + i,
                             *(uint32_t *)(s->data_gen_ptr + i));
                }
            }
        } else {
            log_disas(buf0, prologue_size);
        }
        qemu_log("\n");
        qemu_log_flush();
        qemu_log_unlock();
    }
#endif

    /* Assert that goto_ptr is implemented completely.  */
    if (TCG_TARGET_HAS_goto_ptr) {
        tcg_debug_assert(s->code_gen_epilogue != NULL);
    }
}
