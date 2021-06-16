#ifndef TCG_H_FXBBEZAS
#define TCG_H_FXBBEZAS

#include "../../include/exec/exec-all.h"
#include "../../include/exec/memop.h"
#include "../../include/qemu/config-target.h"

// FIXME
// wow, it almost destoried me
// tcg_target_reg_bits is defined at 
// /home/maritns3/core/ld/x86-qemu-mips/tcg/loongarch/tcg-target.h
// FIXME it here
# define TCG_TARGET_REG_BITS 64

#if tcg_target_reg_bits == 32
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

typedef struct TCGContext {

  /* goto_tb support */
  tcg_insn_unit *code_buf;
  u16 *tb_jmp_reset_offset;      /* tb->jmp_reset_offset */
  uintptr_t *tb_jmp_insn_offset; /* tb->jmp_target_arg if direct_jump */
  uintptr_t *tb_jmp_target_addr; /* tb->jmp_target_arg if !direct_jump */

  u32 tb_cflags; /* cflags of the current TB */

  /* Code generation.  Note that we specifically do not use tcg_insn_unit
     here, because there's too much arithmetic throughout that relies
     on addition and subtraction working on bytes.  Rely on the GCC
     extension that allows arithmetic on void*.  */
  void *code_gen_prologue;
  void *code_gen_epilogue;
  void *code_gen_buffer;
  size_t code_gen_buffer_size;
  void *code_gen_ptr;
  void *data_gen_ptr;

  /* Threshold to flush the translated code buffer.  */
  void *code_gen_highwater;

} TCGContext;

// FIXME tcg.h is being well included
extern TCGContext *tcg_ctx;

/* Combine the MemOp and mmu_idx parameters into a single value.  */
typedef uint32_t TCGMemOpIdx;

#ifndef TARGET_INSN_START_EXTRA_WORDS
# define TARGET_INSN_START_WORDS 1
#else
# define TARGET_INSN_START_WORDS (1 + TARGET_INSN_START_EXTRA_WORDS)
#endif

TranslationBlock *tcg_tb_alloc(TCGContext *s);

// FIXME I include too many prototype here
// remove unnecessary ones later
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
uint64_t helper_le_ldq_mmu(CPUArchState *env, target_ulong addr,
                           TCGMemOpIdx oi, uintptr_t retaddr);
#ifdef CONFIG_X86toMIPS
tcg_target_ulong xtm_helper_ret_ldub_mmu(CPUArchState*, target_ulong, TCGMemOpIdx);
tcg_target_ulong xtm_helper_le_lduw_mmu(CPUArchState*, target_ulong, TCGMemOpIdx);
tcg_target_ulong xtm_helper_le_ldul_mmu(CPUArchState*, target_ulong, TCGMemOpIdx);
uint64_t xtm_helper_le_ldq_mmu(CPUArchState*, target_ulong, TCGMemOpIdx);
#endif

/* Value sign-extended to tcg register size.  */
tcg_target_ulong helper_ret_ldsb_mmu(CPUArchState *env, target_ulong addr,
                                     TCGMemOpIdx oi, uintptr_t retaddr);
tcg_target_ulong helper_le_ldsw_mmu(CPUArchState *env, target_ulong addr,
                                    TCGMemOpIdx oi, uintptr_t retaddr);
tcg_target_ulong helper_le_ldsl_mmu(CPUArchState *env, target_ulong addr,
                                    TCGMemOpIdx oi, uintptr_t retaddr);
#ifdef CONFIG_X86toMIPS
tcg_target_ulong xtm_helper_ret_ldsb_mmu(CPUArchState*, target_ulong, TCGMemOpIdx);
tcg_target_ulong xtm_helper_le_ldsw_mmu(CPUArchState*, target_ulong, TCGMemOpIdx);
tcg_target_ulong xtm_helper_le_ldsl_mmu(CPUArchState*, target_ulong, TCGMemOpIdx);
#endif

void helper_ret_stb_mmu(CPUArchState *env, target_ulong addr, uint8_t val,
                        TCGMemOpIdx oi, uintptr_t retaddr);
void helper_le_stw_mmu(CPUArchState *env, target_ulong addr, uint16_t val,
                       TCGMemOpIdx oi, uintptr_t retaddr);
void helper_le_stl_mmu(CPUArchState *env, target_ulong addr, uint32_t val,
                       TCGMemOpIdx oi, uintptr_t retaddr);
void helper_le_stq_mmu(CPUArchState *env, target_ulong addr, uint64_t val,
                       TCGMemOpIdx oi, uintptr_t retaddr);
#ifdef CONFIG_X86toMIPS
void xtm_helper_ret_stb_mmu(CPUArchState*, target_ulong, uint8_t, TCGMemOpIdx);
void xtm_helper_le_stw_mmu(CPUArchState*, target_ulong, uint16_t, TCGMemOpIdx);
void xtm_helper_le_stl_mmu(CPUArchState*, target_ulong, uint32_t, TCGMemOpIdx);
void xtm_helper_le_stq_mmu(CPUArchState*, target_ulong, uint64_t, TCGMemOpIdx);
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
#endif /* end of include guard: TCG_H_FXBBEZAS */

/**
 * get_mmuidx
 * @oi: combined op/idx parameter
 *
 * Extract the mmu index from the combined value.
 */
static inline unsigned get_mmuidx(TCGMemOpIdx oi)
{
    return oi & 15;
}

// FIXME cputlb.c
// static inline MemOp get_memop(TCGMemOpIdx oi)

/**
 * get_alignment_bits
 * @memop: MemOp value
 *
 * Extract the alignment size from the memop.
 */
static inline unsigned get_alignment_bits(MemOp memop)
{
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
    // FIXME later
    // tcg_debug_assert((TLB_FLAGS_MASK & ((1 << a) - 1)) == 0);
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
#define TB_EXIT_MASK      3
#define TB_EXIT_IDX0      0
#define TB_EXIT_IDX1      1
#define TB_EXIT_IDXMAX    1
#define TB_EXIT_REQUESTED 3


#ifdef HAVE_TCG_QEMU_TB_EXEC
uintptr_t tcg_qemu_tb_exec(CPUArchState *env, uint8_t *tb_ptr);
#elif defined(CONFIG_X86toMIPS)
# define tcg_qemu_tb_exec(env, tb_ptr) \
    ((uintptr_t (*)(void *, void *))context_switch_bt_to_native)(tb_ptr, env); \
    if (current_cpu->exception_index == EXCP_DEBUG) { \
        current_cpu->can_do_io = 1; \
        siglongjmp(current_cpu->jmp_env, 1); \
    }
#else
# define tcg_qemu_tb_exec(env, tb_ptr) \
    ((uintptr_t (*)(void *, void *))tcg_ctx->code_gen_prologue)(env, tb_ptr)
#endif

#endif
