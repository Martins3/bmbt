#include "common.h"
#include "env.h"
#include "reg_alloc.h"
#include "x86tomips-options.h"

bool translate_add(IR1_INST *pir1)
{
    return false;
//    if (option_by_hand) return translate_add_byhand(pir1);
//
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//    load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);
//
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//    append_ir2_opnd3(mips_addu, &dest_opnd, &src_opnd_0, &src_opnd_1);
//    fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);
//
//    generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
//
//    store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//
//    ra_free_temp(&dest_opnd);
//    return true;
}

bool translate_adc(IR1_INST *pir1)
{
    return false;
//    if (option_by_hand) return translate_adc_byhand(pir1);
//
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//    load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);
//
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//    load_eflags_cf_to_ir2(&dest_opnd);
//    append_ir2_opnd3(mips_addu, &dest_opnd, &dest_opnd, &src_opnd_0);
//    append_ir2_opnd3(mips_addu, &dest_opnd, &dest_opnd, &src_opnd_1);
//
//    generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
//
//    store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//
//    ra_free_temp(&dest_opnd);
//    return true;
}

bool translate_inc(IR1_INST *pir1)
{
    return false;
    /*if (option_by_hand) return translate_inc_byhand(pir1);*/

    /*IR2_OPND src_opnd_0 = ra_alloc_itemp();*/
    /*load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);*/

    /*IR2_OPND dest_opnd = ra_alloc_itemp();*/
    /*append_ir2_opnd2i(mips_addiu, &dest_opnd, &src_opnd_0, 1);*/

    /*IR2_OPND t_imm1 = ir2_opnd_new(IR2_OPND_IMM, 1);*/

    /*generate_eflag_calculation(&dest_opnd, &src_opnd_0, &t_imm1, pir1, true);*/

    /*store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);*/

    /*ra_free_temp(&dest_opnd);*/
    /*return true;*/
}

bool translate_dec(IR1_INST *pir1)
{
    return false;
    /*if (option_by_hand) return translate_dec_byhand(pir1);*/

    /*IR2_OPND src_opnd_0 = ra_alloc_itemp();*/
    /*load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);*/

    /*IR2_OPND dest_opnd = ra_alloc_itemp();*/
    /*append_ir2_opnd2i(mips_addiu, &dest_opnd, &src_opnd_0, -1);*/

    /*IR2_OPND t_imm1 = ir2_opnd_new(IR2_OPND_IMM, 1);*/

    /*generate_eflag_calculation(&dest_opnd, &src_opnd_0, &t_imm1, pir1, true);*/

    /*store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);*/

    /*ra_free_temp(&dest_opnd);*/
    /*return true;*/
}

bool translate_sub(IR1_INST *pir1)
{
    return false;
    /*if (option_by_hand) return translate_sub_byhand(pir1);*/

    /*IR2_OPND src_opnd_0 = ra_alloc_itemp();*/
    /*IR2_OPND src_opnd_1 = ra_alloc_itemp();*/
    /*load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);*/
    /*load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);*/

    /*IR2_OPND dest_opnd = ra_alloc_itemp();*/
    /*fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);*/
    /*append_ir2_opnd3(mips_subu, &dest_opnd, &src_opnd_0, &src_opnd_1);*/

    /*generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);*/

    /*store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);*/

    /*ra_free_temp(&dest_opnd);*/
    /*return true;*/
}

