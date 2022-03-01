#include "common.h"
#include "reg-alloc.h"
#include "ir2.h"
#include "latx-options.h"
#include "lsenv.h"
#include "translate.h"

/*
 * FIXME: LA only support 12bit.
 * Modify it for LA platform.
 */
void load_ireg_from_ir1_addrx(IR1_OPND *opnd1, IR2_OPND value_opnd)
{
    longx offset = ir1_opnd_simm(opnd1);
    int16 offset_imm_part;
    longx offset_reg_part;
    IR2_OPND offset_reg_opnd = ra_alloc_itemp();

    if (!(offset >= -2048 && offset <= 2047)){
        offset_imm_part = offset & 0x7FF;
        offset_reg_part = offset - offset_imm_part;
    } else {
        offset_imm_part = offset;
        offset_reg_part = offset - offset_imm_part;
    }

    //offset_imm_part = offset;
    //offset_reg_part = offset - offset_imm_part;
    //lsassert(((offset_reg_part >> 16) << 16) == offset_reg_part);

    /* 1. has no index */
    if (!ir1_opnd_has_index(opnd1)) { /* no index and scale */
        if (ir1_opnd_has_base(opnd1)) {
            /* 1.1. base and an optional offset */
            IR2_OPND base_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
            if (offset_reg_part == 0) {
                la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, value_opnd, base_opnd,
                                  offset_imm_part);
            } else {
                load_ireg_from_imm32(offset_reg_opnd, offset_reg_part,
                                     SIGN_EXTENSION);
                la_append_ir2_opnd3_em(LISA_ADD_ADDRX, value_opnd, base_opnd,
                                 offset_reg_opnd);
                if (offset_imm_part != 0)
                    la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, value_opnd, value_opnd,
                                      offset_imm_part);
            }
        } else {
            load_ireg_from_imm32(value_opnd, offset, SIGN_EXTENSION);
        }
    }

    /* 2. base, index, and an optional offset */
    else if (ir1_opnd_has_base(opnd1)) {
        IR2_OPND base_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
        IR2_OPND index_opnd = ra_alloc_gpr(ir1_opnd_index_reg_num(opnd1));
        IR2_OPND tmp_opnd = ra_alloc_itemp_internal();
        ;

        /* 2.1. prepare base + index*scale */
        if (ir1_opnd_scale(opnd1) == 1) {
            la_append_ir2_opnd3_em(LISA_ADD_ADDRX, value_opnd, base_opnd, index_opnd);
        } else if (ir1_opnd_scale(opnd1) == 2) {
            la_append_ir2_opnd2i_em(LISA_SLLI_W, tmp_opnd, index_opnd, 1);
            la_append_ir2_opnd3_em(LISA_ADD_ADDRX, value_opnd, tmp_opnd, base_opnd);
        } else if (ir1_opnd_scale(opnd1) == 4) {
            la_append_ir2_opnd2i_em(LISA_SLLI_W, tmp_opnd, index_opnd, 2);
            la_append_ir2_opnd3_em(LISA_ADD_ADDRX, value_opnd, tmp_opnd, base_opnd);
        } else if (ir1_opnd_scale(opnd1) == 8) {
            la_append_ir2_opnd2i_em(LISA_SLLI_W, tmp_opnd, index_opnd, 3);
            la_append_ir2_opnd3_em(LISA_ADD_ADDRX, value_opnd, tmp_opnd, base_opnd);
        } else {
            lsassert(0);
        }
        ra_free_temp(tmp_opnd);

        if (offset != 0) {
            if (offset_reg_part == 0) { /* offset_imm_part != 0 */
                la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, value_opnd, value_opnd,
                                  offset_imm_part);
            } else {
                load_ireg_from_imm32(offset_reg_opnd, offset_reg_part,
                                     SIGN_EXTENSION);
                la_append_ir2_opnd3_em(LISA_ADD_ADDRX, value_opnd, value_opnd,
                                 offset_reg_opnd);
                if (offset_imm_part != 0)
                    la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, value_opnd, value_opnd,
                                      offset_imm_part);
            }
        }

    }

    /* 3. index and an optional offset */
    else {
        IR2_OPND index_opnd = ra_alloc_gpr(ir1_opnd_index_reg_num(opnd1));

        /* 2.1. prepare index*scale */
        if (ir1_opnd_scale(opnd1) == 1) {
            la_append_ir2_opnd2i_em(LISA_SLLI_W, value_opnd, index_opnd, 0);
        } else if (ir1_opnd_scale(opnd1) == 2) {
            la_append_ir2_opnd2i_em(LISA_SLLI_W, value_opnd, index_opnd, 1);
        } else if (ir1_opnd_scale(opnd1) == 4) {
            la_append_ir2_opnd2i_em(LISA_SLLI_W, value_opnd, index_opnd, 2);
        } else if (ir1_opnd_scale(opnd1) == 8) {
            la_append_ir2_opnd2i_em(LISA_SLLI_W, value_opnd, index_opnd, 3);
        } else {
            lsassert(0);
        }

        if (offset != 0) {
            if (offset_reg_part == 0) {
                la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, value_opnd, value_opnd,
                                  offset_imm_part);
            } else {
                load_ireg_from_imm32(offset_reg_opnd, offset_reg_part,
                                     SIGN_EXTENSION);
                la_append_ir2_opnd3_em(LISA_ADD_ADDRX, value_opnd, value_opnd,
                                 offset_reg_opnd);
                if (offset_imm_part != 0)
                    la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, value_opnd, value_opnd,
                                      offset_imm_part);
            }
        }
    }

    /* 4. segment? */
    if (ir1_opnd_has_seg(opnd1)) {
        lsassertm(0, "not implemented in %s\n", __FUNCTION__);
        /* IR2_OPND seg_base_opnd = ra_alloc_itemp_internal(); */
        /* append_ir2_opnd2i(mips_load_addrx, seg_base_opnd, env_ir2_opnd, */
        /* env->offset_of_seg_base(ir1_opnd_get_seg_index(opnd1);)); */
        /* append_ir2_opnd3(mips_add_addrx, value_opnd, seg_base_opnd, */
        /* value_opnd); */
    }
    ra_free_temp(offset_reg_opnd);
}

IR2_OPND convert_mem_opnd(IR1_OPND *opnd1)
{
    return convert_mem_opnd_with_bias(opnd1, 0);
}

IR2_OPND convert_mem_opnd_with_no_offset(IR1_OPND *opnd1)
{
    return convert_mem_opnd_with_bias(opnd1, 0xffff);
}


/**
@convert an ir1 memory operand to an ir2 memory operand. internal temp registers
may be used.
@param the ir1 memory operand
@return the ir2 memory operand
*/
IR2_OPND convert_mem_opnd_with_bias(IR1_OPND *opnd1, int bias)
{
    IR2_OPND mem_opnd;

    longx offset = ir1_opnd_simm(opnd1) + ((bias==0xffff)?0:bias);
    int16 offset_imm_part;
    longx offset_reg_part;
    IR2_OPND offset_reg_opnd = ra_alloc_itemp();
    if (offset >= -2048 &&
        offset <= 2047 - 7 && (bias!=0xffff)) { /* minus 7 for the sake of mda process */
        offset_imm_part = offset;
        offset_reg_part = 0;
    } else {
        // error when add and sub, add may overflow, then error in sub
        // offset_imm_part = offset;
        // offset_reg_part = offset - offset_imm_part;
        // lsassert(((offset_reg_part >> 16) << 16) ==
        //          offset_reg_part); /* make sure that the low 16 bits are */
        //                            /* 0//make sure that the low 16 bits are 0 */
        // if (offset_imm_part > 32767 - 7) {
        //     offset_imm_part = 0;
        //     offset_reg_part = offset;
        // }
        offset_imm_part = 0;
        offset_reg_part = offset;
        load_ireg_from_imm32(offset_reg_opnd, offset_reg_part, SIGN_EXTENSION);
    }

    /* 1. has no index */
    if (!ir1_opnd_has_index(opnd1)) { /* no index and scale */
        if (ir1_opnd_has_base(opnd1)) {
            /* 1.1. base and an optional offset */
            IR2_OPND base_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
            if (offset_reg_part == 0) {
                ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
                                ir2_opnd_base_reg_num(&base_opnd),
                                offset_imm_part);
            } else {
                la_append_ir2_opnd3_em(LISA_ADD_ADDRX, offset_reg_opnd,
                                 offset_reg_opnd, base_opnd);
                ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
                                ir2_opnd_base_reg_num(&offset_reg_opnd),
                                offset_imm_part);
            }
        } else {
            /* 1.2. only an offset */

            if (offset_reg_part == 0) {
                ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM, 0, offset_imm_part);
            } else {
                ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
                                ir2_opnd_base_reg_num(&offset_reg_opnd),
                                offset_imm_part);
            }
        }
    }

    /* 2. base, index, and an optional offset */
    else if (ir1_opnd_has_base(opnd1)) {
        IR2_OPND base_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
        IR2_OPND index_opnd = ra_alloc_gpr(ir1_opnd_index_reg_num(opnd1));
        ir2_opnd_set_em(&index_opnd, UNKNOWN_EXTENSION, 32);
        mem_opnd = ra_alloc_itemp_internal();

        /* 2.1. prepare base + index*scale */
        if (ir1_opnd_scale(opnd1) == 1) {
            la_append_ir2_opnd3_em(LISA_ADD_ADDRX, mem_opnd, base_opnd, index_opnd);
        } else if (ir1_opnd_scale(opnd1) == 2) {
            la_append_ir2_opnd2i_em(LISA_SLLI_D, mem_opnd, index_opnd, 1);
            la_append_ir2_opnd3_em(LISA_ADD_ADDRX, mem_opnd, mem_opnd, base_opnd);
        } else if (ir1_opnd_scale(opnd1) == 4) {
            la_append_ir2_opnd2i_em(LISA_SLLI_D, mem_opnd, index_opnd, 2);
            la_append_ir2_opnd3_em(LISA_ADD_ADDRX, mem_opnd, mem_opnd, base_opnd);
        } else if (ir1_opnd_scale(opnd1) == 8) {
            la_append_ir2_opnd2i_em(LISA_SLLI_D, mem_opnd, index_opnd, 3);
            la_append_ir2_opnd3_em(LISA_ADD_ADDRX, mem_opnd, mem_opnd, base_opnd);
        } else {
            lsassert(0);
        }

        /* 2.2. add offset */
        if (offset_reg_part == 0) {
            ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
                            ir2_opnd_base_reg_num(&mem_opnd), offset_imm_part);
        } else {
            la_append_ir2_opnd3_em(LISA_ADD_ADDRX, offset_reg_opnd, offset_reg_opnd,
                             mem_opnd);
            ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
                            ir2_opnd_base_reg_num(&offset_reg_opnd),
                            offset_imm_part);
        }
    }

    /* 3. index and an optional offset */
    else {
        IR2_OPND index_opnd = ra_alloc_gpr(ir1_opnd_index_reg_num(opnd1));
        mem_opnd = ra_alloc_itemp_internal();

        /* 2.1. prepare index*scale */
        if (ir1_opnd_scale(opnd1) == 1) {
            la_append_ir2_opnd2i_em(LISA_SLLI_D, mem_opnd, index_opnd, 0);
        } else if (ir1_opnd_scale(opnd1) == 2) {
            la_append_ir2_opnd2i_em(LISA_SLLI_D, mem_opnd, index_opnd, 1);
        } else if (ir1_opnd_scale(opnd1) == 4) {
            la_append_ir2_opnd2i_em(LISA_SLLI_D, mem_opnd, index_opnd, 2);
        } else if (ir1_opnd_scale(opnd1) == 8) {
            la_append_ir2_opnd2i_em(LISA_SLLI_D, mem_opnd, index_opnd, 3);
        } else {
            lsassert(0);
        }

        /* 2.2. add offset */
        if (offset_reg_part == 0) {
            ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
                            ir2_opnd_base_reg_num(&mem_opnd), offset_imm_part);
        } else {
            la_append_ir2_opnd3_em(LISA_ADD_ADDRX, offset_reg_opnd, mem_opnd,
                             offset_reg_opnd);
            ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
                            ir2_opnd_base_reg_num(&offset_reg_opnd),
                            offset_imm_part);
        }
    }

    /* 4. segment? */
    if (ir1_opnd_has_seg(opnd1)) {
        // lsassertm(0, "not implemented in %s\n", __FUNCTION__);
        IR2_OPND seg_base_opnd = ra_alloc_itemp_internal();
        la_append_ir2_opnd2i_em(LISA_LD_W, seg_base_opnd, env_ir2_opnd,
                          lsenv_offset_of_seg_base(
                              lsenv, ir1_opnd_get_seg_index(opnd1)));
        int old_base_reg_num = ir2_opnd_base_reg_num(&mem_opnd);

        // if (old_base_reg_num != 0)
        //     append_ir2_opnd3(
        //         mips_add_addrx, seg_base_opnd, seg_base_opnd,
        //         ir2_opnd_new(IR2_OPND_IREG, old_base_reg_num)); /* TODO */

        IR2_OPND old_base = ir2_opnd_new(IR2_OPND_IREG, old_base_reg_num);
        ir2_opnd_set_em(&old_base, SIGN_EXTENSION, 32);
        if (old_base_reg_num != 0)
            la_append_ir2_opnd3_em(
                LISA_ADD_ADDRX, seg_base_opnd, seg_base_opnd,
                old_base);

        ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
                        ir2_opnd_base_reg_num(&seg_base_opnd),
                        ir2_opnd_imm(&mem_opnd));

        la_append_ir2_opnd3_em(LISA_AND, seg_base_opnd, seg_base_opnd, n1_ir2_opnd);
    }
    /* ra_free_temp(offset_reg_opnd); */
    return mem_opnd;
}

