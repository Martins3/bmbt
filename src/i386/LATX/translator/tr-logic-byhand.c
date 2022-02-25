#include "../include/common.h"
#include "../translator/translate.h"
#include "../include/reg-alloc.h"
#include "../x86tomips-options.h"
#include "../include/flag-lbt.h"
#include "../include/flag-pattern.h"

static bool translate_and_byhand_32(IR1_INST *pir1, bool is_and);
bool translate_rol_byhand_imm(IR1_INST *pir1);
bool translate_rol_byhand_cl(IR1_INST *pir1);
bool translate_ror_byhand_imm(IR1_INST *pir1);
bool translate_ror_byhand_cl(IR1_INST *pir1);
bool translate_shl_byhand_imm(IR1_INST *pir1);
bool translate_shl_byhand_cl(IR1_INST *pir1);
bool translate_sar_byhand_imm(IR1_INST *pir1);
bool translate_sar_byhand_cl(IR1_INST *pir1);
bool translate_shr_byhand_imm(IR1_INST *pir1);
bool translate_shr_byhand_cl(IR1_INST *pir1);

static bool translate_and_byhand_32(IR1_INST *pir1, bool is_and)
{
    lsassertm(0, "and byhand to be implemented in LoongArch.\n");
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//
//    IR2_OPCODE mips_opcode = mips_invalid;
//    bool is_opnd_sx = false;
//
//    /* 1. prepare source operands and opcode */
//    if (ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0))) { /* dest is x86 address */
//        load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//        lsassert(ir2_opnd_is_x86_address(&src_opnd_0));
//        if (ir1_opnd_is_uimm_within_16bit(ir1_get_opnd(pir1, 1))) {
//            ir2_opnd_build(&src_opnd_1, IR2_OPND_IMM,
//                           ir1_opnd_uimm(ir1_get_opnd(pir1, 1)));
//        } else {
//            load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, false);
//        }
//        mips_opcode = mips_and;
//    }
//
//    else if (ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 1)) &&
//             is_and) { /* src is x86 address and the result will be write back
//                        */
//        load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
//        load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 1), ZERO_EXTENSION, false);
//        lsassert(ir2_opnd_is_x86_address(&src_opnd_1));
//        mips_opcode = mips_and;
//    }
//
//    else { /* none is x86 address or the result doesn't need to write back */
//        load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//        if (ir1_opnd_is_uimm_within_16bit(ir1_get_opnd(pir1, 1))) {
//            ir2_opnd_build(&src_opnd_1, IR2_OPND_IMM,
//                           ir1_opnd_uimm(ir1_get_opnd(pir1, 1)));
//        } else if (ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0)) &&
//                   ir1_opnd_is_gpr(ir1_get_opnd(pir1, 1)) &&
//                   ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)) ==
//                       ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 1))) {
//            src_opnd_1 = src_opnd_0;
//        } else {
//            load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);
//        }
//        mips_opcode = mips_and;
//        is_opnd_sx = true;
//    }
//
//    /* 2. prepare dest operand */
//    IR2_OPND dest_opnd = src_opnd_0;
//    bool dest_opnd_is_temp = !is_and;
//    if (dest_opnd_is_temp) {
//        dest_opnd = ra_alloc_itemp();
//    }
//
//    if (!is_and && (32 - ir1_opnd_size(ir1_get_opnd(pir1, 0))) == 0 &&
//        !ir1_need_calculate_any_flag(pir1)) {
//        dest_opnd = ra_alloc_flag_pattern_saved_opnd0();
//    }
//
//    /* 3. calculate result */
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, is_opnd_sx);
//    }
//
//    append_ir2_opnd3(mips_opcode, &dest_opnd, &src_opnd_0, &src_opnd_1);
//
//    /* 4. calculate eflags */
//    if (option_lbt) {
//        fp_save_dest_opnd(pir1, dest_opnd);
//    } else {
//        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    /* 5. write the result back */
//    if (is_and) {
//        store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//    }
//
//    /* 6. release temp */
//    if (dest_opnd_is_temp) {
//        ra_free_temp(&dest_opnd);
//    }
    return true;
}