bool translate_sbb(IR1_INST *pir1)
{
    return false;
    /*IR2_OPND src_opnd_0 = ra_alloc_itemp();*/
    /*IR2_OPND src_opnd_1 = ra_alloc_itemp();*/
    /*load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);*/
    /*load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);*/

    /*IR2_OPND dest_opnd = ra_alloc_itemp();*/
    /*IR2_OPND tmp_opnd = dest_opnd;*/
    /*load_eflags_cf_to_ir2(&dest_opnd);*/
    /*if (ir2_opnd_cmp(&src_opnd_0, &src_opnd_1)) {*/
        /*append_ir2_opnd3(mips_subu, &src_opnd_0, &zero_ir2_opnd, &dest_opnd);*/
        /*dest_opnd = src_opnd_0;*/
    /*} else {*/
        /*append_ir2_opnd3(mips_subu, &dest_opnd, &src_opnd_0, &dest_opnd);*/
        /*bool dest_opnd_is_temp = ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) ||*/
                                 /*ir1_need_calculate_of(pir1) ||*/
                                 /*ir1_need_calculate_cf(pir1);*/
        /*if (!dest_opnd_is_temp)*/
            /*dest_opnd = src_opnd_0;*/
        /*append_ir2_opnd3(mips_subu, &dest_opnd, &tmp_opnd, &src_opnd_1);*/
    /*}*/

    /*generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);*/

    /*store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);*/

    /*ra_free_temp(&dest_opnd);*/
    /*return true;*/
}

bool translate_neg(IR1_INST *pir1)
{
    return false;
    /*if (option_by_hand) return translate_neg_byhand(pir1);*/

    /*IR2_OPND src_opnd_0 = ra_alloc_itemp();*/
    /*load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);*/

    /*IR2_OPND dest_opnd = ra_alloc_itemp();*/
    /*append_ir2_opnd3(mips_subu, &dest_opnd, &zero_ir2_opnd, &src_opnd_0);*/

    /*generate_eflag_calculation(&dest_opnd, &zero_ir2_opnd, &src_opnd_0, pir1,*/
                               /*true);*/

    /*store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);*/

    /*ra_free_temp(&dest_opnd);*/
    /*return true;*/
}

bool translate_cmp(IR1_INST *pir1)
{
    return false;
    /*if (option_by_hand) return translate_cmp_byhand(pir1);*/

    /*IR2_OPND src_opnd_0 = ra_alloc_itemp();*/
    /*IR2_OPND src_opnd_1 = ra_alloc_itemp();*/
    /*load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);*/
    /*load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);*/

    /*IR2_OPND dest_opnd = ra_alloc_itemp();*/
    /*fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);*/
    /*append_ir2_opnd3(mips_subu, &dest_opnd, &src_opnd_0, &src_opnd_1);*/

    /*generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);*/

    /*ra_free_temp(&dest_opnd);*/

    /*return true;*/
}

bool translate_mul(IR1_INST *pir1)
{
    return false;
    /*if (option_by_hand) return translate_mul_byhand(pir1);*/

    /*IR2_OPND src_opnd_0 = ra_alloc_itemp();*/
    /*load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);*/
    /*IR2_OPND dest_opnd = ra_alloc_itemp();*/

    /*if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8) {*/
        /*IR2_OPND src_opnd_1 = ra_alloc_itemp();*/
        /*load_ir1_gpr_to_ir2(&src_opnd_1, &al_ir1_opnd, ZERO_EXTENSION);*/

        /*append_ir2_opnd3(mips_multu_g, &dest_opnd, &src_opnd_1, &src_opnd_0);*/

        /*store_ir2_to_ir1(&dest_opnd, &ax_ir1_opnd, false);*/
    /*} else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {*/
        /*IR2_OPND src_opnd_1 = ra_alloc_itemp();*/
        /*load_ir1_gpr_to_ir2(&src_opnd_1, &ax_ir1_opnd, ZERO_EXTENSION);*/

        /*append_ir2_opnd3(mips_multu_g, &dest_opnd, &src_opnd_1, &src_opnd_0);*/

        /*store_ir2_to_ir1(&dest_opnd, &ax_ir1_opnd, false);*/
        /*append_ir2_opnd2i(mips_srl, &dest_opnd, &dest_opnd, 16);*/
        /*store_ir2_to_ir1(&dest_opnd, &dx_ir1_opnd, false);*/
    /*} else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {*/
        /*IR2_OPND src_opnd_1 = ra_alloc_itemp();*/
        /*load_ir1_gpr_to_ir2(&src_opnd_1, &eax_ir1_opnd, ZERO_EXTENSION);*/

        /*append_ir2_opnd3(mips_dmultu_g, &dest_opnd, &src_opnd_1, &src_opnd_0);*/

        /*store_ir2_to_ir1(&dest_opnd, &eax_ir1_opnd, false);*/
        /*append_ir2_opnd2i(mips_dsrl32, &dest_opnd, &dest_opnd, 0);*/
        /*store_ir2_to_ir1(&dest_opnd, &edx_ir1_opnd, false);*/
    /*} else {*/
        /*[> NOT_IMPLEMENTED; <]*/
    /*}*/

    /*generate_eflag_calculation(&dest_opnd, NULL, NULL, pir1, true);*/

    /*ra_free_temp(&dest_opnd);*/
    /*return true;*/
}

