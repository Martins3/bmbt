#include "../include/common.h"
#include "../include/env.h"
#include "../include/reg_alloc.h"
#include "../x86tomips-options.h"
#include "../include/flag_lbt.h"
#include "../include/flag_pattern.h"

bool translate_cmpxchg_byhand_8_16_32(IR1_INST *pir1);

static bool translate_add_byhand_32(IR1_INST *pir1)
{
    lsassertm(0, "add byhand to be implemented in LoongArch.\n");
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//
//    IR2_OPCODE mips_opcode = mips_invalid;
//    bool is_opnd_sx = false;
//
//    /* 1. prepare source operands and opcode */
//    if (ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0))) { /* dest is x86 address */
//        lsassert(!ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0) + 1));
//        load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//        lsassert(ir2_opnd_is_x86_address(&src_opnd_0));
//        if (ir1_opnd_is_simm_within_16bit(ir1_get_opnd(pir1, 0) + 1))
//            src_opnd_1 =
//                ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1));
//        else
//            load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
//
//        mips_opcode = mips_add_addrx;
//    } else if (ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0) +
//                                       1)) { /* src is x86 address */
//        load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//        load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
//        lsassert(ir2_opnd_is_x86_address(&src_opnd_1));
//        mips_opcode = mips_add_addrx;
//    } else { /* none is x86 address */
//        load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//        if (ir1_opnd_is_simm_within_16bit(ir1_get_opnd(pir1, 0) + 1))
//            src_opnd_1 =
//                ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1));
//        else
//            load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
//
//        mips_opcode = mips_addu;
//        is_opnd_sx = true;
//    }
//
//    /* 2. prepare dest operand */
//    IR2_OPND dest_opnd = src_opnd_0;
//    bool dest_opnd_is_temp =
//        ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_need_calculate_of(pir1);
//    if (dest_opnd_is_temp)
//        dest_opnd = ra_alloc_itemp();
//
//    /* 3. calculate result */
//    fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
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
//    store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//
//    /* 6. release temp */
//    if (dest_opnd_is_temp)
//        ra_free_temp(&dest_opnd);
    return true;
}

static bool translate_add_byhand_8_16(IR1_INST *pir1)
{
    lsassertm(0, "add byhand to be implemented in LoongArch.\n");
//    EXTENSION_MODE src_prefer_em = UNKNOWN_EXTENSION;
//    bool is_opnd_sx = false;
//    IR2_OPCODE mips_opcode = mips_daddu;
//    /* 1. src prefer extension_mode */
//    if (ir1_need_calculate_of(pir1)) {
//        src_prefer_em = SIGN_EXTENSION;
//        is_opnd_sx = true;
//        mips_opcode = mips_addu;
//    } else if (ir1_need_calculate_cf(pir1)) {
//        src_prefer_em = ZERO_EXTENSION;
//    }
//
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), src_prefer_em, false);
//
//    if (src_prefer_em == SIGN_EXTENSION &&
//        ir1_opnd_is_simm_within_16bit(ir1_get_opnd(pir1, 0) + 1))
//        src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1));
//    else if (src_prefer_em == ZERO_EXTENSION &&
//             ir1_opnd_is_simm_within_16bit(ir1_get_opnd(pir1, 0) + 1) &&
//             ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1) >= 0)
//        src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1));
//    else if (ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0)) && ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0) + 1) &&
//             ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)) ==
//                 ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0) + 1))
//        src_opnd_1 = src_opnd_0;
//    else
//        load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 0) + 1, src_prefer_em, false);
//
//    /* 2. calculate result */
//    fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    append_ir2_opnd3(mips_opcode, &dest_opnd, &src_opnd_0, &src_opnd_1);
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
//    store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//
//    /* 5. release temp */
//    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_add_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
            return translate_add_byhand_32(pir1);
        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_add_byhand_8_16(pir1);
    } else
        return translate_add(pir1);
}

