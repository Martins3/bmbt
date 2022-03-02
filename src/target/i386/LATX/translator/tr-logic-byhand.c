#include "common.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "flag-lbt.h"
#include "flag-pattern.h"
#include "translate.h"

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
    IR2_OPND src_opnd_0;
    IR2_OPND src_opnd_1;
    IR2_OPCODE lisa_opcode = LISA_INVALID;
    bool is_opnd_sx = false;

    /* 1. prepare source operands and opcode */
    if (ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0))) { /* dest is x86 address */
        src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
        lsassert(ir2_opnd_is_x86_address(&src_opnd_0));
        if (ir1_opnd_is_uimm_within_16bit(ir1_get_opnd(pir1, 1))) {
            ir2_opnd_build(&src_opnd_1, IR2_OPND_IMM,
                           ir1_opnd_uimm(ir1_get_opnd(pir1, 1)));
        } else {
            src_opnd_1 =
                load_ireg_from_ir1(ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, false);
        }
        lisa_opcode = LISA_AND;
    }

    else if (ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 1)) &&
             is_and) { /* src is x86 address and the result will be write back
                        */
        src_opnd_0 =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
        src_opnd_1 = load_ireg_from_ir1(ir1_get_opnd(pir1, 1), ZERO_EXTENSION, false);
        lsassert(ir2_opnd_is_x86_address(&src_opnd_1));
        lisa_opcode = LISA_AND;
    }

    else { /* none is x86 address or the result doesn't need to write back */
        src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
        if (ir1_opnd_is_uimm_within_16bit(ir1_get_opnd(pir1, 1))) {
            ir2_opnd_build(&src_opnd_1, IR2_OPND_IMM,
                           ir1_opnd_uimm(ir1_get_opnd(pir1, 1)));
        } else if (ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0)) &&
                   ir1_opnd_is_gpr(ir1_get_opnd(pir1, 1)) &&
                   ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)) ==
                       ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 1))) {
            src_opnd_1 = src_opnd_0;
        } else {
            src_opnd_1 =
                load_ireg_from_ir1(ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);
        }
        lisa_opcode = LISA_AND;
        is_opnd_sx = true;
    }

    /* 2. prepare dest operand */
    IR2_OPND dest_opnd = src_opnd_0;
    bool dest_opnd_is_temp = !is_and;
    if (dest_opnd_is_temp) {
        dest_opnd = ra_alloc_itemp();
    }

    if (!is_and && (32 - ir1_opnd_size(ir1_get_opnd(pir1, 0))) == 0 &&
        !ir1_need_calculate_any_flag(pir1)) {
        dest_opnd = ra_alloc_flag_pattern_saved_opnd0();
    }

    /* 3. calculate result */

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest_opnd, src_opnd_0, src_opnd_1, pir1,
                                   is_opnd_sx);
    }

    la_append_ir2_opnd3_em(lisa_opcode, dest_opnd, src_opnd_0, src_opnd_1);

#ifdef CONFIG_LATX_FLAG_PATTERN
    /* 4. calculate eflags */
    fp_save_dest_opnd(pir1, dest_opnd);
#endif

    /* 5. write the result back */
    if (is_and) {
        store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);
    }

    /* 6. release temp */
    if (dest_opnd_is_temp) {
        ra_free_temp(dest_opnd);
    }
    return true;
}

static bool translate_and_byhand_8_16(IR1_INST *pir1, bool is_and)
{
    bool is_opnd_sx = false;

    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    IR2_OPND src_opnd_1;
    //if (ir1_opnd_is_simm_within_16bit(ir1_get_opnd(pir1, 1))) {
    //    ir2_opnd_build(&src_opnd_1, IR2_OPND_IMM,
    //                   ir1_opnd_simm(ir1_get_opnd(pir1, 1)));
    //} else if (ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0)) &&
    if (ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0)) &&
        ir1_opnd_is_gpr(ir1_get_opnd(pir1, 1)) &&
        ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)) ==
			ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 1))) {
        src_opnd_1 = src_opnd_0;
    } else {
        src_opnd_1 =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, false);
    }

    /* 2. calculate result */
    IR2_OPND dest_opnd = ra_alloc_itemp();

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest_opnd, src_opnd_0, src_opnd_1, pir1,
                                   is_opnd_sx);
    }

    la_append_ir2_opnd3_em(LISA_AND, dest_opnd, src_opnd_0, src_opnd_1);

