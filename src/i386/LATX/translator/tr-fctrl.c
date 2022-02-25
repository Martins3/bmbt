#include "common.h"
#include "reg-alloc.h"
#include "lsenv.h"
#include "latx-options.h"
#include "translate.h"

static void update_fcsr_flag(IR2_OPND status_word, IR2_OPND fcsr)
{
    IR2_OPND temp = ra_alloc_itemp();

    /* reset flags*/
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, fcsr, zero_ir2_opnd,
                            FCSR_OFF_FLAGS_V, FCSR_OFF_FLAGS_I);
    /* IE */
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, temp, status_word,
                            X87_SR_OFF_IE, X87_SR_OFF_IE);
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, fcsr, temp,
                            FCSR_OFF_FLAGS_V, FCSR_OFF_FLAGS_V);
    /* ZE */
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, temp, status_word,
                            X87_SR_OFF_ZE, X87_SR_OFF_ZE);
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, fcsr, temp,
                            FCSR_OFF_FLAGS_Z, FCSR_OFF_FLAGS_Z);
    /* OE */
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, temp, status_word,
                            X87_SR_OFF_OE, X87_SR_OFF_OE);
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, fcsr, temp,
                            FCSR_OFF_FLAGS_O, FCSR_OFF_FLAGS_O);
    /* UE */
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, temp, status_word,
                            X87_SR_OFF_UE, X87_SR_OFF_UE);
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, fcsr, temp,
                            FCSR_OFF_FLAGS_U, FCSR_OFF_FLAGS_U);
    /* PE */
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, temp, status_word,
                            X87_SR_OFF_PE, X87_SR_OFF_PE);
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, fcsr, temp,
                            FCSR_OFF_FLAGS_I, FCSR_OFF_FLAGS_I);
    la_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr_ir2_opnd, fcsr);

    ra_free_temp(temp);
}

static void update_fcsr_enable(IR2_OPND control_word, IR2_OPND fcsr)
{
    IR2_OPND temp = ra_alloc_itemp();

    /* reset enables */
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, fcsr, zero_ir2_opnd,
                            FCSR_OFF_EN_V, FCSR_OFF_EN_I);
    /* IM */
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, temp, control_word,
                            X87_CR_OFF_IM, X87_CR_OFF_IM);
    la_append_ir2_opnd2i_em(LISA_XORI, temp, temp, 1);
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, fcsr, temp,
                            FCSR_OFF_EN_V, FCSR_OFF_EN_V);
    /* ZM */
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, temp, control_word,
                            X87_CR_OFF_ZM, X87_CR_OFF_ZM);
    la_append_ir2_opnd2i_em(LISA_XORI, temp, temp, 1);
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, fcsr, temp,
                            FCSR_OFF_EN_Z, FCSR_OFF_EN_Z);
    /* OM */
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, temp, control_word,
                            X87_CR_OFF_OM, X87_CR_OFF_OM);
    la_append_ir2_opnd2i_em(LISA_XORI, temp, temp, 1);
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, fcsr, temp,
                            FCSR_OFF_EN_O, FCSR_OFF_EN_O);
    /* UM */
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, temp, control_word,
                            X87_CR_OFF_UM, X87_CR_OFF_UM);
    la_append_ir2_opnd2i_em(LISA_XORI, temp, temp, 1);
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, fcsr, temp,
                            FCSR_OFF_EN_U, FCSR_OFF_EN_U);
    /* PM */
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, temp, control_word,
                            X87_CR_OFF_PM, X87_CR_OFF_PM);
    la_append_ir2_opnd2i_em(LISA_XORI, temp, temp, 1);
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, fcsr, temp,
                            FCSR_OFF_EN_I, FCSR_OFF_EN_I);
    la_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr_ir2_opnd, fcsr);

    ra_free_temp(temp);
}

static void update_fcsr_rm(IR2_OPND control_word, IR2_OPND fcsr)
{
    /* reset rounding mode */
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, fcsr, zero_ir2_opnd,
                            FCSR_OFF_RM + 1, FCSR_OFF_RM);
    /* load rounding mode in x86 control register */
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, control_word, control_word,
                            X87_CR_OFF_RC + 1, X87_CR_OFF_RC);
    /*
     *turn x86 rm to LA rm
     *          x86      LA
     *
     * RN       00       00
     * RD       01       11
     * RU       10       10
     * RZ       11       01
     *
     */
    IR2_OPND temp_cw = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ANDI, temp_cw, control_word, 1);
    IR2_OPND label = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, temp_cw, zero_ir2_opnd, label);
    la_append_ir2_opnd2i_em(LISA_XORI, control_word, control_word, 2);
    la_append_ir2_opnd1(LISA_LABEL, label);
    /* set rounding mode in LA FCSR */
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, fcsr, control_word,
                            FCSR_OFF_RM + 1, FCSR_OFF_RM);
    la_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr_ir2_opnd, fcsr);

    ra_free_temp(temp_cw);
}

void update_fcsr_by_sw(IR2_OPND sw)
{
    IR2_OPND old_fcsr = ra_alloc_itemp();
    la_append_ir2_opnd2_em(LISA_MOVFCSR2GR, old_fcsr, fcsr_ir2_opnd);
    update_fcsr_flag(sw, old_fcsr);
}

