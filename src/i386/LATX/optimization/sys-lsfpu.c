#include "lsenv.h"
#include "common.h"
#include "reg-alloc.h"
#include "flag-lbt.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>

void latxs_tr_load_lstop_from_env(IR2_OPND *top)
{
    if (!option_lsfpu || option_soft_fpu) {
        return;
    }

    IR2_OPND label_exit;
    int i;

    label_exit = latxs_ir2_opnd_new_label();

    latxs_append_ir2_opnd2i(LISA_LD_H, top, &latxs_env_ir2_opnd,
            lsenv_offset_of_top(lsenv));

    for (i = 0; i < 8; i++) {
        latxs_append_ir2_opndi(LISA_X86MTTOP, i);
        latxs_append_ir2_opnd3(LISA_BEQ, top,
                &latxs_zero_ir2_opnd, &label_exit);
        latxs_append_ir2_opnd2i(LISA_ADDI_W, top, top, -1);
    }

    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);

}

void latxs_tr_save_lstop_to_env(IR2_OPND *top)
{
    if (!option_lsfpu || option_soft_fpu) {
        return;
    }

    latxs_append_ir2_opnd1(LISA_X86MFTOP, top);
    latxs_append_ir2_opnd2i(LISA_ANDI, top, top, 0x7);
    latxs_append_ir2_opnd2i(LISA_ST_W, top, &latxs_env_ir2_opnd,
            lsenv_offset_of_top(lsenv));
}

void latxs_tr_gen_top_mode_init(void)
{
    if (option_lsfpu && !option_soft_fpu) {
        latxs_append_ir2_opndi(LISA_X86MTTOP, 0);
        latxs_append_ir2_opnd0(LISA_X86SETTM);
    }
}

void latxs_tr_fpu_enable_top_mode(void)
{
    if (option_lsfpu && !option_soft_fpu) {
        latxs_append_ir2_opnd0(LISA_X86SETTM);
    }
}

void latxs_tr_fpu_disable_top_mode(void)
{
    if (option_lsfpu && !option_soft_fpu) {
        latxs_append_ir2_opnd0(LISA_X86CLRTM);
    }
}
