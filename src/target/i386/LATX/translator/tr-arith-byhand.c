#include "common.h"
#include "env.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "flag-lbt.h"
#include "flag-pattern.h"
#include "translate.h"

bool translate_cmpxchg_byhand_8_16_32(IR1_INST *pir1);

static bool translate_add_byhand_32(IR1_INST *pir1)
{
    IR2_OPND src_opnd_0;
    IR2_OPND src_opnd_1;
    IR2_OPCODE lisa_opcode = LISA_INVALID;
    bool is_opnd_sx = false;

    /* 1. prepare source operands and opcode */
    if (ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0))) { /* dest is x86 address */
        lsassert(!ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 1)));
        src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
        lsassert(ir2_opnd_is_x86_address(&src_opnd_0));
        if (ir1_opnd_is_simm_within_16bit(ir1_get_opnd(pir1, 0) + 1))
            src_opnd_1 =
                ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1));
        else
            src_opnd_1 =
                load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);

        lisa_opcode = LISA_ADD_ADDRX;
    } else if (ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0) +
                                       1)) { /* src is x86 address */
        src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
        src_opnd_1 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
        lsassert(ir2_opnd_is_x86_address(&src_opnd_1));
        lisa_opcode = LISA_ADD_ADDRX;
    } else { /* none is x86 address */
        src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
        if (ir1_opnd_is_simm_within_16bit(ir1_get_opnd(pir1, 0) + 1))
            src_opnd_1 =
                ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1));
        else
            src_opnd_1 =
                load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
        lisa_opcode = LISA_ADD_W;
        is_opnd_sx = true;
    }

    /* 2. prepare dest operand */
    IR2_OPND dest_opnd = src_opnd_0;
    bool dest_opnd_is_temp =
        ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_need_calculate_of(pir1);
    if (dest_opnd_is_temp)
        dest_opnd = ra_alloc_itemp();

    /* 3. calculate result */
#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);
#endif

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
    store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);

    /* 6. release temp */
    if (dest_opnd_is_temp)
        ra_free_temp(dest_opnd);
    return true;
}

static bool translate_add_byhand_8_16(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    EXTENSION_MODE src_prefer_em = UNKNOWN_EXTENSION;
    bool is_opnd_sx = false;
    IR2_OPCODE lisa_opcode = LISA_ADD_D;

    /* 1. src prefer extension_mode */
    if (ir1_need_calculate_of(pir1)) {
        src_prefer_em = SIGN_EXTENSION;
        is_opnd_sx = true;
        lisa_opcode = LISA_ADD_W;
    } else if (ir1_need_calculate_cf(pir1)) {
        src_prefer_em = ZERO_EXTENSION;
    }

    IR2_OPND src_opnd_0 = load_ireg_from_ir1(opnd0, src_prefer_em, false);
    IR2_OPND src_opnd_1;
    if (src_prefer_em == SIGN_EXTENSION && ir1_opnd_is_simm_within_16bit(opnd1))
        src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(opnd1));
    else if (src_prefer_em == ZERO_EXTENSION &&
             ir1_opnd_is_simm_within_16bit(opnd1) &&
             ir1_opnd_simm(opnd1) >= 0)
        src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(opnd1));
    else if (ir1_opnd_is_gpr(opnd0) && ir1_opnd_is_gpr(opnd1) &&
             !ir1_opnd_is_8h(opnd0) && !ir1_opnd_is_8h(opnd1) &&
             ir1_opnd_base_reg_num(opnd0) == ir1_opnd_base_reg_num(opnd1))
        src_opnd_1 = src_opnd_0;
    else
        src_opnd_1 = load_ireg_from_ir1(opnd1, src_prefer_em, false);

    /* 2. calculate result */
#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);
#endif
    IR2_OPND dest_opnd = ra_alloc_itemp();

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest_opnd, src_opnd_0, src_opnd_1, pir1,
                                   is_opnd_sx);
    }

    la_append_ir2_opnd3_em(lisa_opcode, dest_opnd, src_opnd_0, src_opnd_1);

#ifdef CONFIG_LATX_FLAG_PATTERN
    /* 3. calculate eflags */
    fp_save_dest_opnd(pir1, dest_opnd);
