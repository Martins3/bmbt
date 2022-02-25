#include <math.h>
#include "common.h"
#include "reg-alloc.h"
#include "lsenv.h"
#include "latx-options.h"
#include "translate.h"

bool translate_fadd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fadd(pir1);
#else
    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);
    if (opnd_num == 1) {
        IR2_OPND st0_opnd = ra_alloc_st(0);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        la_append_ir2_opnd3(LISA_FADD_D, st0_opnd, st0_opnd, src1_opnd);
    } else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        la_append_ir2_opnd3(LISA_FADD_D, dest_opnd, dest_opnd, src1_opnd);
    }

    return true;
#endif
}

bool translate_faddp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_faddp(pir1);
#else
    int opnd_num = ir1_opnd_num(pir1);
    //lsassert(opnd_num == 0 || opnd_num == 2); //capstone may has only one opnd 0xde0xc1

    if (opnd_num == 0) {
        IR2_OPND st0 = ra_alloc_st(0);
        IR2_OPND st1 = ra_alloc_st(1);
        la_append_ir2_opnd3(LISA_FADD_D, st1, st0, st1);
    } 
    else if(opnd_num == 1){
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND st0 = ra_alloc_st(0);
        la_append_ir2_opnd3(LISA_FADD_D, dest_opnd, dest_opnd, st0);
    }
    else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        la_append_ir2_opnd3(LISA_FADD_D, dest_opnd, dest_opnd, src1_opnd);
    }
    tr_fpu_pop();

    return true;
#endif
}

bool translate_fiadd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fiadd(pir1);
#else
    lsassert(ir1_opnd_num(pir1) == 1);

    IR2_OPND st0 = ra_alloc_st(0);
    IR2_OPND t_freg = ra_alloc_ftemp();
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    load_freg_from_ir1_2(t_freg, opnd0, 0);
    if (ir1_opnd_size(opnd0) > 32) {
        la_append_ir2_opnd2(LISA_FFINT_D_L , t_freg, t_freg);
    } else {
        la_append_ir2_opnd2(LISA_FFINT_D_W, t_freg, t_freg);
    }

    la_append_ir2_opnd3(LISA_FADD_D, st0, t_freg, st0);
    ra_free_temp(t_freg);
    return true;
#endif
}

bool translate_fsub(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fsub(pir1);
#else
    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    if (opnd_num == 1) {
        IR2_OPND st0_opnd = ra_alloc_st(0);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        la_append_ir2_opnd3(LISA_FSUB_D, st0_opnd, st0_opnd, src1_opnd);
    } else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        la_append_ir2_opnd3(LISA_FSUB_D, dest_opnd, dest_opnd, src1_opnd);
    }

    return true;
#endif
}

bool translate_fisub(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fisub(pir1);
#else
    lsassert(ir1_opnd_num(pir1) == 1);

    IR2_OPND st0 = ra_alloc_st(0);
    IR2_OPND t_freg = ra_alloc_ftemp();
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    load_freg_from_ir1_2(t_freg, opnd0, 0);
    if (ir1_opnd_size(opnd0) > 32) {
        la_append_ir2_opnd2(LISA_FFINT_D_L , t_freg, t_freg);
    } else {
        la_append_ir2_opnd2(LISA_FFINT_D_W , t_freg, t_freg);
    }

    la_append_ir2_opnd3(LISA_FSUB_D, st0, st0, t_freg);
    ra_free_temp(t_freg);
    return true;
#endif
}

bool translate_fisubr(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fisubr(pir1);
#else
    lsassert(ir1_opnd_num(pir1) == 1);

    IR2_OPND st0 = ra_alloc_st(0);
    IR2_OPND t_freg = ra_alloc_ftemp();
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    load_freg_from_ir1_2(t_freg, opnd0, 0);
    if (ir1_opnd_size(opnd0) > 32) {
        la_append_ir2_opnd2(LISA_FFINT_D_L , t_freg, t_freg);
    } else {
        la_append_ir2_opnd2(LISA_FFINT_D_W , t_freg, t_freg);
    }

    la_append_ir2_opnd3(LISA_FSUB_D, st0, t_freg, st0);
    ra_free_temp(t_freg);
    return true;
#endif
}

bool translate_fsubr(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fsubr(pir1);
#else
    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    if (opnd_num == 1) {
        IR2_OPND st0_opnd = ra_alloc_st(0);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        la_append_ir2_opnd3(LISA_FSUB_D, st0_opnd, src1_opnd, st0_opnd);
    } else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        la_append_ir2_opnd3(LISA_FSUB_D, dest_opnd, src1_opnd, dest_opnd);
    }

    return true;
#endif
}

bool translate_fsubrp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fsubrp(pir1);
#else
    int opnd_num = ir1_opnd_num(pir1);
    //lsassert(opnd_num == 0 || opnd_num == 2);

    if (opnd_num == 0) {
        IR2_OPND st0 = ra_alloc_st(0);
        IR2_OPND st1 = ra_alloc_st(1);
        la_append_ir2_opnd3(LISA_FSUB_D, st1, st0, st1);
    }
    else if(opnd_num == 1){
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND st0 = ra_alloc_st(0);
        la_append_ir2_opnd3(LISA_FSUB_D, dest_opnd, st0, dest_opnd);
    }    
    else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        la_append_ir2_opnd3(LISA_FSUB_D, dest_opnd, src1_opnd, dest_opnd);
    }
    tr_fpu_pop();
    return true;
#endif
}

bool translate_fsubp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fsubp(pir1);
#else
    int opnd_num = ir1_opnd_num(pir1);
//    lsassert(opnd_num == 0 || opnd_num == 2);

    if (opnd_num == 0) {
        IR2_OPND st0 = ra_alloc_st(0);
        IR2_OPND st1 = ra_alloc_st(1);
        la_append_ir2_opnd3(LISA_FSUB_D, st1, st1, st0);
    }
    else if(opnd_num == 1){
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND st0 = ra_alloc_st(0);
        la_append_ir2_opnd3(LISA_FSUB_D, dest_opnd, dest_opnd, st0);
    }    
    else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        la_append_ir2_opnd3(LISA_FSUB_D, dest_opnd, dest_opnd, src1_opnd);
    }
    tr_fpu_pop();
    return true;
#endif
}

bool translate_fmul(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fmul(pir1);
#else
    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    if (opnd_num == 1) {
        IR2_OPND st0_opnd = ra_alloc_st(0);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        la_append_ir2_opnd3(LISA_FMUL_D, st0_opnd, st0_opnd, src1_opnd);
    } else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        la_append_ir2_opnd3(LISA_FMUL_D, dest_opnd, dest_opnd, src1_opnd);
    }

    return true;
#endif
}

bool translate_fimul(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fimul(pir1);
#else
    lsassert(ir1_opnd_num(pir1) == 1);

    IR2_OPND st0 = ra_alloc_st(0);
    IR2_OPND t_freg = ra_alloc_ftemp();
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    load_freg_from_ir1_2(t_freg, opnd0, 0);
    if (ir1_opnd_size(opnd0) > 32) {
        la_append_ir2_opnd2(LISA_FFINT_D_L, t_freg, t_freg);
    } else {
        la_append_ir2_opnd2(LISA_FFINT_D_W, t_freg, t_freg);
    }

    la_append_ir2_opnd3(LISA_FMUL_D, st0, t_freg, st0);
    ra_free_temp(t_freg);
    return true;
#endif
}

