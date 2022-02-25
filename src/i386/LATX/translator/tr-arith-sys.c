#include "../include/common.h"
#include "../include/env.h"
#include "../include/reg-alloc.h"
#include "../x86tomips-options.h"

/* tr_arich.c in system mode
 *
 * 1. translation should follow precise exception
 * 2. generate illop exception instead of exit()
 * 3. div zero should generate DIVZ exception
 *
 * *. only i386 is considered
 * *. only N64 is considered */

bool translate_add(IR1_INST *pir1)
{
//    lsassertm(0, "ARITH add to be implemented in LoongArch.\n");
    if (option_by_hand) return translate_add_byhand(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src_opnd_0 = ra_alloc_itemp();
    IR2_OPND src_opnd_1 = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, opnd0, SIGN_EXTENSION, false);
    load_ir1_to_ir2(&src_opnd_1, opnd1, SIGN_EXTENSION, false);

    IR2_OPND dest_opnd = ra_alloc_itemp();
    append_ir2_opnd3(LISA_ADD_W, &dest_opnd, &src_opnd_0, &src_opnd_1);

    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&dest_opnd, opnd0, false);
        /* calculate elfags after store */
//#ifndef CONFIG_SOFTMMU
//        fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);
//#endif
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
    }
    else {
        /* Destination is GPR, no exception will generate */
//#ifndef CONFIG_SOFTMMU
//        fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);
//#endif
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
        store_ir2_to_ir1(&dest_opnd, opnd0, false);
    }

    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_adc(IR1_INST *pir1)
{
//    lsassertm(0, "ARITH adc to be implemented in LoongArch.\n");
    if (option_by_hand) return translate_adc_byhand(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src_opnd_0 = ra_alloc_itemp();
    IR2_OPND src_opnd_1 = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, opnd0, SIGN_EXTENSION, false);
    load_ir1_to_ir2(&src_opnd_1, opnd1, SIGN_EXTENSION, false);

    IR2_OPND dest_opnd = ra_alloc_itemp();
    load_eflags_cf_to_ir2(&dest_opnd);
    append_ir2_opnd3(LISA_ADD_W, &dest_opnd, &dest_opnd, &src_opnd_0);
    append_ir2_opnd3(LISA_ADD_W, &dest_opnd, &dest_opnd, &src_opnd_1);

    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&dest_opnd, opnd0, false);
        /* calculate elfags after store */
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
    }
    else {
        /* Destination is GPR, no exception will generate */
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
        store_ir2_to_ir1(&dest_opnd, opnd0, false);
    }

    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_inc(IR1_INST *pir1)
{
//    lsassertm(0, "ARITH inc to be implemented in LoongArch.\n");
    IR1_OPND *opnd0     = ir1_get_opnd(pir1, 0);
    IR2_OPND src_opnd_0 = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, opnd0, SIGN_EXTENSION, false);

    IR2_OPND dest_opnd = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ADDI_W, &dest_opnd, &src_opnd_0, 1);

    IR2_OPND t_imm1 = ir2_opnd_new(IR2_OPND_IMMH, 1);

    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&dest_opnd, opnd0, false);
        /* calculate elfags after store */
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &t_imm1, pir1, true);
    }
    else {
        /* Destination is GPR, no exception will generate */
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &t_imm1, pir1, true);
        store_ir2_to_ir1(&dest_opnd, opnd0, false);
    }

    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_dec(IR1_INST *pir1)
{
//    lsassertm(0, "ARITH dec to be implemented in LoongArch.\n");
    IR1_OPND *opnd0     = ir1_get_opnd(pir1, 0);
    IR2_OPND src_opnd_0 = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, opnd0, SIGN_EXTENSION, false);

    IR2_OPND dest_opnd = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ADDI_W, &dest_opnd, &src_opnd_0, -1);

    IR2_OPND t_imm1 = ir2_opnd_new(IR2_OPND_IMMH, 1);

    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&dest_opnd, opnd0, false);
        /* calculate elfags after store */
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &t_imm1, pir1, true);
    }
    else {
        /* Destination is GPR, no exception will generate */
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &t_imm1, pir1, true);
        store_ir2_to_ir1(&dest_opnd, opnd0, false);
    }

    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_sub(IR1_INST *pir1)
{
//    lsassertm(0, "ARITH sub to be implemented in LoongArch.\n");
    if (option_by_hand) return translate_sub_byhand(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src_opnd_0 = ra_alloc_itemp();
    IR2_OPND src_opnd_1 = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, opnd0, SIGN_EXTENSION, false);
    load_ir1_to_ir2(&src_opnd_1, opnd1, SIGN_EXTENSION, false);

    IR2_OPND dest_opnd = ra_alloc_itemp();
    append_ir2_opnd3(LISA_SUB_W, &dest_opnd, &src_opnd_0, &src_opnd_1);

    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
        /* calculate elfags after store */
//#ifndef CONFIG_SOFTMMU
//        fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);
//#endif
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
    }
    else {
        /* Destination is GPR, no exception will generate */
//#ifndef CONFIG_SOFTMMU
//        fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);
//#endif
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
        store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
    }

    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_sbb(IR1_INST *pir1)
{
//    lsassertm(0, "ARITH sbb to be implemented in LoongArch.\n");
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src_opnd_0 = ra_alloc_itemp();
    IR2_OPND src_opnd_1 = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, opnd0, SIGN_EXTENSION, false);
    load_ir1_to_ir2(&src_opnd_1, opnd1, SIGN_EXTENSION, false);

    IR2_OPND dest_opnd = ra_alloc_itemp();
    IR2_OPND tmp_opnd  = dest_opnd;
    load_eflags_cf_to_ir2(&dest_opnd);
    if (ir2_opnd_cmp(&src_opnd_0, &src_opnd_1)) {
        append_ir2_opnd3(LISA_SUB_W, &src_opnd_0, &zero_ir2_opnd, &dest_opnd);
        dest_opnd = src_opnd_0;
    } else {
        append_ir2_opnd3(LISA_SUB_W, &dest_opnd, &src_opnd_0, &dest_opnd);
        bool dest_opnd_is_temp = ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) ||
                                 ir1_need_calculate_of(pir1) ||
                                 ir1_need_calculate_cf(pir1);
        if (!dest_opnd_is_temp)
            dest_opnd = src_opnd_0;
        append_ir2_opnd3(LISA_SUB_W, &dest_opnd, &tmp_opnd, &src_opnd_1);
    }

    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&dest_opnd, opnd0, false);
        /* calculate elfags after store */
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
    }
    else {
        /* Destination is GPR, no exception will generate */
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
        store_ir2_to_ir1(&dest_opnd, opnd0, false);
    }

    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_neg(IR1_INST *pir1)
{
//    lsassertm(0, "ARITH neg to be implemented in LoongArch.\n");
    IR1_OPND *opnd0     = ir1_get_opnd(pir1, 0);
    IR2_OPND src_opnd_0 = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, opnd0, SIGN_EXTENSION, false);

    IR2_OPND dest_opnd = ra_alloc_itemp();
    append_ir2_opnd3(LISA_SUB_W, &dest_opnd, &zero_ir2_opnd, &src_opnd_0);

    if (ir1_opnd_is_mem(opnd0)) {
        store_ir2_to_ir1(&dest_opnd, opnd0, false);
        /* calculate elfags after store */
        generate_eflag_calculation(&dest_opnd, &zero_ir2_opnd, &src_opnd_0, pir1, true);
    }
    else {
        /* Destination is GPR, no exception will generate */
        generate_eflag_calculation(&dest_opnd, &zero_ir2_opnd, &src_opnd_0, pir1, true);
        store_ir2_to_ir1(&dest_opnd, opnd0, false);
    }

    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_cmp(IR1_INST *pir1)
{
//    lsassertm(0, "ARITH cmp to be implemented in LoongArch.\n");
    if (option_by_hand) return translate_cmp_byhand(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src_opnd_0 = ra_alloc_itemp();
    IR2_OPND src_opnd_1 = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, opnd0, SIGN_EXTENSION, false);
    load_ir1_to_ir2(&src_opnd_1, opnd1, SIGN_EXTENSION, false);

    IR2_OPND dest_opnd = ra_alloc_itemp();
    append_ir2_opnd3(LISA_SUB_W, &dest_opnd, &src_opnd_0, &src_opnd_1);

    generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);

    ra_free_temp(&dest_opnd);

    return true;
}