IR2_OPND convert_mem_addr(ADDR value)
{
    IR2_OPND mem_opnd = ra_alloc_itemp_internal();
    load_ireg_from_addr(mem_opnd, (uint64)value);
    return mem_opnd;
}

/**
@convert an ir1 register operand to an ir2 register operand.
@param the ir1 register operand
@return the ir2 register operand. it can be temp register or a mapped register
*/
IR2_OPND convert_gpr_opnd(IR1_OPND *opnd1, EXTENSION_MODE em)
{
    lsassert(ir1_opnd_is_gpr(opnd1));
    /*assert(em == UNKNOWN_EXTENSION || em==SIGN_EXTENSION || em ==
     * ZERO_EXTENSION);*/
    int gpr_num = ir1_opnd_base_reg_num(opnd1);
    int opnd_size = ir1_opnd_size(opnd1);

    IR2_OPND gpr_opnd = ra_alloc_gpr(gpr_num);

    /* 1. 32 bits gpr needs SHIFT operation to handle the extension mode */
    if (opnd_size == 32) {
        /* 1.1. need sign-extended but gpr is not */
        if (em == SIGN_EXTENSION && !ir2_opnd_is_sx(&gpr_opnd, 32)) {
            IR2_OPND ret_opnd = ra_alloc_itemp_internal();
            la_append_ir2_opnd2_em(LISA_MOV32_SX, ret_opnd, gpr_opnd);
            return ret_opnd;
        }
        /* 1.2. need zero-extended but gpr is not */
        else if (em == ZERO_EXTENSION && !ir2_opnd_is_zx(&gpr_opnd, 32)) {
            IR2_OPND ret_opnd = ra_alloc_itemp_internal();
            la_append_ir2_opnd2_em(LISA_MOV32_ZX, ret_opnd, gpr_opnd);
            return ret_opnd;
        }
        /* 1.3. gpr is what we need, or we need any extension, return gpr */
        /* directly */
        else {
            return gpr_opnd;
        }
    }

    /* 2. 64 bits gpr needs no extension */
    else if (opnd_size == 64) {
        return gpr_opnd;
    }

    /* 3. 16 bits gpr uses AND operation when zero extended */
    else if (opnd_size == 16) {
        IR2_OPND gpr_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
        /* 3.1. need sign-extended but gpr is usually not */
        if (em == SIGN_EXTENSION && !ir2_opnd_is_sx(&gpr_opnd, 16)) {
            IR2_OPND ret_opnd = ra_alloc_itemp_internal();
            la_append_ir2_opnd2i_em(LISA_SLLI_W, ret_opnd, gpr_opnd, 16);
            la_append_ir2_opnd2i_em(LISA_SRAI_W, ret_opnd, ret_opnd, 16);
            lsassert(ir2_opnd_is_sx(&ret_opnd, 16));
            return ret_opnd;
        }
        /* 3.2. need zero-extended but gpr is usually not */
        else if (em == ZERO_EXTENSION && !ir2_opnd_is_zx(&gpr_opnd, 16)) {
            IR2_OPND ret_opnd = ra_alloc_itemp_internal();

            la_append_ir2_opnd2ii(LISA_BSTRPICK_W,ret_opnd,gpr_opnd,15,0);
            ir2_opnd_set_em(&ret_opnd,ZERO_EXTENSION,16);

            lsassert(ir2_opnd_is_zx(&ret_opnd, 16));
            return ret_opnd;
        }
        /* 3.3. need any extension, return gpr directly */
        else {
            return gpr_opnd;
        }
    }

    /* 4. 8 bits gpr high */
    else if (ir1_opnd_is_8h(opnd1)) {
        IR2_OPND gpr_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
        IR2_OPND ret_opnd = ra_alloc_itemp_internal();
        /* 4.1. we need sign extension */
        if (em == SIGN_EXTENSION) {
            if (ir2_opnd_is_sx(&gpr_opnd, 16))
                la_append_ir2_opnd2i_em(LISA_SRAI_W, ret_opnd, gpr_opnd, 8);
            else {
                la_append_ir2_opnd2i_em(LISA_SLLI_W, ret_opnd, gpr_opnd, 16);
                la_append_ir2_opnd2i_em(LISA_SRAI_W, ret_opnd, ret_opnd, 24);
            }
            return ret_opnd;
        }
        /* 4.2. we need zero extension */
        else if (em == ZERO_EXTENSION) {
            if (ir2_opnd_is_zx(&gpr_opnd, 16))
                la_append_ir2_opnd2i_em(LISA_SRAI_W, ret_opnd, gpr_opnd, 8);
            else {
                la_append_ir2_opnd2ii(LISA_BSTRPICK_D, ret_opnd, gpr_opnd, 15, 8);
            }
            return ret_opnd;
        }
        /* 4.3. we need any extension */
        else {
            la_append_ir2_opnd2i_em(LISA_SRLI_D, ret_opnd, gpr_opnd, 8);
            return ret_opnd;
        }
    }

    /* 5. 8 bits gpr low */
    else {
        IR2_OPND gpr_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
        /* 5.1. need sign-extended but gpr is usually not */
        if (em == SIGN_EXTENSION && !ir2_opnd_is_sx(&gpr_opnd, 8)) {
            IR2_OPND ret_opnd = ra_alloc_itemp_internal();
            la_append_ir2_opnd2i_em(LISA_SLLI_W, ret_opnd, gpr_opnd, 24);
            la_append_ir2_opnd2i_em(LISA_SRAI_W, ret_opnd, ret_opnd, 24);
            return ret_opnd;
        }
        /* 5.2. need zero-extended but gpr is usually not */
        else if (em == ZERO_EXTENSION && !ir2_opnd_is_zx(&gpr_opnd, 8)) {
            IR2_OPND ret_opnd = ra_alloc_itemp_internal();
            la_append_ir2_opnd2i_em(LISA_ANDI, ret_opnd, gpr_opnd, 0xff);
            return ret_opnd;
        }
        /* 5.3. any extension */
        else {
            return gpr_opnd;
        }
    }
}

/*
 * Load imm based on LA insn.
 */
static void load_imm64(IR2_OPND opnd2, int64 value)
{
    if (value >> 11 == -1 || value >> 11 == 0) {
        la_append_ir2_opnd2i(LISA_ADDI_D, opnd2, zero_ir2_opnd, value);
    } else if (value >> 12 == 0) {
        la_append_ir2_opnd2i(LISA_ORI, opnd2, zero_ir2_opnd, value);
    } else if (value >> 31 == -1 || value >> 31 == 0) {
        la_append_ir2_opnd1i(LISA_LU12I_W, opnd2, value >> 12);
        if (value & 0xfff)
            la_append_ir2_opnd2i(LISA_ORI, opnd2, opnd2, value & 0xfff);
    } else if (value >> 32 == 0) {
        la_append_ir2_opnd1i(LISA_LU12I_W, opnd2, value >> 12);
        if (value & 0xfff)
            la_append_ir2_opnd2i(LISA_ORI, opnd2, opnd2, value & 0xfff);
        la_append_ir2_opnd1i(LISA_LU32I_D, opnd2, 0);
    } else if (value >> 51 == -1 || value >> 51 == 0) {
        if ((value & 0xffffffff) == 0)
            la_append_ir2_opnd3(LISA_OR, opnd2, zero_ir2_opnd, zero_ir2_opnd);
        else if ((value & 0xfff) == 0)
            la_append_ir2_opnd1i(LISA_LU12I_W, opnd2, value >> 12);
        else {
            la_append_ir2_opnd1i(LISA_LU12I_W, opnd2, value >> 12);
            la_append_ir2_opnd2i(LISA_ORI, opnd2, opnd2, value & 0xfff);
        }
        la_append_ir2_opnd1i(LISA_LU32I_D, opnd2, value >> 32);
    } else {
        if ((value & 0xfffffffffffff) == 0) {
            la_append_ir2_opnd2i(LISA_LU52I_D, opnd2, zero_ir2_opnd,
                                 (value >> 52) & 0xfff);
        } else if ((value & 0xffffffff) == 0) {
            la_append_ir2_opnd3(LISA_OR, opnd2, zero_ir2_opnd, zero_ir2_opnd);
            la_append_ir2_opnd1i(LISA_LU32I_D, opnd2, value >> 32);
            la_append_ir2_opnd2i(LISA_LU52I_D, opnd2, opnd2,
                                 (value >> 52) & 0xfff);
        } else if ((value & 0xfff) == 0) {
            la_append_ir2_opnd1i(LISA_LU12I_W, opnd2, value >> 12);
            la_append_ir2_opnd1i(LISA_LU32I_D, opnd2, value >> 32);
            la_append_ir2_opnd2i(LISA_LU52I_D, opnd2, opnd2,
                                 (value >> 52) & 0xfff);
        } else {
            la_append_ir2_opnd1i(LISA_LU12I_W, opnd2, value >> 12);
            la_append_ir2_opnd2i(LISA_ORI, opnd2, opnd2, value & 0xfff);
            la_append_ir2_opnd1i(LISA_LU32I_D, opnd2, value >> 32);
            la_append_ir2_opnd2i(LISA_LU52I_D, opnd2, opnd2,
                                 (value >> 52) & 0xfff);
        }
    }
}