static bool translate_and_byhand_8_16(IR1_INST *pir1, bool is_and)
{
    lsassertm(0, "and byhand to be implemented in LoongArch.\n");
//    bool is_opnd_sx = false;
//
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
//
//    if (ir1_opnd_is_simm_within_16bit(ir1_get_opnd(pir1, 1))) {
//        ir2_opnd_build(&src_opnd_1, IR2_OPND_IMM,
//                       ir1_opnd_simm(ir1_get_opnd(pir1, 1)));
//    }
//    else if (ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0)) &&
//               ir1_opnd_is_gpr(ir1_get_opnd(pir1, 1)) &&
//               ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)) ==
//                   ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 1))) {
//        src_opnd_1 = src_opnd_0;
//    }
//    else {
//        load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, false);
//    }
//
//    /* 2. calculate result */
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1, is_opnd_sx);
//    }
//
//    append_ir2_opnd3(mips_and, &dest_opnd, &src_opnd_0, &src_opnd_1);
//
//    /* 3. calculate eflags */
//    if (option_lbt) {
//        fp_save_dest_opnd(pir1, dest_opnd);
//    } else {
//        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    /* 4. write the result back */
//    if (is_and) {
//        store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//    }
//
//    /* 5. release temp */
//    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_and_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
            return translate_and_byhand_32(pir1, true);
        } else {
            return translate_and_byhand_8_16(pir1, true);
        }
    } else {
        return translate_and(pir1);
    }
}

bool translate_test_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
            return translate_and_byhand_32(pir1, false);
        } else {
            return translate_and_byhand_8_16(pir1, false);
        }
    } else {
        return translate_test(pir1);
    }
}

bool translate_rol_byhand_imm(IR1_INST *pir1)
{
    lsassertm(0, "rol byhand to be implemented in LoongArch.\n");
//    IR2_OPND dest = ra_alloc_itemp();
//
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
//        load_ir1_to_ir2(&dest, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//    }
//    else {
//        dest = ra_alloc_itemp();
//        load_ir1_to_ir2(&dest, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//    }
//
//    int count = ir1_opnd_simm(ir1_get_opnd(pir1, 1));
//    count = count & 0x1f;
//
//    if (count == 0) {
//        return true;
//    }
//
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8){
//            append_ir2_opnd1i(mips_x86rotlb, &dest, count);
//        }
//        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16){
//            append_ir2_opnd1i(mips_x86rotlh, &dest, count);
//        }
//        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32){
//            append_ir2_opnd1i(mips_x86rotlw, &dest, count);
//        }
//    }
//
//    count = count & (ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
//    IR2_OPND count_opnd = ir2_opnd_new(IR2_OPND_IMM, count);
//    if (count == 0) {
//        generate_eflag_calculation(&dest, &dest, &count_opnd, pir1, true);
//        return true;
//    }
//
//    IR2_OPND low_dest = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_sll, &low_dest, &dest, count);
//    int tmp_count = (0 - count) & (ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
//    IR2_OPND high_dest = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_srl, &high_dest, &dest, tmp_count);
//
//    append_ir2_opnd3(mips_or, &dest, &high_dest, &low_dest);
//
//    store_ir2_to_ir1(&dest, ir1_get_opnd(pir1, 0), false);
//
//    count_opnd = ir2_opnd_new(IR2_OPND_IMM, count);
//    generate_eflag_calculation(&dest, &dest, &count_opnd, pir1, true);
//
//    ra_free_temp(&high_dest);
//    ra_free_temp(&low_dest);
    return true;
}

bool translate_rol_byhand_cl(IR1_INST *pir1)
{
    lsassertm(0, "rol byhand to be implemented in LoongArch.\n");
//    IR2_OPND dest = ra_alloc_itemp();
//
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
//        load_ir1_to_ir2(&dest, ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
//    } else {
//        dest = ra_alloc_itemp();
//        load_ir1_to_ir2(&dest, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//    }
//
//    IR2_OPND original_count = ra_alloc_itemp();
//    load_ir1_to_ir2(&original_count, ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, false);
//
//    IR2_OPND count = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_andi, &count, &original_count, 0x1f);
//
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
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8) {
//        append_ir2_opnd2i(mips_andi, &count, &count, 7);
//    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {
//        append_ir2_opnd2i(mips_andi, &count, &count, 15);
//    }
//
//    IR2_OPND label_calc_eflags = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &count, &zero_ir2_opnd, &label_calc_eflags);
//    IR2_OPND low_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_sllv, &low_dest, &dest, &count);
//    IR2_OPND tmp_count = ra_alloc_itemp();
//    append_ir2_opnd3(mips_subu, &tmp_count, &zero_ir2_opnd, &count);
//    append_ir2_opnd2i(mips_andi, &tmp_count, &tmp_count,
//                      ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
//    IR2_OPND high_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_srlv, &high_dest, &dest, &tmp_count);
//    ra_free_temp(&tmp_count);
//
//    append_ir2_opnd3(mips_or, &dest, &high_dest, &low_dest);
//
//    store_ir2_to_ir1(&dest, ir1_get_opnd(pir1, 0), false);
//
//    append_ir2_opnd1(mips_label, &label_calc_eflags);
//    generate_eflag_calculation(&dest, &dest, &count, pir1, true);
//
//    ra_free_temp(&high_dest);
//    ra_free_temp(&low_dest);
//    ra_free_temp(&count);
//    append_ir2_opnd1(mips_label, &label_exit);
    return true;
}