void update_fcsr_by_cw(IR2_OPND cw)
{
    IR2_OPND old_fcsr = ra_alloc_itemp();
    la_append_ir2_opnd2_em(LISA_MOVFCSR2GR, old_fcsr, fcsr_ir2_opnd);
    update_fcsr_enable(cw, old_fcsr);
    update_fcsr_rm(cw, old_fcsr);
}

void update_sw_by_fcsr(IR2_OPND sw_opnd)
{
    int status_offset = lsenv_offset_of_status_word(lsenv);
    lsassert(status_offset <= 0x7ff);
    la_append_ir2_opnd2i_em(LISA_LD_H, sw_opnd, env_ir2_opnd, status_offset);

    IR2_OPND fcsr = ra_alloc_itemp();
    IR2_OPND temp1 = ra_alloc_itemp();
    la_append_ir2_opnd2_em(LISA_MOVFCSR2GR, fcsr, fcsr_ir2_opnd);

    /* update float exceptions from fcsr */
    /*  invalid-arithmetic-operand exception */
    IR2_OPND label_I = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, temp1, fcsr, FCSR_OFF_FLAGS_V, FCSR_OFF_FLAGS_V);
    la_append_ir2_opnd3(LISA_BEQ, temp1, zero_ir2_opnd, label_I);
    la_append_ir2_opnd2i_em(LISA_ORI, sw_opnd, sw_opnd, 0x1 << X87_SR_OFF_IE);
    la_append_ir2_opnd1(LISA_LABEL, label_I);
    /* Divide-By-Zero Exception */
    IR2_OPND label_Z = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, temp1, fcsr, FCSR_OFF_FLAGS_Z, FCSR_OFF_FLAGS_Z);
    la_append_ir2_opnd3(LISA_BEQ, temp1, zero_ir2_opnd, label_Z);
    la_append_ir2_opnd2i_em(LISA_ORI, sw_opnd, sw_opnd, 0x1 << X87_SR_OFF_ZE);
    la_append_ir2_opnd1(LISA_LABEL, label_Z);
    /* Numeric Overflow Exception */
    IR2_OPND label_O = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, temp1, fcsr, FCSR_OFF_FLAGS_O, FCSR_OFF_FLAGS_O);
    la_append_ir2_opnd3(LISA_BEQ, temp1, zero_ir2_opnd, label_O);
    la_append_ir2_opnd2i_em(LISA_ORI, sw_opnd, sw_opnd, 0x1 << X87_SR_OFF_OE);
    la_append_ir2_opnd1(LISA_LABEL, label_O);
    /* Numeric Underflow Exception */
    IR2_OPND label_U = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, temp1, fcsr, FCSR_OFF_FLAGS_U, FCSR_OFF_FLAGS_U);
    la_append_ir2_opnd3(LISA_BEQ, temp1, zero_ir2_opnd, label_U);
    la_append_ir2_opnd2i_em(LISA_ORI, sw_opnd, sw_opnd, 0x1 << X87_SR_OFF_UE);
    la_append_ir2_opnd1(LISA_LABEL, label_U);
    /* Inexact-Result (Precision) Exception */
    IR2_OPND label_P = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, temp1, fcsr, FCSR_OFF_FLAGS_I, FCSR_OFF_FLAGS_I);
    la_append_ir2_opnd3(LISA_BEQ, temp1, zero_ir2_opnd, label_P);
    la_append_ir2_opnd2i_em(LISA_ORI, sw_opnd, sw_opnd, 0x1 << X87_SR_OFF_PE);
    la_append_ir2_opnd1(LISA_LABEL, label_P);

    /* update top */
    if (option_lsfpu) {
        la_append_ir2_opnd1(LISA_X86MFTOP, temp1);
        ir2_opnd_set_em(&temp1, ZERO_EXTENSION, 32);

        la_append_ir2_opnd2i_em(LISA_SLLI_W, temp1, temp1, 11);


        IR2_OPND tmp_opnd = ra_alloc_itemp();
        la_append_ir2_opnd1i_em(LISA_LU12I_W, tmp_opnd, 0xc);
        la_append_ir2_opnd2i_em(LISA_ORI, tmp_opnd, tmp_opnd, 0x7ff);
        la_append_ir2_opnd3_em(LISA_AND, sw_opnd, sw_opnd, tmp_opnd);

        la_append_ir2_opnd3_em(LISA_OR, sw_opnd, sw_opnd, temp1);

    }

    /* clean exception flags in fcsr */
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, fcsr, zero_ir2_opnd, FCSR_OFF_FLAGS_V, FCSR_OFF_FLAGS_I);
    la_append_ir2_opnd2_em(LISA_MOVGR2FCSR, fcsr_ir2_opnd, fcsr);

    la_append_ir2_opnd2i(LISA_ST_H, sw_opnd, env_ir2_opnd, status_offset);
}

bool translate_fnstcw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fnstcw(pir1);
#else
    /* 1. load the value of fpu control word */
    IR2_OPND cw_opnd = ra_alloc_itemp();

    int offset = lsenv_offset_of_control_word(lsenv);
    lsassert(offset <= 0x7ff);
    la_append_ir2_opnd2i_em(LISA_LD_H, cw_opnd, env_ir2_opnd, offset);

    /* 2. store the control word to the dest_opnd */
    store_ireg_to_ir1(cw_opnd, ir1_get_opnd(pir1, 0), false);

    ra_free_temp(cw_opnd);
    return true;