static bool translate_sub_byhand_32(IR1_INST *pir1, bool is_sub)
{
    lsassertm(0, "sub byhand to be implemented in LoongArch.\n");
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//
//    IR2_OPCODE mips_opcode = mips_invalid;
//    bool is_opnd_sx = false;
//
//    /* 1. prepare source operands and opcode */
//    if (ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0) + 1) &&
//        is_sub) { /* conclusion: dest and source are both x86 address */
//        if (ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0))) {
//            em_convert_gpr_to_addrx(ir1_get_opnd(pir1, 0));
//            load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//            lsassert(ir2_opnd_is_x86_address(&src_opnd_0));
//        } else { /* src_opnd_0 may be a temp register, when opnd 0 is mem */
//            load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//            ir2_opnd_set_em(&src_opnd_0, EM_X86_ADDRESS, 32);
//        }
//        load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
//        lsassert(ir2_opnd_is_x86_address(&src_opnd_1));
//        mips_opcode = mips_sub_addrx;
//    } else if (ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0)) &&
//               ir1_opnd_is_simm_within_16bit(ir1_get_opnd(pir1, 0) + 1)) {
//        load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//        lsassert(ir2_opnd_is_x86_address(&src_opnd_0));
//        src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1));
//        mips_opcode = mips_subi_addrx;
//    } else { /* difficult to determine which is x86 address */
//        load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//        if (ir1_opnd_is_simm_within_16bit(ir1_get_opnd(pir1, 0) + 1))
//            src_opnd_1 =
//                ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1));
//        else
//            load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
//
//        mips_opcode = mips_subu;
//        is_opnd_sx = true;
//    }
//
//    /* 2. prepare dest operand */
//    IR2_OPND dest_opnd = src_opnd_0;
//    bool dest_opnd_is_temp = ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || !is_sub ||
//                             ir1_need_calculate_of(pir1) ||
//                             ir1_need_calculate_cf(pir1);
//    if (dest_opnd_is_temp)
//        dest_opnd = ra_alloc_itemp();
//
//    /* if(ir1_opcode(pir1) == x86_cmp &&  ( 32 - ir1_opnd_size(ir1_get_opnd(pir1, 0)) ) */
//    /* ==0 &&!ir1_need_calculate_any_flag(pir1)&& dest_opnd.is_sx(32)) { */
//    if (ir1_opcode(pir1) == X86_INS_CMP && (32 - ir1_opnd_size(ir1_get_opnd(pir1, 0))) == 0 &&
//        !ir1_need_calculate_any_flag(pir1)) {
//        dest_opnd = ra_alloc_flag_pattern_saved_opnd0();
//    }
//
//    /* 3. calculate result */
//    fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    if (is_sub || fp_is_save_dest_opnd(pir1, dest_opnd) ||
//        ir1_need_calculate_any_flag(pir1))
//        append_ir2_opnd3(mips_opcode, &dest_opnd, &src_opnd_0, &src_opnd_1);
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
//    if (is_sub)
//        store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//
//    /* 6. release temp */
//    if (dest_opnd_is_temp)
//        ra_free_temp(&dest_opnd);
    return true;
}

static bool translate_sub_byhand_8_16(IR1_INST *pir1, bool is_sub)
{
    lsassertm(0, "sub byhand to be implemented in LoongArch.\n");
//    EXTENSION_MODE src_prefer_em = UNKNOWN_EXTENSION;
//    bool is_opnd_sx = false;
//    IR2_OPCODE mips_opcode = mips_dsubu;
//
//    /* 1. src prefer extension_mode */
//    if (ir1_need_calculate_of(pir1)) {
//        src_prefer_em = SIGN_EXTENSION;
//        is_opnd_sx = true;
//        mips_opcode = mips_subu;
//    } else if (ir1_need_calculate_cf(pir1)) {
//        src_prefer_em = ZERO_EXTENSION;
//    }
//
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), src_prefer_em, false);
//
//    if (src_prefer_em == SIGN_EXTENSION &&
//        ir1_opnd_is_simm_within_16bit(ir1_get_opnd(pir1, 0) + 1))
//        src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1));
//    else if (src_prefer_em == ZERO_EXTENSION &&
//             ir1_opnd_is_simm_within_16bit(ir1_get_opnd(pir1, 0) + 1) &&
//             ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1) >= 0)
//        src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1));
//    else
//        load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 0) + 1, src_prefer_em, false);
//
//    /* 2. calculate result */
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//    fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    append_ir2_opnd3(mips_opcode, &dest_opnd, &src_opnd_0, &src_opnd_1);
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
//    if (is_sub)
//        store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//
//    /* 5. release temp */
//    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_sub_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
            return translate_sub_byhand_32(pir1, true);
        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_sub_byhand_8_16(pir1, true);
    } else
        return translate_sub(pir1);
}

