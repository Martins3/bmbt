#include "common.h"
#include "reg-alloc.h"
#include "lsenv.h"
#include "latx-options.h"
#include "translate.h"

/* bool translate_cvtpd2pi(IR1_INST * pir1) { return false; } */
/* bool translate_cvtpd2dq(IR1_INST * pir1) { return false; } */
/* bool translate_cvttpd2dq(IR1_INST *pir1) { return false; } */
/* bool translate_cvtdq2ps(IR1_INST * pir1) { return false; } */
/* bool translate_cvtdq2pd(IR1_INST * pir1) { return false; } */
/* bool translate_cvtps2pi(IR1_INST * pir1) { return false; } */
/* bool translate_cvtps2dq(IR1_INST * pir1) { return false; } */
/* bool translate_cvttps2dq(IR1_INST *pir1) { return false; } */
/* bool translate_cvtpi2pd(IR1_INST * pir1) { return false; } */
/* bool translate_cvtpi2ps(IR1_INST * pir1) { return false; } // use mxcsr */
/* bool translate_cvtpd2ps(IR1_INST * pir1) { return false; } */
/* bool translate_cvtss2sd(IR1_INST * pir1) { return false; } */
/* bool translate_cvtsi2ss(IR1_INST * pir1) { return false; } */
/* bool translate_cvtsi2sd(IR1_INST * pir1) { return false; } */
/* bool translate_cvtsd2ss(IR1_INST * pir1) { return false; } */
/* bool translate_cvtsd2si(IR1_INST * pir1) { return false; } */
/* bool translate_cvtss2si(IR1_INST * pir1) { return false; } */
/* bool translate_cvttss2si(IR1_INST * pir1) { return false; } */
/* bool translate_cvtps2pd(IR1_INST * pir1) { return false; } */
/* bool translate_cvttsd2si(IR1_INST * pir1) { return false; } */

bool translate_cvtdq2pd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvtdq2pd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND fcsr_opnd = set_fpu_fcsr_rounding_field_by_x86();
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd2(LISA_VFFINTL_D_W, dest, src);
    set_fpu_fcsr(fcsr_opnd);
    return true;
#endif
}

bool translate_cvtdq2ps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvtdq2ps(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd2(LISA_VFFINT_S_W, dest, src);
    return true;
#endif
}

bool translate_cvtpd2dq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvtpd2dq(pir1);
#else
    IR2_OPND src_lo;
    IR2_OPND src_hi;
    IR2_OPND dest_lo;                                         
    src_lo = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    src_hi = ra_alloc_ftemp();
    la_append_ir2_opnd2i(LISA_VEXTRINS_D,src_hi, src_lo, VEXTRINS_IMM_4_0(0, 1));
    dest_lo = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));

    IR2_OPND temp_int = ra_alloc_itemp();
    load_ireg_from_imm64(
        temp_int, 0x41dfffffffdffffcULL); /* double for 0x7fffffff.499999 */
    IR2_OPND ftemp_over_flow = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_over_flow, temp_int);
    load_ireg_from_imm64(
        temp_int, 0xc1dfffffffdffffcULL); /* double for 0x80000000.499999 */
    IR2_OPND ftemp_under_flow = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_under_flow, temp_int);
    IR2_OPND label_for_flow1 = ir2_opnd_new_type(IR2_OPND_LABEL); 
    /* low 64bit */
    /* is unorder or overflow or under flow? */
    IR2_OPND ftemp_src_lo = ra_alloc_ftemp();
    /* is unorder? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, src_lo, src_lo, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow1);
    /* is overflow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_over_flow, src_lo, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow1);
    /* is under flow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, src_lo, ftemp_under_flow, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow1);
    /* is normal */
    la_append_ir2_opnd2(LISA_FTINT_W_D, ftemp_src_lo, src_lo);
    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL); 
    la_append_ir2_opnd1(LISA_B, label_high);  

    la_append_ir2_opnd1(LISA_LABEL, label_for_flow1);
    load_ireg_from_imm32(temp_int, 0x80000000, UNKNOWN_EXTENSION);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_src_lo, temp_int);
    la_append_ir2_opnd1(LISA_LABEL, label_high); 
    IR2_OPND label_for_flow2 = ir2_opnd_new_type(IR2_OPND_LABEL); 
    /* high 64bit */
    /* is unorder or overflow or under flow? */
    IR2_OPND ftemp_src_hi = ra_alloc_ftemp();
    /* is unorder? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, src_hi, src_hi, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow2);
    /* is overflow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_over_flow, src_hi, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow2);
    /* is under flow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, src_hi, ftemp_under_flow, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow2);
    /* is normal */
    la_append_ir2_opnd2(LISA_FTINT_W_D, ftemp_src_hi, src_hi);
    IR2_OPND label_over = ir2_opnd_new_type(IR2_OPND_LABEL); 
    la_append_ir2_opnd1(LISA_B, label_over);  

    la_append_ir2_opnd1(LISA_LABEL, label_for_flow2); 
    load_ireg_from_imm32(temp_int, 0x80000000, UNKNOWN_EXTENSION);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_src_hi, temp_int);
    la_append_ir2_opnd1(LISA_LABEL, label_over); 

    //la_append_ir2_opnd3(LISA_VILVL_W, dest_lo, ftemp_src_lo, ftemp_src_hi);
    la_append_ir2_opnd3(LISA_VILVL_W, dest_lo, ftemp_src_hi, ftemp_src_lo);
    la_append_ir2_opnd2i(LISA_XVPICKVE_D,dest_lo, dest_lo, 0);
    ra_free_temp(ftemp_src_lo);
    ra_free_temp(ftemp_src_hi);
    ra_free_temp(ftemp_over_flow);
    ra_free_temp(ftemp_under_flow);
    ra_free_temp(temp_int);
    return true;