#endif

    /* 4. write the result back */
    store_ireg_to_ir1(dest_opnd, opnd0, false);

    /* 5. release temp */
    ra_free_temp(dest_opnd);
    return true;
}

bool translate_add_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_is_prefix_lock(pir1)) {
            return translate_add(pir1);
        }
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
            return translate_add_byhand_32(pir1);
        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_add_byhand_8_16(pir1);
    }
    return translate_add(pir1);
}

static int ir1_opnd_is_sub_12bit_imm(IR1_OPND *opnd)
{
    return ir1_opnd_is_imm(opnd) && ir1_opnd_simm(opnd) > -2048 &&
           ir1_opnd_simm(opnd) <= 2048;
}

static bool translate_sub_byhand_32(IR1_INST *pir1, bool is_sub)
{
    IR2_OPND src_opnd_0;
    IR2_OPND src_opnd_1;
    IR2_OPCODE lisa_opcode = LISA_INVALID;
    bool is_opnd_sx = false;

    /* 1. prepare source operands and opcode */
    if (ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0) + 1) &&
        is_sub) { /* conclusion: dest and source are both x86 address */
        if (ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0))) {
            em_convert_gpr_to_addrx(ir1_get_opnd(pir1, 0));
            src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
            lsassert(ir2_opnd_is_x86_address(&src_opnd_0));
        } else { /* src_opnd_0 may be a temp register, when opnd 0 is mem */
            src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
            ir2_opnd_set_em(&src_opnd_0, EM_X86_ADDRESS, 32);
        }
        src_opnd_1 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
        lsassert(ir2_opnd_is_x86_address(&src_opnd_1));
        lisa_opcode = LISA_SUB_ADDRX;
    } else if (ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0)) &&
               ir1_opnd_is_simm_within_16bit(ir1_get_opnd(pir1, 0) + 1)) {
        src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
        lsassert(ir2_opnd_is_x86_address(&src_opnd_0));
        src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1));
        lisa_opcode = LISA_SUBI_ADDRX;
    } else { /* difficult to determine which is x86 address */
        src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
        if (ir1_opnd_is_sub_12bit_imm(ir1_get_opnd(pir1, 0) + 1))
            src_opnd_1 =
                ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1));
        else
            src_opnd_1 =
                load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
        lisa_opcode = LISA_SUB_W;
        is_opnd_sx = true;
    }

    /* 2. prepare dest operand */
    IR2_OPND dest_opnd = src_opnd_0;
    bool dest_opnd_is_temp = ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || !is_sub ||
                             ir1_need_calculate_of(pir1) ||
                             ir1_need_calculate_cf(pir1);
    if (dest_opnd_is_temp)
        dest_opnd = ra_alloc_itemp();

    /* if(ir1_opcode(pir1) == x86_cmp &&  ( 32 - ir1_opnd_size(ir1_get_opnd(pir1, 0)) ) */
    /* ==0 &&!ir1_need_calculate_any_flag(pir1)&& dest_opnd.is_sx(32)) { */
    if (ir1_opcode(pir1) == X86_INS_CMP && (32 - ir1_opnd_size(ir1_get_opnd(pir1, 0))) == 0 &&
        !ir1_need_calculate_any_flag(pir1)) {
        dest_opnd = ra_alloc_flag_pattern_saved_opnd0();
    }

    /* 3. calculate result */
#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);
#endif

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest_opnd, src_opnd_0, src_opnd_1, pir1,
                                   is_opnd_sx);
    }

    if (is_sub || ir1_need_calculate_any_flag(pir1))
        la_append_ir2_opnd3_em(lisa_opcode, dest_opnd, src_opnd_0, src_opnd_1);
#ifdef CONFIG_LATX_FLAG_PATTERN
    else if (fp_is_save_dest_opnd(pir1, dest_opnd))
        la_append_ir2_opnd3_em(lisa_opcode, dest_opnd, src_opnd_0, src_opnd_1);
#endif

#ifdef CONFIG_LATX_FLAG_PATTERN
    /* 4. calculate eflags */
    fp_save_dest_opnd(pir1, dest_opnd);
#endif

    /* 5. write the result back */
    if (is_sub)
        store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);

    /* 6. release temp */
    if (dest_opnd_is_temp)
        ra_free_temp(dest_opnd);
    return true;
}