#endif
}

bool translate_fstcw(IR1_INST *pir1) { return translate_fnstcw(pir1); }

bool translate_fldcw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fldcw(pir1);
#else

    /* 1. load new control word from the source opnd(mem) */
    IR2_OPND new_cw = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

    /* 2. store the value into the env->fpu_control_word(reg to reg)*/
    la_append_ir2_opnd2i(LISA_ST_H, new_cw, env_ir2_opnd, lsenv_offset_of_control_word(lsenv));

    update_fcsr_by_cw(new_cw);
    //tr_gen_call_to_helper1((ADDR)update_fp_status, 1);

    return true;
#endif
}

bool translate_stmxcsr(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_stmxcsr(pir1);
#else
    /* 1. load the value of the mxcsr register state from env */
    IR2_OPND mxcsr_opnd = ra_alloc_itemp();
    int offset = lsenv_offset_of_mxcsr(lsenv);

    lsassert(offset <= 0x7ff);
    la_append_ir2_opnd2i_em(LISA_LD_WU, mxcsr_opnd, env_ir2_opnd, offset);

    /* 2. store  the value of the mxcsr register state to the dest_opnd */
    store_ireg_to_ir1(mxcsr_opnd, ir1_get_opnd(pir1, 0), false);

    ra_free_temp(mxcsr_opnd);
    return true;
#endif
}

bool translate_ldmxcsr(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_ldmxcsr(pir1);
#else
    /* 1. load new mxcsr value from the source opnd */
    IR2_OPND new_mxcsr =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    int offset = lsenv_offset_of_mxcsr(lsenv);

    /* 2. store the value into the env->mxcsr */
    lsassert(offset <= 0x7ff);
    la_append_ir2_opnd2i(LISA_ST_W, new_mxcsr, env_ir2_opnd, offset);

    tr_gen_call_to_helper1((ADDR)update_mxcsr_status, 1);

    return true;
#endif
}

#ifndef CONFIG_SOFTMMU
/* FIXME: for now JUST use 3 (bcnez + b) to implement this inst,
 * maybe it will be optimized later.
 */
static bool translate_fcomi_internal(IR1_INST *pir1, bool unordered)
{
    bool is_zpc_def =
        ir1_is_zf_def(pir1) || ir1_is_pf_def(pir1) || ir1_is_cf_def(pir1);
    bool is_osa_def =
        ir1_is_of_def(pir1) || ir1_is_sf_def(pir1) || ir1_is_af_def(pir1);
    if (is_zpc_def || is_osa_def) {
        /* calculate OF, SF and AF */
        IR2_OPND eflags_temp;
        if (is_osa_def || is_zpc_def) {
            eflags_temp = ra_alloc_itemp();
            la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x3f);
        }

        /* calculate ZF, PF, CF */
        if (is_zpc_def) {
            IR2_OPND st0 = ra_alloc_st(0);
            IR2_OPND sti = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);

            la_append_ir2_opnd2i_em(LISA_ORI, eflags_temp, zero_ir2_opnd, 0xfff);

            IR2_OPND label_for_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
            IR2_OPND label_for_not_unordered = ir2_opnd_new_type(IR2_OPND_LABEL);
            IR2_OPND label_for_sti_cle_st0 = ir2_opnd_new_type(IR2_OPND_LABEL);
            IR2_OPND label_for_sti_ceq_st0 = ir2_opnd_new_type(IR2_OPND_LABEL);

            /* First: if unordered, set ZF/PF/CF and exit, else jmp to label_for_not_unordered */
            if(unordered)
                la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, sti, st0, FCMP_COND_CUN);
            else
                la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, sti, st0, FCMP_COND_SUN);
            la_append_ir2_opnd2(LISA_BCEQZ, fcc0_ir2_opnd, label_for_not_unordered);
            la_append_ir2_opnd1i(LISA_X86MTFLAG, eflags_temp, 0xb);
            la_append_ir2_opnd1(LISA_B, label_for_exit);

            /* LABEL: label_for_not_unordered, if(st0>=sti) then jmp to label_for_sti_cle_st0 */
            la_append_ir2_opnd1(LISA_LABEL, label_for_not_unordered);
            la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, sti, st0, FCMP_COND_CLE);
            la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_sti_cle_st0);

            /* else if (st0<st1), set CF and exit*/
            la_append_ir2_opnd1i(LISA_X86MTFLAG, eflags_temp, 0x1);
            la_append_ir2_opnd1(LISA_B, label_for_exit);

            /* LABEL: label_for_sti_cle_st0 if(st0 == sti), set fcc0 then jmp to label label_for_sti_ceq_st0*/
            la_append_ir2_opnd1(LISA_LABEL, label_for_sti_cle_st0);
            la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, sti, st0, FCMP_COND_CEQ);
            la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_sti_ceq_st0);

            /* else if (st0 > sti), ZF, PF and CF are all clear, just exit directly */
            la_append_ir2_opnd1(LISA_B, label_for_exit);

            /* LABEL: label_for_sti_ceq_st0, set ZF if (st0 == sti) and exit*/
            la_append_ir2_opnd1(LISA_LABEL, label_for_sti_ceq_st0);
            la_append_ir2_opnd1i(LISA_X86MTFLAG, eflags_temp, 0x8);

            la_append_ir2_opnd1(LISA_LABEL, label_for_exit);
            ra_free_temp(eflags_temp);
        }
    }
    return true;
}
#endif