bool translate_fmulp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fmulp(pir1);
#else
    int opnd_num = ir1_opnd_num(pir1);
    //lsassert(opnd_num == 0 || opnd_num == 2);

    if (opnd_num == 0) {
        IR2_OPND st0 = ra_alloc_st(0);
        IR2_OPND st1 = ra_alloc_st(1);
        la_append_ir2_opnd3(LISA_FMUL_D, st1, st0, st1);
    }
    else if(opnd_num == 1){
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND st0 = ra_alloc_st(0);
        la_append_ir2_opnd3(LISA_FMUL_D, dest_opnd, dest_opnd, st0);
    }    
    else {
        IR2_OPND src0_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        la_append_ir2_opnd3(LISA_FMUL_D, src0_opnd, src0_opnd, src1_opnd);
    }
    tr_fpu_pop();

    return true;
#endif
}

bool translate_fdiv(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fdiv(pir1);
#else
    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    if (opnd_num == 1) {
        IR2_OPND st0_opnd = ra_alloc_st(0);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        la_append_ir2_opnd3(LISA_FDIV_D, st0_opnd, st0_opnd, src1_opnd);
    } else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        la_append_ir2_opnd3(LISA_FDIV_D, dest_opnd, dest_opnd, src1_opnd);
    }

    /* get status word and load in sw_value */
    IR2_OPND sw_value = ra_alloc_itemp();
    IR2_OPND fcsr0 = ra_alloc_itemp();
    IR2_OPND temp1 = ra_alloc_itemp();
    int offset = lsenv_offset_of_status_word(lsenv);
    assert(offset < 0x7ff);
    la_append_ir2_opnd2i_em(LISA_LD_HU, sw_value, env_ir2_opnd, offset);
    la_append_ir2_opnd2_em(LISA_MOVFCSR2GR, fcsr0, fcsr_ir2_opnd);
    /* divide-by-zero exception */
    IR2_OPND label_z = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd2i(LISA_SRLI_W, temp1, fcsr0, FCSR_OFF_CAUSE_Z);
    la_append_ir2_opnd2i_em(LISA_ANDI, temp1, temp1, 0x1);
    la_append_ir2_opnd3(LISA_BEQ, temp1, zero_ir2_opnd, label_z);
    la_append_ir2_opnd2i_em(LISA_ORI, temp1, zero_ir2_opnd, 0x1 << X87_SR_OFF_ZE);
    la_append_ir2_opnd3_em(LISA_OR, sw_value, temp1, sw_value);
    la_append_ir2_opnd1(LISA_LABEL, label_z);
    /*  invalid-arithmetic-operand exception */
    IR2_OPND label_i = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd2i(LISA_SRLI_W, temp1, fcsr0, FCSR_OFF_CAUSE_V);
    la_append_ir2_opnd2i_em(LISA_ANDI, temp1, temp1, 0x1);
    la_append_ir2_opnd3(LISA_BEQ, temp1, zero_ir2_opnd, label_i);
    la_append_ir2_opnd2i_em(LISA_ORI, temp1, zero_ir2_opnd, 0x1 << X87_SR_OFF_IE);
    la_append_ir2_opnd3_em(LISA_OR, sw_value, temp1, sw_value);
    la_append_ir2_opnd1(LISA_LABEL, label_i);
    la_append_ir2_opnd2i(LISA_ST_H, sw_value, env_ir2_opnd,
                        lsenv_offset_of_status_word(lsenv));

    return true;
#endif
}
bool translate_fdivr(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fdivr(pir1);
#else
    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    if (opnd_num == 1) {
        IR2_OPND st0_opnd = ra_alloc_st(0);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        la_append_ir2_opnd3(LISA_FDIV_D, st0_opnd, src1_opnd, st0_opnd);
    } else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        la_append_ir2_opnd3(LISA_FDIV_D, dest_opnd, src1_opnd, dest_opnd);
    }

    /* get status word and load in sw_value */
    IR2_OPND sw_value = ra_alloc_itemp();
    IR2_OPND fcsr0 = ra_alloc_itemp();
    IR2_OPND temp1 = ra_alloc_itemp();
    int offset = lsenv_offset_of_status_word(lsenv);
    assert(offset < 0x7ff);
    la_append_ir2_opnd2i_em(LISA_LD_HU, sw_value, env_ir2_opnd, offset);
    la_append_ir2_opnd2_em(LISA_MOVFCSR2GR, fcsr0, fcsr_ir2_opnd);
    /* divide-by-zero exception */
    IR2_OPND label_z = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd2i(LISA_SRLI_W, temp1, fcsr0, FCSR_OFF_CAUSE_Z);
    la_append_ir2_opnd2i_em(LISA_ANDI, temp1, temp1, 0x1);
    la_append_ir2_opnd3(LISA_BEQ, temp1, zero_ir2_opnd, label_z);
    la_append_ir2_opnd2i_em(LISA_ORI, temp1, zero_ir2_opnd, 0x1 << X87_SR_OFF_ZE);
    la_append_ir2_opnd3_em(LISA_OR, sw_value, temp1, sw_value);
    la_append_ir2_opnd1(LISA_LABEL, label_z);
    /*  invalid-arithmetic-operand exception */
    IR2_OPND label_i = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd2i(LISA_SRLI_W, temp1, fcsr0, FCSR_OFF_CAUSE_V);
    la_append_ir2_opnd2i_em(LISA_ANDI, temp1, temp1, 0x1);
    la_append_ir2_opnd3(LISA_BEQ, temp1, zero_ir2_opnd, label_i);
    la_append_ir2_opnd2i_em(LISA_ORI, temp1, zero_ir2_opnd, 0x1 << X87_SR_OFF_IE);
    la_append_ir2_opnd3_em(LISA_OR, sw_value, temp1, sw_value);
    la_append_ir2_opnd1(LISA_LABEL, label_i);
    la_append_ir2_opnd2i(LISA_ST_H, sw_value, env_ir2_opnd,
                        lsenv_offset_of_status_word(lsenv));

    return true;
#endif
}

bool translate_fidiv(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fidiv(pir1);
#else
    lsassert(ir1_opnd_num(pir1) == 1);

    IR2_OPND st0 = ra_alloc_st(0);
    IR2_OPND t_freg = ra_alloc_ftemp();
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    load_freg_from_ir1_2(t_freg, opnd0, 0);
    if (ir1_opnd_size(opnd0) > 32) {
        la_append_ir2_opnd2(LISA_FFINT_D_L, t_freg, t_freg);
    } else {
        la_append_ir2_opnd2(LISA_FFINT_D_W, t_freg, t_freg);
    }

    la_append_ir2_opnd3(LISA_FDIV_D, st0, st0, t_freg);
    ra_free_temp(t_freg);
    return true;
#endif
}
bool translate_fidivr(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fidivr(pir1);
#else
    lsassert(ir1_opnd_num(pir1) == 1);

    IR2_OPND st0 = ra_alloc_st(0);
    IR2_OPND t_freg = ra_alloc_ftemp();
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    load_freg_from_ir1_2(t_freg, opnd0, 0);
    if (ir1_opnd_size(opnd0) > 32) {
        la_append_ir2_opnd2(LISA_FFINT_D_L, t_freg, t_freg);
    } else {
        la_append_ir2_opnd2(LISA_FFINT_D_W, t_freg, t_freg);
    }

    la_append_ir2_opnd3(LISA_FDIV_D, st0, t_freg, st0);
    ra_free_temp(t_freg);
    return true;
#endif
}