#endif
}

bool translate_cvttps2dq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvttps2dq(pir1);
#else
    /* the low 64 bits of src */
    IR2_OPND dest_lo;
    /* the high 64 bits of src */
    IR2_OPND dest_hi;

    IR2_OPND real_dest;
    IR2_OPND real_src;

    real_dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    real_src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    dest_lo = ra_alloc_ftemp();
    dest_hi = ra_alloc_ftemp();

    IR2_OPND temp_int = ra_alloc_itemp();
    IR2_OPND ftemp_over_flow = ra_alloc_ftemp();
    IR2_OPND ftemp_under_flow = ra_alloc_ftemp();
    IR2_OPND label_for_flow1 = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_for_flow2 = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_over = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
    load_ireg_from_imm64(
        temp_int, 0x41dfffffffdffffcULL); /* double for 0x7fffffff.499999 */
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_over_flow, temp_int);
    load_ireg_from_imm64(
        temp_int, 0xc1dfffffffdffffcULL); /* double for 0x80000000.499999 */
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_under_flow, temp_int);
    /* the first single scalar operand */
    /* is unorder or overflow or under flow? */
    IR2_OPND ftemp_src1 = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FCVT_D_S, ftemp_src1, real_src);
    /* is unorder? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src1, ftemp_src1, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow1);
    /* is overflow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_over_flow, ftemp_src1, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow1);
    /* is under flow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src1, ftemp_under_flow, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow1);
    /* is normal */
    la_append_ir2_opnd2(LISA_FTINTRZ_W_D, ftemp_src1,
                     ftemp_src1);
    la_append_ir2_opnd1(LISA_B, label_high);

    la_append_ir2_opnd1(LISA_LABEL, label_for_flow1);
    load_ireg_from_imm32(temp_int, 0x80000000, UNKNOWN_EXTENSION);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_src1, temp_int);
    la_append_ir2_opnd1(LISA_LABEL, label_high);
    /* the second single scalar operand */
    /* is unorder or overflow or under flow? */
    IR2_OPND ftemp_src2 = ra_alloc_ftemp();
    la_append_ir2_opnd2i(LISA_VEXTRINS_W, ftemp_src2, real_src, 
                        VEXTRINS_IMM_4_0(0, 1));
    la_append_ir2_opnd2(LISA_FCVT_D_S, ftemp_src2, ftemp_src2);
    /* is unorder? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src2, ftemp_src2, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow2);
    /* is overflow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_over_flow, ftemp_src2, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow2);
    /* is under flow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src2, ftemp_under_flow, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow2);
    /* is normal */
    la_append_ir2_opnd2(LISA_FTINTRZ_W_D, ftemp_src2,
                     ftemp_src2);
    la_append_ir2_opnd1(LISA_B, label_over);

    la_append_ir2_opnd1(LISA_LABEL, label_for_flow2);
    load_ireg_from_imm32(temp_int, 0x80000000, UNKNOWN_EXTENSION);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_src2, temp_int);
    la_append_ir2_opnd1(LISA_LABEL, label_over);
    /* merge */
    la_append_ir2_opnd3(LISA_VILVL_W, dest_lo, ftemp_src2, ftemp_src1);


    IR2_OPND label_for_flow3 = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_for_flow4 = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_over2 = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_high2 = ir2_opnd_new_type(IR2_OPND_LABEL);
    /* the first single scalar operand */
    /* is unorder or overflow or under flow? */
    la_append_ir2_opnd2i(LISA_VEXTRINS_W, ftemp_src1, real_src, 
                        VEXTRINS_IMM_4_0(0, 2));
    la_append_ir2_opnd2(LISA_FCVT_D_S, ftemp_src1, ftemp_src1);
    /* is unorder? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src1, ftemp_src1, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow3);
    /* is overflow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_over_flow, ftemp_src1, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow3);
    /* is under flow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src1, ftemp_under_flow, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow3);
    /* is normal */
    la_append_ir2_opnd2(LISA_FTINTRZ_W_D, ftemp_src1,
                     ftemp_src1);
    la_append_ir2_opnd1(LISA_B, label_high2);

    la_append_ir2_opnd1(LISA_LABEL, label_for_flow3);
    load_ireg_from_imm32(temp_int, 0x80000000, UNKNOWN_EXTENSION);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_src1, temp_int);
    la_append_ir2_opnd1(LISA_LABEL, label_high2);
    /* the second single scalar operand */
    /* is unorder or overflow or under flow? */
    la_append_ir2_opnd2i(LISA_VEXTRINS_W, ftemp_src2, real_src, 
                        VEXTRINS_IMM_4_0(0, 3));
    la_append_ir2_opnd2(LISA_FCVT_D_S, ftemp_src2, ftemp_src2);
    /* is unorder? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src2, ftemp_src2, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow4);
    /* is overflow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_over_flow, ftemp_src2, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow4);
    /* is under flow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src2, ftemp_under_flow, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow4);
    /* is normal */
    la_append_ir2_opnd2(LISA_FTINTRZ_W_D, ftemp_src2,
                     ftemp_src2);
    la_append_ir2_opnd1(LISA_B, label_over2);

    la_append_ir2_opnd1(LISA_LABEL, label_for_flow4);
    load_ireg_from_imm32(temp_int, 0x80000000, UNKNOWN_EXTENSION);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_src2, temp_int);
    la_append_ir2_opnd1(LISA_LABEL, label_over2);
    /* merge */
    la_append_ir2_opnd3(LISA_VILVL_W, dest_hi, ftemp_src2, ftemp_src1);
    la_append_ir2_opnd2i(LISA_VEXTRINS_D, real_dest, dest_lo, VEXTRINS_IMM_4_0(0, 0));
    la_append_ir2_opnd2i(LISA_VEXTRINS_D, real_dest, dest_hi, VEXTRINS_IMM_4_0(1, 0));
    ra_free_temp(ftemp_src1);
    ra_free_temp(ftemp_src2);
    ra_free_temp(ftemp_over_flow);
    ra_free_temp(ftemp_under_flow);
    ra_free_temp(temp_int);
    return true;