bool translate_fucomi(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fucomi(pir1);
#else
    translate_fcomi_internal(pir1, true);
    return true;
#endif
}

bool translate_fucomip(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fucomip(pir1);
#else
    translate_fcomi_internal(pir1, true);
    tr_fpu_pop();
    return true;
#endif
}

bool translate_fcomi(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fcomi(pir1);
#else
    translate_fcomi_internal(pir1, false);
    return true;
#endif
}

bool translate_fcomip(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fcomip(pir1);
#else
    translate_fcomi_internal(pir1, false);
    tr_fpu_pop();

    return true;
#endif
}

#ifndef CONFIG_SOFTMMU
static bool translate_fcom_internal(IR1_INST *pir1, bool unordered)
{
#define C0_BIT (8)
#define C1_BIT (9)
#define C2_BIT (10)
#define C3_BIT (14)
    // int opnd_num = pir1->opnd_num;
    int opnd_num = ir1_get_opnd_num(pir1);

    IR2_OPND src;
    if (opnd_num == 0)
        src = ra_alloc_st(1);
    else
        src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
    IR2_OPND st0 = ra_alloc_st(0);
    IR2_OPND sw_opnd = ra_alloc_itemp();

    /* load status_word */
    int offset = lsenv_offset_of_status_word(lsenv);
    lsassert(offset <= 0x7ff);
    la_append_ir2_opnd2i_em(LISA_LD_H, sw_opnd, env_ir2_opnd, offset);

    /* clear status_word c0 c1 c2 c3 */
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, sw_opnd, zero_ir2_opnd, C0_BIT, C0_BIT);
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, sw_opnd, zero_ir2_opnd, C1_BIT, C1_BIT);
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, sw_opnd, zero_ir2_opnd, C2_BIT, C2_BIT);
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, sw_opnd, zero_ir2_opnd, C3_BIT, C3_BIT);

    IR2_OPND label_for_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_for_not_unordered = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_for_src_cle_st0 = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_for_src_ceq_st0 = ir2_opnd_new_type(IR2_OPND_LABEL);

    /* First: if unordered, set C0/C2/C3 and exit, else jmp to label_for_not_unordered */
    if(unordered)
        la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, src, st0, FCMP_COND_CUN);
    else
        la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, src, st0, FCMP_COND_SUN);
    la_append_ir2_opnd2(LISA_BCEQZ, fcc0_ir2_opnd, label_for_not_unordered);
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, sw_opnd, n1_ir2_opnd, C0_BIT, C0_BIT);
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, sw_opnd, n1_ir2_opnd, C2_BIT, C2_BIT);
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, sw_opnd, n1_ir2_opnd, C3_BIT, C3_BIT);
    la_append_ir2_opnd1(LISA_B, label_for_exit);

    /* LABEL: label_for_not_unordered, if(st0>=src) then jmp to label_for_src_cle_st0 */
    la_append_ir2_opnd1(LISA_LABEL, label_for_not_unordered);
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, src, st0, FCMP_COND_CLE);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_src_cle_st0);

    /* else if (st0<st1), set CF and exit*/
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, sw_opnd, n1_ir2_opnd, C0_BIT, C0_BIT);
    la_append_ir2_opnd1(LISA_B, label_for_exit);

    /* LABEL: label_for_src_cle_st0 if(st0 == src), set fcc0 then jmp to label label_for_src_ceq_st0*/
    la_append_ir2_opnd1(LISA_LABEL, label_for_src_cle_st0);
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, src, st0, FCMP_COND_CEQ);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_src_ceq_st0);

    /* else if (st0 > sti), ZF, PF and CF are all clear, just exit directly */
    la_append_ir2_opnd1(LISA_B, label_for_exit);

    /* LABEL: label_for_src_ceq_st0, set ZF if (st0 == src) and exit*/
    la_append_ir2_opnd1(LISA_LABEL, label_for_src_ceq_st0);
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, sw_opnd, n1_ir2_opnd, C3_BIT, C3_BIT);

    la_append_ir2_opnd1(LISA_LABEL, label_for_exit);

    la_append_ir2_opnd2i(LISA_ST_H, sw_opnd, env_ir2_opnd, offset);
    ra_free_temp(sw_opnd);

    return true;
}
#endif

bool translate_fcom(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fcom(pir1);
#else
    translate_fcom_internal(pir1, false);

    return true;
#endif
}

bool translate_fcomp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fcomp(pir1);
#else
    translate_fcom_internal(pir1, false);
    tr_fpu_pop();

    return true;
#endif
}

bool translate_fucom(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fucom(pir1);
#else
    translate_fcom_internal(pir1, true);
    return true;
#endif
}

bool translate_fucomp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fucomp(pir1);
#else
    translate_fcom_internal(pir1, true);
    tr_fpu_pop();

    return true;
#endif
}

bool translate_fcompp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fcompp(pir1);
#else
    translate_fcom_internal(pir1, false);
    tr_fpu_pop();
    tr_fpu_pop();

    return true;
#endif
}