bool translate_fdivrp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fdivrp(pir1);
#else
    int opnd_num = ir1_opnd_num(pir1);
    //lsassert(opnd_num == 0 || opnd_num == 2);

    if (opnd_num == 0) {
        IR2_OPND st0 = ra_alloc_st(0);
        IR2_OPND st1 = ra_alloc_st(1);
        la_append_ir2_opnd3(LISA_FDIV_D, st1, st0, st1);
    }
    else if(opnd_num == 1){
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND st0 = ra_alloc_st(0);
        la_append_ir2_opnd3(LISA_FDIV_D, dest_opnd, st0, dest_opnd);
    }    
    else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        la_append_ir2_opnd3(LISA_FDIV_D, dest_opnd, src1_opnd, dest_opnd);
    }
    tr_fpu_pop();

    /* get status word and load in sw_value */
    IR2_OPND sw_value = ra_alloc_itemp();
    IR2_OPND fcsr0 = ra_alloc_itemp();
    IR2_OPND temp1 = ra_alloc_itemp();
    int offset = lsenv_offset_of_status_word(lsenv);
    assert(offset < 0x7ff);
    la_append_ir2_opnd2i_em(LISA_LD_HU, sw_value, env_ir2_opnd, offset);
    la_append_ir2_opnd2_em(LISA_MOVFCSR2GR, fcsr0, fcsr_ir2_opnd);
    /* divide-by-zero exception */
    IR2_OPND label_z = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd2i(LISA_SRLI_W, temp1, fcsr0, FCSR_OFF_CAUSE_Z);
    la_append_ir2_opnd2i_em(LISA_ANDI, temp1, temp1, 0x1);
    la_append_ir2_opnd3(LISA_BEQ, temp1, zero_ir2_opnd, label_z);
    la_append_ir2_opnd2i_em(LISA_ORI, temp1, zero_ir2_opnd, 0x1 << X87_SR_OFF_ZE);
    la_append_ir2_opnd3_em(LISA_OR, sw_value, temp1, sw_value);
    la_append_ir2_opnd1(LISA_LABEL, label_z);
    /*  invalid-arithmetic-operand exception */
    IR2_OPND label_i = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd2i(LISA_SRLI_W, temp1, fcsr0, FCSR_OFF_CAUSE_V);
    la_append_ir2_opnd2i_em(LISA_ANDI, temp1, temp1, 0x1);
    la_append_ir2_opnd3(LISA_BEQ, temp1, zero_ir2_opnd, label_i);
    la_append_ir2_opnd2i_em(LISA_ORI, temp1, zero_ir2_opnd, 0x1 << X87_SR_OFF_IE);
    la_append_ir2_opnd3_em(LISA_OR, sw_value, temp1, sw_value);
    la_append_ir2_opnd1(LISA_LABEL, label_i);
    la_append_ir2_opnd2i(LISA_ST_H, sw_value, env_ir2_opnd,
                        lsenv_offset_of_status_word(lsenv));

    return true;
#endif
}
bool translate_fdivp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fdivp(pir1);
#else
    int opnd_num = ir1_opnd_num(pir1);
    //lsassert(opnd_num == 0 || opnd_num == 2);

    if (opnd_num == 0) {
        IR2_OPND st0 = ra_alloc_st(0);
        IR2_OPND st1 = ra_alloc_st(1);
        la_append_ir2_opnd3(LISA_FDIV_D, st1, st1, st0);
    } 
    else if(opnd_num == 1){
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND st0 = ra_alloc_st(0);
        la_append_ir2_opnd3(LISA_FDIV_D, dest_opnd, dest_opnd, st0);
    }
    else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        la_append_ir2_opnd3(LISA_FDIV_D, dest_opnd, dest_opnd, src1_opnd);
    }
    tr_fpu_pop();

    /* get status word and load in sw_value */
    IR2_OPND sw_value = ra_alloc_itemp();
    IR2_OPND fcsr0 = ra_alloc_itemp();
    IR2_OPND temp1 = ra_alloc_itemp();
    int offset = lsenv_offset_of_status_word(lsenv);
    assert(offset < 0x7ff);
    la_append_ir2_opnd2i_em(LISA_LD_HU, sw_value, env_ir2_opnd, offset);
    la_append_ir2_opnd2_em(LISA_MOVFCSR2GR, fcsr0, fcsr_ir2_opnd);
    /* divide-by-zero exception */
    IR2_OPND label_z = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd2i(LISA_SRLI_W, temp1, fcsr0, FCSR_OFF_CAUSE_Z);
    la_append_ir2_opnd2i_em(LISA_ANDI, temp1, temp1, 0x1);
    la_append_ir2_opnd3(LISA_BEQ, temp1, zero_ir2_opnd, label_z);
    la_append_ir2_opnd2i_em(LISA_ORI, temp1, zero_ir2_opnd, 0x1 << X87_SR_OFF_ZE);
    la_append_ir2_opnd3_em(LISA_OR, sw_value, temp1, sw_value);
    la_append_ir2_opnd1(LISA_LABEL, label_z);
    /*  invalid-arithmetic-operand exception */
    IR2_OPND label_i = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd2i(LISA_SRLI_W, temp1, fcsr0, FCSR_OFF_CAUSE_V);
    la_append_ir2_opnd2i_em(LISA_ANDI, temp1, temp1, 0x1);
    la_append_ir2_opnd3(LISA_BEQ, temp1, zero_ir2_opnd, label_i);
    la_append_ir2_opnd2i_em(LISA_ORI, temp1, zero_ir2_opnd, 0x1 << X87_SR_OFF_IE);
    la_append_ir2_opnd3_em(LISA_OR, sw_value, temp1, sw_value);
    la_append_ir2_opnd1(LISA_LABEL, label_i);
    la_append_ir2_opnd2i(LISA_ST_H, sw_value, env_ir2_opnd,
                        lsenv_offset_of_status_word(lsenv));

    return true;
#endif
}

bool translate_fnop(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fnop(pir1);
#else
    /* append_ir2_opnd0(mips_nop); */
    return true;
#endif
}

bool translate_fsqrt(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fsqrt(pir1);
#else
    IR2_OPND st0_opnd = ra_alloc_st(0);
    la_append_ir2_opnd3(LISA_FSQRT_D, st0_opnd, st0_opnd, st0_opnd);

    return true;
#endif
}

bool translate_fabs(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fabs(pir1);
#else
    IR2_OPND st0_opnd = ra_alloc_st(0);
    la_append_ir2_opnd3(LISA_FABS_D, st0_opnd, st0_opnd, st0_opnd);

    return true;
#endif
}

bool translate_fchs(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fchs(pir1);
#else
    IR2_OPND st0_opnd = ra_alloc_st(0);
    la_append_ir2_opnd3(LISA_FNEG_D, st0_opnd, st0_opnd, st0_opnd);

    return true;
#endif
}

bool translate_fdecstp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fdecstp(pir1);
#else
    tr_fpu_dec();

    /* decrements the top-of-stack pointer */
    IR2_OPND value_status = ra_alloc_itemp();
    IR2_OPND value_status_temp = ra_alloc_itemp();

    la_append_ir2_opnd2i_em(LISA_LD_H, value_status, env_ir2_opnd,
                      lsenv_offset_of_status_word(lsenv)); /* status_word */
    if (!option_lsfpu) {
        la_append_ir2_opnd2ii(LISA_BSTRPICK_D, value_status_temp, value_status, 13, 11);
        IR2_OPND temp = ra_alloc_itemp();
        load_ireg_from_imm32(temp, 1, ZERO_EXTENSION);
        la_append_ir2_opnd3(LISA_SUB_W, value_status_temp, value_status_temp, temp);
        la_append_ir2_opnd2i_em(LISA_ANDI, value_status_temp, value_status_temp, 0x7);
    } else {
        la_append_ir2_opnd1(LISA_X86MFTOP, value_status_temp);
    }
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, value_status, zero_ir2_opnd, 13, 11);
    la_append_ir2_opnd2i_em(LISA_SLLI_W, value_status_temp, value_status_temp, 11);
    la_append_ir2_opnd3_em(LISA_OR, value_status, value_status, value_status_temp);
    /* set C1 flag to 0 */
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, value_status, zero_ir2_opnd, 9, 9);
    la_append_ir2_opnd2i(LISA_ST_H, value_status, env_ir2_opnd,
                      lsenv_offset_of_status_word(lsenv)); /* status_word */
    return true;