#ifdef CONFIG_LATX_FLAG_PATTERN
    /* 3. calculate eflags */
    fp_save_dest_opnd(pir1, dest_opnd);
#endif

    /* 4. write the result back */
    if (is_and) {
        store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);
    }

    /* 5. release temp */
    ra_free_temp(dest_opnd);
    return true;
}

bool translate_and_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_is_prefix_lock(pir1)) {
            return translate_and(pir1);
        }
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
    IR2_OPND dest;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        dest = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    } else {
            //dest = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
        dest = ra_alloc_itemp();
        load_ireg_from_ir1_2(dest, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
    }
    int count = ir1_opnd_simm(ir1_get_opnd(pir1, 1));
    count = count & 0x1f;
    if (count == 0) {
        return true;
    }

    if (ir1_need_calculate_any_flag(pir1)) {
        if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8){
            la_append_ir2_opnd1i(LISA_X86ROTLI_B, dest, count);
        }
        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16){
            la_append_ir2_opnd1i(LISA_X86ROTLI_H, dest, count);
        }
        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32){
            la_append_ir2_opnd1i(LISA_X86ROTLI_W, dest, count);
        }
    }

    count = count & (ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
    if (count == 0) {
        generate_eflag_calculation(
            dest, dest, ir2_opnd_new(IR2_OPND_IMM, count), pir1, true);
        return true;
    }

    IR2_OPND low_dest = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_SLLI_W, low_dest, dest, count);
    int tmp_count = (0 - count) & (ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
    IR2_OPND high_dest = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_SRLI_W, high_dest, dest, tmp_count);

    la_append_ir2_opnd3_em(LISA_OR, dest, high_dest, low_dest);

    store_ireg_to_ir1(dest, ir1_get_opnd(pir1, 0), false);

    generate_eflag_calculation(dest, dest, ir2_opnd_new(IR2_OPND_IMM, count),
                               pir1, true);

    ra_free_temp(high_dest);
    ra_free_temp(low_dest);
    return true;
}

