#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>

void latxs_sys_fctrl_register_ir1(void)
{
    latxs_register_ir1(X86_INS_FNSTCW);
    latxs_register_ir1(X86_INS_FLDCW);

    latxs_register_ir1(X86_INS_FNSTSW);

    latxs_register_ir1(X86_INS_STMXCSR);
    latxs_register_ir1(X86_INS_LDMXCSR);

    latxs_register_ir1(X86_INS_FCOMI);
    latxs_register_ir1(X86_INS_FCOMIP);
    latxs_register_ir1(X86_INS_FUCOMI);
    latxs_register_ir1(X86_INS_FUCOMIP);

    latxs_register_ir1(X86_INS_FCOM);
    latxs_register_ir1(X86_INS_FCOMP);
    latxs_register_ir1(X86_INS_FCOMPP);
    latxs_register_ir1(X86_INS_FUCOM);
    latxs_register_ir1(X86_INS_FUCOMP);
    latxs_register_ir1(X86_INS_FUCOMPP);

    latxs_register_ir1(X86_INS_FICOM);
    latxs_register_ir1(X86_INS_FICOMP);

    latxs_register_ir1(X86_INS_FNCLEX);
    latxs_register_ir1(X86_INS_EMMS);
}

#if defined(LATX_SYS_FCSR_EXCP)
/*
 * disabled in common.h:
 *      For now, just ignore fcsr exception in sys-mode.
 * TODO: support fcsr exception in sys-mode
 */

static void latxs_update_fcsr_flag(IR2_OPND status_word, IR2_OPND fcsr)
{
    IR2_OPND temp = latxs_ra_alloc_itemp();
    IR2_OPND zero_ir2_opnd = latxs_zero_ir2_opnd;

    /* reset flags*/
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &zero_ir2_opnd,
                            FCSR_OFF_FLAGS_V, FCSR_OFF_FLAGS_I);
    /* IE */
    latxs_append_ir2_opnd2ii(LISA_BSTRPICK_W, &temp, &status_word,
                            X87_SR_OFF_IE, X87_SR_OFF_IE);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &temp,
                            FCSR_OFF_FLAGS_V, FCSR_OFF_FLAGS_V);
    /* ZE */
    latxs_append_ir2_opnd2ii(LISA_BSTRPICK_W, &temp, &status_word,
                            X87_SR_OFF_ZE, X87_SR_OFF_ZE);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &temp,
                            FCSR_OFF_FLAGS_Z, FCSR_OFF_FLAGS_Z);
    /* OE */
    latxs_append_ir2_opnd2ii(LISA_BSTRPICK_W, &temp, &status_word,
                            X87_SR_OFF_OE, X87_SR_OFF_OE);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &temp,
                            FCSR_OFF_FLAGS_O, FCSR_OFF_FLAGS_O);
    /* UE */
    latxs_append_ir2_opnd2ii(LISA_BSTRPICK_W, &temp, &status_word,
                            X87_SR_OFF_UE, X87_SR_OFF_UE);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &temp,
                            FCSR_OFF_FLAGS_U, FCSR_OFF_FLAGS_U);
    /* PE */
    latxs_append_ir2_opnd2ii(LISA_BSTRPICK_W, &temp, &status_word,
                            X87_SR_OFF_PE, X87_SR_OFF_PE);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &temp,
                            FCSR_OFF_FLAGS_I, FCSR_OFF_FLAGS_I);
    latxs_append_ir2_opnd2(LISA_MOVGR2FCSR, &latxs_fcsr_ir2_opnd, &fcsr);

    latxs_ra_free_temp(&temp);
}

static void latxs_update_fcsr_by_sw(IR2_OPND sw)
{
    IR2_OPND old_fcsr = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2(LISA_MOVFCSR2GR, &old_fcsr, &latxs_fcsr_ir2_opnd);
    latxs_update_fcsr_flag(sw, old_fcsr);
    latxs_ra_free_temp(&old_fcsr);
}