bool translate_cmp_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
            return translate_sub_byhand_32(pir1, false);
        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_sub_byhand_8_16(pir1, false);
    } else
        return translate_cmp(pir1);
}

static bool translate_adc_byhand_32(IR1_INST *pir1)
{
    lsassertm(0, "adc byhand to be implemented in LoongArch.\n");
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//
//    bool is_opnd_sx = false;
//
//    /* 1. prepare source operands and opcode */
//    lsassert(!ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0)) &&
//             !ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0) + 1));
//    /* none is x86 address */
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//    if (ir1_opnd_is_simm_within_16bit(ir1_get_opnd(pir1, 0) + 1)) {
//        src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1));
//    } else
//        load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
//    is_opnd_sx = true;
//
//    /* 2. prepare cf_opnd from eflags */
//    IR2_OPND tmp_opnd = ra_alloc_itemp();
//    load_eflags_cf_to_ir2(&tmp_opnd);
//
//    /* 3. prepare dest operand */
//    IR2_OPND dest_opnd = src_opnd_0;
//    bool dest_opnd_is_temp = ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) ||
//                             ir1_need_calculate_of(pir1) ||
//                             ir1_need_calculate_cf(pir1);
//    if (dest_opnd_is_temp)
//        dest_opnd = ra_alloc_itemp();
//
//    /* 4. calculate result */
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    if (!ir2_opnd_is_imm(&src_opnd_1) || ir2_opnd_imm(&src_opnd_1) != 0)
//        append_ir2_opnd3(mips_addu, &tmp_opnd, &tmp_opnd, &src_opnd_1);
//    append_ir2_opnd3(mips_addu, &dest_opnd, &src_opnd_0, &tmp_opnd);
//
//    /* 5. calculate eflags */
//    if (option_lbt) {
//        fp_save_dest_opnd(pir1, dest_opnd);
//    } else {
//        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    /* 6. write the result back */
//    store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//
//    /* 6. release temp */
//    if (dest_opnd_is_temp)
//        ra_free_temp(&dest_opnd);
//    ra_free_temp(&tmp_opnd);
    return true;
}

static bool translate_adc_byhand_8_16(IR1_INST *pir1)
{
    lsassertm(0, "adc byhand to be implemented in LoongArch.\n");
//    lsassert(!ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0)) &&
//             !ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0) + 1));
//
//    EXTENSION_MODE src_prefer_em = UNKNOWN_EXTENSION;
//    bool is_opnd_sx = false;
//    IR2_OPCODE mips_opcode = mips_daddu;
//    /* 1. src prefer extension_mode */
//    if (ir1_need_calculate_of(pir1)) {
//        src_prefer_em = SIGN_EXTENSION;
//        is_opnd_sx = true;
//        mips_opcode = mips_addu;
//    } else if (ir1_need_calculate_cf(pir1)) {
//        src_prefer_em = ZERO_EXTENSION;
//    }
//
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), src_prefer_em, false);
//
//    if (src_prefer_em == SIGN_EXTENSION &&
//        ir1_opnd_is_simm_within_16bit(ir1_get_opnd(pir1, 0) + 1))
//        src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1));
//    else if (src_prefer_em == ZERO_EXTENSION &&
//             ir1_opnd_is_simm_within_16bit(ir1_get_opnd(pir1, 0) + 1) &&
//             ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1) >= 0)
//        src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1));
//    else if (ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0)) && ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0) + 1) &&
//             ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)) ==
//                 ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0) + 1))
//        src_opnd_1 = src_opnd_0;
//    else
//        load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 0) + 1, src_prefer_em, false);
//
//    /* 2. load cf */
//    IR2_OPND cf_opnd = ra_alloc_itemp();
//    load_eflags_cf_to_ir2(&cf_opnd);
//
//    /* 3. calculate result */
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    append_ir2_opnd3(mips_opcode, &dest_opnd, &src_opnd_0, &src_opnd_1);
//    append_ir2_opnd3(mips_daddu, &dest_opnd, &dest_opnd, &cf_opnd);
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
//    store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//
//    /* 6. release temp */
//    ra_free_temp(&dest_opnd);
//    ra_free_temp(&cf_opnd);
    return true;
}