bool translate_rol_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 1))) {
            return translate_rol_byhand_imm(pir1);
        } else {
            return translate_rol_byhand_cl(pir1);
        }
    } else {
        return translate_rol(pir1);
    }
}

bool translate_ror_byhand_imm(IR1_INST *pir1)
{
    lsassertm(0, "ror byhand to be implemented in LoongArch.\n");
//    IR2_OPND dest = ra_alloc_itemp();
//
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
//        load_ir1_to_ir2(&dest, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//    } else {
//        dest = ra_alloc_itemp();
//        load_ir1_to_ir2(&dest, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//    }
//
//    int count = ir1_opnd_simm(ir1_get_opnd(pir1, 1));
//    count = count & 0x1f;
//
//    if (count == 0) {
//        return true;
//    }
//
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8){
//            append_ir2_opnd1i(mips_x86rotrb, &dest, count);
//        }
//        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16){
//            append_ir2_opnd1i(mips_x86rotrh, &dest, count);
//        }
//        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32){
//            append_ir2_opnd1i(mips_x86rotrw, &dest, count);
//        }
//    }
//
//    count = count & (ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
//    IR2_OPND count_opnd = ir2_opnd_new(IR2_OPND_IREG, count);
//    if (count == 0) {
//        generate_eflag_calculation(&dest, &dest, &count_opnd, pir1, true);
//        return true;
//    }
//
//    IR2_OPND high_dest = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_srl, &high_dest, &dest, count);
//    int tmp_count = (0 - count) & (ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
//    IR2_OPND low_dest = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_sll, &low_dest, &dest, tmp_count);
//
//    append_ir2_opnd3(mips_or, &dest, &high_dest, &low_dest);
//
//    store_ir2_to_ir1(&dest, ir1_get_opnd(pir1, 0), false);
//
//    count_opnd = ir2_opnd_new(IR2_OPND_IREG, count);
//    generate_eflag_calculation(&dest, &dest, &count_opnd, pir1, true);
//
//    ra_free_temp(&high_dest);
//    ra_free_temp(&low_dest);

    return true;
}

bool translate_ror_byhand_cl(IR1_INST *pir1)
{
    lsassertm(0, "ror byhand to be implemented in LoongArch.\n");
//    IR2_OPND dest = ra_alloc_itemp();
//
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
//        load_ir1_to_ir2(&dest, ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
//    } else {
//        dest = ra_alloc_itemp();
//        load_ir1_to_ir2(&dest, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//    }
//
//    IR2_OPND original_count = ra_alloc_itemp();
//    load_ir1_to_ir2(&original_count, ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, false);
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
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8) {
//        append_ir2_opnd2i(mips_andi, &count, &count, 7);
//    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {
//        append_ir2_opnd2i(mips_andi, &count, &count, 15);
//    }
//
//    IR2_OPND label_calc_eflags = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &count, &zero_ir2_opnd, &label_calc_eflags);
//
//    IR2_OPND high_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_srlv, &high_dest, &dest, &count);
//    IR2_OPND tmp_count = ra_alloc_itemp();
//    append_ir2_opnd3(mips_subu, &tmp_count, &zero_ir2_opnd, &count);
//    append_ir2_opnd2i(mips_andi, &tmp_count, &tmp_count,
//                      ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
//    IR2_OPND low_dest = ra_alloc_itemp();
//    append_ir2_opnd3(mips_sllv, &low_dest, &dest, &tmp_count);
//    ra_free_temp(&tmp_count);
//
//    append_ir2_opnd3(mips_or, &dest, &high_dest, &low_dest);
//
//    store_ir2_to_ir1(&dest, ir1_get_opnd(pir1, 0), false);
//
//    append_ir2_opnd1(mips_label, &label_calc_eflags);
//    generate_eflag_calculation(&dest, &dest, &count, pir1, true);
//
//    ra_free_temp(&high_dest);
//    ra_free_temp(&low_dest);
//    ra_free_temp(&count);
//
//    append_ir2_opnd1(mips_label, &label_exit);
    return true;
}