//static bool translate_imul_1_opnd(IR1_INST *pir1)
//{
//    return false;
    /*IR2_OPND src_opnd_0 = ra_alloc_itemp();*/
    /*load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);*/
    /*IR2_OPND dest = ra_alloc_itemp();*/
    /*if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8) {*/
        /*IR2_OPND src_opnd_1 = ra_alloc_itemp();*/
        /*load_ir1_gpr_to_ir2(&src_opnd_1, &al_ir1_opnd, SIGN_EXTENSION);*/
        /*append_ir2_opnd3(mips_mult_g, &dest, &src_opnd_1, &src_opnd_0);*/
        /*[> calculate eflag <]*/
        /*generate_eflag_calculation(&dest, &src_opnd_0, &src_opnd_1, pir1, true);*/

        /*store_ir2_to_ir1(&dest, &ax_ir1_opnd, false);*/
    /*} else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {*/
        /*IR2_OPND src_opnd_1 = ra_alloc_itemp();*/
        /*load_ir1_gpr_to_ir2(&src_opnd_1, &ax_ir1_opnd, SIGN_EXTENSION);*/
        /*append_ir2_opnd3(mips_mult_g, &dest, &src_opnd_1, &src_opnd_0);*/
        /*generate_eflag_calculation(&dest, &src_opnd_0, &src_opnd_1, pir1, true);*/
        /*store_ir2_to_ir1(&dest, &ax_ir1_opnd, false);*/
        /*append_ir2_opnd2i(mips_dsrl, &dest, &dest, 16);*/
        /*store_ir2_to_ir1(&dest, &dx_ir1_opnd, false);*/
    /*} else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {*/
        /*IR2_OPND src_opnd_1 = ra_alloc_itemp();*/
        /*load_ir1_gpr_to_ir2(&src_opnd_1, &eax_ir1_opnd, SIGN_EXTENSION);*/
        /*append_ir2_opnd3(mips_dmult_g, &dest, &src_opnd_1, &src_opnd_0);*/
        /*generate_eflag_calculation(&dest, NULL, NULL, pir1, true);*/

        /*store_ir2_to_ir1(&dest, &eax_ir1_opnd, false);*/
        /*append_ir2_opnd2i(mips_dsrl32, &dest, &dest, 0);*/
        /*store_ir2_to_ir1(&dest, &edx_ir1_opnd, false);*/
    /*} else {*/
        /*[> IR2_OPND dest_opnd_0 = <]*/
        /* load_ireg_from_ir1(pir1->get_dest_opnd_implicit(0), SIGN_EXTENSION,
         */
        /*[> false); IR2_OPND dest_opnd_1 = <]*/
        /* load_ireg_from_ir1(pir1->get_dest_opnd_implicit(1), SIGN_EXTENSION,
         */
        /*[> false); append_ir2_opnd2(mips_dmult, src_opnd_1, src_opnd_0); <]*/
        /*[> append_ir2_opnd1(mips_mflo, dest_opnd_0); <]*/
        /*[> append_ir2_opnd1(mips_mfhi, dest_opnd_1); <]*/
        /*lsassertm(0, "64-bit translate_imul_1_opnd is unimplemented.\n");*/
    /*}*/
    /*ra_free_temp(&dest);*/
    /*return true;*/
