#include <math.h>
#include "../include/common.h"
#include "../include/reg_alloc.h"
#include "../include/env.h"
#include "../x86tomips-options.h"

void temply_set_fcsr_rounding_mode_according_x86_cw(void);

bool translate_fld(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    /* 1. the position to be overwritten is st(7) at this time */
    IR2_OPND dest_opnd = ra_alloc_st(7);

    /* 2. load the value */
    load_freg_from_ir1_2(&dest_opnd, ir1_get_opnd(pir1, 0), false, true);

    /* 3. adjust top */
    tr_fpu_push();

    return true;
}

bool translate_fldz(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    /* 1. the position to be overwritten is st(0) */
    tr_fpu_push();
    IR2_OPND dest_opnd = ra_alloc_st(0);

    /* 2. load the value */
    append_ir2_opnd2(LISA_MOVGR2FR_D, &dest_opnd, &zero_ir2_opnd);
    append_ir2_opnd2(LISA_FFINT_D_W, &dest_opnd, &dest_opnd);
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_opnd);
//    append_ir2_opnd2(mips_cvt_d_w, &dest_opnd, &dest_opnd);

    return true;
}

bool translate_fld1(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    /* 1. the position to be overwritten is st(0) */
    tr_fpu_push();
    IR2_OPND dest_opnd = ra_alloc_st(0);

    /* 2. load the value */
    IR2_OPND itemp_1 = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ORI, &itemp_1, &zero_ir2_opnd, 1);
    append_ir2_opnd2(LISA_MOVGR2FR_D, &dest_opnd, &itemp_1);
    append_ir2_opnd2(LISA_FFINT_D_W, &dest_opnd, &dest_opnd);
//    append_ir2_opnd2(mips_dmtc1, &itemp_1, &dest_opnd);
//    append_ir2_opnd2(mips_cvt_d_w, &dest_opnd, &dest_opnd);

    ra_free_temp(&itemp_1);
    return true;
}

static double l2e;
bool translate_fldl2e(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    l2e = 1 / log(2.0);
    /* 1. the position to be overwritten is st(0) */
    tr_fpu_push();
    IR2_OPND dest_opnd = ra_alloc_st(0);

    /* 2. load the value */
    IR2_OPND value_addr_opnd = ra_alloc_itemp();
    load_addr_to_ir2(&value_addr_opnd, (ADDR)&l2e);
    append_ir2_opnd2i(LISA_FLD_D, &dest_opnd, &value_addr_opnd, 0);

    return true;
}

static double l2t;
bool translate_fldl2t(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    l2t = 1 / log10(2.0);

    /* 1. the position to be overwritten is st(0) */
    tr_fpu_push();
    IR2_OPND dest_opnd = ra_alloc_st(0);

    /* 2. load the value */
    IR2_OPND value_addr_opnd = ra_alloc_itemp();
    load_addr_to_ir2(&value_addr_opnd, (ADDR)&l2t);
    append_ir2_opnd2i(LISA_FLD_D, &dest_opnd, &value_addr_opnd, 0);

    return true;
}

static double lg2;
bool translate_fldlg2(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    lg2 = log10(2.0);

    /* 1. the position to be overwritten is st(0) */
    tr_fpu_push();
    IR2_OPND dest_opnd = ra_alloc_st(0);

    /* 2. load the value */
    IR2_OPND value_addr_opnd = ra_alloc_itemp();
    load_addr_to_ir2(&value_addr_opnd, (ADDR)&lg2);
    append_ir2_opnd2i(LISA_FLD_D, &dest_opnd, &value_addr_opnd, 0);

    return true;
}

static double ln2;
bool translate_fldln2(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    ln2 = log(2.0);

    /* 1. the position to be overwritten is st(0) */
    tr_fpu_push();
    IR2_OPND dest_opnd = ra_alloc_st(0);

    /* 2. load the value */
    IR2_OPND value_addr_opnd = ra_alloc_itemp();
    load_addr_to_ir2(&value_addr_opnd, (ADDR)&ln2);
    append_ir2_opnd2i(LISA_FLD_D, &dest_opnd, &value_addr_opnd, 0);

    return true;
}