#endif
}

bool translate_fincstp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fincstp(pir1);
#else
    tr_fpu_inc();

    /* increments the top-of-stack pointer */
    IR2_OPND value_status = ra_alloc_itemp();
    IR2_OPND value_status_temp = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_LD_H, value_status, env_ir2_opnd,
                      lsenv_offset_of_status_word(lsenv)); /* status_word */
    if (!option_lsfpu) {
        la_append_ir2_opnd2ii(LISA_BSTRPICK_D, value_status_temp, value_status, 13, 11);
        IR2_OPND temp = ra_alloc_itemp();
        load_ireg_from_imm32(temp, 1, ZERO_EXTENSION);
        la_append_ir2_opnd3(LISA_ADD_W, value_status_temp, value_status_temp, temp);
        la_append_ir2_opnd2i_em(LISA_ANDI, value_status_temp, value_status_temp, 0x7);
    } else {
        la_append_ir2_opnd1(LISA_X86MFTOP, value_status_temp);
    }
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, value_status, zero_ir2_opnd, 13, 11);
    la_append_ir2_opnd2i_em(LISA_SLLI_W, value_status_temp, value_status_temp, 11);
    la_append_ir2_opnd3_em(LISA_OR, value_status, value_status, value_status_temp);

    /* set C1 flag to 0 */
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, value_status, zero_ir2_opnd, 9, 9);
    la_append_ir2_opnd2i(LISA_ST_H, value_status, env_ir2_opnd,
                      lsenv_offset_of_status_word(lsenv)); /* status_word */
    return true;
#endif
}

bool translate_fsin(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fsin(pir1);
#else
#ifdef USE_FSIN_HELPER
    tr_gen_call_to_helper1((ADDR)helper_fsin);
#else
     /*
     * For LA $fa 0 is defined as argument and return value reg.
     * To avoid reg destroied, move argument setting after storing
     * regs to env.
      */
    IR2_OPND param_0 = ir2_opnd_new(IR2_OPND_FREG, 0);
    IR2_OPND param_8 = ra_alloc_ftemp();
    IR2_OPND st0_opnd = ra_alloc_st(0);
    la_append_ir2_opnd2(LISA_FMOV_D, param_8, st0_opnd);

    /* save regs before call helper func */
    tr_save_registers_to_env(GPR_USEDEF_TO_SAVE, FPR_USEDEF_TO_SAVE,
                             XMM_LO_USEDEF_TO_SAVE, XMM_HI_USEDEF_TO_SAVE, 0x1|options_to_save()); 

    la_append_ir2_opnd2(LISA_FMOV_D, param_0, param_8);

    /* Call the function */
    tr_gen_call_to_helper((ADDR)sin);

    /* f0 is mapped to x86 reg now, it will be destroyed by the following registers load,
     * save it
     */
    IR2_OPND ret_opnd, ret_value;
    ret_opnd = ir2_opnd_new(IR2_OPND_FREG, 0);
    ret_value = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FMOV_D, ret_value, ret_opnd);

    /* restore regs after native call*/
    tr_load_registers_from_env(GPR_USEDEF_TO_SAVE, FPR_USEDEF_TO_SAVE,
                               XMM_LO_USEDEF_TO_SAVE, XMM_HI_USEDEF_TO_SAVE,
                               0x1|options_to_save());

    la_append_ir2_opnd2(LISA_FMOV_D, st0_opnd, ret_value);

    /* clear c2 flag in status word */
    IR2_OPND sw_value = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_LD_HU, sw_value, env_ir2_opnd,
                            lsenv_offset_of_status_word(lsenv));
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, sw_value,  zero_ir2_opnd, 9, 9);
    la_append_ir2_opnd2i(LISA_ST_H, sw_value, env_ir2_opnd,
                         lsenv_offset_of_status_word(lsenv));
    ra_free_temp(sw_value);
#endif
    return true;
#endif
}

bool translate_fcos(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fcos(pir1);
#else
#ifdef USE_FCOS_HELPER
    tr_gen_call_to_helper1((ADDR)helper_fcos);
#else
     /*
     * For LA $fa 0 is defined as argument and return value reg.
     * To avoid reg destoried, move argument setting after storing
     * regs to env.
      */
    IR2_OPND param_0 = ir2_opnd_new(IR2_OPND_FREG, 0);
    IR2_OPND param_8 = ra_alloc_ftemp();
    IR2_OPND st0_opnd = ra_alloc_st(0);
    la_append_ir2_opnd2(LISA_FMOV_D, param_8, st0_opnd);

    /* save regs before call helper func */
    tr_save_registers_to_env(GPR_USEDEF_TO_SAVE, FPR_USEDEF_TO_SAVE,
                             XMM_LO_USEDEF_TO_SAVE, XMM_HI_USEDEF_TO_SAVE, 0x1|options_to_save()); 
    la_append_ir2_opnd2(LISA_FMOV_D, param_0, param_8);

    /* Call the function */
    tr_gen_call_to_helper((ADDR)cos);

    /* f0 is mapped to x86 reg now, it will be destroyed by the following registers load,
     * save it
     */
    IR2_OPND ret_opnd, ret_value;
    ret_opnd = ir2_opnd_new(IR2_OPND_FREG, 0);
    ret_value = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FMOV_D,  ret_value, ret_opnd);

    /* restore regs after native call*/
    tr_load_registers_from_env(GPR_USEDEF_TO_SAVE, FPR_USEDEF_TO_SAVE,
                               XMM_LO_USEDEF_TO_SAVE, XMM_HI_USEDEF_TO_SAVE,
                               0x1|options_to_save());

    la_append_ir2_opnd2(LISA_FMOV_D,  st0_opnd, ret_value);

    /* clear c2 flag in status word */
    IR2_OPND sw_value = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_LD_HU, sw_value, env_ir2_opnd,
                            lsenv_offset_of_status_word(lsenv));
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, sw_value,  zero_ir2_opnd, 9, 9);
    la_append_ir2_opnd2i(LISA_ST_H, sw_value, env_ir2_opnd,
                      lsenv_offset_of_status_word(lsenv));
    ra_free_temp(sw_value);
#endif
    return true;
#endif
}

double pi = +3.141592653589793239;