bool translate_ror_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 1))) {
            return translate_ror_byhand_imm(pir1);
        } else {
            return translate_ror_byhand_cl(pir1);
        }
    } else {
        return translate_ror(pir1);
    }
}

bool translate_shl_byhand_imm(IR1_INST *pir1)
{
    lsassertm(0, "shl byhand to be implemented in LoongArch.\n");
//    int origin_count = ir1_opnd_simm(ir1_get_opnd(pir1, 1));
//    int count = origin_count & 0x1f;
//    if (count == 0) {
//        return true;
//    }
//
//    /* 1. prepare source */
//    IR2_OPND src = ra_alloc_itemp();
//    load_ir1_to_ir2(&src, ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
//
//    /* 2. adjust eflag calculation */
//    if (origin_count >= ir1_opnd_size(ir1_get_opnd(pir1, 0))) {
//        tr_skip_eflag_calculation(CF_USEDEF_BIT);
//    }
//    if (count != 1) {
//        tr_skip_eflag_calculation(OF_USEDEF_BIT);
//    }
//
//    /* 3. prepare dest */
//    bool dest_is_temp = false;
//    IR2_OPND dest = src;
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
//        if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_need_calculate_of(pir1) ||
//            ir1_need_calculate_cf(pir1)) {
//            dest_is_temp = true;
//            dest = ra_alloc_itemp();
//        }
//    } else {
//        dest_is_temp = true;
//        dest = ra_alloc_itemp();
//    }
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        IR2_OPND opnd = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 1)));
//        generate_eflag_by_lbt(&dest, &src, &opnd, pir1, true);
//    }
//
//    append_ir2_opnd2i(mips_sll, &dest, &src, count);
//    if (option_lbt) {
//        fp_save_dest_opnd(pir1, dest);
//    } else {
//        IR2_OPND tmp = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 1)));
//        generate_eflag_calculation(&dest, &src, &tmp, pir1, true);
//    }
//    store_ir2_to_ir1(&dest, ir1_get_opnd(pir1, 0), false);
//
//    if (dest_is_temp) {
//        ra_free_temp(&dest);
//    }
    return true;
}

bool translate_shl_byhand_cl(IR1_INST *pir1)
{
    lsassertm(0, "shl byhand to be implemented in LoongArch.\n");
//    /* 1. prepare count */
//    IR2_OPND original_count = ra_alloc_itemp();
//    load_ir1_to_ir2(&original_count, ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, false);
//
//    bool count_is_temp = false;
//    IR2_OPND count = original_count;
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    if (ir1_need_calculate_any_flag(pir1)) { /* when low 5 bits of count is 0,
//                                                the shift result is the */
//        /* same, but eflag calculation should be skipped */
//        count = ra_alloc_itemp();
//        count_is_temp = true;
//        append_ir2_opnd2i(mips_andi, &count, &original_count, 31);
//
//        append_ir2_opnd3(mips_beq, &count, &zero_ir2_opnd, &label_exit);
//    }
//
//    IR2_OPND src = ra_alloc_itemp();
//    load_ir1_to_ir2(&src, ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
//
//    bool dest_is_temp = false;
//    IR2_OPND dest = src;
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
//        if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_is_of_def(pir1) ||
//            ir1_is_cf_def(pir1)) {
//            dest_is_temp = true;
//            dest = ra_alloc_itemp();
//        }
//    } else {
//        dest_is_temp = true;
//        dest = ra_alloc_itemp();
//    }
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest, &src, &count, pir1, true);
//    }
//
//    append_ir2_opnd3(mips_sllv, &dest, &src, &count);
//    if (option_lbt) {
//        fp_save_dest_opnd(pir1, dest);
//    } else {
//        generate_eflag_calculation(&dest, &src, &count, pir1, true);
//    }
//    store_ir2_to_ir1(&dest, ir1_get_opnd(pir1, 0), false);
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    if (count_is_temp) {
//        ra_free_temp(&count);
//    }
//    if (dest_is_temp) {
//        ra_free_temp(&dest);
//    }
    return true;
}

bool translate_shl_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 1))) {
            return translate_shl_byhand_imm(pir1);
        } else {
            return translate_shl_byhand_cl(pir1);
        }
    } else {
        return translate_shl(pir1);
    }
}