static bool translate_sub_byhand_8_16(IR1_INST *pir1, bool is_sub)
{
    EXTENSION_MODE src_prefer_em = UNKNOWN_EXTENSION;
    bool is_opnd_sx = false;
    IR2_OPCODE lisa_opcode = LISA_SUB_D;

    /* 1. src prefer extension_mode */
    if (ir1_need_calculate_of(pir1)) {
        src_prefer_em = SIGN_EXTENSION;
        is_opnd_sx = true;
        lisa_opcode = LISA_SUB_W;
    } else if (ir1_need_calculate_cf(pir1)) {
        src_prefer_em = ZERO_EXTENSION;
    }

    IR2_OPND src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), src_prefer_em, false);
    IR2_OPND src_opnd_1;
    if (src_prefer_em == SIGN_EXTENSION &&
        ir1_opnd_is_simm_within_16bit(ir1_get_opnd(pir1, 0) + 1))
        src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1));
    else if (src_prefer_em == ZERO_EXTENSION &&
             ir1_opnd_is_simm_within_16bit(ir1_get_opnd(pir1, 0) + 1) &&
             ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1) >= 0)
        src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(ir1_get_opnd(pir1, 0) + 1));
    else
        src_opnd_1 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, src_prefer_em, false);

    /* 2. calculate result */
    IR2_OPND dest_opnd = ra_alloc_itemp();
#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);
#endif

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest_opnd, src_opnd_0, src_opnd_1, pir1,
                                   is_opnd_sx);
    }

    la_append_ir2_opnd3_em(lisa_opcode, dest_opnd, src_opnd_0, src_opnd_1);

#ifdef CONFIG_LATX_FLAG_PATTERN
    /* 3. calculate eflags */
    fp_save_dest_opnd(pir1, dest_opnd);
#endif

    /* 4. write the result back */
    if (is_sub)
        store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);

    /* 5. release temp */
    ra_free_temp(dest_opnd);
    return true;
}

bool translate_sub_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_is_prefix_lock(pir1)) {
            return translate_sub(pir1);
        }
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
            return translate_sub_byhand_32(pir1, true);
        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_sub_byhand_8_16(pir1, true);
    }
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
    }
    return translate_cmp(pir1);
}

static bool translate_adc_byhand_32(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR2_OPND src_opnd_0;
    IR2_OPND src_opnd_1;
    bool is_opnd_sx = false;

    /* 1. prepare source operands and opcode */
    if(ir1_opnd_is_x86_address(opnd0) || ir1_opnd_is_x86_address(opnd1))
        return translate_adc(pir1);

    /* none is x86 address */
    src_opnd_0 = load_ireg_from_ir1(opnd0, SIGN_EXTENSION, false);

    if (ir1_opnd_is_simm_within_16bit(opnd1))
        src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(opnd1));
    else
        src_opnd_1 = load_ireg_from_ir1(opnd1, SIGN_EXTENSION, false);

    is_opnd_sx = true;

    /* 2. prepare cf_opnd from eflags */
    IR2_OPND tmp_opnd = ra_alloc_itemp();
    load_ireg_from_cf_opnd(&tmp_opnd);

    /* 3. prepare dest operand */
    IR2_OPND dest_opnd = src_opnd_0;
    bool dest_opnd_is_temp = ir1_opnd_is_mem(opnd0) ||
                             ir1_need_calculate_of(pir1) ||
                             ir1_need_calculate_cf(pir1);
    if (dest_opnd_is_temp)
        dest_opnd = ra_alloc_itemp();

    /* 4. calculate result */

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest_opnd, src_opnd_0, src_opnd_1, pir1,
                                is_opnd_sx);
    }

    if (!ir2_opnd_is_imm(&src_opnd_1) || ir2_opnd_imm(&src_opnd_1) != 0)
        la_append_ir2_opnd3_em(LISA_ADD_W, tmp_opnd, tmp_opnd, src_opnd_1);

    la_append_ir2_opnd3_em(LISA_ADD_W, dest_opnd, src_opnd_0, tmp_opnd);

#ifdef CONFIG_LATX_FLAG_PATTERN
    /* 5. calculate eflags */
    fp_save_dest_opnd(pir1, dest_opnd);
