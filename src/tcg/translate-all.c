#include "../../include/exec/cpu-all.h"
#include "../../include/exec/cpu-defs.h"
#include "../../include/exec/exec-all.h"
#include "../../include/exec/tb-lookup.h"
#include "../../include/hw/core/cpu.h"
#include "../../include/qemu/osdep.h"
#include "../../include/sysemu/replay.h"
#include "../../include/todo.h"
#include "../../include/types.h"

#include "./tcg.h" // FIXME move tcg.h to include dir ?
#include <assert.h>
#include <stdbool.h> // FIXME header for bool


TCGContext *tcg_ctx;

// FIXME copied from  util/cacheinfo.c
// why I should take care of icache size ?
int qemu_icache_linesize = 0;
int qemu_icache_linesize_log;
int qemu_dcache_linesize = 0;
int qemu_dcache_linesize_log;

void tb_flush(CPUState *cpu) {
  // TODO
}

/* add a new TB and link it to the physical page tables. phys_page2 is
 * (-1) to indicate that only one page contains the TB.
 *
 * Called with mmap_lock held for user-mode emulation.
 *
 * Returns a pointer @tb, or a pointer to an existing TB that matches @tb.
 * Note that in !user-mode, another thread might have already added a TB
 * for the same block of guest code that @tb corresponds to. In that case,
 * the caller should discard the original @tb, and use instead the returned TB.
 */
static TranslationBlock *tb_link_page(TranslationBlock *tb,
                                      tb_page_addr_t phys_pc,
                                      tb_page_addr_t phys_page2) {
  // TODO
}

/* reset the jump entry 'n' of a TB so that it is not chained to
   another TB */
static inline void tb_reset_jump(TranslationBlock *tb, int n) {
  // TODO
}

// TODO /home/maritns3/core/ld/x86-qemu-mips/tcg/tcg.h
#define TCG_MAX_INSNS 512

