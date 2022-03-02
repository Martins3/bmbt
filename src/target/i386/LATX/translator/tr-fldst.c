#include <math.h>
#include "common.h"
#include "reg-alloc.h"
#include "lsenv.h"
#include "latx-options.h"
#include "translate.h"

/* TODO: long_double */
/* bool is_long_double_ir1_opnd(IR1_OPND* opnd){ */
/*     if(!OPTIONS::long_double()) */
/*         return false; */
/*     if(opnd->size()==80 || opnd->size() == 128) */
/*         return true; */
/*     return false; */
/* } */

bool translate_fld(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fld(pir1);
#else
    /* TODO: long_double */
    /* if(is_long_double_ir1_opnd(ir1_get_opnd(pir1, 0))){ */
    /*     fprintf(stderr, "Long double for %s not implemented. translation */
    /*     failed.\n", __FUNCTION__); return false; */
    /* } */

    /* 1. the position to be overwritten is st(7) at this time */
    IR2_OPND dest_opnd = ra_alloc_st(7);

    /* 2. load the value */
    load_freg_from_ir1_2(dest_opnd, ir1_get_opnd(pir1, 0), IS_CONVERT);

    /* 3. adjust top */
    tr_fpu_push();

    return true;
#endif
}

bool translate_fldz(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fldz(pir1);
#else
    /* 1. the position to be overwritten is st(0) */
    tr_fpu_push();
    IR2_OPND dest_opnd = ra_alloc_st(0);

    /* 2. load the value */

    la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, dest_opnd, zero_ir2_opnd);
    la_append_ir2_opnd2_em(LISA_FFINT_D_W, dest_opnd, dest_opnd);

    return true;
#endif
}

bool translate_fld1(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fld1(pir1);
#else
    /* 1. the position to be overwritten is st(0) */
    tr_fpu_push();
    IR2_OPND dest_opnd = ra_alloc_st(0);

    /* 2. load the value */
    IR2_OPND itemp_1 = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ORI, itemp_1, zero_ir2_opnd, 1);
    la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, dest_opnd, itemp_1);
    la_append_ir2_opnd2_em(LISA_FFINT_D_W, dest_opnd, dest_opnd);

    ra_free_temp(itemp_1);
    return true;
#endif
}

static double l2e;
bool translate_fldl2e(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    (void)l2e; /* to avoid compile warning */
    return latxs_translate_fldl2e(pir1);
#else
    l2e = 1 / log(2.0);
    /* 1. the position to be overwritten is st(0) */
    tr_fpu_push();
    IR2_OPND dest_opnd = ra_alloc_st(0);

    /* 2. load the value */
    IR2_OPND value_addr_opnd = convert_mem_addr((ADDR)&l2e);
    la_append_ir2_opnd2i(LISA_FLD_D, dest_opnd, value_addr_opnd, 0);

    return true;
#endif
}

static double l2t;
bool translate_fldl2t(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    (void)l2t; /* to avoid compile warning */
    return latxs_translate_fldl2t(pir1);
#else
    l2t = 1 / log10(2.0);

    /* 1. the position to be overwritten is st(0) */
    tr_fpu_push();
    IR2_OPND dest_opnd = ra_alloc_st(0);

    /* 2. load the value */
    IR2_OPND value_addr_opnd = convert_mem_addr((ADDR)&l2t);
    la_append_ir2_opnd2i(LISA_FLD_D, dest_opnd, value_addr_opnd, 0);

    return true;
#endif
}

static double lg2;
bool translate_fldlg2(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    (void)lg2; /* to avoid compile warning */
    return latxs_translate_fldlg2(pir1);
#else
    lg2 = log10(2.0);

    /* 1. the position to be overwritten is st(0) */
    tr_fpu_push();
    IR2_OPND dest_opnd = ra_alloc_st(0);

    /* 2. load the value */
    IR2_OPND value_addr_opnd = convert_mem_addr((ADDR)&lg2);
    la_append_ir2_opnd2i(LISA_FLD_D, dest_opnd, value_addr_opnd, 0);

    return true;
#endif
}

