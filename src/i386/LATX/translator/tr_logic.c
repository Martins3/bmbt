#include "../include/common.h"
#include "../include/env.h"
#include "../include/reg_alloc.h"
#include "../include/flag_lbt.h"
#include "../x86tomips-options.h"

bool translate_shrd_imm(IR1_INST *pir1);
bool translate_shrd_cl(IR1_INST *pir1);
bool translate_shld_cl(IR1_INST *pir1);
bool translate_shld_imm(IR1_INST *pir1);

bool translate_xor(IR1_INST *pir1)
{
    lsassertm(0, "xor to be implemented in LoongArch.\n");
//    if (option_by_hand) return translate_xor_byhand(pir1);
//
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//    load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);
//
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//    append_ir2_opnd3(mips_xor, &dest_opnd, &src_opnd_0, &src_opnd_1);
//
//    generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
//
//    store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//
//    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_and(IR1_INST *pir1)
{
    lsassertm(0, "and to be implemented in LoongArch.\n");
//    if (option_by_hand) return translate_and_byhand(pir1);
//
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//    load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
//
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//    append_ir2_opnd3(mips_and, &dest_opnd, &src_opnd_0, &src_opnd_1);
//
//    generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
//
//    store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//
//    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_test(IR1_INST *pir1)
{
    lsassertm(0, "test to be implemented in LoongArch.\n");
//    if (option_by_hand) return translate_test_byhand(pir1);
//
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//    load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
//
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//    append_ir2_opnd3(mips_and, &dest_opnd, &src_opnd_0, &src_opnd_1);
//
//    generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
//
//    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_or(IR1_INST *pir1)
{
    lsassertm(0, "or to be implemented in LoongArch.\n");
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//    load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
//
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//    append_ir2_opnd3(mips_or, &dest_opnd, &src_opnd_0, &src_opnd_1);
//
//    generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, true);
//
//    store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//
//    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_not(IR1_INST *pir1)
{
    lsassertm(0, "not to be implemented in LoongArch.\n");
//    if (option_by_hand) return translate_not_byhand(pir1);
//
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//    append_ir2_opnd3(mips_nor, &dest_opnd, &zero_ir2_opnd, &src_opnd_0);
//
//    store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//
//    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_shl(IR1_INST *pir1)
{
    lsassertm(0, "shl to be implemented in LoongArch.\n");
//    if (option_by_hand) return translate_shl_byhand(pir1);
//
//    IR2_OPND src = ra_alloc_itemp();
//    IR2_OPND original_count = ra_alloc_itemp();
//    IR2_OPND dest = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//    load_ir1_to_ir2(&original_count, ir1_get_opnd(pir1, 1), ZERO_EXTENSION, false);
//
//    IR2_OPND count = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_andi, &count, &original_count, 31);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &count, &zero_ir2_opnd, &label_exit);
//
//    append_ir2_opnd3(mips_sllv, &dest, &src, &count);
//
//    generate_eflag_calculation(&dest, &src, &count, pir1, true);
//
//    store_ir2_to_ir1(&dest, ir1_get_opnd(pir1, 0), false);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&dest);
    return true;
}

bool translate_shr(IR1_INST *pir1)
{
    lsassertm(0, "shr to be implemented in LoongArch.\n");
//    if (option_by_hand) return translate_shr_byhand(pir1);
//
//    EXTENSION_MODE em = ZERO_EXTENSION;
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
//        em = SIGN_EXTENSION;
//    IR2_OPND src = ra_alloc_itemp();
//    IR2_OPND original_count = ra_alloc_itemp();
//    IR2_OPND dest = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src, ir1_get_opnd(pir1, 0), em, false);
//    load_ir1_to_ir2(&original_count, ir1_get_opnd(pir1, 1), ZERO_EXTENSION, false);
//
//    IR2_OPND count = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_andi, &count, &original_count, 31);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &count, &zero_ir2_opnd, &label_exit);
//
//    append_ir2_opnd3(mips_srlv, &dest, &src, &count);
//
//    generate_eflag_calculation(&dest, &src, &count, pir1, true);
//
//    store_ir2_to_ir1(&dest, ir1_get_opnd(pir1, 0), false);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&dest);
    return true;
}

bool translate_sal(IR1_INST *pir1)
{
    lsassertm(0, "sal to be implemented in LoongArch.\n");
//    if (option_by_hand) return translate_sal_byhand(pir1);
//
//    IR2_OPND src = ra_alloc_itemp();
//    IR2_OPND original_count = ra_alloc_itemp();
//    IR2_OPND dest = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//    load_ir1_to_ir2(&original_count, ir1_get_opnd(pir1, 1), ZERO_EXTENSION, false);
//
//    IR2_OPND count = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_andi, &count, &original_count, 31);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &count, &zero_ir2_opnd, &label_exit);
//
//    append_ir2_opnd3(mips_sllv, &dest, &src, &count);
//
//    generate_eflag_calculation(&dest, &src, &count, pir1, true);
//
//    store_ir2_to_ir1(&dest, ir1_get_opnd(pir1, 0), false);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&dest);
    return true;
}

bool translate_sar(IR1_INST *pir1)
{
    lsassertm(0, "sar to be implemented in LoongArch.\n");
//    if (option_by_hand) return translate_sar_byhand(pir1);
//
//    IR2_OPND src = ra_alloc_itemp();
//    IR2_OPND original_count = ra_alloc_itemp();
//    IR2_OPND dest = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//    load_ir1_to_ir2(&original_count, ir1_get_opnd(pir1, 1), ZERO_EXTENSION, false);
//
//    IR2_OPND count = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_andi, &count, &original_count, 31);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &count, &zero_ir2_opnd, &label_exit);
//
//    append_ir2_opnd3(mips_srav, &dest, &src, &count);
//
//    generate_eflag_calculation(&dest, &src, &count, pir1, true);
//
//    store_ir2_to_ir1(&dest, ir1_get_opnd(pir1, 0), false);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&dest);
    return true;
}

bool translate_rol(IR1_INST *pir1)
{
    lsassertm(0, "rol to be implemented in LoongArch.\n");
//    if (option_by_hand) return translate_rol_byhand(pir1);
//
//    IR2_OPND dest = ra_alloc_itemp();
//    IR2_OPND original_count = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&dest, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//    load_ir1_to_ir2(&original_count, ir1_get_opnd(pir1, 1), ZERO_EXTENSION, false);
//
//    IR2_OPND count = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_andi, &count, &original_count, 0x1f);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &count, &zero_ir2_opnd, &label_exit);
//
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8){
//            append_ir2_opnd2(mips_x86rotlvb, &original_count, &dest);
//        }
//        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16){
//            append_ir2_opnd2(mips_x86rotlvh, &original_count, &dest);
//        }
//        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32){
//            append_ir2_opnd2(mips_x86rotlvw, &original_count, &dest);
//        }
//    }
//
//
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
//        append_ir2_opnd2i(mips_andi, &count, &count, 7);
//    else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16)
//        append_ir2_opnd2i(mips_andi, &count, &count, 15);
//
//    IR2_OPND label_calc_eflags = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &count, &zero_ir2_opnd, &label_calc_eflags);
//    IR2_OPND low_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_dsllv, &low_dest, &dest, &count);
//    IR2_OPND tmp_count = ra_alloc_itemp();
//    append_ir2_opnd3(mips_subu, &tmp_count, &zero_ir2_opnd, &count);
//    append_ir2_opnd2i(mips_andi, &tmp_count, &tmp_count,
//                      ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
//    IR2_OPND high_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_dsrlv, &high_dest, &dest, &tmp_count);
//    ra_free_temp(&tmp_count);
//
//    append_ir2_opnd3(mips_or, &dest, &high_dest, &low_dest);
//
//    store_ir2_to_ir1(&dest, ir1_get_opnd(pir1, 0), false);
//
//    append_ir2_opnd1(mips_label, &label_calc_eflags);
//    generate_eflag_calculation(&dest, &dest, &count, pir1, true);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&high_dest);
//    ra_free_temp(&low_dest);
//    ra_free_temp(&count);

    return true;
}

bool translate_ror(IR1_INST *pir1)
{
    lsassertm(0, "ror to be implemented in LoongArch.\n");
//    if (option_by_hand) return translate_ror_byhand(pir1);
//
//    IR2_OPND dest = ra_alloc_itemp();
//    IR2_OPND original_count = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&dest, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//    load_ir1_to_ir2(&original_count, ir1_get_opnd(pir1, 1), ZERO_EXTENSION, false);
//
//    IR2_OPND count = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_andi, &count, &original_count, 0x1f);
//
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &count, &zero_ir2_opnd, &label_exit);
//
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8){
//            append_ir2_opnd2(mips_x86rotrvb, &original_count, &dest);
//        }
//        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16){
//            append_ir2_opnd2(mips_x86rotrvh, &original_count, &dest);
//        }
//        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32){
//            append_ir2_opnd2(mips_x86rotrvw, &original_count, &dest);
//        }
//    }
//
//
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
//        append_ir2_opnd2i(mips_andi, &count, &count, 7);
//    else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16)
//        append_ir2_opnd2i(mips_andi, &count, &count, 15);
//
//    IR2_OPND label_calc_eflags = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &count, &zero_ir2_opnd, &label_calc_eflags);
//
//    IR2_OPND high_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_dsrlv, &high_dest, &dest, &count);
//    IR2_OPND tmp_count = ra_alloc_itemp();
//    append_ir2_opnd3(mips_subu, &tmp_count, &zero_ir2_opnd, &count);
//    append_ir2_opnd2i(mips_andi, &tmp_count, &tmp_count,
//                      ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
//    IR2_OPND low_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_dsllv, &low_dest, &dest, &tmp_count);
//    ra_free_temp(&tmp_count);
//
//    append_ir2_opnd3(mips_or, &dest, &high_dest, &low_dest);
//
//    store_ir2_to_ir1(&dest, ir1_get_opnd(pir1, 0), false);
//
//    append_ir2_opnd1(mips_label, &label_calc_eflags);
//    generate_eflag_calculation(&dest, &dest, &count, pir1, true);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&high_dest);
//    ra_free_temp(&low_dest);
//    ra_free_temp(&count);
    return true;
}

bool translate_rcl(IR1_INST *pir1)
{
    lsassertm(0, "rcl to be implemented in LoongArch.\n");
//    IR2_OPND dest = ra_alloc_itemp();
//    IR2_OPND original_count = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&dest, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//    load_ir1_to_ir2(&original_count, ir1_get_opnd(pir1, 1), ZERO_EXTENSION, false);
//
//    IR2_OPND count = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_andi, &count, &original_count, 0x1f);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &count, &zero_ir2_opnd, &label_exit);
//
//    IR2_OPND tmp_imm = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_daddiu, &tmp_imm, &zero_ir2_opnd,
//                      ir1_opnd_size(ir1_get_opnd(pir1, 0)) + 1);
//    append_ir2_opnd2(mips_ddivu, &count, &tmp_imm);
//    append_ir2_opnd1(mips_mfhi, &count);
//    append_ir2_opnd3(mips_beq, &count, &zero_ir2_opnd, &label_exit);
//    ra_free_temp(&tmp_imm);
//
//    IR2_OPND cf = ra_alloc_itemp();
//
//    get_eflag_condition(&cf, pir1);
//
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
//        append_ir2_opnd2i(mips_dsll32, &cf, &cf, 0);
//    else
//        append_ir2_opnd2i(mips_dsll, &cf, &cf, ir1_opnd_size(ir1_get_opnd(pir1, 0)));
//    IR2_OPND tmp_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_or, &tmp_dest, &dest, &cf);
//
//    IR2_OPND high_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_dsllv, &high_dest, &tmp_dest, &count);
//
//    IR2_OPND tmp_count = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_daddiu, &tmp_count, &count,
//                      -1 - ir1_opnd_size(ir1_get_opnd(pir1, 0)));
//    append_ir2_opnd3(mips_dsubu, &tmp_count, &zero_ir2_opnd, &tmp_count);
//
//    IR2_OPND low_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_dsrlv, &low_dest, &tmp_dest, &tmp_count);
//    ra_free_temp(&tmp_count);
//    ra_free_temp(&tmp_dest);
//    ra_free_temp(&cf);
//
//    IR2_OPND final_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_or, &final_dest, &high_dest, &low_dest);
//
//    generate_eflag_calculation(&final_dest, &dest, &count, pir1, true);
//
//    store_ir2_to_ir1(&final_dest, ir1_get_opnd(pir1, 0), false);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&high_dest);
//    ra_free_temp(&low_dest);
//    ra_free_temp(&final_dest);
//    ra_free_temp(&count);
    return true;
}

bool translate_rcr(IR1_INST *pir1)
{
    lsassertm(0, "rcr to be implemented in LoongArch.\n");
//    IR2_OPND dest = ra_alloc_itemp();
//    IR2_OPND original_count = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&dest, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//    load_ir1_to_ir2(&original_count, ir1_get_opnd(pir1, 1), ZERO_EXTENSION, false);
//
//    IR2_OPND count = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_andi, &count, &original_count, 0x1f);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &count, &zero_ir2_opnd, &label_exit);
//
//    IR2_OPND tmp_imm = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_daddiu, &tmp_imm, &zero_ir2_opnd,
//                      ir1_opnd_size(ir1_get_opnd(pir1, 0)) + 1);
//    append_ir2_opnd2(mips_ddivu, &count, &tmp_imm);
//    append_ir2_opnd1(mips_mfhi, &count);
//    append_ir2_opnd3(mips_beq, &count, &zero_ir2_opnd, &label_exit);
//    ra_free_temp(&tmp_imm);
//
//    IR2_OPND cf = ra_alloc_itemp();
//
//    get_eflag_condition(&cf, pir1);
//
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
//        append_ir2_opnd2i(mips_dsll32, &cf, &cf, 0);
//    else
//        append_ir2_opnd2i(mips_dsll, &cf, &cf, ir1_opnd_size(ir1_get_opnd(pir1, 0)));
//    IR2_OPND tmp_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_or, &tmp_dest, &dest, &cf);
//
//    IR2_OPND low_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_dsrlv, &low_dest, &tmp_dest, &count);
//
//    IR2_OPND tmp_count = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_daddiu, &tmp_count, &count,
//                      -1 - ir1_opnd_size(ir1_get_opnd(pir1, 0)));
//    append_ir2_opnd3(mips_dsubu, &tmp_count, &zero_ir2_opnd, &tmp_count);
//
//    IR2_OPND high_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_dsllv, &high_dest, &tmp_dest, &tmp_count);
//    ra_free_temp(&tmp_count);
//    ra_free_temp(&tmp_dest);
//    ra_free_temp(&cf);
//
//    IR2_OPND final_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_or, &final_dest, &high_dest, &low_dest);
//
//    generate_eflag_calculation(&final_dest, &dest, &count, pir1, true);
//
//    store_ir2_to_ir1(&final_dest, ir1_get_opnd(pir1, 0), false);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&high_dest);
//    ra_free_temp(&low_dest);
//    ra_free_temp(&final_dest);
//    ra_free_temp(&count);
    return true;
}

bool translate_shrd_cl(IR1_INST *pir1)
{
    lsassertm(0, "shrd cl to be implemented in LoongArch.\n");
//    IR2_OPND count_opnd = ra_alloc_itemp();
//    load_ir1_to_ir2(&count_opnd, ir1_get_opnd(pir1, 2), ZERO_EXTENSION, false);
//
//    IR2_OPND count = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_andi, &count, &count_opnd, 31);
//    ra_free_temp(&count_opnd);
//
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &count, &zero_ir2_opnd, &label_exit);
//
//    IR2_OPND size = ra_alloc_itemp();
//    load_imm32_to_ir2(&size, ir1_opnd_size(ir1_get_opnd(pir1, 0)), SIGN_EXTENSION);
//    IR2_OPND left_count = ra_alloc_itemp();
//    append_ir2_opnd3(mips_subu, &left_count, &size, &count);
//    ra_free_temp(&size);
//
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//    load_ir1_to_ir2(&dest_opnd, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//    IR2_OPND low_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_dsrlv, &low_dest, &dest_opnd, &count);
//
//    IR2_OPND src_opnd = ra_alloc_itemp();
//    load_ir1_to_ir2(&src_opnd, ir1_get_opnd(pir1, 1), ZERO_EXTENSION, false);
//    IR2_OPND high_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_dsllv, &high_dest, &src_opnd, &left_count);
//    ra_free_temp(&left_count);
//    ra_free_temp(&src_opnd);
//
//    IR2_OPND final_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_or, &final_dest, &high_dest, &low_dest);
//    ra_free_temp(&low_dest);
//    ra_free_temp(&high_dest);
//
//    generate_eflag_calculation(&final_dest, &dest_opnd, &count, pir1, true);
//    ra_free_temp(&dest_opnd);
//
//    store_ir2_to_ir1(&final_dest, ir1_get_opnd(pir1, 0), false);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&final_dest);
//    ra_free_temp(&count);
    return true;
}

bool translate_shrd_imm(IR1_INST *pir1)
{
    lsassertm(0, "shrd imm to be implemented in LoongArch.\n");
//    lsassert(ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 2));
//    int count = ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 2) & 0x1f;
//    if (count == 0)
//        return true;
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//    load_ir1_to_ir2(&dest_opnd, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//    IR2_OPND low_dest = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_dsrl, &low_dest, &dest_opnd, count);
//
//    int left_count = ir1_opnd_size(ir1_get_opnd(pir1, 0)) - count;
//    lsassert(left_count >= 0 && left_count <= 31);
//    IR2_OPND src_opnd = ra_alloc_itemp();
//    load_ir1_to_ir2(&src_opnd, ir1_get_opnd(pir1, 1), ZERO_EXTENSION, false);
//    IR2_OPND high_dest = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_dsll, &high_dest, &src_opnd, left_count);
//    ra_free_temp(&src_opnd);
//
//    IR2_OPND final_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_or, &final_dest, &high_dest, &low_dest);
//    ra_free_temp(&low_dest);
//    ra_free_temp(&high_dest);
//
//    IR2_OPND count_opnd = ir2_opnd_new(IR2_OPND_IMM, (int16)count);
//    generate_eflag_calculation(&final_dest, &dest_opnd, &count_opnd, pir1, true);
//
//    store_ir2_to_ir1(&final_dest, ir1_get_opnd(pir1, 0), false);
//
//    ra_free_temp(&final_dest);
    return true;
}

bool translate_shrd(IR1_INST *pir1)
{
    if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 2))
        return translate_shrd_imm(pir1);
    else
        return translate_shrd_cl(pir1);

    return true;
}

bool translate_shld_cl(IR1_INST *pir1)
{
    lsassertm(0, "shld cl to be implemented in LoongArch.\n");
//    IR2_OPND count_opnd = ra_alloc_itemp();
//    load_ir1_to_ir2(&count_opnd, ir1_get_opnd(pir1, 2), ZERO_EXTENSION, false);
//
//    IR2_OPND count = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_andi, &count, &count_opnd, 31);
//    ra_free_temp(&count_opnd);
//
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &count, &zero_ir2_opnd, &label_exit);
//
//    IR2_OPND size = ra_alloc_itemp();
//    load_imm32_to_ir2(&size, ir1_opnd_size(ir1_get_opnd(pir1, 0)), SIGN_EXTENSION);
//    IR2_OPND left_count = ra_alloc_itemp();
//    append_ir2_opnd3(mips_subu, &left_count, &size, &count);
//    ra_free_temp(&size);
//
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//    load_ir1_to_ir2(&dest_opnd, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//    IR2_OPND high_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_dsllv, &high_dest, &dest_opnd, &count);
//
//    IR2_OPND src_opnd = ra_alloc_itemp();
//    load_ir1_to_ir2(&src_opnd, ir1_get_opnd(pir1, 1), ZERO_EXTENSION, false);
//    IR2_OPND low_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_dsrlv, &low_dest, &src_opnd, &left_count);
//    ra_free_temp(&left_count);
//    ra_free_temp(&src_opnd);
//
//    IR2_OPND final_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_or, &final_dest, &high_dest, &low_dest);
//    ra_free_temp(&low_dest);
//    ra_free_temp(&high_dest);
//
//    generate_eflag_calculation(&final_dest, &dest_opnd, &count, pir1, true);
//
//    store_ir2_to_ir1(&final_dest, ir1_get_opnd(pir1, 0), false);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&final_dest);
//    ra_free_temp(&count);
    return true;
}

