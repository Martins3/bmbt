#include <math.h>
#include "common.h"
#include "reg_alloc.h"
#include "env.h"
#include "x86tomips-options.h"

bool translate_fadd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);
    if (opnd_num == 1) {
        IR2_OPND st0_opnd = ra_alloc_st(0);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        append_ir2_opnd3(LISA_FADD_D, &st0_opnd, &st0_opnd, &src1_opnd);
    } else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        append_ir2_opnd3(LISA_FADD_D, &dest_opnd, &dest_opnd, &src1_opnd);
    }

    return true;
}

bool translate_faddp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    int opnd_num = ir1_opnd_num(pir1);

    if (opnd_num == 0) {
        IR2_OPND st0 = ra_alloc_st(0);
        IR2_OPND st1 = ra_alloc_st(1);
        append_ir2_opnd3(LISA_FADD_D, &st1, &st0, &st1);
    } 
    else if(opnd_num == 1){
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND st0 = ra_alloc_st(0);
        append_ir2_opnd3(LISA_FADD_D, &dest_opnd, &dest_opnd, &st0);
    }
    else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        append_ir2_opnd3(LISA_FADD_D, &dest_opnd, &dest_opnd, &src1_opnd);
    }
    tr_fpu_pop();

    return true;
}

bool translate_fiadd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1);

    IR2_OPND st0 = ra_alloc_st(0);
    IR2_OPND t_freg = ra_alloc_ftemp();
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    load_freg_from_ir1_2(&t_freg, opnd0, false, false);
    if (ir1_opnd_size(opnd0) > 32) {
        append_ir2_opnd2(LISA_FFINT_D_L, &t_freg, &t_freg);
    } else {
        append_ir2_opnd2(LISA_FFINT_D_W, &t_freg, &t_freg);
    }

    append_ir2_opnd3(LISA_FADD_D, &st0, &t_freg, &st0);
    ra_free_temp(&t_freg);
    return true;
}

bool translate_fsub(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    if (opnd_num == 1) {
        IR2_OPND st0_opnd = ra_alloc_st(0);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        append_ir2_opnd3(LISA_FSUB_D, &st0_opnd, &st0_opnd, &src1_opnd);
    } else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        append_ir2_opnd3(LISA_FSUB_D, &dest_opnd, &dest_opnd, &src1_opnd);
    }

    return true;
}

bool translate_fisub(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1);

    IR2_OPND st0 = ra_alloc_st(0);
    IR2_OPND t_freg = ra_alloc_ftemp();
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    load_freg_from_ir1_2(&t_freg, opnd0, false, false);
    if (ir1_opnd_size(opnd0) > 32) {
        append_ir2_opnd2(LISA_FFINT_D_L, &t_freg, &t_freg);
    } else {
        append_ir2_opnd2(LISA_FFINT_D_W, &t_freg, &t_freg);
    }
    append_ir2_opnd3(LISA_FSUB_D, &st0, &st0, &t_freg);
    ra_free_temp(&t_freg);
    return true;
}

bool translate_fisubr(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1);

    IR2_OPND st0 = ra_alloc_st(0);
    IR2_OPND t_freg = ra_alloc_ftemp();
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    load_freg_from_ir1_2(&t_freg, opnd0, false, false);
    if (ir1_opnd_size(opnd0) > 32) {
        append_ir2_opnd2(LISA_FFINT_D_L, &t_freg, &t_freg);
    } else {
        append_ir2_opnd2(LISA_FFINT_D_W, &t_freg, &t_freg);
    }
    append_ir2_opnd3(LISA_FSUB_D, &st0, &t_freg, &st0);
    ra_free_temp(&t_freg);
    return true;
}

bool translate_fsubr(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    if (opnd_num == 1) {
        IR2_OPND st0_opnd = ra_alloc_st(0);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        append_ir2_opnd3(LISA_FSUB_D, &st0_opnd, &src1_opnd, &st0_opnd);
    } else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        append_ir2_opnd3(LISA_FSUB_D, &dest_opnd, &src1_opnd, &dest_opnd);
    }

    return true;
}

bool translate_fsubrp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    int opnd_num = ir1_opnd_num(pir1);

    if (opnd_num == 0) {
        IR2_OPND st0 = ra_alloc_st(0);
        IR2_OPND st1 = ra_alloc_st(1);
        append_ir2_opnd3(LISA_FSUB_D, &st1, &st0, &st1);
    }
    else if(opnd_num == 1){
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND st0 = ra_alloc_st(0);
        append_ir2_opnd3(LISA_FSUB_D, &dest_opnd, &st0, &dest_opnd);
    }    
    else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        append_ir2_opnd3(LISA_FSUB_D, &dest_opnd, &src1_opnd, &dest_opnd);
    }
    tr_fpu_pop();
    return true;
}

bool translate_fsubp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    int opnd_num = ir1_opnd_num(pir1);

    if (opnd_num == 0) {
        IR2_OPND st0 = ra_alloc_st(0);
        IR2_OPND st1 = ra_alloc_st(1);
        append_ir2_opnd3(LISA_FSUB_D, &st1, &st1, &st0);
    }
    else if(opnd_num == 1){
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND st0 = ra_alloc_st(0);
        append_ir2_opnd3(LISA_FSUB_D, &dest_opnd, &dest_opnd, &st0);
    }    
    else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        append_ir2_opnd3(LISA_FSUB_D, &dest_opnd, &dest_opnd, &src1_opnd);
    }
    tr_fpu_pop();
    return true;
}