static void latxs_update_fcsr_enable(
        IR2_OPND control_word, IR2_OPND fcsr)
{
    IR2_OPND temp = latxs_ra_alloc_itemp();

    /* reset enables */
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr,
            &latxs_zero_ir2_opnd, FCSR_OFF_EN_V, FCSR_OFF_EN_I);
    /* IM */
    latxs_append_ir2_opnd2ii(LISA_BSTRPICK_W, &temp, &control_word,
            X87_CR_OFF_IM, X87_CR_OFF_IM);
    latxs_append_ir2_opnd2i(LISA_XORI, &temp, &temp, 1);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &temp,
            FCSR_OFF_EN_V, FCSR_OFF_EN_V);
    /* DM */
    latxs_append_ir2_opnd2ii(LISA_BSTRPICK_W, &temp, &control_word,
            X87_CR_OFF_DM, X87_CR_OFF_DM);
    latxs_append_ir2_opnd2i(LISA_XORI, &temp, &temp, 1);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &temp,
            FCSR_OFF_EN_I, FCSR_OFF_EN_I);
    /* ZM */
    latxs_append_ir2_opnd2ii(LISA_BSTRPICK_W, &temp, &control_word,
            X87_CR_OFF_ZM, X87_CR_OFF_ZM);
    latxs_append_ir2_opnd2i(LISA_XORI, &temp, &temp, 1);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &temp,
            FCSR_OFF_EN_Z, FCSR_OFF_EN_Z);
    /* OM */
    latxs_append_ir2_opnd2ii(LISA_BSTRPICK_W, &temp, &control_word,
            X87_CR_OFF_OM, X87_CR_OFF_OM);
    latxs_append_ir2_opnd2i(LISA_XORI, &temp, &temp, 1);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &temp,
            FCSR_OFF_EN_O, FCSR_OFF_EN_O);
    /* UM */
    latxs_append_ir2_opnd2ii(LISA_BSTRPICK_W, &temp, &control_word,
            X87_CR_OFF_UM, X87_CR_OFF_UM);
    latxs_append_ir2_opnd2i(LISA_XORI, &temp, &temp, 1);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &temp,
            FCSR_OFF_EN_U, FCSR_OFF_EN_U);
    latxs_append_ir2_opnd2(LISA_MOVGR2FCSR, &fcsr_ir2_opnd, &fcsr);

    latxs_ra_free_temp(&temp);
}
#endif

static void latxs_update_fcsr_rm(IR2_OPND control_word, IR2_OPND fcsr)
{
    /* reset rounding mode */
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr,
            &latxs_zero_ir2_opnd, FCSR_OFF_RM + 1, FCSR_OFF_RM);
    /* load rounding mode in x86 control register */
    latxs_append_ir2_opnd2ii(LISA_BSTRPICK_W, &control_word,
            &control_word, X87_CR_OFF_RC + 1, X87_CR_OFF_RC);
    /*
     * turn x86 rm to LA rm
     *          x86      LA
     *
     * RN       00       00
     * RD       01       11
     * RU       10       10
     * RZ       11       01
     *
     */
    IR2_OPND temp_cw = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ANDI, &temp_cw, &control_word, 1);

    IR2_OPND label = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &temp_cw, &latxs_zero_ir2_opnd, &label);
    latxs_append_ir2_opnd2i(LISA_XORI, &control_word, &control_word, 2);
    latxs_append_ir2_opnd1(LISA_LABEL, &label);
    /* set rounding mode in LA FCSR */
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &fcsr, &control_word,
            FCSR_OFF_RM + 1, FCSR_OFF_RM);
    latxs_append_ir2_opnd2(LISA_MOVGR2FCSR, &latxs_fcsr_ir2_opnd, &fcsr);

    latxs_ra_free_temp(&temp_cw);
}

void latxs_update_fcsr_by_cw(IR2_OPND cw)
{
    IR2_OPND old_fcsr = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2(LISA_MOVFCSR2GR, &old_fcsr,
            &latxs_fcsr_ir2_opnd);
    /* latxs_update_fcsr_enable(cw, old_fcsr); */
    latxs_update_fcsr_rm(cw, old_fcsr);
    latxs_ra_free_temp(&old_fcsr);
}