bool translate_adc_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
            return translate_adc_byhand_32(pir1);
        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_adc_byhand_8_16(pir1);
    } else
        return translate_adc(pir1);
}

static bool translate_or_byhand_8_16_32(IR1_INST *pir1)
{
    lsassertm(0, "or byhand to be implemented in LoongArch.\n");
//    /* It's not likely that address will have 'or' operation */
//    lsassert(!ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0)));
//    lsassert(!ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0) + 1));
//
//    bool is_opnd_sx = false;
//
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//
//    /* 1. prepare src opnd */
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
//    if (ir1_opnd_is_uimm_within_16bit(ir1_get_opnd(pir1, 0) + 1)) {
//        src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_uimm(ir1_get_opnd(pir1, 0) + 1));
//    } else {
//        load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
//    }
//    /* 2. prepare dest opnd */
//    /* OF is always cleared in 'or', so there is no need to alloc temp register
//     */
//    /* for dest to calc OF */
//    IR2_OPND dest_opnd = src_opnd_0;
//    bool dest_is_temp = true;
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32 && ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0)))
//        dest_is_temp = false;
//
//    if (dest_is_temp)
//        dest_opnd = ra_alloc_itemp();
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    append_ir2_opnd3(mips_or, &dest_opnd, &src_opnd_0, &src_opnd_1);
//    /* 4. calculate eflags */
//    if (option_lbt) {
//        fp_save_dest_opnd(pir1, dest_opnd);
//    } else {
//        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//    /* 5. write back */
//    store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//
//    /* 6. release temp register */
//    if (dest_is_temp)
//        ra_free_temp(&dest_opnd);
    return true;
}

bool translate_or_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_or_byhand_8_16_32(pir1);
    } else
        return translate_or(pir1);
}

static bool translate_xor_byhand_8_16_32(IR1_INST *pir1)
{
    lsassertm(0, "xor byhand to be implemented in LoongArch.\n");
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, 0);
//
//    bool is_opnd_sx = false;
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//
//    if (ir1_opnd_is_uimm_within_16bit(ir1_get_opnd(pir1, 1))) {
//        ir2_opnd_build(&src_opnd_1, IR2_OPND_IMM,
//                       ir1_opnd_uimm(ir1_get_opnd(pir1, 1)));
//    } else {
//        load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, 0);
//    }
//
//    IR2_OPND dest_opnd = src_opnd_0;
//    int dest_is_temp = 1;
//
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32 &&
//        ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0)))
//        dest_is_temp = false;
//
//    if (dest_is_temp)
//        dest_opnd = ra_alloc_itemp();
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    append_ir2_opnd3(mips_xor, &dest_opnd, &src_opnd_0, &src_opnd_1);
//    /* only OK for this */
//
//    if (option_lbt) {
//        fp_save_dest_opnd(pir1, dest_opnd);
//    } else {
//        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), 0);
//
//    if (dest_is_temp)
//        ra_free_temp(&dest_opnd);
//
    return true;
}

bool translate_xor_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_xor_byhand_8_16_32(pir1);
    } else
        return translate_xor(pir1);
}