static double pi = M_PI;
bool translate_fldpi(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    /* 1. the position to be overwritten is st(0) */
    tr_fpu_push();
    IR2_OPND dest_opnd = ra_alloc_st(0);

    /* 2. load the value */
    IR2_OPND value_addr_opnd = ra_alloc_itemp();
    load_addr_to_ir2(&value_addr_opnd, (ADDR)&pi);
    append_ir2_opnd2i(LISA_FLD_D, &dest_opnd, &value_addr_opnd, 0);

    return true;
}

bool translate_fstp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    /* 1. load value from st(0) */
    IR2_OPND src_opnd = ra_alloc_st(0);

    /* 2. write to target */
    store_freg_to_ir1(&src_opnd, ir1_get_opnd(pir1, 0), false, true);

    /* 3. adjust top */
    tr_fpu_pop();

    return true;
}

bool translate_fst(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    /* 1. load value from st(0) */
    IR2_OPND src_opnd = ra_alloc_st(0);

    /* 2. write to target */
    store_freg_to_ir1(&src_opnd, ir1_get_opnd(pir1, 0), false, true);

    return true;
}

bool translate_fild(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    lsassert(ir1_get_opnd_num(pir1) == 1);

    IR2_OPND dest_opnd = ra_alloc_st(7);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    load_freg_from_ir1_2(&dest_opnd, opnd0, false, false);
    if (ir1_opnd_size(opnd0) > 32) {
    	append_ir2_opnd2(LISA_FFINT_D_L, &dest_opnd, &dest_opnd);
    } else {
    	append_ir2_opnd2(LISA_FFINT_D_W, &dest_opnd, &dest_opnd);
    }

    tr_fpu_push();

    return true;
}

void temply_set_fcsr_rounding_mode_according_x86_cw(void)
{
    lsassertm(0, "set fcsr to be implemented in LoongArch.\n");
//    IR2_OPND control_word_mem =
//        ir2_opnd_new2(IR2_OPND_MEM, env_ir2_opnd._reg_num,
//                      lsenv_offset_of_control_word(lsenv));
//    IR2_OPND control_word = ra_alloc_itemp();
//
//    IR2_OPND temp_fcsr = ra_alloc_itemp();
//    append_ir2_opnd2(mips_cfc1, &temp_fcsr, &fcsr_ir2_opnd);
//    append_ir2_opnd2(mips_lh, &control_word, &control_word_mem);
//    append_ir2_opnd2i(mips_dsrl, &control_word, &control_word, 10);
//    append_ir2_opnd2i(mips_andi, &control_word, &control_word, 3);
//    IR2_OPND temp_cw = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_andi, &temp_cw, &control_word, 1);
//    IR2_OPND label = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &temp_cw, &zero_ir2_opnd, &label);
//    ra_free_temp(&temp_cw);
//    append_ir2_opnd2i(mips_daddiu, &control_word, &control_word, 2);
//    append_ir2_opnd2i(mips_andi, &control_word, &control_word, 3);
//
//    append_ir2_opnd1(mips_label, &label);
//    append_ir2_opnd2i(mips_dsrl, &temp_fcsr, &temp_fcsr, 2);
//    append_ir2_opnd2i(mips_dsll, &temp_fcsr, &temp_fcsr, 2);
//    append_ir2_opnd3(mips_or, &temp_fcsr, &temp_fcsr, &control_word);
//
//    append_ir2_opnd2(mips_ctc1, &temp_fcsr, &fcsr_ir2_opnd);
//
//    ra_free_temp(&control_word);
//    ra_free_temp(&temp_fcsr);
}

bool translate_fist(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    /* 1. load rounding_mode from x86_control_word to mips_fcsr */
    IR2_OPND tmp_fcsr = ra_alloc_itemp();
    append_ir2_opnd2(LISA_MOVFCSR2GR, &tmp_fcsr, &fcsr_ir2_opnd);
//    append_ir2_opnd2(mips_cfc1, &tmp_fcsr, &fcsr_ir2_opnd);

//    temply_set_fcsr_rounding_mode_according_x86_cw();

    lsassert(ir1_get_opnd_num(pir1) == 1);

    IR2_OPND dest_opnd = ra_alloc_st(0);

    IR2_OPND dest_int = ra_alloc_itemp();

    /* first, we should make sure if dest_opnd is unordered? overflow? */
    /* underflow? */
    uint64 low_bound, high_bound;
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
        lsassertm(0, "Wrong opnd size : %d!\n", ir1_opnd_size(ir1_get_opnd(pir1, 0)));
    }

    IR2_OPND bounder_opnd = ra_alloc_itemp();
    load_imm64_to_ir2(&bounder_opnd, high_bound); /* double for 0x7fff+0.499999 */
    IR2_OPND f_high_bounder_opnd = ra_alloc_ftemp();
    append_ir2_opnd2(LISA_MOVGR2FR_D, &f_high_bounder_opnd, &bounder_opnd);