void load_ireg_from_imm32(IR2_OPND opnd2, uint32 value, EXTENSION_MODE em)
{
    lsassert(ir2_opnd_is_ireg(&opnd2));
    int16 high_16_bits = value >> 16;
    int16 low_16_bits = value;
    int64 imm = 0;

    if (high_16_bits == 0) {
        /* 1. should be 0000 0000 0000 xxxx */
        //append_ir2_opnd2i(mips_ori, opnd2, zero_ir2_opnd, low_16_bits);
        imm = low_16_bits;
        load_imm64(opnd2, imm & 0xFFFF);
        ir2_opnd_set_em(&opnd2, ZERO_EXTENSION, 16);
        return;
    } else if (high_16_bits == -1) {
        if (em == ZERO_EXTENSION) {
            /* lsassertm(0, "not implemented in %s\n", __FUNCTION__); */
            /* 2. should be 0000 0000 ffff xxxx */
            imm = value;
            imm = imm & 0xFFFFFFFF;
            lsassert((imm & 0x00000000FFFF0000) == 0xFFFF0000);
            load_imm64(opnd2, imm);
            ir2_opnd_set_em(&opnd2, ZERO_EXTENSION, 32);
            return;
        } else {
            /* 3. should be ffff ffff ffff xxxx */
            if (low_16_bits < 0) {
                imm = low_16_bits;
                lsassertm(((imm & 0xFFFFFFFFFFFF0000) == 0xFFFFFFFFFFFF0000),
                        "value=0x%x, imm = 0x%lx\n", value, imm);
                load_imm64(opnd2, imm);
                ir2_opnd_set_em(&opnd2, SIGN_EXTENSION, 16);
                return;
            } else {
                imm = (int32)value;
                lsassertm(((imm & 0xFFFFFFFFFFFF0000) == 0xFFFFFFFFFFFF0000),
                        "value=0x%x, imm = 0x%lx\n", value, imm);
                load_imm64(opnd2, value);
                ir2_opnd_set_em(&opnd2, SIGN_EXTENSION, 17);
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
                ir2_opnd_set_em(&opnd2, ZERO_EXTENSION, 32);
                return;
            } else {
                imm = value;
                imm = imm & 0xFFFFFFFF;
                load_imm64(opnd2, imm);
                ir2_opnd_set_em(&opnd2, ZERO_EXTENSION, 31);
                return;
            }
        } else {
            /* 6. should be ssss ssss xxxx xxxx */
            imm = (int32)value;
            load_imm64(opnd2,imm);
            ir2_opnd_set_em(&opnd2, SIGN_EXTENSION, 32);
            return;
        }
    }
}

/**
@load an immediate value to a specific ir2 register operand
@param the ir2 operand
@param the immediate value
@return the ir2 register operand
*/
void load_ireg_from_imm64(IR2_OPND opnd2, uint64 value)
{
    lsassert(ir2_opnd_is_ireg(&opnd2));

    int32 high_32_bits = value >> 32;
    int32 low_32_bits = value;

    if (high_32_bits == 0) {
        /* 1. 0000 0000 xxxx xxxx */
        load_ireg_from_imm32(opnd2, low_32_bits, ZERO_EXTENSION);
        lsassert(ir2_opnd_is_zx(&opnd2, 32));
        return;
    } else if (high_32_bits == -1) {
        /* 2. ffff ffff xxxx xxxx */
        if (low_32_bits < 0) {
            load_ireg_from_imm32(opnd2, low_32_bits, SIGN_EXTENSION);
            lsassert(ir2_opnd_is_sx(&opnd2, 32));
            return;
        } else {
            load_ireg_from_imm32(opnd2, ~low_32_bits, ZERO_EXTENSION);
            la_append_ir2_opnd3_em(LISA_NOR, opnd2, opnd2, opnd2);
            lsassert(ir2_opnd_is_sx(&opnd2, 33));
            return;
        }
    } else {
        /* 3. xxxx xxxx xxxx xxxx */
        /*
         * I don't want to re-use mips implementation
         * to generate 64bits data with more than 4 insns.
         * Bits calculation comes from load_ireg_from_imm32.
         * In the end, bits + 32 is same as dsll did in mips.
         */
        int16_t high_16_bits = high_32_bits >> 16;
        int16_t low_16_bits = high_32_bits;
        int bits = 0;
        EXTENSION_MODE em = SIGN_EXTENSION;
        if (high_16_bits == 0) {
            em = ZERO_EXTENSION;
            bits = 16;
        } else if (high_16_bits == -1) {
            if (low_16_bits < 0) {
                bits = 16;
            } else {
                bits = 17;
            }
        } else {
            bits = 32;
        }
        load_imm64(opnd2,value);
        ir2_opnd_set_em(&opnd2, em, bits + 32);
        lsassert(ir2_opnd_is_ax(&opnd2, 32));
        return;
    }
}

void load_ireg_from_addrx(IR2_OPND opnd, ADDRX a)
{
#ifdef N64 /* validate address */
    load_ireg_from_imm32(opnd, a, ZERO_EXTENSION);
    lsassert(ir2_opnd_is_zx(&opnd, 32));
    ir2_opnd_set_em(&opnd, EM_X86_ADDRESS, 32);
#else
    load_ireg_from_imm32(opnd, a, ZERO_EXTENSION);
    lsassert(ir2_opnd_is_sx(&opnd) && ir2_opnd_is_zx(&opnd));
#endif
}

void load_ireg_from_addr(IR2_OPND opnd, ADDR a)
{
#if defined N64
    load_ireg_from_imm64(opnd, a);
    ir2_opnd_set_em(&opnd, EM_MIPS_ADDRESS, 32);
#else
    load_ireg_from_imm32(opnd, a, ZERO_EXTENSION);
    lsassert(ir2_opnd_is_sx(&opnd) && ir2_opnd_is_zx(&opnd));
#endif
}

/**
@load an ir1 immediate operand to a specific ir2 register operand
@param the ir2 operand
@param the ir1 immediate operand
@return the ir2 register operand
*/
static void load_ireg_from_ir1_imm(IR2_OPND opnd2, IR1_OPND *opnd1,
                                   EXTENSION_MODE em)
{
    lsassert(ir2_opnd_is_ireg(&opnd2));
    lsassert(ir1_opnd_is_imm(opnd1));
    if (em == ZERO_EXTENSION) {
        uint32 value = ir1_opnd_uimm(opnd1);
        load_ireg_from_imm32(opnd2, value, ZERO_EXTENSION);
    } else {
        int32 value = ir1_opnd_simm(opnd1);
        load_ireg_from_imm32(opnd2, value, SIGN_EXTENSION);
    }
}

IR2_OPND mem_ir1_to_ir2_opnd(IR1_OPND *opnd1, bool is_xmm_hi)
{
    IR2_OPND mem_opnd;
    if (is_xmm_hi)
        mem_opnd = convert_mem_opnd_with_bias(opnd1, 8);
    else
        mem_opnd = convert_mem_opnd(opnd1);

    int base_ireg_num = ir2_opnd_base_reg_num(&mem_opnd);

#ifdef N64
    /* take mips_lw for example
     *
     * original : lw    opnd2.reg,	opnd1.imm(opnd1.reg)
     *
     * via qemu : ld    tmp,	guest_base
     *     		  add   tmp,	tmp,	opnd1.reg
     *     		  ld	opnd2.reg,	opnd1.imm(tmp)
     *
     * register mapping optimization : $14(t2) always stores guest_base
     *
     *            add   tmp,    opnd1.reg,   $14
     *            ld    opnd2.reg, opnd1.imm(tmp)
     */
    /* no matter what, mem_opnd.reg stores the final address to access
     * we need to add this reg with guest_base to get the real host address
     * to access */
    if (cpu_get_guest_base() != 0) {
        IR2_OPND tmp = ra_alloc_itemp();
        IR2_OPND gbase = ra_alloc_guest_base();
        IR2_OPND mem = ir2_opnd_new(IR2_OPND_IREG, base_ireg_num);

        if (!ir2_opnd_is_address(&mem)) {
            la_append_ir2_opnd2_em(LISA_MOV_ADDRX, mem, mem);
        }
        la_append_ir2_opnd3_em(LISA_ADD_ADDR, tmp, mem, gbase);
        mem_opnd._reg_num = base_ireg_num = ir2_opnd_base_reg_num(&tmp);
        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
    }
#else
    lsassertm(0, "not implement for o32/n32 load/store.\n");
#endif
    return mem_opnd;
}

/**
@load an ir1 memory operand to a specific ir2 register operand. internal temp
registers may be used.
@param the ir1 memory operand
@return the ir2 register operand
*/
static void load_ireg_from_ir1_mem(IR2_OPND opnd2, IR1_OPND *opnd1,
                                   EXTENSION_MODE em, bool is_xmm_hi)
{
    lsassert(ir2_opnd_is_ireg(&opnd2));
    lsassert(ir1_opnd_is_mem(opnd1));

    IR2_OPND mem_opnd;
    mem_opnd = mem_ir1_to_ir2_opnd(opnd1, is_xmm_hi);
    int mem_imm = ir2_opnd_imm(&mem_opnd);
    mem_opnd._type = IR2_OPND_IREG; //decouple IR2_OPND_MEM to ireg and imm

    if (ir2_opnd_is_address(&mem_opnd))
        la_append_ir2_opnd3_em(LISA_AND, mem_opnd, mem_opnd, n1_ir2_opnd);

    if (ir1_opnd_size(opnd1) == 32) {
        if (em == ZERO_EXTENSION)
            la_append_ir2_opnd2i_em(LISA_LD_WU, opnd2, mem_opnd, mem_imm);
        else
            la_append_ir2_opnd2i_em(LISA_LD_W, opnd2, mem_opnd, mem_imm);
    } else if (ir1_opnd_size(opnd1) == 64 || ir1_opnd_size(opnd1) == 128) {
        la_append_ir2_opnd2i_em(LISA_LD_D, opnd2, mem_opnd, mem_imm);
    } else if (ir1_opnd_size(opnd1) == 8) {
        if (em == ZERO_EXTENSION)
            la_append_ir2_opnd2i_em(LISA_LD_BU, opnd2, mem_opnd, mem_imm);
        else
            la_append_ir2_opnd2i_em(LISA_LD_B, opnd2, mem_opnd, mem_imm);
    } else if (ir1_opnd_size(opnd1) == 16) {
        if (em == ZERO_EXTENSION)
            la_append_ir2_opnd2i_em(LISA_LD_HU, opnd2, mem_opnd, mem_imm);
        else
            la_append_ir2_opnd2i_em(LISA_LD_H, opnd2, mem_opnd, mem_imm);
    } else {
        lsassert(0);
    }
}