bool translate_fpatan(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fpatan(pir1);
#else
#ifdef USE_FPATAN_HELPER
    tr_gen_call_to_helper1((ADDR)helper_fpatan, 1);

    if (!option_lsfpu)
        tr_fpu_pop();
#else
     /*
     * For LA $fa 0 is defined as argument and return value reg.
     * To avoid reg destoried, move argument setting after storing
     * regs to env.
      */
    /* dispose the arguments */
    IR2_OPND st0_opnd = ra_alloc_st(0);
    IR2_OPND st1_opnd = ra_alloc_st(1);
    IR2_OPND param_0 = ir2_opnd_new(IR2_OPND_FREG, 0);
    IR2_OPND param_1 = ir2_opnd_new(IR2_OPND_FREG, 1);
    IR2_OPND param_8 = ra_alloc_ftemp();
    IR2_OPND param_9 = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FMOV_D, param_8, st0_opnd);
    la_append_ir2_opnd2(LISA_FMOV_D, param_9, st1_opnd);
    /* save regs before call helper func */
    tr_save_registers_to_env(GPR_USEDEF_TO_SAVE, FPR_USEDEF_TO_SAVE,
                             XMM_LO_USEDEF_TO_SAVE, XMM_HI_USEDEF_TO_SAVE, 0x1|options_to_save()); 
    la_append_ir2_opnd2(LISA_FMOV_D, param_1, param_8);
    la_append_ir2_opnd2(LISA_FMOV_D, param_0, param_9);

    /* Call the function */
    tr_gen_call_to_helper((ADDR)atan2);

    IR2_OPND ret_opnd, ret_value;
    ret_opnd = ir2_opnd_new(IR2_OPND_FREG, 0);
    ret_value = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FMOV_D, ret_value, ret_opnd);

    /* restore regs after native call*/
    tr_load_registers_from_env(GPR_USEDEF_TO_SAVE, FPR_USEDEF_TO_SAVE,
                               XMM_LO_USEDEF_TO_SAVE, XMM_HI_USEDEF_TO_SAVE,
                               0x1|options_to_save());

    tr_fpu_pop();
    IR2_OPND new_st0_opnd = ra_alloc_st(0);
    la_append_ir2_opnd2(LISA_FMOV_D, new_st0_opnd, ret_value);

#endif
    return true;
#endif
}

bool translate_fprem(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fprem(pir1);
#else
    tr_gen_call_to_helper1((ADDR)helper_fprem, 1);

    return true;
#endif
}

bool translate_fprem1(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fprem1(pir1);
#else
    tr_gen_call_to_helper1((ADDR)helper_fprem1, 1);

    return true;
#endif
}

bool translate_frndint(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_frndint(pir1);
#else
#ifdef USE_HELPER_FRNDINT
    tr_gen_call_to_helper1((ADDR)helper_frndint, 1);
#else

    IR2_OPND st0_opnd = ra_alloc_st(0);
    la_append_ir2_opnd2(LISA_FRINT_D, st0_opnd, st0_opnd);
#endif

    return true;
#endif
}

bool translate_fscale(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fscale(pir1);
#else
    IR2_OPND st0_opnd = ra_alloc_st(0);
    IR2_OPND st1_opnd = ra_alloc_st(1);
    IR2_OPND ret_value = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FTINTRZ_L_D, ret_value, st1_opnd);
    la_append_ir2_opnd3(LISA_FSCALEB_D, st0_opnd, st0_opnd, ret_value);

    return true;
#endif
}

bool translate_fxam(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fxam(pir1);
#else
    if (!option_lsfpu) {
        tr_gen_call_to_helper1((ADDR)helper_fxam_ST0, 1);
    } else {
        IR2_OPND st0_opnd = ra_alloc_st(0);
        IR2_OPND gpr_st0_opnd = ra_alloc_itemp();
        IR2_OPND top_opnd = ra_alloc_itemp();
        IR2_OPND fpus = ra_alloc_itemp();
        IR2_OPND temp1 = ra_alloc_itemp();
        IR2_OPND temp2 = ra_alloc_itemp();
        int status_offset = lsenv_offset_of_status_word(lsenv);
        IR2_OPND not_set_c1 = ir2_opnd_new_type(IR2_OPND_LABEL);
        IR2_OPND label_next = ir2_opnd_new_type(IR2_OPND_LABEL);
        IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
        IR2_OPND label_infinity_nan = ir2_opnd_new_type(IR2_OPND_LABEL);
        IR2_OPND label_zero_denormal = ir2_opnd_new_type(IR2_OPND_LABEL);
        IR2_OPND label_infinity = ir2_opnd_new_type(IR2_OPND_LABEL);
        IR2_OPND label_zero = ir2_opnd_new_type(IR2_OPND_LABEL);


        /*
         * C3, C2, C1, C0 ← 0000
         */
        load_ireg_from_imm32(temp1, 0xb8ff, ZERO_EXTENSION);
        la_append_ir2_opnd2i_em(LISA_LD_H, fpus, env_ir2_opnd, status_offset);
        la_append_ir2_opnd3_em(LISA_AND, fpus, fpus, temp1);

        /*
         * C1 ← sign bit of ST
         */
        la_append_ir2_opnd2(LISA_MOVFR2GR_D, gpr_st0_opnd, st0_opnd);
        load_ireg_from_imm64(temp1, 0x8000000000000000ULL);
        la_append_ir2_opnd3_em(LISA_AND, temp2, gpr_st0_opnd, temp1);
        la_append_ir2_opnd3(LISA_BNE, temp2, temp1, not_set_c1);
        la_append_ir2_opnd2i_em(LISA_ORI, fpus, fpus, 0x200);
        la_append_ir2_opnd1(LISA_LABEL, not_set_c1);

        /*
         * fast path
         */
        la_append_ir2_opnd1(LISA_X86MFTOP, top_opnd);
        la_append_ir2_opnd3(LISA_BNE, top_opnd, zero_ir2_opnd, label_next);
        load_ireg_from_imm32(temp1, 0x4100UL, ZERO_EXTENSION);
        la_append_ir2_opnd3(LISA_OR, fpus, fpus, temp1);
        la_append_ir2_opnd1(LISA_B, label_exit);

        /*
         * slow path
         */
        la_append_ir2_opnd1(LISA_LABEL, label_next);
        load_ireg_from_imm64(temp1, (uint64)(0x7ff0000000000000ULL));
        la_append_ir2_opnd3_em(LISA_AND, temp2, gpr_st0_opnd, temp1);
        la_append_ir2_opnd3(LISA_BEQ, temp2, temp1, label_infinity_nan);
        la_append_ir2_opnd3(LISA_BEQ, temp2, zero_ir2_opnd, label_zero_denormal);
        /* Normal finite number */
        la_append_ir2_opnd2i_em(LISA_ORI, fpus, fpus, 0x400);
        la_append_ir2_opnd1(LISA_B, label_exit);
        la_append_ir2_opnd1(LISA_LABEL, label_infinity_nan);
        load_ireg_from_imm64(temp1, 0xfffffffffffffULL);
        la_append_ir2_opnd3_em(LISA_AND, temp2, gpr_st0_opnd, temp1);
        la_append_ir2_opnd3(LISA_BEQ, temp2, zero_ir2_opnd, label_infinity);
        /* NaN */
        la_append_ir2_opnd2i_em(LISA_ORI, fpus, fpus, 0x100);
        la_append_ir2_opnd1(LISA_B, label_exit);
        /* Infinity */
        la_append_ir2_opnd1(LISA_LABEL, label_infinity);
        la_append_ir2_opnd2i_em(LISA_ORI, fpus, fpus, 0x500);
        la_append_ir2_opnd1(LISA_B, label_exit);
        la_append_ir2_opnd1(LISA_LABEL, label_zero_denormal);
        load_ireg_from_imm64(temp1, 0xfffffffffffffULL);
        la_append_ir2_opnd3_em(LISA_AND, temp2, gpr_st0_opnd, temp1);
        la_append_ir2_opnd3(LISA_BEQ, temp2, zero_ir2_opnd, label_zero);
        /* Denormal */
        load_ireg_from_imm32(temp1, 0x4400UL, ZERO_EXTENSION);
        la_append_ir2_opnd3_em(LISA_OR, fpus, fpus, temp1);
        la_append_ir2_opnd1(LISA_B, label_exit);
        /* Zero */
        la_append_ir2_opnd1(LISA_LABEL, label_zero);
        load_ireg_from_imm32(temp1, 0x4000UL, ZERO_EXTENSION);
        la_append_ir2_opnd3_em(LISA_OR, fpus, fpus, temp1);
        la_append_ir2_opnd1(LISA_B, label_exit);
        /*
         * exit
         */
        la_append_ir2_opnd1(LISA_LABEL, label_exit);
        la_append_ir2_opnd2i(LISA_ST_H, fpus, env_ir2_opnd, status_offset);
    }

    return true;
#endif
}