#endif

    /* 6. write the result back */
    store_ireg_to_ir1(dest_opnd, opnd0, false);

    /* 6. release temp */
    if (dest_opnd_is_temp)
        ra_free_temp(dest_opnd);
    ra_free_temp(tmp_opnd);
    return true;
}

static bool translate_adc_byhand_8_16(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if(ir1_opnd_is_x86_address(opnd0) || ir1_opnd_is_x86_address(opnd1))
        return translate_adc(pir1);

    EXTENSION_MODE src_prefer_em = UNKNOWN_EXTENSION;
    bool is_opnd_sx = false;
    IR2_OPCODE lisa_opcode = LISA_ADD_D;
    /* 1. src prefer extension_mode */
    if (ir1_need_calculate_of(pir1)) {
        src_prefer_em = SIGN_EXTENSION;
        is_opnd_sx = true;
        lisa_opcode = LISA_ADD_W;
    } else if (ir1_need_calculate_cf(pir1)) {
        src_prefer_em = ZERO_EXTENSION;
    }

    IR2_OPND src_opnd_0 = load_ireg_from_ir1(opnd0, src_prefer_em, false);
    IR2_OPND src_opnd_1;
    if (src_prefer_em == SIGN_EXTENSION && ir1_opnd_is_simm_within_16bit(opnd1))
        src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(opnd1));
    else if (src_prefer_em == ZERO_EXTENSION &&
             ir1_opnd_is_simm_within_16bit(opnd1) &&
             ir1_opnd_simm(opnd1) >= 0)
        src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_simm(opnd1));
    else if (ir1_opnd_is_gpr(opnd0) && ir1_opnd_is_gpr(opnd1) &&
             !ir1_opnd_is_8h(opnd0) && !ir1_opnd_is_8h(opnd1) &&
             ir1_opnd_base_reg_num(opnd0) == ir1_opnd_base_reg_num(opnd1))
        src_opnd_1 = src_opnd_0;
    else
        src_opnd_1 = load_ireg_from_ir1(opnd1, src_prefer_em, false);

    /* 2. load cf */
    IR2_OPND cf_opnd = ra_alloc_itemp();
    load_ireg_from_cf_opnd(&cf_opnd);

    /* 3. calculate result */
    IR2_OPND dest_opnd = ra_alloc_itemp();

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest_opnd, src_opnd_0, src_opnd_1, pir1,
                                   is_opnd_sx);
    }

    la_append_ir2_opnd3_em(lisa_opcode, dest_opnd, src_opnd_0, src_opnd_1);
    la_append_ir2_opnd3_em(LISA_ADD_D, dest_opnd, dest_opnd, cf_opnd);

#ifdef CONFIG_LATX_FLAG_PATTERN
    /* 4. calculate eflags */
    fp_save_dest_opnd(pir1, dest_opnd);
#endif

    /* 5. write the result back */
    store_ireg_to_ir1(dest_opnd, opnd0, false);

    /* 6. release temp */
    ra_free_temp(dest_opnd);
    ra_free_temp(cf_opnd);
    return true;
}

bool translate_adc_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_is_prefix_lock(pir1)) {
            return translate_adc(pir1);
        }
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
            return translate_adc_byhand_32(pir1);
        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_adc_byhand_8_16(pir1);
    }
    return translate_adc(pir1);
}

static bool translate_or_byhand_8_16_32(IR1_INST *pir1)
{
    /* It's not likely that address will have 'or' operation */
    lsassert(!ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0)));
    lsassert(!ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0) + 1));

    bool is_opnd_sx = false;
    IR2_OPND src_opnd_1;

    /* 1. prepare src opnd */
    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    if (ir1_opnd_is_uimm_within_16bit(ir1_get_opnd(pir1, 0) + 1)) {
        src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, ir1_opnd_uimm(ir1_get_opnd(pir1, 0) + 1));
    } else {
        src_opnd_1 =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
    }
    /* 2. prepare dest opnd */
    /* OF is always cleared in 'or', so there is no need to alloc temp register
     */
    /* for dest to calc OF */
    IR2_OPND dest_opnd = src_opnd_0;
    bool dest_is_temp = true;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32 && ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0)))
        dest_is_temp = false;

    if (dest_is_temp)
        dest_opnd = ra_alloc_itemp();

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest_opnd, src_opnd_0, src_opnd_1, pir1,
                                   is_opnd_sx);
    }

    la_append_ir2_opnd3_em(LISA_OR, dest_opnd, src_opnd_0, src_opnd_1);