static bool translate_inc_byhand_32(IR1_INST *pir1)
{
    lsassertm(0, "inc byhand to be implemented in LoongArch.\n");
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//
//    IR2_OPCODE mips_opcode;
//    bool is_opnd_sx = false;
//
//    if (ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0))) {
//        load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//        is_opnd_sx = false;
//        mips_opcode = mips_add_addrx;
//    } else {
//        load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//        is_opnd_sx = true;
//        mips_opcode = mips_addu;
//    }
//    IR2_OPND src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, 1);
//
//    IR2_OPND dest_opnd = src_opnd_0;
//    bool dest_opnd_is_temp =
//        ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_need_calculate_of(pir1);
//    if (dest_opnd_is_temp)
//        dest_opnd = ra_alloc_itemp();
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    append_ir2_opnd3(mips_opcode, &dest_opnd, &src_opnd_0, &src_opnd_1);
//
//    if (option_lbt) {
//        fp_save_dest_opnd(pir1, dest_opnd);
//    } else {
//        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//
//    if (dest_opnd_is_temp)
//        ra_free_temp(&dest_opnd);
    return true;
}

static bool translate_inc_byhand_8_16(IR1_INST *pir1)
{
    lsassertm(0, "inc byhand to be implemented in LoongArch.\n");
//    /* 1. set src prefer_em */
//    EXTENSION_MODE src_prefer_em = UNKNOWN_EXTENSION;
//    bool is_opnd_sx = false;
//    IR2_OPCODE mips_opcode = mips_daddu;
//    if (ir1_need_calculate_of(pir1)) {
//        src_prefer_em = SIGN_EXTENSION;
//        is_opnd_sx = true;
//        mips_opcode = mips_addu;
//    }
//
//    /* 2. get src0 and src1 */
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, 1);
//
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), src_prefer_em, false);
//
//    /* 3. calc dest */
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    append_ir2_opnd3(mips_opcode, &dest_opnd, &src_opnd_0, &src_opnd_1);
//
//    if (option_lbt) {
//        fp_save_dest_opnd(pir1, dest_opnd);
//    } else {
//        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//
//    ra_free_temp(&dest_opnd);
//    return true;
}

bool translate_inc_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
            return translate_inc_byhand_32(pir1);
        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_inc_byhand_8_16(pir1);
    } else
        return translate_inc(pir1);
}

static bool translate_dec_byhand_32(IR1_INST *pir1)
{
    lsassertm(0, "dec byhand to be implemented in LoongArch.\n");
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//    IR2_OPND dest_opnd;
//
//    IR2_OPCODE mips_opcode;
//    bool is_opnd_sx = false;
//    bool dest_opnd_is_temp;
//
//    if (ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0))) {
//        load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//        is_opnd_sx = false;
//        mips_opcode = mips_sub_addrx;
//    } else {
//        load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//        is_opnd_sx = true;
//        mips_opcode = mips_subu;
//    }
//    src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, 1);
//    dest_opnd_is_temp =
//        ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_need_calculate_of(pir1);
//    if (dest_opnd_is_temp)
//        dest_opnd = ra_alloc_itemp();
//    else
//        dest_opnd = src_opnd_0;
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    append_ir2_opnd3(mips_opcode, &dest_opnd, &src_opnd_0, &src_opnd_1);
//
//    if (option_lbt) {
//        fp_save_dest_opnd(pir1, dest_opnd);
//    } else {
//        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//    store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//    if (dest_opnd_is_temp)
//        ra_free_temp(&dest_opnd);
    return true;
}

static bool translate_dec_byhand_8_16(IR1_INST *pir1)
{
    lsassertm(0, "dec byhand to be implemented in LoongArch.\n");
//    /* 1. set src prefer_em */
//    EXTENSION_MODE src_prefer_em = UNKNOWN_EXTENSION;
//    bool is_opnd_sx = false;
//    IR2_OPCODE mips_opcode = mips_dsubu;
//    if (ir1_need_calculate_of(pir1)) {
//        src_prefer_em = SIGN_EXTENSION;
//        is_opnd_sx = true;
//        mips_opcode = mips_subu;
//    }
//
//    /* 2. get src0 and src1 */
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, 1);
//
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), src_prefer_em, false);
//
//    /* 3. calc dest */
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    append_ir2_opnd3(mips_opcode, &dest_opnd, &src_opnd_0, &src_opnd_1);
//
//    if (option_lbt) {
//        fp_save_dest_opnd(pir1, dest_opnd);
//    } else {
//        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//
//    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_dec_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
            return translate_dec_byhand_32(pir1);
        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_dec_byhand_8_16(pir1);
    } else
        return translate_dec(pir1);
}