bool translate_fmul(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    if (opnd_num == 1) {
        IR2_OPND st0_opnd = ra_alloc_st(0);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        append_ir2_opnd3(LISA_FMUL_D, &st0_opnd, &st0_opnd, &src1_opnd);
//        append_ir2_opnd3(mips_mul_d, &st0_opnd, &st0_opnd, &src1_opnd);
    } else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        append_ir2_opnd3(LISA_FMUL_D, &dest_opnd, &dest_opnd, &src1_opnd);
//        append_ir2_opnd3(mips_mul_d, &dest_opnd, &dest_opnd, &src1_opnd);
    }

    return true;
}

bool translate_fimul(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1);

    IR2_OPND st0 = ra_alloc_st(0);
    IR2_OPND t_freg = ra_alloc_ftemp();
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    load_freg_from_ir1_2(&t_freg, opnd0, false, false);
    if (ir1_opnd_size(opnd0) > 32) {
        append_ir2_opnd2(LISA_FFINT_D_L, &t_freg, &t_freg);
    } else {
        append_ir2_opnd2(LISA_FFINT_D_W, &t_freg, &t_freg);
    }
    append_ir2_opnd3(LISA_FMUL_D, &st0, &t_freg, &st0);
    ra_free_temp(&t_freg);
    return true;
}

bool translate_fmulp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    int opnd_num = ir1_opnd_num(pir1);

    if (opnd_num == 0) {
        IR2_OPND st0 = ra_alloc_st(0);
        IR2_OPND st1 = ra_alloc_st(1);
        append_ir2_opnd3(LISA_FMUL_D, &st1, &st0, &st1);
    }
    else if(opnd_num == 1){
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND st0 = ra_alloc_st(0);
        append_ir2_opnd3(LISA_FMUL_D, &dest_opnd, &dest_opnd, &st0);
    }    
    else {
        IR2_OPND src0_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        append_ir2_opnd3(LISA_FMUL_D, &src0_opnd, &src0_opnd, &src1_opnd);
    }
    tr_fpu_pop();

    return true;
}

bool translate_fdiv(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    if (opnd_num == 1) {
        IR2_OPND st0_opnd = ra_alloc_st(0);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        append_ir2_opnd3(LISA_FDIV_D, &st0_opnd, &st0_opnd, &src1_opnd);
//        append_ir2_opnd3(mips_div_d, &st0_opnd, &st0_opnd, &src1_opnd);
    } else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        append_ir2_opnd3(LISA_FDIV_D, &dest_opnd, &dest_opnd, &src1_opnd);
//        append_ir2_opnd3(mips_div_d, &dest_opnd, &dest_opnd, &src1_opnd);
    }
    return true;
}
bool translate_fdivr(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    if (opnd_num == 1) {
        IR2_OPND st0_opnd = ra_alloc_st(0);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        append_ir2_opnd3(LISA_FDIV_D, &st0_opnd, &src1_opnd, &st0_opnd);
    } else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        append_ir2_opnd3(LISA_FDIV_D, &dest_opnd, &src1_opnd, &dest_opnd);
    }
    return true;
}

bool translate_fidiv(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1);

    IR2_OPND st0 = ra_alloc_st(0);
    IR2_OPND t_freg = ra_alloc_ftemp();
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    load_freg_from_ir1_2(&t_freg, opnd0, false, false);
    if (ir1_opnd_size(opnd0) > 32) {
        append_ir2_opnd2(LISA_FFINT_D_L, &t_freg, &t_freg);
    } else {
        append_ir2_opnd2(LISA_FFINT_D_W, &t_freg, &t_freg);
    }
    append_ir2_opnd3(LISA_FDIV_D, &st0, &st0, &t_freg);
    ra_free_temp(&t_freg);
    return true;
}
bool translate_fidivr(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1);

    IR2_OPND st0 = ra_alloc_st(0);
    IR2_OPND t_freg = ra_alloc_ftemp();
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    load_freg_from_ir1_2(&t_freg, opnd0, false, false);
    if (ir1_opnd_size(opnd0) > 32) {
        append_ir2_opnd2(LISA_FFINT_D_L, &t_freg, &t_freg);
    } else {
        append_ir2_opnd2(LISA_FFINT_D_W, &t_freg, &t_freg);
    }
    append_ir2_opnd3(LISA_FDIV_D, &st0, &t_freg, &st0);
    ra_free_temp(&t_freg);
    return true;
}

bool translate_fdivrp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    int opnd_num = ir1_opnd_num(pir1);

    if (opnd_num == 0) {
        IR2_OPND st0 = ra_alloc_st(0);
        IR2_OPND st1 = ra_alloc_st(1);
        append_ir2_opnd3(LISA_FDIV_D, &st1, &st0, &st1);
    }
    else if(opnd_num == 1){
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND st0 = ra_alloc_st(0);
        append_ir2_opnd3(LISA_FDIV_D, &dest_opnd, &st0, &dest_opnd);
    }    
    else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        append_ir2_opnd3(LISA_FDIV_D, &dest_opnd, &src1_opnd, &dest_opnd);
    }
    tr_fpu_pop();
    return true;
}
bool translate_fdivp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    int opnd_num = ir1_opnd_num(pir1);

    if (opnd_num == 0) {
        IR2_OPND st0 = ra_alloc_st(0);
        IR2_OPND st1 = ra_alloc_st(1);
        append_ir2_opnd3(LISA_FDIV_D, &st1, &st1, &st0);
    } 
    else if(opnd_num == 1){
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND st0 = ra_alloc_st(0);
        append_ir2_opnd3(LISA_FDIV_D, &dest_opnd, &dest_opnd, &st0);
    }
    else {
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        append_ir2_opnd3(LISA_FDIV_D, &dest_opnd, &dest_opnd, &src1_opnd);
    }
    tr_fpu_pop();
    return true;
}

bool translate_fnop(IR1_INST *pir1)
{
#ifndef CONFIG_SOFTMMU
//    append_ir2_opnd0(mips_nop);
#else
    helper_cfg_t cfg = all_helper_cfg;
    tr_sys_gen_call_to_helper1_cfg((ADDR)helper_fwait, cfg);
#endif
    return true;
}