/**
@load an ir1 register operand to a specific ir2 register operand.
@param the ir1 register operand
@return the ir2 register operand. it can be temp register or a mapped register
*/
static void load_ireg_from_ir1_gpr(IR2_OPND opnd2, IR1_OPND *opnd1,
                                   EXTENSION_MODE em)
{
    lsassert(ir2_opnd_is_ireg(&opnd2));
    lsassert(ir1_opnd_is_gpr(opnd1));
    lsassert(em == UNKNOWN_EXTENSION || em == SIGN_EXTENSION ||
             em == ZERO_EXTENSION);

    int gpr_num = ir1_opnd_base_reg_num(opnd1);
    IR2_OPND gpr_opnd = ra_alloc_gpr(gpr_num);

    /* 1. 32 bits gpr needs SHIFT operation to handle the extension mode */
    if (ir1_opnd_size(opnd1) == 32) {
        /* 1.1. need sign-extended but gpr is not */
        if (em == SIGN_EXTENSION && !ir2_opnd_is_sx(&gpr_opnd, 32)) {
            la_append_ir2_opnd2_em(LISA_MOV32_SX, opnd2, gpr_opnd);
            return;
        }
        /* 1.2. need zero-extended but gpr is not */
        else if (em == ZERO_EXTENSION && !ir2_opnd_is_zx(&gpr_opnd, 32)) {
            la_append_ir2_opnd2_em(LISA_MOV32_ZX, opnd2, gpr_opnd);
            return;
        }
        /* 1.3. gpr is what we need, or we need any extension, return gpr */
        else {
            la_append_ir2_opnd2_em(LISA_MOV64, opnd2, gpr_opnd);
            return;
        }
    }

    /* 2. 64 bits gpr needs no extension */
    else if (ir1_opnd_size(opnd1) == 64) {
        la_append_ir2_opnd2_em(LISA_MOV64, opnd2, gpr_opnd);
        return;
    }

    /* 3. 16 bits gpr uses AND operation when zero extended */
    else if (ir1_opnd_size(opnd1) == 16) {
        /* 3.1. need sign-extended but gpr is usually not */
        if (em == SIGN_EXTENSION) {
            if (ir2_opnd_is_sx(&gpr_opnd, 16))
                la_append_ir2_opnd2_em(LISA_MOV32_SX, opnd2, gpr_opnd);
            else {
                la_append_ir2_opnd2i_em(LISA_SLLI_W, opnd2, gpr_opnd, 16);
                la_append_ir2_opnd2i_em(LISA_SRAI_W, opnd2, opnd2, 16);
            }
            return;
        }
        /* 3.1. need zero-extended but gpr is usually not */
        else {
            la_append_ir2_opnd2ii(LISA_BSTRPICK_D, opnd2, gpr_opnd, 15, 0);
            return;
        }
    }

    /* 4. 8 bits gpr high */
    else if (ir1_opnd_is_8h(opnd1)) {
        /* 4.1. we need sign extension */
        if (em == SIGN_EXTENSION) {
            if (ir2_opnd_is_sx(&gpr_opnd, 16))
                la_append_ir2_opnd2i_em(LISA_SRAI_W, opnd2, gpr_opnd, 8);
            else {
                la_append_ir2_opnd2i_em(LISA_SLLI_W, opnd2, gpr_opnd, 16);
                la_append_ir2_opnd2i_em(LISA_SRAI_W, opnd2, opnd2, 24);
            }
        }
        /* 4.2. we need zero extension */
        else if (em == ZERO_EXTENSION) {
            if (ir2_opnd_is_zx(&gpr_opnd, 16))
                la_append_ir2_opnd2i_em(LISA_SRAI_W, opnd2, gpr_opnd, 8);
            else {
                la_append_ir2_opnd2ii(LISA_BSTRPICK_D, opnd2, gpr_opnd, 15, 8);
            }
        }
        /* 4.3 we need any extension */
        else {
            la_append_ir2_opnd2i_em(LISA_SRAI_W, opnd2, gpr_opnd, 8);
        }
        return;
    }

    /* 5. 8 bits gpr low */
    else {
        /* 5.1. need sign-extended but gpr is usually not */
        if (em == SIGN_EXTENSION) {
            if (ir2_opnd_is_sx(&gpr_opnd, 8))
                la_append_ir2_opnd2_em(LISA_MOV32_SX, opnd2, gpr_opnd);
            else {
                la_append_ir2_opnd2i_em(LISA_SLLI_W, opnd2, gpr_opnd, 24);
                la_append_ir2_opnd2i_em(LISA_SRAI_W, opnd2, opnd2, 24);
            }
            return;
        }
        /* 5.2. zero extended */
        else {
            la_append_ir2_opnd2ii(LISA_BSTRPICK_D, opnd2, gpr_opnd, 7, 0);
            return;
        }
    }
}

static void load_ireg_from_ir1_mmx(IR2_OPND opnd2, IR1_OPND *opnd1,
                                   EXTENSION_MODE em)
{
    IR2_OPND mmx_opnd = ra_alloc_mmx(ir1_opnd_base_reg_num(opnd1));

    if (em == SIGN_EXTENSION) {
        la_append_ir2_opnd2_em(LISA_MOVFR2GR_S, opnd2, mmx_opnd);
    } else if (em == ZERO_EXTENSION) {
        la_append_ir2_opnd2_em(LISA_MOVFR2GR_D, opnd2, mmx_opnd);
        la_append_ir2_opnd2_em(LISA_MOV32_ZX, opnd2, opnd2);
    } else {
        la_append_ir2_opnd2_em(LISA_MOVFR2GR_D, opnd2, mmx_opnd);
    }
}

static void load_ireg_from_ir1_xmm(IR2_OPND opnd2, IR1_OPND *opnd1,
                                   EXTENSION_MODE em, bool is_xmm_hi)
{
    IR2_OPND xmm_opnd;

    if (is_xmm_hi) {
        xmm_opnd = ra_alloc_xmm_hi(ir1_opnd_base_reg_num(opnd1));
    } else {
        xmm_opnd = ra_alloc_xmm_lo(ir1_opnd_base_reg_num(opnd1));
    }

    if (em == SIGN_EXTENSION) {
        la_append_ir2_opnd2_em(LISA_MOVFR2GR_S, opnd2, xmm_opnd);
    } else if (em == ZERO_EXTENSION) {
        la_append_ir2_opnd2_em(LISA_MOVFR2GR_D, opnd2, xmm_opnd);
        la_append_ir2_opnd2_em(LISA_MOV32_ZX, opnd2, opnd2);
    } else {
        la_append_ir2_opnd2_em(LISA_MOVFR2GR_D, opnd2, xmm_opnd);
    }
}

/**
@load the value of an ir1 operand to an ir2 register operand
@param the ir1 operand (any type is ok)
@param the extension mode
@return the ir2 register operand. it can be temp register or a mapped register
*/
IR2_OPND load_ireg_from_ir1(IR1_OPND *opnd1, EXTENSION_MODE em, bool is_xmm_hi)
{
    lsassert(em == SIGN_EXTENSION || em == ZERO_EXTENSION ||
             em == UNKNOWN_EXTENSION);
    IR2_OPND ret_opnd;

    switch (ir1_opnd_type(opnd1)) {
    case X86_OP_IMM: {
        if (ir1_opnd_simm(opnd1) == 0)
            ret_opnd = zero_ir2_opnd;
        else {
            ret_opnd = ra_alloc_itemp_internal();
            load_ireg_from_ir1_imm(ret_opnd, opnd1, em);
        }
        break;
    }

    case X86_OP_REG:  {
        if(ir1_opnd_is_gpr(opnd1)){
            ret_opnd = convert_gpr_opnd(opnd1, em);
            /* Now it is OK for UNKNOWN_EXTENSION */
            break;
        }
        else if(ir1_opnd_is_mmx(opnd1)){
            ret_opnd = ra_alloc_itemp_internal();
            load_ireg_from_ir1_mmx(ret_opnd, opnd1, em);
            break;
        }
        else if(ir1_opnd_is_xmm(opnd1)){
            ret_opnd = ra_alloc_itemp_internal();
            load_ireg_from_ir1_xmm(ret_opnd, opnd1, em, is_xmm_hi);
            break;
        }
        else if(ir1_opnd_is_ymm(opnd1)){
            lsassert(0);
        }
        else if(ir1_opnd_is_seg(opnd1)){
            ret_opnd = ra_alloc_itemp_internal();
            int seg_num = ir1_opnd_base_reg_num(opnd1);
            la_append_ir2_opnd2i_em(LISA_LD_WU, ret_opnd, env_ir2_opnd,
                              lsenv_offset_of_seg_selector(lsenv, seg_num));
            break;
        }
        lsassert(0);
        break;
    }

    case X86_OP_MEM: {
        ret_opnd = ra_alloc_itemp_internal();
        load_ireg_from_ir1_mem(ret_opnd, opnd1, em, is_xmm_hi);
        break;
    }
    default:
        lsassert(0);
        break;
    }

    return ret_opnd;
}

/**
@load the value of an ir1 operand to a specific ir2 register operand
@param the ir2 operand
@param the ir1 operand (any type is ok)
@param the extension mode
*/
void load_ireg_from_ir1_2(IR2_OPND opnd2, IR1_OPND *opnd1, EXTENSION_MODE em,
                          bool is_xmm_hi)
{
    lsassert(ir2_opnd_is_ireg(&opnd2));
    switch (ir1_opnd_type(opnd1)) {
    case X86_OP_IMM:
        load_ireg_from_ir1_imm(opnd2, opnd1, em);
        break;
    case X86_OP_REG:  {
        if(ir1_opnd_is_gpr(opnd1)){
            load_ireg_from_ir1_gpr(opnd2, opnd1, em);
            break;
        }
        else if(ir1_opnd_is_mmx(opnd1)){
            load_ireg_from_ir1_mmx(opnd2, opnd1, em);
            break;
        }
        else if(ir1_opnd_is_xmm(opnd1)){
            load_ireg_from_ir1_xmm(opnd2, opnd1, em, is_xmm_hi);
            break;
        }
        else if(ir1_opnd_is_ymm(opnd1)){
            lsassert(0);
        }
        else if(ir1_opnd_is_seg(opnd1)){
            int seg_num = ir1_opnd_base_reg_num(opnd1);
            la_append_ir2_opnd2i_em(LISA_LD_WU, opnd2, env_ir2_opnd,
                              lsenv_offset_of_seg_selector(lsenv, seg_num));
            break;
        }
        lsassert(0);
        break;
    }
    case X86_OP_MEM:  {
        load_ireg_from_ir1_mem(opnd2, opnd1, em, is_xmm_hi);
        break;
    }
    default:
        lsassert(0);
    }

    return;
}

IR2_OPND load_ireg_from_cf_void(void) 
{
    IR2_OPND ret_opnd = ra_alloc_itemp_internal();

    la_append_ir2_opnd1i_em(LISA_X86MFFLAG, ret_opnd, 0x1);
    
    return ret_opnd;
} 

void load_ireg_from_cf_opnd(IR2_OPND *opnd2)
{
    lsassert(ir2_opnd_is_ireg(opnd2));
    la_append_ir2_opnd1i_em(LISA_X86MFFLAG, *opnd2, 0x1);
    return;
}