bool translate_sal_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        return translate_shl_byhand(pir1);
    } else {
        return translate_sal(pir1);
    }
}

bool translate_sar_byhand_imm(IR1_INST *pir1)
{
    lsassertm(0, "sar byhand to be implemented in LoongArch.\n");
//    IR2_OPND src = ra_alloc_itemp();
//    load_ir1_to_ir2(&src, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//
//    int count = ir1_opnd_simm(ir1_get_opnd(pir1, 1)) & 0x1f;
//    if (count == 0) {
//        return true;
//    }
//
//    bool dest_is_temp = false;
//    IR2_OPND dest = src;
//
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
//        if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_is_of_def(pir1) ||
//            ir1_is_cf_def(pir1)) {
//            dest_is_temp = true;
//            dest = ra_alloc_itemp();
//        }
//    }
//    else {
//        dest_is_temp = true;
//        dest = ra_alloc_itemp();
//    }
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        IR2_OPND opnd = ir2_opnd_new(IR2_OPND_IMM, count);
//        generate_eflag_by_lbt(&dest, &src, &opnd, pir1, true);
//    }
//
//    append_ir2_opnd2i(mips_sra, &dest, &src, count);
//
//    if (option_lbt) {
//        fp_save_dest_opnd(pir1, dest);
//    } else {
//        IR2_OPND tmp = ir2_opnd_new(IR2_OPND_IMM, count);
//        generate_eflag_calculation(&dest, &src, &tmp, pir1, true);
//    }
//
//    store_ir2_to_ir1(&dest, ir1_get_opnd(pir1, 0), false);
//
//    if (dest_is_temp) {
//        ra_free_temp(&dest);
//    }
    return true;
}

bool translate_sar_byhand_cl(IR1_INST *pir1)
{
    lsassertm(0, "sar byhand to be implemented in LoongArch.\n");
//    IR2_OPND src = ra_alloc_itemp();
//    IR2_OPND original_count = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//    load_ir1_to_ir2(&original_count, ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, false);
//
//    bool dest_is_temp = false;
//    IR2_OPND dest = src;
//
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
//        if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_is_of_def(pir1) ||
//            ir1_is_cf_def(pir1)) {
//            dest_is_temp = true;
//            dest = ra_alloc_itemp();
//        }
//    }
//    else {
//        dest_is_temp = true;
//        dest = ra_alloc_itemp();
//    }
//
//    IR2_OPND count = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_andi, &count, &original_count, 31);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &count, &zero_ir2_opnd, &label_exit);
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest, &src, &count, pir1, true);
//    }
//
//    append_ir2_opnd3(mips_srav, &dest, &src, &count);
//
//    if (option_lbt) {
//        fp_save_dest_opnd(pir1, dest);
//    } else {
//        generate_eflag_calculation(&dest, &src, &count, pir1, true);
//    }
//
//    store_ir2_to_ir1(&dest, ir1_get_opnd(pir1, 0), false);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    if (dest_is_temp) {
//        ra_free_temp(&dest);
//    }
    return true;
}

bool translate_sar_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 1))) {
            return translate_sar_byhand_imm(pir1);
        } else {
            return translate_sar_byhand_cl(pir1);
        }
    } else {
        return translate_sar(pir1);
    }
}

