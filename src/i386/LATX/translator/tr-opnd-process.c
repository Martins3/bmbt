#include "../include/common.h"
#include "../include/reg-alloc.h"
#include "../include/env.h"
#include "../ir2/ir2.h"
#include "../x86tomips-options.h"

#include "../translator/tr-excp.h"

/* Calculate the memory address value of IR1_OPND to IR2_OPND
 * Effective Address = disp + [base + index << scale]
 *
 * @value_opnd : IR2_OPND_IREG
 * @opnd1 : IR1_OPND_MEM
 */
void load_ir1_mem_addr_to_ir2(
        IR2_OPND *value_opnd,
        IR1_OPND *opnd1,
        int addr_size)
{
    IR2_OPND ea = ra_alloc_itemp();
    int ea_valid = 0;
    int ea_base_only = 0;
    IR2_OPND ea_base;

    /* 1. ea = disp + [base + index << scale] */
    if (ir1_opnd_has_index(opnd1)) {
        /* 1.1 ea = base + index << scale */
        IR2_OPND index = ra_alloc_gpr(ir1_opnd_index_reg_num(opnd1));
        switch (ir1_opnd_scale(opnd1)) {
        case 1:  append_ir2_opnd2_(lisa_mov,    &ea, &index);    break;
        case 2:  append_ir2_opnd2i(LISA_SLLI_D, &ea, &index, 1);  break;
        case 4:  append_ir2_opnd2i(LISA_SLLI_D, &ea, &index, 2);  break;
        case 8:  append_ir2_opnd2i(LISA_SLLI_D, &ea, &index, 3);  break;
        default:
            lsassertm(0, "unknown scale %d of ir1 opnd.\n", ir1_opnd_scale(opnd1));
            break;
        }
        if (ir1_opnd_has_base(opnd1)) {
            IR2_OPND base = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
            append_ir2_opnd3(LISA_ADD_D, &ea, &ea, &base);
        }
        ea_valid = 1;
    }
    else if (ir1_opnd_has_base(opnd1)) {
        /* 1.2 ea = base */
        ea_base = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
        ea_base_only = 1;
        ea_valid = 1;
    }
    if (!ea_valid) {
        /* 1.3 ea = disp */
        longx offset = ir1_opnd_simm(opnd1);
        load_imm32_to_ir2(&ea, offset, SIGN_EXTENSION);
    }
    else if(ir1_opnd_simm(opnd1) != 0) {
        /* 1.4 ea = disp + ea */
        longx offset = ir1_opnd_simm(opnd1);
        if (int32_in_int12(offset)) {
            append_ir2_opnd2i(LISA_ADDI_D, &ea,
                    ea_base_only ? &ea_base : &ea, offset);
        } else {
            IR2_OPND offset_opnd = ra_alloc_itemp();
            load_imm32_to_ir2(&offset_opnd, offset, SIGN_EXTENSION);
            append_ir2_opnd3(LISA_ADD_D, &ea,
                    ea_base_only ? &ea_base : &ea, &offset_opnd);
            ra_free_temp(&offset_opnd);
        }
        ea_base_only = 0;
    } else if (ea_base_only) {
        append_ir2_opnd2_(lisa_mov, &ea, &ea_base);
        ea_base_only = 0;
    }

    /* 2. apply address size */
    switch (addr_size) {
    case 2:
        append_ir2_opnd2_(lisa_mov16z, &ea, &ea);
        break;
    case 4:
        append_ir2_opnd2_(lisa_mov32z, &ea, &ea);
        break;
    default:
        lsassertm(0, "unknown addr size %d in convert mem opnd.\n", addr_size);
        break;
    }

    /* 3. move to value */
    append_ir2_opnd2_(lisa_mov, value_opnd, &ea);
}

void convert_mem_opnd(
        IR2_OPND *opnd2,
        IR1_OPND *opnd1,
        int addr_size)
{
    if (addr_size < 0)
        addr_size = ir1_addr_size(lsenv->tr_data->curr_ir1_inst);
    convert_mem_opnd_with_bias(opnd2, opnd1, 0, addr_size);
}

/* Convert IR1_OPND_MEM to IR2_OPND_MEM
 * > Temp register might be used.
 * > The IR2_OPND_MEM's base will always be temp register !!!
 *
 * @opnd1 : IR1_OPND_MEM
 * @bias  : bias will be plused on offset
 */
void convert_mem_opnd_with_bias(
        IR2_OPND *opnd2,
        IR1_OPND *opnd1,
        int bias,
        int addr_size)
{
//    lsassertm(0, "OPND: convert mem to be implemented in LoongArch.\n");
    if (addr_size < 0)
        addr_size = ir1_addr_size(lsenv->tr_data->curr_ir1_inst);

//#ifdef CONFIG_SOFTMMU
//    longx imm = ir1_opnd_simm(opnd1) + bias;
//    if (xtm_risk_opt() && !lsenv->tr_data->sys.popl_esp_hack &&
//        !lsenv->tr_data->sys.addseg && lsenv->tr_data->sys.ss32 &&
//        addr_size == 4 && ir1_mem_opnd_is_base_imm(opnd1) &&
//        int32_in_int16(imm)) {
//        IR2_OPND mem_base_reg = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
//
//        if (!ir2_opnd_is_x86_address(&mem_base_reg)) {
//            append_ir2_opnd2(mips_mov_addrx, &mem_base_reg, &mem_base_reg);
//        }
//
//        ir2_opnd_build2(opnd2, IR2_OPND_MEM,
//                        ir2_opnd_base_reg_num(&mem_base_reg), imm);
//        return;
//    }
//
//    if (xtm_risk_opt() && !lsenv->tr_data->sys.popl_esp_hack &&
//        !lsenv->tr_data->sys.addseg && lsenv->tr_data->sys.ss32 &&
//        addr_size == 4 && !ir1_opnd_has_index(opnd1) &&
//        !ir1_opnd_has_base(opnd1) && !ir1_opnd_has_seg(opnd1)) {
//        IR2_OPND ea = ra_alloc_itemp();
//        load_imm32_to_ir2(&ea, imm, ZERO_EXTENSION);
//        ir2_opnd_build2(opnd2, IR2_OPND_MEM, ir2_opnd_base_reg_num(&ea), 0);
//        return;
//    }
//#endif

    IR2_OPND ea = ra_alloc_itemp();
    int ea_valid = 0;
    int ea_base_only = 0;
    IR2_OPND ea_base;

    int ea_ir1_reg = 0;
    int ea_off = 0;

    /* 1. ea = disp + [base + index << scale] */
    if (ir1_opnd_has_index(opnd1)) {
        /* 1.1 ea = base + index << scale */
        ea_ir1_reg = ir1_opnd_index_reg_num(opnd1);
        IR2_OPND index = ra_alloc_gpr(ea_ir1_reg);
        switch (ir1_opnd_scale(opnd1)) {
        case 1:  append_ir2_opnd3(LISA_OR, &ea, &index, &zero_ir2_opnd); break;
        case 2:  append_ir2_opnd2i(LISA_SLLI_D, &ea, &index, 1);  break;
        case 4:  append_ir2_opnd2i(LISA_SLLI_D, &ea, &index, 2);  break;
        case 8:  append_ir2_opnd2i(LISA_SLLI_D, &ea, &index, 3);  break;
        default:
            lsassertm(0, "unknown scale %d of ir1 opnd.\n", ir1_opnd_scale(opnd1));
            break;
        }
        if (ir1_opnd_has_base(opnd1)) {
            IR2_OPND base = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
            append_ir2_opnd3(LISA_ADD_D, &ea, &ea, &base);
        }
        ea_valid = 1;
    }
    else if (ir1_opnd_has_base(opnd1)) {
        /* 1.2 ea = base */
        ea_ir1_reg = ir1_opnd_base_reg_num(opnd1);
        ea_base = ra_alloc_gpr(ea_ir1_reg);
        ea_base_only = 1;
        ea_valid = 1;
    }

    if (!ea_valid) {
        /* 1.3 ea = disp */
        longx offset = ir1_opnd_simm(opnd1) + bias;
        load_imm32_to_ir2(&ea, offset, SIGN_EXTENSION);
    }
    else if(ir1_opnd_simm(opnd1) != 0 || bias != 0) {
        /* 1.4 ea = disp + ea */
        longx offset = ir1_opnd_simm(opnd1) + bias;
        if (int32_in_int12(offset)) {
            if (xtm_risk_opt() && /* allow to do risk optimization */
                    ea_ir1_reg != esp_index &&
                    ea_ir1_reg != ebp_index) {
                ea_off = offset;
            } else {
                append_ir2_opnd2i(LISA_ADDI_D, &ea,
                        ea_base_only ? &ea_base : &ea, offset);
                ea_base_only = 0;
            }
        } else {
            IR2_OPND offset_opnd = ra_alloc_itemp();
            load_imm32_to_ir2(&offset_opnd, offset, SIGN_EXTENSION);
            append_ir2_opnd3(LISA_ADD_D, &ea,
                    ea_base_only ? &ea_base : &ea, &offset_opnd);
            ra_free_temp(&offset_opnd);
            ea_base_only = 0;
        }
    }

    /* At here, 3 situations:
     * > ea_base_only = 1 with no ea_off
     * > ea_base_only = 0 with no ea_off
     * > ea_base_only = 1 with ea_off (risky) */

#ifdef CONFIG_SOFTMMU
    /* 1.1 fix pop (esp) address */
    TRANSLATION_DATA *td = lsenv->tr_data;
    if (td->sys.popl_esp_hack) {
        append_ir2_opnd2i(LISA_ADDI_D, &ea,
                ea_base_only ? &ea_base : &ea, td->sys.popl_esp_hack);
        ea_base_only = 0;
    }
#endif

    /* 2. apply segment base and address size */
#ifndef CONFIG_SOFTMMU
    if (ir1_opnd_has_seg(opnd1))
#else
    int seg_num = ir1_opnd_get_seg_index(opnd1);
    if (ir1_opnd_has_seg(opnd1) && seg_num >= 0 &&
        ((seg_num == ss_index ||
         seg_num == ds_index ||
         seg_num == es_index ) ? td->sys.addseg : 1))
#endif
    { /* 2.1 need segment base */
        IR2_OPND seg_base = ra_alloc_itemp();
        int seg_reg = ir1_opnd_get_seg_index(opnd1);
        append_ir2_opnd2i(LISA_LD_WU, &seg_base, &env_ir2_opnd,
                lsenv_offset_of_seg_base(lsenv, seg_reg));

        append_ir2_opnd3(LISA_ADD_D, &ea,
                ea_base_only ? &ea_base : &ea, &seg_base);
        ra_free_temp(&seg_base);

        append_ir2_opnd2_(lisa_mov32z, &ea, &ea);
    }
    else { /* 2.2 no segment base */
        IR2_OPND *__ea = ea_base_only ? &ea_base : &ea;

        switch (addr_size) {
        case 2: append_ir2_opnd2_(lisa_mov16z, &ea, __ea); break;
        case 4: append_ir2_opnd2_(lisa_mov32z, &ea, __ea); break;
        default:
            lsassertm(0, "unknown addr size %d in %s.\n",
                    addr_size, __func__);
            break;
        }
    }

    /* 3. construct IR2_OPND_MEM */
    int mem_base = ir2_opnd_reg(&ea);
    ir2_opnd_build_mem(opnd2, mem_base, ea_off);
}

/* Convert IR1_OPND_GPR to IR2_OPND_IREG
 * > The return IR2_OPDN_IREG might be temp or mapping !!!
 * > Convert according to IR1_OPND's attribute
 *   eg. AL/AH  8-bit to 64-bit
 *       AX    16-bit to 64-bit
 *       EAX   32-bit to 64-bit
 * > Extend according to EXTENSION_MODE (sign or unsign)
 *
 * @opnd1 : IR1_OPND_GPR
 * @em : sign, zero or unknown extension
 */
