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

#ifndef TARGET_INSN_START_EXTRA_WORDS
# define TARGET_INSN_START_WORDS 1
#else
# define TARGET_INSN_START_WORDS (1 + TARGET_INSN_START_EXTRA_WORDS)
#endif

TranslationBlock *tcg_tb_alloc(TCGContext *s);
#endif /* end of include guard: TCG_H_FXBBEZAS */