//}

bool translate_imul(IR1_INST *pir1)
{
    return false;
    /*if (option_by_hand) return translate_imul_byhand(pir1);*/

    /*if (ir1_opnd_num(pir1) == 1)*/
        /*return translate_imul_1_opnd(pir1);*/

    /*IR2_OPND src_opnd_0 = ra_alloc_itemp();*/
    /*IR2_OPND src_opnd_1 = ra_alloc_itemp();*/
    /*load_ir1_to_ir2(&src_opnd_0, ir1_get_src_opnd(pir1, 0), SIGN_EXTENSION, false);*/
    /*load_ir1_to_ir2(&src_opnd_1, ir1_get_src_opnd(pir1, 1), SIGN_EXTENSION, false);*/

    /*IR2_OPND dest_opnd = ra_alloc_itemp();*/
    /*lsassertm(ir1_opnd_size(ir1_get_opnd(pir1, 0)) != 64,*/
              /*"64-bit translate_imul is unimplemented.\n");*/
    /*IR2_OPCODE mips_opcode;*/
    /*if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)*/
        /*mips_opcode = mips_dmult_g;*/
    /*else*/
        /*mips_opcode = mips_mult_g;*/
    /*append_ir2_opnd3(mips_opcode, &dest_opnd, &src_opnd_1, &src_opnd_0);*/
    /*generate_eflag_calculation(&dest_opnd, NULL, NULL, pir1, true);*/

    /*if (ir1_opnd_num(pir1) == 3)*/
        /*store_ir2_to_ir1(&dest_opnd, ir1_get_dest_opnd(pir1, 0), false);*/
    /*else*/
        /*store_ir2_to_ir1(&dest_opnd, ir1_get_src_opnd(pir1, 0), false);*/
    /*ra_free_temp(&dest_opnd);*/
    /*return true;*/
}