#ifdef CONFIG_LATX_FLAG_PATTERN
    /* 4. calculate eflags */
    fp_save_dest_opnd(pir1, dest_opnd);
#endif
    /* 5. write back */
    store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);

    /* 6. release temp register */
    if (dest_is_temp)
        ra_free_temp(dest_opnd);
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
    }
    return translate_or(pir1);
}

static bool translate_xor_byhand_8_16_32(IR1_INST *pir1)
{
    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, 0);

    bool is_opnd_sx = false;
    IR2_OPND src_opnd_1;

    if (ir1_opnd_is_uimm_within_16bit(ir1_get_opnd(pir1, 1))) {
        ir2_opnd_build(&src_opnd_1, IR2_OPND_IMM,
                       ir1_opnd_uimm(ir1_get_opnd(pir1, 1)));
    } else {
        src_opnd_1 = load_ireg_from_ir1(ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, 0);
    }

    IR2_OPND dest_opnd = src_opnd_0;
    int dest_is_temp = 1;

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32 &&
        ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0)))
        dest_is_temp = false;

    if (dest_is_temp)
        dest_opnd = ra_alloc_itemp();

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest_opnd, src_opnd_0, src_opnd_1, pir1,
                                   is_opnd_sx);
    }

    la_append_ir2_opnd3_em(LISA_XOR, dest_opnd, src_opnd_0, src_opnd_1);
    /* only OK for this */

#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_save_dest_opnd(pir1, dest_opnd);
#endif

    store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), 0); /* TODO */

    if (dest_is_temp)
        ra_free_temp(dest_opnd);

    return true;
}

bool translate_xor_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_is_prefix_lock(pir1)) {
            return translate_xor(pir1);
        }
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_xor_byhand_8_16_32(pir1);
    }
    return translate_xor(pir1);
}

static bool translate_inc_byhand_32(IR1_INST *pir1)
{
    IR2_OPND src_opnd_0;
    IR2_OPCODE lisa_opcode;
    bool is_opnd_sx = false;

    if (ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0))) {
        src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
        is_opnd_sx = false;
        lisa_opcode = LISA_ADD_ADDRX;
    } else {
        src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
        is_opnd_sx = true;
        lisa_opcode = LISA_ADD_W;
    }
    IR2_OPND src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, 1);

    IR2_OPND dest_opnd = src_opnd_0;
    bool dest_opnd_is_temp =
        ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_need_calculate_of(pir1);
    if (dest_opnd_is_temp)
        dest_opnd = ra_alloc_itemp();

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest_opnd, src_opnd_0, src_opnd_1, pir1,
                                   is_opnd_sx);
    }

    la_append_ir2_opnd3_em(lisa_opcode, dest_opnd, src_opnd_0, src_opnd_1);

#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_save_dest_opnd(pir1, dest_opnd);
#endif

    store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);

    if (dest_opnd_is_temp)
        ra_free_temp(dest_opnd);
    return true;
}

static bool translate_inc_byhand_8_16(IR1_INST *pir1)
{
    /* assert(0 && "translate_inc_byhand_8_16 need to be implemented correctly"); */
    /* 1. set src prefer_em */
    EXTENSION_MODE src_prefer_em = UNKNOWN_EXTENSION;
    bool is_opnd_sx = false;
    IR2_OPCODE lisa_opcode = LISA_ADD_D;
    if (ir1_need_calculate_of(pir1)) {
        src_prefer_em = SIGN_EXTENSION;
        is_opnd_sx = true;
        lisa_opcode = LISA_ADD_W;
    }

    /* 2. get src0 and src1 */
    IR2_OPND src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), src_prefer_em, false);
    IR2_OPND src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, 1);

    /* 3. calc dest */
    IR2_OPND dest_opnd = ra_alloc_itemp();

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest_opnd, src_opnd_0, src_opnd_1, pir1,
                                   is_opnd_sx);
    }

    la_append_ir2_opnd3_em(lisa_opcode, dest_opnd, src_opnd_0, src_opnd_1);