//    append_ir2_opnd2(mips_dmtc1, &bounder_opnd, &f_high_bounder_opnd);

    load_imm64_to_ir2(&bounder_opnd, low_bound); /* double for 0x8000+(-0.499999) */
    IR2_OPND f_low_bounder_opnd = ra_alloc_ftemp();
    append_ir2_opnd2(LISA_MOVGR2FR_D, &f_low_bounder_opnd, &bounder_opnd);
//    append_ir2_opnd2(mips_dmtc1, &bounder_opnd, &f_low_bounder_opnd);
    ra_free_temp(&bounder_opnd);

    /*is unorder? */
    append_ir2_opnd3i(LISA_FCMP_COND_D, &fcc0_ir2_opnd,
            &dest_opnd, &dest_opnd, FCMP_COND_CUN);
//    append_ir2_opnd2(mips_c_un_d, &dest_opnd, &dest_opnd);
    IR2_OPND label_flow = ir2_opnd_new_label();
    append_ir2_opnd2(LISA_BCNEZ, &fcc0_ir2_opnd, &label_flow);
//    append_ir2_opnd1(mips_bc1t, &label_flow);

    /* is underflow or overflow? */
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) != 64) {
        append_ir2_opnd3i(LISA_FCMP_COND_D, &fcc0_ir2_opnd,
                &f_high_bounder_opnd, &dest_opnd, FCMP_COND_CLE);
        append_ir2_opnd2(LISA_BCNEZ, &fcc0_ir2_opnd, &label_flow);
        append_ir2_opnd3i(LISA_FCMP_COND_D, &fcc0_ir2_opnd,
                &dest_opnd, &f_low_bounder_opnd, FCMP_COND_CLE);
        append_ir2_opnd2(LISA_BCNEZ, &fcc0_ir2_opnd, &label_flow);
//        append_ir2_opnd2(mips_c_le_d, &f_high_bounder_opnd, &dest_opnd);
//        append_ir2_opnd1(mips_bc1t, &label_flow);
//        append_ir2_opnd2(mips_c_le_d, &dest_opnd, &f_low_bounder_opnd);
//        append_ir2_opnd1(mips_bc1t, &label_flow);
    } else {
        append_ir2_opnd3i(LISA_FCMP_COND_D, &fcc0_ir2_opnd,
                &f_high_bounder_opnd, &dest_opnd, FCMP_COND_CLT);
        append_ir2_opnd2(LISA_BCNEZ, &fcc0_ir2_opnd, &label_flow);
        append_ir2_opnd3i(LISA_FCMP_COND_D, &fcc0_ir2_opnd,
                &dest_opnd, &f_low_bounder_opnd, FCMP_COND_CLT);
        append_ir2_opnd2(LISA_BCNEZ, &fcc0_ir2_opnd, &label_flow);
//        append_ir2_opnd2(mips_c_lt_d, &f_high_bounder_opnd, &dest_opnd);
//        append_ir2_opnd1(mips_bc1t, &label_flow);
//        append_ir2_opnd2(mips_c_lt_d, &dest_opnd, &f_low_bounder_opnd);
//        append_ir2_opnd1(mips_bc1t, &label_flow);
    }
    ra_free_temp(&f_low_bounder_opnd);
    ra_free_temp(&f_high_bounder_opnd);

    /* not unorder or flow */
    IR2_OPND fp_opnd = ra_alloc_ftemp();
    append_ir2_opnd2(LISA_FTINT_L_D,  &fp_opnd,  &dest_opnd);
    append_ir2_opnd2(LISA_MOVFR2GR_D, &dest_int, &fp_opnd);