static double ln2;
bool translate_fldln2(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    (void)ln2; /* to avoid compile warning */
    return latxs_translate_fldln2(pir1);
#else
    ln2 = log(2.0);

    /* 1. the position to be overwritten is st(0) */
    tr_fpu_push();
    IR2_OPND dest_opnd = ra_alloc_st(0);

    /* 2. load the value */
    IR2_OPND value_addr_opnd = convert_mem_addr((ADDR)&ln2);
    la_append_ir2_opnd2i(LISA_FLD_D, dest_opnd, value_addr_opnd, 0);

    return true;
#endif
}

static double pi = M_PI;
bool translate_fldpi(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    (void)pi; /* to avoid compile warning */
    return latxs_translate_fldpi(pir1);
#else
    /* 1. the position to be overwritten is st(0) */
    tr_fpu_push();
    IR2_OPND dest_opnd = ra_alloc_st(0);

    /* 2. load the value */
    IR2_OPND value_addr_opnd = convert_mem_addr((ADDR)&pi);
    la_append_ir2_opnd2i(LISA_FLD_D, dest_opnd, value_addr_opnd, 0);

    return true;
#endif
}

bool translate_fstp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fstp(pir1);
#else
    /* TODO: long_double */
    /* if(is_long_double_ir1_opnd(ir1_get_opnd(pir1, 0))){ */
    /*     fprintf(stderr, "Long double for %s not implemented. translation */
    /*     failed.\n", __FUNCTION__); return false; */
    /* } */

    /* 1. load value from st(0) */
    IR2_OPND src_opnd = ra_alloc_st(0);

    /* 2. write to target */
    store_freg_to_ir1(src_opnd, ir1_get_opnd(pir1, 0), false, true);

    /* 3. adjust top */
    tr_fpu_pop();

    return true;
#endif
}

bool translate_fst(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fst(pir1);
#else
    /* if(is_long_double_ir1_opnd(ir1_get_opnd(pir1, 0))){ */
    /*     fprintf(stderr, "Long double for %s not implemented. translation */
    /*     failed.\n", __FUNCTION__); return false; */
    /* } */

    /* 1. load value from st(0) */
    IR2_OPND src_opnd = ra_alloc_st(0);

    /* 2. write to target */
    store_freg_to_ir1(src_opnd, ir1_get_opnd(pir1, 0), false, true);

    return true;
#endif
}

bool translate_fild(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fild(pir1);
#else
    lsassert(ir1_get_opnd_num(pir1) == 1);

    tr_fpu_push();

    IR2_OPND dest_opnd = ra_alloc_st(0);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    load_freg_from_ir1_2(dest_opnd, opnd0, 0);
    if (ir1_opnd_size(opnd0) > 32) {
        la_append_ir2_opnd2(LISA_FFINT_D_L, dest_opnd, dest_opnd);
    } else {
        la_append_ir2_opnd2(LISA_FFINT_D_W, dest_opnd, dest_opnd);
    }

    return true;
#endif
}