static bool translate_neg_byhand_32(IR1_INST *pir1)
{
    lsassertm(0, "neg byhand to be implemented in LoongArch.\n");
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    bool is_opnd_sx = true;
//
//    /* 1. prepare source operands and opcode */
//    lsassert(!ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0)));
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
//
//    /* 3. prepare dest operand */
//    IR2_OPND dest_opnd = src_opnd_0;
//    bool dest_opnd_is_temp =
//        ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_need_calculate_of(pir1);
//    if (dest_opnd_is_temp)
//        dest_opnd = ra_alloc_itemp();
//
//    /* 4. calculate result */
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest_opnd, &zero_ir2_opnd, &src_opnd_0, pir1,
//                                   is_opnd_sx);
//    }
//
//    append_ir2_opnd3(mips_subu, &dest_opnd, &zero_ir2_opnd, &src_opnd_0);
//
//    /* 5. calculate eflags */
//    if (option_lbt) {
//        fp_save_dest_opnd(pir1, dest_opnd);
//    } else {
//        generate_eflag_calculation(&dest_opnd, &zero_ir2_opnd, &src_opnd_0, pir1,
//                                   is_opnd_sx);
//    }
//
//    /* 6. write the result back */
//    store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//
//    /* 6. release temp */
//    if (dest_opnd_is_temp)
//        ra_free_temp(&dest_opnd);
    return true;
}

static bool translate_neg_byhand_8_16(IR1_INST *pir1)
{
    lsassertm(0, "neg byhand to be implemented in LoongArch.\n");
//    /* 1. set src prefer_em */
//    EXTENSION_MODE src_prefer_em = UNKNOWN_EXTENSION;
//    bool is_opnd_sx = false;
//    IR2_OPCODE mips_opcode = mips_dsubu;
//    if (ir1_need_calculate_of(pir1)) {
//        src_prefer_em = SIGN_EXTENSION;
//        is_opnd_sx = true;
//        mips_opcode = mips_subu;
//    }
//
//    /* 2. get src0 and src1 */
//    IR2_OPND src_opnd_0 = zero_ir2_opnd;
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src_opnd_1, ir1_get_opnd(pir1, 0), src_prefer_em, false);
//
//    /* 3. calc dest */
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    append_ir2_opnd3(mips_opcode, &dest_opnd, &src_opnd_0, &src_opnd_1);
//
//    if (option_lbt) {
//        fp_save_dest_opnd(pir1, dest_opnd);
//    } else {
//        generate_eflag_calculation(&dest_opnd, &src_opnd_0, &src_opnd_1, pir1,
//                                   is_opnd_sx);
//    }
//
//    store_ir2_to_ir1(&dest_opnd, ir1_get_opnd(pir1, 0), false);
//
//    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_neg_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
            return translate_neg_byhand_32(pir1);
        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_neg_byhand_8_16(pir1);
    } else
        return translate_neg(pir1);
}

static bool translate_not_byhand_32(IR1_INST *pir1)
{
    lsassertm(0, "not byhand to be implemented in LoongArch.\n");
//    IR2_OPND src0 = ra_alloc_itemp();
//    bool dest_opnd_is_temp = false;
//    IR2_OPND dest = src0;
//
//    load_ir1_to_ir2(&src0, ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
//
//    if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 0))) {
//        dest_opnd_is_temp = true;
//        dest = ra_alloc_itemp();
//    }
//    append_ir2_opnd3(mips_nor, &dest, &zero_ir2_opnd, &src0);
//
//    store_ir2_to_ir1(&dest, ir1_get_opnd(pir1, 0), false);
//
//    if (dest_opnd_is_temp)
//        ra_free_temp(&dest);
    return true;
}