static void store_ireg_to_ir1_gpr(IR2_OPND opnd2, IR1_OPND *opnd1)
{
    lsassert(ir2_opnd_is_ireg(&opnd2));
    lsassert(ir1_opnd_is_gpr(opnd1) && ir2_opnd_is_ireg(&opnd2));
    int gpr_num = ir1_opnd_base_reg_num(opnd1);
    IR2_OPND gpr_opnd = ra_alloc_gpr(gpr_num);

    /* 1. 32 bits gpr needs SHIFT operation to handle the extension mode */
    if (ir1_opnd_size(opnd1) == 32) {
        if (!ir2_opnd_cmp(&opnd2, &gpr_opnd)) {
            /* validate address */
            if (ir2_opnd_is_ax(&opnd2, 32))
                la_append_ir2_opnd2_em(LISA_MOV32_SX, gpr_opnd, opnd2);
            else
                la_append_ir2_opnd2_em(LISA_MOV64, gpr_opnd, opnd2);
        }
    } else if (ir1_opnd_size(opnd1) == 64) {
        if (!ir2_opnd_cmp(&opnd2, &gpr_opnd)) {
            la_append_ir2_opnd2_em(LISA_MOV64, gpr_opnd, opnd2);
        }
    } else if (ir1_opnd_size(opnd1) == 16) {
        if (ir2_opnd_is_zx(&opnd2, 16)) {
            if (ir2_opnd_is_zx(
                    &gpr_opnd,
                    16)) { /* both gpr and opnd2 have just low 16 bits */
                if (ir2_opnd_base_reg_num(&gpr_opnd) !=
                    ir2_opnd_base_reg_num(&opnd2))
                    la_append_ir2_opnd2_em(LISA_MOV64, gpr_opnd, opnd2);
            } else { /* opnd2 has only low 16 bits */
                la_append_ir2_opnd2i_em(LISA_SRAI_D, gpr_opnd, gpr_opnd, 16);
                la_append_ir2_opnd2i_em(LISA_SLLI_D, gpr_opnd, gpr_opnd, 16);
                la_append_ir2_opnd3_em(LISA_OR, gpr_opnd, gpr_opnd, opnd2);
            }
        } else {
            if (ir2_opnd_is_zx(&gpr_opnd, 16)) { /*  gpr has only low 16 bits */
                la_append_ir2_opnd2ii(LISA_BSTRPICK_W, gpr_opnd, opnd2, 15, 0);
            } else { /* both gpr and opnd2 have high 16-bit data */

		la_append_ir2_opnd2ii(LISA_BSTRINS_D,gpr_opnd,opnd2,15,0);

            }
        }
        return;
    } else if (ir1_opnd_is_8h(opnd1)) {
        /* 1. put the result in place (8h) */
        IR2_OPND result_in_place = ra_alloc_itemp_internal();
        IR2_OPND imm = ra_alloc_itemp_internal();
        la_append_ir2_opnd2i_em(LISA_SLLI_W, result_in_place, opnd2, 8);
        if (!ir2_opnd_is_zx(&result_in_place, 16)) {
            //append_ir2_opnd2i(mips_andi, result_in_place, result_in_place,
                              //0xff00);
            /*
             * FIXME: Could not use pick. Consider better implementation later.
             */
            load_ireg_from_imm32(imm, 0xff00, ZERO_EXTENSION);
            la_append_ir2_opnd3_em(LISA_AND, result_in_place, result_in_place, imm);
        }

        /* 2. set into GPR */
        if (ir2_opnd_is_zx(&gpr_opnd, 8)) {
            la_append_ir2_opnd3_em(LISA_OR, gpr_opnd, gpr_opnd, result_in_place);
        } else if (ir2_opnd_is_zx(&gpr_opnd, 16)) {
            la_append_ir2_opnd2i_em(LISA_ANDI, gpr_opnd, gpr_opnd, 0xff);
            la_append_ir2_opnd3_em(LISA_OR, gpr_opnd, gpr_opnd, result_in_place);
        } else {
            /*
             * This make 0xAAAAAAAA ---> 0xAAAA00AA
             */
            //append_ir2_opnd2i(mips_ori, gpr_opnd, gpr_opnd, 0xff00);
            //append_ir2_opnd2i(mips_xori, gpr_opnd, gpr_opnd, 0xff00);
            la_append_ir2_opnd2ii(LISA_BSTRINS_D, gpr_opnd, zero_ir2_opnd, 15, 8);
            la_append_ir2_opnd3_em(LISA_OR, gpr_opnd, gpr_opnd, result_in_place);
        }

        ra_free_temp(result_in_place);
        ra_free_temp(imm);
        return;
    } else {
        if (ir2_opnd_is_zx(&opnd2, 8)) {
            if (ir2_opnd_is_zx(
                    &gpr_opnd,
                    8)) { /* both gpr and opnd2 have just low 8 bits */
                if (ir2_opnd_base_reg_num(&gpr_opnd) !=
                    ir2_opnd_base_reg_num(&opnd2))
                    la_append_ir2_opnd2_em(LISA_MOV64, gpr_opnd, opnd2);
            } else { /* opnd2 has only low 8 bits */
                la_append_ir2_opnd2i_em(LISA_SRAI_D, gpr_opnd, gpr_opnd, 8);
                la_append_ir2_opnd2i_em(LISA_SLLI_D, gpr_opnd, gpr_opnd, 8);
                la_append_ir2_opnd3_em(LISA_OR, gpr_opnd, gpr_opnd, opnd2);
            }
        } else {
            if (ir2_opnd_is_zx(&gpr_opnd, 8)) { /*  gpr has only low 8 bits */
                la_append_ir2_opnd2i_em(LISA_ANDI, gpr_opnd, opnd2, 0xff);
            } else { /* both gpr and opnd2 have high 24-bit data */
                IR2_OPND low_8_bits = ra_alloc_itemp_internal();
                la_append_ir2_opnd2i_em(LISA_ANDI, low_8_bits, opnd2, 0xff);
                la_append_ir2_opnd2i_em(LISA_SRAI_D, gpr_opnd, gpr_opnd, 8);
                la_append_ir2_opnd2i_em(LISA_SLLI_D, gpr_opnd, gpr_opnd, 8);
                la_append_ir2_opnd3_em(LISA_OR, gpr_opnd, gpr_opnd, low_8_bits);
                ra_free_temp(low_8_bits);
            }
        }
        return;
    }
}

static void store_ireg_to_ir1_mem(IR2_OPND value_opnd, IR1_OPND *opnd1,
                                  bool is_xmm_hi)
{
    lsassert(ir2_opnd_is_ireg(&value_opnd));
    IR2_OPND mem_opnd;
    if (is_xmm_hi)
        mem_opnd = convert_mem_opnd_with_bias(opnd1, 8);
    else
        mem_opnd = convert_mem_opnd(opnd1);

    int base_ireg_num = ir2_opnd_base_reg_num(&mem_opnd);

    int mem_imm = ir2_opnd_imm(&mem_opnd);
    /* decouple IR2_OPND_MEM to ireg and imm */
    mem_opnd._type = IR2_OPND_IREG;

    if (ir2_opnd_is_address(&mem_opnd))
        la_append_ir2_opnd3_em(LISA_AND, mem_opnd, mem_opnd, n1_ir2_opnd);
#ifdef N64
    if (cpu_get_guest_base() != 0) {
        IR2_OPND tmp = ra_alloc_itemp();
        IR2_OPND gbase = ra_alloc_guest_base();
        IR2_OPND mem = ir2_opnd_new(IR2_OPND_IREG, base_ireg_num);

        if (!ir2_opnd_is_address(&mem)) {
            la_append_ir2_opnd2_em(LISA_MOV_ADDRX, mem, mem);
        }
        la_append_ir2_opnd3_em(LISA_ADD_ADDR, tmp, mem, gbase);
        /* reset mem_opnd.reg as tmp */
        mem_opnd._reg_num = ir2_opnd_base_reg_num(&tmp);
        base_ireg_num = ir2_opnd_base_reg_num(&mem_opnd);
        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
}
#else
    lsassertm(0, "not implement for o32/n32 load/store.\n");
#endif
    if (ir1_opnd_size(opnd1) == 32) {
        la_append_ir2_opnd2i(LISA_ST_W, value_opnd, mem_opnd, mem_imm);
    } else if (ir1_opnd_size(opnd1) == 64 || ir1_opnd_size(opnd1) == 128) {
        la_append_ir2_opnd2i(LISA_ST_D, value_opnd, mem_opnd, mem_imm);
    } else if (ir1_opnd_size(opnd1) == 8) {
        la_append_ir2_opnd2i(LISA_ST_B, value_opnd, mem_opnd, mem_imm);
    } else if (ir1_opnd_size(opnd1) == 16) {
        la_append_ir2_opnd2i(LISA_ST_H, value_opnd, mem_opnd, mem_imm);
    } else {
        lsassert(0);
    }

    return;
}

void store_ireg_to_ir1_seg(IR2_OPND seg_value_opnd, IR1_OPND *opnd1)
{
    lsassert(ir2_opnd_is_ireg(&seg_value_opnd));
#ifdef N64

    /* 1. set selector */
    int seg_num = ir1_opnd_base_reg_num(opnd1);
    la_append_ir2_opnd2i(LISA_ST_W, seg_value_opnd, env_ir2_opnd,
                      lsenv_offset_of_seg_selector(lsenv, seg_num));

    /* 2. update seg cache : read data in GDT and store into seg cache */

    /* TI = 0 : GDT, TI = 1 : LDT */
    IR2_OPND label_ldt = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_base_end = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND is_ldt = ra_alloc_itemp_internal(); /* [51:48] [15: 0] limit */
    IR2_OPND dt_opnd = ra_alloc_itemp_internal();
    la_append_ir2_opnd2i_em(LISA_ANDI, is_ldt, seg_value_opnd, 0x4);
    la_append_ir2_opnd3(LISA_BNE, is_ldt, zero_ir2_opnd, label_ldt);
    ra_free_temp(is_ldt);
    /* 2.1 get gdt base */
    la_append_ir2_opnd2i_em(LISA_LD_W, dt_opnd, env_ir2_opnd,
                      lsenv_offset_of_gdt_base(lsenv));
    la_append_ir2_opnd1(LISA_B, label_base_end);
    /* 2.1 get ldt base */
    la_append_ir2_opnd1(LISA_LABEL, label_ldt);
    la_append_ir2_opnd2i_em(LISA_LD_W, dt_opnd, env_ir2_opnd,
                      lsenv_offset_of_ldt_base(lsenv));
    la_append_ir2_opnd1(LISA_LABEL, label_base_end);
    la_append_ir2_opnd3_em(LISA_AND, dt_opnd, dt_opnd, n1_ir2_opnd);

    /* 2.2 get entry offset of gdt and add it on gdt-base */
    IR2_OPND offset_in_gdt_opnd = ra_alloc_itemp_internal();
    IR2_OPND offset_imm16 = ra_alloc_itemp_internal();
    /*
     * We cannot generate imm16 directly.
     */
    load_ireg_from_imm32(offset_imm16, 0xfff8, ZERO_EXTENSION);
    la_append_ir2_opnd3_em(LISA_AND, offset_in_gdt_opnd, seg_value_opnd, offset_imm16);
    la_append_ir2_opnd3_em(LISA_ADD_D, dt_opnd, dt_opnd, offset_in_gdt_opnd);
    //append_ir2_opnd2i(mips_andi, offset_in_gdt_opnd, seg_value_opnd, 0xfff8);
    //append_ir2_opnd3(mips_daddu, dt_opnd, dt_opnd, offset_in_gdt_opnd);
    ra_free_temp(offset_in_gdt_opnd);
    ra_free_temp(offset_imm16);

    if (cpu_get_guest_base() != 0) {
        /* 2.3 add guest-base */
        IR2_OPND guest_base = ra_alloc_guest_base();
        if (!ir2_opnd_is_address(&dt_opnd)) {
            la_append_ir2_opnd2_em(LISA_MOV_ADDRX, dt_opnd, dt_opnd);
        }
        la_append_ir2_opnd3_em(LISA_ADD_ADDR, dt_opnd, dt_opnd, guest_base);
        ir2_opnd_set_em(&dt_opnd, EM_MIPS_ADDRESS, 32);
    }

    /* 2.4 read segment entry */
    IR2_OPND gdt_entry = ra_alloc_itemp_internal();

    la_append_ir2_opnd2i_em(LISA_LD_D, gdt_entry, dt_opnd, 0);
    ra_free_temp(dt_opnd);

    IR2_OPND seg_limit = ra_alloc_itemp_internal(); /* [51:48] [15: 0] limit */
    IR2_OPND seg_base = ra_alloc_itemp_internal();  /* [63:56] [39:16] base */
    IR2_OPND seg_flags = ra_alloc_itemp_internal(); /* [55:40] flags */

    /* 2.5 get new base */
    IR2_OPND tmp = ra_alloc_itemp_internal();
    /*FIXME: keep original mips code for review later. */
    //append_ir2_opnd2i(mips_dsra, seg_base, gdt_entry, 16);
    //append_ir2_opnd2i(mips_andi, seg_base, seg_base, 0xffff); /* [31:16] */
    la_append_ir2_opnd2ii(LISA_BSTRPICK_D, seg_base, gdt_entry, 31, 16);

    //append_ir2_opnd2i(mips_dsra32, tmp, gdt_entry, 0);
    //append_ir2_opnd2i(mips_andi, tmp, tmp, 0xff);
    //append_ir2_opnd2i(mips_sll, tmp, tmp, 16);
    la_append_ir2_opnd2i_em(LISA_SRAI_D, tmp, gdt_entry, 32);
    la_append_ir2_opnd2i_em(LISA_ANDI, tmp, tmp, 0xff);
    la_append_ir2_opnd2i_em(LISA_SLLI_W, tmp, tmp, 16);

    la_append_ir2_opnd3_em(LISA_OR, seg_base, seg_base, tmp); /* [39:32] */
    la_append_ir2_opnd2i_em(LISA_SRAI_D, tmp, gdt_entry, 24 + 32);
    la_append_ir2_opnd2i_em(LISA_ANDI, tmp, tmp, 0xff);
    la_append_ir2_opnd2i_em(LISA_SLLI_W, tmp, tmp, 24);

    la_append_ir2_opnd3_em(LISA_OR, seg_base, seg_base, tmp); /* [63:56] */

    /* 2.5 get new limit */
    //append_ir2_opnd2i(mips_andi, seg_limit, gdt_entry, 0xffff); /* [15: 0] */
    offset_imm16 = ra_alloc_itemp_internal();
    load_ireg_from_imm32(offset_imm16, 0xffff, ZERO_EXTENSION);
    la_append_ir2_opnd3_em(LISA_AND, seg_limit, gdt_entry, offset_imm16);

    la_append_ir2_opnd2i_em(LISA_SRAI_D, tmp, gdt_entry, 16 + 32);
    la_append_ir2_opnd2i_em(LISA_ANDI, tmp, tmp, 0xf);
    la_append_ir2_opnd2i_em(LISA_SLLI_W, tmp, tmp, 16);

    la_append_ir2_opnd3_em(LISA_OR, seg_limit, seg_limit, tmp); /* [35:32] */
    la_append_ir2_opnd2i_em(LISA_SLLI_W, seg_limit, seg_limit, 12);
    //append_ir2_opnd2i(mips_ori, seg_limit, seg_limit, 0xfff);
    /* imm16 is now 0xffff*/
    la_append_ir2_opnd3_em(LISA_AND, seg_limit, seg_limit, offset_imm16);

    /* 2.6 get flags in GDT */
    la_append_ir2_opnd2i_em(LISA_SRAI_D, tmp, gdt_entry, 8 + 32);
    //append_ir2_opnd2i(mips_andi, seg_flags, tmp, 0xffff);
    la_append_ir2_opnd3_em(LISA_AND, seg_flags, tmp, offset_imm16);
    la_append_ir2_opnd2i_em(LISA_ORI, seg_flags, seg_flags,
                      1); /* TypeField in GDT whichrepresent accessed, */
                          /* TypeField should be written to GDT,we ommit it */
    la_append_ir2_opnd2i_em(LISA_SLLI_W, seg_flags, seg_flags, 8);
    ra_free_temp(tmp);
    ra_free_temp(offset_imm16);
    ra_free_temp(gdt_entry);

    /* 2.7 write into seg cache */
    la_append_ir2_opnd2i(LISA_ST_D, seg_base, env_ir2_opnd,
                      lsenv_offset_of_seg_base(lsenv, seg_num));
    la_append_ir2_opnd2i(LISA_ST_W, seg_limit, env_ir2_opnd,
                      lsenv_offset_of_seg_limit(lsenv, seg_num));
    la_append_ir2_opnd2i(LISA_ST_W, seg_flags, env_ir2_opnd,
                      lsenv_offset_of_seg_flags(lsenv, seg_num));
    ra_free_temp(seg_limit);
    ra_free_temp(seg_base);
    ra_free_temp(seg_flags);

#else
    lsassertm(0, "not implement for MIPS o32/n32.\n");
#endif
}