bool translate_shld_imm(IR1_INST *pir1)
{
    lsassertm(0, "shld imm to be implemented in LoongArch.\n");
//    lsassert(ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 2));
//    int count = ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 2) & 0x1f;
//    if (count == 0)
//        return true;
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//    load_ir1_to_ir2(&dest_opnd, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//    IR2_OPND high_dest = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_dsll, &high_dest, &dest_opnd, count);
//
//    int left_count = ir1_opnd_size(ir1_get_opnd(pir1, 0)) - count;
//    lsassert(left_count >= 0 && left_count <= 31);
//    IR2_OPND src_opnd = ra_alloc_itemp();
//    load_ir1_to_ir2(&src_opnd, ir1_get_opnd(pir1, 1), ZERO_EXTENSION, false);
//    IR2_OPND low_dest = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_dsrl, &low_dest, &src_opnd, left_count);
//    ra_free_temp(&src_opnd);
//
//    IR2_OPND final_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_or, &final_dest, &high_dest, &low_dest);
//    ra_free_temp(&low_dest);
//    ra_free_temp(&high_dest);
//
//    IR2_OPND count_opnd = ir2_opnd_new(IR2_OPND_IMM, (int16)count);
//    generate_eflag_calculation(&final_dest, &dest_opnd, &count_opnd, pir1, true);
//
//    store_ir2_to_ir1(&final_dest, ir1_get_opnd(pir1, 0), false);
//
//    ra_free_temp(&final_dest);
    return true;
}