bool latxs_translate_fnstcw(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fnstcw_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    /* 1. load the value of fpu control word */
    IR2_OPND cw_opnd = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_HU, &cw_opnd, &latxs_env_ir2_opnd,
            lsenv_offset_of_control_word(lsenv));

    /* 2. store the control word to the dest_opnd */
    /* fix capstone bug */
    pir1->info->detail->x86.operands[0].size = 2;
    latxs_store_ir2_to_ir1(&cw_opnd, ir1_get_opnd(pir1, 0));

    latxs_ra_free_temp(&cw_opnd);
    return true;
}

bool latxs_translate_fldcw(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fldcw_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    /* 1. load new control word from the source opnd */
    IR2_OPND new_cw = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&new_cw, ir1_get_opnd(pir1, 0), EXMode_N);

    /* 2. store the value into the env->fpu_control_word */
    latxs_append_ir2_opnd2i(LISA_ST_H, &new_cw, &latxs_env_ir2_opnd,
            lsenv_offset_of_control_word(lsenv));

    latxs_update_fcsr_by_cw(new_cw);

    return true;
}

bool latxs_translate_fnstsw(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fnstsw_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    lsassert(ir1_get_opnd_num(pir1) == 1);

    IR2_OPND sw_value = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_HU, &sw_value, &latxs_env_ir2_opnd,
            lsenv_offset_of_status_word(lsenv));

    IR2_OPND mask = latxs_ra_alloc_itemp();
    latxs_load_imm32_to_ir2(&mask, 0xc7ff, EXMode_Z);
    latxs_append_ir2_opnd3(LISA_AND, &sw_value, &sw_value, &mask);
    latxs_ra_free_temp(&mask);

    IR2_OPND top = latxs_ra_alloc_itemp();
    if (!option_lsfpu) {
        latxs_append_ir2_opnd2i(LISA_LD_W, &top,
                &latxs_env_ir2_opnd,
                lsenv_offset_of_top_bias(lsenv));

        latxs_append_ir2_opnd2i(LISA_ADDI_D, &top, &top,
                                             latxs_td_fpu_get_top());
        latxs_append_ir2_opnd2i(LISA_ANDI, &top, &top, 0x7);
    } else {
        latxs_append_ir2_opnd1(LISA_X86MFTOP, &top);
        latxs_append_ir2_opnd2i(LISA_ANDI, &top, &top, 0x7);
    }
    latxs_append_ir2_opnd2i(LISA_SLLI_W, &top, &top, 11);
    latxs_append_ir2_opnd3(LISA_OR, &sw_value, &sw_value, &top);

    /* status_word in memory won't get timely update, */
    /* write back to help a bit */
    if (option_lsfpu) {
        latxs_append_ir2_opnd2i(LISA_ST_H, &sw_value,
                &latxs_env_ir2_opnd,
                lsenv_offset_of_status_word(lsenv));
    }

    /* 2. store the current value of status_word to dest_opnd */
    pir1->info->detail->x86.operands[0].size = 2;
    latxs_store_ir2_to_ir1(&sw_value, ir1_get_opnd(pir1, 0));

    /* 3. free tmp */
    latxs_ra_free_temp(&sw_value);

    return true;
}

bool latxs_translate_stmxcsr(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    if (latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    /* 1. load the value of the mxcsr register state from env */
    IR2_OPND mxcsr_opnd = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_WU, &mxcsr_opnd,
            &latxs_env_ir2_opnd,
            lsenv_offset_of_mxcsr(lsenv));

    /* 2. store  the value of the mxcsr register state to the dest_opnd */
    latxs_store_ir2_to_ir1(&mxcsr_opnd, ir1_get_opnd(pir1, 0));

    latxs_ra_free_temp(&mxcsr_opnd);
    return true;
}

bool latxs_translate_ldmxcsr(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    if (latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    /* 1. load new mxcsr value from the source opnd */
    IR2_OPND new_mxcsr = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&new_mxcsr, ir1_get_opnd(pir1, 0), EXMode_N);

    /* 2. store the value into the env->mxcsr */
    latxs_append_ir2_opnd2i(LISA_ST_W, &new_mxcsr, &latxs_env_ir2_opnd,
                      lsenv_offset_of_mxcsr(lsenv));

    /* latxs_tr_gen_call_to_helper1((ADDR)update_mxcsr_status, 1); */
    latxs_tr_gen_call_to_helper1_cfg((ADDR)update_mxcsr_status,
            all_helper_cfg);

    return true;
}