bool translate_mul(IR1_INST *pir1)
{
//    lsassertm(0, "ARITH mul to be implemented in LoongArch.\n");
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND src_opnd_0 = ra_alloc_itemp();
    IR2_OPND src_opnd_1 = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, opnd0, ZERO_EXTENSION, false);

    IR2_OPND dest_opnd  = ra_alloc_itemp();

    int opnd_size = ir1_opnd_size(opnd0);
    switch (opnd_size) {
    case 8:
        load_ir1_gpr_to_ir2(&src_opnd_1, &al_ir1_opnd, ZERO_EXTENSION);
        append_ir2_opnd3(LISA_MUL_W, &dest_opnd, &src_opnd_1, &src_opnd_0);
        ra_free_temp(&src_opnd_0);
        ra_free_temp(&src_opnd_1);
        store_ir2_to_ir1(&dest_opnd, &ax_ir1_opnd, false);
        break;
    case 16:
        load_ir1_gpr_to_ir2(&src_opnd_1, &ax_ir1_opnd, ZERO_EXTENSION);
        append_ir2_opnd3(LISA_MUL_W, &dest_opnd, &src_opnd_1, &src_opnd_0);
        ra_free_temp(&src_opnd_0);
        ra_free_temp(&src_opnd_1);
        store_ir2_to_ir1(&dest_opnd, &ax_ir1_opnd, false);
        append_ir2_opnd2i(LISA_SRLI_W, &dest_opnd, &dest_opnd, 16);
        store_ir2_to_ir1(&dest_opnd, &dx_ir1_opnd, false);
        break;
    case 32:
        load_ir1_gpr_to_ir2(&src_opnd_1, &eax_ir1_opnd, ZERO_EXTENSION);
        append_ir2_opnd3(LISA_MUL_D, &dest_opnd, &src_opnd_1, &src_opnd_0);
        ra_free_temp(&src_opnd_0);
        ra_free_temp(&src_opnd_1);
        store_ir2_to_ir1(&dest_opnd, &eax_ir1_opnd, false);
        append_ir2_opnd2i(LISA_SRLI_D, &dest_opnd, &dest_opnd, 32);
        store_ir2_to_ir1(&dest_opnd, &edx_ir1_opnd, false);
        break;
    default:
        lsassertm_illop(ir1_addr(pir1), 0,
                "64-bit translate_mul is unimplemented.\n");
        break;
    }

    generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);

    ra_free_temp(&dest_opnd);
    return true;
}