bool translate_shr_byhand_imm(IR1_INST *pir1)
{
    lsassertm(0, "shr byhand to be implemented in LoongArch.\n");
//    int origin_count = ir1_opnd_simm(ir1_get_opnd(pir1, 1));
//    int count = origin_count & 0x1f;
//    if (count == 0) {
//        return true;
//    }
//
//    /* 1. prepare source */
//    EXTENSION_MODE em = ZERO_EXTENSION;
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
//        em = SIGN_EXTENSION;
//    }
//    IR2_OPND src = ra_alloc_itemp();
//    load_ir1_to_ir2(&src, ir1_get_opnd(pir1, 0), em, false);
//
//    /* 2. adjust eflag calculation */
//    if (origin_count >= ir1_opnd_size(ir1_get_opnd(pir1, 0))) {
//        tr_skip_eflag_calculation(CF_USEDEF_BIT);
//    }
//    if (count != 1) {
//        tr_skip_eflag_calculation(OF_USEDEF_BIT);
//    }
//
//    /* 3. prepare dest */
//    bool dest_is_temp = false;
//    IR2_OPND dest =
//        src; /* when 8/16 bits, if src is actually a mapped GPR, it means all */
//    /* high bits are zero. It's still correct if we overwrite the GPR */
//    if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_need_calculate_of(pir1) ||
//        ir1_need_calculate_cf(pir1)) {
//        dest_is_temp = true;
//        dest = ra_alloc_itemp();
//    }
//
//    /* 4. calculate */
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        IR2_OPND opnd = ir2_opnd_new(IR2_OPND_IMM, count);
//        generate_eflag_by_lbt(&dest, &src, &opnd, pir1, true);
//    }
//
//    append_ir2_opnd2i(mips_srl, &dest, &src, count);
//    if (option_lbt) {
//        fp_save_dest_opnd(pir1, dest);
//    } else {
//        IR2_OPND tmp = ir2_opnd_new(IR2_OPND_IMM, count);
//        generate_eflag_calculation(&dest, &src, &tmp, pir1, true);
//    }
//    store_ir2_to_ir1(&dest, ir1_get_opnd(pir1, 0), false);
//
//    if (dest_is_temp) {
//        ra_free_temp(&dest);
//    }
    return true;
}

bool translate_shr_byhand_cl(IR1_INST *pir1)
{
    lsassertm(0, "shr byhand to be implemented in LoongArch.\n");
//    /* 1. prepare count */
//    IR2_OPND original_count = ra_alloc_itemp();
//    load_ir1_to_ir2(&original_count, ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION,
//                           false); /* only last 5 bits matters */
//    bool count_is_temp = false;
//    IR2_OPND count = original_count;
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    if (ir1_need_calculate_any_flag(pir1)) { /* when low 5 bits of count is 0,
//                                                the shift result is the */
//        /* same, but eflag calculation should be skipped */
//        count = ra_alloc_itemp();
//        count_is_temp = true;
//        append_ir2_opnd2i(mips_andi, &count, &original_count, 31);
//
//        /* append_ir2_opnd2i(mips_andi, &original_count, &count , 18); */
//
//        append_ir2_opnd3(mips_beq, &count, &zero_ir2_opnd, &label_exit);
//    }
//
//    /* 2. prepare source */
//    EXTENSION_MODE em = ZERO_EXTENSION;
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
//        em = SIGN_EXTENSION;
//    }
//
//    IR2_OPND src = ra_alloc_itemp();
//    load_ir1_to_ir2(&src, ir1_get_opnd(pir1, 0), em, false);
//
//    /* 3. prepare dest */
//    bool dest_is_temp = false;
//    IR2_OPND dest = src; /*  when 8/16 bits, if src is actually a mapped GPR, it
//                            means all */
//    /*  high bits are zero. It's still correct if we overwrite the GPR */
//    if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_need_calculate_of(pir1) ||
//        ir1_need_calculate_cf(pir1)) {
//        dest_is_temp = true;
//        dest = ra_alloc_itemp();
//    }
//
//    /* 4. calculate */
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest, &src, &count, pir1, true);
//    }
//    append_ir2_opnd3(mips_srlv, &dest, &src, &count);
//    if (option_lbt) {
//        fp_save_dest_opnd(pir1, dest);
//    } else {
//        generate_eflag_calculation(&dest, &src, &count, pir1, true);
//    }
//    store_ir2_to_ir1(&dest, ir1_get_opnd(pir1, 0), false);
//
//    /* 5. exit */
//    append_ir2_opnd1(mips_label, &label_exit);
//    if (count_is_temp) {
//        ra_free_temp(&count);
//    }
//    if (dest_is_temp) {
//        ra_free_temp(&dest);
//    }
    return true;
}

bool translate_shr_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 1))) {
            return translate_shr_byhand_imm(pir1);
        } else {
            return translate_shr_byhand_cl(pir1);
        }
    } else {
        return translate_shr(pir1);
    }
}