bool latxs_translate_fcomi(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fcomi_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    if (latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    bool is_zpc_def = ir1_is_zf_def(pir1) ||
                      ir1_is_pf_def(pir1) ||
                      ir1_is_cf_def(pir1);
    bool is_osa_def = ir1_is_of_def(pir1) ||
                      ir1_is_sf_def(pir1) ||
                      ir1_is_af_def(pir1);

    if (is_zpc_def || is_osa_def) {
        IR2_OPND eflags_temp = latxs_ra_alloc_itemp();

        /* calculate OF, SF and AF */
        if (is_osa_def || is_zpc_def) {
            latxs_append_ir2_opnd1i(LISA_X86MTFLAG,
                    &latxs_zero_ir2_opnd, 0x3f);
        }

        /* calculate ZF, PF, CF */
        if (is_zpc_def) {
            IR2_OPND st0 = latxs_ra_alloc_st(0);
            IR2_OPND sti = latxs_load_freg_from_ir1_1(
                    ir1_get_opnd(pir1, 0), true);

            latxs_append_ir2_opnd2i(LISA_ORI, &eflags_temp,
                    &latxs_zero_ir2_opnd, 0xfff);

            IR2_OPND label_for_exit = latxs_ir2_opnd_new_label();
            IR2_OPND label_for_not_unordered = latxs_ir2_opnd_new_label();
            IR2_OPND label_for_sti_cle_st0 = latxs_ir2_opnd_new_label();
            IR2_OPND label_for_sti_ceq_st0 = latxs_ir2_opnd_new_label();

            /* First: if unordered, set ZF/PF/CF and exit, */
            /* else jmp to label_for_not_unordered */
            latxs_append_ir2_opnd3i(LISA_FCMP_COND_D,
                    &latxs_fcc0_ir2_opnd, &sti, &st0, FCMP_COND_CUN);
            latxs_append_ir2_opnd2(LISA_BCEQZ,
                    &latxs_fcc0_ir2_opnd, &label_for_not_unordered);
            latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &eflags_temp, 0xb);
            latxs_append_ir2_opnd1(LISA_B, &label_for_exit);

            /* LABEL: label_for_not_unordered, */
            /* if(st0>=sti) then jmp to label_for_sti_cle_st0 */
            latxs_append_ir2_opnd1(LISA_LABEL, &label_for_not_unordered);
            latxs_append_ir2_opnd3i(LISA_FCMP_COND_D,
                    &latxs_fcc0_ir2_opnd, &sti, &st0, FCMP_COND_CLE);
            latxs_append_ir2_opnd2(LISA_BCNEZ,
                    &latxs_fcc0_ir2_opnd, &label_for_sti_cle_st0);

            /* else if (st0<st1), set CF and exit*/
            latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &eflags_temp, 0x1);
            latxs_append_ir2_opnd1(LISA_B, &label_for_exit);

            /* LABEL: label_for_sti_cle_st0 if(st0 == sti), */
            /* set fcc0 then jmp to label label_for_sti_ceq_st0*/
            latxs_append_ir2_opnd1(LISA_LABEL, &label_for_sti_cle_st0);
            latxs_append_ir2_opnd3i(LISA_FCMP_COND_D,
                    &latxs_fcc0_ir2_opnd, &sti, &st0, FCMP_COND_CEQ);
            latxs_append_ir2_opnd2(LISA_BCNEZ,
                    &latxs_fcc0_ir2_opnd, &label_for_sti_ceq_st0);

            /* else if (st0 > sti), ZF, PF and CF are all clear, */
            /* just exit directly */
            latxs_append_ir2_opnd1(LISA_B, &label_for_exit);

            /* LABEL: label_for_sti_ceq_st0, */
            /* set ZF if (st0 == sti) and exit*/
            latxs_append_ir2_opnd1(LISA_LABEL, &label_for_sti_ceq_st0);
            latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &eflags_temp, 0x8);

            latxs_append_ir2_opnd1(LISA_LABEL, &label_for_exit);
        }

        latxs_ra_free_temp(&eflags_temp);
    }

    return true;
}