bool translate_fsqrt(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    IR2_OPND st0_opnd = ra_alloc_st(0);
    append_ir2_opnd3(LISA_FSQRT_D, &st0_opnd, &st0_opnd, &st0_opnd);
    return true;
}

bool translate_fabs(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    IR2_OPND st0_opnd = ra_alloc_st(0);
    append_ir2_opnd3(LISA_FABS_D, &st0_opnd, &st0_opnd, &st0_opnd);
    return true;
}

bool translate_fchs(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    IR2_OPND st0_opnd = ra_alloc_st(0);
    append_ir2_opnd3(LISA_FNEG_D, &st0_opnd, &st0_opnd, &st0_opnd);
    return true;
}

#ifndef CONFIG_SOFTMMU
bool translate_fdecstp(IR1_INST *pir1)
{
    lsassertm(0, "FPU fdecstp to be implemented in LoongArch.\n");
//    tr_fpu_dec();
//
//    /* decrements the top-of-stack pointer */
//    IR2_OPND value_status = ra_alloc_itemp();
//    IR2_OPND value_status_temp = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_lh, &value_status, &env_ir2_opnd,
//                      lsenv_offset_of_status_word(lsenv)); /* status_word */
//    append_ir2_opnd2i(mips_andi, &value_status, &value_status, 0xc7ffULL);
//    if (!option_lsfpu) {
//        append_ir2_opnd2i(mips_lh, &value_status_temp, &env_ir2_opnd,
//                lsenv_offset_of_status_word(lsenv)); /* status_word */
//        append_ir2_opnd2i(mips_andi, &value_status_temp, &value_status_temp,
//                0x3800ULL);
//        append_ir2_opnd2i(mips_sra, &value_status_temp, &value_status_temp, 11);
//        IR2_OPND temp = ra_alloc_itemp();
//        load_imm32_to_ir2(&temp, 1, ZERO_EXTENSION);
//        append_ir2_opnd3(mips_subu, &value_status_temp, &value_status_temp, &temp);
//        append_ir2_opnd2i(mips_andi, &value_status_temp, &value_status_temp, 7);
//    } else {
//        append_ir2_opnd1(mips_mftop, &value_status_temp);
//    }
//    append_ir2_opnd2i(mips_sll, &value_status_temp, &value_status_temp, 11);
//    append_ir2_opnd3(mips_or, &value_status, &value_status, &value_status_temp);
//    /* set C1 flag to 0 */
//    append_ir2_opnd2i(mips_andi, &value_status, &value_status, 0xfdffULL);
//    append_ir2_opnd2i(mips_sh, &value_status, &env_ir2_opnd,
//                      lsenv_offset_of_status_word(lsenv)); /* status_word */
    return true;
}
#endif

#ifndef CONFIG_SOFTMMU
bool translate_fincstp(IR1_INST *pir1)
{
    lsassertm(0, "FPU fincstp to be implemented in LoongArch.\n");
//    tr_fpu_inc();
//
//    /* increments the top-of-stack pointer */
//    IR2_OPND value_status = ra_alloc_itemp();
//    IR2_OPND value_status_temp = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_lh, &value_status, &env_ir2_opnd,
//                      lsenv_offset_of_status_word(lsenv)); /* status_word */
//    append_ir2_opnd2i(mips_andi, &value_status, &value_status, 0xc7ffULL);
//    if (!option_lsfpu) {
//        append_ir2_opnd2i(mips_lh, &value_status_temp, &env_ir2_opnd,
//                lsenv_offset_of_status_word(lsenv)); /* status_word */
//        append_ir2_opnd2i(mips_andi, &value_status_temp, &value_status_temp,
//                0x3800ULL);
//        append_ir2_opnd2i(mips_sra, &value_status_temp, &value_status_temp, 11);
//        IR2_OPND temp = ra_alloc_itemp();
//        load_imm32_to_ir2(&temp, 1, ZERO_EXTENSION);
//        append_ir2_opnd3(mips_addu, &value_status_temp, &value_status_temp, &temp);
//        append_ir2_opnd2i(mips_andi, &value_status_temp, &value_status_temp, 7);
//    } else {
//        append_ir2_opnd1(mips_mftop, &value_status_temp);
//    }
//    append_ir2_opnd2i(mips_sll, &value_status_temp, &value_status_temp, 11);
//    append_ir2_opnd3(mips_or, &value_status, &value_status, &value_status_temp);
//
//    /* set C1 flag to 0 */
//    append_ir2_opnd2i(mips_andi, &value_status, &value_status, 0xfdffULL);
//    append_ir2_opnd2i(mips_sh, &value_status, &env_ir2_opnd,
//                      lsenv_offset_of_status_word(lsenv)); /* status_word */
    return true;
}
#endif