/* Called with mmap_lock held for user mode emulation.  */
TranslationBlock *tb_gen_code(CPUState *cpu, target_ulong pc,
                              target_ulong cs_base, u32 flags, int cflags) {

  CPUArchState *env = cpu->env_ptr;
  TranslationBlock *tb, *existing_tb;
  tb_page_addr_t phys_pc, phys_page2;
  target_ulong virt_page2;
  // FIXME so, there are tcg instructions?
  tcg_insn_unit *gen_code_buf;
  int max_insns;
  int gen_code_size, search_size;

  // FIXME why pc is virtual address
  phys_pc = get_page_addr_code(env, pc);

  if (phys_pc == -1) {
    /* Generate a temporary TB with 1 insn in it */
    cflags &= ~CF_COUNT_MASK;
    cflags |= CF_NOCACHE | 1;
  }

  // FIXME wow, cflags define max_insns !
  // check how it get initiated
  cflags &= ~CF_CLUSTER_MASK;
  cflags |= cpu->cluster_index << CF_CLUSTER_SHIFT;

  max_insns = cflags & CF_COUNT_MASK;
  if (max_insns == 0) {
    max_insns = CF_COUNT_MASK;
  }
  if (max_insns > TCG_MAX_INSNS) {
    max_insns = TCG_MAX_INSNS;
  }

  // TODO is it possible to support singlestep?
  if (cpu->singlestep_enabled || singlestep) {
    max_insns = 1;
  }

  // FIXME this is temporary bug fix from xqm
  // see commit of xqm : 17da52287dadf474622523b110c02fb16f785b7d
  int xtm_is_bo = 0;

  // FIXME what does buffer and tb overflow means?
buffer_overflow:
  tb = tcg_tb_alloc(tcg_ctx);

  if (unlikely(!tb) || xtm_is_bo) {
    /* flush must be done */
    tb_flush(cpu);
    /* Make the execution loop process the flush as soon as possible.  */
    cpu->exception_index = EXCP_INTERRUPT;
    cpu_loop_exit(cpu);
  }

  gen_code_buf = tcg_ctx->code_gen_ptr;
  tb->tc.ptr = gen_code_buf;
  tb->pc = pc;
  tb->cs_base = cs_base;
  tb->flags = flags;
  tb->cflags = cflags;
  tb->orig_tb = NULL;
  // TODO how trace works ?
  // tb->trace_vcpu_dstate = *cpu->trace_dstate;

  // TODO what's tcg_ctx's role ?
  tcg_ctx->tb_cflags = cflags;

tb_overflow:
  /* generate machine code */
  tb->jmp_reset_offset[0] = TB_JMP_RESET_OFFSET_INVALID;
  tb->jmp_reset_offset[1] = TB_JMP_RESET_OFFSET_INVALID;
  tcg_ctx->tb_jmp_reset_offset = tb->jmp_reset_offset;

  // TODO maybe move this macro to proper locations
#define TCG_TARGET_HAS_direct_jump 1
  if (TCG_TARGET_HAS_direct_jump) {
    tcg_ctx->tb_jmp_insn_offset = tb->jmp_target_arg;
    tcg_ctx->tb_jmp_target_addr = NULL;
  } else {
    tcg_ctx->tb_jmp_insn_offset = NULL;
    tcg_ctx->tb_jmp_target_addr = tb->jmp_target_arg;
  }

  xtm_is_bo = 0;
  gen_code_size = target_x86_to_mips_host(
      cpu, tb, max_insns, tcg_ctx->code_gen_highwater, &search_size);

  if (unlikely(gen_code_size < 0)) {
    switch (gen_code_size) {
    case -1:
      xtm_is_bo = 1;
      goto buffer_overflow;
    case -2:
      // TODO what's icount
      // max_insns = tb->icount;
      assert(max_insns > 1);
      max_insns /= 2;
      goto tb_overflow;
    default:
      g_assert_not_reached();
    }
  }
  tb->tc.size = gen_code_size;
  atomic_set(
      &tcg_ctx->code_gen_ptr,
      (void *)ROUND_UP((uintptr_t)gen_code_buf + gen_code_size + search_size,
                       CODE_GEN_ALIGN));

  /* init jump list */
  // qemu_spin_init(&tb->jmp_lock); // TODO no spin lock
  tb->jmp_list_head = (uintptr_t)NULL;
  tb->jmp_list_next[0] = (uintptr_t)NULL;
  tb->jmp_list_next[1] = (uintptr_t)NULL;
  tb->jmp_dest[0] = (uintptr_t)NULL;
  tb->jmp_dest[1] = (uintptr_t)NULL;

  /* init original jump addresses which have been set during tcg_gen_code() */
  if (tb->jmp_reset_offset[0] != TB_JMP_RESET_OFFSET_INVALID) {
    tb_reset_jump(tb, 0);
  }
  if (tb->jmp_reset_offset[1] != TB_JMP_RESET_OFFSET_INVALID) {
    tb_reset_jump(tb, 1);
  }

  /* check next page if needed */
  virt_page2 = (pc + tb->size - 1) & TARGET_PAGE_MASK;
  phys_page2 = -1;
  if ((pc & TARGET_PAGE_MASK) != virt_page2) {
    phys_page2 = get_page_addr_code(env, virt_page2);
  }
  /*
   * No explicit memory barrier is required -- tb_link_page() makes the
   * TB visible in a consistent state.
   */
  existing_tb = tb_link_page(tb, phys_pc, phys_page2);
  /* if the TB already exists, discard what we just translated */
  if (unlikely(existing_tb != tb)) {
    uintptr_t orig_aligned = (uintptr_t)gen_code_buf;

    orig_aligned -= ROUND_UP(sizeof(*tb), qemu_icache_linesize);
    atomic_set(&tcg_ctx->code_gen_ptr, (void *)orig_aligned);
    return existing_tb;
  }
  tcg_tb_insert(tb);
  return tb;
}