bool translate_fucompp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fucompp(pir1);
#else
    translate_fcom_internal(pir1, true);
    tr_fpu_pop();
    tr_fpu_pop();

    return true;
#endif
}

bool translate_ficom(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_ficom(pir1);
#else
    assert(0 && "translate_ficom need to implemented correctly");
    IR2_OPND st0_opnd = ra_alloc_st(0);
    IR2_OPND mint_opnd = ra_alloc_ftemp();
    IR2_OPND tmp_opnd = ra_alloc_itemp();
    load_freg_from_ir1_2(mint_opnd, ir1_get_opnd(pir1, 0), 0);
    la_append_ir2_opnd2(LISA_FFINT_D_L, mint_opnd, mint_opnd);
    IR2_OPND sw_opnd = ra_alloc_itemp();

    /* load status_word */
    int offset = lsenv_offset_of_status_word(lsenv);

    lsassert(offset <= 0x7ff);
    la_append_ir2_opnd2i_em(LISA_LD_HU, sw_opnd, env_ir2_opnd, offset);

    /* clear status_word c0 c2 c3 */

    la_append_ir2_opnd1i_em(LISA_LU12I_W, tmp_opnd, 0xb);
    la_append_ir2_opnd2i_em(LISA_ORI, tmp_opnd, tmp_opnd, 0xaff);
    la_append_ir2_opnd3_em(LISA_AND, sw_opnd, sw_opnd, tmp_opnd);

    IR2_OPND label_un = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_eq = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_lt = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);

    /* check is unordered */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, st0_opnd, mint_opnd, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_un);
    /* check is equal */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, st0_opnd, mint_opnd, FCMP_COND_CEQ);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_eq);
    /* check is less than */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, st0_opnd, mint_opnd, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_lt);

    la_append_ir2_opnd1(LISA_B, label_exit);
    /* lt: */
    la_append_ir2_opnd1(LISA_LABEL, label_lt);
    la_append_ir2_opnd2i_em(LISA_ORI, sw_opnd, sw_opnd, 0x100);
    la_append_ir2_opnd1(LISA_B, label_exit);
    /* eq: */
    la_append_ir2_opnd1(LISA_LABEL, label_eq);

    la_append_ir2_opnd1i(LISA_LU12I_W, tmp_opnd, 0x4);
    la_append_ir2_opnd3_em(LISA_OR, sw_opnd, sw_opnd, tmp_opnd);

    la_append_ir2_opnd1(LISA_B, label_exit);
    /* un: */
    la_append_ir2_opnd1(LISA_LABEL, label_un);

    la_append_ir2_opnd1i(LISA_LU12I_W, tmp_opnd, 0x4);
    la_append_ir2_opnd2i_em(LISA_ORI, tmp_opnd, tmp_opnd, 0x500);
    la_append_ir2_opnd3_em(LISA_OR, sw_opnd, sw_opnd, tmp_opnd);

    /* exit: */
    la_append_ir2_opnd1(LISA_LABEL, label_exit);
    /* append_ir2_opnd2i(mips_sh, sw_opnd, mem_opnd); */
    ra_free_temp(sw_opnd);
    ra_free_temp(mint_opnd);
    ra_free_temp(tmp_opnd);

    return true;
#endif
}

bool translate_ficomp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_ficomp(pir1);
#else
    translate_ficom(pir1);
    tr_fpu_pop();
    return true;
#endif
}

/* TODO: */
bool translate_ud2(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_ud2(pir1);
#else
    /* lsassert(lsenv->tr_data->static_translation); */
    return true;
#endif
}

bool translate_ffree(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_ffree(pir1);
#else
    /* 根据 status_word 的 TOP bits 处理 tag word: mark the ST(i) register as */
    /* empty */
    /* TODO:r */
    IR2_OPND value_tag = ra_alloc_itemp();
    IR2_OPND value_status = ra_alloc_itemp();

    int tag_offset = lsenv_offset_of_tag_word(lsenv);
    lsassert(tag_offset <= 0x7ff);
    la_append_ir2_opnd2i_em(LISA_LD_H, value_tag, env_ir2_opnd, tag_offset);

    int status_offset = lsenv_offset_of_status_word(lsenv);
    lsassert(status_offset <= 0x7ff);
    la_append_ir2_opnd2i_em(LISA_LD_H, value_status, env_ir2_opnd, status_offset);

    la_append_ir2_opnd2i_em(LISA_SRAI_W, value_status, value_status, 11);
    la_append_ir2_opnd2i_em(LISA_ANDI, value_status, value_status, 0x7ULL);
    la_append_ir2_opnd2i_em(LISA_SLLI_W, value_status, value_status, 1);
    IR2_OPND temp = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_XOR, temp, temp, temp);
    la_append_ir2_opnd2i_em(LISA_ORI, temp, temp, 0x3ULL);
    la_append_ir2_opnd3_em(LISA_SLL_W, temp, temp, value_status);
    la_append_ir2_opnd3_em(LISA_OR, value_tag, value_tag, temp);
    la_append_ir2_opnd2i(LISA_ST_H, value_tag, env_ir2_opnd, tag_offset);

    return true;
#endif
}

