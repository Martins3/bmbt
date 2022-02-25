#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>
#include <math.h>

void latxs_sys_fldst_register_ir1(void)
{
    latxs_register_ir1(X86_INS_FLD);
    latxs_register_ir1(X86_INS_FLDZ);
    latxs_register_ir1(X86_INS_FLD1);
    latxs_register_ir1(X86_INS_FLDL2E);
    latxs_register_ir1(X86_INS_FLDL2T);
    latxs_register_ir1(X86_INS_FLDLG2);
    latxs_register_ir1(X86_INS_FLDLN2);
    latxs_register_ir1(X86_INS_FLDPI);
    latxs_register_ir1(X86_INS_FSTP);
    latxs_register_ir1(X86_INS_FST);
    latxs_register_ir1(X86_INS_FILD);
    latxs_register_ir1(X86_INS_FIST);
    latxs_register_ir1(X86_INS_FISTP);
    latxs_register_ir1(X86_INS_FBLD);
    latxs_register_ir1(X86_INS_FBSTP);
}

bool latxs_translate_fld(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fld_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    /* 1. the position to be overwritten is st(7) at this time */
    IR2_OPND dest_opnd = latxs_ra_alloc_st(7);

    /* 2. load the value */
    latxs_load_freg_from_ir1_2(&dest_opnd, ir1_get_opnd(pir1, 0), true);

    /* 3. adjust top */
    latxs_tr_fpu_push();

    return true;
}

bool latxs_translate_fldz(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fldz_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    /* 1. the position to be overwritten is st(0) */
    latxs_tr_fpu_push();
    IR2_OPND dest_opnd = latxs_ra_alloc_st(0);

    /* 2. load the value */
    latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &dest_opnd, &zero_ir2_opnd);
    latxs_append_ir2_opnd2(LISA_FFINT_D_W, &dest_opnd, &dest_opnd);

    return true;
}

bool latxs_translate_fld1(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fld1_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    /* 1. the position to be overwritten is st(0) */
    latxs_tr_fpu_push();
    IR2_OPND dest_opnd = latxs_ra_alloc_st(0);

    /* 2. load the value */
    IR2_OPND itemp_1 = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ORI, &itemp_1, &zero_ir2_opnd, 1);
    latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &dest_opnd, &itemp_1);
    latxs_append_ir2_opnd2(LISA_FFINT_D_W, &dest_opnd, &dest_opnd);

    latxs_ra_free_temp(&itemp_1);
    return true;
}

static double l2e;
bool latxs_translate_fldl2e(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fldl2e_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    l2e = 1 / log(2.0);
    /* 1. the position to be overwritten is st(0) */
    latxs_tr_fpu_push();
    IR2_OPND dest_opnd = latxs_ra_alloc_st(0);

    /* 2. load the value */
    IR2_OPND value_addr_opnd = latxs_ra_alloc_itemp();
    latxs_load_addr_to_ir2(&value_addr_opnd, (ADDR)(&l2e));
    latxs_append_ir2_opnd2i(LISA_FLD_D, &dest_opnd, &value_addr_opnd, 0);

    return true;
}

static double l2t;
bool latxs_translate_fldl2t(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fldl2t_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    l2t = 1 / log10(2.0);

    /* 1. the position to be overwritten is st(0) */
    latxs_tr_fpu_push();
    IR2_OPND dest_opnd = latxs_ra_alloc_st(0);

    /* 2. load the value */
    IR2_OPND value_addr_opnd = latxs_ra_alloc_itemp();
    latxs_load_addr_to_ir2(&value_addr_opnd, (ADDR)(&l2t));
    latxs_append_ir2_opnd2i(LISA_FLD_D, &dest_opnd, &value_addr_opnd, 0);

    return true;
}

static double lg2;
bool latxs_translate_fldlg2(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fldlg2_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    lg2 = log10(2.0);

    /* 1. the position to be overwritten is st(0) */
    latxs_tr_fpu_push();
    IR2_OPND dest_opnd = latxs_ra_alloc_st(0);

    /* 2. load the value */
    IR2_OPND value_addr_opnd = latxs_ra_alloc_itemp();
    latxs_load_addr_to_ir2(&value_addr_opnd, (ADDR)(&lg2));
    latxs_append_ir2_opnd2i(LISA_FLD_D, &dest_opnd, &value_addr_opnd, 0);

    return true;
}