//    append_ir2_opnd2(mips_cvt_l_d, &fp_opnd, &dest_opnd);
//    append_ir2_opnd2(mips_dmfc1, &dest_int, &fp_opnd);
    ra_free_temp(&fp_opnd);

    IR2_OPND label_end = ir2_opnd_new_label();
    append_ir2_opnd1(LISA_B, &label_end);

    /*overflow, mov ox8000 or 0x80000000 to dest*/
    append_ir2_opnd1(LISA_LABEL, &label_flow);
    switch (ir1_opnd_size(ir1_get_opnd(pir1, 0))) {
    case 16:
        load_imm32_to_ir2(&dest_int, 0x8000, SIGN_EXTENSION);
        break;
    case 32:
        load_imm32_to_ir2(&dest_int, 0x80000000, SIGN_EXTENSION);
        break;
    case 64:
        load_imm64_to_ir2(&dest_int, 0x8000000000000000ull);
        break;
    default:
        lsassertm(0, "Wrong opnd size : %d!\n", ir1_opnd_size(ir1_get_opnd(pir1, 0)));
    }

    append_ir2_opnd1(LISA_LABEL, &label_end);

//    append_ir2_opnd2(mips_ctc1, &tmp_fcsr, &fcsr_ir2_opnd);
    append_ir2_opnd2(LISA_MOVGR2FCSR, &fcsr_ir2_opnd, &tmp_fcsr);
    ra_free_temp(&tmp_fcsr);

    store_ir2_to_ir1(&dest_int, ir1_get_opnd(pir1, 0), false);
    ra_free_temp(&dest_int);

    return true;
}

bool translate_fistp(IR1_INST *pir1)
{
    translate_fist(pir1);
    tr_fpu_pop();
    return true;
}

bool translate_fnstsw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    lsassert(ir1_get_opnd_num(pir1) == 1);

    IR2_OPND sw_value = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_LD_HU, &sw_value, &env_ir2_opnd,
                      lsenv_offset_of_status_word(lsenv));

    IR2_OPND mask = ra_alloc_itemp();
    load_imm32_to_ir2(&mask, 0xc7ff, ZERO_EXTENSION);
    append_ir2_opnd3(LISA_AND, &sw_value, &sw_value, &mask);
    ra_free_temp(&mask);
//    append_ir2_opnd2i(mips_andi, &sw_value, &sw_value, 0xc7ff);

    IR2_OPND top = ra_alloc_itemp();
    if (!option_lsfpu) {
        append_ir2_opnd2i(LISA_LD_W, &top, &env_ir2_opnd, lsenv_offset_of_top_bias(lsenv));

        append_ir2_opnd2i(LISA_ADDI_D, &top, &top, td_fpu_get_top());
        append_ir2_opnd2i(LISA_ANDI, &top, &top, 0x7);
    } else {
        append_ir2_opnd1(LISA_X86MFTOP, &top);
        append_ir2_opnd2i(LISA_ANDI, &top, &top, 0x7);
    }
    append_ir2_opnd2i(LISA_SLLI_W, &top, &top, 11);
    append_ir2_opnd3(LISA_OR, &sw_value, &sw_value, &top);
    /* status_word in memory won't get timely update, write back to help a bit */
    if (option_lsfpu) {
        append_ir2_opnd2i(LISA_ST_H, &sw_value, &env_ir2_opnd,
                lsenv_offset_of_status_word(lsenv));
    }

    /* 2. store the current value of status_word to dest_opnd */
    //fix capstone bug
    pir1->info->detail->x86.operands[0].size = 2;
    store_ir2_to_ir1(&sw_value, ir1_get_opnd(pir1, 0), false);

    /* 3. free tmp */
    ra_free_temp(&sw_value);
    /* ra_free_temp(&top); */

    return true;
}

bool translate_fstsw(IR1_INST *pir1) { return translate_fnstsw(pir1); }

static void __attribute__((__constructor__))
x86tomips_tr_fldst_init(void)
{
    pi = M_PI;
    lg2 = log10(2.0);
    l2t = 1 / log10(2.0);
    l2e = 1 / log(2.0);
    ln2 = log(2.0);
}