bool translate_div(IR1_INST *pir1)
{
    return false;
    /*if (option_by_hand) return translate_div_byhand(pir1);*/

    /*IR2_OPND small_opnd = ra_alloc_itemp();*/
    /*load_ir1_to_ir2(&small_opnd, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);*/
    /*IR2_OPND result = ra_alloc_itemp();*/
    /*IR2_OPND result_remainder = ra_alloc_itemp();*/

    /*if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8) {*/
        /*IR2_OPND large_opnd = ra_alloc_itemp();*/
        /*load_ir1_to_ir2(&large_opnd, &ax_ir1_opnd, ZERO_EXTENSION, false);*/

        /*append_ir2_opnd2(mips_ddivu, &large_opnd, &small_opnd);*/
        /*append_ir2_opnd1(mips_mflo, &result);*/
        /*append_ir2_opnd1(mips_mfhi, &result_remainder);*/
        /*ir2_opnd_set_em(*/
            /*&result, ZERO_EXTENSION,*/
            /*8); [>  result larger than uint8 would raise an exception <]*/
        /*ir2_opnd_set_em(&result_remainder, ZERO_EXTENSION,*/
                        /*8); [> remainder does not exceed small_opnd <]*/

        /*[> set AL and AH at the same time <]*/
        /*append_ir2_opnd2i(mips_dsll, &result_remainder, &result_remainder, 8);*/
        /*append_ir2_opnd3(mips_or, &result, &result, &result_remainder);*/
        /*store_ir2_to_ir1(&result, &ax_ir1_opnd, false);*/
    /*} else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {*/
        /*IR2_OPND large_opnd = ra_alloc_itemp();*/
        /*load_ir1_gpr_to_ir2(&large_opnd, &ax_ir1_opnd, ZERO_EXTENSION);*/
        /*IR2_OPND large_opnd_high_bits = ra_alloc_itemp();*/
        /*load_ir1_gpr_to_ir2(&large_opnd_high_bits, &dx_ir1_opnd, ZERO_EXTENSION);*/
        /*append_ir2_opnd2i(mips_dsll, &large_opnd_high_bits, &large_opnd_high_bits,*/
                          /*16);*/
        /*append_ir2_opnd3(mips_or, &large_opnd, &large_opnd_high_bits, &large_opnd);*/

        /*append_ir2_opnd2(mips_ddivu, &large_opnd, &small_opnd);*/
        /*append_ir2_opnd1(mips_mflo, &result);*/
        /*append_ir2_opnd1(mips_mfhi, &result_remainder);*/
        /*ir2_opnd_set_em(&result, ZERO_EXTENSION, 16);*/
        /*ir2_opnd_set_em(&result_remainder, ZERO_EXTENSION, 16);*/

        /*store_ir2_to_ir1(&result, &ax_ir1_opnd, false);*/
        /*store_ir2_to_ir1(&result_remainder, &dx_ir1_opnd, false);*/
    /*} else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {*/
        /*IR2_OPND large_opnd = ra_alloc_itemp();*/
        /*load_ir1_gpr_to_ir2(&large_opnd, &eax_ir1_opnd, ZERO_EXTENSION);*/
        /*IR2_OPND large_opnd_high_bits = ra_alloc_itemp();*/
        /*load_ir1_gpr_to_ir2(&large_opnd_high_bits, &edx_ir1_opnd, UNKNOWN_EXTENSION);*/
        /*append_ir2_opnd2i(mips_dsll32, &large_opnd_high_bits,*/
                          /*&large_opnd_high_bits, 0);*/
        /*append_ir2_opnd3(mips_or, &large_opnd, &large_opnd_high_bits, &large_opnd);*/

        /*append_ir2_opnd2(mips_ddivu, &large_opnd, &small_opnd);*/
        /*IR2_OPND ir2_eax = ra_alloc_gpr(ir1_opnd_base_reg_num(&eax_ir1_opnd));*/
        /*IR2_OPND ir2_edx = ra_alloc_gpr(ir1_opnd_base_reg_num(&edx_ir1_opnd));*/
        /*append_ir2_opnd1(mips_mflo, &ir2_eax);*/
        /*append_ir2_opnd1(mips_mfhi, &ir2_edx);*/
        /*ir2_opnd_set_em(&ir2_eax, ZERO_EXTENSION, 32);*/
        /*ir2_opnd_set_em(&ir2_edx, ZERO_EXTENSION, 32);*/
/*#ifdef N64*/
/*#else*/
        /*[> convert to sign-extended mode <]*/
        /*append_ir2_opnd2i(mips_sll, &ir2_eax, &ir2_eax, 0);*/
        /*append_ir2_opnd2i(mips_sll, &ir2_edx, &ir2_edx, 0);*/
/*#endif*/
    /*} else*/
        /*lsassertm(0, "64-bit translate_div is unimplemented.\n");*/

    /*ra_free_temp(&result);*/
    /*ra_free_temp(&result_remainder);*/
    /*return true;*/
}