bool translate_fldenv(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fldenv(pir1);
#else
    IR2_OPND value = ra_alloc_itemp();

    /* mem_opnd is not supported in ir2 assemble */
    /* convert mem_opnd to ireg_opnd */

    IR1_OPND* opnd1 = ir1_get_opnd(pir1, 0);
    IR2_OPND mem_opnd = convert_mem_opnd_with_no_offset(opnd1);
    int mem_imm = ir2_opnd_imm(&mem_opnd);
    mem_opnd._type = IR2_OPND_IREG;

    if (cpu_get_guest_base() != 0) {
        IR2_OPND gbase = ra_alloc_guest_base();
        IR2_OPND tmp = ra_alloc_itemp();
        IR2_OPND mem = ir2_opnd_new(IR2_OPND_IREG, mem_opnd._reg_num);
#ifdef N64
        la_append_ir2_opnd3_em(LISA_ADD_D, tmp, mem, gbase);
#else
        la_append_ir2_opnd3_em(LISA_ADD_W, tmp, mem, gbase);
#endif
        mem_opnd._reg_num = ir2_opnd_base_reg_num(&tmp);
        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
    }

    /* store float control register at env memory */
    la_append_ir2_opnd2i_em(LISA_LD_H,value,mem_opnd,mem_imm);
    int control_offset = lsenv_offset_of_control_word(lsenv);
    lsassert(control_offset <= 0x7ff);
    la_append_ir2_opnd2i(LISA_ST_H, value, env_ir2_opnd, control_offset);
    update_fcsr_by_cw(value);

    /* store float status register at env memory */
    assert(mem_opnd._imm16 + 24 <= 2047);
    la_append_ir2_opnd2i_em(LISA_LD_H, value, mem_opnd, mem_imm + 4);
    int status_offset = lsenv_offset_of_status_word(lsenv);
    lsassert(status_offset <= 0x7ff);
    la_append_ir2_opnd2i(LISA_ST_H, value, env_ir2_opnd, status_offset);
    update_fcsr_by_sw(value);

    /* get top value */
    la_append_ir2_opnd2i_em(LISA_SRLI_W, value, value, 11);
    la_append_ir2_opnd2i_em(LISA_ANDI, value, value, 7);

    /* set fpstt */
    int top_offset = lsenv_offset_of_top(lsenv);
    lsassert(top_offset <= 0x7ff);
    la_append_ir2_opnd2i(LISA_ST_H, value, env_ir2_opnd, top_offset);
    tr_load_top_from_env(); 

    //tr_gen_call_to_helper1((ADDR)update_fp_status, 1);

    la_append_ir2_opnd2i_em(LISA_LD_H, value, mem_opnd,mem_imm + 8);
    /* dispose tag word */
    IR2_OPND temp_1 = ra_alloc_itemp();
    la_append_ir2_opnd2_em(LISA_MOV32_SX, temp_1, value);
    la_append_ir2_opnd2i_em(LISA_SLLI_W, temp_1, temp_1, 1);
    la_append_ir2_opnd3_em(LISA_AND, value, value, temp_1);

    IR2_OPND tmp_opnd = ra_alloc_itemp();
    la_append_ir2_opnd1i_em(LISA_LU12I_W, tmp_opnd, 0x5);
    la_append_ir2_opnd2i_em(LISA_ORI, tmp_opnd, tmp_opnd, 0x555);
    la_append_ir2_opnd3_em(LISA_OR, value, value, tmp_opnd);


    int tag_offset = lsenv_offset_of_tag_word(lsenv);
    lsassert(tag_offset <= 0x7ff);
    la_append_ir2_opnd2i(LISA_ST_H, value, env_ir2_opnd, tag_offset);

    /* append_ir2_opnd2(mips_lw, value, ir2_opnd_new2(IR2_OPND_MEM, */
    /* mem_opnd->_reg_num, mem_opnd->_imm16+12)); append_ir2_opnd2i(mips_sw, */
    /* value, env_ir2_opnd, lsenv_offset_of_instruction_pointer_offset(lsenv));
     */
    /* // */
    /* */
    /* append_ir2_opnd2(mips_lw, value, ir2_opnd_new2(IR2_OPND_MEM, */
    /* mem_opnd->_reg_num, mem_opnd->_imm16+16)); append_ir2_opnd2i(mips_sw, */
    /* value, env_ir2_opnd, lsenv_offset_of_selector_opcode(lsenv)); // */
    ra_free_temp(value);
    return true;
#endif
}

