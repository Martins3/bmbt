#ifndef TCG_H_FXBBEZAS
#define TCG_H_FXBBEZAS

#include "loongarch/tcg-target.h"
#include <exec/cpu-all.h>
#include <exec/exec-all.h>
#include <exec/memop.h>
#include <qemu/config-target.h>
#include <stddef.h>
#include <stdint.h>
#include <uglib.h>

#if TARGET_LONG_BITS == 32
#define TCGv TCGv_i32
#elif TARGET_LONG_BITS == 64
#define TCGv TCGv_i64
#else
#error Unhandled TARGET_LONG_BITS value
#endif

#if TCG_TARGET_REG_BITS == 32
typedef int32_t tcg_target_long;
typedef uint32_t tcg_target_ulong;
#define TCG_PRIlx PRIx32
#define TCG_PRIld PRId32
#elif TCG_TARGET_REG_BITS == 64
typedef int64_t tcg_target_long;
typedef uint64_t tcg_target_ulong;
#define TCG_PRIlx PRIx64
#define TCG_PRIld PRId64
#else
#error unsupported
#endif

#if TCG_TARGET_NB_REGS <= 32
typedef uint32_t TCGRegSet;
#elif TCG_TARGET_NB_REGS <= 64
typedef uint64_t TCGRegSet;
#else
#error unsupported
#endif

#ifndef TCG_TARGET_INSN_UNIT_SIZE
#error "Missing TCG_TARGET_INSN_UNIT_SIZE"
#elif TCG_TARGET_INSN_UNIT_SIZE == 1
typedef uint8_t tcg_insn_unit;
#elif TCG_TARGET_INSN_UNIT_SIZE == 2
typedef uint16_t tcg_insn_unit;
#elif TCG_TARGET_INSN_UNIT_SIZE == 4
typedef uint32_t tcg_insn_unit;
#elif TCG_TARGET_INSN_UNIT_SIZE == 8
typedef uint64_t tcg_insn_unit;
#else
/* The port better have done this.  */
#endif

#define TCG_MAX_INSNS 512

#ifndef TARGET_INSN_START_EXTRA_WORDS
#define TARGET_INSN_START_WORDS 1
#else
#define TARGET_INSN_START_WORDS (1 + TARGET_INSN_START_EXTRA_WORDS)
#endif

typedef struct TCGContext {
  /* goto_tb support */
  tcg_insn_unit *code_buf;
  uint16_t *tb_jmp_reset_offset; /* tb->jmp_reset_offset */
  uintptr_t *tb_jmp_insn_offset; /* tb->jmp_target_arg if direct_jump */
  uintptr_t *tb_jmp_target_addr; /* tb->jmp_target_arg if !direct_jump */

  // xqm doesn't need it
  // uint32_t tb_cflags; /* cflags of the current TB */

  /* Code generation.  Note that we specifically do not use tcg_insn_unit
     here, because there's too much arithmetic throughout that relies
     on addition and subtraction working on bytes.  Rely on the GCC
     extension that allows arithmetic on void*.  */
#ifdef BMBT
  void *code_gen_prologue;
  void *code_gen_epilogue;
#endif
  void *code_gen_buffer;
  size_t code_gen_buffer_size;
  void *code_gen_ptr;
  void *data_gen_ptr;

  /* Threshold to flush the translated code buffer.  */
  void *code_gen_highwater;

  tcg_insn_unit *code_ptr;

  TCGRegSet reserved_regs;

  size_t tb_phys_invalidate_count;

  uint16_t gen_insn_end_off[TCG_MAX_INSNS];
  target_ulong gen_insn_data[TCG_MAX_INSNS][TARGET_INSN_START_WORDS];
} TCGContext;

extern TCGContext tcg_init_ctx;
extern TCGContext *tcg_ctx;

/* Combine the MemOp and mmu_idx parameters into a single value.  */
typedef uint32_t TCGMemOpIdx;

/*
 * Memory helpers that will be used by TCG generated code.
 */
#ifdef CONFIG_SOFTMMU
/* Value zero-extended to tcg register size.  */
tcg_target_ulong helper_ret_ldub_mmu(CPUArchState *env, target_ulong addr,
                                     TCGMemOpIdx oi, uintptr_t retaddr);