#endif
}

bool translate_cvttpd2dq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvttpd2dq(pir1);
#else
    IR2_OPND src_lo;
    IR2_OPND src_hi;
    IR2_OPND dest_lo;
    src_lo = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    src_hi = ra_alloc_ftemp();
    la_append_ir2_opnd2i(LISA_VEXTRINS_D,src_hi, src_lo, VEXTRINS_IMM_4_0(0, 1));
    dest_lo = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));

    IR2_OPND temp_int = ra_alloc_itemp();
    load_ireg_from_imm64(
        temp_int, 0x41DFFFFFFFFFFFFCULL); /* double for 0x7fffffff.999999 */
    IR2_OPND ftemp_over_flow = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_over_flow, temp_int);

    load_ireg_from_imm64(
        temp_int, 0xc1dfffffffdffffcULL); /* double for 0x80000000.499999 */
    IR2_OPND ftemp_under_flow = ra_alloc_ftemp();

    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_under_flow, temp_int);
    IR2_OPND label_for_flow1 = ir2_opnd_new_type(IR2_OPND_LABEL);
    /* low 64bit */
    /* is unorder or overflow or under flow? */
    IR2_OPND ftemp_src_lo = ra_alloc_ftemp();
    /* is unorder? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D,
        fcc0_ir2_opnd, src_lo, src_lo, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow1);
    /* is overflow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D,
        fcc0_ir2_opnd, ftemp_over_flow, src_lo, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow1);
    /* is normal */
    la_append_ir2_opnd2(LISA_FTINTRZ_W_D, ftemp_src_lo, src_lo);
    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd1(LISA_B, label_high);

    la_append_ir2_opnd1(LISA_LABEL, label_for_flow1);
    load_ireg_from_imm32(temp_int, 0x80000000, UNKNOWN_EXTENSION);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_src_lo, temp_int);
    la_append_ir2_opnd1(LISA_LABEL, label_high);
    IR2_OPND label_for_flow2 = ir2_opnd_new_type(IR2_OPND_LABEL);
    /* high 64bit */
    /* is unorder or overflow or under flow? */
    IR2_OPND ftemp_src_hi = ra_alloc_ftemp();
    /* is unorder? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D,
        fcc0_ir2_opnd, src_hi, src_hi, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow2);
    /* is overflow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D,
        fcc0_ir2_opnd, ftemp_over_flow, src_hi, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow2);
    /* is normal */
    la_append_ir2_opnd2(LISA_FTINTRZ_W_D, ftemp_src_hi, src_hi);
    IR2_OPND label_over = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd1(LISA_B, label_over);

    la_append_ir2_opnd1(LISA_LABEL, label_for_flow2);
    load_ireg_from_imm32(temp_int, 0x80000000, UNKNOWN_EXTENSION);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_src_hi, temp_int);
    la_append_ir2_opnd1(LISA_LABEL, label_over);

    //la_append_ir2_opnd3(LISA_VILVL_W, dest_lo, ftemp_src_lo, ftemp_src_hi);
    la_append_ir2_opnd3(LISA_VILVL_W, dest_lo, ftemp_src_hi, ftemp_src_lo);
    la_append_ir2_opnd2i(LISA_XVPICKVE_D,dest_lo, dest_lo, 0);
    ra_free_temp(ftemp_src_lo);
    ra_free_temp(ftemp_src_hi);
    ra_free_temp(ftemp_over_flow);
    ra_free_temp(ftemp_under_flow);
    ra_free_temp(temp_int);
    return true;
#endif
}

#if !defined(CONFIG_SOFTMMU)
/* TODO x87 FPU and MMX */
static void tr_x87_to_mmx(void)
{
    //reset top
    lsenv->tr_data->curr_top = 0;
    la_append_ir2_opnd2i(LISA_ST_W, zero_ir2_opnd, env_ir2_opnd,
                      lsenv_offset_of_top(lsenv));

    tr_gen_top_mode_init();
}
#endif