bool translate_f2xm1(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_f2xm1(pir1);
#else
    tr_gen_call_to_helper1((ADDR)helper_f2xm1, 1);

    return true;
#endif
}

bool translate_fxtract(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fxtract(pir1);
#else
#ifdef USE_FXTRACT_HELPER
    tr_gen_call_to_helper1((ADDR)helper_fxtract, 1);

    if (!option_lsfpu)
        tr_fpu_push();
#else
    /* if the source operand is 0.0 or -0.0, goto label_zero for special process
     */
    IR2_OPND f_zero = ra_alloc_ftemp();
    la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, f_zero, zero_ir2_opnd);
    la_append_ir2_opnd2(LISA_FFINT_D_L, f_zero, f_zero);
    IR2_OPND st0_opnd = ra_alloc_st(0);
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, st0_opnd, f_zero, FCMP_COND_CEQ);
    IR2_OPND label_zero = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_zero);

    IR2_OPND fp_zero = ra_alloc_itemp();
    load_ireg_from_imm64(fp_zero, 0x8000000000000000ull);
    la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, f_zero, fp_zero);
    ra_free_temp(fp_zero);
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, st0_opnd, f_zero, FCMP_COND_CEQ);
    ra_free_temp(f_zero);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_zero);

	//inf 
    IR2_OPND label_inf = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND fp_inf = ra_alloc_itemp();
    IR2_OPND f_inf = ra_alloc_ftemp();
    load_ireg_from_imm64(fp_inf, 0x7ff0000000000000ull);
    la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, f_inf, fp_inf);
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, st0_opnd, f_inf, FCMP_COND_CEQ);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_inf);
	//-inf
    load_ireg_from_imm64(fp_inf, 0xfff0000000000000ull);
    la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, f_inf, fp_inf);
    ra_free_temp(fp_inf);
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, st0_opnd, f_inf, FCMP_COND_CEQ);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_inf);


    /* store st0 onto native stack for further use */
    /* Mapping to LA 29 -> 3 */
    IR2_OPND native_sp = ir2_opnd_new(IR2_OPND_IREG, 3);
    la_append_ir2_opnd2i_em(LISA_ADDI_ADDR, native_sp, native_sp, -8);
    la_append_ir2_opnd2i(LISA_FST_D, st0_opnd, native_sp, 0);

    IR2_OPND param_0 = ir2_opnd_new(IR2_OPND_FREG, 0);
    IR2_OPND param_1 = ir2_opnd_new(IR2_OPND_FREG, 1);
    IR2_OPND param_8 = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FMOV_D, param_8, st0_opnd);

    /* save regs before call helper func */
    tr_save_fcsr_to_env();
    tr_save_registers_to_env(GPR_USEDEF_TO_SAVE, FPR_USEDEF_TO_SAVE,
                             XMM_LO_USEDEF_TO_SAVE, XMM_HI_USEDEF_TO_SAVE, 0x1|options_to_save()); 

    la_append_ir2_opnd2(LISA_FMOV_D, param_0, param_8);
    /* Call the function */
    /* explicitly specify function prototype to select from overloaded
     * prototypes */
    double (*logb_p)(double) = logb;
    tr_gen_call_to_helper((ADDR)logb_p);

    IR2_OPND ret_value = ir2_opnd_new(IR2_OPND_FREG, 0);
    la_append_ir2_opnd2(LISA_FMOV_D, param_1, ret_value); //ret val -> pow(2.0,y)
    /* store the new st0 value to env->st0 */
    //int fpr_index = (0 + lsenv->tr_data->curr_top) & 7;
    //la_append_ir2_opnd2i(LISA_FST_D, ret_value, env_ir2_opnd,
    //                    lsenv_offset_of_fpr(lsenv, fpr_index));
	IR2_OPND env_st0_addr_opnd = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_LD_W, env_st0_addr_opnd, env_ir2_opnd, lsenv_offset_of_top(lsenv));
    la_append_ir2_opnd2i_em(LISA_SLLI_W, env_st0_addr_opnd, env_st0_addr_opnd, 4);
    la_append_ir2_opnd3_em(LISA_ADD_ADDR, env_st0_addr_opnd, env_st0_addr_opnd, env_ir2_opnd);
    //la_append_ir2_opnd2i_em(LISA_ADDI_ADDR, env_st0_addr_opnd, env_st0_addr_opnd, lsenv_offset_of_fpr(lsenv, 0));
    la_append_ir2_opnd2i(LISA_FST_D, ret_value, env_st0_addr_opnd, lsenv_offset_of_fpr(lsenv, 0));

    IR2_OPND imm2_opnd = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ADDI_D, imm2_opnd, zero_ir2_opnd, 2);
    la_append_ir2_opnd2_em(LISA_MOVGR2FR_W, param_0, imm2_opnd);
    la_append_ir2_opnd2(LISA_FFINT_D_W, param_0, param_0);
    ra_free_temp(imm2_opnd);

    /* Call the function */
    /* explicitly specify function prototype to select from overloaded
     * prototypes */
    double (*pow_p)(double, double) = pow;
    tr_gen_call_to_helper((ADDR)pow_p);

    IR2_OPND ret_val = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FMOV_D, ret_val, ret_value);

    /* restore regs after native call*/
    tr_load_registers_from_env(GPR_USEDEF_TO_SAVE, FPR_USEDEF_TO_SAVE,
                               XMM_LO_USEDEF_TO_SAVE, XMM_HI_USEDEF_TO_SAVE,
                               0x1|options_to_save());
    /*
     * TODO: restore fcsr saved before helper to pass glibc logb tests,
     * helper fucntion may set fcsr incorrectly, should inspect later.
     */
    tr_load_fcsr_from_env();

    IR2_OPND origin_st0_value = ra_alloc_ftemp();
    la_append_ir2_opnd2i(LISA_FLD_D, origin_st0_value, native_sp, 0);
    la_append_ir2_opnd2i_em(LISA_ADDI_ADDR, native_sp, native_sp, 8);

    la_append_ir2_opnd3(LISA_FDIV_D, origin_st0_value, origin_st0_value, ret_val);
    ra_free_temp(ret_val);
    tr_fpu_push();

    IR2_OPND new_st0_opnd = ra_alloc_st(0);
    la_append_ir2_opnd2(LISA_FMOV_D, new_st0_opnd, origin_st0_value);
    ra_free_temp(origin_st0_value);
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd1(LISA_B, label_exit);

    /* special process for 0.0 and -0.0 */
    la_append_ir2_opnd1(LISA_LABEL, label_zero);
    IR2_OPND temp_st0 = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FMOV_D, temp_st0, st0_opnd);
    /* the float register stack has been changed, now put -inf to ST1 and */
    /* 0.0(-0.0) to ST0 */
    IR2_OPND inf_opnd = ra_alloc_itemp();
    load_ireg_from_imm64(inf_opnd, 0xfff0000000000000ull);
    IR2_OPND st1_opnd = ra_alloc_st(1);
    la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, st1_opnd, inf_opnd);
    la_append_ir2_opnd2(LISA_FMOV_D, new_st0_opnd, temp_st0);
    la_append_ir2_opnd1(LISA_B, label_exit);

    /* special process for inf and -inf */
    la_append_ir2_opnd1(LISA_LABEL, label_inf);
    //IR2_OPND temp_st0 = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FMOV_D, temp_st0, st0_opnd);
    /*put ST1 to ST0 and inf to ST1 */
    load_ireg_from_imm64(inf_opnd, 0x7ff0000000000000ull);
    //IR2_OPND st1_opnd = ra_alloc_st(1);
    la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, st1_opnd, inf_opnd);
    la_append_ir2_opnd2(LISA_FMOV_D, new_st0_opnd, temp_st0);
    //la_append_ir2_opnd1(LISA_B, label_exit);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);
    ra_free_temp(inf_opnd);
    ra_free_temp(temp_st0);