static bool translate_imul_1_opnd(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND src_opnd_0 = ra_alloc_itemp();
    IR2_OPND src_opnd_1 = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, opnd0, SIGN_EXTENSION, false);

    IR2_OPND dest_opnd = ra_alloc_itemp();

    /* Destination is always GPR, no exception will generate */
    int opnd_size = ir1_opnd_size(opnd0);
    switch (opnd_size) {
    case 8:
        load_ir1_gpr_to_ir2(&src_opnd_1, &al_ir1_opnd, SIGN_EXTENSION);
        append_ir2_opnd3(LISA_MUL_W, &dest_opnd, &src_opnd_1, &src_opnd_0);
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
        ra_free_temp(&src_opnd_0);
        ra_free_temp(&src_opnd_1);
        store_ir2_to_ir1(&dest_opnd, &ax_ir1_opnd, false);
        break;
    case 16:
        load_ir1_gpr_to_ir2(&src_opnd_1, &ax_ir1_opnd, SIGN_EXTENSION);
        append_ir2_opnd3(LISA_MUL_W, &dest_opnd, &src_opnd_1, &src_opnd_0);
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
        ra_free_temp(&src_opnd_0);
        ra_free_temp(&src_opnd_1);
        store_ir2_to_ir1(&dest_opnd, &ax_ir1_opnd, false);
        append_ir2_opnd2i(LISA_SRLI_D, &dest_opnd, &dest_opnd, 16);
        store_ir2_to_ir1(&dest_opnd, &dx_ir1_opnd, false);
        break;
    case 32:
        load_ir1_gpr_to_ir2(&src_opnd_1, &eax_ir1_opnd, SIGN_EXTENSION);
        append_ir2_opnd3(LISA_MUL_D, &dest_opnd, &src_opnd_1, &src_opnd_0);
        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
        ra_free_temp(&src_opnd_0);
        ra_free_temp(&src_opnd_1);
        store_ir2_to_ir1(&dest_opnd, &eax_ir1_opnd, false);
        append_ir2_opnd2i(LISA_SRLI_D, &dest_opnd, &dest_opnd, 32);
        store_ir2_to_ir1(&dest_opnd, &edx_ir1_opnd, false);
        break;
    default:
        lsassertm_illop(ir1_addr(pir1), 0,
                "64-bit translate_imul_1_opnd is unimplemented.\n");
        break;
    }

    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_imul(IR1_INST *pir1)
{
//    lsassertm(0, "ARITH imul to be implemented in LoongArch.\n");
    if (ir1_opnd_num(pir1) == 1)
        return translate_imul_1_opnd(pir1);

    IR1_OPND *srcopnd0 = ir1_get_src_opnd(pir1, 0);
    IR1_OPND *srcopnd1 = ir1_get_src_opnd(pir1, 1);

    IR2_OPND src_opnd_0 = ra_alloc_itemp();
    IR2_OPND src_opnd_1 = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, srcopnd0, SIGN_EXTENSION, false);
    load_ir1_to_ir2(&src_opnd_1, srcopnd1, SIGN_EXTENSION, false);

    IR2_OPND dest_opnd = ra_alloc_itemp();

    int opnd_size = ir1_opnd_size(ir1_get_opnd(pir1, 0));

    lsassertm_illop(ir1_addr(pir1), opnd_size != 64,
            "64-bit translate_imul is unimplemented.\n");

    if (opnd_size == 32) {
        append_ir2_opnd3(LISA_MUL_D, &dest_opnd, &src_opnd_1, &src_opnd_0);
    } else {
        append_ir2_opnd3(LISA_MUL_W, &dest_opnd, &src_opnd_1, &src_opnd_0);
    }
    ra_free_temp(&src_opnd_0);
    ra_free_temp(&src_opnd_1);

    generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);

    /* Destination is always GPR, no exception will generate */
    if (ir1_opnd_num(pir1) == 3)
        store_ir2_to_ir1(&dest_opnd, ir1_get_dest_opnd(pir1, 0), false);
    else
        store_ir2_to_ir1(&dest_opnd, ir1_get_src_opnd(pir1, 0), false);

    ra_free_temp(&dest_opnd);
    return true;
}