bool translate_cvtpd2pi(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvtpd2pi(pir1);
#else
    tr_x87_to_mmx();

    IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                         true); /* fill default parameter */

    IR2_OPND src_lo;
    IR2_OPND src_hi;                                         
    src_lo = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    src_hi = ra_alloc_ftemp();
    la_append_ir2_opnd2i(LISA_VEXTRINS_D,src_hi, src_lo, VEXTRINS_IMM_4_0(0, 1));

    IR2_OPND temp_int = ra_alloc_itemp();
    IR2_OPND ftemp_over_flow = ra_alloc_ftemp();
    IR2_OPND ftemp_under_flow = ra_alloc_ftemp();
    IR2_OPND label_for_flow1 = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_for_flow2 = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_over = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL); 
    load_ireg_from_imm64(
        temp_int, 0x41dfffffffdffffcULL); /* double for 0x7fffffff.499999 */
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_over_flow, temp_int);
    load_ireg_from_imm64(
        temp_int, 0xc1dfffffffdffffcULL); /* double for 0x80000000.499999 */
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_under_flow, temp_int);
    /* low 64bit */
    /* is unorder or overflow or under flow? */
    IR2_OPND ftemp_src_lo = ra_alloc_ftemp();
    /* is unorder? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, src_lo, src_lo, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow1);
    /* is overflow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_over_flow, src_lo, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow1);
    /* is under flow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, src_lo, ftemp_under_flow, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow1);

    la_append_ir2_opnd2(LISA_FTINT_W_D, ftemp_src_lo,
                     src_lo);             
    la_append_ir2_opnd1(LISA_B, label_high); 

    la_append_ir2_opnd1(LISA_LABEL, label_for_flow1); 
    load_ireg_from_imm32(temp_int, 0x80000000, UNKNOWN_EXTENSION);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_src_lo, temp_int);
    la_append_ir2_opnd1(LISA_LABEL, label_high); 
    /* high 64bit */
    /* is unorder or overflow or under flow? */
    IR2_OPND ftemp_src_hi = ra_alloc_ftemp();
    /* is unorder? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, src_hi, src_hi, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow2);
    /* is overflow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_over_flow, src_hi, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow2);
    /* is under flow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, src_hi, ftemp_under_flow, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow2);

    la_append_ir2_opnd2(LISA_FTINT_W_D, ftemp_src_hi,
                     src_hi);             
    la_append_ir2_opnd1(LISA_B, label_over); 

    la_append_ir2_opnd1(LISA_LABEL, label_for_flow2); 
    load_ireg_from_imm32(temp_int, 0x80000000, UNKNOWN_EXTENSION);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_src_hi, temp_int);
    la_append_ir2_opnd1(LISA_LABEL, label_over); 

    //TODO dest is mmx register, but high 64 bits is tainted
    //la_append_ir2_opnd3(LISA_VILVL_W, dest, ftemp_src_lo, ftemp_src_hi);
    la_append_ir2_opnd3(LISA_VILVL_W, dest, ftemp_src_hi, ftemp_src_lo);

    ra_free_temp(ftemp_src_lo);
    ra_free_temp(ftemp_src_hi);
    ra_free_temp(ftemp_over_flow);
    ra_free_temp(ftemp_under_flow);
    ra_free_temp(temp_int);
    return true;
#endif
}

bool translate_cvtpd2ps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvtpd2ps(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VFCVT_S_D, dest, dest, src);
    la_append_ir2_opnd2i(LISA_XVPICKVE_D, dest, dest, 0);
    return true;
#endif
}

bool translate_cvtpi2ps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvtpi2ps(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    tr_x87_to_mmx();
    IR2_OPND fcsr_opnd = set_fpu_fcsr_rounding_field_by_x86();
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_VFFINT_S_W, temp, src);
    la_append_ir2_opnd2i(LISA_VEXTRINS_D, dest, temp, VEXTRINS_IMM_4_0(0, 0));
    set_fpu_fcsr(fcsr_opnd);
    return true;
#endif
}

bool translate_cvtpi2pd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvtpi2pd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    tr_x87_to_mmx();
    IR2_OPND fcsr_opnd = set_fpu_fcsr_rounding_field_by_x86();
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);

    IR2_OPND temp = ra_alloc_ftemp();
    IR2_OPND temp0 = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FFINT_D_W, temp0, src);
    la_append_ir2_opnd2i(LISA_VSHUF4I_W, temp, src, 0x55);
    la_append_ir2_opnd2(LISA_FFINT_D_W, temp, temp);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 1);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp0, 0);
    set_fpu_fcsr(fcsr_opnd);
    return true;
#endif
}

bool translate_cvtps2dq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvtps2dq(pir1);
#else
    /* the low 64 bits of src */
    IR2_OPND dest_lo;
    /* the high 64 bits of src */
    IR2_OPND dest_hi;

    IR2_OPND real_dest;
    IR2_OPND real_src;
 
    real_dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    real_src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    dest_lo = ra_alloc_ftemp();
    dest_hi = ra_alloc_ftemp();

    IR2_OPND temp_int = ra_alloc_itemp();
    IR2_OPND ftemp_over_flow = ra_alloc_ftemp();
    IR2_OPND ftemp_under_flow = ra_alloc_ftemp();
    IR2_OPND label_for_flow1 = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_for_flow2 = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_over = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL); 
    load_ireg_from_imm64(
        temp_int, 0x41dfffffffdffffcULL); /* double for 0x7fffffff.499999 */
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_over_flow, temp_int);
    load_ireg_from_imm64(
        temp_int, 0xc1dfffffffdffffcULL); /* double for 0x80000000.499999 */
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_under_flow, temp_int);
    /* the first single scalar operand */
    /* is unorder or overflow or under flow? */
    IR2_OPND ftemp_src1 = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FCVT_D_S, ftemp_src1, real_src);
    /* is unorder? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src1, ftemp_src1, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow1);
    /* is overflow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_over_flow, ftemp_src1, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow1);
    /* is under flow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src1, ftemp_under_flow, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow1);
    /* is normal */
    la_append_ir2_opnd2(LISA_FTINT_W_D, ftemp_src1,
                     ftemp_src1);         
    la_append_ir2_opnd1(LISA_B, label_high); 

    la_append_ir2_opnd1(LISA_LABEL, label_for_flow1); 
    load_ireg_from_imm32(temp_int, 0x80000000, UNKNOWN_EXTENSION);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_src1, temp_int);
    la_append_ir2_opnd1(LISA_LABEL, label_high); 
    /* the second single scalar operand */
    /* is unorder or overflow or under flow? */
    IR2_OPND ftemp_src2 = ra_alloc_ftemp();
    la_append_ir2_opnd2i(LISA_VEXTRINS_W, ftemp_src2, real_src, 
                        VEXTRINS_IMM_4_0(0, 1));
    la_append_ir2_opnd2(LISA_FCVT_D_S, ftemp_src2, ftemp_src2);
    /* is unorder? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src2, ftemp_src2, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow2);
    /* is overflow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_over_flow, ftemp_src2, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow2);
    /* is under flow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src2, ftemp_under_flow, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow2);
    /* is normal */
    la_append_ir2_opnd2(LISA_FTINT_W_D, ftemp_src2,
                     ftemp_src2);         
    la_append_ir2_opnd1(LISA_B, label_over); 

    la_append_ir2_opnd1(LISA_LABEL, label_for_flow2); 
    load_ireg_from_imm32(temp_int, 0x80000000, UNKNOWN_EXTENSION);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_src2, temp_int);
    la_append_ir2_opnd1(LISA_LABEL, label_over); 
    /* merge */
    la_append_ir2_opnd3(LISA_VILVL_W, dest_lo, ftemp_src2, ftemp_src1);


    IR2_OPND label_for_flow3 = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_for_flow4 = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_over2 = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_high2 = ir2_opnd_new_type(IR2_OPND_LABEL); 
    /* the first single scalar operand */
    /* is unorder or overflow or under flow? */
    la_append_ir2_opnd2i(LISA_VEXTRINS_W, ftemp_src1, real_src, 
                        VEXTRINS_IMM_4_0(0, 2));
    la_append_ir2_opnd2(LISA_FCVT_D_S, ftemp_src1, ftemp_src1);
    /* is unorder? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src1, ftemp_src1, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow3);
    /* is overflow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_over_flow, ftemp_src1, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow3);
    /* is under flow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src1, ftemp_under_flow, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow3);
    /* is normal */
    la_append_ir2_opnd2(LISA_FTINT_W_D, ftemp_src1,
                     ftemp_src1);          
    la_append_ir2_opnd1(LISA_B, label_high2); 

    la_append_ir2_opnd1(LISA_LABEL, label_for_flow3); 
    load_ireg_from_imm32(temp_int, 0x80000000, UNKNOWN_EXTENSION);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_src1, temp_int);
    la_append_ir2_opnd1(LISA_LABEL, label_high2); 
    /* the second single scalar operand */
    /* is unorder or overflow or under flow? */
    la_append_ir2_opnd2i(LISA_VEXTRINS_W, ftemp_src2, real_src, 
                        VEXTRINS_IMM_4_0(0, 3));
    la_append_ir2_opnd2(LISA_FCVT_D_S, ftemp_src2, ftemp_src2);
    /* is unorder? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src2, ftemp_src2, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow4);
    /* is overflow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_over_flow, ftemp_src2, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow4);
    /* is under flow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src2, ftemp_under_flow, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow4);
    /* is normal */
    la_append_ir2_opnd2(LISA_FTINT_W_D, ftemp_src2,
                     ftemp_src2);          
    la_append_ir2_opnd1(LISA_B, label_over2); 

    la_append_ir2_opnd1(LISA_LABEL, label_for_flow4); 
    load_ireg_from_imm32(temp_int, 0x80000000, UNKNOWN_EXTENSION);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_src2, temp_int);
    la_append_ir2_opnd1(LISA_LABEL, label_over2); 
    /* merge */
    la_append_ir2_opnd3(LISA_VILVL_W, dest_hi, ftemp_src2, ftemp_src1);
    la_append_ir2_opnd2i(LISA_VEXTRINS_D, real_dest, dest_lo, VEXTRINS_IMM_4_0(0, 0));
    la_append_ir2_opnd2i(LISA_VEXTRINS_D, real_dest, dest_hi, VEXTRINS_IMM_4_0(1, 0));
    ra_free_temp(ftemp_src1);
    ra_free_temp(ftemp_src2);
    ra_free_temp(ftemp_over_flow);
    ra_free_temp(ftemp_under_flow);
    ra_free_temp(temp_int);
    return true;