#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_save_dest_opnd(pir1, dest_opnd);
#endif

    store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);

    ra_free_temp(dest_opnd);
    return true;
}

bool translate_inc_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_is_prefix_lock(pir1)) {
            return translate_inc(pir1);
        }
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
            return translate_inc_byhand_32(pir1);
        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_inc_byhand_8_16(pir1);
    }
    return translate_inc(pir1);
}

static bool translate_dec_byhand_32(IR1_INST *pir1)
{
    IR2_OPND src_opnd_0, src_opnd_1, dest_opnd;
    IR2_OPCODE lisa_opcode;
    bool is_opnd_sx = false;
    bool dest_opnd_is_temp;

    if (ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0))) {
        src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
        is_opnd_sx = false;
        lisa_opcode = LISA_SUB_ADDRX;
    } else {
        src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
        is_opnd_sx = true;
        lisa_opcode = LISA_SUB_W;
    }
    src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, 1);
    dest_opnd_is_temp =
        ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_need_calculate_of(pir1);
    if (dest_opnd_is_temp)
        dest_opnd = ra_alloc_itemp();
    else
        dest_opnd = src_opnd_0;

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest_opnd, src_opnd_0, src_opnd_1, pir1,
                                   is_opnd_sx);
    }

    la_append_ir2_opnd3_em(lisa_opcode, dest_opnd, src_opnd_0, src_opnd_1);

#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_save_dest_opnd(pir1, dest_opnd);
#endif
    store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);
    if (dest_opnd_is_temp)
        ra_free_temp(dest_opnd);
    return true;
}

static bool translate_dec_byhand_8_16(IR1_INST *pir1)
{
    /* 1. set src prefer_em */
    EXTENSION_MODE src_prefer_em = UNKNOWN_EXTENSION;
    bool is_opnd_sx = false;
    IR2_OPCODE lisa_opcode = LISA_SUB_D;
    if (ir1_need_calculate_of(pir1)) {
        src_prefer_em = SIGN_EXTENSION;
        is_opnd_sx = true;
        lisa_opcode = LISA_SUB_W;
    }

    /* 2. get src0 and src1 */
    IR2_OPND src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), src_prefer_em, false);
    IR2_OPND src_opnd_1 = ir2_opnd_new(IR2_OPND_IMM, 1);

    /* 3. calc dest */
    IR2_OPND dest_opnd = ra_alloc_itemp();

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest_opnd, src_opnd_0, src_opnd_1, pir1,
                                   is_opnd_sx);
    }

    la_append_ir2_opnd3_em(lisa_opcode, dest_opnd, src_opnd_0, src_opnd_1);

#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_save_dest_opnd(pir1, dest_opnd);
#endif

    store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);

    ra_free_temp(dest_opnd);
    return true;
}

bool translate_dec_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_is_prefix_lock(pir1)) {
            return translate_dec(pir1);
        }
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
            return translate_dec_byhand_32(pir1);
        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_dec_byhand_8_16(pir1);
    }
    return translate_dec(pir1);
}

static bool translate_neg_byhand_32(IR1_INST *pir1)
{
    IR2_OPND src_opnd_0;
    bool is_opnd_sx = true;

    /* 1. prepare source operands and opcode */
    lsassert(!ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0)));
    src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);

    /* 3. prepare dest operand */
    IR2_OPND dest_opnd = src_opnd_0;
    bool dest_opnd_is_temp =
        ir1_opnd_is_mem(ir1_get_opnd(pir1, 0)) || ir1_need_calculate_of(pir1);
    if (dest_opnd_is_temp)
        dest_opnd = ra_alloc_itemp();

    /* 4. calculate result */

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest_opnd, zero_ir2_opnd, src_opnd_0, pir1,
                                   is_opnd_sx);
    }

    la_append_ir2_opnd3_em(LISA_SUB_W, dest_opnd, zero_ir2_opnd, src_opnd_0);

#ifdef CONFIG_LATX_FLAG_PATTERN
    /* 5. calculate eflags */
    fp_save_dest_opnd(pir1, dest_opnd);
#endif

    /* 6. write the result back */
    store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);

    /* 6. release temp */
    if (dest_opnd_is_temp)
        ra_free_temp(dest_opnd);
    return true;
}