bool translate_fsin(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "FPU fsin to be implemented in LoongArch.\n");
//#ifdef USE_FSIN_HELPER
//    tr_gen_call_to_helper1((ADDR)helper_fsin);
//#else
//    /* dispose the arguments. because helper prologue will disable fpu top mode
//     * so we need to save the arguments to param registers(which is not affected by top)
//     * before it.
//     */
//    IR2_OPND param_0 = ir2_opnd_new(IR2_OPND_FREG, 12);
//    IR2_OPND st0_opnd = ra_alloc_st(0);
//    append_ir2_opnd2(mips_mov_d, &param_0, &st0_opnd);
//
//    /* save regs before call helper func */
//    int use_fp = 0;
//    tr_gen_call_to_helper_prologue(use_fp);
//
//    /* Call the function */
//    tr_gen_call_to_helper((ADDR)sin);
//
//    /* f0 is mapped to x86 reg now, it will be destroyed by the following registers load,
//     * save it
//     */
//    IR2_OPND ret_opnd, ret_value;
//    ret_opnd = ir2_opnd_new(IR2_OPND_FREG, 0);
//    ret_value = ra_alloc_ftemp();
//    append_ir2_opnd2(mips_mov_d, &ret_value, &ret_opnd);
//
//    /* restore regs after native call*/
//    tr_gen_call_to_helper_epilogue(use_fp);
//
//    append_ir2_opnd2(mips_mov_d, &st0_opnd, &ret_value);
//
//    /* clear c2 flag in status word */
//    IR2_OPND sw_value = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_lhu, &sw_value, &env_ir2_opnd,
//                      lsenv_offset_of_status_word(lsenv));
//    append_ir2_opnd2i(mips_andi, &sw_value, &sw_value, 0xfbff);
//    append_ir2_opnd2i(mips_sh, &sw_value, &env_ir2_opnd,
//                      lsenv_offset_of_status_word(lsenv));
//    ra_free_temp(&sw_value);
//#endif
    return true;
}

bool translate_fcos(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "FPU fcos to be implemented in LoongArch.\n");
//#ifdef USE_FCOS_HELPER
//    tr_gen_call_to_helper1((ADDR)helper_fcos);
//#else
//    IR2_OPND param_0 = ir2_opnd_new(IR2_OPND_FREG, 12);
//    IR2_OPND st0_opnd = ra_alloc_st(0);
//    append_ir2_opnd2(mips_mov_d, &param_0, &st0_opnd);
//
//    /* save regs before call helper func */
//    int use_fp = 0;
//    tr_gen_call_to_helper_prologue(use_fp);
//
//    /* Call the function */
//    tr_gen_call_to_helper((ADDR)cos);
//
//    /* f0 is mapped to x86 reg now, it will be destroyed by the following registers load,
//     * save it
//     */
//    IR2_OPND ret_opnd, ret_value;
//    ret_opnd = ir2_opnd_new(IR2_OPND_FREG, 0);
//    ret_value = ra_alloc_ftemp();
//    append_ir2_opnd2(mips_mov_d, &ret_value, &ret_opnd);
//
//    /* restore regs after native call*/
//    tr_gen_call_to_helper_epilogue(use_fp);
//
//    append_ir2_opnd2(mips_mov_d, &st0_opnd, &ret_value);
//
//    /* clear c2 flag in status word */
//    IR2_OPND sw_value = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_lhu, &sw_value, &env_ir2_opnd,
//                      lsenv_offset_of_status_word(lsenv));
//    append_ir2_opnd2i(mips_andi, &sw_value, &sw_value, 0xfbff);
//    append_ir2_opnd2i(mips_sh, &sw_value, &env_ir2_opnd,
//                      lsenv_offset_of_status_word(lsenv));
//    ra_free_temp(&sw_value);
//#endif
    return true;
}

double pi = +3.141592653589793239;

bool translate_fpatan(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "FPU fpatan to be implemented in LoongArch.\n");
//#ifdef USE_FPATAN_HELPER
//    tr_gen_call_to_helper1((ADDR)helper_fpatan);
//
//    if (!option_lsfpu)
//        tr_fpu_pop();
//#else
//    /* dispose the arguments */
//    IR2_OPND st0_opnd = ra_alloc_st(0);
//    IR2_OPND st1_opnd = ra_alloc_st(1);
//    IR2_OPND param_0 = ir2_opnd_new(IR2_OPND_FREG, 12);
//    append_ir2_opnd3(mips_div_d, &param_0, &st1_opnd, &st0_opnd);
//
//    /* save regs before call helper func */
//    int use_fp = 0;
//    tr_gen_call_to_helper_prologue(use_fp);
//
//    /* Call the function */
//    tr_gen_call_to_helper((ADDR)atan);
//
//    IR2_OPND ret_opnd, ret_value;
//    ret_opnd = ir2_opnd_new(IR2_OPND_FREG, 0);
//    ret_value = ra_alloc_ftemp();
//    append_ir2_opnd2(mips_mov_d, &ret_value, &ret_opnd);
//    /* restore regs after native call*/
//    tr_gen_call_to_helper_epilogue(use_fp);
//
//    IR2_OPND pi_base = ra_alloc_itemp();
//    load_addr_to_ir2(&pi_base, (uint64)(&pi));
//    IR2_OPND pi_value = ra_alloc_ftemp();
//    append_ir2_opnd2i(mips_ldc1, &pi_value, &pi_base, 0);
//    ra_free_temp(&pi_base);
//
//    IR2_OPND f_zero = ra_alloc_ftemp();
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &f_zero);
//    append_ir2_opnd2(mips_c_eq_d, &st1_opnd, &f_zero);
//    IR2_OPND label_st0_lt0 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_bc1f, &label_st0_lt0);
//    append_ir2_opnd2(mips_c_le_d, &f_zero, &st0_opnd);
//    IR2_OPND label_result_pi = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_bc1f, &label_result_pi);
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &ret_value);
//    IR2_OPND label_end = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_b, &label_end);
//
//    append_ir2_opnd1(mips_label, &label_result_pi);
//    append_ir2_opnd2(mips_mov_d, &ret_value, &pi_value);
//    append_ir2_opnd1(mips_b, &label_end);
//
//    append_ir2_opnd1(mips_label, &label_st0_lt0);
//    append_ir2_opnd2(mips_c_lt_d, &st0_opnd, &f_zero);
//    append_ir2_opnd1(mips_bc1f, &label_end);
//    append_ir2_opnd2(mips_c_lt_d, &f_zero, &st1_opnd);
//    IR2_OPND label_st1_ltN0 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_bc1f, &label_st1_ltN0);
//    append_ir2_opnd3(mips_add_d, &ret_value, &pi_value, &ret_value);
//
//    append_ir2_opnd1(mips_b, &label_end);
//    append_ir2_opnd1(mips_label, &label_st1_ltN0);
//    append_ir2_opnd2(mips_c_lt_d, &st1_opnd, &f_zero);
//    append_ir2_opnd1(mips_bc1f, &label_end);
//    append_ir2_opnd3(mips_sub_d, &ret_value, &ret_value, &pi_value);
//    append_ir2_opnd1(mips_label, &label_end);
//
//    tr_fpu_pop();
//    IR2_OPND new_st0_opnd = ra_alloc_st(0);
//    append_ir2_opnd2(mips_mov_d, &new_st0_opnd, &ret_value);
//    ra_free_temp(&ret_value);
//    ra_free_temp(&pi_value);
//    ra_free_temp(&f_zero);
//#endif
    return true;
}