#endif
}

bool translate_cvtps2pi(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvtps2pi(pir1);
#else
    tr_x87_to_mmx();

    IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                         true); /* fill default parameter */
    IR2_OPND src_lo;                                         
    src_lo = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp_int = ra_alloc_itemp();
    IR2_OPND ftemp_over_flow = ra_alloc_ftemp();
    IR2_OPND ftemp_under_flow = ra_alloc_ftemp();
    IR2_OPND label_for_flow1 = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_for_flow2 = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_over = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL); 
    load_ireg_from_imm64(
        temp_int, 0x41dfffffffdffffcULL); /* double for 0x7fffffff.499999 */
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_over_flow, temp_int);
    load_ireg_from_imm64(
        temp_int, 0xc1dfffffffdffffcULL); /* double for 0x80000000.499999 */
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_under_flow, temp_int);
    /* the first single scalar operand */
    /* is unorder or overflow or under flow? */
    IR2_OPND ftemp_src1 = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FCVT_D_S, ftemp_src1, src_lo);
    /* is unorder? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src1, ftemp_src1, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow1);
    /* is overflow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_over_flow, ftemp_src1, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow1);
    /* is under flow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src1, ftemp_under_flow, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow1);
    /* is normal */
    la_append_ir2_opnd2(LISA_FTINT_W_D, ftemp_src1,
                     ftemp_src1);         
    la_append_ir2_opnd1(LISA_B, label_high); 

    la_append_ir2_opnd1(LISA_LABEL, label_for_flow1); 
    load_ireg_from_imm32(temp_int, 0x80000000, UNKNOWN_EXTENSION);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_src1, temp_int);
    la_append_ir2_opnd1(LISA_LABEL, label_high); 
    /* the second single scalar operand */
    /* is unorder or overflow or under flow? */
    IR2_OPND ftemp_src2 = ra_alloc_ftemp();
    la_append_ir2_opnd2i(LISA_VEXTRINS_W, ftemp_src2, src_lo, 
                        VEXTRINS_IMM_4_0(0, 1));
    la_append_ir2_opnd2(LISA_FCVT_D_S, ftemp_src2, ftemp_src2);
    /* is unorder? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src2, ftemp_src2, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow2);
    /* is overflow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_over_flow, ftemp_src2, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow2);
    /* is under flow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_src2, ftemp_under_flow, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow2);
    /* is normal */
    la_append_ir2_opnd2(LISA_FTINT_W_D, ftemp_src2,
                     ftemp_src2);         
    la_append_ir2_opnd1(LISA_B, label_over); 

    la_append_ir2_opnd1(LISA_LABEL, label_for_flow2); 
    load_ireg_from_imm32(temp_int, 0x80000000, UNKNOWN_EXTENSION);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_src2, temp_int);
    la_append_ir2_opnd1(LISA_LABEL, label_over); 
    /* merge */
    //la_append_ir2_opnd3(LISA_VILVL_W, dest, ftemp_src1, ftemp_src2);
    la_append_ir2_opnd3(LISA_VILVL_W, dest, ftemp_src2, ftemp_src1);

    ra_free_temp(ftemp_src1);
    ra_free_temp(ftemp_src2);
    ra_free_temp(ftemp_over_flow);
    ra_free_temp(ftemp_under_flow);
    ra_free_temp(temp_int);
    return true;