tcg_target_ulong helper_le_lduw_mmu(CPUArchState *env, target_ulong addr,
                                    TCGMemOpIdx oi, uintptr_t retaddr);
tcg_target_ulong helper_le_ldul_mmu(CPUArchState *env, target_ulong addr,
                                    TCGMemOpIdx oi, uintptr_t retaddr);
uint64_t helper_le_ldq_mmu(CPUArchState *env, target_ulong addr, TCGMemOpIdx oi,
                           uintptr_t retaddr);
/* Value sign-extended to tcg register size.  */
tcg_target_ulong helper_ret_ldsb_mmu(CPUArchState *env, target_ulong addr,
                                     TCGMemOpIdx oi, uintptr_t retaddr);
tcg_target_ulong helper_le_ldsw_mmu(CPUArchState *env, target_ulong addr,
                                    TCGMemOpIdx oi, uintptr_t retaddr);
tcg_target_ulong helper_le_ldsl_mmu(CPUArchState *env, target_ulong addr,
                                    TCGMemOpIdx oi, uintptr_t retaddr);

void helper_ret_stb_mmu(CPUArchState *env, target_ulong addr, uint8_t val,
                        TCGMemOpIdx oi, uintptr_t retaddr);
void helper_le_stw_mmu(CPUArchState *env, target_ulong addr, uint16_t val,
                       TCGMemOpIdx oi, uintptr_t retaddr);
void helper_le_stl_mmu(CPUArchState *env, target_ulong addr, uint32_t val,
                       TCGMemOpIdx oi, uintptr_t retaddr);
void helper_le_stq_mmu(CPUArchState *env, target_ulong addr, uint64_t val,
                       TCGMemOpIdx oi, uintptr_t retaddr);

#if defined(TARGET_I386) && defined(CONFIG_LATX)
void latxs_helper_le_lddq_mmu(CPUArchState *env, target_ulong addr,
                              TCGMemOpIdx oi, uintptr_t retaddr);
void latxs_helper_le_stdq_mmu(CPUArchState *env, target_ulong addr,
                              uint64_t val, TCGMemOpIdx oi, uintptr_t retaddr);
#endif

uint8_t helper_ret_ldub_cmmu(CPUArchState *env, target_ulong addr,
                             TCGMemOpIdx oi, uintptr_t retaddr);
int8_t helper_ret_ldsb_cmmu(CPUArchState *env, target_ulong addr,
                            TCGMemOpIdx oi, uintptr_t retaddr);
uint16_t helper_le_lduw_cmmu(CPUArchState *env, target_ulong addr,
                             TCGMemOpIdx oi, uintptr_t retaddr);
int16_t helper_le_ldsw_cmmu(CPUArchState *env, target_ulong addr,
                            TCGMemOpIdx oi, uintptr_t retaddr);
uint32_t helper_le_ldl_cmmu(CPUArchState *env, target_ulong addr,
                            TCGMemOpIdx oi, uintptr_t retaddr);
uint64_t helper_le_ldq_cmmu(CPUArchState *env, target_ulong addr,
                            TCGMemOpIdx oi, uintptr_t retaddr);

/* Temporary aliases until backends are converted.  */
#ifdef TARGET_WORDS_BIGENDIAN
#define helper_ret_ldsw_mmu helper_be_ldsw_mmu
#define helper_ret_lduw_mmu helper_be_lduw_mmu
#define helper_ret_ldsl_mmu helper_be_ldsl_mmu
#define helper_ret_ldul_mmu helper_be_ldul_mmu
#define helper_ret_ldl_mmu helper_be_ldul_mmu
#define helper_ret_ldq_mmu helper_be_ldq_mmu
#define helper_ret_stw_mmu helper_be_stw_mmu
#define helper_ret_stl_mmu helper_be_stl_mmu
#define helper_ret_stq_mmu helper_be_stq_mmu
#define helper_ret_lduw_cmmu helper_be_lduw_cmmu
#define helper_ret_ldsw_cmmu helper_be_ldsw_cmmu
#define helper_ret_ldl_cmmu helper_be_ldl_cmmu
#define helper_ret_ldq_cmmu helper_be_ldq_cmmu
#else
#define helper_ret_ldsw_mmu helper_le_ldsw_mmu
#define helper_ret_lduw_mmu helper_le_lduw_mmu
#define helper_ret_ldsl_mmu helper_le_ldsl_mmu
#define helper_ret_ldul_mmu helper_le_ldul_mmu
#define helper_ret_ldl_mmu helper_le_ldul_mmu
#define helper_ret_ldq_mmu helper_le_ldq_mmu
#define helper_ret_stw_mmu helper_le_stw_mmu
#define helper_ret_stl_mmu helper_le_stl_mmu
#define helper_ret_stq_mmu helper_le_stq_mmu
#define helper_ret_lduw_cmmu helper_le_lduw_cmmu
#define helper_ret_ldsw_cmmu helper_le_ldsw_cmmu
#define helper_ret_ldl_cmmu helper_le_ldl_cmmu
#define helper_ret_ldq_cmmu helper_le_ldq_cmmu
#endif
#endif