bool translate_fprem(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif
    tr_gen_call_to_helper1((ADDR)helper_fprem, 1);

    return true;
}

bool translate_fprem1(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif
    tr_gen_call_to_helper1((ADDR)helper_fprem1, 1);

    return true;
}

bool translate_frndint(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "FPU frndint to be implemented in LoongArch.\n");
//#ifdef USE_HELPER_FRNDINT
//    tr_gen_call_to_helper1((ADDR)helper_frndint, 1);
//#else
//
//    IR2_OPND st0_opnd = ra_alloc_st(0);
//    IR2_OPND ret_value = ra_alloc_ftemp();
//    IR2_OPND temp_param_opnd = ra_alloc_itemp();
//
//    IR2_OPND rint_label_opnd   = ir2_opnd_new_type(IR2_OPND_LABEL); 
//    IR2_OPND trunc_label_opnd  = ir2_opnd_new_type(IR2_OPND_LABEL); 
//    IR2_OPND floor_label_opnd  = ir2_opnd_new_type(IR2_OPND_LABEL); 
//    IR2_OPND ceil_label_opnd   = ir2_opnd_new_type(IR2_OPND_LABEL); 
//    IR2_OPND exit_label_opnd   = ir2_opnd_new_type(IR2_OPND_LABEL); 
//    IR2_OPND invalid_label_opnd  = ir2_opnd_new_type(IR2_OPND_LABEL); 
//
//    /* estimate parameter, not process infinite value */
//    IR2_OPND temp_opnd = ra_alloc_itemp();
//    IR2_OPND temp_opnd_1 = ra_alloc_itemp();
//
//    /*load_ireg_from_imm64(&temp_opnd, (uint64)(0x7fffffffffffffffULL));
//      load_ireg_from_imm64(&temp_opnd_1, (uint64)(0x7ff0000000000000ULL));
//     */
//    append_ir2_opnd1i(mips_lui, &temp_opnd, -32768);
//    append_ir2_opnd2i(mips_dsll32, &temp_opnd, &temp_opnd, 0);
//    append_ir2_opnd2i(mips_daddiu, &temp_opnd, &temp_opnd, -1);
//
//    append_ir2_opnd1i(mips_lui, &temp_opnd_1, 0x7ff0);
//    append_ir2_opnd2i(mips_dsll32, &temp_opnd_1, &temp_opnd_1, 0);
//
//    append_ir2_opnd2(mips_dmfc1, &temp_param_opnd, &st0_opnd);
//    append_ir2_opnd3(mips_and, &temp_opnd, &temp_param_opnd, &temp_opnd);
//
//    append_ir2_opnd3_not_nop(mips_beq, &temp_opnd, &temp_opnd_1, &invalid_label_opnd);
//    append_ir2_opnd2(mips_mov_d, &ret_value, &st0_opnd);
//
//    /* get rc */
//    IR2_OPND rc_opnd = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_lh, &rc_opnd, &env_ir2_opnd, lsenv_offset_of_control_word(lsenv));
//    append_ir2_opnd2i(mips_srl, &rc_opnd, &rc_opnd, 10);
//    append_ir2_opnd2i(mips_andi, &rc_opnd, &rc_opnd, 3);
//    append_ir2_opnd3_not_nop(mips_beq, &rc_opnd, &zero_ir2_opnd, &rint_label_opnd);
//    append_ir2_opnd2i(mips_addiu, &rc_opnd, &rc_opnd, -1);
//    append_ir2_opnd3_not_nop(mips_beq, &rc_opnd, &zero_ir2_opnd, &floor_label_opnd);
//    append_ir2_opnd2i(mips_addiu, &rc_opnd, &rc_opnd, -1);
//    append_ir2_opnd3_not_nop(mips_beq, &rc_opnd, &zero_ir2_opnd, &ceil_label_opnd);
//    append_ir2_opnd2i(mips_addiu, &rc_opnd, &rc_opnd, -1);
//
//    append_ir2_opnd3(mips_beq, &rc_opnd, &zero_ir2_opnd, &trunc_label_opnd);
//
//    /* do rint() */
//    append_ir2_opnd1(mips_label, &rint_label_opnd); 
//    append_ir2_opnd2(mips_round_l_d, &ret_value, &ret_value);
//    append_ir2_opnd1(mips_b, &exit_label_opnd); 
//
//    /* do floor() */
//    append_ir2_opnd1(mips_label, &floor_label_opnd);
//    append_ir2_opnd2(mips_floor_l_d, &ret_value, &ret_value);
//    append_ir2_opnd1(mips_b, &exit_label_opnd);
//
//    /* do ceil() */
//    append_ir2_opnd1(mips_label, &ceil_label_opnd); 
//    append_ir2_opnd2(mips_ceil_l_d, &ret_value, &ret_value);
//    append_ir2_opnd1(mips_b, &exit_label_opnd); 
//
//    /* do trunc() */
//    append_ir2_opnd1(mips_label, &trunc_label_opnd); 
//    append_ir2_opnd2(mips_trunc_l_d, &ret_value, &ret_value);
//    append_ir2_opnd1(mips_b, &exit_label_opnd); 
//
//    /* exit */
//    append_ir2_opnd1(mips_label, &exit_label_opnd); 
//    append_ir2_opnd2(mips_cvt_d_l, &ret_value, &ret_value);
//    append_ir2_opnd2(mips_mov_d, &st0_opnd, &ret_value);
//
//    append_ir2_opnd1(mips_label, &invalid_label_opnd); 
//
//    ra_free_temp(&rc_opnd);
//    ra_free_temp(&temp_opnd);
//    ra_free_temp(&temp_opnd_1);
//    ra_free_temp(&temp_param_opnd);
//#endif

    return true;
}