bool translate_rol_byhand_cl(IR1_INST *pir1)
{
    IR2_OPND dest = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
    IR2_OPND original_count =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, false);
    IR2_OPND count = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ANDI, count, original_count, 0x1f);

    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_exit);

    if (ir1_need_calculate_any_flag(pir1)) {
        if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8){
            la_append_ir2_opnd2(LISA_X86ROTL_B, dest, original_count);
        }
        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16){
            la_append_ir2_opnd2(LISA_X86ROTL_H, dest, original_count);
        }
        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32){
            la_append_ir2_opnd2(LISA_X86ROTL_W, dest, original_count);
        }
    }

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8) {
        la_append_ir2_opnd2i_em(LISA_ANDI, count, count, 7);
    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {
        la_append_ir2_opnd2i_em(LISA_ANDI, count, count, 15);
    }

    IR2_OPND label_calc_eflags = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_calc_eflags);
    IR2_OPND low_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SLL_W, low_dest, dest, count);
    IR2_OPND tmp_count = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SUB_W, tmp_count, zero_ir2_opnd, count);
    la_append_ir2_opnd2i_em(LISA_ANDI, tmp_count, tmp_count,
                      ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
    IR2_OPND high_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SRL_W, high_dest, dest, tmp_count);
    ra_free_temp(tmp_count);

    la_append_ir2_opnd3_em(LISA_OR, dest, high_dest, low_dest);

    store_ireg_to_ir1(dest, ir1_get_opnd(pir1, 0), false);

    la_append_ir2_opnd1(LISA_LABEL, label_calc_eflags);
    generate_eflag_calculation(dest, dest, count, pir1, true);

    ra_free_temp(high_dest);
    ra_free_temp(low_dest);
    ra_free_temp(count);
    la_append_ir2_opnd1(LISA_LABEL, label_exit);
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
    IR2_OPND dest;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        dest = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    } else {
        //dest = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
        dest = ra_alloc_itemp();
        load_ireg_from_ir1_2(dest, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
    }
    int count = ir1_opnd_simm(ir1_get_opnd(pir1, 1));
    count = count & 0x1f;
    if (count == 0) {
        return true;
    }

    if (ir1_need_calculate_any_flag(pir1)) {
        if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8){
            la_append_ir2_opnd1i(LISA_X86ROTRI_B, dest, count);
        }
        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16){
            la_append_ir2_opnd1i(LISA_X86ROTRI_H, dest, count);
        }
        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32){
            la_append_ir2_opnd1i(LISA_X86ROTRI_W, dest, count);
        }
    }

    count = count & (ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
    if (count == 0) {
        generate_eflag_calculation(
            dest, dest, ir2_opnd_new(IR2_OPND_IMM, count), pir1, true);
        return true;
    }

    IR2_OPND high_dest = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_SRLI_W, high_dest, dest, count);
    int tmp_count = (0 - count) & (ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
    IR2_OPND low_dest = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_SLLI_W, low_dest, dest, tmp_count);

    la_append_ir2_opnd3_em(LISA_OR, dest, high_dest, low_dest);

    store_ireg_to_ir1(dest, ir1_get_opnd(pir1, 0), false);

    generate_eflag_calculation(dest, dest, ir2_opnd_new(IR2_OPND_IMM, count),
                               pir1, true);

    ra_free_temp(high_dest);
    ra_free_temp(low_dest);

    return true;
}

bool translate_ror_byhand_cl(IR1_INST *pir1)
{
    IR2_OPND dest;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        dest = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    } else {
            //dest = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
        dest = ra_alloc_itemp();
        load_ireg_from_ir1_2(dest, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
    }

    IR2_OPND original_count =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, false);
    IR2_OPND count = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ANDI, count, original_count, 0x1f);

    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_exit);

    if (ir1_need_calculate_any_flag(pir1)) {
        if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8){
            la_append_ir2_opnd2(LISA_X86ROTR_B, dest, original_count);
        }
        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16){
            la_append_ir2_opnd2(LISA_X86ROTR_H, dest, original_count);
        }
        else if(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32){
            la_append_ir2_opnd2(LISA_X86ROTR_W, dest, original_count);
        }
    }

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8) {
        la_append_ir2_opnd2i_em(LISA_ANDI, count, count, 7);
    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {
        la_append_ir2_opnd2i_em(LISA_ANDI, count, count, 15);
    }

    IR2_OPND label_calc_eflags = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_calc_eflags);

    IR2_OPND high_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SRL_W, high_dest, dest, count);
    IR2_OPND tmp_count = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SUB_W, tmp_count, zero_ir2_opnd, count);
    la_append_ir2_opnd2i_em(LISA_ANDI, tmp_count, tmp_count,
                      ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
    IR2_OPND low_dest = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SLL_W, low_dest, dest, tmp_count);
    ra_free_temp(tmp_count);

    la_append_ir2_opnd3_em(LISA_OR, dest, high_dest, low_dest);

    store_ireg_to_ir1(dest, ir1_get_opnd(pir1, 0), false);

    la_append_ir2_opnd1(LISA_LABEL, label_calc_eflags);
    generate_eflag_calculation(dest, dest, count, pir1, true);

    ra_free_temp(high_dest);
    ra_free_temp(low_dest);
    ra_free_temp(count);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);
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
    int origin_count = ir1_opnd_simm(ir1_get_opnd(pir1, 1));
    int count = origin_count & 0x1f;
    if (count == 0) {
        return true;
    }

    /* 1. prepare source */
    IR2_OPND src =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

    /* 2. adjust eflag calculation */
    if (origin_count >= ir1_opnd_size(ir1_get_opnd(pir1, 0))) {
        tr_skip_eflag_calculation(CF_USEDEF_BIT);
    }
    if (count != 1) {
        tr_skip_eflag_calculation(OF_USEDEF_BIT);
    }

    /* 3. prepare dest */
    bool dest_is_temp = false;
    IR2_OPND dest = src;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_need_calculate_of(pir1) ||
            ir1_need_calculate_cf(pir1)) {
            dest_is_temp = true;
            dest = ra_alloc_itemp();
        }
    } else {
        dest_is_temp = true;
        dest = ra_alloc_itemp();
    }

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(
            dest, src, ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 1))),
            pir1, true);
    }

    la_append_ir2_opnd2i_em(LISA_SLLI_W, dest, src, count);