IR2_OPND convert_gpr_opnd(
        IR1_OPND *opnd1,
        EXTENSION_MODE em)
{
    IR2_OPND tmp = ra_alloc_itemp();
    load_ir1_gpr_to_ir2(&tmp, opnd1, em);
    return tmp;
//    lsassertm(0, "OPND: convert gpr to be implemented in LoongArch.\n");
//    lsassert(ir1_opnd_is_gpr(opnd1));
//    /*assert(em == UNKNOWN_EXTENSION || em==SIGN_EXTENSION || em ==
//     * ZERO_EXTENSION);*/
//    int gpr_num = ir1_opnd_base_reg_num(opnd1);
//    int opnd_size = ir1_opnd_size(opnd1);
//
//    IR2_OPND gpr_opnd = ra_alloc_gpr(gpr_num);
//
//    /* 1. 32 bits gpr needs SHIFT operation to handle the extension mode */
//    if (opnd_size == 32) {
//        /* 1.1. need sign-extended but gpr is not */
//        if (em == SIGN_EXTENSION && !ir2_opnd_is_sx(&gpr_opnd, 32)) {
//            IR2_OPND ret_opnd = ra_alloc_itemp_internal();
//            append_ir2_opnd2(mips_mov32_sx, &ret_opnd, &gpr_opnd);
//            return ret_opnd;
//        }
//        /* 1.2. need zero-extended but gpr is not */
//        else if (em == ZERO_EXTENSION && !ir2_opnd_is_zx(&gpr_opnd, 32)) {
//            IR2_OPND ret_opnd = ra_alloc_itemp_internal();
//            append_ir2_opnd2(mips_mov32_zx, &ret_opnd, &gpr_opnd);
//            return ret_opnd;
//        }
//        /* 1.3. gpr is what we need, or we need any extension, return gpr */
//        /* directly */
//        else {
//            return gpr_opnd;
//        }
//    }
//
//    /* 2. 64 bits gpr needs no extension */
//    else if (opnd_size == 64) {
//        return gpr_opnd;
//    }
//
//    /* 3. 16 bits gpr uses AND operation when zero extended */
//    else if (opnd_size == 16) {
//        IR2_OPND gpr_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
//        /* 3.1. need sign-extended but gpr is usually not */
//        if (em == SIGN_EXTENSION && !ir2_opnd_is_sx(&gpr_opnd, 16)) {
//            IR2_OPND ret_opnd = ra_alloc_itemp_internal();
//            append_ir2_opnd2i(mips_sll, &ret_opnd, &gpr_opnd, 16);
//            append_ir2_opnd2i(mips_sra, &ret_opnd, &ret_opnd, 16);
//            lsassert(ir2_opnd_is_sx(&ret_opnd, 16));
//            return ret_opnd;
//        }
//        /* 3.2. need zero-extended but gpr is usually not */
//        else if (em == ZERO_EXTENSION && !ir2_opnd_is_zx(&gpr_opnd, 16)) {
//            IR2_OPND ret_opnd = ra_alloc_itemp_internal();
//            append_ir2_opnd2i(mips_andi, &ret_opnd, &gpr_opnd, 0xffff);
//            lsassert(ir2_opnd_is_zx(&ret_opnd, 16));
//            return ret_opnd;
//        }
//        /* 3.3. need any extension, return gpr directly */
//        else {
//            return gpr_opnd;
//        }
//    }
//
//    /* 4. 8 bits gpr high */
//    else if (ir1_opnd_is_8h(opnd1)) {
//        IR2_OPND gpr_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
//        IR2_OPND ret_opnd = ra_alloc_itemp_internal();
//        /* 4.1. we need sign extension */
//        if (em == SIGN_EXTENSION) {
//            if (ir2_opnd_is_sx(&gpr_opnd, 16))
//                append_ir2_opnd2i(mips_sra, &ret_opnd, &gpr_opnd, 8);
//            else {
//                append_ir2_opnd2i(mips_sll, &ret_opnd, &gpr_opnd, 16);
//                append_ir2_opnd2i(mips_sra, &ret_opnd, &ret_opnd, 24);
//            }
//            return ret_opnd;
//        }
//        /* 4.2. we need zero extension */
//        else if (em == ZERO_EXTENSION) {
//            if (ir2_opnd_is_zx(&gpr_opnd, 16))
//                append_ir2_opnd2i(mips_sra, &ret_opnd, &gpr_opnd, 8);
//            else {
//                append_ir2_opnd2i(mips_andi, &ret_opnd, &gpr_opnd, 0xff00);
//                append_ir2_opnd2i(mips_srl, &ret_opnd, &ret_opnd, 8);
//            }
//            return ret_opnd;
//        }
//        /* 4.3. we need any extension */
//        else {
//            append_ir2_opnd2i(mips_dsrl, &ret_opnd, &gpr_opnd, 8);
//            return ret_opnd;
//        }
//    }
//
//    /* 5. 8 bits gpr low */
//    else {
//        IR2_OPND gpr_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
//        /* 5.1. need sign-extended but gpr is usually not */
//        if (em == SIGN_EXTENSION && !ir2_opnd_is_sx(&gpr_opnd, 8)) {
//            IR2_OPND ret_opnd = ra_alloc_itemp_internal();
//            append_ir2_opnd2i(mips_sll, &ret_opnd, &gpr_opnd, 24);
//            append_ir2_opnd2i(mips_sra, &ret_opnd, &ret_opnd, 24);
//            return ret_opnd;
//        }
//        /* 5.2. need zero-extended but gpr is usually not */
//        else if (em == ZERO_EXTENSION && !ir2_opnd_is_zx(&gpr_opnd, 8)) {
//            IR2_OPND ret_opnd = ra_alloc_itemp_internal();
//            append_ir2_opnd2i(mips_andi, &ret_opnd, &gpr_opnd, 0xff);
//            return ret_opnd;
//        }
//        /* 5.3. any extension */
//        else {
//            return gpr_opnd;
//        }
//    }
}

static
void load_imm64(IR2_OPND *opnd2, int64_t value)
{
    IR2_OPND *zero = &zero_ir2_opnd;
    if (value >> 11 == -1 || value >> 11 == 0) {
        append_ir2_opnd2i(LISA_ADDI_D, opnd2, zero, value);
    } else if (value >> 12 == 0) {
        append_ir2_opnd2i(LISA_ORI, opnd2, zero, value);
    } else if (value >> 31 == -1 || value >> 31 == 0) {
        append_ir2_opnd1i(LISA_LU12I_W, opnd2, value >> 12);
        if (value & 0xfff)
            append_ir2_opnd2i(LISA_ORI, opnd2, opnd2, value & 0xfff);
    } else if (value >> 32 == 0) {
        append_ir2_opnd1i(LISA_LU12I_W, opnd2, value >> 12);
        if (value & 0xfff)
            append_ir2_opnd2i(LISA_ORI, opnd2, opnd2, value & 0xfff);
        append_ir2_opnd1i(LISA_LU32I_D, opnd2, 0);
    } else if (value >> 51 == -1 || value >> 51 == 0) {
        if ((value & 0xffffffff) == 0)
            append_ir2_opnd3(LISA_OR, opnd2, zero, zero);
        else if ((value & 0xfff) == 0)
            append_ir2_opnd1i(LISA_LU12I_W, opnd2, value >> 12);
        else {
            append_ir2_opnd1i(LISA_LU12I_W, opnd2, value >> 12);
            append_ir2_opnd2i(LISA_ORI, opnd2, opnd2, value & 0xfff);
        }
        append_ir2_opnd1i(LISA_LU32I_D, opnd2, value >> 32);
    } else {
        if ((value & 0xfffffffffffff) == 0)
            append_ir2_opnd3(LISA_OR, opnd2, zero, zero);
        else if ((value & 0xffffffff) == 0) {
            append_ir2_opnd3(LISA_OR, opnd2, zero, zero);
            append_ir2_opnd1i(LISA_LU32I_D, opnd2, value >> 32);
        } else if ((value & 0xfff) == 0) {
            append_ir2_opnd1i(LISA_LU12I_W, opnd2, value >> 12);
        } else {
            append_ir2_opnd1i(LISA_LU12I_W, opnd2, value >> 12);
            append_ir2_opnd2i(LISA_ORI, opnd2, opnd2, value & 0xfff);
            append_ir2_opnd1i(LISA_LU32I_D, opnd2, value >> 32);
        }
        append_ir2_opnd2i(LISA_LU52I_D, opnd2, opnd2, (value >> 52) & 0xfff);
    }
}

void load_imm32_to_ir2(
        IR2_OPND *opnd2,
        uint32_t value,
        EXTENSION_MODE em)
{
    lsassertm(ir2_opnd_is_gpr(opnd2),
            "load imm 32 to ir2: IR2 OPND is not GPR\n");

    int16_t high_16_bits = value >> 16;
    int16_t low_16_bits = value;
    int64_t imm = 0;

    if (high_16_bits == 0) {
        /* 1. should be 0000 0000 0000 xxxx */
        imm = low_16_bits;
        load_imm64(opnd2, imm & 0xFFFF);
//        ir2_opnd_set_em(opnd2, ZERO_EXTENSION, 16);
        return;
    } else if (high_16_bits == -1) {
        if (em == ZERO_EXTENSION) {
            /* 2. should be 0000 0000 ffff xxxx */
            imm = value;
            imm = imm & 0xFFFFFFFF;
            lsassertm((imm & 0x00000000FFFF0000) == 0xFFFF0000,
                     "value=0x%x, imm = 0x%lx\n", value, imm);
            load_imm64(opnd2, imm);
//            ir2_opnd_set_em(opnd2, ZERO_EXTENSION, 32);
            return;
        } else {
            /* 3. should be ffff ffff ffff xxxx */
            if (low_16_bits < 0) {
                imm = low_16_bits;
                lsassertm(((imm & 0xFFFFFFFFFFFF0000) == 0xFFFFFFFFFFFF0000),
                        "value=0x%x, imm = 0x%lx\n", value, imm);
                load_imm64(opnd2, imm);
//                ir2_opnd_set_em(opnd2, SIGN_EXTENSION, 16);
                return;
            } else {
                imm = (int32_t)value;
                lsassertm(((imm & 0xFFFFFFFFFFFF0000) == 0xFFFFFFFFFFFF0000),
                        "value=0x%x, imm = 0x%lx\n", value, imm);
                load_imm64(opnd2, value);
//                ir2_opnd_set_em(opnd2, SIGN_EXTENSION, 17);
                return;
            }
        }
    } else {
        if (em == ZERO_EXTENSION) {
            /* 5. should be 0000 0000 xxxx xxxx */
            if (high_16_bits < 0) {
                imm = value;
                imm = imm & 0xFFFFFFFF;
                load_imm64(opnd2, imm);
//                ir2_opnd_set_em(opnd2, ZERO_EXTENSION, 32);
                return;
            } else {
                imm = value;
                imm = imm & 0xFFFFFFFF;
                load_imm64(opnd2, imm);
//                ir2_opnd_set_em(opnd2, ZERO_EXTENSION, 31);
                return;
            }
        } else {
            /* 6. should be ssss ssss xxxx xxxx */
            imm = (int32_t)value;
            load_imm64(opnd2, imm);
//            ir2_opnd_set_em(opnd2, SIGN_EXTENSION, 32);
            return;
        }
    }
}

void load_imm64_to_ir2(
        IR2_OPND *opnd2,
        uint64_t value)
{
    lsassertm(ir2_opnd_is_gpr(opnd2),
            "load imm 64 to ir2: IR2 OPND is not GPR\n");

    int32 high_32_bits = value >> 32;
    int32 low_32_bits = value;

    if (high_32_bits == 0) {
        /* 1. 0000 0000 xxxx xxxx */
        load_imm32_to_ir2(opnd2, low_32_bits, ZERO_EXTENSION);
        return;
    } else if (high_32_bits == -1) {
        /* 2. ffff ffff xxxx xxxx */
        if (low_32_bits < 0) {
            load_imm32_to_ir2(opnd2, low_32_bits, SIGN_EXTENSION);
            return;
        } else {
            load_imm32_to_ir2(opnd2, ~low_32_bits, ZERO_EXTENSION);
            append_ir2_opnd3(LISA_NOR, opnd2, opnd2, opnd2);
            return;
        }
    } else {
        /* 3. xxxx xxxx xxxx xxxx */
        load_imm64(opnd2,value);
        return;
    }
}

void load_addrx_to_ir2(
        IR2_OPND *opnd,
        ADDRX addrx)
{
    load_imm32_to_ir2(opnd, (uint32_t)addrx, ZERO_EXTENSION);
//    lsassertm(0, "OPND: load addrx to be implemented in LoongArch.\n");
//#ifdef N64 /* validate address */
//    load_imm32_to_ir2(opnd, addrx, ZERO_EXTENSION);
//    ir2_opnd_set_em(opnd, EM_X86_ADDRESS, 32);
//#else
//    load_imm32_to_ir2(&opnd, addrx, ZERO_EXTENSION);
//    lsassert(ir2_opnd_is_sx(opnd) && ir2_opnd_is_zx(opnd));
//#endif
}

void load_addr_to_ir2(
        IR2_OPND *opnd,
        ADDR addr)
{
    load_imm64_to_ir2(opnd, (uint64_t)addr);
//    lsassertm(0, "OPND: load addr to be implemented in LoongArch.\n");
//#ifdef N64 /* validate address */
//#if defined N64
//    load_imm64_to_ir2(opnd, addr);
//    ir2_opnd_set_em(opnd, EM_MIPS_ADDRESS, 32);
//#else
//    load_imm32_to_ir2(opnd, addr, ZERO_EXTENSION);
//    lsassert(ir2_opnd_is_sx(opnd) && ir2_opnd_is_zx(opnd));
//#endif
}