bool translate_fscale(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif
    tr_gen_call_to_helper1((ADDR)helper_fscale, 1);

    return true;
}

bool translate_fxam(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif
    tr_gen_call_to_helper1((ADDR)helper_fxam_ST0, 1);

    return true;
}

bool translate_f2xm1(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif
    tr_gen_call_to_helper1((ADDR)helper_f2xm1, 1);

    return true;
}

bool translate_fxtract(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "FPU fxtract to be implemented in LoongArch.\n");
//#ifdef USE_FXTRACT_HELPER
//    tr_gen_call_to_helper1((ADDR)helper_fxtract, 1);
//
//    if (!option_lsfpu)
//        tr_fpu_push();
//#else
//    /* if the source operand is 0.0 or -0.0, goto label_zero for special process
//     */
//    IR2_OPND f_zero = ra_alloc_ftemp();
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &f_zero);
//    append_ir2_opnd2(mips_cvt_d_l, &f_zero, &f_zero);
//    IR2_OPND st0_opnd = ra_alloc_st(0);
//    append_ir2_opnd2(mips_c_eq_d, &st0_opnd, &f_zero);
//    IR2_OPND label_zero = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_bc1t, &label_zero);
//
//    IR2_OPND fp_zero = ra_alloc_itemp();
//    load_imm64_to_ir2(&fp_zero, 0x8000000000000000ull);
//    append_ir2_opnd2(mips_dmtc1, &fp_zero, &f_zero);
//    ra_free_temp(&fp_zero);
//    append_ir2_opnd2(mips_c_eq_d, &st0_opnd, &f_zero);
//    ra_free_temp(&f_zero);
//    append_ir2_opnd1(mips_bc1t, &label_zero);
//
//    /* store st0 onto native stack for further use */
//    IR2_OPND native_sp = ir2_opnd_new(IR2_OPND_IREG, 29);
//    append_ir2_opnd2i(mips_addi_addr, &native_sp, &native_sp, -8);
//    append_ir2_opnd2i(mips_sdc1, &st0_opnd, &native_sp, 0);
//
//    /* save regs before call helper func */
//    int use_fp = 0;
//    tr_gen_call_to_helper_prologue(use_fp);
//
//    IR2_OPND param_0 = ir2_opnd_new(IR2_OPND_FREG, 12);
//    append_ir2_opnd2(mips_mov_d, &param_0, &st0_opnd);
//
//    /* Call the function */
//    /* explicitly specify function prototype to select from overloaded
//     * prototypes */
//    double (*logb_p)(double) = logb;
//    tr_gen_call_to_helper((ADDR)logb_p);
//
//    IR2_OPND ret_value = ir2_opnd_new(IR2_OPND_FREG, 0);
//    append_ir2_opnd2(mips_mov_d, &st0_opnd, &ret_value);
//    /* store the new st0 value to env->st0 */
//    int fpr_index = (0 + td_fpu_get_top()) & 7;
//    append_ir2_opnd2i(mips_sdc1, &st0_opnd, &env_ir2_opnd,
//                      lsenv_offset_of_fpr(lsenv, fpr_index));
//
//    IR2_OPND imm2_opnd = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_daddiu, &imm2_opnd, &zero_ir2_opnd, 2);
//    append_ir2_opnd2(mips_mtc1, &imm2_opnd, &param_0);
//    append_ir2_opnd2(mips_cvt_d_w, &param_0, &param_0);
//    ra_free_temp(&imm2_opnd);
//
//    IR2_OPND param_1 =
//        ir2_opnd_new(IR2_OPND_FREG, 13); /* this is only right for n32 */
//    append_ir2_opnd2(mips_mov_d, &param_1, &st0_opnd);
//
//    /* Call the function */
//    /* explicitly specify function prototype to select from overloaded
//     * prototypes */
//    double (*pow_p)(double, double) = pow;
//    tr_gen_call_to_helper((ADDR)pow_p);
//
//    IR2_OPND ret_val = ra_alloc_ftemp();
//    append_ir2_opnd2(mips_mov_d, &ret_val, &ret_value);
//
//    /* restore regs after native call*/
//    tr_gen_call_to_helper_epilogue(use_fp);
//
//    IR2_OPND origin_st0_value = ra_alloc_ftemp();
//    append_ir2_opnd2i(mips_ldc1, &origin_st0_value, &native_sp, 0);
//    append_ir2_opnd2i(mips_addi_addr, &native_sp, &native_sp, 8);
//
//    append_ir2_opnd3(mips_div_d, &origin_st0_value, &origin_st0_value, &ret_val);
//    ra_free_temp(&ret_val);
//    tr_fpu_push();
//
//    IR2_OPND new_st0_opnd = ra_alloc_st(0);
//    append_ir2_opnd2(mips_mov_d, &new_st0_opnd, &origin_st0_value);
//    ra_free_temp(&origin_st0_value);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_b, &label_exit);
//
//    /* special process for 0.0 and -0.0 */
//    append_ir2_opnd1(mips_label, &label_zero);
//    IR2_OPND temp_st0 = ra_alloc_ftemp();
//    append_ir2_opnd2(mips_mov_d, &temp_st0, &st0_opnd);
//    /* the float register stack has been changed, now put -inf to ST1 and */
//    /* 0.0(-0.0) to ST0 */
//    IR2_OPND inf_opnd = ra_alloc_itemp();
//    load_imm64_to_ir2(&inf_opnd, 0xfff0000000000000ull);
//    IR2_OPND st1_opnd = ra_alloc_st(1);
//    append_ir2_opnd2(mips_dmtc1, &inf_opnd, &st1_opnd);
//    append_ir2_opnd2(mips_mov_d, &new_st0_opnd, &temp_st0);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//    ra_free_temp(&inf_opnd);
//    ra_free_temp(&temp_st0);
//#endif

    return true;
}