#endif
}

bool translate_cvtsd2si(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvtsd2si(pir1);
#else
    IR2_OPND src_lo;                                         
    src_lo = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp_int = ra_alloc_itemp();
    IR2_OPND ftemp_over_flow = ra_alloc_ftemp();
    load_ireg_from_imm64(
        temp_int, 0x41dfffffffdffffcULL); /* double for 0x7fffffff.499999 */
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_over_flow, temp_int);
    IR2_OPND ftemp_under_flow = ra_alloc_ftemp();
    load_ireg_from_imm64(
        temp_int, 0xc1dfffffffdffffcULL); /* double for 0x80000000.499999 */
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_under_flow, temp_int);
    IR2_OPND label_for_flow = ir2_opnd_new_type(IR2_OPND_LABEL); 
    /* is unorder? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, src_lo, src_lo, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow);
    /* is overflow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_over_flow, src_lo, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow);
    /* is under flow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, src_lo, ftemp_under_flow, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow);
    /* is normal */
    IR2_OPND fcsr_opnd = set_fpu_fcsr_rounding_field_by_x86();
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64)
        la_append_ir2_opnd2(LISA_FTINT_L_D, ftemp_over_flow, src_lo);
    else
        la_append_ir2_opnd2(LISA_FTINT_W_D, ftemp_over_flow, src_lo);
    la_append_ir2_opnd2(LISA_MOVFR2GR_D, temp_int,
                     ftemp_over_flow); /* temp_int is used for saving */
    /* reload fscr */
    set_fpu_fcsr(fcsr_opnd);

    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);        
    la_append_ir2_opnd1(LISA_B, label_exit);         
    la_append_ir2_opnd1(LISA_LABEL, label_for_flow); 
    load_ireg_from_imm32(temp_int, 0x80000000, UNKNOWN_EXTENSION);
    la_append_ir2_opnd1(LISA_LABEL, label_exit); 
    store_ireg_to_ir1(temp_int, ir1_get_opnd(pir1, 0),
                      false); /* fill default parameter */

    ra_free_temp(temp_int);
    ra_free_temp(ftemp_over_flow);
    ra_free_temp(ftemp_under_flow);
    return true;
#endif
}

bool translate_cvtsd2ss(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvtsd2ss(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND fcsr_opnd = set_fpu_fcsr_rounding_field_by_x86();
    //TODO:simply
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FCVT_S_D, temp, src);
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp, 0);
    set_fpu_fcsr(fcsr_opnd);
    return true;