#endif

    return true;
#endif
}

bool translate_fyl2x(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fyl2x(pir1);
#else
#ifdef USE_FYL2X_HELPER
    tr_gen_call_to_helper1((ADDR)helper_fyl2x);

    if (!option_lsfpu)
        tr_fpu_pop();
#else
     /*
     * For LA $fa 0 is defined as argument and return value reg.
     * To avoid reg destoried, move argument setting after storing
     * regs to env.
      */
    IR2_OPND st0_opnd = ra_alloc_st(0);
    /* dispose the arguments */
    IR2_OPND param_0 = ir2_opnd_new(IR2_OPND_FREG, 0);
    IR2_OPND param_8 = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FMOV_D, param_8, st0_opnd);

    /* save regs before call helper func */
    tr_save_registers_to_env(GPR_USEDEF_TO_SAVE, FPR_USEDEF_TO_SAVE,
                             XMM_LO_USEDEF_TO_SAVE, XMM_HI_USEDEF_TO_SAVE, 0x1|options_to_save()); 
    la_append_ir2_opnd2(LISA_FMOV_D, param_0, param_8);
    /* Call the function  */
    tr_gen_call_to_helper((ADDR)log2);

    IR2_OPND ret_opnd, ret_value;
    ret_opnd = ir2_opnd_new(IR2_OPND_FREG, 0);
    ret_value = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FMOV_D, ret_value, ret_opnd);

    /* restore regs after native call*/
    tr_load_registers_from_env(GPR_USEDEF_TO_SAVE, FPR_USEDEF_TO_SAVE,
                               XMM_LO_USEDEF_TO_SAVE, XMM_HI_USEDEF_TO_SAVE,
                               0x1|options_to_save());

    la_append_ir2_opnd2(LISA_FMOV_D, st0_opnd, ret_value);

    IR2_OPND st1_opnd = ra_alloc_st(1);

    la_append_ir2_opnd3(LISA_FMUL_D, st1_opnd, st0_opnd, st1_opnd);

    tr_fpu_pop();
#endif

    return true;
#endif
}

bool translate_fyl2xp1(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fyl2xp1(pir1);
#else
#ifdef USE_FYL2XP1_HELPER
    tr_gen_call_to_helper1((ADDR)helper_fyl2xp1);

    if (!option_lsfpu)
        tr_fpu_pop();
#else
    IR2_OPND st0_opnd = ra_alloc_st(0);

     /*
     * For LA $fa 0 is defined as argument and return value reg.
     * To avoid reg destoried, move argument setting after storing
     * regs to env.
      */
    /* dispose the arguments */
    IR2_OPND param_0 = ir2_opnd_new(IR2_OPND_FREG, 0);
    IR2_OPND param_8 = ra_alloc_ftemp();
    IR2_OPND itemp_1 = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ORI, itemp_1, zero_ir2_opnd, 1);
    la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, param_8, itemp_1);
    la_append_ir2_opnd2(LISA_FFINT_D_L, param_8,param_8);
    la_append_ir2_opnd3(LISA_FADD_D, param_8, st0_opnd, param_8);
    ra_free_temp(itemp_1);

    /* save regs before call helper func */
    tr_save_registers_to_env(GPR_USEDEF_TO_SAVE, FPR_USEDEF_TO_SAVE,
                             XMM_LO_USEDEF_TO_SAVE, XMM_HI_USEDEF_TO_SAVE, 0x1|options_to_save());

    la_append_ir2_opnd2(LISA_FMOV_D, param_0, param_8);
    /* Call the function  */
    tr_gen_call_to_helper((ADDR)log2);

    IR2_OPND ret_opnd, ret_value;
    ret_opnd = ir2_opnd_new(IR2_OPND_FREG, 0);
    ret_value = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FMOV_D, ret_value, ret_opnd);

    /* restore regs after native call*/
    tr_load_registers_from_env(GPR_USEDEF_TO_SAVE, FPR_USEDEF_TO_SAVE,
                               XMM_LO_USEDEF_TO_SAVE, XMM_HI_USEDEF_TO_SAVE,
                               0x1|options_to_save());

    la_append_ir2_opnd2(LISA_FMOV_D, st0_opnd, ret_value);

    IR2_OPND st1_opnd = ra_alloc_st(1);

    la_append_ir2_opnd3(LISA_FMUL_D, st1_opnd, st0_opnd, st1_opnd);

    tr_fpu_pop();
#endif

    return true;
#endif
}

bool translate_fsincos(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fsincos(pir1);
#else
#ifdef USE_FSINCOS_HELPER
    tr_gen_call_to_helper1((ADDR)helper_fsincos);

    if (!option_lsfpu)
        tr_fpu_push();
#else
    /* set status_word.c2 = 0 */
    IR2_OPND status_word = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_LD_HU, status_word, env_ir2_opnd,
                      lsenv_offset_of_status_word(lsenv));
    /* USE LISA_BSTRINS_D to clear bit 10 of status_word */
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, status_word, zero_ir2_opnd, 10, 10);
    la_append_ir2_opnd2i(LISA_ST_H, status_word, env_ir2_opnd,
                      lsenv_offset_of_status_word(lsenv));

     /*
     * For LA $fa 0 is defined as argument and return value reg.
     * To avoid reg destoried, move argument setting after storing
     * regs to env.
      */
    /* dispose the arguments */
    IR2_OPND param_1 = ir2_opnd_new(IR2_OPND_FREG, 0);
    IR2_OPND param_8 = ra_alloc_ftemp();
    IR2_OPND st0_opnd = ra_alloc_st(0);
    la_append_ir2_opnd2(LISA_FMOV_D, param_8, st0_opnd);

    /* save regs before call helper func */
    tr_save_registers_to_env(GPR_USEDEF_TO_SAVE, FPR_USEDEF_TO_SAVE,
                             XMM_LO_USEDEF_TO_SAVE, XMM_HI_USEDEF_TO_SAVE, 0x1|options_to_save()); 
    la_append_ir2_opnd2(LISA_FMOV_D, param_1, param_8);


     /*
     * How to transfer float parameter to func.
     * Here is a example on native C code.
      *
     * 1200008a0:   001501a5        move    $r5,$r13
     * 1200008a4:   00150184        move    $r4,$r12
     * 1200008a8:   2bbfa2c0        fld.d   $f0,$r22,-24(0xfe8)
     * 1200008ac:   54003400        bl      52(0x34) # 1200008e0 <__sincos>
     * According to above disassamble code, r4/r5 will be used as paramter.
     * f0 is used as double float parameter reg.
     * So modify parameter2/3 reg defination.
      */

    IR2_OPND top_opnd = ir2_opnd_new(IR2_OPND_IREG, 6);
    IR2_OPND param_2 = ir2_opnd_new(IR2_OPND_IREG, 4);
    /* fpreg_base + top * 16 */
    la_append_ir2_opnd2i_em(LISA_LD_W, top_opnd, env_ir2_opnd, lsenv_offset_of_top(lsenv));
    la_append_ir2_opnd2i_em(LISA_SLLI_W, param_2, top_opnd, 4);
    la_append_ir2_opnd3_em(LISA_ADD_ADDR, param_2, param_2, env_ir2_opnd);
    la_append_ir2_opnd2i_em(LISA_ADDI_ADDR, param_2, param_2, lsenv_offset_of_fpr(lsenv, 0));

    IR2_OPND param_3 = ir2_opnd_new(IR2_OPND_IREG, 5);
    la_append_ir2_opnd2i_em(LISA_ADDI_W, top_opnd, top_opnd, -1);
    la_append_ir2_opnd2i_em(LISA_ANDI, top_opnd, top_opnd, 0x7);
    la_append_ir2_opnd2i_em(LISA_SLLI_W, param_3, top_opnd, 4);
    la_append_ir2_opnd3_em(LISA_ADD_ADDR, param_3, param_3, env_ir2_opnd);
    la_append_ir2_opnd2i_em(LISA_ADDI_ADDR, param_3, param_3, lsenv_offset_of_fpr(lsenv, 0));

    /* Call the function  */
    tr_gen_call_to_helper((ADDR)sincos);

    /* restore regs after helper call */
    tr_load_registers_from_env(GPR_USEDEF_TO_SAVE, FPR_USEDEF_TO_SAVE,
                               XMM_LO_USEDEF_TO_SAVE, XMM_HI_USEDEF_TO_SAVE,
                               0x1|options_to_save());

    tr_fpu_push();