void store_ireg_to_ir1(IR2_OPND opnd2, IR1_OPND *opnd1, bool is_xmm_hi)
{
    lsassert(ir2_opnd_is_ireg(&opnd2));
    if (!ir2_opnd_is_ireg(&opnd2)) {
        lsassertm(0, "error");
    }

    switch (ir1_opnd_type(opnd1)) {
    case X86_OP_REG:  {
        if(ir1_opnd_is_gpr(opnd1)){
            store_ireg_to_ir1_gpr(opnd2, opnd1);
            return;
        }
        else if(ir1_opnd_is_mmx(opnd1)){
            IR2_OPND mmx_opnd = ra_alloc_mmx(ir1_opnd_base_reg_num(opnd1));
            la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, mmx_opnd, opnd2);
            return;
        }
        else if(ir1_opnd_is_xmm(opnd1)){
            IR2_OPND mmx_opnd;
            if (is_xmm_hi)
                mmx_opnd = ra_alloc_xmm_hi(ir1_opnd_base_reg_num(opnd1));
            else
                mmx_opnd = ra_alloc_xmm_lo(ir1_opnd_base_reg_num(opnd1));
            la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, mmx_opnd, opnd2);
            return;
        }
        else if(ir1_opnd_is_ymm(opnd1)){
            lsassert(0);
        }
        else if (ir1_opnd_is_seg(opnd1)){
            store_ireg_to_ir1_seg(opnd2, opnd1);
            return;
        }
        lsassert(0);
        return;
    }

    case X86_OP_MEM:
        store_ireg_to_ir1_mem(opnd2, opnd1, is_xmm_hi);
        return;
    default:
        lsassert(0);
        return;
    }
}

/**
    load 80bit float from memory and convert to 64bit
*/
void load_64_bit_freg_from_ir1_80_bit_mem(IR2_OPND opnd2,
                                                 IR2_OPND mem_opnd, int mem_imm)
{
    /* load 80bit float from memory and convert it to 64bit float */
    IR2_OPND ir2_sign_exp = ra_alloc_ftemp();
    IR2_OPND ir2_fraction = ra_alloc_ftemp();

    lsassert(cpu_get_guest_base() == 0);
    lsassert(mem_imm + 8 <= 2047);

    if (ir2_opnd_is_address(&mem_opnd))
        la_append_ir2_opnd3_em(LISA_AND, mem_opnd, mem_opnd, n1_ir2_opnd);
    la_append_ir2_opnd2i(LISA_FLD_S, ir2_sign_exp, mem_opnd, mem_imm + 8);
    la_append_ir2_opnd2i(LISA_FLD_D, ir2_fraction, mem_opnd, mem_imm);

    IR2_OPND itemp_reg = ra_alloc_itemp();
    IR2_OPND itemp1_reg = ra_alloc_itemp();
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_no_excp1 = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_no_excp2 = ir2_opnd_new_type(IR2_OPND_LABEL);

    /* Temporarily mask V to make the conversion don't trigger SIGFPE */
    la_append_ir2_opnd2_em(LISA_MOVFCSR2GR, itemp_reg, fcsr_ir2_opnd);
    la_append_ir2_opnd2ii(LISA_BSTRPICK_D, itemp1_reg, itemp_reg, 4, 4);
    la_append_ir2_opnd3(LISA_BEQ, itemp1_reg, zero_ir2_opnd, label_no_excp1);
    la_append_ir2_opnd2i(LISA_XORI, itemp_reg, itemp_reg, 0x10);
    la_append_ir2_opnd2_em(LISA_MOVGR2FCSR, fcsr_ir2_opnd, itemp_reg);
    la_append_ir2_opnd1(LISA_LABEL, label_no_excp1);

    la_append_ir2_opnd3(LISA_FCVT_D_LD, opnd2, ir2_fraction, ir2_sign_exp);
    ra_free_temp(ir2_sign_exp);
    ra_free_temp(ir2_fraction);

    la_append_ir2_opnd2_em(LISA_MOVFCSR2GR, itemp_reg, fcsr_ir2_opnd);
    /* unmask V if necessary */
    la_append_ir2_opnd3(LISA_BEQ, itemp1_reg, zero_ir2_opnd, label_no_excp2);
    la_append_ir2_opnd2i(LISA_XORI, itemp_reg, itemp_reg, 0x10);
    la_append_ir2_opnd2_em(LISA_MOVGR2FCSR, fcsr_ir2_opnd, itemp_reg);
    la_append_ir2_opnd1(LISA_LABEL, label_no_excp2);

    la_append_ir2_opnd2ii(LISA_BSTRPICK_D, itemp_reg, itemp_reg, 28, 28);
    la_append_ir2_opnd3(LISA_BEQ, itemp_reg, zero_ir2_opnd, label_exit);
    /* Identify SNAN and change opnd2 to SNAN
     * exp==0x7fff && bit[62]==0 && bit[61:0]!=0
     *
     * FIXME: assume SNAN->QNAN when V, should check other cases
     */
    la_append_ir2_opnd2_em(LISA_MOVFR2GR_D, itemp_reg, opnd2);
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, itemp_reg, zero_ir2_opnd, 51, 51);
    la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, opnd2, itemp_reg);
    ra_free_temp(itemp_reg);
    ra_free_temp(itemp1_reg);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);
}

/**
@load an ir1 memory operand to a specific ir2 register operand. internal temp
registers may be used.
@param the ir1 memory operand
@return the ir2 register operand
*/
static void load_freg_from_ir1_mem(IR2_OPND opnd2, IR1_OPND *opnd1,
                                   bool is_xmm_hi, uint32 options)
{
    IR2_OPND mem_opnd;
    bool is_convert = (options & IS_CONVERT);
    bool is_dest_mmx = (options & IS_DEST_MMX) >> 3;

    if (is_xmm_hi)
        mem_opnd = convert_mem_opnd_with_bias(opnd1, 8);
    else
        mem_opnd = convert_mem_opnd(opnd1);
    int mem_imm = ir2_opnd_imm(&mem_opnd);
    mem_opnd._type = IR2_OPND_IREG; //decouple IR2_OPND_MEM to ireg and imm
    if (ir2_opnd_is_address(&mem_opnd))
        la_append_ir2_opnd3_em(LISA_AND, mem_opnd, mem_opnd, n1_ir2_opnd);

    lsassert(cpu_get_guest_base() == 0);

    if (ir1_opnd_size(opnd1) == 32) {
        la_append_ir2_opnd2i(LISA_FLD_S, opnd2, mem_opnd, mem_imm);
        if (is_convert)
            la_append_ir2_opnd2(LISA_FCVT_D_S, opnd2, opnd2);
    } else if (ir1_opnd_size(opnd1) == 64 || ir1_opnd_size(opnd1) == 128) {
        IR2_OPND ftemp = ra_alloc_ftemp_internal();
        la_append_ir2_opnd2i(LISA_FLD_D, opnd2, mem_opnd, mem_imm);
        //64->80->64 to handle the implicit SNAN->QNAN of fld
        if (!is_dest_mmx) {
            la_append_ir2_opnd2(LISA_FCVT_LD_D, ftemp, opnd2);
            la_append_ir2_opnd2(LISA_FCVT_UD_D, opnd2, opnd2);
            la_append_ir2_opnd3(LISA_FCVT_D_LD, opnd2, ftemp, opnd2);
        }
        ra_free_temp(ftemp);
    } else if (ir1_opnd_size(opnd1) == 16) {
        IR2_OPND itemp = ra_alloc_itemp_internal();
        la_append_ir2_opnd2i_em(LISA_LD_H, itemp, mem_opnd, mem_imm);
        la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, opnd2, itemp);
        ra_free_temp(itemp);

        lsassertm(!is_convert, "convert 16-bit floating point?\n");
    } else if (ir1_opnd_size(opnd1) == 8) {
        IR2_OPND itemp = ra_alloc_itemp_internal();
        la_append_ir2_opnd2i_em(LISA_LD_B, itemp, mem_opnd, mem_imm);
        la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, opnd2, itemp);
        ra_free_temp(itemp);

        lsassertm(!is_convert, "convert 8-bit floating point?\n");
    } else {
        load_64_bit_freg_from_ir1_80_bit_mem(opnd2, mem_opnd, mem_imm);
    }
}

/**
@load an ir1 register operand to a specific ir2 register operand.
@param the ir1 register operand
@return the ir2 register operand. it can be temp register or a mapped register
*/

static void load_freg_from_ir1_fpr(IR2_OPND opnd2, IR1_OPND *opnd1)
{
    lsassert(ir2_opnd_is_freg(&opnd2));
    IR2_OPND value_opnd = ra_alloc_st(ir1_opnd_base_reg_num(opnd1));
    if (!ir2_opnd_cmp(&opnd2, &value_opnd))
        la_append_ir2_opnd2_em(LISA_FMOV_D, opnd2, value_opnd);
}

static void load_freg_from_ir1_mmx(IR2_OPND opnd2, IR1_OPND *opnd1)
{
    lsassert(ir2_opnd_is_freg(&opnd2));
    lsassert(ir1_opnd_is_mmx(opnd1));
    IR2_OPND mmx_opnd = ra_alloc_mmx(ir1_opnd_base_reg_num(opnd1));
    if (!ir2_opnd_cmp(&opnd2, &mmx_opnd))
        la_append_ir2_opnd2_em(LISA_FMOV_D, opnd2, mmx_opnd);
}

static void load_freg_from_ir1_xmm(IR2_OPND opnd2, IR1_OPND *opnd1,
                                   bool is_xmm_hi)
{
    lsassert(ir2_opnd_is_freg(&opnd2));
    lsassert(ir1_opnd_is_xmm(opnd1));
    IR2_OPND xmm_opnd;
    if (is_xmm_hi)
        xmm_opnd = ra_alloc_xmm_hi(ir1_opnd_base_reg_num(opnd1));
    else
        xmm_opnd = ra_alloc_xmm_lo(ir1_opnd_base_reg_num(opnd1));
    if (!ir2_opnd_cmp(&opnd2, &xmm_opnd))
        la_append_ir2_opnd2_em(LISA_FMOV_D, opnd2, xmm_opnd);
}