bool translate_fist(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fist(pir1);
#else
    /* 1. load rounding_mode from x86_control_word to mips_fcsr */
    IR2_OPND tmp_fcsr = ra_alloc_itemp();
    la_append_ir2_opnd2(LISA_MOVFCSR2GR, tmp_fcsr, fcsr_ir2_opnd);

    lsassert(ir1_get_opnd_num(pir1) == 1);

    IR2_OPND dest_opnd = ra_alloc_st(0);

    IR2_OPND dest_int = ra_alloc_itemp();

    /* first, we should make sure if dest_opnd is unordered? overflow? */
    /* underflow? */
    uint64 low_bound =0;
    uint64 high_bound = 0;
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
    load_ireg_from_imm64(bounder_opnd,
                         high_bound); /* double for 0x7fff+0.499999 */
    IR2_OPND f_high_bounder_opnd = ra_alloc_ftemp();
    la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, f_high_bounder_opnd, bounder_opnd);

    load_ireg_from_imm64(bounder_opnd,
                         low_bound); /* double for 0x8000+(-0.499999) */
    IR2_OPND f_low_bounder_opnd = ra_alloc_ftemp();
    la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, f_low_bounder_opnd, bounder_opnd);
    ra_free_temp(bounder_opnd);

    /*is unorder? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, dest_opnd, dest_opnd, FCMP_COND_CUN);
    IR2_OPND label_flow = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_flow);

    /* is underflow or overflow? */
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) != 64) {

        la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, f_high_bounder_opnd, dest_opnd, FCMP_COND_CLE);
        la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_flow);
        la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, dest_opnd, f_low_bounder_opnd, FCMP_COND_CLE);
        la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_flow);
    } else {

        la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, f_high_bounder_opnd, dest_opnd, FCMP_COND_CLT);
        la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_flow);
        la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, dest_opnd, f_low_bounder_opnd, FCMP_COND_CLT);
        la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_flow);
    }
    ra_free_temp(f_low_bounder_opnd);
    ra_free_temp(f_high_bounder_opnd);
    /* not unorder or flow */
    IR2_OPND fp_opnd = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FTINT_L_D, fp_opnd, dest_opnd);
    la_append_ir2_opnd2_em(LISA_MOVFR2GR_D, dest_int, fp_opnd);
    ra_free_temp(fp_opnd);
    IR2_OPND label_end = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd1(LISA_B, label_end);

    /*overflow, mov ox8000 or 0x80000000 to dest*/
    la_append_ir2_opnd1(LISA_LABEL, label_flow);
    switch (ir1_opnd_size(ir1_get_opnd(pir1, 0))) {
    case 16:
        load_ireg_from_imm32(dest_int, 0x8000, SIGN_EXTENSION);
        break;
    case 32:
        load_ireg_from_imm32(dest_int, 0x80000000, SIGN_EXTENSION);
        break;
    case 64:
        load_ireg_from_imm64(dest_int, 0x8000000000000000ull);
        break;
    default:
        lsassertm(0, "Wrong opnd size : %d!\n", ir1_opnd_size(ir1_get_opnd(pir1, 0)));
    }

    la_append_ir2_opnd1(LISA_LABEL, label_end);
    store_ireg_to_ir1(dest_int, ir1_get_opnd(pir1, 0), false);
    ra_free_temp(dest_int);

    la_append_ir2_opnd2_em(LISA_MOVGR2FCSR, fcsr_ir2_opnd, tmp_fcsr);
    ra_free_temp(tmp_fcsr);

    return true;
#endif
}

bool translate_fistp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fistp(pir1);
#else
    translate_fist(pir1);
    tr_fpu_pop();
    return true;
#endif
}

bool translate_fnstsw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fnstsw(pir1);
#else
    lsassert(ir1_get_opnd_num(pir1) == 1);

    IR2_OPND sw_value = ra_alloc_itemp();

   /*
    * NOTE: Refer to X86 Docs, there is m2bytes
    * condition ONLY, TCG hardcode write 16bit as well.
    * So change the opnd size to 16 bit to avoid data
    * overwtite.
    */
    IR1_OPND *ir1_opnd = ir1_get_opnd(pir1, 0);
    int opnd_size = ir1_opnd_size(ir1_opnd);

    if (opnd_size == 32) {
        ir1_opnd->size = (16 >> 3);
     }

    update_sw_by_fcsr(sw_value);

    /* 2. store the current value of status_word to dest_opnd */
    store_ireg_to_ir1(sw_value, ir1_get_opnd(pir1, 0), false);

    /* 3. free tmp */
    ra_free_temp(sw_value);

    return true;
#endif
}

bool translate_fstsw(IR1_INST *pir1) { return translate_fnstsw(pir1); }
