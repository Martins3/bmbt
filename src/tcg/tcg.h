#ifndef TCG_H_FXBBEZAS
#define TCG_H_FXBBEZAS

#include "../../include/exec/exec-all.h"

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

/* call flags */
/* Helper does not read globals (either directly or through an exception). It
   implies TCG_CALL_NO_WRITE_GLOBALS. */
#define TCG_CALL_NO_READ_GLOBALS    0x0001
/* Helper does not write globals */
#define TCG_CALL_NO_WRITE_GLOBALS   0x0002
/* Helper can be safely suppressed if the return value is not used. */
#define TCG_CALL_NO_SIDE_EFFECTS    0x0004
/* Helper is QEMU_NORETURN.  */
#define TCG_CALL_NO_RETURN          0x0008

/* convenience version of most used call flags */
#define TCG_CALL_NO_RWG         TCG_CALL_NO_READ_GLOBALS
#define TCG_CALL_NO_WG          TCG_CALL_NO_WRITE_GLOBALS
#define TCG_CALL_NO_SE          TCG_CALL_NO_SIDE_EFFECTS
#define TCG_CALL_NO_RWG_SE      (TCG_CALL_NO_RWG | TCG_CALL_NO_SE)
#define TCG_CALL_NO_WG_SE       (TCG_CALL_NO_WG | TCG_CALL_NO_SE)

TranslationBlock *tcg_tb_alloc(TCGContext *s);

#ifdef CONFIG_X86toMIPS
void xtm_helper_ret_stb_mmu(CPUArchState*, target_ulong, uint8_t, TCGMemOpIdx);
void xtm_helper_le_stw_mmu(CPUArchState*, target_ulong, uint16_t, TCGMemOpIdx);
void xtm_helper_le_stl_mmu(CPUArchState*, target_ulong, uint32_t, TCGMemOpIdx);
void xtm_helper_le_stq_mmu(CPUArchState*, target_ulong, uint64_t, TCGMemOpIdx);
#endif
void helper_be_stw_mmu(CPUArchState *env, target_ulong addr, uint16_t val,
                       TCGMemOpIdx oi, uintptr_t retaddr);
void helper_be_stl_mmu(CPUArchState *env, target_ulong addr, uint32_t val,
                       TCGMemOpIdx oi, uintptr_t retaddr);
void helper_be_stq_mmu(CPUArchState *env, target_ulong addr, uint64_t val,
                       TCGMemOpIdx oi, uintptr_t retaddr);

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
uint16_t helper_be_lduw_cmmu(CPUArchState *env, target_ulong addr,
                             TCGMemOpIdx oi, uintptr_t retaddr);
int16_t helper_be_ldsw_cmmu(CPUArchState *env, target_ulong addr,
                             TCGMemOpIdx oi, uintptr_t retaddr);
uint32_t helper_be_ldl_cmmu(CPUArchState *env, target_ulong addr,
                            TCGMemOpIdx oi, uintptr_t retaddr);
uint64_t helper_be_ldq_cmmu(CPUArchState *env, target_ulong addr,
                            TCGMemOpIdx oi, uintptr_t retaddr);
#endif /* end of include guard: TCG_H_FXBBEZAS */