bool translate_fnstenv(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fnstenv(pir1);
#else
    IR2_OPND value = ra_alloc_itemp();
    IR2_OPND temp = ra_alloc_itemp();
    load_ireg_from_imm32(temp, 0xffff0000ULL, UNKNOWN_EXTENSION);

    /* mem_opnd is not supported in ir2 assemble */
    /* convert mem_opnd to ireg_opnd */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 0);
    IR2_OPND mem_opnd = convert_mem_opnd_with_no_offset(opnd1);
    int mem_imm = ir2_opnd_imm(&mem_opnd);
    mem_opnd._type = IR2_OPND_IREG;

    if (cpu_get_guest_base() != 0) {
        IR2_OPND gbase = ra_alloc_guest_base();
        IR2_OPND tmp = ra_alloc_itemp();
        IR2_OPND mem = ir2_opnd_new(IR2_OPND_IREG, mem_opnd._reg_num);
#ifdef N64
        la_append_ir2_opnd3_em(LISA_ADD_D, tmp, mem, gbase);
#else
        la_append_ir2_opnd3_em(LISA_ADD_W, tmp, mem, gbase);
#endif
        mem_opnd._reg_num = ir2_opnd_base_reg_num(&tmp);
        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
    }

    int control_offset = lsenv_offset_of_control_word(lsenv);
    lsassert(control_offset <= 0x7ff);
    la_append_ir2_opnd2i_em(LISA_LD_H, value, env_ir2_opnd, control_offset);

    la_append_ir2_opnd3_em(LISA_OR, value, temp, value);
    la_append_ir2_opnd2i(LISA_ST_W, value, mem_opnd, mem_imm);

    /* Mask all floating-point exceptions */
    la_append_ir2_opnd2i_em(LISA_ORI, value, value, X87_CR_EXCP_MASK);
    la_append_ir2_opnd2i(LISA_ST_H, value, env_ir2_opnd, control_offset);
    /* Disable FCSR VZOUI accordingly */
    la_append_ir2_opnd2_em(LISA_MOVFCSR2GR, value , fcsr_ir2_opnd);
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, value, zero_ir2_opnd,
                            FCSR_OFF_EN_V, FCSR_OFF_EN_I);
    la_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr_ir2_opnd, value);


    assert(mem_opnd._imm16 + 24 <= 2047);

    update_sw_by_fcsr(value);
    la_append_ir2_opnd3_em(LISA_OR, value, temp, value);
    la_append_ir2_opnd2i( LISA_ST_W, value, mem_opnd,mem_imm + 4);

    int tag_offset = lsenv_offset_of_tag_word(lsenv);
    lsassert(tag_offset <= 0x7ff);
    la_append_ir2_opnd2i_em(LISA_LD_H, value, env_ir2_opnd, tag_offset);

    la_append_ir2_opnd3_em(LISA_OR, value, temp, value);

    la_append_ir2_opnd2i(
        LISA_ST_W, value,
	mem_opnd,mem_imm + 8);

    /* append_ir2_opnd2i(mips_lw, value, env_ir2_opnd, */
    /* lsenv_offset_of_instruction_pointer_offset()); // append_ir2(mips_sw, */
    /* value, ir2_opnd_new2(IR2_OPND_MEM, mem_opnd->_reg_num, */
    /* mem_opnd->_imm16+12)); */
    /* */
    /* append_ir2(mips_lw, value, env_ir2_opnd, */
    /* env->offset_of_selector_opcode()); // append_ir2(mips_sw, value, */
    /* ir2_opnd_new2(IR2_OPND_MEM, mem_opnd->_reg_num, mem_opnd->_imm16+16)); */

    la_append_ir2_opnd2i(
        LISA_ST_W, temp,
	mem_opnd, mem_imm + 24);

    ra_free_temp(value);
    return true;
#endif
}

static float float_max = FLT_MAX;
static float float_min = FLT_MIN;
static float float_1e32 = 1.0e32f;
static float float_2 = 2.0f;
static float float_3 = 3.0f;

bool translate_wait(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    (void)float_max; /* to avoid compile warning */
    (void)float_min; /* to avoid compile warning */
    (void)float_1e32; /* to avoid compile warning */
    (void)float_2; /* to avoid compile warning */
    (void)float_3; /* to avoid compile warning */
    return latxs_translate_wait(pir1);
#else
    IR2_OPND temp = ra_alloc_itemp();
    IR2_OPND sw_opnd = ra_alloc_itemp();
    IR2_OPND ftemp1_opnd = ra_alloc_ftemp();
    IR2_OPND ftemp2_opnd = ra_alloc_ftemp();
    IR2_OPND value_addr_opnd;

    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_O_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_U_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd2i_em(LISA_LD_H, temp, env_ir2_opnd,
                            lsenv_offset_of_control_word(lsenv));
    la_append_ir2_opnd2i_em(LISA_LD_H, sw_opnd, env_ir2_opnd,
                            lsenv_offset_of_status_word(lsenv));
    la_append_ir2_opnd3_em(LISA_ANDN, sw_opnd, sw_opnd, temp);
    la_append_ir2_opnd2i_em(LISA_ANDI, sw_opnd, sw_opnd, 0x3d); /* PUOZ_I */

    la_append_ir2_opnd3(LISA_BEQ, sw_opnd, zero_ir2_opnd, label_exit);
    /* raise umasked float exceptions */
    /* Invalid-arithmetic-operand exception */
    /* TODO */
    /* Divide-By-Zero Exception */
    /* TODO */
    /* Numeric Overflow Exception */
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, temp, sw_opnd,
                            X87_SR_OFF_OE, X87_SR_OFF_OE);
    la_append_ir2_opnd3(LISA_BEQ, temp, zero_ir2_opnd, label_O_exit);
    value_addr_opnd = convert_mem_addr((ADDR)&float_max);
    la_append_ir2_opnd2i(LISA_FLD_S, ftemp1_opnd, value_addr_opnd, 0);
    value_addr_opnd = convert_mem_addr((ADDR)&float_1e32);
    la_append_ir2_opnd2i(LISA_FLD_S, ftemp2_opnd, value_addr_opnd, 0);
    la_append_ir2_opnd3(LISA_FADD_S, ftemp1_opnd, ftemp1_opnd, ftemp2_opnd);
    la_append_ir2_opnd1(LISA_LABEL, label_O_exit);

    /* Numeric Underflow Exception */
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, temp, sw_opnd,
                            X87_SR_OFF_UE, X87_SR_OFF_UE);
    la_append_ir2_opnd3(LISA_BEQ, temp, zero_ir2_opnd, label_U_exit);
    value_addr_opnd = convert_mem_addr((ADDR)&float_min);
    la_append_ir2_opnd2i(LISA_FLD_S, ftemp1_opnd, value_addr_opnd, 0);
    value_addr_opnd = convert_mem_addr((ADDR)&float_3);
    la_append_ir2_opnd2i(LISA_FLD_S, ftemp2_opnd, value_addr_opnd, 0);
    la_append_ir2_opnd3(LISA_FDIV_S, ftemp1_opnd, ftemp1_opnd, ftemp2_opnd);
    la_append_ir2_opnd1(LISA_LABEL, label_U_exit);

    /* Inexact-Result (Precision) Exception */
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, temp, sw_opnd,
                            X87_SR_OFF_PE, X87_SR_OFF_PE);
    la_append_ir2_opnd3(LISA_BEQ, temp, zero_ir2_opnd, label_exit);
    value_addr_opnd = convert_mem_addr((ADDR)&float_2);
    la_append_ir2_opnd2i(LISA_FLD_S, ftemp1_opnd, value_addr_opnd, 0);
    value_addr_opnd = convert_mem_addr((ADDR)&float_3);
    la_append_ir2_opnd2i(LISA_FLD_S, ftemp2_opnd, value_addr_opnd, 0);
    la_append_ir2_opnd3(LISA_FDIV_S, ftemp1_opnd, ftemp1_opnd, ftemp2_opnd);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    return true;