static bool translate_neg_byhand_8_16(IR1_INST *pir1)
{
    /* 1. set src prefer_em */
    EXTENSION_MODE src_prefer_em = UNKNOWN_EXTENSION;
    bool is_opnd_sx = false;
    IR2_OPCODE lisa_opcode = LISA_SUB_D;
    if (ir1_need_calculate_of(pir1)) {
        src_prefer_em = SIGN_EXTENSION;
        is_opnd_sx = true;
        lisa_opcode = LISA_SUB_W;
    }

    /* 2. get src0 and src1 */
    IR2_OPND src_opnd_0 = zero_ir2_opnd;
    IR2_OPND src_opnd_1 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), src_prefer_em, false);

    /* 3. calc dest */
    IR2_OPND dest_opnd = ra_alloc_itemp();

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest_opnd, src_opnd_0, src_opnd_1, pir1,
                                   is_opnd_sx);
    }

    la_append_ir2_opnd3_em(lisa_opcode, dest_opnd, src_opnd_0, src_opnd_1);

#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_save_dest_opnd(pir1, dest_opnd);
#endif

    store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);

    ra_free_temp(dest_opnd);
    return true;
}

bool translate_neg_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_is_prefix_lock(pir1)) {
            return translate_neg(pir1);
        }
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
            return translate_neg_byhand_32(pir1);
        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_neg_byhand_8_16(pir1);
    }
    return translate_neg(pir1);
}

static bool translate_not_byhand_32(IR1_INST *pir1)
{
    IR2_OPND src0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    bool dest_opnd_is_temp = false;
    IR2_OPND dest = src0;

    if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 0))) {
        dest_opnd_is_temp = true;
        dest = ra_alloc_itemp();
    }
    la_append_ir2_opnd3_em(LISA_NOR, dest, zero_ir2_opnd, src0);

    store_ireg_to_ir1(dest, ir1_get_opnd(pir1, 0), false);

    if (dest_opnd_is_temp)
        ra_free_temp(dest);
    return true;
}

static bool translate_not_byhand_8_16(IR1_INST *pir1)
{
    IR2_OPND src0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    IR2_OPND dest = ra_alloc_itemp();

    la_append_ir2_opnd3_em(LISA_NOR, dest, zero_ir2_opnd, src0);

    store_ireg_to_ir1(dest, ir1_get_opnd(pir1, 0), false);

    ra_free_temp(dest);
    return true;
}

bool translate_not_byhand(IR1_INST *pir1)
{
    /* There is no way that an address has 'not' operation */
    if(!ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0))){
    //lsassert(!ir1_opnd_is_x86_address(ir1_get_opnd(pir1, 0)));
        if (option_by_hand) {
            if (ir1_is_prefix_lock(pir1)) {
                return translate_not(pir1);
            }
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
    return translate_not(pir1);
}

static bool translate_mul_byhand_32(IR1_INST *pir1)
{
    /* currently only one kind of optimization is found and applied: src1 = src2
     */
    IR2_OPND dest_opnd = ra_alloc_itemp();
    IR2_OPND src_opnd_1 =
        load_ireg_from_ir1(&eax_ir1_opnd, ZERO_EXTENSION, false);
    IR2_OPND src_opnd_0;
    if (ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)) ==
        ir1_opnd_base_reg_num(&eax_ir1_opnd))
        src_opnd_0 = src_opnd_1;
    else
        src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);

    la_append_ir2_opnd3_em(LISA_MUL_D, dest_opnd, src_opnd_1, src_opnd_0);

    store_ireg_to_ir1(dest_opnd, &eax_ir1_opnd, false);
    la_append_ir2_opnd2i_em(LISA_SRLI_D, dest_opnd, dest_opnd, 32);
    store_ireg_to_ir1(dest_opnd, &edx_ir1_opnd, false);
    generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);

    ra_free_temp(dest_opnd);
    return true;
}