#endif

    return true;
#endif
}

bool translate_fxch(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fxch(pir1);
#else
    IR2_OPND st0_opnd = ra_alloc_st(0);
    int opnd2_index;
    if (ir1_opnd_num(pir1) == 0) {
        opnd2_index = 1;
    } else {
        opnd2_index = ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0));
    }
    IR2_OPND opnd2 = ra_alloc_st(opnd2_index);
    IR2_OPND tmp_opnd = ra_alloc_ftemp();
    la_append_ir2_opnd2_em(LISA_FMOV_D, tmp_opnd, opnd2);
    la_append_ir2_opnd2_em(LISA_FMOV_D, opnd2, st0_opnd);
    la_append_ir2_opnd2_em(LISA_FMOV_D, st0_opnd, tmp_opnd);
    ra_free_temp(tmp_opnd);
    return true;
#endif
}
bool translate_ftst(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_ftst(pir1);
#else
    IR2_OPND status_word = ra_alloc_itemp();
    IR2_OPND tmp_opnd = ra_alloc_itemp();
    /* load status_word */
    int offset = lsenv_offset_of_status_word(lsenv);

    lsassert(offset <= 0x7ff);
    la_append_ir2_opnd2i_em(LISA_LD_HU, status_word, env_ir2_opnd, offset);

    /* clear status_word c0 c2 c3 */
    la_append_ir2_opnd1i_em(LISA_LU12I_W, tmp_opnd, 0xb);
    la_append_ir2_opnd2i_em(LISA_ORI, tmp_opnd, tmp_opnd, 0xaff);
    la_append_ir2_opnd3_em(LISA_AND, status_word, status_word, tmp_opnd);

    IR2_OPND f_zero = ra_alloc_ftemp();
    la_append_ir2_opnd2_em(LISA_MOVGR2FR_W, f_zero, zero_ir2_opnd);
    la_append_ir2_opnd2_em(LISA_FFINT_D_W, f_zero, f_zero);

    IR2_OPND st0_opnd = ra_alloc_st(0);
    IR2_OPND label_for_lt = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_for_un = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_for_eq = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_for_exit = ir2_opnd_new_type(IR2_OPND_LABEL);

    /* check is unordered */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, st0_opnd, f_zero, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_un);
    /* check is equal */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, st0_opnd, f_zero, FCMP_COND_CEQ);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_eq);
    /* check is less than */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0_ir2_opnd, st0_opnd, f_zero, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCNEZ, fcc0_ir2_opnd, label_for_lt);

    /* greater than */
    la_append_ir2_opnd1(LISA_B, label_for_exit);
    /* lt: */
    la_append_ir2_opnd1(LISA_LABEL, label_for_lt);
    la_append_ir2_opnd2i_em(LISA_ORI, status_word, status_word, 0x100);
    la_append_ir2_opnd1(LISA_B, label_for_exit);
    /* eq: */
    la_append_ir2_opnd1(LISA_LABEL, label_for_eq);
    la_append_ir2_opnd1i(LISA_LU12I_W, tmp_opnd, 0x4);
    la_append_ir2_opnd3_em(LISA_OR, status_word, status_word, tmp_opnd);
    la_append_ir2_opnd1(LISA_B, label_for_exit);
    /* un: */
    la_append_ir2_opnd1(LISA_LABEL, label_for_un);
    la_append_ir2_opnd1i(LISA_LU12I_W, tmp_opnd, 0x4);
    la_append_ir2_opnd2i_em(LISA_ORI, tmp_opnd, tmp_opnd, 0x500);
    la_append_ir2_opnd3_em(LISA_OR, status_word, status_word, tmp_opnd);
    /* exit: */
    la_append_ir2_opnd1(LISA_LABEL, label_for_exit);

    la_append_ir2_opnd2i(LISA_ST_H, status_word, env_ir2_opnd,
                      lsenv_offset_of_status_word(lsenv));
    ra_free_temp(status_word);
    ra_free_temp(f_zero);
    return true;
#endif
}

bool translate_fptan(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fptan(pir1);
#else
    tr_gen_call_to_helper1((ADDR)helper_fptan, 1);

    /* for software matained top we have a difficulty here:
       fptan can push(normal input) or not push(for out of range input)
       we seems to have no way to decide next top statically
       with lsfpu support, the adjusted top will be restored to hardware
       when we return from helper.
     */
    if (!option_lsfpu)
        tr_fpu_push();

    return true;
#endif
}

bool translate_fisttp(IR1_INST *pir1)
{
    fprintf(stderr, "%s not implemented. translation failed.\n", __FUNCTION__);
    return false;
}
bool translate_feni(IR1_INST *pir1)
{
    fprintf(stderr, "%s not implemented. translation failed.\n", __FUNCTION__);
    return false;
}
bool translate_fedisi(IR1_INST *pir1)
{
    fprintf(stderr, "%s not implemented. translation failed.\n", __FUNCTION__);
    return false;
}
bool translate_fsetpm(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fsetpm(pir1);
#else
    fprintf(stderr, "%s not implemented. translation failed.\n", __FUNCTION__);
    return false;
#endif
}
bool translate_fbld(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fbld(pir1);
#else
    fprintf(stderr, "%s not implemented. translation failed.\n", __FUNCTION__);
    return false;
#endif
}
bool translate_fbstp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fbstp(pir1);
#else
    fprintf(stderr, "%s not implemented. translation failed.\n", __FUNCTION__);
    return false;
#endif
}
/*
 * NOTE:invoke helper is much easier than native insns, there is 512 bytes need to
 * be safed or restored.
 * On the other hand, fxsave/fxrestore is not under hot path so that is acceptable
 * from performance perspective.
 */
bool translate_fxsave(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fxsave(pir1);
#else
    IR2_OPND dest_opnd =
        mem_ir1_to_ir2_opnd(ir1_get_opnd(pir1, 0), false);

    tr_gen_call_to_helper2((ADDR)helper_fxsave, dest_opnd, 0);
    return true;
#endif
}
bool translate_fxrstor(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fxrstor(pir1);
#else
    IR2_OPND dest_opnd =
          mem_ir1_to_ir2_opnd(ir1_get_opnd(pir1, 0), false);

    tr_gen_call_to_helper2((ADDR)helper_fxrstor, dest_opnd, 0);
    return true;
#endif
}