#endif
}

bool translate_fnclex(IR1_INST *pir1) {
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fnclex(pir1);
#else
    /* get status word and load in sw_value */
    IR2_OPND sw_value = ra_alloc_itemp();
    IR2_OPND fcsr0 = ra_alloc_itemp();
    int offset = lsenv_offset_of_status_word(lsenv);
    la_append_ir2_opnd2i_em(LISA_LD_HU, sw_value, env_ir2_opnd, offset);
    la_append_ir2_opnd2_em(LISA_MOVFCSR2GR, fcsr0, fcsr_ir2_opnd);
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, sw_value, zero_ir2_opnd, X87_SR_OFF_ES, X87_SR_OFF_IE);
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, sw_value, zero_ir2_opnd, X87_SR_OFF_B, X87_SR_OFF_B);
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, fcsr0, zero_ir2_opnd, FCSR_OFF_FLAGS_V, FCSR_OFF_FLAGS_I);
    la_append_ir2_opnd2_em(LISA_MOVGR2FCSR, fcsr_ir2_opnd, fcsr0);
    la_append_ir2_opnd2i(LISA_ST_H, sw_value, env_ir2_opnd,
                        lsenv_offset_of_status_word(lsenv));
    return true;
#endif
}

bool translate_finit(IR1_INST *pir1)
{
   /*
    * FIXME: checking for pending unmasked exceptions?
    */
    translate_fninit(pir1);
    return true;
}

bool translate_fninit(IR1_INST *pir1) {
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fninit(pir1);
#else
    IR2_OPND temp = ra_alloc_itemp();
    IR2_OPND fcsr0 = ra_alloc_itemp();
    int offset;

    /* clear status and set control*/
    offset = lsenv_offset_of_status_word(lsenv);
    lsassert(offset <= 0x7ff);
    load_ireg_from_imm32(temp, 0x37f0000ULL, UNKNOWN_EXTENSION);
    la_append_ir2_opnd2i(LISA_ST_W, temp, env_ir2_opnd, offset);

    /* clear top */
    if (option_lsfpu) {
        la_append_ir2_opndi(LISA_X86MTTOP, 0);
        la_append_ir2_opnd0(LISA_X86SETTM);
    } else {
        offset = lsenv_offset_of_top(lsenv);
        lsassert(offset <= 0x7ff);
        la_append_ir2_opnd2i(LISA_ST_W, zero_ir2_opnd, env_ir2_opnd, offset);
    }

    /* set fptags */
    offset = lsenv_offset_of_tag_word(lsenv);
    lsassert(offset <= 0x7ff);
    load_ireg_from_imm64(temp, 0x101010101010101ULL);
    la_append_ir2_opnd2i(LISA_ST_D, temp, env_ir2_opnd, offset);

    la_append_ir2_opnd2_em(LISA_MOVFCSR2GR, fcsr0, fcsr_ir2_opnd);
    /* VZOUI disable */
    load_ireg_from_imm32(temp, FCSR_ENABLE_CLEAR, UNKNOWN_EXTENSION);
    la_append_ir2_opnd3_em(LISA_AND, fcsr0, fcsr0, temp);
    /* RM = 00 */
    load_ireg_from_imm32(temp, FCSR_RM_CLEAR, UNKNOWN_EXTENSION);
    la_append_ir2_opnd3_em(LISA_AND, fcsr0, fcsr0, temp);
    la_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr_ir2_opnd, fcsr0);

    return true;
#endif
}