void load_ir1_imm_to_ir2(
        IR2_OPND *opnd2,
        IR1_OPND *opnd1,
        EXTENSION_MODE em)
{
    lsassert(ir2_opnd_is_gpr(opnd2));
    lsassert(ir1_opnd_is_imm(opnd1));
    if (em == ZERO_EXTENSION) {
        uint32_t value = ir1_opnd_uimm(opnd1);
        load_imm32_to_ir2(opnd2, value, ZERO_EXTENSION);
    } else {
        int32_t value = ir1_opnd_simm(opnd1);
        load_imm32_to_ir2(opnd2, value, SIGN_EXTENSION);
    }
}

void load_ir1_seg_to_ir2(
        IR2_OPND *opnd2,
        IR1_OPND *opnd1)
{
//    lsassertm(0, "OPND: load seg to be implemented in LoongArch.\n");
    lsassert(ir2_opnd_is_gpr(opnd2));
    lsassert(ir1_opnd_is_seg(opnd1));
    int seg_num = ir1_opnd_base_reg_num(opnd1);

#ifdef CONFIG_SOFTMMU
    TRANSLATION_DATA *td = lsenv->tr_data;
    IR1_INST *pir1 = td->curr_ir1_inst;
    if (ir1_opcode(pir1) == X86_INS_MOV) {
        CHECK_EXCP_MOV_FROM_SEG(pir1, seg_num);
    }
#endif

    append_ir2_opnd2i(LISA_LD_HU, opnd2, &env_ir2_opnd,
                      lsenv_offset_of_seg_selector(lsenv, seg_num));
}

/* Load data from the memory specified by IR1_OPND_MEM to IR2_OPND_IREG
 * > If you want to obtain the memory addrss, use conver_mem_opnd
 * > Softmmu helper might be called if QEMU TLB miss in system-mode
 */
void load_ir1_mem_to_ir2(
        IR2_OPND *opnd2,
        IR1_OPND *opnd1,
        EXTENSION_MODE em,
        bool is_xmm_hi,
        int addr_size)
{
//    lsassertm(0, "OPND: load mem to be implemented in LoongArch.\n");
    lsassert(ir2_opnd_is_gpr(opnd2));
    lsassert(ir1_opnd_is_mem(opnd1));

    IR2_OPND mem_opnd;

    if (addr_size < 0) {
        /* Use instruction's default address size */
        addr_size = ir1_addr_size(lsenv->tr_data->curr_ir1_inst);
    }

    if (is_xmm_hi) {
        convert_mem_opnd_with_bias(&mem_opnd, opnd1, 8, addr_size);
    } else {
        convert_mem_opnd(&mem_opnd, opnd1, addr_size);
    }

//    int base_ireg_num = ir2_opnd_base_reg_num(&mem_opnd);

//#ifndef CONFIG_SOFTMMU
//
//#ifdef N64
//    /* take mips_lw for example
//     *
//     * original : lw    opnd2.reg,	opnd1.imm(opnd1.reg)
//     *
//     * via qemu : ld    tmp,	guest_base
//     *     		  add   tmp,	tmp,	opnd1.reg
//     *     		  ld	opnd2.reg,	opnd1.imm(tmp)
//     *
//     * register mapping optimization : $14(t2) always stores guest_base
//     *
//     *            add   tmp,    opnd1.reg,   $14
//     *            ld    opnd2.reg, opnd1.imm(tmp)
//     */
//    /* no matter what, mem_opnd.reg stores the final address to access
//     * we need to add this reg with guest_base to get the real host address
//     * to access */
//    if (cpu_get_guest_base() != 0) {
//        IR2_OPND tmp = ra_alloc_itemp();
//        IR2_OPND gbase = ra_alloc_guest_base();
//        IR2_OPND mem = ir2_opnd_new(IR2_OPND_IREG, base_ireg_num);
//
//        if (!ir2_opnd_is_address(&mem)) {
//            append_ir2_opnd2(mips_mov_addrx, &mem, &mem);
//        }
//        append_ir2_opnd3(mips_add_addr, &tmp, &mem, &gbase);
//        ir2_opnd_mem_set_base_ireg(&mem_opnd, &tmp);
//        base_ireg_num = ir2_opnd_base_reg_num(&tmp);
//        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
//        ra_free_temp(&mem);
//    }
//#else
//    lsassertm(0, "not implement for o32/n32 load/store.\n");
//#endif
//
//#endif /* ifndef CONFIG_SOFTMMU */

    IR2_OPCODE mem_opcode = LISA_INVALID;

    bool is_zero = em == ZERO_EXTENSION;
    switch (ir1_opnd_size(opnd1)) {
    case 128:
    case 64: mem_opcode = LISA_LD_D; break;
    case 32: mem_opcode = is_zero ? LISA_LD_WU : LISA_LD_W; break;
    case 16: mem_opcode = is_zero ? LISA_LD_HU : LISA_LD_H; break;
    case 8:  mem_opcode = is_zero ? LISA_LD_BU : LISA_LD_B; break;
    default:
        lsassertm(0, "load ir1 mem to ir2 should not reach here.\n");
        break;
    }

#ifdef CONFIG_SOFTMMU
    gen_ldst_softmmu_helper(mem_opcode, opnd2, &mem_opnd, true);
#else
    append_ir2_opnd2(mem_opcode, opnd2, &mem_opnd);
#endif
    ra_free_temp(&mem_opnd);
}

/* Load the data in IR1_OPND_GPR to IR2_OPND_IREG
 * > Unlike convert_gpr_opnd, we put the data to the given IR2_OPND_IREG here
 *   > The given IR2_OPND_IREG should not be mapping register
 *   > Always use IR1_OPND to operate the mapping registers
 * > Convert according to IR1_OPND's attribute
 *   eg. AL/AH  8-bit to 64-bit
 *       AX    16-bit to 64-bit
 *       EAX   32-bit to 64-bit
 * > Extend according to EXTENSION_MODE (sign or unsign)
 */
void load_ir1_gpr_to_ir2(
        IR2_OPND *opnd2,
        IR1_OPND *opnd1,
        EXTENSION_MODE em)
{
//    lsassertm(0, "OPND: load ir1 gpr to be implemented in LoongArch.\n");
    lsassert(ir2_opnd_is_gpr(opnd2));
    lsassert(ir1_opnd_is_gpr(opnd1));
//    lsassert(em == UNKNOWN_EXTENSION || em == SIGN_EXTENSION ||
//             em == ZERO_EXTENSION);

    int gpr_num = ir1_opnd_base_reg_num(opnd1);
    IR2_OPND gpr_opnd = ra_alloc_gpr(gpr_num);

    /* 1. 32 bits gpr needs SHIFT operation to handle the extension mode */
    if (ir1_opnd_size(opnd1) == 32) {
        if (em == SIGN_EXTENSION) {
            append_ir2_opnd2ii(LISA_BSTRPICK_W, opnd2, &gpr_opnd, 31, 0);
        } else {
            append_ir2_opnd2ii(LISA_BSTRPICK_D, opnd2, &gpr_opnd, 31, 0);
        }
    }

    /* 2. 64 bits gpr needs no extension */
    else if (ir1_opnd_size(opnd1) == 64) {
        append_ir2_opnd3(LISA_OR, opnd2, &gpr_opnd, &zero_ir2_opnd);
    }

    /* 3. 16 bits gpr */
    else if (ir1_opnd_size(opnd1) == 16) {
        if (em == SIGN_EXTENSION) {
            append_ir2_opnd2(LISA_EXT_W_H, opnd2, &gpr_opnd);
        } else {
            append_ir2_opnd2ii(LISA_BSTRPICK_D, opnd2, &gpr_opnd, 15, 0);
        }
    }

    /* 4. 8 bits gpr high */
    else if (ir1_opnd_is_8h(opnd1)) {
        if (em == SIGN_EXTENSION) {
            append_ir2_opnd2i(LISA_SRLI_D, opnd2, &gpr_opnd, 8);
            append_ir2_opnd2(LISA_EXT_W_B, opnd2, opnd2);
        } else {
            append_ir2_opnd2ii(LISA_BSTRPICK_D, opnd2, &gpr_opnd, 15, 8);
        }
    }

    /* 5. 8 bits gpr low */
    else {
        if (em == SIGN_EXTENSION) {
            append_ir2_opnd2(LISA_EXT_W_B, opnd2, &gpr_opnd);
        } else {
            append_ir2_opnd2ii(LISA_BSTRPICK_D, opnd2, &gpr_opnd, 7, 0);
        }
    }
}

void load_ir1_mmx_to_ir2(
        IR2_OPND *opnd2,
        IR1_OPND *opnd1,
        EXTENSION_MODE em)
{
    lsassertm(0, "OPND: load ir1 mmx to be implemented in LoongArch.\n");
//    IR2_OPND mmx_opnd = ra_alloc_mmx(ir1_opnd_base_reg_num(opnd1));
//
//    if (em == SIGN_EXTENSION) {
//        append_ir2_opnd2(mips_mfc1, opnd2, &mmx_opnd);
//    } else if (em == ZERO_EXTENSION) {
//        append_ir2_opnd2(mips_dmfc1, opnd2, &mmx_opnd);
//        append_ir2_opnd2(mips_mov32_zx, opnd2, opnd2);
//    } else {
//        append_ir2_opnd2(mips_dmfc1, opnd2, &mmx_opnd);
//    }
}

void load_ir1_xmm_to_ir2(
        IR2_OPND *opnd2,
        IR1_OPND *opnd1,
        EXTENSION_MODE em,
        bool is_xmm_hi)
{
    lsassertm(0, "OPND: load ir1 xmm to be implemented in LoongArch.\n");
//    IR2_OPND xmm_opnd;
//
//    if (is_xmm_hi) {
//        xmm_opnd = ra_alloc_xmm_hi(ir1_opnd_base_reg_num(opnd1));
//    } else {
//        xmm_opnd = ra_alloc_xmm_lo(ir1_opnd_base_reg_num(opnd1));
//    }
//
//    // copy from 64bits part of xmm to 64bits ireg, no need to do extension
//    append_ir2_opnd2(mips_dmfc1, opnd2, &xmm_opnd);
//    // if (em == SIGN_EXTENSION) {
//    //     append_ir2_opnd2(mips_mfc1, opnd2, xmm_opnd);
//    // } else if (em == ZERO_EXTENSION) {
//    //     append_ir2_opnd2(mips_dmfc1, opnd2, xmm_opnd);
//    //     append_ir2_opnd2(mips_mov32_zx, opnd2, opnd2);
//    // } else {
//    //     append_ir2_opnd2(mips_dmfc1, opnd2, xmm_opnd);
//    // }
}

#ifdef CONFIG_SOFTMMU

/* This function should be used ONLY in 'mov from control register'
 * Previous chechking guarantees that opnd1 is control register */
void load_ir1_cr_to_ir2(
        IR2_OPND *opnd2,
        IR1_OPND *opnd1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    IR1_INST *pir1 = td->curr_ir1_inst;
    CHECK_EXCP_MOV_CR(pir1);

    int flag = 0;
    int crn = ir1_opnd_get_cr_num(opnd1, &flag);
    if (!flag) {
        tr_gen_excp_illegal_op(pir1, 1);
        return;
    }

    lsassert(ir2_opnd_is_gpr(opnd2));

    /* 0. save current instruciton's EIP to env */
    tr_gen_save_curr_eip();

    /* 1. call helper_read_crN */
    tr_sys_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* target/i386/misc_helper.c
     *
     * target_ulong helper_read_crN(CPUX86State *env, int reg)
     *
     * arg0 : CPUX86State *env
     * arg1 : control register number */

    /* arg0 : env */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);

    /* arg1 : crn */
    append_ir2_opnd2i(LISA_ORI, &arg1_ir2_opnd, &zero_ir2_opnd, crn);

    tr_gen_call_to_helper((ADDR)helper_read_crN);

    tr_sys_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* move $v0 to the correct destination */
    append_ir2_opnd3(LISA_OR, opnd2, &ret0_ir2_opnd, &zero_ir2_opnd);
}

/* This function should be used ONLY in 'mov to control register'
 * Previous chechking guarantees that opnd1 is control register */