#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_save_dest_opnd(pir1, dest);
#endif
    store_ireg_to_ir1(dest, ir1_get_opnd(pir1, 0), false);

    if (dest_is_temp) {
        ra_free_temp(dest);
    }
    return true;
}

bool translate_shl_byhand_cl(IR1_INST *pir1)
{
    /* 1. prepare count */
    IR2_OPND original_count =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, false);

    bool count_is_temp = false;
    IR2_OPND count = original_count;
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    if (ir1_need_calculate_any_flag(pir1)) { /* when low 5 bits of count is 0,
                                                the shift result is the */
        /* same, but eflag calculation should be skipped */
        count = ra_alloc_itemp();
        count_is_temp = true;
        la_append_ir2_opnd2i_em(LISA_ANDI, count, original_count, 31);

        la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_exit);
    }

    IR2_OPND src =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

    bool dest_is_temp = false;
    IR2_OPND dest = src;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_is_of_def(pir1) ||
            ir1_is_cf_def(pir1)) {
            dest_is_temp = true;
            dest = ra_alloc_itemp();
        }
    } else {
        dest_is_temp = true;
        dest = ra_alloc_itemp();
    }

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest, src, count, pir1, true);
    }

    la_append_ir2_opnd3_em(LISA_SLL_W, dest, src, count);
#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_save_dest_opnd(pir1, dest);
#endif
    store_ireg_to_ir1(dest, ir1_get_opnd(pir1, 0), false);
    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    if (count_is_temp) {
        ra_free_temp(count);
    }
    if (dest_is_temp) {
        ra_free_temp(dest);
    }
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
    IR2_OPND src = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    int count = ir1_opnd_simm(ir1_get_opnd(pir1, 1)) & 0x1f;
    if (count == 0) {
        return true;
    }
    bool dest_is_temp = false;
    IR2_OPND dest = src;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_is_of_def(pir1) ||
            ir1_is_cf_def(pir1)) {
            dest_is_temp = true;
            dest = ra_alloc_itemp();
        }
    } else {
        dest_is_temp = true;
        dest = ra_alloc_itemp();
    }

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest, src, ir2_opnd_new(IR2_OPND_IMM, count),
                                   pir1, true);
    }

    la_append_ir2_opnd2i_em(LISA_SRAI_W, dest, src, count);

#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_save_dest_opnd(pir1, dest);
#endif

    store_ireg_to_ir1(dest, ir1_get_opnd(pir1, 0), false);

    if (dest_is_temp) {
        ra_free_temp(dest);
    }
    return true;
}

bool translate_sar_byhand_cl(IR1_INST *pir1)
{
    IR2_OPND src = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    IR2_OPND original_count =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, false);
    bool dest_is_temp = false;
    IR2_OPND dest = src;

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_is_of_def(pir1) ||
            ir1_is_cf_def(pir1)) {
            dest_is_temp = true;
            dest = ra_alloc_itemp();
        }
    } else {
        dest_is_temp = true;
        dest = ra_alloc_itemp();
    }

    IR2_OPND count = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_ANDI, count, original_count, 31);
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_exit);

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest, src, count, pir1, true);
    }

    la_append_ir2_opnd3_em(LISA_SRA_W, dest, src, count);