static double ln2;
bool latxs_translate_fldln2(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fldln2_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    ln2 = log(2.0);

    /* 1. the position to be overwritten is st(0) */
    latxs_tr_fpu_push();
    IR2_OPND dest_opnd = latxs_ra_alloc_st(0);

    /* 2. load the value */
    IR2_OPND value_addr_opnd = latxs_ra_alloc_itemp();
    latxs_load_addr_to_ir2(&value_addr_opnd, (ADDR)(&ln2));
    latxs_append_ir2_opnd2i(LISA_FLD_D, &dest_opnd, &value_addr_opnd, 0);

    return true;
}

static double pi = M_PI;
bool latxs_translate_fldpi(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fldpi_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    /* 1. the position to be overwritten is st(0) */
    latxs_tr_fpu_push();
    IR2_OPND dest_opnd = latxs_ra_alloc_st(0);

    /* 2. load the value */
    IR2_OPND value_addr_opnd = latxs_ra_alloc_itemp();
    latxs_load_addr_to_ir2(&value_addr_opnd, (ADDR)(&pi));
    latxs_append_ir2_opnd2i(LISA_FLD_D, &dest_opnd, &value_addr_opnd, 0);

    return true;
}

bool latxs_translate_fstp(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fstp_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    IR2_OPND src_opnd = latxs_ra_alloc_st(0);

    latxs_store_freg_to_ir1(&src_opnd, ir1_get_opnd(pir1, 0), true);

    latxs_tr_fpu_pop();

    return true;
}

bool latxs_translate_fst(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fst_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    IR2_OPND src_opnd = latxs_ra_alloc_st(0);

    latxs_store_freg_to_ir1(&src_opnd, ir1_get_opnd(pir1, 0), true);

    return true;
}

bool latxs_translate_fild(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fild_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    lsassert(ir1_get_opnd_num(pir1) == 1);

    IR2_OPND dest_opnd = latxs_ra_alloc_st(7);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    latxs_load_freg_from_ir1_2(&dest_opnd, opnd0, false);
    if (ir1_opnd_size(opnd0) > 32) {
        latxs_append_ir2_opnd2(LISA_FFINT_D_L, &dest_opnd, &dest_opnd);
    } else {
        latxs_append_ir2_opnd2(LISA_FFINT_D_W, &dest_opnd, &dest_opnd);
    }

    latxs_tr_fpu_push();

    return true;
}

bool latxs_translate_fist(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fist_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    IR2_OPND *fcsr = &latxs_fcsr_ir2_opnd;
    IR2_OPND *fcc0 = &latxs_fcc0_ir2_opnd;

    /* 1. load rounding_mode from x86_control_word to mips_fcsr */
    IR2_OPND tmp_fcsr = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2(LISA_MOVFCSR2GR, &tmp_fcsr, fcsr);

#if !defined(LATX_SYS_FCSR)
    {
        IR2_OPND *env = &latxs_env_ir2_opnd;
        IR2_OPND cw = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_LD_W, &cw, env,
                lsenv_offset_of_control_word(lsenv));
        latxs_update_fcsr_by_cw(cw);
        latxs_ra_free_temp(&cw);
    }