void store_ir2_to_ir1_cr(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    IR1_INST *pir1 = td->curr_ir1_inst;
    CHECK_EXCP_MOV_CR(pir1);

    int flag = 0;
    int crn = ir1_opnd_get_cr_num(opnd1, &flag);
    if (!flag) {
        tr_gen_excp_illegal_op(pir1, 1);
        return;
    }

    lsassert(ir2_opnd_is_gpr(opnd2));

    /* 0. save current instruciton's EIP to env */
    tr_gen_save_curr_eip();

    /* 1. call helper_write_crN */
    tr_sys_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* target/i386/misc_helper.c
     *
     * void helper_write_crN(CPUX86State *env, int reg, target_ulong t0)
     *
     * arg0 : CPUX86State *env
     * arg1 : control register number
     * arg2 : data to store */

    /* 0. arg3: data to store
     * opnd2 might be temp regsiter, so do this fitst */
    append_ir2_opnd3(LISA_OR, &arg2_ir2_opnd, opnd2, &zero_ir2_opnd);

    /* 1. arg1: env */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);

    /* 2. arg2: control register number */
    append_ir2_opnd2i(LISA_ORI, &arg1_ir2_opnd, &zero_ir2_opnd, crn);

    tr_gen_call_to_helper((ADDR)helper_write_crN);

    tr_sys_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
}

/* This function should be used ONLY in 'mov from debug register'
 * Previous chechking guarantees that opnd1 is debug register */
void load_ir1_dr_to_ir2(
        IR2_OPND *opnd2,
        IR1_OPND *opnd1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    IR1_INST *pir1 = td->curr_ir1_inst;
    CHECK_EXCP_MOV_DR(pir1);

    lsassert(ir2_opnd_is_gpr(opnd2));

    /* 1. save context */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    /* 2. call helper_get_dr
     *
     * target/i386/bpt_helper.c
     * target_ulong helper_get_dr(
     *      CPUX86State *env,
     *      int         reg)
     * >> might generate exception
     */

    /* 2.1 arg0 : env */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
    /* 2.2 arg1 : drn */
    int drn = ir1_opnd_get_dr_num(opnd1);
    append_ir2_opnd2i(LISA_ORI, &arg1_ir2_opnd, &zero_ir2_opnd, drn);
    /* 2.3 call helper */
    tr_gen_call_to_helper((ADDR)helper_get_dr);

    /* 3. restore context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);

    /* 4. move $v0 to the correct destination */
    append_ir2_opnd3(LISA_OR, opnd2, &ret0_ir2_opnd, &zero_ir2_opnd);
}

/* This function should be used ONLY in 'mov to debug register'
 * Previous chechking guarantees that opnd1 is debug register */
void store_ir2_to_ir1_dr(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    IR1_INST *pir1 = td->curr_ir1_inst;
    CHECK_EXCP_MOV_DR(pir1);

    lsassert(ir2_opnd_is_gpr(opnd2));

    /* 1. save context */
    helper_cfg_t cfg = default_helper_cfg;
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    /* 2. call helper_set_dr
     *
     * target/i386/bpt_helper.c
     * void helper_set_dr(
     *      CPUX86State *env,
     *      int reg,
     *      target_ulong t0)
     * >> might generate exception
     * */

    /* 2.1. arg2: data to store
     * opnd2 might be temp regsiter, so do this fitst */
    append_ir2_opnd3(LISA_OR, &arg2_ir2_opnd, opnd2, &zero_ir2_opnd);
    /* 2.2. arg0: env */
    append_ir2_opnd3(LISA_OR, &arg0_ir2_opnd, &env_ir2_opnd, &zero_ir2_opnd);
    /* 2.3 arg1: debug register number */
    int drn = ir1_opnd_get_dr_num(opnd1);
    append_ir2_opnd2i(LISA_ORI, &arg1_ir2_opnd, &zero_ir2_opnd, drn);
    /* 2.4 call helper */
    tr_gen_call_to_helper((ADDR)helper_set_dr);

    /* 3. restore context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);
}

#endif

/**
@load the value of an ir1 operand to an ir2 register operand
@param the ir1 operand (any type is ok)
@param the extension mode
@return the ir2 register operand. it can be temp register or a mapped register
*/
void load_ir1_to_ir2_ptr(
        IR2_OPND *opnd2,
        IR1_OPND *opnd1,
        EXTENSION_MODE em,
        bool is_xmm_hi)
{
    lsassert(em == SIGN_EXTENSION || em == ZERO_EXTENSION ||
             em == UNKNOWN_EXTENSION);

    IR2_OPND ret_opnd;

    switch (ir1_opnd_type(opnd1)) {
    case X86_OP_IMM: {
        ret_opnd = ra_alloc_itemp_internal();
        load_ir1_imm_to_ir2(&ret_opnd, opnd1, em);
        break;
    }
    case X86_OP_REG: {
        if(ir1_opnd_is_gpr(opnd1)){
            ret_opnd = convert_gpr_opnd(opnd1, em);
            break;
        }
        else if(ir1_opnd_is_seg(opnd1)){
            ret_opnd = ra_alloc_itemp_internal();
            load_ir1_seg_to_ir2(&ret_opnd, opnd1);
            break;
        }
#ifdef CONFIG_SOFTMMU
        else if(ir1_opnd_is_cr(opnd1)) {
            ret_opnd = ra_alloc_itemp_internal();
            load_ir1_cr_to_ir2(&ret_opnd, opnd1);
            break;
        }
        else if(ir1_opnd_is_dr(opnd1)) {
            ret_opnd = ra_alloc_itemp_internal();
            load_ir1_dr_to_ir2(&ret_opnd, opnd1);
            break;
        }
#endif
        else if(ir1_opnd_is_mmx(opnd1)){
            ret_opnd = ra_alloc_itemp_internal();
            load_ir1_mmx_to_ir2(&ret_opnd, opnd1, em);
            break;
        }
        else if(ir1_opnd_is_xmm(opnd1)){
            ret_opnd = ra_alloc_itemp_internal();
            load_ir1_xmm_to_ir2(&ret_opnd, opnd1, em, is_xmm_hi);
            break;
        }
        lsassert(0);
    }
    case X86_OP_MEM: {
        ret_opnd = ra_alloc_itemp_internal();
        load_ir1_mem_to_ir2(&ret_opnd, opnd1, em, is_xmm_hi, -1);
        break;
    }
    default:
        lsassert(0);
        break;
    }

    *opnd2 = ret_opnd;
}

/* Load the value from IR1_OPND to IR2_OPND
 *
 * @opnd2: IR2_OPND_IREG
 * @opnd1: chould be - immediate
 *                   - GPR
 *                   - Segment selector
 *                   - CR, DR
 *                   - MMX, XMM
 *                   - Memory
 * @em: sign, zero or unknown
 *      Used only for imm, gpr, mem, mmx, xmm
 */
void load_ir1_to_ir2(
        IR2_OPND *opnd2,
        IR1_OPND *opnd1,
        EXTENSION_MODE em,
        bool is_xmm_hi)
{
    lsassert(ir2_opnd_is_gpr(opnd2));

    switch (ir1_opnd_type(opnd1)) {
    case X86_OP_IMM:
        load_ir1_imm_to_ir2(opnd2, opnd1, em);
        break;
    case X86_OP_REG:  {
        if(ir1_opnd_is_gpr(opnd1)){
            load_ir1_gpr_to_ir2(opnd2, opnd1, em);
            break;
        }
        else if(ir1_opnd_is_seg(opnd1)){
            load_ir1_seg_to_ir2(opnd2, opnd1);
            break;
        }
#ifdef CONFIG_SOFTMMU
        else if(ir1_opnd_is_cr(opnd1)) {
            load_ir1_cr_to_ir2(opnd2, opnd1);
            break;
        }
        else if(ir1_opnd_is_dr(opnd1)) {
            load_ir1_dr_to_ir2(opnd2, opnd1);
            break;
        }
#endif
        else if(ir1_opnd_is_mmx(opnd1)){
            load_ir1_mmx_to_ir2(opnd2, opnd1, em);
            break;
        }
        else if(ir1_opnd_is_xmm(opnd1)){
            load_ir1_xmm_to_ir2(opnd2, opnd1, em, is_xmm_hi);
            break;
        }
        lsassert(0);
    }
    case X86_OP_MEM:  {
        load_ir1_mem_to_ir2(opnd2, opnd1, em, is_xmm_hi, -1);
        break;
    }
    default:
        lsassert(0);
    }

    return;
}

void load_eflags_cf_to_ir2(IR2_OPND *opnd2)
{
    lsassert(ir2_opnd_is_gpr(opnd2));

    if (option_lbt) {
        append_ir2_opnd1i(LISA_X86MFFLAG, opnd2, 0x1);
    } else {
        append_ir2_opnd2i(LISA_ANDI, opnd2, &eflags_ir2_opnd, 1);
    }

    return;
}

void store_ir2_to_ir1_gpr(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
//    lsassertm(0, "store to ir1 gpr to be implemented in LoongArch.\n");
//    lsassert(ir2_opnd_is_gpr(opnd2));
    lsassert(ir1_opnd_is_gpr(opnd1) && ir2_opnd_is_gpr(opnd2));
    int gpr_num = ir1_opnd_base_reg_num(opnd1);
    IR2_OPND gpr_opnd = ra_alloc_gpr(gpr_num);

    /* 1. 32 bits gpr needs SHIFT operation to handle the extension mode */
    if (ir1_opnd_size(opnd1) == 32) {
        if (!ir2_opnd_cmp(opnd2, &gpr_opnd)) {
            append_ir2_opnd3(LISA_OR, &gpr_opnd, opnd2, &zero_ir2_opnd);
//#ifdef N64 /* validate address */
//            if (ir2_opnd_is_ax(opnd2, 32))
//                append_ir2_opnd2(mips_mov32_sx, &gpr_opnd, opnd2);
//            else
//                append_ir2_opnd2(mips_mov64, &gpr_opnd, opnd2);
//#else
//            append_ir2_opnd2(mips_mov32_sx, &gpr_opnd, opnd2);
//#endif
        }
    }
    else if (ir1_opnd_size(opnd1) == 64) {
        if (!ir2_opnd_cmp(opnd2, &gpr_opnd))
            append_ir2_opnd3(LISA_OR, &gpr_opnd, opnd2, &zero_ir2_opnd);
    }
    else if (ir1_opnd_size(opnd1) == 16) {
        append_ir2_opnd2ii(LISA_BSTRINS_D, &gpr_opnd, opnd2, 15, 0);
    }
    else if (ir1_opnd_is_8h(opnd1)) {
        append_ir2_opnd2ii(LISA_BSTRINS_D, &gpr_opnd, opnd2, 15, 8);
    }
    else {
        append_ir2_opnd2ii(LISA_BSTRINS_D, &gpr_opnd, opnd2,  7, 0);
    }
}

void store_ir2_to_ir1_mem(
        IR2_OPND *value_opnd,
        IR1_OPND *opnd1,
        bool is_xmm_hi,
        int addr_size)
{
//    lsassertm(0, "store to ir1 mem to be implemented in LoongArch.\n");
    lsassert(ir2_opnd_is_gpr(value_opnd));

    IR2_OPND mem_opnd;

    if (addr_size < 0)
        addr_size = ir1_addr_size(lsenv->tr_data->curr_ir1_inst);

    if (is_xmm_hi) {
        convert_mem_opnd_with_bias(&mem_opnd, opnd1, 8, addr_size);
    } else {
        convert_mem_opnd(&mem_opnd, opnd1, addr_size);
    }

//    int base_ireg_num = ir2_opnd_base_reg_num(&mem_opnd);
//
//#ifndef CONFIG_SOFTMMU
//
//#ifdef N64
//    if (cpu_get_guest_base() != 0) {
//        IR2_OPND tmp = ra_alloc_itemp();
//        IR2_OPND gbase = ra_alloc_guest_base();
//        IR2_OPND mem = ir2_opnd_new(IR2_OPND_IREG, base_ireg_num);
//
//        if (!ir2_opnd_is_address(&mem)) {
//            append_ir2_opnd2(mips_mov_addrx, &mem, &mem);
//        }
//        append_ir2_opnd3(mips_add_addr, &tmp, &mem, &gbase);
//        /* reset mem_opnd.reg as tmp */
//        ir2_opnd_mem_set_base_ireg(&mem_opnd, &tmp);
//        base_ireg_num = ir2_opnd_base_reg_num(&mem_opnd);
//        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
//        ra_free_temp(&mem);
//    }
//#else
//    lsassertm(0, "not implement for o32/n32 load/store.\n");
//#endif
//
//#endif /* ifndef CONFIG_SOFTMMU */

    IR2_OPCODE mem_opcode = LISA_INVALID;

    switch (ir1_opnd_size(opnd1)) {
    case 128:
    case 64: mem_opcode = LISA_ST_D; break;
    case 32: mem_opcode = LISA_ST_W; break;
    case 16: mem_opcode = LISA_ST_H; break;
    case 8:  mem_opcode = LISA_ST_B; break;
    default:
        lsassertm(0, "store ir2 to ir1 mem should not reach here.\n");
        break;
    }

#ifdef CONFIG_SOFTMMU
    gen_ldst_softmmu_helper(mem_opcode, value_opnd, &mem_opnd, true);
#else
    append_ir2_opnd2(mem_opcode, value_opnd, &mem_opnd);
#endif
    ra_free_temp(&mem_opnd);

    return;
}

void store_ir2_to_ir1_seg(IR2_OPND *seg_value_opnd, IR1_OPND *opnd1)
{
    lsassert(ir2_opnd_is_gpr(seg_value_opnd));

#ifdef CONFIG_SOFTMMU /* sys-mode mov to seg */

    int seg_num = ir1_opnd_base_reg_num(opnd1);

    TRANSLATION_DATA *td = lsenv->tr_data;
    helper_cfg_t cfg = default_helper_cfg;

    IR1_INST *pir1 = td->curr_ir1_inst;
    if (ir1_opcode(pir1) == X86_INS_MOV) {
        CHECK_EXCP_MOV_TO_SEG(pir1, seg_num);
    }

    /* protected mode */
    if (td->sys.pe && !td->sys.vm86) {
        goto _NORMAL_MODE_;
    }

    /* real-address mode || vm86 mode */
    /* 0.1 selector = 16-bits */
    IR2_OPND _selector_reg = ra_alloc_itemp();
    append_ir2_opnd2_(lisa_mov16z, &_selector_reg, seg_value_opnd);
    append_ir2_opnd2i(LISA_ST_W, &_selector_reg, &env_ir2_opnd,
                      lsenv_offset_of_seg_selector(lsenv, seg_num));
    ra_free_temp(&_selector_reg);

    /* 0.2 base = 20-bits = selector << 4 */
    IR2_OPND _base_reg = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_SLLI_D, &_base_reg, &_selector_reg, 0x4);
    append_ir2_opnd2i(LISA_ST_W, &_base_reg, &env_ir2_opnd,
                      lsenv_offset_of_seg_base(lsenv, seg_num));
    ra_free_temp(&_base_reg);

    goto _GEN_EOB_;

_NORMAL_MODE_:
    /* seg_num = ir1_opnd_base_reg_num(opnd1);
     *
     *      x86tomips          QEMU/target/i386
     *                           typedef enum X86Seg {
     *  #define es_index 0           R_ES = 0,
     *  #define cs_index 1           R_CS = 1,
     *  #define ss_index 2           R_SS = 2,
     *  #define ds_index 3           R_DS = 3,
     *  #define fs_index 4           R_FS = 4,
     *  #define gs_index 5           R_GS = 5,
     *                               R_LDTR = 6
     *                               R_TR = 7, } X86Seg; */
    lsassert(seg_num != cs_index);

    /* 0. save native context */
    tr_sys_gen_call_to_helper_prologue_cfg(cfg);

    /* target/i386/seg_helper.c
     * void helper_load_seg(
     *      CPUX86State *env,
     *      int seg_reg,
     *      int selector) */
    /* arg2: selector = seg_value_opnd */
    IR2_OPND seg_tmp = ra_alloc_itemp();
    append_ir2_opnd2_(lisa_mov16z, &seg_tmp, seg_value_opnd);
//    append_ir2_opnd2i(mips_andi, &seg_tmp, seg_value_opnd, 0xffff);
    append_ir2_opnd2_(lisa_mov, &arg2_ir2_opnd, &seg_tmp);
    ra_free_temp(&seg_tmp);
    /* arg0: env*/
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    /* arg1: segment register numeber */
    append_ir2_opnd2i(LISA_ORI, &arg1_ir2_opnd, &zero_ir2_opnd, seg_num); // CS is checked in tr_excp.c
    /* call to helper */
    tr_gen_call_to_helper((ADDR)helper_load_seg);

    /* 8. restore native context */
    tr_sys_gen_call_to_helper_epilogue_cfg(cfg);

_GEN_EOB_:

    if (seg_num == ss_index) {
        td->recheck_tf  = 0;
        td->inhibit_irq = 1;
    }

    return;

#else /* user-mode mov to seg */
    lsassertm(0, "mov to seg no implemented in user-mode LATX\n");
//
//
//#ifdef N64
//    /* 1. set selector */
//    append_ir2_opnd2i(mips_sw, seg_value_opnd, &env_ir2_opnd,
//                      lsenv_offset_of_seg_selector(lsenv, seg_num));
//
//    /* 2. update seg cache : read data in GDT and store into seg cache */
//
//    /* 2.1 get gdt base */
//    IR2_OPND gdt_opnd = ra_alloc_itemp_internal();
//    append_ir2_opnd2i(mips_lw, &gdt_opnd, &env_ir2_opnd,
//                      lsenv_offset_of_gdt_base(lsenv));
//    append_ir2_opnd3(mips_and, &gdt_opnd, &gdt_opnd, &n1_ir2_opnd);
//
//    /* 2.2 get entry offset of gdt and add it on gdt-base */
//    IR2_OPND offset_in_gdt_opnd = ra_alloc_itemp_internal();
//    append_ir2_opnd2i(mips_andi, &offset_in_gdt_opnd, seg_value_opnd, 0xfff8);
//    append_ir2_opnd3(mips_daddu, &gdt_opnd, &gdt_opnd, &offset_in_gdt_opnd);
//
//    if (cpu_get_guest_base() != 0) {
//        /* 2.3 add guest-base */
//        IR2_OPND guest_base = ra_alloc_guest_base();
//        if (!ir2_opnd_is_address(&gdt_opnd)) {
//            append_ir2_opnd2(mips_mov_addrx, &gdt_opnd, &gdt_opnd);
//        }
//        append_ir2_opnd3(mips_add_addr, &gdt_opnd, &gdt_opnd, &guest_base);
//        ir2_opnd_set_em(&gdt_opnd, EM_MIPS_ADDRESS, 32);
//    }
//
//    /* 2.4 read segment entry */
//    IR2_OPND gdt_entry = ra_alloc_itemp_internal();
//    append_ir2_opnd2i(mips_ld, &gdt_entry, &gdt_opnd, 0);
//
//    IR2_OPND seg_limit = ra_alloc_itemp_internal(); /* [51:48] [15: 0] limit */
//    IR2_OPND seg_base = ra_alloc_itemp_internal();  /* [63:56] [39:16] base */
//    IR2_OPND seg_flags = ra_alloc_itemp_internal(); /* [55:40] flags */
//
//    /* 2.5 get new base */
//    IR2_OPND tmp = ra_alloc_itemp_internal();
//    append_ir2_opnd2i(mips_dsra, &seg_base, &gdt_entry, 16);
//    append_ir2_opnd2i(mips_andi, &seg_base, &seg_base, 0xffff); /* [31:16] */
//    append_ir2_opnd2i(mips_dsra32, &tmp, &gdt_entry, 0);
//    append_ir2_opnd2i(mips_andi, &tmp, &tmp, 0xff);
//    append_ir2_opnd2i(mips_sll, &tmp, &tmp, 16);
//    append_ir2_opnd3(mips_or, &seg_base, &seg_base, &tmp); /* [39:32] */
//    append_ir2_opnd2i(mips_dsra32, &tmp, &gdt_entry, 24);
//    append_ir2_opnd2i(mips_andi, &tmp, &tmp, 0xff);
//    append_ir2_opnd2i(mips_sll, &tmp, &tmp, 24);
//    append_ir2_opnd3(mips_or, &seg_base, &seg_base, &tmp); /* [63:56] */
//
//    /* 2.5 get new limit */
//    append_ir2_opnd2i(mips_andi, &seg_limit, &gdt_entry, 0xffff); /* [15: 0] */
//    append_ir2_opnd2i(mips_dsra32, &tmp, &gdt_entry, 16);
//    append_ir2_opnd2i(mips_andi, &tmp, &tmp, 0xf);
//    append_ir2_opnd2i(mips_sll, &tmp, &tmp, 16);
//    append_ir2_opnd3(mips_or, &seg_limit, &seg_limit, &tmp); /* [35:32] */
//    append_ir2_opnd2i(mips_sll, &seg_limit, &seg_limit, 12);
//    append_ir2_opnd2i(mips_ori, &seg_limit, &seg_limit, 0xfff);
//
//    /* 2.6 get flags in GDT */
//    append_ir2_opnd2i(mips_dsra32, &tmp, &gdt_entry, 8);
//    append_ir2_opnd2i(mips_andi, &seg_flags, &tmp, 0xffff);
//    append_ir2_opnd2i(mips_ori, &seg_flags, &seg_flags,
//                      1); /* TypeField in GDT whichrepresent accessed, */
//                          /* TypeField should be written to GDT,we ommit it */
//    append_ir2_opnd2i(mips_sll, &seg_flags, &seg_flags, 8);
//
//    /* 2.7 write into seg cache */
//    append_ir2_opnd2i(mips_sdi, &seg_base, &env_ir2_opnd,
//                      lsenv_offset_of_seg_base(lsenv, seg_num));
//    append_ir2_opnd2i(mips_sw, &seg_limit, &env_ir2_opnd,
//                      lsenv_offset_of_seg_limit(lsenv, seg_num));
//    append_ir2_opnd2i(mips_sw, &seg_flags, &env_ir2_opnd,
//                      lsenv_offset_of_seg_flags(lsenv, seg_num));
//
//#else
//    lsassertm(0, "not implement for MIPS o32/n32.\n");
//#endif
//
#endif
}