#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_save_dest_opnd(pir1, dest);
#endif

    store_ireg_to_ir1(dest, ir1_get_opnd(pir1, 0), false);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    if (dest_is_temp) {
        ra_free_temp(dest);
    }
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
    int origin_count = ir1_opnd_simm(ir1_get_opnd(pir1, 1));
    int count = origin_count & 0x1f;
    if (count == 0) {
        return true;
    }

    /* 1. prepare source */
    EXTENSION_MODE em = ZERO_EXTENSION;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        em = SIGN_EXTENSION;
    }
    IR2_OPND src = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), em, false);

    /* 2. adjust eflag calculation */
    if (origin_count >= ir1_opnd_size(ir1_get_opnd(pir1, 0))) {
        tr_skip_eflag_calculation(CF_USEDEF_BIT);
    }
    if (count != 1) {
        tr_skip_eflag_calculation(OF_USEDEF_BIT);
    }

    /* 3. prepare dest */
    bool dest_is_temp = false;
    IR2_OPND dest =
        src; /* when 8/16 bits, if src is actually a mapped GPR, it means all */
    /* high bits are zero. It's still correct if we overwrite the GPR */
    if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_need_calculate_of(pir1) ||
        ir1_need_calculate_cf(pir1)) {
        dest_is_temp = true;
        dest = ra_alloc_itemp();
    }

    /* 4. calculate */

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest, src, ir2_opnd_new(IR2_OPND_IMM, count),
                                   pir1, true);
    }

    la_append_ir2_opnd2i_em(LISA_SRLI_W, dest, src, count);
#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_save_dest_opnd(pir1, dest);
#endif
    store_ireg_to_ir1(dest, ir1_get_opnd(pir1, 0), false);

    if (dest_is_temp) {
        ra_free_temp(dest);
    }
    return true;
}

bool translate_shr_byhand_cl(IR1_INST *pir1)
{
    /* 1. prepare count */
    IR2_OPND original_count =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION,
                           false); /* only last 5 bits matters */
    bool count_is_temp = false;
    IR2_OPND count = original_count;
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    if (ir1_need_calculate_any_flag(pir1)) { /* when low 5 bits of count is 0,
                                                the shift result is the */
        /* same, but eflag calculation should be skipped */
        count = ra_alloc_itemp();
        count_is_temp = true;
        la_append_ir2_opnd2i_em(LISA_ANDI, count, original_count, 31);

        la_append_ir2_opnd3(LISA_BEQ, count, zero_ir2_opnd, label_exit);
    }

    /* 2. prepare source */
    EXTENSION_MODE em = ZERO_EXTENSION;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        em = SIGN_EXTENSION;
    }
    IR2_OPND src = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), em, false);

    /* 3. prepare dest */
    bool dest_is_temp = false;
    IR2_OPND dest = src; /*  when 8/16 bits, if src is actually a mapped GPR, it
                            means all */
    /*  high bits are zero. It's still correct if we overwrite the GPR */
    if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_need_calculate_of(pir1) ||
        ir1_need_calculate_cf(pir1)) {
        dest_is_temp = true;
        dest = ra_alloc_itemp();
    }

    /* 4. calculate */

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest, src, count, pir1, true);
    }

    la_append_ir2_opnd3_em(LISA_SRL_W, dest, src, count);
#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_save_dest_opnd(pir1, dest);
#endif
    store_ireg_to_ir1(dest, ir1_get_opnd(pir1, 0), false);

    /* 5. exit */
    la_append_ir2_opnd1(LISA_LABEL, label_exit);
    if (count_is_temp) {
        ra_free_temp(count);
    }
    if (dest_is_temp) {
        ra_free_temp(dest);
    }
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