static bool translate_not_byhand_8_16(IR1_INST *pir1)
{
    lsassertm(0, "not byhand to be implemented in LoongArch.\n");
//    IR2_OPND src0 = ra_alloc_itemp();
//    IR2_OPND dest = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src0, ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
//
//    append_ir2_opnd3(mips_nor, &dest, &zero_ir2_opnd, &src0);
//
//    store_ir2_to_ir1(&dest, ir1_get_opnd(pir1, 0), false);
//
//    ra_free_temp(&dest);
    return true;
}

bool translate_not_byhand(IR1_INST *pir1)
{
    /* There is no way that an address has 'not' operation */
    lsassert(!ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0)));
    if (option_by_hand) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
            return translate_not_byhand_32(pir1);
        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_not_byhand_8_16(pir1);
    } else
        return translate_not(pir1);
}

static bool translate_mul_byhand_32(IR1_INST *pir1)
{
    lsassertm(0, "mul byhand to be implemented in LoongArch.\n");
//    /* currently only one kind of optimization is found and applied: src1 = src2
//     */
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//    IR2_OPND src_opnd_1 = ra_alloc_itemp();
//    load_ir1_to_ir2(&src_opnd_1, &eax_ir1_opnd, ZERO_EXTENSION, false);
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//
//    if (ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)) ==
//        ir1_opnd_base_reg_num(&eax_ir1_opnd))
//        src_opnd_0 = src_opnd_1;
//    else
//        load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//
//    append_ir2_opnd3(mips_dmultu_g, &dest_opnd, &src_opnd_1, &src_opnd_0);
//
//    store_ir2_to_ir1(&dest_opnd, &eax_ir1_opnd, false);
//    append_ir2_opnd2i(mips_dsrl32, &dest_opnd, &dest_opnd, 0);
//    store_ir2_to_ir1(&dest_opnd, &edx_ir1_opnd, false);
//    generate_eflag_calculation(&dest_opnd, NULL, NULL, pir1, true);
//
//    ra_free_temp(&dest_opnd);
    return true;
}

static bool translate_mul_byhand_8_16(IR1_INST *pir1)
{
    lsassertm(0, "mul byhand to be implemented in LoongArch.\n");
//    /* currently only one kind of optimization is found and applied: src1 = src2
//     */
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8) {
//        IR2_OPND src_opnd_1 = ra_alloc_itemp();
//        load_ir1_gpr_to_ir2(&src_opnd_1, &al_ir1_opnd, ZERO_EXTENSION);
//        if (ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)) ==
//            ir1_opnd_base_reg_num(&al_ir1_opnd))
//            src_opnd_0 = src_opnd_1;
//        else
//            load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//
//        append_ir2_opnd3(mips_multu_g, &dest_opnd, &src_opnd_1, &src_opnd_0);
//
//        store_ir2_to_ir1(&dest_opnd, &ax_ir1_opnd, false);
//    }
//    else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {
//        IR2_OPND src_opnd_1 = ra_alloc_itemp();
//        load_ir1_gpr_to_ir2(&src_opnd_1, &ax_ir1_opnd, ZERO_EXTENSION);
//
//        if (ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)) ==
//            ir1_opnd_base_reg_num(&ax_ir1_opnd))
//            src_opnd_0 = src_opnd_1;
//        else
//            load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//
//        append_ir2_opnd3(mips_multu_g, &dest_opnd, &src_opnd_1, &src_opnd_0);
//
//        store_ir2_to_ir1(&dest_opnd, &ax_ir1_opnd, false);
//        append_ir2_opnd2i(mips_srl, &dest_opnd, &dest_opnd, 16);
//        store_ir2_to_ir1(&dest_opnd, &dx_ir1_opnd, false);
//    }
//
//    generate_eflag_calculation(&dest_opnd, NULL, NULL, pir1, true);
//
//    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_mul_byhand(IR1_INST *pir1)
{
    /* There is no way that an address has 'mul' operation */
    lsassert(!ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0)));
    if (option_by_hand) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
            return translate_mul_byhand_32(pir1);
        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_mul_byhand_8_16(pir1);
    } else
        return translate_mul(pir1);
}