bool translate_idiv(IR1_INST *pir1)
{
    return false;
    /*IR2_OPND dest_opnd = ra_alloc_itemp();*/
    /*IR2_OPND src_opnd_0 = ra_alloc_itemp();*/
    /*load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);*/

    /*if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8) {*/
        /*IR2_OPND src_opnd_1 = ra_alloc_itemp();*/
        /*load_ir1_gpr_to_ir2(&src_opnd_1, &ax_ir1_opnd, SIGN_EXTENSION);*/
        /*IR2_OPND low_8_bit = ra_alloc_itemp();*/
        /*append_ir2_opnd2(mips_ddiv, &src_opnd_1, &src_opnd_0);*/
        /*append_ir2_opnd1(mips_mflo, &low_8_bit);*/
        /*[> TODO <]*/
        /*append_ir2_opnd1(mips_mfhi, &dest_opnd);*/
        /*store_ir2_to_ir1(&dest_opnd, &ah_ir1_opnd, false);*/
    /*} else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {*/
        /*IR2_OPND src_opnd_1 = ra_alloc_itemp();*/
        /*load_ir1_gpr_to_ir2(&src_opnd_1, &ax_ir1_opnd, ZERO_EXTENSION);*/
        /*IR2_OPND src_opnd_2 = ra_alloc_itemp();*/
        /*load_ir1_gpr_to_ir2(&src_opnd_2, &dx_ir1_opnd, UNKNOWN_EXTENSION);*/
        /*IR2_OPND temp_src = ra_alloc_itemp();*/
        /*append_ir2_opnd2i(mips_sll, &temp_src, &src_opnd_2, 16);*/
        /*append_ir2_opnd3(mips_or, &temp_src, &temp_src, &src_opnd_1);*/
        /*append_ir2_opnd2(mips_ddiv, &temp_src, &src_opnd_0);*/
        /*append_ir2_opnd1(mips_mflo, &dest_opnd);*/
        /*store_ir2_to_ir1(&dest_opnd, &ax_ir1_opnd, false);*/
        /*append_ir2_opnd1(mips_mfhi, &dest_opnd);*/
        /*store_ir2_to_ir1(&dest_opnd, &dx_ir1_opnd, false);*/
        /*ra_free_temp(&temp_src);*/
    /*} else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {*/
        /*IR2_OPND src_opnd_1 = ra_alloc_itemp();*/
        /*load_ir1_gpr_to_ir2(&src_opnd_1, &eax_ir1_opnd, ZERO_EXTENSION);*/
        /*IR2_OPND src_opnd_2 = ra_alloc_itemp();*/
        /*load_ir1_gpr_to_ir2(&src_opnd_2, &edx_ir1_opnd, UNKNOWN_EXTENSION);*/
        /*IR2_OPND temp_src = ra_alloc_itemp();*/
        /*append_ir2_opnd2i(mips_dsll32, &temp_src, &src_opnd_2, 0);*/
        /*append_ir2_opnd3(mips_or, &temp_src, &temp_src, &src_opnd_1);*/
        /*append_ir2_opnd2(mips_ddiv, &temp_src, &src_opnd_0);*/
        /*append_ir2_opnd1(mips_mflo, &dest_opnd);*/
        /*store_ir2_to_ir1(&dest_opnd, &eax_ir1_opnd, false);*/
        /*append_ir2_opnd1(mips_mfhi, &dest_opnd);*/
        /*store_ir2_to_ir1(&dest_opnd, &edx_ir1_opnd, false);*/
        /*ra_free_temp(&temp_src);*/
    /*} else*/
        /*lsassertm(0, "64-bit translate_idiv is unimplemented.\n");*/

    /*ra_free_temp(&dest_opnd);*/
    /*return true;*/
}

#ifndef CONFIG_SOFTMMU
bool translate_xadd(IR1_INST *pir1)
{
    return false;
    /*IR2_OPND src_opnd_0 = ra_alloc_itemp();*/
    /*load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);*/
    /*IR2_OPND src_opnd_1 = ra_alloc_itemp();*/
    /*load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);*/

    /*IR2_OPND dest_opnd = ra_alloc_itemp();*/
    /*append_ir2_opnd3(mips_addu, &dest_opnd, &src_opnd_0, &src_opnd_1);*/

    /*generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);*/

    /*store_ir2_to_ir1(&src_opnd_0, ir1_get_opnd(pir1, 0) + 1, false);*/
    /*store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);*/

    /*ra_free_temp(&dest_opnd);*/

    /*return true;*/
}
#endif