bool latxs_translate_fucomi(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fucomi_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_translate_fcomi(pir1);
    return true;
}

bool latxs_translate_fucomip(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fucomip_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    latxs_translate_fcomi(pir1);
    latxs_tr_fpu_pop();
    return true;
}

bool latxs_translate_fcomip(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fcomip_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    latxs_translate_fcomi(pir1);
    latxs_tr_fpu_pop();
    return true;
}

bool latxs_translate_fcom(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fcom_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    IR2_OPND *zero = &latxs_zero_ir2_opnd;

#define C0_BIT (8)
#define C1_BIT (9)
#define C2_BIT (10)
#define C3_BIT (14)

    int opnd_num = ir1_get_opnd_num(pir1);

    IR2_OPND src;
    if (opnd_num == 0) {
        src = latxs_ra_alloc_st(1);
    } else {
        src = latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), true);
    }

    IR2_OPND st0 = latxs_ra_alloc_st(0);
    IR2_OPND sw_opnd = latxs_ra_alloc_itemp();

    IR2_OPND all_one = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ADDI_D, &all_one, zero, -1);

    /* load status_word */
    IR2_OPND mem_opnd;
    latxs_ir2_opnd_build_mem(&mem_opnd,
            latxs_ir2_opnd_reg(&latxs_env_ir2_opnd),
            lsenv_offset_of_status_word(lsenv));
    latxs_append_ir2_opnd2(LISA_LD_HU, &sw_opnd, &mem_opnd);

    /* clear status_word c0 c2 c3 */
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &sw_opnd,
                                             zero, C0_BIT, C0_BIT);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &sw_opnd,
                                             zero, C1_BIT, C1_BIT);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &sw_opnd,
                                             zero, C2_BIT, C2_BIT);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &sw_opnd,
                                             zero, C3_BIT, C3_BIT);

    IR2_OPND label_for_exit = latxs_ir2_opnd_new_label();
    IR2_OPND label_for_not_unordered = latxs_ir2_opnd_new_label();
    IR2_OPND label_for_src_cle_st0 = latxs_ir2_opnd_new_label();
    IR2_OPND label_for_src_ceq_st0 = latxs_ir2_opnd_new_label();

    /* First: if unordered, set C0/C2/C3 and exit, */
    /* else jmp to label_for_not_unordered */
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, &latxs_fcc0_ir2_opnd,
            &src, &st0, FCMP_COND_CUN);
    latxs_append_ir2_opnd2(LISA_BCEQZ, &latxs_fcc0_ir2_opnd,
            &label_for_not_unordered);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &sw_opnd,
                                             &all_one, C0_BIT, C0_BIT);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &sw_opnd,
                                             &all_one, C2_BIT, C2_BIT);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &sw_opnd,
                                             &all_one, C3_BIT, C3_BIT);
    latxs_append_ir2_opnd1(LISA_B, &label_for_exit);

    /* LABEL: label_for_not_unordered, */
    /* if(st0>=src) then jmp to label_for_src_cle_st0 */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_for_not_unordered);
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, &latxs_fcc0_ir2_opnd,
            &src, &st0, FCMP_COND_CLE);
    latxs_append_ir2_opnd2(LISA_BCNEZ, &latxs_fcc0_ir2_opnd,
            &label_for_src_cle_st0);

    /* else if (st0<st1), set CF and exit*/
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &sw_opnd,
                                             &all_one, C0_BIT, C0_BIT);
    latxs_append_ir2_opnd1(LISA_B, &label_for_exit);

    /* LABEL: label_for_src_cle_st0 if(st0 == src), */
    /* set fcc0 then jmp to label label_for_src_ceq_st0*/
    latxs_append_ir2_opnd1(LISA_LABEL, &label_for_src_cle_st0);
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, &latxs_fcc0_ir2_opnd,
                                              &src, &st0, FCMP_COND_CEQ);
    latxs_append_ir2_opnd2(LISA_BCNEZ, &latxs_fcc0_ir2_opnd,
                                       &label_for_src_ceq_st0);

    /* else if (st0 > sti), ZF, PF and CF are all clear, */
    /* just exit directly */
    latxs_append_ir2_opnd1(LISA_B, &label_for_exit);

    /* LABEL: label_for_src_ceq_st0, set ZF if (st0 == src) and exit*/
    latxs_append_ir2_opnd1(LISA_LABEL, &label_for_src_ceq_st0);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &sw_opnd,
                                             &all_one, C3_BIT, C3_BIT);
    latxs_ra_free_temp(&all_one);

    latxs_append_ir2_opnd1(LISA_LABEL, &label_for_exit);

    latxs_append_ir2_opnd2(LISA_ST_H, &sw_opnd, &mem_opnd);
    latxs_ra_free_temp(&sw_opnd);

    return true;
}

