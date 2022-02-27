#include "i386/LATX/include/common.h"
#include "i386/LATX/include/latx-types.h"
#include "i386/LATX/include/la-ir2.h"
#include <exec/exec-all.h>
int sigint_enabled(void) { return 0; }
void latxs_sigint_prepare_check_jmp_glue_2(IR2_OPND lst, IR2_OPND led) {}
void latxs_tb_relink(TranslationBlock *utb) {}
void latxs_tr_gen_save_currtb_for_int(void) {}