/* Store the value from IR2_OPND to IR1_OPND
 *
 * @opnd2: IR2_OPND_IREG
 * @opnd1: chould be - GPR
 *                   - Segment selector
 *                   - CR, DR
 *                   - MMX, XMM
 *                   - Memory
 */
void store_ir2_to_ir1(
        IR2_OPND *opnd2,
        IR1_OPND *opnd1,
        bool is_xmm_hi)
{
    lsassert(ir2_opnd_is_gpr(opnd2));

    switch (ir1_opnd_type(opnd1)) {
    case X86_OP_REG:  {
        if(ir1_opnd_is_gpr(opnd1)) {
            store_ir2_to_ir1_gpr(opnd2, opnd1);
            break;
        }
        else if (ir1_opnd_is_seg(opnd1)) {
            store_ir2_to_ir1_seg(opnd2, opnd1);
            break;
        }
#ifdef CONFIG_SOFTMMU
        else if (ir1_opnd_is_cr(opnd1)) {
            store_ir2_to_ir1_cr(opnd2, opnd1);
            break;
        }
        else if (ir1_opnd_is_dr(opnd1)) {
            store_ir2_to_ir1_dr(opnd2, opnd1);
            break;
        }
#endif
        else if(ir1_opnd_is_mmx(opnd1)) {
            IR2_OPND mmx_opnd = ra_alloc_mmx(ir1_opnd_base_reg_num(opnd1));
            append_ir2_opnd2(LISA_MOVGR2FR_D, &mmx_opnd, opnd2);
//            append_ir2_opnd2(mips_dmtc1, opnd2, &mmx_opnd);
            break;
        }
        else if(ir1_opnd_is_xmm(opnd1)) {
            IR2_OPND mmx_opnd;

            if (is_xmm_hi)
                mmx_opnd = ra_alloc_xmm_hi(ir1_opnd_base_reg_num(opnd1));
            else
                mmx_opnd = ra_alloc_xmm_lo(ir1_opnd_base_reg_num(opnd1));

            append_ir2_opnd2(LISA_MOVGR2FR_D, &mmx_opnd, opnd2);
//            append_ir2_opnd2(mips_dmtc1, opnd2, &mmx_opnd);

            break;
        }
        lsassert(0);
    }

    case X86_OP_MEM:
        store_ir2_to_ir1_mem(opnd2, opnd1, is_xmm_hi, -1);
        break;
    default:
        lsassert(0);
    }

    return;
}