bool translate_fyl2x(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "FPU fyl2x to be implemented in LoongArch.\n");
//#ifdef USE_FYL2X_HELPER
//    tr_gen_call_to_helper1((ADDR)helper_fyl2x);
//
//    if (!option_lsfpu)
//        tr_fpu_pop();
//#else
//    IR2_OPND st0_opnd = ra_alloc_st(0);
//    /* dispose the arguments */
//    IR2_OPND param_0 = ir2_opnd_new(IR2_OPND_FREG, 12);
//    append_ir2_opnd2(mips_mov_d, &param_0, &st0_opnd);
//
//    /* save regs before call helper func */
//    int use_fp = 0;
//    tr_gen_call_to_helper_prologue(use_fp);
//
//    /* Call the function  */
//    tr_gen_call_to_helper((ADDR)log2);
//
//    IR2_OPND ret_opnd, ret_value;
//    ret_opnd = ir2_opnd_new(IR2_OPND_FREG, 0);
//    ret_value = ra_alloc_ftemp();
//    append_ir2_opnd2(mips_mov_d, &ret_value, &ret_opnd);
//
//    /* restore regs after native call*/
//    tr_gen_call_to_helper_epilogue(use_fp);
//
//    append_ir2_opnd2(mips_mov_d, &st0_opnd, &ret_value);
//
//    IR2_OPND st1_opnd = ra_alloc_st(1);
//
//    append_ir2_opnd3(mips_mul_d, &st1_opnd, &st0_opnd, &st1_opnd);
//
//    tr_fpu_pop();
//#endif
//
    return true;
}

bool translate_fyl2xp1(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "FPU fyl2xp1 to be implemented in LoongArch.\n");
//#ifdef USE_FYL2XP1_HELPER
//    tr_gen_call_to_helper1((ADDR)helper_fyl2xp1);
//
//    if (!option_lsfpu)
//        tr_fpu_pop();
//#else
//    IR2_OPND st0_opnd = ra_alloc_st(0);
//
//    /* dispose the arguments */
//    IR2_OPND param_0 = ir2_opnd_new(IR2_OPND_FREG, 12);
//    IR2_OPND itemp_1 = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_ori, &itemp_1, &zero_ir2_opnd, 1);
//    append_ir2_opnd2(mips_dmtc1, &itemp_1, &param_0);
//    append_ir2_opnd2(mips_cvt_d_l, &param_0, &param_0);
//    append_ir2_opnd3(mips_add_d, &param_0, &st0_opnd, &param_0);
//    ra_free_temp(&itemp_1);
//
//    /* save regs before call helper func */
//    int use_fp = 0;
//    tr_gen_call_to_helper_prologue(use_fp);
//
//    /* Call the function  */
//    tr_gen_call_to_helper((ADDR)log2);
//
//    IR2_OPND ret_opnd, ret_value;
//    ret_opnd = ir2_opnd_new(IR2_OPND_FREG, 0);
//    ret_value = ra_alloc_ftemp();
//    append_ir2_opnd2(mips_mov_d, &ret_value, &ret_opnd);
//
//    /* restore regs after native call*/
//    tr_gen_call_to_helper_epilogue(use_fp);
//
//    append_ir2_opnd2(mips_mov_d, &st0_opnd, &ret_value);
//
//    IR2_OPND st1_opnd = ra_alloc_st(1);
//
//    append_ir2_opnd3(mips_mul_d, &st1_opnd, &st0_opnd, &st1_opnd);
//
//    tr_fpu_pop();
//#endif

    return true;
}

bool translate_fsincos(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "FPU fsincos to be implemented in LoongArch.\n");
//#ifdef USE_FSINCOS_HELPER
//    tr_gen_call_to_helper1((ADDR)helper_fsincos);
//
//    if (!option_lsfpu)
//        tr_fpu_push();
//#else
//    /* set status_word.c2 = 0 */
//    IR2_OPND status_word = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_lhu, &status_word, &env_ir2_opnd,
//                      lsenv_offset_of_status_word(lsenv));
//    append_ir2_opnd2i(mips_andi, &status_word, &status_word, ~0x400);
//    append_ir2_opnd2i(mips_sh, &status_word, &env_ir2_opnd,
//                      lsenv_offset_of_status_word(lsenv));
//
//    /* dispose the arguments */
//    IR2_OPND param_1 = ir2_opnd_new(IR2_OPND_FREG, 12);
//    IR2_OPND st0_opnd = ra_alloc_st(0);
//    append_ir2_opnd2(mips_mov_d, &param_1, &st0_opnd);
//
//    /* save regs before call helper func */
//    int use_fp = 0;
//    tr_gen_call_to_helper_prologue(use_fp);
//
//    /* should use ra_alloc_itemp, but since a1/a2 is temp registers too they
//       might conflict with it. We use unused a0 instead.
//     */
//    IR2_OPND top_opnd = ir2_opnd_new(IR2_OPND_IREG, 4);
//    IR2_OPND param_2 = ir2_opnd_new(IR2_OPND_IREG, 5);
//    /* fpreg_base + top * 16 */
//    append_ir2_opnd2i(mips_lw, &top_opnd, &env_ir2_opnd, lsenv_offset_of_top(lsenv));
//    append_ir2_opnd2i(mips_sll, &param_2, &top_opnd, 4);
//    append_ir2_opnd3(mips_add_addr, &param_2, &param_2, &env_ir2_opnd);
//    append_ir2_opnd2i(mips_addi_addr, &param_2, &param_2, lsenv_offset_of_fpr(lsenv, 0));
//
//    IR2_OPND param_3 = ir2_opnd_new(IR2_OPND_IREG, 6);
//    append_ir2_opnd2i(mips_addiu, &top_opnd, &top_opnd, -1);
//    append_ir2_opnd2i(mips_andi, &top_opnd, &top_opnd, 0x7);
//    append_ir2_opnd2i(mips_sll, &param_3, &top_opnd, 4);
//    append_ir2_opnd3(mips_add_addr, &param_3, &param_3, &env_ir2_opnd);
//    append_ir2_opnd2i(mips_addi_addr, &param_3, &param_3, lsenv_offset_of_fpr(lsenv, 0));
//
//    /* Call the function  */
//    tr_gen_call_to_helper((ADDR)sincos);
//
//    /* restore regs after helper call */
//    tr_gen_call_to_helper_epilogue(use_fp);
//
//    tr_fpu_push();
//#endif
//
    return true;
}