static void tr_gen_div_result_check(
        IR1_INST *pir1,
        IR2_OPND result, int size, int is_idiv)
{
    /* u8   [0,     2^8 -1] = [0x0,         0xff        ]
     * u16  [0,     2^16-1] = [0x0,         0xffff      ]
     * u32  [0,     2^32-1] = [0x0,         0xffff_ffff ]
     * s8   [-2^7,  2^7 -1] = [0xffff_ff80, 0x7f        ]
     * s16  [-2^15, 2^15-1] = [0xffff_8000, 0x7fff      ]
     * s32  [-2^31, 2^31-1] = [0x8000_0000, 0x7fff_ffff ] */
    IR2_OPND label_no_excp = ir2_opnd_new_label();
    IR2_OPND tmp = ra_alloc_itemp();

    /* 1. chechk and branch to 'label_no_excp' */
    switch (size) {
    case 8:
        if (is_idiv) { /* signed 8-bit */
            append_ir2_opnd2_(lisa_mov8s, &tmp, &result);
        } else { /* unsigned 8-bit */
            append_ir2_opnd2_(lisa_mov8z, &tmp, &result);
        }
        break;
    case 16:
        if (is_idiv) { /* signed 16-bit */
            append_ir2_opnd2_(lisa_mov16s, &tmp, &result);
        } else { /* unsigned 16-bit */
            append_ir2_opnd2_(lisa_mov16z, &tmp, &result);
        }
        break;
    case 32:
        if (is_idiv) { /* signed 32-bit */
            append_ir2_opnd2_(lisa_mov32s, &tmp, &result);
        } else { /* unsigned 32-bit */
            append_ir2_opnd2_(lisa_mov32z, &tmp, &result);
        }
        break;
    default:
        /* should never reach here */
        lsassertm(0, "should never reach here [%s:%d].\n", __func__, __LINE__);
        break;
    }
    append_ir2_opnd3(LISA_BEQ, &result, &tmp, &label_no_excp);
    ra_free_temp(&tmp);

    /* 2. not branch: generate exception */
    tr_gen_excp_divz(pir1, 0);

    /* 3. branch: no exception */
    append_ir2_opnd1(LISA_LABEL, &label_no_excp);
}