static bool translate_mul_byhand_8_16(IR1_INST *pir1)
{
    /* currently only one kind of optimization is found and applied: src1 = src2
     */
    IR2_OPND src_opnd_0;
    IR2_OPND src_opnd_1;
    IR2_OPND dest_opnd = ra_alloc_itemp();

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8) {
        src_opnd_1 =
            load_ireg_from_ir1(&al_ir1_opnd, ZERO_EXTENSION, false);
        if (ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)) ==
            ir1_opnd_base_reg_num(&al_ir1_opnd))
            src_opnd_0 = src_opnd_1;
        else
            src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
        ir2_opnd_set_em(&src_opnd_0,ZERO_EXTENSION,16);
        ir2_opnd_set_em(&src_opnd_1,ZERO_EXTENSION,16);
        la_append_ir2_opnd3_em(LISA_MUL_W, dest_opnd, src_opnd_1, src_opnd_0);

        store_ireg_to_ir1(dest_opnd, &ax_ir1_opnd, false);
    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {
        src_opnd_1 =
            load_ireg_from_ir1(&ax_ir1_opnd, ZERO_EXTENSION, false);

        if (ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)) ==
            ir1_opnd_base_reg_num(&ax_ir1_opnd))
            src_opnd_0 = src_opnd_1;
        else
            src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);

        ir2_opnd_set_em(&src_opnd_0,ZERO_EXTENSION,16);
        ir2_opnd_set_em(&src_opnd_1,ZERO_EXTENSION,16);
        la_append_ir2_opnd3_em(LISA_MUL_W, dest_opnd, src_opnd_1, src_opnd_0);

        store_ireg_to_ir1(dest_opnd, &ax_ir1_opnd, false);
        la_append_ir2_opnd2i_em(LISA_SRLI_W, dest_opnd, dest_opnd, 16);
        store_ireg_to_ir1(dest_opnd, &dx_ir1_opnd, false);
    }

    generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);

    ra_free_temp(dest_opnd);
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
    }
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
    }
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
    }
    return translate_imul(pir1);
}

bool translate_cmpxchg_byhand_8_16_32(IR1_INST *pir1)
{
    /* 1. set prefer em */
    EXTENSION_MODE prefer_em = ZERO_EXTENSION;
    if (ir1_need_calculate_any_flag(pir1) || ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
        prefer_em = SIGN_EXTENSION;

    /* 2. load operands */
    IR1_OPND *reg_ir1 = NULL;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
        reg_ir1 = &eax_ir1_opnd;
    else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
        reg_ir1 = &al_ir1_opnd;
    else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16)
        reg_ir1 = &ax_ir1_opnd;
    IR2_OPND src_opnd_0 = load_ireg_from_ir1(ir1_get_opnd(pir1, 0), prefer_em, false);
    IR2_OPND eax_opnd = load_ireg_from_ir1(reg_ir1, prefer_em, false);

    /* 3. compare, and branch */
    IR2_OPND dest_opnd = ra_alloc_itemp();

    /* calculate eflags before calculate result if enabling lbt*/
    if (ir1_need_calculate_any_flag(pir1)) {
        generate_eflag_by_lbt(dest_opnd, eax_opnd, src_opnd_0, pir1,
                                   prefer_em == SIGN_EXTENSION);
    }

    la_append_ir2_opnd3_em(LISA_SUB_W, dest_opnd, eax_opnd, src_opnd_0);

#ifdef CONFIG_LATX_FLAG_PATTERN
    fp_save_dest_opnd(pir1, dest_opnd);
#endif

    IR2_OPND label_not_equal = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BNE, dest_opnd, zero_ir2_opnd, label_not_equal);

    /* 4. case when equal */
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32 && ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0))) {
        IR2_OPND dest_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)));
        load_ireg_from_ir1_2(dest_opnd, ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION,
                             false);
    } else {
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
        store_ireg_to_ir1(src_opnd, ir1_get_opnd(pir1, 0), false);
    }
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd1(LISA_B, label_exit);

    /* 5. case when not equal */
    la_append_ir2_opnd1(LISA_LABEL, label_not_equal);
    store_ireg_to_ir1(src_opnd_0, reg_ir1, false);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);
    ra_free_temp(dest_opnd);
    return true;
}

bool translate_cmpxchg_byhand(IR1_INST *pir1)
{
    if (option_by_hand) {
        if (ir1_is_prefix_lock(pir1)) {
            return translate_cmpxchg(pir1);
        }
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
            lsassertm(0, "%s for 64bit is not implemented yet.\n",
                      __FUNCTION__);
        } else
            return translate_cmpxchg_byhand_8_16_32(pir1);
    }
    return translate_cmpxchg(pir1);
}