bool translate_fxch(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif

    IR2_OPND st0_opnd = ra_alloc_st(0);
    int opnd2_index;
    if (ir1_opnd_num(pir1) == 0) {
        opnd2_index = 1;
    } else {
        opnd2_index = ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0));
    }
    IR2_OPND opnd2 = ra_alloc_st(opnd2_index);
    IR2_OPND tmp_opnd = ra_alloc_ftemp();
    append_ir2_opnd2(LISA_FMOV_D, &tmp_opnd, &opnd2);
    append_ir2_opnd2(LISA_FMOV_D, &opnd2, &st0_opnd);
    append_ir2_opnd2(LISA_FMOV_D, &st0_opnd, &tmp_opnd);
    ra_free_temp(&tmp_opnd);
    return true;
}
bool translate_ftst(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "FPU ftst to be implemented in LoongArch.\n");
//    IR2_OPND status_word = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_lh, &status_word, &env_ir2_opnd,
//                      lsenv_offset_of_status_word(lsenv));
//    append_ir2_opnd2i(mips_andi, &status_word, &status_word, 0xbaff);
//
//    IR2_OPND f_zero = ra_alloc_ftemp();
//    append_ir2_opnd2(mips_mtc1, &zero_ir2_opnd, &f_zero);
//    append_ir2_opnd2(mips_cvt_d_w, &f_zero, &f_zero);
//
//    IR2_OPND st0_opnd = ra_alloc_st(0);
//    IR2_OPND label_for_lt = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_for_un = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_for_eq = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_for_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    /* check for unordered */
//    append_ir2_opnd2(mips_c_un_d, &st0_opnd, &f_zero);
//    append_ir2_opnd1(mips_bc1t, &label_for_un);
//
//    /* check for equal */
//    append_ir2_opnd2(mips_c_eq_d, &st0_opnd, &f_zero);
//    append_ir2_opnd1(mips_bc1t, &label_for_eq);
//
//    /* check for less than */
//    append_ir2_opnd2(mips_c_lt_d, &st0_opnd, &f_zero);
//    append_ir2_opnd1(mips_bc1t, &label_for_lt);
//
//    /* greater than */
//    append_ir2_opnd1(mips_b, &label_for_exit);
//    /* lt: */
//    append_ir2_opnd1(mips_label, &label_for_lt);
//    append_ir2_opnd2i(mips_ori, &status_word, &status_word, 0x0100);
//    append_ir2_opnd1(mips_b, &label_for_exit);
//    /* eq: */
//    append_ir2_opnd1(mips_label, &label_for_eq);
//    append_ir2_opnd2i(mips_ori, &status_word, &status_word, 0x4000);
//    append_ir2_opnd1(mips_b, &label_for_exit);
//    /* un: */
//    append_ir2_opnd1(mips_label, &label_for_un);
//    append_ir2_opnd2i(mips_ori, &status_word, &status_word, 0x4500);
//    /* exit: */
//    append_ir2_opnd1(mips_label, &label_for_exit);
//
//    append_ir2_opnd2i(mips_sh, &status_word, &env_ir2_opnd,
//                      lsenv_offset_of_status_word(lsenv));
//    ra_free_temp(&status_word);
//    ra_free_temp(&f_zero);

    return true;
}

bool translate_fptan(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_fp_common_excp_check(pir1)) return true;
#endif
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
}

#ifndef CONFIG_SOFTMMU
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
    fprintf(stderr, "%s not implemented. translation failed.\n", __FUNCTION__);
    return false;
}
bool translate_frstor(IR1_INST *pir1)
{
    fprintf(stderr, "%s not implemented. translation failed.\n", __FUNCTION__);
    return false;
}
bool translate_fbld(IR1_INST *pir1)
{
    fprintf(stderr, "%s not implemented. translation failed.\n", __FUNCTION__);
    return false;
}
bool translate_fbstp(IR1_INST *pir1)
{
    fprintf(stderr, "%s not implemented. translation failed.\n", __FUNCTION__);
    return false;
}
bool translate_fxsave(IR1_INST *pir1)
{
    fprintf(stderr, "%s not implemented. translation failed.\n", __FUNCTION__);
    return false;
}
bool translate_fxrstor(IR1_INST *pir1)
{
    fprintf(stderr, "%s not implemented. translation failed.\n", __FUNCTION__);
    return false;
}
#endif