bool latxs_translate_fcomp(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fcomp_softfpu(pir1);
    }
    latxs_translate_fcom(pir1);
    latxs_tr_fpu_pop();
    return true;
}

bool latxs_translate_fcompp(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fcompp_softfpu(pir1);
    }
    latxs_translate_fcom(pir1);
    latxs_tr_fpu_pop();
    latxs_tr_fpu_pop();
    return true;
}

bool latxs_translate_fucom(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fucom_softfpu(pir1);
    }
    latxs_translate_fcom(pir1);
    return true;
}

bool latxs_translate_fucomp(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fucomp_softfpu(pir1);
    }
    latxs_translate_fcom(pir1);
    latxs_tr_fpu_pop();
    return true;
}

bool latxs_translate_fucompp(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fucompp_softfpu(pir1);
    }
    latxs_translate_fcom(pir1);
    latxs_tr_fpu_pop();
    latxs_tr_fpu_pop();
    return true;
}

bool latxs_translate_ficom(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_ficom_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }
    if (latxs_tr_gen_excp_check(pir1)) {
        return true;
    }

    lsassertm(0, "ficom to be implemented in LoongArch.\n");

    return true;
}

bool latxs_translate_ficomp(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_ficomp_softfpu(pir1);
    }
    translate_ficom(pir1);
    latxs_tr_fpu_pop();
    return true;
}

bool latxs_translate_fnclex(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fnclex_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    /* env->fpus &= 0x7f00 */

    IR2_OPND sw_opnd = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_HU, &sw_opnd,
            &latxs_env_ir2_opnd,
            lsenv_offset_of_status_word(lsenv));

    IR2_OPND mask = latxs_ra_alloc_itemp();
    latxs_load_imm32_to_ir2(&mask, 0x7f00, EXMode_Z);

    latxs_append_ir2_opnd3(LISA_AND, &sw_opnd, &sw_opnd, &mask);

    latxs_append_ir2_opnd2i(LISA_ST_H, &sw_opnd,
            &latxs_env_ir2_opnd,
            lsenv_offset_of_status_word(lsenv));

    latxs_ra_free_temp(&sw_opnd);
    latxs_ra_free_temp(&mask);

    return true;
}

bool latxs_translate_emms(IR1_INST *pir1)
{
    /*
     *  QEMU implements emms by helper func, which is simple,
     *  so I choose to implements it by mips assembly.
     *  void helper_emms(CPUX86State *env)
     *  {
     *      //set to empty state
     *      *(uint32_t *)(env->fptags) = 0x01010101;
     *      *(uint32_t *)(env->fptags + 4) = 0x01010101;
     *  }
     */

    /* uint8_t fptags[8]; */
    /* 0 = valid, 1 = empty */
    IR2_OPND empty = latxs_ra_alloc_itemp();

    /* empty = 0x0101 0101 */
    latxs_load_imm32_to_ir2(&empty, 0x01010101, UNKNOWN_EXTENSION);
    /* 64bit 0x0101010101010101 ok       */
    /* store [empty, empty] to fptags[8] */
    latxs_append_ir2_opnd2i(LISA_ST_W, &empty, &latxs_env_ir2_opnd,
                                lsenv_offset_of_tag_word(lsenv));
    latxs_append_ir2_opnd2i(LISA_ST_W, &empty, &latxs_env_ir2_opnd,
                                lsenv_offset_of_tag_word(lsenv) + 4);
    latxs_ra_free_temp(&empty);
    return true;
}