#endif

    lsassert(ir1_get_opnd_num(pir1) == 1);

    IR2_OPND dest_opnd = latxs_ra_alloc_st(0);

    IR2_OPND dest_int = latxs_ra_alloc_itemp();

    /* first, we should make sure if dest_opnd is unordered? overflow? */
    /* underflow? */
    uint64_t low_bound = 0;
    uint64_t high_bound = 0;
    switch (ir1_opnd_size(ir1_get_opnd(pir1, 0))) {
    case 16:
        low_bound = 0xc0dfffe000000000ull;
        high_bound = 0x40dfffe000000000ull;
        break;
    case 32:
        low_bound = 0xc1dfffffffe00000ull;
        high_bound = 0x41dfffffffe00000ull;
        break;
    case 64:
        low_bound = 0xc3dfffffffffffffull;
        high_bound = 0x43dfffffffffffffull;
        break;
    default:
        lsassertm(0, "Wrong opnd size : %d!\n",
                ir1_opnd_size(ir1_get_opnd(pir1, 0)));
        break;
    }

    IR2_OPND bounder_opnd = latxs_ra_alloc_itemp();
    /* double for 0x7fff+0.499999 */
    latxs_load_imm64_to_ir2(&bounder_opnd, high_bound);
    IR2_OPND f_high_bounder_opnd = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &f_high_bounder_opnd,
                                            &bounder_opnd);

    /* double for 0x8000+(-0.499999) */
    latxs_load_imm64_to_ir2(&bounder_opnd, low_bound);
    IR2_OPND f_low_bounder_opnd = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &f_low_bounder_opnd,
                                            &bounder_opnd);
    latxs_ra_free_temp(&bounder_opnd);

    /*is unorder? */
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0,
            &dest_opnd, &dest_opnd, FCMP_COND_CUN);
    IR2_OPND label_flow = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd2(LISA_BCNEZ, fcc0, &label_flow);

    /* is underflow or overflow? */
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) != 64) {
        latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0,
                &f_high_bounder_opnd, &dest_opnd, FCMP_COND_CLE);
        latxs_append_ir2_opnd2(LISA_BCNEZ, fcc0, &label_flow);
        latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0,
                &dest_opnd, &f_low_bounder_opnd, FCMP_COND_CLE);
        latxs_append_ir2_opnd2(LISA_BCNEZ, fcc0, &label_flow);
    } else {
        latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0,
                &f_high_bounder_opnd, &dest_opnd, FCMP_COND_CLT);
        latxs_append_ir2_opnd2(LISA_BCNEZ, fcc0, &label_flow);
        latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0,
                &dest_opnd, &f_low_bounder_opnd, FCMP_COND_CLT);
        latxs_append_ir2_opnd2(LISA_BCNEZ, fcc0, &label_flow);
    }
    latxs_ra_free_temp(&f_low_bounder_opnd);
    latxs_ra_free_temp(&f_high_bounder_opnd);

    /* not unorder or flow */
    IR2_OPND fp_opnd = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2(LISA_FTINT_L_D,  &fp_opnd,  &dest_opnd);
    latxs_append_ir2_opnd2(LISA_MOVFR2GR_D, &dest_int, &fp_opnd);
    latxs_ra_free_temp(&fp_opnd);

    IR2_OPND label_end = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd1(LISA_B, &label_end);

    /*overflow, mov ox8000 or 0x80000000 to dest*/
    latxs_append_ir2_opnd1(LISA_LABEL, &label_flow);
    switch (ir1_opnd_size(ir1_get_opnd(pir1, 0))) {
    case 16:
        latxs_load_imm32_to_ir2(&dest_int, 0x8000, EXMode_S);
        break;
    case 32:
        latxs_load_imm32_to_ir2(&dest_int, 0x80000000, EXMode_S);
        break;
    case 64:
        latxs_load_imm64_to_ir2(&dest_int, 0x8000000000000000ull);
        break;
    default:
        lsassertm(0, "Wrong opnd size : %d!\n",
                ir1_opnd_size(ir1_get_opnd(pir1, 0)));
        break;
    }

    latxs_append_ir2_opnd1(LISA_LABEL, &label_end);

    latxs_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr, &tmp_fcsr);
    latxs_ra_free_temp(&tmp_fcsr);

    latxs_store_ir2_to_ir1(&dest_int, ir1_get_opnd(pir1, 0));
    latxs_ra_free_temp(&dest_int);

    return true;
}

bool latxs_translate_fistp(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fistp_softfpu(pir1);
    }
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    latxs_translate_fist(pir1);
    latxs_tr_fpu_pop();
    return true;
}

bool latxs_translate_fbld(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fbld_softfpu(pir1);
    }
    lsassert(0);
    return false;
}
bool latxs_translate_fbstp(IR1_INST *pir1)
{
    if (option_soft_fpu) {
        return latxs_translate_fbstp_softfpu(pir1);
    }
    lsassert(0);
    return false;
}

static void __attribute__((__constructor__)) latxs_tr_fldst_init(void)
{
    pi = M_PI;
    lg2 = log10(2.0);
    l2t = 1 / log10(2.0);
    l2e = 1 / log(2.0);
    ln2 = log(2.0);
}