IR2_OPND load_freg_from_ir1_1(IR1_OPND *opnd1, bool is_xmm_hi, uint32_t options)
{
    /*
     * NOTE: Previous arg support is_covert only, to minimum modification,
     * here is a around to support new options.
     * bit0: false
     * bit1: true (is_covert)
     * bit3: (1 << 3) to avoid float covertion
     */

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
        break;
    }
    case X86_OP_MEM: {
        IR2_OPND ret_opnd = ra_alloc_ftemp_internal();
        load_freg_from_ir1_mem(ret_opnd, opnd1, is_xmm_hi, options);
        return ret_opnd;
    }
    default:
        lsassert(0);
    }
    abort();
}

void load_freg_from_ir1_2(IR2_OPND opnd2, IR1_OPND *opnd1, uint32_t options)
{
    lsassert(ir2_opnd_is_freg(&opnd2));
    bool is_xmm_hi = (options & IS_XMM_HI) >> 2;
    assert(is_xmm_hi == 0);

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
        return;
    }

    case X86_OP_MEM: {
        load_freg_from_ir1_mem(opnd2, opnd1, is_xmm_hi,
                               options & (IS_CONVERT | IS_DEST_MMX));
        break;
    }
    default:
        lsassert(0);
        break;
    }

    return;
}

void load_singles_from_ir1_pack(IR2_OPND single0, IR2_OPND single1,
                                IR1_OPND *opnd1, bool is_xmm_hi)
{
    switch (ir1_opnd_type(opnd1)) {
    // case IR1_OPND_XMM: {
    case X86_OP_REG: {
        if(ir1_opnd_is_xmm(opnd1)){
            IR2_OPND half_xmm;
            if (is_xmm_hi)
                half_xmm = ra_alloc_xmm_hi(ir1_opnd_base_reg_num(opnd1));
            else
                half_xmm = ra_alloc_xmm_lo(ir1_opnd_base_reg_num(opnd1));

            /* retrieve the 1th single from half_xmm */
            IR2_OPND itemp0 = ra_alloc_itemp();
            la_append_ir2_opnd2_em(LISA_MOVFR2GR_D, itemp0, half_xmm);
            IR2_OPND itemp1 = ra_alloc_itemp();
            la_append_ir2_opnd2i_em(LISA_SRLI_D, itemp1, itemp0, 32);
            la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, single1, itemp1);
            ra_free_temp(itemp1);
            /* retrive the 0th single from src xmm operand */
            la_append_ir2_opnd2i_em(LISA_SLLI_D, itemp0, itemp0, 32);
            la_append_ir2_opnd2i_em(LISA_SRLI_D, itemp0, itemp0, 32);
            la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, single0, itemp0);
            ra_free_temp(itemp0);
        }
        break;
    }
    case X86_OP_MEM:  {
        /* retrieve the 1th single */
        IR2_OPND itemp0 = ra_alloc_itemp();
        load_ireg_from_ir1_mem(itemp0, opnd1, ZERO_EXTENSION, is_xmm_hi);
        IR2_OPND itemp1 = ra_alloc_itemp();
        la_append_ir2_opnd2i_em(LISA_SRLI_D, itemp1, itemp0, 32);
        la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, single1, itemp1);
        ra_free_temp(itemp1);
        /* retrive the 0th single */
        la_append_ir2_opnd2i_em(LISA_SLLI_D, itemp0, itemp0, 32);
        la_append_ir2_opnd2i_em(LISA_SRLI_D, itemp0, itemp0, 32);
        la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, single0, itemp0);
        ra_free_temp(itemp0);
        break;
    }
    default:
        lsassert(0);
        break;
    }
    return;
}

void store_singles_to_ir2_pack(IR2_OPND single0, IR2_OPND single1,
                               IR2_OPND pack)
{
    lsassert(ir2_opnd_is_freg(&single0) && ir2_opnd_is_freg(&single1) &&
             ir2_opnd_is_freg(&pack));
    IR2_OPND itemp1 = ra_alloc_itemp();
    la_append_ir2_opnd2_em(LISA_MOVFR2GR_D, itemp1, single1);
    la_append_ir2_opnd2i_em(LISA_SLLI_D, itemp1, itemp1, 32);
    IR2_OPND itemp0 = ra_alloc_itemp();
    la_append_ir2_opnd2_em(LISA_MOVFR2GR_D, itemp0, single0);
    la_append_ir2_opnd3_em(LISA_OR, itemp1, itemp1, itemp0);
    la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, pack, itemp1);
    ra_free_temp(itemp1);
    ra_free_temp(itemp0);
}

void store_64_bit_freg_to_ir1_80_bit_mem(IR2_OPND opnd2, IR2_OPND mem_opnd)
{
    IR2_OPND ir2_sign_exp = ra_alloc_ftemp();
    IR2_OPND ir2_fraction = ra_alloc_ftemp();
    IR2_OPND itemp = ra_alloc_itemp();
    IR2_OPND itemp1 = ra_alloc_itemp();
    IR2_OPND label_ok = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_no_excp1 = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_no_excp2 = ir2_opnd_new_type(IR2_OPND_LABEL);
    int mem_imm = ir2_opnd_imm(&mem_opnd);
    mem_opnd._type = IR2_OPND_IREG; //decouple IR2_OPND_MEM to ireg and imm
    if (ir2_opnd_is_address(&mem_opnd))
        la_append_ir2_opnd3_em(LISA_AND, mem_opnd, mem_opnd, n1_ir2_opnd);

    /* Temporarily mask V to make the conversion don't trigger SIGFPE */
    la_append_ir2_opnd2_em(LISA_MOVFCSR2GR, itemp, fcsr_ir2_opnd);
    la_append_ir2_opnd2ii(LISA_BSTRPICK_D, itemp1, itemp, 4, 4);
    la_append_ir2_opnd3(LISA_BEQ, itemp1, zero_ir2_opnd, label_no_excp1);
    la_append_ir2_opnd2i(LISA_XORI, itemp, itemp, 0x10);
    la_append_ir2_opnd2_em(LISA_MOVGR2FCSR, fcsr_ir2_opnd, itemp);
    la_append_ir2_opnd1(LISA_LABEL, label_no_excp1);

    lsassert(mem_imm + 8 <= 2047);
    la_append_ir2_opnd2(LISA_FCVT_LD_D, ir2_fraction, opnd2);
    la_append_ir2_opnd2(LISA_FCVT_UD_D, ir2_sign_exp, opnd2);
    la_append_ir2_opnd2_em(LISA_MOVFCSR2GR, itemp, fcsr_ir2_opnd);
    /* unmask V if necessary */
    la_append_ir2_opnd3(LISA_BEQ, itemp1, zero_ir2_opnd, label_no_excp2);
    la_append_ir2_opnd2i(LISA_XORI, itemp, itemp, 0x10);
    la_append_ir2_opnd2_em(LISA_MOVGR2FCSR, fcsr_ir2_opnd, itemp);
    la_append_ir2_opnd1(LISA_LABEL, label_no_excp2);

    la_append_ir2_opnd2ii(LISA_BSTRPICK_D, itemp, itemp, 28, 28);
    la_append_ir2_opnd2_em(LISA_MOVFR2GR_S, itemp1, ir2_sign_exp);
    la_append_ir2_opnd2i(LISA_ST_H, itemp1, mem_opnd, mem_imm + 8);
    la_append_ir2_opnd2_em(LISA_MOVFR2GR_D, itemp1, ir2_fraction);
    la_append_ir2_opnd3(LISA_BEQ, itemp, zero_ir2_opnd, label_ok);
    ra_free_temp(ir2_sign_exp);
    ra_free_temp(ir2_fraction);

    //Identify SNAN and write snan to opnd2
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, itemp1, zero_ir2_opnd, 62, 62);

    la_append_ir2_opnd1(LISA_LABEL, label_ok);
    la_append_ir2_opnd2i(LISA_ST_D, itemp1, mem_opnd, mem_imm);

    ra_free_temp(itemp);
    ra_free_temp(itemp1);
}

static void store_freg_to_ir1_mem(IR2_OPND opnd2, IR1_OPND *opnd1,
                                  bool is_xmm_hi, bool is_convert)
{
    IR2_OPND mem_opnd;
    if (is_xmm_hi)
        mem_opnd = convert_mem_opnd_with_bias(opnd1, 8);
    else
        mem_opnd = convert_mem_opnd(opnd1);
    int base_ireg_num = mem_opnd._reg_num;
    int mem_imm = ir2_opnd_imm(&mem_opnd);
    mem_opnd._type = IR2_OPND_IREG; //decouple IR2_OPND_MEM to ireg and imm
    if (ir2_opnd_is_address(&mem_opnd))
        la_append_ir2_opnd3_em(LISA_AND, mem_opnd, mem_opnd, n1_ir2_opnd);

    lsassert(cpu_get_guest_base() == 0);

    if (ir1_opnd_size(opnd1) == 32) {
        IR2_OPND ftemp = ra_alloc_ftemp_internal();
        if (is_convert) {
            la_append_ir2_opnd2_em(LISA_FCVT_S_D, ftemp, opnd2);
            la_append_ir2_opnd2i(LISA_FST_S, ftemp, mem_opnd, mem_imm);
        } else
            la_append_ir2_opnd2i(LISA_FST_S, opnd2, mem_opnd, mem_imm);
        ra_free_temp(ftemp);
    } else if (ir1_opnd_size(opnd1) == 64 || ir1_opnd_size(opnd1) == 128) {
        la_append_ir2_opnd2i(LISA_FST_D, opnd2, mem_opnd, mem_imm);
    } else {
        mem_opnd = ir2_opnd_new2(IR2_OPND_MEM, base_ireg_num, mem_imm);
        store_64_bit_freg_to_ir1_80_bit_mem(opnd2, mem_opnd);
    }
}

static void store_freg_to_ir1_fpr(IR2_OPND opnd2, IR1_OPND *opnd1)
{
    lsassert(ir2_opnd_is_freg(&opnd2));
    IR2_OPND target_opnd = ra_alloc_st(ir1_opnd_base_reg_num(opnd1));
    if (!ir2_opnd_cmp(&opnd2, &target_opnd))
        la_append_ir2_opnd2_em(LISA_FMOV_D, target_opnd, opnd2);
}

/**
@store an ir1 mmx operand
@param the ir2 fp register operand
@param the ir1 mmx operand
*/
static void store_freg_to_ir1_mmx(IR2_OPND opnd2, IR1_OPND *opnd1)
{
    lsassert(ir2_opnd_is_ireg(&opnd2));
    lsassert(ir1_opnd_is_mmx(opnd1));

    // IR2_OPND mmx_opnd = ra_alloc_mmx(opnd1->_reg_num);
    IR2_OPND mmx_opnd = ra_alloc_mmx(ir1_opnd_base_reg_num(opnd1));

    
    if (ir2_opnd_cmp(&opnd2, &mmx_opnd))
        la_append_ir2_opnd2_em(LISA_FMOV_D, mmx_opnd, opnd2);
}

static void store_freg_to_ir1_xmm(IR2_OPND opnd2, IR1_OPND *opnd1,
                                  bool is_xmm_hi)
{
    lsassert(ir2_opnd_is_freg(&opnd2));
    lsassert(ir1_opnd_is_xmm(opnd1));

    IR2_OPND xmm_opnd;
    if (is_xmm_hi)
        xmm_opnd = ra_alloc_xmm_hi(ir1_opnd_base_reg_num(opnd1));
    else
        xmm_opnd = ra_alloc_xmm_lo(ir1_opnd_base_reg_num(opnd1));
    if (!ir2_opnd_cmp(&opnd2, &xmm_opnd))
        la_append_ir2_opnd2_em(LISA_FMOV_D, xmm_opnd, opnd2);
}