#endif
}

bool translate_cvtsi2sd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvtsi2sd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND fcsr_opnd = set_fpu_fcsr_rounding_field_by_x86();
    //TODO:simply
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false); 
    IR2_OPND temp_src = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, temp_src, src);
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0) + 1) == 64) {
        la_append_ir2_opnd2(LISA_FFINT_D_L, temp_src, temp_src);
    } else {
        la_append_ir2_opnd2(LISA_FFINT_D_W, temp_src, temp_src);
    }
    la_append_ir2_opnd2i(LISA_VEXTRINS_D, dest, temp_src, VEXTRINS_IMM_4_0(0, 0));
    set_fpu_fcsr(fcsr_opnd);
    return true;
#endif
}

bool translate_cvtsi2ss(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvtsi2ss(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND fcsr_opnd = set_fpu_fcsr_rounding_field_by_x86();
    //TODO:simply
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false); 
    IR2_OPND temp_src = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, temp_src, src);
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0) + 1) == 64) {
        la_append_ir2_opnd2(LISA_FFINT_S_L, temp_src, temp_src);
    } else {
        la_append_ir2_opnd2(LISA_FFINT_S_W, temp_src, temp_src);
    }
    la_append_ir2_opnd2i(LISA_VEXTRINS_W, dest, temp_src, VEXTRINS_IMM_4_0(0, 0));
    set_fpu_fcsr(fcsr_opnd);
    return true;
#endif
}

bool translate_cvtss2sd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvtss2sd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FCVT_D_S, temp, src);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 0);
    return true;
#endif
}

bool translate_cvtss2si(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvtss2si(pir1);
#else
    IR2_OPND src_lo;                                         
    src_lo = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp_src = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FCVT_D_S, temp_src,
        src_lo); /* only use xmm low 32bit */
    IR2_OPND temp_int = ra_alloc_itemp();
    IR2_OPND ftemp_over_flow = ra_alloc_ftemp();
    load_ireg_from_imm64(temp_int,
                         0x41dfffffffe00000ULL); /* double for 0x7fffffff */
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_over_flow, temp_int);
    IR2_OPND ftemp_under_flow = ra_alloc_ftemp();
    load_ireg_from_imm64(temp_int,
                         0xc1e0000000000000ULL); /* double for 0x80000000 */
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_under_flow, temp_int);
    IR2_OPND label_for_flow = ir2_opnd_new_type(IR2_OPND_LABEL); 
    /* is unorder? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, temp_src, temp_src, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow);
    /* is overflow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_over_flow, temp_src, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow);
    /* is under flow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, temp_src, ftemp_under_flow, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow);
    /* is normal ! */
    IR2_OPND fcsr_opnd = set_fpu_fcsr_rounding_field_by_x86();
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64)
        la_append_ir2_opnd2(LISA_FTINT_L_D, ftemp_over_flow, temp_src);
    else
        la_append_ir2_opnd2(LISA_FTINT_W_D, ftemp_over_flow, temp_src);
    la_append_ir2_opnd2(LISA_MOVFR2GR_D, temp_int,
                     ftemp_over_flow); /* temp_int is used for saving */
                                       /* temp_dest */
    /* reload fscr */
    set_fpu_fcsr(fcsr_opnd);

    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);        
    la_append_ir2_opnd1(LISA_B, label_exit);         
    la_append_ir2_opnd1(LISA_LABEL, label_for_flow); 
    load_ireg_from_imm32(temp_int, 0x80000000, UNKNOWN_EXTENSION);
    la_append_ir2_opnd1(LISA_LABEL, label_exit); 
    store_ireg_to_ir1(temp_int, ir1_get_opnd(pir1, 0),
                      false); /* fill default parameter */

    ra_free_temp(temp_int);
    ra_free_temp(temp_src);
    ra_free_temp(ftemp_over_flow);
    ra_free_temp(ftemp_under_flow);
    return true;
#endif
}