/**
 * get_mmuidx
 * @oi: combined op/idx parameter
 *
 * Extract the mmu index from the combined value.
 */
static inline unsigned get_mmuidx(TCGMemOpIdx oi) { return oi & 15; }

/**
 * get_memop
 * @oi: combined op/idx parameter
 *
 * Extract the memory operation from the combined value.
 */
static inline MemOp get_memop(TCGMemOpIdx oi) { return oi >> 4; }

#if defined CONFIG_DEBUG_TCG || defined QEMU_STATIC_ANALYSIS
#define tcg_debug_assert(X)                                                    \
  do {                                                                         \
    assert(X);                                                                 \
  } while (0)
#else
#define tcg_debug_assert(X)                                                    \
  do {                                                                         \
    if (!(X)) {                                                                \
      __builtin_unreachable();                                                 \
    }                                                                          \
  } while (0)
#endif

/**
 * get_alignment_bits
 * @memop: MemOp value
 *
 * Extract the alignment size from the memop.
 */
static inline unsigned get_alignment_bits(MemOp memop) {
  unsigned a = memop & MO_AMASK;

  if (a == MO_UNALN) {
    /* No alignment required.  */
    a = 0;
  } else if (a == MO_ALIGN) {
    /* A natural alignment requirement.  */
    a = memop & MO_SIZE;
  } else {
    /* A specific alignment requirement.  */
    a = a >> MO_ASHIFT;
  }
#if defined(CONFIG_SOFTMMU)
  /* The requested alignment cannot overlap the TLB flags.  */
  tcg_debug_assert((TLB_FLAGS_MASK & ((1 << a) - 1)) == 0);
#endif
  return a;
}

/**
 * tcg_qemu_tb_exec:
 * @env: pointer to CPUArchState for the CPU
 * @tb_ptr: address of generated code for the TB to execute
 *
 * Start executing code from a given translation block.
 * Where translation blocks have been linked, execution
 * may proceed from the given TB into successive ones.
 * Control eventually returns only when some action is needed
 * from the top-level loop: either control must pass to a TB
 * which has not yet been directly linked, or an asynchronous
 * event such as an interrupt needs handling.
 *
 * Return: The return value is the value passed to the corresponding
 * tcg_gen_exit_tb() at translation time of the last TB attempted to execute.
 * The value is either zero or a 4-byte aligned pointer to that TB combined
 * with additional information in its two least significant bits. The
 * additional information is encoded as follows:
 *  0, 1: the link between this TB and the next is via the specified
 *        TB index (0 or 1). That is, we left the TB via (the equivalent
 *        of) "goto_tb <index>". The main loop uses this to determine
 *        how to link the TB just executed to the next.
 *  2:    we are using instruction counting code generation, and we
 *        did not start executing this TB because the instruction counter
 *        would hit zero midway through it. In this case the pointer
 *        returned is the TB we were about to execute, and the caller must
 *        arrange to execute the remaining count of instructions.
 *  3:    we stopped because the CPU's exit_request flag was set
 *        (usually meaning that there is an interrupt that needs to be
 *        handled). The pointer returned is the TB we were about to execute
 *        when we noticed the pending exit request.
 *
 * If the bottom two bits indicate an exit-via-index then the CPU
 * state is correctly synchronised and ready for execution of the next
 * TB (and in particular the guest PC is the address to execute next).
 * Otherwise, we gave up on execution of this TB before it started, and
 * the caller must fix up the CPU state by calling the CPU's
 * synchronize_from_tb() method with the TB pointer we return (falling
 * back to calling the CPU's set_pc method with tb->pb if no
 * synchronize_from_tb() method exists).
 *
 * Note that TCG targets may use a different definition of tcg_qemu_tb_exec
 * to this default (which just calls the prologue.code emitted by
 * tcg_target_qemu_prologue()).
 */