bool translate_div(IR1_INST *pir1)
{
//    lsassertm(0, "ARITH div to be implemented in LoongArch.\n");
    IR1_OPND *opnd0     = ir1_get_opnd(pir1, 0);

    IR2_OPND src_opnd_0 = ra_alloc_itemp();
    IR2_OPND src_opnd_1 = ra_alloc_itemp();
    IR2_OPND src_opnd_2 = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, opnd0, ZERO_EXTENSION, false);

    /* 1. if div zero, generate DIVZ exception */
    IR2_OPND label_not_zero = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BNE, &src_opnd_0, &zero_ir2_opnd, &label_not_zero);
    /* 1.1 not branch : generate exception and never return */
    tr_gen_excp_divz(pir1, 0);
    /* 1.2 branch: go on finish div operation */
    append_ir2_opnd1(LISA_LABEL, &label_not_zero);

    IR2_OPND div_res = ra_alloc_itemp();
    IR2_OPND div_mod = ir2_opnd_new_inv();

    IR2_OPND src_tmp = ir2_opnd_new_inv();

    /* 2. do div operation */
    int opnd_size = ir1_opnd_size(opnd0);
    switch (opnd_size) {
    case 8:
        load_ir1_gpr_to_ir2(&src_opnd_1, &ax_ir1_opnd, ZERO_EXTENSION);
        div_mod = ra_alloc_itemp();

        /* 2.1 do div operation and get result */
        append_ir2_opnd3(LISA_DIV_DU, &div_res, &src_opnd_1, &src_opnd_0);
        append_ir2_opnd3(LISA_MOD_DU, &div_mod, &src_opnd_1, &src_opnd_0);
        ra_free_temp(&src_opnd_0);
        ra_free_temp(&src_opnd_1);

        /* 2.2 check if result is valid and generate exception if not */
        tr_gen_div_result_check(pir1, div_res, 8, 0);
        /* 2.3 store the results into destination */
        append_ir2_opnd2i(LISA_SLLI_D, &div_mod, &div_mod, 8);
        append_ir2_opnd3(LISA_OR, &div_res, &div_res, &div_mod);
        ra_free_temp(&div_mod);
        store_ir2_to_ir1(&div_res, &ax_ir1_opnd, false);
        break;
    case 16:
        load_ir1_gpr_to_ir2(&src_opnd_1, &ax_ir1_opnd, ZERO_EXTENSION);
        load_ir1_gpr_to_ir2(&src_opnd_2, &dx_ir1_opnd, ZERO_EXTENSION);
        src_tmp = ra_alloc_itemp();
        div_mod = ra_alloc_itemp();

        /* 2.1 do div operation and get result */
        append_ir2_opnd2i(LISA_SLLI_D, &src_tmp, &src_opnd_2, 16);
        append_ir2_opnd3 (LISA_OR,     &src_tmp, &src_tmp,    &src_opnd_1);
        ra_free_temp(&src_opnd_1);
        ra_free_temp(&src_opnd_2);

        append_ir2_opnd3(LISA_DIV_DU, &div_res, &src_tmp, &src_opnd_0);
        append_ir2_opnd3(LISA_MOD_DU, &div_mod, &src_tmp, &src_opnd_0);
        ra_free_temp(&src_tmp);
        ra_free_temp(&src_opnd_0);

        /* 2.2 check if result is valid and generate exception if not */
        tr_gen_div_result_check(pir1, div_res, 16, 0);
        /* 2.3 store the results into destination */
        store_ir2_to_ir1(&div_mod, &dx_ir1_opnd, false);
        ra_free_temp(&div_mod);
        store_ir2_to_ir1(&div_res, &ax_ir1_opnd, false);
        break;
    case 32:
        load_ir1_gpr_to_ir2(&src_opnd_1, &eax_ir1_opnd, ZERO_EXTENSION);
        load_ir1_gpr_to_ir2(&src_opnd_2, &edx_ir1_opnd, UNKNOWN_EXTENSION);
        src_tmp = ra_alloc_itemp();
        div_mod = ra_alloc_itemp();

        /* 2.1 do div operation and get result */
        append_ir2_opnd2i(LISA_SLLI_D, &src_tmp, &src_opnd_2, 32);
        append_ir2_opnd3 (LISA_OR,     &src_tmp, &src_tmp,    &src_opnd_1);
        ra_free_temp(&src_opnd_1);
        ra_free_temp(&src_opnd_2);

        append_ir2_opnd3(LISA_DIV_DU, &div_res, &src_tmp, &src_opnd_0);
        append_ir2_opnd3(LISA_MOD_DU, &div_mod, &src_tmp, &src_opnd_0);
        ra_free_temp(&src_tmp);
        ra_free_temp(&src_opnd_0);

        /* 2.2 check if result is valid and generate exception if not */
        tr_gen_div_result_check(pir1, div_res, 32, 0);
        /* 2.3 store the results into destination */
        store_ir2_to_ir1(&div_mod, &edx_ir1_opnd, false);
        ra_free_temp(&div_mod);
        store_ir2_to_ir1(&div_res, &eax_ir1_opnd, false);
        break;
    default:
        lsassertm_illop(ir1_addr(pir1), 0,
                "64-bit translate_div is unimplemented.\n");
        break;
    }

    ra_free_temp(&div_res);
    return true;
}