/**
    load 80bit float from memory and convert to 64bit
*/
static void load_64_bit_freg_from_ir1_80_bit_mem(IR2_OPND *opnd2,
                                                 IR2_OPND *mem_opnd)
{
    /* 1. load sign and exponent */
    IR2_OPND ir2_sign_exp = ra_alloc_itemp();
    IR2_OPND ir2_fraction = ra_alloc_itemp();

    int mem_base = ir2_opnd_reg(mem_opnd);
    int mem_off  = ir2_opnd_offset(mem_opnd);
    IR2_OPND mem_sign_exp;
    ir2_opnd_build_mem(&mem_sign_exp, mem_base, mem_off + 8);

#ifndef CONFIG_SOFTMMU
    append_ir2_opnd2(LISA_LD_HU, &ir2_sign_exp, &mem_sign_exp);
    append_ir2_opnd2(LISA_LD_D,  &ir2_fraction,  mem_opnd);
#else
    gen_ldst_softmmu_helper(LISA_LD_HU, &ir2_sign_exp, &mem_sign_exp, 1);
    gen_ldst_softmmu_helper(LISA_LD_D,  &ir2_fraction,  mem_opnd,     1);
#endif

    IR2_OPND f_ir2_sign_exp = ra_alloc_ftemp();
    IR2_OPND f_ir2_fraction = ra_alloc_ftemp();
    append_ir2_opnd2(LISA_MOVGR2FR_W, &f_ir2_sign_exp, &ir2_sign_exp);
    append_ir2_opnd2(LISA_MOVGR2FR_D, &f_ir2_fraction, &ir2_fraction);
    ra_free_temp(&ir2_sign_exp);
    ra_free_temp(&ir2_fraction);

    append_ir2_opnd3(LISA_FCVT_D_LD, opnd2, &f_ir2_fraction, &f_ir2_sign_exp);
    ra_free_temp(&f_ir2_sign_exp);
    ra_free_temp(&f_ir2_fraction);

    IR2_OPND flag_san = ra_alloc_itemp();
    IR2_OPND label_exit = ir2_opnd_new_label();
    append_ir2_opnd2  (LISA_MOVFCSR2GR, &flag_san, &fcsr_ir2_opnd);
    append_ir2_opnd2ii(LISA_BSTRPICK_D, &flag_san, &flag_san, 28, 28);

    append_ir2_opnd3(LISA_BEQ, &flag_san, &zero_ir2_opnd, &label_exit);
    append_ir2_opnd2  (LISA_MOVFR2GR_D, &flag_san, opnd2);
    append_ir2_opnd2ii(LISA_BSTRINS_D,  &flag_san, &zero_ir2_opnd, 51, 51);
    append_ir2_opnd2  (LISA_MOVGR2FR_D, opnd2,     &flag_san);
    ra_free_temp(&flag_san);

    append_ir2_opnd1(LISA_LABEL, &label_exit);
}

/**
@load an ir1 memory operand to a specific ir2 register operand. internal temp
registers may be used.
@param the ir1 memory operand
@return the ir2 register operand
*/
static void load_freg_from_ir1_mem(IR2_OPND *opnd2, IR1_OPND *opnd1,
                                   bool is_xmm_hi, bool is_convert)
{
    IR2_OPND mem_opnd;
    int addr_size = ir1_addr_size(lsenv->tr_data->curr_ir1_inst);
    if (is_xmm_hi) {
        convert_mem_opnd_with_bias(&mem_opnd, opnd1, 8, addr_size);
    } else {
        convert_mem_opnd(&mem_opnd, opnd1, addr_size);
    }

#ifndef CONFIG_SOFTMMU
    int mem_base = ir2_opnd_reg(&mem_opnd);
    IR2_OPND mem_opnd_ireg;

    if (cpu_get_guest_base() != 0) {
        IR2_OPND tmp = ir2_opnd_new(IR2_OPND_GPR, mem_base);
        IR2_OPND gbase = ra_alloc_guest_base();

        mem_opnd_ireg = ra_alloc_itemp();
        append_ir2_opnd3(LISA_ADD_D, &mem_opnd_ireg, &tmp, &gbase);
        ir2_opnd_mem_set_base(&mem_opnd, &mem_opnd_ireg);
    } else {
        mem_opnd_ireg = ir2_opnd_new(IR2_OPND_GPR, mem_base);
    }
#endif

    if (ir1_opnd_size(opnd1) == 32) {
#ifndef CONFIG_SOFTMMU
        append_ir2_opnd2i(LISA_FLD_S, opnd2, &mem_opnd_ireg,
                          ir2_opnd_imm(&mem_opnd));
#else
        gen_ldst_c1_softmmu_helper(LISA_FLD_S, opnd2, &mem_opnd, 1);
#endif
        if (is_convert)
            append_ir2_opnd2(LISA_FCVT_D_S, opnd2, opnd2);
    } else if (ir1_opnd_size(opnd1) == 64 || ir1_opnd_size(opnd1) == 128) {
#ifndef CONFIG_SOFTMMU
        append_ir2_opnd2i(LISA_FLD_D, opnd2, &mem_opnd_ireg,
                          ir2_opnd_imm(&mem_opnd));
#else
        gen_ldst_c1_softmmu_helper(LISA_FLD_D, opnd2, &mem_opnd, 1);
#endif
    } else if (ir1_opnd_size(opnd1) == 16) {
        IR2_OPND itemp = ra_alloc_itemp_internal();
#ifndef CONFIG_SOFTMMU
        append_ir2_opnd2i(LISA_LD_H, &itemp, &mem_opnd_ireg,
                          ir2_opnd_imm(&mem_opnd));
#else
        gen_ldst_softmmu_helper(LISA_LD_H, &itemp, &mem_opnd, 1);
#endif
        append_ir2_opnd2(LISA_MOVGR2FR_D, opnd2, &itemp);
        ra_free_temp(&itemp);

        lsassertm(!is_convert, "convert 16-bit floating point?\n");
    } else if (ir1_opnd_size(opnd1) == 8) {
        IR2_OPND itemp = ra_alloc_itemp_internal();
#ifndef CONFIG_SOFTMMU
        append_ir2_opnd2i(LISA_LD_B, &itemp, &mem_opnd_ireg,
                          ir2_opnd_imm(&mem_opnd));
#else
        gen_ldst_softmmu_helper(LISA_LD_B, &itemp, &mem_opnd, 1);
#endif
        append_ir2_opnd2(LISA_MOVGR2FR_D, opnd2, &itemp);
        ra_free_temp(&itemp);

        lsassertm(!is_convert, "convert 8-bit floating point?\n");
    } else {
        load_64_bit_freg_from_ir1_80_bit_mem(opnd2, &mem_opnd);
#ifndef CONFIG_SOFTMMU
        if (cpu_get_guest_base() != 0)
            ra_free_temp(&mem_opnd_ireg);
#endif
    }

    ra_free_temp(&mem_opnd);
}

/**
@load an ir1 register operand to a specific ir2 register operand.
@param the ir1 register operand
@return the ir2 register operand. it can be temp register or a mapped register
*/

static void load_freg_from_ir1_fpr(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    lsassert(ir2_opnd_is_fpr(opnd2));
    IR2_OPND value_opnd = ra_alloc_st(ir1_opnd_base_reg_num(opnd1));
    if (!ir2_opnd_cmp(opnd2, &value_opnd))
        append_ir2_opnd2(LISA_FMOV_D, opnd2, &value_opnd);
}

static void load_freg_from_ir1_mmx(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    lsassert(ir2_opnd_is_fpr(opnd2));
    lsassert(ir1_opnd_is_mmx(opnd1));
    IR2_OPND mmx_opnd = ra_alloc_mmx(ir1_opnd_base_reg_num(opnd1));
    if (!ir2_opnd_cmp(opnd2, &mmx_opnd))
        append_ir2_opnd2(LISA_FMOV_D, opnd2, &mmx_opnd);
}

static void load_freg_from_ir1_xmm(IR2_OPND *opnd2, IR1_OPND *opnd1,
                                   bool is_xmm_hi)
{
    lsassert(ir2_opnd_is_fpr(opnd2));
    lsassert(ir1_opnd_is_xmm(opnd1));
    IR2_OPND xmm_opnd;
    if (is_xmm_hi)
        xmm_opnd = ra_alloc_xmm_hi(ir1_opnd_base_reg_num(opnd1));
    else
        xmm_opnd = ra_alloc_xmm_lo(ir1_opnd_base_reg_num(opnd1));
    if (!ir2_opnd_cmp(opnd2, &xmm_opnd))
        append_ir2_opnd2(LISA_FMOV_D, opnd2, &xmm_opnd);
}

IR2_OPND load_freg_from_ir1_1(IR1_OPND *opnd1, bool is_xmm_hi, bool is_convert)
{
    switch (ir1_opnd_type(opnd1)) {
    case X86_OP_REG: {
        if (ir1_opnd_is_fpr(opnd1)){
            return ra_alloc_st(ir1_opnd_base_reg_num(opnd1));

        }
        else if(ir1_opnd_is_mmx(opnd1)){
            return ra_alloc_mmx(ir1_opnd_base_reg_num(opnd1));

        }
        else if(ir1_opnd_is_xmm(opnd1)){
            if (is_xmm_hi)
                return ra_alloc_xmm_hi(ir1_opnd_base_reg_num(opnd1));
            else
                return ra_alloc_xmm_lo(ir1_opnd_base_reg_num(opnd1));
        }
        else if(ir1_opnd_is_ymm(opnd1)){
            lsassert(0);
        }
        lsassert(0);

    }
    case X86_OP_MEM: {
        IR2_OPND ret_opnd = ra_alloc_ftemp_internal();
        load_freg_from_ir1_mem(&ret_opnd, opnd1, is_xmm_hi, is_convert);
        return ret_opnd;
    }
    default:
        lsassert(0);
    }
}

void load_freg_from_ir1_2(IR2_OPND *opnd2, IR1_OPND *opnd1, bool is_xmm_hi,
                          bool is_convert)
{
    lsassert(ir2_opnd_is_fpr(opnd2));

    switch (ir1_opnd_type(opnd1)) {
    case X86_OP_REG: {
        if (ir1_opnd_is_fpr(opnd1)){
            load_freg_from_ir1_fpr(opnd2, opnd1);
            return;
        }
        else if(ir1_opnd_is_mmx(opnd1)){
            load_freg_from_ir1_mmx(opnd2, opnd1);
            return;
        }
        else if(ir1_opnd_is_xmm(opnd1)){
            load_freg_from_ir1_xmm(opnd2, opnd1, is_xmm_hi);
            return;
        }
        else if(ir1_opnd_is_ymm(opnd1)){
            lsassert(0);
        }
        lsassertm(0,"REG:%s\n", ir1_reg_name(opnd1->reg));
    }

    case X86_OP_MEM: {
        load_freg_from_ir1_mem(opnd2, opnd1, is_xmm_hi, is_convert);
        break;
    }
    default:
        lsassert(0);
        break;
    }

    return;
}

void load_singles_from_ir1_pack(IR2_OPND *single0, IR2_OPND *single1,
                                IR1_OPND *opnd1, bool is_xmm_hi)
{
    lsassertm(0, "load single from ir1 pack to be implemented in LoongArch.\n");
//    switch (ir1_opnd_type(opnd1)) {
//    // case IR1_OPND_XMM: {
//    case X86_OP_REG: {
//        if(ir1_opnd_is_xmm(opnd1)){
//            IR2_OPND half_xmm;
//            if (is_xmm_hi)
//                half_xmm = ra_alloc_xmm_hi(ir1_opnd_base_reg_num(opnd1));
//            else
//                half_xmm = ra_alloc_xmm_lo(ir1_opnd_base_reg_num(opnd1));
//
//            /* retrieve the 1th single from half_xmm */
//            IR2_OPND itemp0 = ra_alloc_itemp();
//            append_ir2_opnd2(mips_dmfc1, &itemp0, &half_xmm);
//            IR2_OPND itemp1 = ra_alloc_itemp();
//            append_ir2_opnd2i(mips_dsrl32, &itemp1, &itemp0, 0);
//            append_ir2_opnd2(mips_dmtc1, &itemp1, single1);
//            ra_free_temp(&itemp1);
//            /* retrive the 0th single from src xmm operand */
//            append_ir2_opnd2i(mips_dsll32, &itemp0, &itemp0, 0);
//            append_ir2_opnd2i(mips_dsrl32, &itemp0, &itemp0, 0);
//            append_ir2_opnd2(mips_dmtc1, &itemp0, single0);
//            ra_free_temp(&itemp0);
//        }
//        break;
//    }
//    case X86_OP_MEM:  {
//        /* retrieve the 1th single */
//        IR2_OPND itemp0 = ra_alloc_itemp();
//        load_ir1_mem_to_ir2(&itemp0, opnd1, ZERO_EXTENSION, is_xmm_hi, -1);
//        IR2_OPND itemp1 = ra_alloc_itemp();
//        append_ir2_opnd2i(mips_dsrl32, &itemp1, &itemp0, 0);
//        append_ir2_opnd2(mips_dmtc1, &itemp1, single1);
//        ra_free_temp(&itemp1);
//        /* retrive the 0th single */
//        append_ir2_opnd2i(mips_dsll32, &itemp0, &itemp0, 0);
//        append_ir2_opnd2i(mips_dsrl32, &itemp0, &itemp0, 0);
//        append_ir2_opnd2(mips_dmtc1, &itemp0, single0);
//        ra_free_temp(&itemp0);
//        break;
//    }
//    default:
//        lsassert(0);
//        break;
//    }
//    return;
}