#define TB_EXIT_MASK 3
#define TB_EXIT_IDX0 0
#define TB_EXIT_IDX1 1
#define TB_EXIT_IDXMAX 1
#define TB_EXIT_REQUESTED 3

#ifdef HAVE_TCG_QEMU_TB_EXEC
uintptr_t tcg_qemu_tb_exec(CPUArchState *env, uint8_t *tb_ptr);
#elif defined(CONFIG_LATX)
#define tcg_qemu_tb_exec(env, tb_ptr)                                          \
  ((uintptr_t(*)(void *, void *))context_switch_bt_to_native)(tb_ptr, env);    \
  if (current_cpu->exception_index == EXCP_DEBUG) {                            \
    current_cpu->can_do_io = 1;                                                \
    longjmp(current_cpu->jmp_env, 1);                                          \
  }
#else
#define tcg_qemu_tb_exec(env, tb_ptr)                                          \
  ((uintptr_t(*)(void *, void *))tcg_ctx->code_gen_prologue)(env, tb_ptr)
#endif

/**
 * tcg_ptr_byte_diff
 * @a, @b: addresses to be differenced
 *
 * There are many places within the TCG backends where we need a byte
 * difference between two pointers.  While this can be accomplished
 * with local casting, it's easy to get wrong -- especially if one is
 * concerned with the signedness of the result.
 *
 * This version relies on GCC's void pointer arithmetic to get the
 * correct result.
 */

static inline ptrdiff_t tcg_ptr_byte_diff(void *a, void *b) { return a - b; }

/**
 * tcg_current_code_size
 * @s: the tcg context
 *
 * Compute the current code size within the translation block.
 * This is used to fill in qemu's data structures for goto_tb.
 */

static inline size_t tcg_current_code_size(TCGContext *s) {
  return tcg_ptr_byte_diff(s->code_ptr, s->code_buf);
}

#define tcg_abort()                                                            \
  do {                                                                         \
  } while (0)

/* when the size of the arguments of a called function is smaller than
   this value, they are statically allocated in the TB stack frame */
#define TCG_STATIC_CALL_ARGS_SIZE 128

#define tcg_regset_set_reg(d, r) ((d) |= (TCGRegSet)1 << (r))
#define tcg_regset_reset_reg(d, r) ((d) &= ~((TCGRegSet)1 << (r)))
#define tcg_regset_test_reg(d, r) (((d) >> (r)) & 1)

size_t tcg_nb_tbs(void);

void tcg_region_init(void);
void tcg_region_reset_all(void);

void tcg_context_init(TCGContext *s);

/**
 * make_memop_idx
 * @op: memory operation
 * @idx: mmu index
 *
 * Encode these values into a single parameter.
 */
static inline TCGMemOpIdx make_memop_idx(MemOp op, unsigned idx) {
  tcg_debug_assert(idx <= 15);
  return (op << 4) | idx;
}

void tcg_prologue_init(TCGContext *s);

// bmbt: only one copied here
uint64_t helper_atomic_cmpxchgq_le_mmu(CPUArchState *env, target_ulong addr,
                                       uint64_t cmpv, uint64_t newv,
                                       TCGMemOpIdx oi, uintptr_t retaddr);

TranslationBlock *tcg_tb_lookup(uintptr_t tc_ptr);
TranslationBlock *tcg_tb_alloc(TCGContext *s);
void tcg_tb_insert(TranslationBlock *tb);
void tcg_tb_remove(TranslationBlock *tb);
void tcg_tb_foreach(GTraverseFunc func, gpointer user_data);

size_t tcg_code_size(void);

void tcg_register_thread(void);

#endif /* end of include guard: TCG_H_FXBBEZAS */