bool translate_idiv(IR1_INST *pir1)
{
//    lsassertm(0, "ARITH idiv to be implemented in LoongArch.\n");
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND src_opnd_0 = ra_alloc_itemp();
    IR2_OPND src_opnd_1 = ra_alloc_itemp();
    IR2_OPND src_opnd_2 = ra_alloc_itemp();

    load_ir1_to_ir2(&src_opnd_0, opnd0, SIGN_EXTENSION, false);

    /* 1. if div zero, generate DIVZ exception */
    IR2_OPND label_not_zero = ir2_opnd_new_label();
    append_ir2_opnd3(LISA_BNE, &src_opnd_0, &zero_ir2_opnd, &label_not_zero);
    /* 1.1 not branch : generate exception and never return */
    tr_gen_excp_divz(pir1, 0);
    /* 1.2 branch: go on finish div operation */
    append_ir2_opnd1(LISA_LABEL, &label_not_zero);

    IR2_OPND div_res = ra_alloc_itemp();
    IR2_OPND div_mod = ir2_opnd_new_inv();

    IR2_OPND src_tmp = ir2_opnd_new_inv();

    /* 2. do div operation */
    int opnd_size = ir1_opnd_size(opnd0);
    switch (opnd_size) {
    case 8:
        load_ir1_gpr_to_ir2(&src_opnd_1, &ax_ir1_opnd, SIGN_EXTENSION);
        div_mod = ra_alloc_itemp();

        /* 2.1 do div operation and get result */
        append_ir2_opnd3(LISA_DIV_D, &div_res, &src_opnd_1, &src_opnd_0);
        append_ir2_opnd3(LISA_MOD_D, &div_mod, &src_opnd_1, &src_opnd_0);
        ra_free_temp(&src_opnd_0);
        ra_free_temp(&src_opnd_1);

        /* 2.2 check if result is valid and generate exception if not */
        tr_gen_div_result_check(pir1, div_res, 8, 1);
        /* 2.3 store the results into destination */
        append_ir2_opnd2i(LISA_SLLI_D, &div_mod, &div_mod, 8);
        append_ir2_opnd3 (LISA_OR, &div_res, &div_res, &div_mod);
        ra_free_temp(&div_mod);
        store_ir2_to_ir1(&div_res, &ax_ir1_opnd, false);
        break;
    case 16:
        load_ir1_gpr_to_ir2(&src_opnd_1, &ax_ir1_opnd, ZERO_EXTENSION);
        load_ir1_gpr_to_ir2(&src_opnd_2, &dx_ir1_opnd, SIGN_EXTENSION);
        src_tmp = ra_alloc_itemp();
        div_mod = ra_alloc_itemp();

        /* 2.1 do div operation and get result */
        append_ir2_opnd2i(LISA_SLLI_D, &src_tmp, &src_opnd_2, 16);
        append_ir2_opnd3 (LISA_OR,     &src_tmp, &src_tmp,    &src_opnd_1);
        ra_free_temp(&src_opnd_1);
        ra_free_temp(&src_opnd_2);

        append_ir2_opnd3(LISA_DIV_D, &div_res, &src_tmp, &src_opnd_0);
        append_ir2_opnd3(LISA_MOD_D, &div_mod, &src_tmp, &src_opnd_0);
        ra_free_temp(&src_tmp);
        ra_free_temp(&src_opnd_0);

        /* 2.2 check if result is valid and generate exception if not */
        tr_gen_div_result_check(pir1, div_res, 16, 1);
        /* 2.3 store the results into destination */
        store_ir2_to_ir1(&div_mod, &dx_ir1_opnd, false);
        ra_free_temp(&div_mod);
        store_ir2_to_ir1(&div_res, &ax_ir1_opnd, false);
        break;
    case 32:
        load_ir1_gpr_to_ir2(&src_opnd_1, &eax_ir1_opnd, ZERO_EXTENSION);
        load_ir1_gpr_to_ir2(&src_opnd_2, &edx_ir1_opnd, UNKNOWN_EXTENSION);
        src_tmp = ra_alloc_itemp();
        div_mod = ra_alloc_itemp();

        /* 2.1 do div operation and get result */
        append_ir2_opnd2i(LISA_SLLI_D, &src_tmp, &src_opnd_2, 32);
        append_ir2_opnd3 (LISA_OR,     &src_tmp, &src_tmp,    &src_opnd_1);
        ra_free_temp(&src_opnd_1);
        ra_free_temp(&src_opnd_2);

        append_ir2_opnd3(LISA_DIV_D, &div_res, &src_tmp, &src_opnd_0);
        append_ir2_opnd3(LISA_MOD_D, &div_mod, &src_tmp, &src_opnd_0);
        ra_free_temp(&src_tmp);
        ra_free_temp(&src_opnd_0);

        /* 2.2 check if result is valid and generate exception if not */
        tr_gen_div_result_check(pir1, div_res, 32, 1);
        /* 2.3 store the results into destination */
        store_ir2_to_ir1(&div_mod, &edx_ir1_opnd, false);
        ra_free_temp(&div_mod);
        store_ir2_to_ir1(&div_res, &eax_ir1_opnd, false);
        break;
    default:
        lsassertm_illop(ir1_addr(pir1), 0,
                "64-bit translate_idiv is unimplemented.\n");
        break;
    }

    ra_free_temp(&div_res);
    return true;
}