bool translate_shld(IR1_INST *pir1)
{
    if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 2))
        return translate_shld_imm(pir1);
    else
        return translate_shld_cl(pir1);

    return true;
}

bool translate_bswap(IR1_INST *pir1)
{
    lsassertm(0, "bswap to be implemented in LoongArch.\n");
//    lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32);
//
//    IR2_OPND old_value = ra_alloc_itemp();
//    load_ir1_to_ir2(&old_value, ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
//
//    /* prepare the 4 bytes */
//    IR2_OPND new_low_2 = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_srl, &new_low_2, &old_value, 8);
//    append_ir2_opnd2i(mips_andi, &new_low_2, &new_low_2, 0xff00);
//
//    IR2_OPND new_low_3 = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_andi, &new_low_3, &old_value, 0xff00);
//    append_ir2_opnd2i(mips_sll, &new_low_3, &new_low_3, 8);
//
//    IR2_OPND new_low_1 = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_srl, &new_low_1, &old_value, 24);
//
//    IR2_OPND new_low_4 = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_sll, &new_low_4, &old_value, 24);
//
//    /* OR them */
//    append_ir2_opnd3(mips_or, &new_low_1, &new_low_1, &new_low_2);
//    append_ir2_opnd3(mips_or, &new_low_4, &new_low_4, &new_low_3);
//    append_ir2_opnd3(mips_or, &old_value, &new_low_1, &new_low_4);
//    lsassert(ir2_opnd_is_sx(&old_value, 32));
//
//    store_ir2_to_ir1(&old_value, ir1_get_opnd(pir1, 0), false);

    return true;
}