void store_singles_to_ir2_pack(IR2_OPND *single0, IR2_OPND *single1,
                               IR2_OPND *pack)
{
    lsassert(ir2_opnd_is_fpr(single0) &&
             ir2_opnd_is_fpr(single1) &&
             ir2_opnd_is_fpr(pack));

    lsassertm(0, "store single to ir2 to be implemented in LoongArch.\n");
//    IR2_OPND itemp1 = ra_alloc_itemp();
//    append_ir2_opnd2(mips_dmfc1, &itemp1, single1);
//    append_ir2_opnd2i(mips_dsll32, &itemp1, &itemp1, 0);
//
//    IR2_OPND itemp0 = ra_alloc_itemp();
//    append_ir2_opnd2(mips_dmfc1, &itemp0, single0);
//    append_ir2_opnd3(mips_or, &itemp1, &itemp1, &itemp0);
//    append_ir2_opnd2(mips_dmtc1, &itemp1, pack);
    return;
}

void store_64_bit_freg_to_ir1_80_bit_mem(IR2_OPND *opnd2, IR2_OPND *mem_opnd)
{
    IR2_OPND f_ir2_sign_exp = ra_alloc_ftemp();
    IR2_OPND f_ir2_fraction = ra_alloc_ftemp();

    IR2_OPND flag_san = ra_alloc_itemp();
    IR2_OPND ir2_sign_exp = ra_alloc_itemp();
    IR2_OPND ir2_fraction = ra_alloc_itemp();
    IR2_OPND label_ok = ir2_opnd_new_label();

    append_ir2_opnd2(LISA_FCVT_LD_D, &f_ir2_fraction, opnd2);
    append_ir2_opnd2(LISA_FCVT_UD_D, &f_ir2_sign_exp, opnd2);

    append_ir2_opnd2  (LISA_MOVFCSR2GR, &flag_san, &fcsr_ir2_opnd);
    append_ir2_opnd2ii(LISA_BSTRPICK_D, &flag_san, &flag_san, 28, 28);

    append_ir2_opnd2(LISA_MOVFR2GR_S, &ir2_sign_exp, &f_ir2_sign_exp);
    append_ir2_opnd2(LISA_MOVFR2GR_D, &ir2_fraction, &f_ir2_fraction);
    ra_free_temp(&f_ir2_sign_exp);
    ra_free_temp(&f_ir2_fraction);

    /* Identify SNAN and write snan to opnd2 */
    append_ir2_opnd3(LISA_BEQ, &flag_san, &zero_ir2_opnd, &label_ok);
    append_ir2_opnd2ii(LISA_BSTRINS_D, &ir2_fraction, &zero_ir2_opnd, 62, 62);
    append_ir2_opnd1(LISA_LABEL, &label_ok);
    ra_free_temp(&flag_san);

    int mem_base = ir2_opnd_reg(mem_opnd);
    int mem_off  = ir2_opnd_offset(mem_opnd);
    IR2_OPND mem_sign_exp;
    ir2_opnd_build_mem(&mem_sign_exp, mem_base, mem_off + 8);

#ifndef CONFIG_SOFTMMU
    append_ir2_opnd2(LISA_ST_H, &ir2_sign_exp, &mem_sign_exp);
    append_ir2_opnd2(LISA_ST_D, &ir2_fraction,  mem_opnd);
#else
    gen_ldst_softmmu_helper(LISA_ST_H, &ir2_sign_exp, &mem_sign_exp, 1);
    gen_ldst_softmmu_helper(LISA_ST_D, &ir2_fraction,  mem_opnd,     1);
#endif

    ra_free_temp(&ir2_sign_exp);
    ra_free_temp(&ir2_fraction);
}

static void store_freg_to_ir1_mem(IR2_OPND *opnd2, IR1_OPND *opnd1,
                                  bool is_xmm_hi, bool is_convert)
{
    IR2_OPND mem_opnd;
    int addr_size = ir1_addr_size(lsenv->tr_data->curr_ir1_inst);
    if (is_xmm_hi) {
        convert_mem_opnd_with_bias(&mem_opnd, opnd1, 8, addr_size);
    } else {
        convert_mem_opnd(&mem_opnd, opnd1, addr_size);
    }

#ifndef CONFIG_SOFTMMU
    int mem_base = ir2_opnd_reg(&mem_opnd);
    IR2_OPND mem_opnd_ireg;

    if (cpu_get_guest_base() != 0) {
        IR2_OPND tmp = ir2_opnd_new(IR2_OPND_GPR, mem_base);
        IR2_OPND gbase = ra_alloc_guest_base();

        mem_opnd_ireg = ra_alloc_itemp();
        append_ir2_opnd3(LISA_ADD_D, &mem_opnd_ireg, &tmp, &gbase);
        ir2_opnd_mem_set_base(&mem_opnd, &mem_opnd_ireg);
    } else {
        mem_opnd_ireg = ir2_opnd_new(IR2_OPND_GPR, mem_base);
    }
#else
    int save_temp = 1;
#endif


    if (ir1_opnd_size(opnd1) == 32) {
        IR2_OPND ftemp = ra_alloc_ftemp_internal();
        if (is_convert) {
            append_ir2_opnd2(LISA_FCVT_S_D, &ftemp, opnd2);
#ifndef CONFIG_SOFTMMU
            append_ir2_opnd2i(LISA_FST_S, &ftemp, &mem_opnd_ireg,
                              ir2_opnd_imm(&mem_opnd));
#else
            gen_ldst_c1_softmmu_helper(LISA_FST_S, &ftemp, &mem_opnd, save_temp);
#endif
        } else
#ifndef CONFIG_SOFTMMU
            append_ir2_opnd2i(LISA_FST_S, opnd2, &mem_opnd_ireg,
                              ir2_opnd_imm(&mem_opnd));
#else
            gen_ldst_c1_softmmu_helper(LISA_FST_S, opnd2, &mem_opnd, save_temp);
#endif
        ra_free_temp(&ftemp);
    } else if (ir1_opnd_size(opnd1) == 64 || ir1_opnd_size(opnd1) == 128) {
#ifndef CONFIG_SOFTMMU
        append_ir2_opnd2i(LISA_FST_D, opnd2, &mem_opnd_ireg,
                          ir2_opnd_imm(&mem_opnd));
#else
        gen_ldst_c1_softmmu_helper(LISA_FST_D, opnd2, &mem_opnd, save_temp);
#endif
    } else {
        store_64_bit_freg_to_ir1_80_bit_mem(opnd2, &mem_opnd);
    }

    ra_free_temp(&mem_opnd);
}

static void store_freg_to_ir1_fpr(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    lsassert(ir2_opnd_is_fpr(opnd2));
    IR2_OPND target_opnd = ra_alloc_st(ir1_opnd_base_reg_num(opnd1));
    if (!ir2_opnd_cmp(opnd2, &target_opnd))
        append_ir2_opnd2(LISA_FMOV_D, &target_opnd, opnd2);
}

/**
@store an ir1 mmx operand
@param the ir2 fp register operand
@param the ir1 mmx operand
*/
static void store_freg_to_ir1_mmx(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    lsassert(ir2_opnd_is_fpr(opnd2));
    lsassert(ir1_opnd_is_mmx(opnd1));

    // IR2_OPND mmx_opnd = ra_alloc_mmx(opnd1->_reg_num);
    IR2_OPND mmx_opnd = ra_alloc_mmx(ir1_opnd_base_reg_num(opnd1));


    if (ir2_opnd_cmp(opnd2, &mmx_opnd))
        append_ir2_opnd2(LISA_FMOV_D, &mmx_opnd, opnd2);
}

static void store_freg_to_ir1_xmm(IR2_OPND *opnd2, IR1_OPND *opnd1,
                                  bool is_xmm_hi)
{
    lsassert(ir2_opnd_is_fpr(opnd2));
    lsassert(ir1_opnd_is_xmm(opnd1));

    IR2_OPND xmm_opnd;
    if (is_xmm_hi)
        xmm_opnd = ra_alloc_xmm_hi(ir1_opnd_base_reg_num(opnd1));
    else
        xmm_opnd = ra_alloc_xmm_lo(ir1_opnd_base_reg_num(opnd1));
    if (!ir2_opnd_cmp(opnd2, &xmm_opnd))
        append_ir2_opnd2(LISA_FMOV_D, &xmm_opnd, opnd2);
}

void store_freg_to_ir1(IR2_OPND *opnd2, IR1_OPND *opnd1, bool is_xmm_hi,
                       bool is_convert)
{
    lsassert(ir2_opnd_is_fpr(opnd2));

    switch (ir1_opnd_type(opnd1)) {

    case X86_OP_REG:  {
        if(ir1_opnd_is_fpr(opnd1)){
            store_freg_to_ir1_fpr(opnd2, opnd1);
            return;
        }
        else if(ir1_opnd_is_mmx(opnd1)){
            store_freg_to_ir1_mmx(opnd2, opnd1);
            return;
        }
        else if(ir1_opnd_is_xmm(opnd1)){
            store_freg_to_ir1_xmm(opnd2, opnd1, is_xmm_hi);
            return;
        }
        else if(ir1_opnd_is_ymm(opnd1)){
            lsassert(0);
        }
        lsassertm(0, "REG:%s\n", ir1_reg_name(opnd1->reg));
    }
    case X86_OP_MEM: {
        store_freg_to_ir1_mem(opnd2, opnd1, is_xmm_hi, is_convert);
        return;
    }
    default:
        lsassert(0);
        return;
    }
}

