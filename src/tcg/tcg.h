#ifndef TCG_H_FXBBEZAS
#define TCG_H_FXBBEZAS

#include "../../include/exec/exec-all.h"
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

#endif