void store_freg_to_ir1(IR2_OPND opnd2, IR1_OPND *opnd1, bool is_xmm_hi,
                       bool is_convert)
{
    lsassert(ir2_opnd_is_freg(&opnd2));

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
        return;
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

/* save old fcsr in fcsr_opnd temporary register  for reload , then set fcsr
 * according to x86 MXCSR register */

IR2_OPND set_fpu_fcsr_rounding_field_by_x86(void)
{
    IR2_OPND temp_fcsr = ra_alloc_itemp_internal();
    la_append_ir2_opnd2(LISA_MOVFCSR2GR, temp_fcsr, fcsr_ir2_opnd);
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, temp_fcsr, zero_ir2_opnd, 9, 8);
    /* save fscr in fcsr_opnd for reload */
    IR2_OPND fcsr_opnd = ra_alloc_itemp_internal();
    la_append_ir2_opnd3_em(LISA_OR, fcsr_opnd, temp_fcsr, zero_ir2_opnd);
    /* set fcsr according to x86 MXCSR register */
    IR2_OPND temp_mxcsr = ra_alloc_itemp_internal();
    la_append_ir2_opnd2i_em(LISA_LD_WU, temp_mxcsr, env_ir2_opnd,
        lsenv_offset_of_mxcsr(lsenv));
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, temp_mxcsr, temp_mxcsr, 14, 13);
    IR2_OPND temp_int = ra_alloc_itemp_internal();
    la_append_ir2_opnd2i_em(LISA_ANDI, temp_int, temp_mxcsr, 0x1);
    IR2_OPND label1 = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BEQ, temp_int, zero_ir2_opnd, label1);
    la_append_ir2_opnd2i_em(LISA_XORI, temp_mxcsr, temp_mxcsr, 0x2);
    la_append_ir2_opnd1(LISA_LABEL, label1);
    la_append_ir2_opnd2ii(LISA_BSTRINS_W, temp_fcsr, temp_mxcsr, 9, 8);
    la_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr_ir2_opnd, temp_fcsr);
    ra_free_temp(temp_fcsr);
    ra_free_temp(temp_mxcsr);
    ra_free_temp(temp_int);
    return fcsr_opnd;
}

/* reload fcsr according to previously saved fcsr */

void set_fpu_fcsr(IR2_OPND new_fcsr)
{
    la_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr_ir2_opnd, new_fcsr);
}

IR2_OPND convert_mem_opnd_with_bias_within_imm_bits(IR1_OPND *opnd1, int bias, int bits)
{
    IR2_OPND mem_opnd;

    longx offset = ir1_opnd_simm(opnd1) + bias;
    int32_t offset_imm_part;
    longx offset_reg_part;
    IR2_OPND offset_reg_opnd = ra_alloc_itemp();
    if (offset >= -(1 << (bits - 1)) &&
        offset <= ((1 << (bits - 1)) - 1)) { /* minus 7 for the sake of mda process */
        offset_imm_part = offset;
        offset_reg_part = 0;
    } else {
        offset_imm_part = 0;
        offset_reg_part = offset;
        load_ireg_from_imm32(offset_reg_opnd, offset_reg_part, SIGN_EXTENSION);
    }

    // if (offset >= -32768 &&
    //     offset <= 32767 - 7) { /* minus 7 for the sake of mda process */
    //     offset_imm_part = offset;
    //     offset_reg_part = 0;
    // } else {
    //     offset_imm_part = offset;
    //     offset_reg_part = offset - offset_imm_part;
    //     lsassert(((offset_reg_part >> 16) << 16) ==
    //              offset_reg_part); /* make sure that the low 16 bits are */
    //                                /* 0//make sure that the low 16 bits are 0 */
    //     if (offset_imm_part > 32767 - 7) {
    //         offset_imm_part = 0;
    //         offset_reg_part = offset;
    //     }
    //     load_ireg_from_imm32(offset_reg_opnd, offset_reg_part, SIGN_EXTENSION);
    // }

    /* 1. has no index */
    if (!ir1_opnd_has_index(opnd1)) { /* no index and scale */
        if (ir1_opnd_has_base(opnd1)) {
            /* 1.1. base and an optional offset */
            IR2_OPND base_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
            if (offset_reg_part == 0) {
                ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
                                ir2_opnd_base_reg_num(&base_opnd),
                                offset_imm_part);
            } else {
                la_append_ir2_opnd3_em(LISA_ADD_ADDRX, offset_reg_opnd,
                                 offset_reg_opnd, base_opnd);
                ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
                                ir2_opnd_base_reg_num(&offset_reg_opnd),
                                offset_imm_part);
            }
        } else {
            /* 1.2. only an offset */

            if (offset_reg_part == 0) {
                ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM, 0, offset_imm_part);
            } else {
                ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
                                ir2_opnd_base_reg_num(&offset_reg_opnd),
                                offset_imm_part);
            }
        }
    }

    /* 2. base, index, and an optional offset */
    else if (ir1_opnd_has_base(opnd1)) {
        IR2_OPND base_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
        IR2_OPND index_opnd = ra_alloc_gpr(ir1_opnd_index_reg_num(opnd1));
        mem_opnd = ra_alloc_itemp_internal();

        /* 2.1. prepare base + index*scale */
        if (ir1_opnd_scale(opnd1) == 1) {
            la_append_ir2_opnd3_em(LISA_ADD_ADDRX, mem_opnd, base_opnd, index_opnd);
        } else if (ir1_opnd_scale(opnd1) == 2) {
            la_append_ir2_opnd2i_em(LISA_SLLI_W, mem_opnd, index_opnd, 1);
            la_append_ir2_opnd3_em(LISA_ADD_ADDRX, mem_opnd, mem_opnd, base_opnd);
        } else if (ir1_opnd_scale(opnd1) == 4) {
            la_append_ir2_opnd2i_em(LISA_SLLI_W, mem_opnd, index_opnd, 2);
            la_append_ir2_opnd3_em(LISA_ADD_ADDRX, mem_opnd, mem_opnd, base_opnd);
        } else if (ir1_opnd_scale(opnd1) == 8) {
            la_append_ir2_opnd2i_em(LISA_SLLI_W, mem_opnd, index_opnd, 3);
            la_append_ir2_opnd3_em(LISA_ADD_ADDRX, mem_opnd, mem_opnd, base_opnd);
        } else {
            lsassert(0);
        }

        /* 2.2. add offset */
        if (offset_reg_part == 0) {
            ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
                            ir2_opnd_base_reg_num(&mem_opnd), offset_imm_part);
        } else {
            la_append_ir2_opnd3_em(LISA_ADD_ADDRX, offset_reg_opnd, offset_reg_opnd,
                             mem_opnd);
            ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
                            ir2_opnd_base_reg_num(&offset_reg_opnd),
                            offset_imm_part);
        }
    }

    /* 3. index and an optional offset */
    else {
        IR2_OPND index_opnd = ra_alloc_gpr(ir1_opnd_index_reg_num(opnd1));
        mem_opnd = ra_alloc_itemp_internal();

        /* 2.1. prepare index*scale */
        if (ir1_opnd_scale(opnd1) == 1) {
            la_append_ir2_opnd2i_em(LISA_SLLI_W, mem_opnd, index_opnd, 0);
        } else if (ir1_opnd_scale(opnd1) == 2) {
            la_append_ir2_opnd2i_em(LISA_SLLI_W, mem_opnd, index_opnd, 1);
        } else if (ir1_opnd_scale(opnd1) == 4) {
            la_append_ir2_opnd2i_em(LISA_SLLI_W, mem_opnd, index_opnd, 2);
        } else if (ir1_opnd_scale(opnd1) == 8) {
            la_append_ir2_opnd2i_em(LISA_SLLI_W, mem_opnd, index_opnd, 3);
        } else {
            lsassert(0);
        }

        /* 2.2. add offset */
        if (offset_reg_part == 0) {
            ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
                            ir2_opnd_base_reg_num(&mem_opnd), offset_imm_part);
        } else {
            la_append_ir2_opnd3_em(LISA_ADD_ADDRX, offset_reg_opnd, mem_opnd,
                             offset_reg_opnd);
            ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
                            ir2_opnd_base_reg_num(&offset_reg_opnd),
                            offset_imm_part);
        }
    }

    /* 4. segment? */
    if (ir1_opnd_has_seg(opnd1)) {
        //lsassertm(0, "not implemented in %s\n", __FUNCTION__);
        IR2_OPND seg_base_opnd = ra_alloc_itemp_internal();
        la_append_ir2_opnd2i_em(LISA_LOAD_ADDRX, seg_base_opnd, env_ir2_opnd,
            lsenv_offset_of_seg_base(lsenv, ir1_opnd_get_seg_index(opnd1)));

        int old_base_reg_num = ir2_opnd_base_reg_num(&mem_opnd);

        // if (old_base_reg_num != 0)
        //     append_ir2_opnd3(
        //         mips_add_addrx, seg_base_opnd, seg_base_opnd,
        //         ir2_opnd_new(IR2_OPND_IREG, old_base_reg_num)); /* TODO */

        IR2_OPND old_base = ir2_opnd_new(IR2_OPND_IREG, old_base_reg_num);
        ir2_opnd_set_em(&old_base, SIGN_EXTENSION, 32);
        if (old_base_reg_num != 0)
            la_append_ir2_opnd3_em(LISA_ADD_ADDRX, seg_base_opnd, seg_base_opnd,
                old_base);

        ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM,
                        ir2_opnd_base_reg_num(&seg_base_opnd),
                        ir2_opnd_imm(&mem_opnd));
    }

	IR2_OPND mem_base_opnd = ir2_opnd_new(IR2_OPND_IREG, ir2_opnd_base_reg_num(&mem_opnd));
	if (ir2_opnd_is_address(&mem_base_opnd))
        la_append_ir2_opnd3_em(LISA_AND, mem_base_opnd, mem_base_opnd, n1_ir2_opnd);

    ra_free_temp(offset_reg_opnd);
    return mem_opnd;
}

void load_freg128_from_ir1_mem(IR2_OPND opnd2, IR1_OPND *opnd1){
    lsassert(ir1_opnd_is_mem(opnd1));
    lsassert(ir2_opnd_is_freg(&opnd2));

    IR2_OPND mem_ir2 =
        convert_mem_opnd_with_bias_within_imm_bits(opnd1, 0, 12);
    int little_disp = ir2_opnd_imm(&mem_ir2);
    la_append_ir2_opnd2i_em(LISA_VLD, opnd2,
        ir2_opnd_new(IR2_OPND_IREG, ir2_opnd_base_reg_num(&mem_ir2)),
        little_disp);
    return;
}

void store_freg128_to_ir1_mem(IR2_OPND opnd2, IR1_OPND *opnd1){
    lsassert(ir1_opnd_is_mem(opnd1));
    lsassert(ir2_opnd_is_freg(&opnd2));

    IR2_OPND mem_ir2 =
        convert_mem_opnd_with_bias_within_imm_bits(opnd1, 0, 12);
    int little_disp = ir2_opnd_imm(&mem_ir2);
    la_append_ir2_opnd2i(LISA_VST, opnd2,
        ir2_opnd_new(IR2_OPND_IREG, ir2_opnd_base_reg_num(&mem_ir2)),
        little_disp);
    return;
}

IR2_OPND load_freg128_from_ir1(IR1_OPND *opnd1){
    lsassert(ir1_opnd_is_xmm(opnd1) || ir1_opnd_is_mem(opnd1));
    if (ir1_opnd_is_xmm(opnd1)){
        return ra_alloc_xmm(ir1_opnd_base_reg_num(opnd1));
    }
    else if (ir1_opnd_is_mem(opnd1)){
        IR2_OPND ret_opnd = ra_alloc_ftemp();
        if (ir1_opnd_size(opnd1) == 128) {
            load_freg128_from_ir1_mem(ret_opnd, opnd1);
        }
        else {
            load_freg_from_ir1_mem(ret_opnd, opnd1, 0, IS_DEST_MMX);
        }
        return ret_opnd;
    }
    else {
        lsassert(0);
    }
    abort();
}