IR2_OPND convert_mem_opnd_with_bias_within_imm_bits(IR1_OPND *opnd1, int bias, int bits)
{
    lsassertm(0, "convert mem with bias in imm to be implemented in LoongArch.\n");
//    IR2_OPND mem_opnd;
//
//    longx offset = ir1_opnd_simm(opnd1) + bias;
//    int32_t offset_imm_part;
//    longx offset_reg_part;
//    IR2_OPND offset_reg_opnd = ra_alloc_itemp();
//    if (offset >= -(1 << (bits - 1)) &&
//        offset <= ((1 << (bits - 1)) - 1)) { /* minus 7 for the sake of mda process */
//        offset_imm_part = offset;
//        offset_reg_part = 0;
//    } else {
//        offset_imm_part = 0;
//        offset_reg_part = offset;
//        load_imm32_to_ir2(&offset_reg_opnd, offset_reg_part, SIGN_EXTENSION);
//    }
//
//    // if (offset >= -32768 &&
//    //     offset <= 32767 - 7) { /* minus 7 for the sake of mda process */
//    //     offset_imm_part = offset;
//    //     offset_reg_part = 0;
//    // } else {
//    //     offset_imm_part = offset;
//    //     offset_reg_part = offset - offset_imm_part;
//    //     lsassert(((offset_reg_part >> 16) << 16) ==
//    //              offset_reg_part); /* make sure that the low 16 bits are */
//    //                                /* 0//make sure that the low 16 bits are 0 */
//    //     if (offset_imm_part > 32767 - 7) {
//    //         offset_imm_part = 0;
//    //         offset_reg_part = offset;
//    //     }
//    //     load_imm32_to_ir2(&offset_reg_opnd, offset_reg_part, SIGN_EXTENSION);
//    // }
//
//    /* 1. has no index */
//    if (!ir1_opnd_has_index(opnd1)) { /* no index and scale */
//        if (ir1_opnd_has_base(opnd1)) {
//            /* 1.1. base and an optional offset */
//            IR2_OPND base_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
//            if (offset_reg_part == 0) {
//                ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
//                                ir2_opnd_base_reg_num(&base_opnd),
//                                offset_imm_part);
//            } else {
//                append_ir2_opnd3(mips_add_addrx, &offset_reg_opnd,
//                                 &offset_reg_opnd, &base_opnd);
//                ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
//                                ir2_opnd_base_reg_num(&offset_reg_opnd),
//                                offset_imm_part);
//            }
//        } else {
//            /* 1.2. only an offset */
//
//            if (offset_reg_part == 0) {
//                ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM, 0, offset_imm_part);
//            } else {
//                ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
//                                ir2_opnd_base_reg_num(&offset_reg_opnd),
//                                offset_imm_part);
//            }
//        }
//    }
//
//    /* 2. base, index, and an optional offset */
//    else if (ir1_opnd_has_base(opnd1)) {
//        IR2_OPND base_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
//        IR2_OPND index_opnd = ra_alloc_gpr(ir1_opnd_index_reg_num(opnd1));
//        mem_opnd = ra_alloc_itemp_internal();
//
//        /* 2.1. prepare base + index*scale */
//        if (ir1_opnd_scale(opnd1) == 1) {
//            append_ir2_opnd3(mips_add_addrx, &mem_opnd, &base_opnd, &index_opnd);
//        } else if (ir1_opnd_scale(opnd1) == 2) {
//            append_ir2_opnd2i(mips_sll, &mem_opnd, &index_opnd, 1);
//            append_ir2_opnd3(mips_add_addrx, &mem_opnd, &mem_opnd, &base_opnd);
//        } else if (ir1_opnd_scale(opnd1) == 4) {
//            append_ir2_opnd2i(mips_sll, &mem_opnd, &index_opnd, 2);
//            append_ir2_opnd3(mips_add_addrx, &mem_opnd, &mem_opnd, &base_opnd);
//        } else if (ir1_opnd_scale(opnd1) == 8) {
//            append_ir2_opnd2i(mips_sll, &mem_opnd, &index_opnd, 3);
//            append_ir2_opnd3(mips_add_addrx, &mem_opnd, &mem_opnd, &base_opnd);
//        } else {
//            lsassert(0);
//        }
//
//        /* 2.2. add offset */
//        if (offset_reg_part == 0) {
//            ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
//                            ir2_opnd_base_reg_num(&mem_opnd), offset_imm_part);
//        } else {
//            append_ir2_opnd3(mips_add_addrx, &offset_reg_opnd, &offset_reg_opnd,
//                             &mem_opnd);
//            ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
//                            ir2_opnd_base_reg_num(&offset_reg_opnd),
//                            offset_imm_part);
//        }
//    }
//
//    /* 3. index and an optional offset */
//    else {
//        IR2_OPND index_opnd = ra_alloc_gpr(ir1_opnd_index_reg_num(opnd1));
//        mem_opnd = ra_alloc_itemp_internal();
//
//        /* 2.1. prepare index*scale */
//        if (ir1_opnd_scale(opnd1) == 1) {
//            append_ir2_opnd2i(mips_sll, &mem_opnd, &index_opnd, 0);
//        } else if (ir1_opnd_scale(opnd1) == 2) {
//            append_ir2_opnd2i(mips_sll, &mem_opnd, &index_opnd, 1);
//        } else if (ir1_opnd_scale(opnd1) == 4) {
//            append_ir2_opnd2i(mips_sll, &mem_opnd, &index_opnd, 2);
//        } else if (ir1_opnd_scale(opnd1) == 8) {
//            append_ir2_opnd2i(mips_sll, &mem_opnd, &index_opnd, 3);
//        } else {
//            lsassert(0);
//        }
//
//        /* 2.2. add offset */
//        if (offset_reg_part == 0) {
//            ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
//                            ir2_opnd_base_reg_num(&mem_opnd), offset_imm_part);
//        } else {
//            append_ir2_opnd3(mips_add_addrx, &offset_reg_opnd, &mem_opnd,
//                             &offset_reg_opnd);
//            ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
//                            ir2_opnd_base_reg_num(&offset_reg_opnd),
//                            offset_imm_part);
//        }
//    }
//
//    /* 4. segment? */
//    if (ir1_opnd_has_seg(opnd1)) {
//        // lsassertm(0, "not implemented in %s\n", __FUNCTION__);
//        IR2_OPND seg_base_opnd = ra_alloc_itemp_internal();
//        append_ir2_opnd2i(mips_load_addrx, &seg_base_opnd, &env_ir2_opnd,
//                          lsenv_offset_of_seg_base(
//                              lsenv, ir1_opnd_get_seg_index(opnd1)));
//        int old_base_reg_num = ir2_opnd_base_reg_num(&mem_opnd);
//
//        // if (old_base_reg_num != 0)
//        //     append_ir2_opnd3(
//        //         mips_add_addrx, seg_base_opnd, seg_base_opnd,
//        //         ir2_opnd_new(IR2_OPND_IREG, old_base_reg_num)); /* TODO */
//
//        IR2_OPND old_base = ir2_opnd_new(IR2_OPND_IREG, old_base_reg_num);
//        ir2_opnd_set_em(&old_base, SIGN_EXTENSION, 32);
//        if (old_base_reg_num != 0)
//            append_ir2_opnd3(mips_add_addrx, &seg_base_opnd, &seg_base_opnd, &old_base);
//
//        ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
//                        ir2_opnd_base_reg_num(&seg_base_opnd),
//                        ir2_opnd_imm(&mem_opnd));
//    }
//    ra_free_temp(&offset_reg_opnd);
//    return mem_opnd;
}

void load_freg128_from_ir1_mem(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    lsassert(ir1_opnd_is_mem(opnd1));
    lsassert(ir2_opnd_is_fpr(opnd2));

    IR2_OPND mem_opnd;
    convert_mem_opnd(&mem_opnd, opnd1, -1);
#ifdef CONFIG_SOFTMMU
    int new_tmp = 0;
    IR2_OPND mem_no_offset = convert_mem_ir2_opnd_no_offset(&mem_opnd, &new_tmp);
    IR2_OPND mem_base = ir2_opnd_mem_get_base(&mem_no_offset);
    if (new_tmp) ra_free_temp(&mem_opnd);

    static uint64_t freg128_space[4];
    IR2_OPND data_addr = ra_alloc_itemp();
    load_imm64_to_ir2(&data_addr, (uint64_t)&freg128_space[0]);

    IR2_OPND data64 = ra_alloc_itemp();
    gen_ldst_softmmu_helper(LISA_LD_D, &data64, &mem_no_offset, 1);
    append_ir2_opnd2i(LISA_ST_D, &data64, &data_addr, 0);
    append_ir2_opnd2i(LISA_ADDI_D, &mem_base, &mem_base, 8);

    gen_ldst_softmmu_helper(LISA_LD_D, &data64, &mem_no_offset, 1);
    append_ir2_opnd2i(LISA_ST_D, &data64, &data_addr, 8);

    append_ir2_opnd2i(LISA_VLD, opnd2, &data_addr, 0);
#else
    IR2_OPND mem_base = ir2_opnd_mem_get_base(&mem_opnd);
    IR2_OPND mem_off  = ra_alloc_itemp();

    int disp = ir2_opnd_imm(&mem_opnd);
    load_imm64_to_ir2(&mem_off, disp);

    append_ir2_opnd3(LISA_ADD_D, &mem_base, &mem_base, &mem_off);
    append_ir2_opnd2i(LISA_VLD, opnd2, &mem_opnd, 0);
#endif
}

void store_freg128_to_ir1_mem(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    lsassert(ir1_opnd_is_mem(opnd1));
    lsassert(ir2_opnd_is_fpr(opnd2));

    IR2_OPND mem_opnd;
    convert_mem_opnd(&mem_opnd, opnd1, -1);
#ifdef CONFIG_SOFTMMU
    int new_tmp = 0;
    IR2_OPND mem_no_offset = convert_mem_ir2_opnd_no_offset(&mem_opnd, &new_tmp);
    IR2_OPND mem_base = ir2_opnd_mem_get_base(&mem_no_offset);
    if (new_tmp) ra_free_temp(&mem_opnd);

    static uint64_t freg128_space[4];
    IR2_OPND data_addr = ra_alloc_itemp();
    load_imm64_to_ir2(&data_addr, (uint64_t)&freg128_space[0]);

    append_ir2_opnd2i(LISA_VST, opnd2, &data_addr, 0);

    IR2_OPND data64 = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_LD_D, &data64, &data_addr, 0);
    gen_ldst_softmmu_helper(LISA_ST_D, &data64, &mem_no_offset, 1);
    append_ir2_opnd2i(LISA_ADDI_D, &mem_base, &mem_base, 8);

    append_ir2_opnd2i(LISA_LD_D, &data64, &data_addr, 8);
    gen_ldst_softmmu_helper(LISA_ST_D, &data64, &mem_no_offset, 1);
#else
    IR2_OPND mem_base = ir2_opnd_mem_get_base(&mem_opnd);
    IR2_OPND mem_off  = ra_alloc_itemp();

    int disp = ir2_opnd_imm(&mem_opnd);
    load_imm64_to_ir2(&mem_off, disp);

    append_ir2_opnd3(LISA_ADD_D, &mem_base, &mem_base, &mem_off);
    append_ir2_opnd2i(LISA_VST, opnd2, &mem_opnd, 0);
#endif
}

IR2_OPND load_freg128_from_ir1(IR1_OPND *opnd1)
{
    lsassert(ir1_opnd_is_xmm(opnd1) || ir1_opnd_is_mem(opnd1));
    if (ir1_opnd_is_xmm(opnd1)){
        return ra_alloc_xmm(ir1_opnd_base_reg_num(opnd1));
    }
    else if (ir1_opnd_is_mem(opnd1)){
        IR2_OPND ret_opnd = ra_alloc_ftemp();
        if (ir1_opnd_size(opnd1) == 128) {
            load_freg128_from_ir1_mem(&ret_opnd, opnd1);
        }
        else {
            load_freg_from_ir1_mem(&ret_opnd, opnd1, false, false);
        }
        return ret_opnd;
    }
    else {
        lsassert(0);
    }

}

void reverse_ir2_ireg_opnd_endian(IR2_OPND *opnd, int size_in_bit)
{
    lsassertm(0, "reverse ir2 endian to be implemented in LoongArch.\n");
//    lsassert(ir2_opnd_is_ireg(opnd));
//    switch(size_in_bit)
//    {
//        case 8:
//            return;
//        case 16:
//            append_ir2_opnd2(mips_wsbh, opnd, opnd);
//            break;
//        case 32:
//            append_ir2_opnd2(mips_dsbh, opnd, opnd);
//            append_ir2_opnd2(mips_dshd, opnd, opnd);
//            append_ir2_opnd2i(mips_dsrl32, opnd, opnd, 0);
//            break;
//        case 64:
//            append_ir2_opnd2(mips_dsbh, opnd, opnd);
//            append_ir2_opnd2(mips_dshd, opnd, opnd);
//            break;
//        default:
//            lsassert(0);
//    }
}

#ifdef CONFIG_SOFTMMU

static IR2_OPND convert_mem_ir2_opnd_plus_offset(
        IR2_OPND *mem,
        int addend)
{
    lsassert(ir2_opnd_is_mem(mem));
    IR2_OPND mem_opnd = *mem;

    int mem_offset = ir2_opnd_imm(mem);

    /* addend = 2 : 0x7fe
     * addend = 4 : 0x7fc */
    int mem_offset_high = ((int)0x800) - addend;

    /* situation 1: >> directly adjust mem_offset in IR2_OPND */
    /* situation 2: >> directly adjust base register in IR2_OPND */
    /* situation 3: >> use a new temp register as new base */
    if (likely(mem_offset < mem_offset_high)) {
        /* situation 1 */
        ir2_opnd_mem_adjust_offset(&mem_opnd, addend);
    } else {
        IR2_OPND mem_old_base = ir2_opnd_mem_get_base(mem);
        if (ir2_opnd_is_itemp(&mem_old_base)) {
            /* situation 2 */
            append_ir2_opnd2i(LISA_ADDI_D, &mem_old_base,
                    &mem_old_base, addend);
        } else {
            /* situation 3 */
            IR2_OPND mem_new_base = ra_alloc_itemp();
            append_ir2_opnd2i(LISA_ADDI_D, &mem_new_base,
                    &mem_old_base, addend);
            ir2_opnd_mem_set_base(&mem_opnd, &mem_new_base);
        }

    }

    return mem_opnd;
}

IR2_OPND convert_mem_ir2_opnd_plus_2(IR2_OPND *mem)
{
    return convert_mem_ir2_opnd_plus_offset(mem, 2);
}
IR2_OPND convert_mem_ir2_opnd_plus_4(IR2_OPND *mem)
{
    return convert_mem_ir2_opnd_plus_offset(mem, 4);
}

/*
 * @mem                        return mem_no_offset
 * --------------------       ------------------------
 *    GPR   |  offset     =>       GPR     |  offset
 * ---------+----------       -------------+----------
 *  mapping |  no                new temp  |  no
 *  mapping |  yes               new temp  |  no
 *   temp   |  no                old temp  |  no
 *   temp   |  yes               new temp  |  no
 * ---------+----------       -------------+----------
 */
IR2_OPND convert_mem_ir2_opnd_no_offset(IR2_OPND *mem, int *newtmp)
{
    lsassert(ir2_opnd_is_mem(mem));
    IR2_OPND mem_opnd;

    int mem_offset = ir2_opnd_mem_get_offset(mem);

    IR2_OPND mem_old_base = ir2_opnd_mem_get_base(mem);
    IR2_OPND mem_new_base;

    if (mem_offset) {
        mem_new_base = ra_alloc_itemp(); *newtmp = 1;
        append_ir2_opnd2i(LISA_ADDI_D, &mem_new_base, &mem_old_base, mem_offset);
    } else {
        if (ir2_opnd_is_itemp(&mem_old_base)) {
             *newtmp = 0;
            mem_new_base = mem_old_base;
        } else {
            mem_new_base = ra_alloc_itemp(); *newtmp = 1;
            append_ir2_opnd3(LISA_OR, &mem_new_base, &mem_old_base,
                                                     &zero_ir2_opnd);
        }
    }

    ir2_opnd_build_mem(&mem_opnd,
            ir2_opnd_reg(&mem_new_base), /* base   */
            0);                          /* offset */

    return mem_opnd;
}

#endif