bool translate_cvttss2si(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvttss2si(pir1);
#else
    IR2_OPND src_lo;                                         
    src_lo = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp_src = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FCVT_D_S, temp_src,
        src_lo); /* only use xmm low 32bit */
    IR2_OPND temp_over_flow = ra_alloc_itemp();
    IR2_OPND ftemp_over_flow = ra_alloc_ftemp();
    la_append_ir2_opnd2i(LISA_LU52I_D, temp_over_flow, zero_ir2_opnd, 0x41e);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_over_flow, temp_over_flow);

    IR2_OPND temp_under_flow = ra_alloc_itemp();
    IR2_OPND ftemp_under_flow = ra_alloc_ftemp();
    la_append_ir2_opnd2i(LISA_LU52I_D, temp_under_flow, zero_ir2_opnd, 0xc1e);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_under_flow, temp_under_flow);

    /*is unoder?*/
    IR2_OPND label_for_flow = ir2_opnd_new_type(IR2_OPND_LABEL); 
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, temp_src, temp_src, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow);

    /* is overflow? */
    /*is over flow or under flow*/
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_over_flow, temp_src, FCMP_COND_CLE);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow);
    /* is under flow? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, temp_src, ftemp_under_flow, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow);
    /*not over flow and under flow*/
    IR2_OPND temp_fcsr = ra_alloc_itemp();
    /*save fscr in temp_int for reload*/
    la_append_ir2_opnd2(LISA_MOVFCSR2GR, temp_fcsr, fcsr_ir2_opnd);
    la_append_ir2_opnd3_em(LISA_OR, temp_under_flow, temp_under_flow,
                    temp_fcsr); /* temp_under_flow save fcsr */
    /*set fscr for rounding to zero according to x86 operation*/
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, temp_fcsr, zero_ir2_opnd, 9, 8);
    la_append_ir2_opnd2i_em(LISA_ORI, temp_fcsr, temp_fcsr, 0x1<<8);
    la_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr_ir2_opnd, temp_fcsr);

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64)
        la_append_ir2_opnd2(LISA_FTINT_L_D, ftemp_over_flow, temp_src);
    else
        la_append_ir2_opnd2(LISA_FTINT_W_D, ftemp_over_flow, temp_src);

    la_append_ir2_opnd2(LISA_MOVFR2GR_D, temp_fcsr,
        ftemp_over_flow); /* temp_fscr used as temp_dest//replace */
                          /* by shell script */
    /* reload fcsr */
    la_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr_ir2_opnd, temp_under_flow);

    IR2_OPND label_for_exit = ir2_opnd_new_type(IR2_OPND_LABEL);        
    la_append_ir2_opnd1(LISA_B, label_for_exit);     
    la_append_ir2_opnd1(LISA_LABEL, label_for_flow); 
    load_ireg_from_imm32(temp_fcsr, 0x80000000, ZERO_EXTENSION);
    la_append_ir2_opnd1(LISA_LABEL, label_for_exit); 
    store_ireg_to_ir1(temp_fcsr, ir1_get_opnd(pir1, 0),
                      false); /* fill default parameter */

    ra_free_temp(temp_fcsr);
    ra_free_temp(temp_src);
    ra_free_temp(temp_over_flow);
    ra_free_temp(temp_under_flow);
    ra_free_temp(ftemp_over_flow);
    ra_free_temp(ftemp_under_flow);
    return true;
#endif
}

bool translate_cvtps2pd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvtps2pd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    //TODO:simply
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    IR2_OPND temp0 = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FCVT_D_S, temp0, src);
    la_append_ir2_opnd2i(LISA_VSHUF4I_W, temp, src, 0x55);
    la_append_ir2_opnd2(LISA_FCVT_D_S, temp, temp);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 1);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp0, 0);
    return true;
#endif
}

bool translate_cvttsd2si(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cvttsd2si(pir1);
#else
    IR2_OPND src_lo = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp_over_flow = ra_alloc_itemp();
    IR2_OPND ftemp_over_flow = ra_alloc_ftemp();
    la_append_ir2_opnd2i(LISA_LU52I_D, temp_over_flow, zero_ir2_opnd, 0x41e);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_over_flow, temp_over_flow);

    IR2_OPND temp_under_flow = ra_alloc_itemp();
    IR2_OPND ftemp_under_flow = ra_alloc_ftemp();
    la_append_ir2_opnd2i(LISA_LU52I_D, temp_under_flow, zero_ir2_opnd, 0xc1e);
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp_under_flow, temp_under_flow);

    /*is unoder?*/
    IR2_OPND label_for_flow = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, src_lo, src_lo, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow);

    /*is over flow or under flow*/
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, ftemp_over_flow, src_lo, FCMP_COND_CLE);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow);
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, src_lo, ftemp_under_flow, FCMP_COND_CLE);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_flow);
    /*not over flow and under flow*/
    IR2_OPND temp_fcsr = ra_alloc_itemp();
    /*save fscr in temp_int for reload*/
    la_append_ir2_opnd2_em(LISA_MOVFCSR2GR, temp_fcsr, fcsr_ir2_opnd);
    la_append_ir2_opnd3_em(LISA_OR, temp_under_flow, temp_under_flow,
                     temp_fcsr); /* temp_under_flow save fcsr */
    /*set fscr for rounding to zero according to x86 operation*/
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, temp_fcsr, zero_ir2_opnd, 9, 8);
    la_append_ir2_opnd2i_em(LISA_ORI, temp_fcsr, temp_fcsr, 0x1<<8);
    la_append_ir2_opnd2_em(LISA_MOVGR2FCSR, fcsr_ir2_opnd, temp_fcsr);

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
        la_append_ir2_opnd2(LISA_FTINT_L_D, ftemp_over_flow, src_lo);
    } else {
        la_append_ir2_opnd2(LISA_FTINT_W_D, ftemp_over_flow, src_lo);
    }

    la_append_ir2_opnd2(LISA_MOVFR2GR_D, temp_fcsr,
                     ftemp_over_flow); /* temp_fscr used as temp_dest */
    /* reload fcsr */
    la_append_ir2_opnd2_em(LISA_MOVGR2FCSR, fcsr_ir2_opnd, temp_under_flow);

    IR2_OPND label_for_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd1(LISA_B, label_for_exit);
    la_append_ir2_opnd1(LISA_LABEL, label_for_flow);
    load_ireg_from_imm32(temp_fcsr, 0x80000000, ZERO_EXTENSION);
    la_append_ir2_opnd1(LISA_LABEL, label_for_exit);
    store_ireg_to_ir1(temp_fcsr, ir1_get_opnd(pir1, 0), false);

    ra_free_temp(temp_fcsr);
    ra_free_temp(temp_over_flow);
    ra_free_temp(temp_under_flow);
    ra_free_temp(ftemp_over_flow);
    ra_free_temp(ftemp_under_flow);

    return true;
#endif
}
