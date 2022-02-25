#include "../include/common.h"
#include "../include/env.h"
#include "../include/etb.h"
#include "../ir2/ir2.h"
#include "../ir1/ir1.h"
#include "../include/reg-alloc.h"

#include "../include/flag-lbt.h"
#include "../x86tomips-options.h"

#include "../include/ibtc.h"
#include "../include/profile.h"
#include "../include/flag-pattern.h"
#include "../include/shadow-stack.h"

#include <string.h>

/* we have no inst to mov from gpr to top, so we have to be silly */
void tr_load_lstop_from_env(IR2_OPND *top)
{
    if (!option_lsfpu) return;

    IR2_OPND label_exit;
    int i;

    label_exit = ir2_opnd_new_label();

    append_ir2_opnd2i(LISA_LD_H, top, &env_ir2_opnd,
            lsenv_offset_of_top(lsenv));

    for (i = 0; i < 8; i++) {
        append_ir2_opndi(LISA_X86MTTOP, i);
        append_ir2_opnd2i(LISA_ADDI_W, top, top, -1);
        append_ir2_opnd3(LISA_BEQ, top, &zero_ir2_opnd, &label_exit);
    }

    append_ir2_opnd1(LISA_LABEL, &label_exit);

}

void tr_save_lstop_to_env(IR2_OPND *top)
{
    if (!option_lsfpu) return;

    append_ir2_opnd1(LISA_X86MFTOP, top);
    append_ir2_opnd2i(LISA_ANDI, top, top, 0x7);
    append_ir2_opnd2i(LISA_ST_W, top, &env_ir2_opnd,
            lsenv_offset_of_top(lsenv));
}

void tr_gen_top_mode_init(void)
{
    if (option_lsfpu) {
        append_ir2_opndi(LISA_X86MTTOP, 0);
        append_ir2_opnd0(LISA_X86SETTM);
    }
}

void tr_fpu_enable_top_mode(void)
{
    if (option_lsfpu) {
        append_ir2_opnd0(LISA_X86SETTM);
    }
}

void tr_fpu_disable_top_mode(void)
{
    if (option_lsfpu) {
        append_ir2_opnd0(LISA_X86CLRTM);
    }
}