static bool translate_div_byhand_32(IR1_INST *pir1)
{
    /* currently no optimization found */
    return translate_div(pir1);
}

static bool translate_div_byhand_8_16(IR1_INST *pir1)
{
    /* currently no optimization found */
    return translate_div(pir1);
}

bool translate_div_byhand(IR1_INST *pir1)
{
    /* There is no way that an address has 'div' operation */
    lsassert(!ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0)));
    if (option_by_hand) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
            return translate_div_byhand_32(pir1);
        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_div_byhand_8_16(pir1);
    } else
        return translate_div(pir1);
}

static bool translate_imul_byhand_32(IR1_INST *pir1)
{
    /* currently no optimization found. */
    return translate_imul(pir1);
}

static bool translate_imul_byhand_8_16(IR1_INST *pir1)
{
    /* currently no optimization found */
    return translate_imul(pir1);
}

bool translate_imul_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        /* An address is not likely to have a 'imul' operation */
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
            return translate_imul_byhand_32(pir1);
        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64)
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        else
            return translate_imul_byhand_8_16(pir1);
    } else
        return translate_imul(pir1);
}

bool translate_cmpxchg_byhand_8_16_32(IR1_INST *pir1)
{
    lsassertm(0, "mul byhand to be implemented in LoongArch.\n");
//    /* 1. set prefer em */
//    EXTENSION_MODE prefer_em = ZERO_EXTENSION;
//    if (ir1_need_calculate_any_flag(pir1) || ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
//        prefer_em = SIGN_EXTENSION;
//
//    /* 2. load operands */
//    IR1_OPND *reg_ir1 = NULL;
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
//        reg_ir1 = &eax_ir1_opnd;
//    else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
//        reg_ir1 = &al_ir1_opnd;
//    else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16)
//        reg_ir1 = &ax_ir1_opnd;
//
//    IR2_OPND src_opnd_0 = ra_alloc_itemp();
//    IR2_OPND eax_opnd = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src_opnd_0, ir1_get_opnd(pir1, 0), prefer_em, false);
//    load_ir1_gpr_to_ir2(&eax_opnd, reg_ir1, prefer_em);
//
//    /* 3. compare, and branch */
//    IR2_OPND dest_opnd = ra_alloc_itemp();
//
//    /* calculate eflags before calculate result if enabling lbt*/
//    if (option_lbt && ir1_need_calculate_any_flag(pir1)) {
//        generate_eflag_by_lbt(&dest_opnd, &eax_opnd, &src_opnd_0, pir1,
//                                   prefer_em == SIGN_EXTENSION);
//    }
//
//    append_ir2_opnd3(mips_subu, &dest_opnd, &eax_opnd, &src_opnd_0);
//
//    if (option_lbt) {
//        fp_save_dest_opnd(pir1, dest_opnd);
//    } else {
//        generate_eflag_calculation(&dest_opnd, &eax_opnd, &src_opnd_0, pir1,
//                                   prefer_em == SIGN_EXTENSION);
//    }
//
//    IR2_OPND label_not_equal = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_bne, &dest_opnd, &zero_ir2_opnd, &label_not_equal);
//
//    /* 4. case when equal */
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32 && ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0))) {
//        IR2_OPND dest_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)));
//        load_ir1_to_ir2(&dest_opnd, ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, false);
//    }
//    else {
//        IR2_OPND src_opnd = ra_alloc_itemp();
//        load_ir1_to_ir2(&src_opnd, ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, false);
//        store_ir2_to_ir1(&src_opnd, ir1_get_opnd(pir1, 0), false);
//    }
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_b, &label_exit);
//
//    /* 5. case when not equal */
//    append_ir2_opnd1(mips_label, &label_not_equal);
//    store_ir2_to_ir1(&src_opnd_0, reg_ir1, false);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//    ra_free_temp(&dest_opnd);
    return true;
}

bool translate_cmpxchg_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_cmpxchg_byhand_8_16_32(pir1);
    } else
        return translate_cmpxchg(pir1);
}
