#include "common.h"
#include "latx-types.h"
#include "la-ir2.h"
#include <exec/exec-all.h>
// sys-sigint.c
int sigint_enabled(void) { return 0; }
void latxs_sigint_prepare_check_jmp_glue_2(IR2_OPND lst, IR2_OPND led) {}
void latxs_tb_relink(TranslationBlock *utb) {}
void latxs_tr_gen_save_currtb_for_int(void) {}
void latxs_init_rr_thread_signal(CPUState *cpu) {}
void tcg_sigint_init(void) {}
