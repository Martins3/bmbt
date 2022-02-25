#include "common.h"
#include "reg-alloc.h"
#include "ir2.h"
#include "latx-options.h"
#include "lsenv.h"
#include "translate.h"

#include "sys-excp.h"

void latxs_load_imm64(IR2_OPND *opnd2, int64_t value)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    if (value >> 11 == -1 || value >> 11 == 0) {
        latxs_append_ir2_opnd2i(LISA_ADDI_D, opnd2, zero, value);
    } else if (value >> 12 == 0) {
        latxs_append_ir2_opnd2i(LISA_ORI, opnd2, zero, value);
    } else if (value >> 31 == -1 || value >> 31 == 0) {
        latxs_append_ir2_opnd1i(LISA_LU12I_W, opnd2, value << 32 >> 44);
        if (value & 0xfff) {
            latxs_append_ir2_opnd2i(LISA_ORI, opnd2, opnd2, value & 0xfff);
        }
    } else if (value >> 32 == 0) {
        latxs_append_ir2_opnd1i(LISA_LU12I_W, opnd2, value << 32 >> 44);
        if (value & 0xfff) {
            latxs_append_ir2_opnd2i(LISA_ORI, opnd2, opnd2, value & 0xfff);
        }
        latxs_append_ir2_opnd1i(LISA_LU32I_D, opnd2, 0);
    } else if (value >> 51 == -1 || value >> 51 == 0) {
        if ((value & 0xffffffff) == 0) {
            latxs_append_ir2_opnd3(LISA_OR, opnd2, zero, zero);
        } else if ((value & 0xfff) == 0) {
            latxs_append_ir2_opnd1i(LISA_LU12I_W, opnd2, value << 32 >> 44);
        } else {
            latxs_append_ir2_opnd1i(LISA_LU12I_W, opnd2, value << 32 >> 44);
            latxs_append_ir2_opnd2i(LISA_ORI, opnd2, opnd2, value & 0xfff);
        }
        latxs_append_ir2_opnd1i(LISA_LU32I_D, opnd2, value << 12 >> 44);
    } else {
        if ((value & 0xfffffffffffff) == 0) {
            latxs_append_ir2_opnd2i(LISA_LU52I_D, opnd2,
                    zero, value >> 52);
        } else if ((value & 0xffffffff) == 0) {
            latxs_append_ir2_opnd3(LISA_OR, opnd2, zero, zero);
            latxs_append_ir2_opnd1i(LISA_LU32I_D, opnd2, value << 12 >> 44);
            latxs_append_ir2_opnd2i(LISA_LU52I_D,
                    opnd2, opnd2, value >> 52);
        } else if ((value & 0xfff) == 0) {
            latxs_append_ir2_opnd1i(LISA_LU12I_W, opnd2, value << 32 >> 44);
            latxs_append_ir2_opnd1i(LISA_LU32I_D, opnd2, value << 12 >> 44);
            latxs_append_ir2_opnd2i(LISA_LU52I_D,
                    opnd2, opnd2, value >> 52);
        } else {
            latxs_append_ir2_opnd1i(LISA_LU12I_W, opnd2, value << 32 >> 44);
            latxs_append_ir2_opnd2i(LISA_ORI, opnd2, opnd2, value & 0xfff);
            latxs_append_ir2_opnd1i(LISA_LU32I_D, opnd2, value << 12 >> 44);
            latxs_append_ir2_opnd2i(LISA_LU52I_D,
                    opnd2, opnd2, value >> 52);
        }
    }
}

void latxs_load_imm32_to_ir2(IR2_OPND *opnd2, uint32_t value, EXMode em)
{
    lsassertm(latxs_ir2_opnd_is_gpr(opnd2),
            "load imm 32 to ir2: IR2 OPND is not GPR\n");
    latxs_load_imm64(opnd2, value);
}

/* TODO: remove this function, work was done by latxs_load_imm64*/
void latxs_load_imm64_to_ir2(IR2_OPND *opnd2, uint64_t value)
{
    lsassertm(latxs_ir2_opnd_is_gpr(opnd2),
            "load imm 64 to ir2: IR2 OPND is not GPR\n");

    int32 high_32_bits = value >> 32;
    int32 low_32_bits = value;

    if (high_32_bits == 0) {
        /* 1. 0000 0000 xxxx xxxx */
        latxs_load_imm32_to_ir2(opnd2, low_32_bits, EXMode_Z);
        return;
    } else if (high_32_bits == -1) {
        /* 2. ffff ffff xxxx xxxx */
        if (low_32_bits < 0) {
            latxs_load_imm64(opnd2, value);
            return;
        } else {
            latxs_load_imm32_to_ir2(opnd2, ~low_32_bits, EXMode_Z);
            latxs_append_ir2_opnd3(LISA_NOR, opnd2, opnd2, opnd2);
            return;
        }
    } else {
        /* 3. xxxx xxxx xxxx xxxx */
        latxs_load_imm64(opnd2, value);
        return;
    }
}

void latxs_load_addrx_to_ir2(IR2_OPND *opnd, ADDRX addrx)
{
#ifdef TARGET_X86_64
    if (latxs_ir1_addr_size(lsenv->tr_data->curr_ir1_inst) == 8) {
        latxs_load_imm64_to_ir2(opnd, addrx);
    } else {
        latxs_load_imm32_to_ir2(opnd, (uint32_t)addrx, EXMode_Z);
    }
#else
    latxs_load_imm32_to_ir2(opnd, (uint32_t)addrx, EXMode_Z);
#endif
}

void latxs_load_addr_to_ir2(IR2_OPND *opnd, ADDR addr)
{
    latxs_load_imm64_to_ir2(opnd, (uint64_t)addr);
}

static IR2_OPND latxs_convert_mem_ir2_opnd_plus_offset(
        IR2_OPND *mem, int addend)
{
    lsassert(latxs_ir2_opnd_is_mem(mem));
    IR2_OPND mem_opnd = *mem;

    int mem_offset = latxs_ir2_opnd_imm(mem);

    /* addend = 2 : 0x7fe */
    /* addend = 4 : 0x7fc */
    int mem_offset_high = ((int)0x800) - addend;

    /* situation 1: >> directly adjust mem_offset in IR2_OPND */
    /* situation 2: >> directly adjust base register in IR2_OPND */
    /* situation 3: >> use a new temp register as new base */
    if (likely(mem_offset < mem_offset_high)) {
        /* situation 1 */
        latxs_ir2_opnd_mem_adjust_offset(&mem_opnd, addend);
    } else {
        IR2_OPND mem_old_base = latxs_ir2_opnd_mem_get_base(mem);
        if (latxs_ir2_opnd_is_itemp(&mem_old_base)) {
            /* situation 2 */
            latxs_append_ir2_opnd2i(LISA_ADDI_D, &mem_old_base,
                    &mem_old_base, addend);
        } else {
            /* situation 3 */
            IR2_OPND mem_new_base = latxs_ra_alloc_itemp();
            latxs_append_ir2_opnd2i(LISA_ADDI_D, &mem_new_base,
                    &mem_old_base, addend);
            latxs_ir2_opnd_mem_set_base(&mem_opnd, &mem_new_base);
        }

    }

    return mem_opnd;
}

IR2_OPND latxs_convert_mem_ir2_opnd_plus_2(IR2_OPND *mem)
{
    return latxs_convert_mem_ir2_opnd_plus_offset(mem, 2);
}
IR2_OPND latxs_convert_mem_ir2_opnd_plus_4(IR2_OPND *mem)
{
    return latxs_convert_mem_ir2_opnd_plus_offset(mem, 4);
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
IR2_OPND latxs_convert_mem_ir2_opnd_no_offset(IR2_OPND *mem, int *newtmp)
{
    lsassert(latxs_ir2_opnd_is_mem(mem));
    IR2_OPND mem_opnd;

    int mem_offset = latxs_ir2_opnd_mem_get_offset(mem);

    IR2_OPND mem_old_base = latxs_ir2_opnd_mem_get_base(mem);
    IR2_OPND mem_new_base;

    if (mem_offset) {
        mem_new_base = latxs_ra_alloc_itemp();
        *newtmp = 1;
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &mem_new_base,
                &mem_old_base, mem_offset);
    } else {
        if (latxs_ir2_opnd_is_itemp(&mem_old_base)) {
            *newtmp = 0;
            mem_new_base = mem_old_base;
        } else {
            mem_new_base = latxs_ra_alloc_itemp();
            *newtmp = 1;
            latxs_append_ir2_opnd3(LISA_OR, &mem_new_base,
                    &mem_old_base, &latxs_zero_ir2_opnd);
        }
    }

    latxs_ir2_opnd_build_mem(&mem_opnd,
            latxs_ir2_opnd_reg(&mem_new_base), /* base   */
            0);                                /* offset */

    return mem_opnd;
}

/*
 * Calculate the memory address value of IR1_OPND to IR2_OPND
 * Effective Address = disp + [base + index << scale]
 *
 * @value_opnd : IR2_OPND_IREG
 * @opnd1 : IR1_OPND_MEM
 */
void latxs_load_ir1_mem_addr_to_ir2(IR2_OPND *value_opnd,
        IR1_OPND *opnd1, int addr_size)
{
    IR2_OPND ea = latxs_ra_alloc_itemp();
    int ea_valid = 0;
    int ea_base_only = 0;
    IR2_OPND ea_base;

#ifdef TARGET_X86_64
    /* x86_64 : RIP/EIP relative addressing */
    if (ir1_opnd_is_pc_relative(opnd1)) {
        /* offset = next_IP + offset */
        int64_t offset =
            ir1_addr_next(lsenv->tr_data->curr_ir1_inst) + ir1_opnd_simm(opnd1);
        latxs_load_imm64_to_ir2(value_opnd, offset);
        return;
    }
#endif

    /* 1. ea = disp + [base + index << scale] */
    if (ir1_opnd_has_index(opnd1)) {
        /* 1.1 ea = base + index << scale */
        int ir1_reg_index = ir1_opnd_index_reg_num(opnd1);
        IR2_OPND index = latxs_ra_alloc_gpr(ir1_reg_index);
        switch (ir1_opnd_scale(opnd1)) {
        case 1:
            latxs_append_ir2_opnd2_(lisa_mov, &ea, &index);
            break;
        case 2:
            latxs_append_ir2_opnd2i(LISA_SLLI_D, &ea, &index, 1);
            break;
        case 4:
            latxs_append_ir2_opnd2i(LISA_SLLI_D, &ea, &index, 2);
            break;
        case 8:
            latxs_append_ir2_opnd2i(LISA_SLLI_D, &ea, &index, 3);
            break;
        default:
            lsassertm(0, "unknown scale %d of ir1 opnd.\n",
                    ir1_opnd_scale(opnd1));
            break;
        }
        if (ir1_opnd_has_base(opnd1)) {
            int ir1_reg_base = ir1_opnd_base_reg_num(opnd1);
            IR2_OPND base = latxs_ra_alloc_gpr(ir1_reg_base);
            latxs_append_ir2_opnd3(LISA_ADD_D, &ea, &ea, &base);
        }
        ea_valid = 1;
    } else if (ir1_opnd_has_base(opnd1)) {
        /* 1.2 ea = base */
        int ir1_reg_base = ir1_opnd_base_reg_num(opnd1);
        ea_base = latxs_ra_alloc_gpr(ir1_reg_base);
        ea_base_only = 1;
        ea_valid = 1;
    }

    if (!ea_valid) {
        /* 1.3 ea = disp */
        longx offset = ir1_opnd_simm(opnd1);
        latxs_load_imm64_to_ir2(&ea, offset);
    } else if (ir1_opnd_simm(opnd1) != 0) {
        /* 1.4 ea = disp + ea */
        longx offset = ir1_opnd_simm(opnd1);
        if (int32_in_int12(offset)) {
            latxs_append_ir2_opnd2i(LISA_ADDI_D, &ea,
                    ea_base_only ? &ea_base : &ea, offset);
        } else {
            IR2_OPND offset_opnd = latxs_ra_alloc_itemp();
            latxs_load_imm64_to_ir2(&offset_opnd, offset);
            latxs_append_ir2_opnd3(LISA_ADD_D, &ea,
                    ea_base_only ? &ea_base : &ea, &offset_opnd);
            latxs_ra_free_temp(&offset_opnd);
        }
        ea_base_only = 0;
    } else if (ea_base_only) {
        latxs_append_ir2_opnd2_(lisa_mov, &ea, &ea_base);
        ea_base_only = 0;
    }

    /* 2. apply address size */
    switch (addr_size) {
    case 2:
        latxs_append_ir2_opnd2_(lisa_mov16z, &ea, &ea);
        break;
    case 4:
        latxs_append_ir2_opnd2_(lisa_mov32z, &ea, &ea);
        break;
#ifdef TARGET_X86_64
    case 8:
        break;
#endif
    default:
        lsassertm(0, "unknown addr size %d in convert mem opnd.\n", addr_size);
        break;
    }

    /* 3. move to value */
    latxs_append_ir2_opnd2_(lisa_mov, value_opnd, &ea);
}

void latxs_convert_mem_opnd(IR2_OPND *opnd2,
        IR1_OPND *opnd1, int addr_size)
{
    if (addr_size < 0) {
        addr_size = latxs_ir1_addr_size(lsenv->tr_data->curr_ir1_inst);
    }
    latxs_convert_mem_opnd_with_bias(opnd2, opnd1, 0, addr_size);
}

/*
 * Convert IR1_OPND_MEM to IR2_OPND_REG without offset
 */
void latxs_convert_mem_opnd_no_offset(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    IR2_OPND mem_opnd;
    int addr_size = latxs_ir1_addr_size(lsenv->tr_data->curr_ir1_inst);
    latxs_convert_mem_opnd_with_bias(&mem_opnd, opnd1, 0, addr_size);

    lsassert(latxs_ir2_opnd_is_mem(&mem_opnd));

    int mem_offset = latxs_ir2_opnd_mem_get_offset(&mem_opnd);

    IR2_OPND mem_old_base = latxs_ir2_opnd_mem_get_base(&mem_opnd);

    latxs_append_ir2_opnd2i(LISA_ADDI_D, opnd2, &mem_old_base, mem_offset);
}

/*
 * Convert IR1_OPND_MEM to IR2_OPND_MEM
 * > Temp register might be used.
 * > The IR2_OPND_MEM's base will always be temp register !!!
 *
 * @opnd1 : IR1_OPND_MEM
 * @bias  : bias will be plused on offset
 */
void latxs_convert_mem_opnd_with_bias(IR2_OPND *opnd2,
        IR1_OPND *opnd1, int bias, int addr_size)
{
    if (addr_size < 0) {
        addr_size = latxs_ir1_addr_size(lsenv->tr_data->curr_ir1_inst);
    }

    IR2_OPND ea = latxs_ra_alloc_itemp();
    int ea_valid = 0;
    int ea_base_only = 0;
    IR2_OPND ea_base;

    int ea_ir1_reg = 0;
    int ea_off = 0;

#ifdef TARGET_X86_64
    /* x86_64 : RIP/EIP relative addressing */
    if (ir1_opnd_is_pc_relative(opnd1)) {
        /* offset = next_IP + offset */
        int64_t offset = ir1_addr_next(lsenv->tr_data->curr_ir1_inst) +
                         ir1_opnd_simm(opnd1) + bias;
        latxs_load_imm64_to_ir2(&ea, offset);

        if (ir1_opnd_has_seg(opnd1)) {
            int seg_reg = ir1_opnd_get_seg_index(opnd1);
            if (seg_reg != gs_index && seg_reg != fs_index) {
                lsassert(!lsenv->tr_data->sys.addseg);
            } else {
                IR2_OPND seg_base = latxs_ra_alloc_itemp();
                /* TODO */
                latxs_append_ir2_opnd2i(LISA_LD_D, &seg_base,
                        &latxs_env_ir2_opnd,
                        lsenv_offset_of_seg_base(lsenv, seg_reg));

                latxs_append_ir2_opnd3(LISA_ADD_D, &ea, &ea, &seg_base);
                latxs_ra_free_temp(&seg_base);
            }
        }

        /* construct IR2_OPND_MEM */
        int mem_base = latxs_ir2_opnd_reg(&ea);
        latxs_ir2_opnd_build_mem(opnd2, mem_base, 0);
        return;
    }
#endif


    /* 1. ea = disp + [base + index << scale] */
    if (ir1_opnd_has_index(opnd1)) {
        /* 1.1 ea = base + index << scale */
        ea_ir1_reg = ir1_opnd_index_reg_num(opnd1);
        IR2_OPND index = latxs_ra_alloc_gpr(ea_ir1_reg);
        switch (ir1_opnd_scale(opnd1)) {
        case 1:
            latxs_append_ir2_opnd2_(lisa_mov, &ea, &index);
            break;
        case 2:
            latxs_append_ir2_opnd2i(LISA_SLLI_D, &ea, &index, 1);
            break;
        case 4:
            latxs_append_ir2_opnd2i(LISA_SLLI_D, &ea, &index, 2);
            break;
        case 8:
            latxs_append_ir2_opnd2i(LISA_SLLI_D, &ea, &index, 3);
            break;
        default:
            lsassertm(0, "unknown scale %d of ir1 opnd.\n",
                    ir1_opnd_scale(opnd1));
            break;
        }
        if (ir1_opnd_has_base(opnd1)) {
            IR2_OPND base = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
            latxs_append_ir2_opnd3(LISA_ADD_D, &ea, &ea, &base);
        }
        ea_valid = 1;
    } else if (ir1_opnd_has_base(opnd1)) {
        /* 1.2 ea = base */
        ea_ir1_reg = ir1_opnd_base_reg_num(opnd1);
        ea_base = latxs_ra_alloc_gpr(ea_ir1_reg);
        ea_base_only = 1;
        ea_valid = 1;
    }

    if (!ea_valid) {
        /* 1.3 ea = disp */
        longx offset = ir1_opnd_simm(opnd1) + bias;
        latxs_load_imm64_to_ir2(&ea, offset);
    } else if (ir1_opnd_simm(opnd1) != 0 || bias != 0) {
        /* 1.4 ea = disp + ea */
        longx offset = ir1_opnd_simm(opnd1) + bias;
        /* TARGET_X86_64 int32 is ok */
        if (int32_in_int12(offset)) {
            /* allow to do risk optimization */
            /* if (xtm_risk_opt() && TODO */
                    /* ea_ir1_reg != esp_index && */
                    /* ea_ir1_reg != ebp_index) { */
                /* ea_off = offset; */
            /* } else { */
            latxs_append_ir2_opnd2i(LISA_ADDI_D, &ea,
                    ea_base_only ? &ea_base : &ea, offset);
            ea_base_only = 0;
            /* } */
        } else {
            IR2_OPND offset_opnd = latxs_ra_alloc_itemp();
            latxs_load_imm64_to_ir2(&offset_opnd, offset);
            latxs_append_ir2_opnd3(LISA_ADD_D, &ea,
                    ea_base_only ? &ea_base : &ea, &offset_opnd);
            latxs_ra_free_temp(&offset_opnd);
            ea_base_only = 0;
        }
    }

    /*
     * At here, 3 situations:
     * > ea_base_only = 1 with no ea_off
     * > ea_base_only = 0 with no ea_off
     * > ea_base_only = 1 with ea_off (risky)
     */

    /* 1.1 fix pop (esp) address */
    TRANSLATION_DATA *td = lsenv->tr_data;
    if (td->sys.popl_esp_hack) {
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &ea,
                ea_base_only ? &ea_base : &ea,
                td->sys.popl_esp_hack);
        ea_base_only = 0;
    }

    /* 2.0 apply address size */
    switch (addr_size) {
    case 2:
        latxs_append_ir2_opnd2_(lisa_mov16z, &ea,
                ea_base_only ? &ea_base : &ea);
        break;
    case 4:
        latxs_append_ir2_opnd2_(lisa_mov32z, &ea,
                ea_base_only ? &ea_base : &ea);
        break;
#ifdef TARGET_X86_64
    case 8:
        latxs_append_ir2_opnd2_(lisa_mov, &ea,
                ea_base_only ? &ea_base : &ea);
        break;
#endif
    default:
        lsassertm(0, "unknown addr size %d in %s.\n",
                addr_size, __func__);
        break;
    }
    ea_base_only = 0;

    /* 2.1 apply segment base */
    int seg_num = -1;
    if (ir1_opnd_has_seg(opnd1)) {
        seg_num = ir1_opnd_get_seg_index(opnd1);
    }
    if (seg_num >= 0 &&
        ((seg_num == ss_index ||
         seg_num == ds_index ||
         seg_num == es_index) ? td->sys.addseg : 1))
    {
        /* 2.1 need segment base */
        IR2_OPND seg_base = latxs_ra_alloc_itemp();
        int seg_reg = ir1_opnd_get_seg_index(opnd1);
#ifdef TARGET_X86_64
        latxs_append_ir2_opnd2i(LISA_LD_D, &seg_base,
                &latxs_env_ir2_opnd,
                lsenv_offset_of_seg_base(lsenv, seg_reg));
#else
        latxs_append_ir2_opnd2i(LISA_LD_WU, &seg_base,
                &latxs_env_ir2_opnd,
                lsenv_offset_of_seg_base(lsenv, seg_reg));
#endif

        latxs_append_ir2_opnd3(LISA_ADD_D, &ea, &ea, &seg_base);
        latxs_ra_free_temp(&seg_base);
#ifdef TARGET_X86_64
        if (addr_size != 8) {
            latxs_append_ir2_opnd2_(lisa_mov32z, &ea, &ea);
        }
#else
        latxs_append_ir2_opnd2_(lisa_mov32z, &ea, &ea);
#endif
    }

    /* 3. construct IR2_OPND_MEM */
    int mem_base = latxs_ir2_opnd_reg(&ea);
    latxs_ir2_opnd_build_mem(opnd2, mem_base, ea_off);
}

void latxs_load_ir1_imm_to_ir2(IR2_OPND *opnd2,
        IR1_OPND *opnd1, EXMode em)
{
    lsassert(latxs_ir2_opnd_is_gpr(opnd2));
    lsassert(ir1_opnd_is_imm(opnd1));

    uint32_t value = 0;
    if (em == EXMode_Z) {
        value = ir1_opnd_uimm(opnd1);
    } else {
        value = ir1_opnd_simm(opnd1);
    }
#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        latxs_load_imm64_to_ir2(opnd2, ir1_opnd_simm(opnd1));
    } else {
        latxs_load_imm32_to_ir2(opnd2, value, em);
    }
#else
    latxs_load_imm32_to_ir2(opnd2, value, em);
#endif
}

void latxs_load_ir1_seg_to_ir2(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    lsassert(latxs_ir2_opnd_is_gpr(opnd2));
    lsassert(ir1_opnd_is_seg(opnd1));
    int seg_num = ir1_opnd_base_reg_num(opnd1);

    TRANSLATION_DATA *td = lsenv->tr_data;
    IR1_INST *pir1 = td->curr_ir1_inst;
    if (ir1_opcode(pir1) == X86_INS_MOV) {
        CHECK_EXCP_MOV_FROM_SEG(pir1, seg_num);
    }

    latxs_append_ir2_opnd2i(LISA_LD_HU, opnd2,
            &latxs_env_ir2_opnd,
            lsenv_offset_of_seg_selector(lsenv, seg_num));

    if (option_by_hand) {
        latxs_ir2_opnd_set_emb(opnd2, EXMode_Z, 16);
    }
}

/*
 * Load data from the memory specified by IR1_OPND_MEM to IR2_OPND_IREG
 * > If you want to obtain the memory addrss, use conver_mem_opnd
 * > Softmmu helper might be called if QEMU TLB miss in system-mode
 *
 * TODO remove xmm hi
 */
void latxs_load_ir1_mem_to_ir2(IR2_OPND *opnd2,
        IR1_OPND *opnd1, EXMode em, int addr_size)
{
    lsassert(latxs_ir2_opnd_is_gpr(opnd2));
    lsassert(ir1_opnd_is_mem(opnd1));

    if (addr_size < 0) {
        /* Use instruction's default address size */
        addr_size = latxs_ir1_addr_size(lsenv->tr_data->curr_ir1_inst);
    }

    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd1, addr_size);

    IR2_OPCODE mem_opcode = LISA_INVALID;

    bool is_zero = em == EXMode_Z;

    int opnd_size = ir1_opnd_size(opnd1);
    switch (opnd_size) {
    case 128:
        mem_opcode = LISA_LD_D;
        break;
    case 64:
        mem_opcode = LISA_LD_D;
        break;
    case 32:
        mem_opcode = is_zero ? LISA_LD_WU : LISA_LD_W;
        break;
    case 16:
        mem_opcode = is_zero ? LISA_LD_HU : LISA_LD_H;
        break;
    case 8:
        mem_opcode = is_zero ? LISA_LD_BU : LISA_LD_B;
        break;
    default:
        lsassertm(0, "load ir1 mem to ir2 should not reach here.\n");
        break;
    }

    gen_ldst_softmmu_helper(mem_opcode, opnd2, &mem_opnd, 1);

    if (option_by_hand) {
        latxs_ir2_opnd_set_emb(opnd2, em, opnd_size);
    }

    latxs_ra_free_temp(&mem_opnd);
}

/*
 * Load the data in IR1_OPND_GPR to IR2_OPND_IREG
 * > Unlike convert_gpr_opnd, we put the data to the given IR2_OPND_IREG here
 *   > The given IR2_OPND_IREG should not be mapping register
 *   > Always use IR1_OPND to operate the mapping registers
 * > Convert according to IR1_OPND's attribute
 *   eg. AL/AH  8-bit to 64-bit
 *       AX    16-bit to 64-bit
 *       EAX   32-bit to 64-bit
 * > Extend according to EXTENSION_MODE (sign or unsign)
 */
void latxs_load_ir1_gpr_to_ir2(IR2_OPND *opnd2,
        IR1_OPND *opnd1, EXMode em)
{
    lsassert(latxs_ir2_opnd_is_gpr(opnd2));
    lsassert(ir1_opnd_is_gpr(opnd1));

    int gpr_num = ir1_opnd_base_reg_num(opnd1);
    IR2_OPND gpr_opnd = latxs_ra_alloc_gpr(gpr_num);

    int opnd_size = ir1_opnd_size(opnd1);

    if (option_by_hand &&
        latxs_ir2_opnd_cmp(opnd2, &gpr_opnd) &&
        em == latxs_td_get_reg_extm(gpr_num)) {
        return;
    }

    /* 1. 32 bits gpr needs SHIFT operation to handle the extension mode */
    if (opnd_size == 32) {
        if (em == EXMode_S) {
            latxs_append_ir2_opnd2ii(LISA_BSTRPICK_W,
                    opnd2, &gpr_opnd, 31, 0);
        } else {
            latxs_append_ir2_opnd2ii(LISA_BSTRPICK_D,
                    opnd2, &gpr_opnd, 31, 0);
        }
    } else if (opnd_size == 64) {
        /* 2. 64 bits gpr needs no extension */
        latxs_append_ir2_opnd2_(lisa_mov, opnd2, &gpr_opnd);
    } else if (opnd_size == 16) {
        /* 3. 16 bits gpr */
        if (em == EXMode_S) {
            latxs_append_ir2_opnd2(LISA_EXT_W_H, opnd2, &gpr_opnd);
        } else {
            latxs_append_ir2_opnd2ii(LISA_BSTRPICK_D,
                    opnd2, &gpr_opnd, 15, 0);
        }
    } else if (ir1_opnd_is_8h(opnd1)) {
        /* 4. 8 bits gpr high */
        if (em == EXMode_S) {
            latxs_append_ir2_opnd2i(LISA_SRLI_D, opnd2, &gpr_opnd, 8);
            latxs_append_ir2_opnd2(LISA_EXT_W_B, opnd2, opnd2);
        } else {
            latxs_append_ir2_opnd2ii(LISA_BSTRPICK_D,
                    opnd2, &gpr_opnd, 15, 8);
        }
    } else {
        /* 5. 8 bits gpr low */
        if (em == EXMode_S) {
            latxs_append_ir2_opnd2(LISA_EXT_W_B, opnd2, &gpr_opnd);
        } else {
            latxs_append_ir2_opnd2ii(LISA_BSTRPICK_D,
                    opnd2, &gpr_opnd, 7, 0);
        }
    }

    if (option_by_hand) {
        latxs_ir2_opnd_set_emb(opnd2, em, opnd_size);
    }
}

void latxs_load_ir1_mmx_to_ir2(IR2_OPND *opnd2,
        IR1_OPND *opnd1, EXMode em)
{
    lsassertm(0, "OPND: load ir1 mmx to be implemented in LoongArch.\n");
}

void latxs_load_ir1_xmm_to_ir2(IR2_OPND *opnd2,
        IR1_OPND *opnd1, EXMode em)
{
    lsassertm(0, "OPND: load ir1 xmm to be implemented in LoongArch.\n");
}

/*
 * This function should be used ONLY in 'mov from control register
 * Previous chechking guarantees that opnd1 is control register
 */
void latxs_load_ir1_cr_to_ir2(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    IR1_INST *pir1 = td->curr_ir1_inst;
    CHECK_EXCP_MOV_CR(pir1);

    int flag = 0;
    int crn = latxs_ir1_opnd_get_cr_num(opnd1, &flag);
    if (!flag) {
        latxs_tr_gen_excp_illegal_op(pir1, 1);
        return;
    }

    lsassert(latxs_ir2_opnd_is_gpr(opnd2));

    /* 0. save current instruciton's EIP to env */
    latxs_tr_gen_save_curr_eip();

    /* 1. call helper_read_crN */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /*
     * target/i386/misc_helper.c
     *
     * target_ulong helper_read_crN(CPUX86State *env, int reg)
     *
     * arg0 : CPUX86State *env
     * arg1 : control register number
     */

    /* arg0 : env */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);

    /* arg1 : crn */
    latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg1_ir2_opnd,
            &latxs_zero_ir2_opnd, crn);

    latxs_tr_gen_call_to_helper((ADDR)helper_read_crN);

    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* move $v0 to the correct destination */
    latxs_append_ir2_opnd3(LISA_OR, opnd2,
            &latxs_ret0_ir2_opnd, &latxs_zero_ir2_opnd);

    if (option_by_hand) {
        latxs_ir2_opnd_set_emb(opnd2, EXMode_Z, 32);
    }
}

/*
 * This function should be used ONLY in 'mov to control register
 * Previous chechking guarantees that opnd1 is control register
 */
void latxs_store_ir2_to_ir1_cr(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    IR1_INST *pir1 = td->curr_ir1_inst;
    CHECK_EXCP_MOV_CR(pir1);

    int flag = 0;
    int crn = latxs_ir1_opnd_get_cr_num(opnd1, &flag);
    if (!flag) {
        latxs_tr_gen_excp_illegal_op(pir1, 1);
        return;
    }

    lsassert(latxs_ir2_opnd_is_gpr(opnd2));

    /* 0. save current instruciton's EIP to env */
    latxs_tr_gen_save_curr_eip();

    /* 1. call helper_write_crN */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /*
     * target/i386/misc_helper.c
     *
     * void helper_write_crN(CPUX86State *env, int reg, target_ulong t0)
     *
     * arg0 : CPUX86State *env
     * arg1 : control register number
     * arg2 : data to store
     */

    /* 0. arg3: data to store */
    /* opnd2 might be temp regsiter, so do this fitst */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg2_ir2_opnd,
            opnd2, &latxs_zero_ir2_opnd);

    /* 1. arg1: env */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);

    /* 2. arg2: control register number */
    latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg1_ir2_opnd,
            &latxs_zero_ir2_opnd, crn);

    latxs_tr_gen_call_to_helper((ADDR)helper_write_crN);

    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
}

/*
 * This function should be used ONLY in 'mov from debug register
 * Previous chechking guarantees that opnd1 is debug register
 */
void latxs_load_ir1_dr_to_ir2(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    IR1_INST *pir1 = td->curr_ir1_inst;
    CHECK_EXCP_MOV_DR(pir1);

    lsassert(latxs_ir2_opnd_is_gpr(opnd2));

    /* 1. save context */
    helper_cfg_t cfg = default_helper_cfg;
    latxs_tr_gen_call_to_helper_prologue_cfg(cfg);

    /*
     * 2. call helper_get_dr
     *
     * target/i386/bpt_helper.c
     * target_ulong helper_get_dr(
     *      CPUX86State *env,
     *      int         reg)
     * >> might generate exception
     */

    /* 2.1 arg0 : env */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);

    /* 2.2 arg1 : drn */
    int drn = latxs_ir1_opnd_get_dr_num(opnd1);
    latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg1_ir2_opnd,
            &latxs_zero_ir2_opnd, drn);

    /* 2.3 call helper */
    latxs_tr_gen_call_to_helper((ADDR)helper_get_dr);

    /* 3. restore context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(cfg);

    /* 4. move $v0 to the correct destination */
    latxs_append_ir2_opnd3(LISA_OR, opnd2,
            &latxs_ret0_ir2_opnd, &latxs_zero_ir2_opnd);

    if (option_by_hand) {
        latxs_ir2_opnd_set_emb(opnd2, EXMode_Z, 32);
    }
}

/*
 * This function should be used ONLY in 'mov to debug register
 * Previous chechking guarantees that opnd1 is debug register
 */
void latxs_store_ir2_to_ir1_dr(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    IR1_INST *pir1 = td->curr_ir1_inst;
    CHECK_EXCP_MOV_DR(pir1);

    lsassert(latxs_ir2_opnd_is_gpr(opnd2));

    /* 1. save context */
    helper_cfg_t cfg = default_helper_cfg;
    latxs_tr_gen_call_to_helper_prologue_cfg(cfg);

    /*
     * 2. call helper_set_dr
     *
     * target/i386/bpt_helper.c
     * void helper_set_dr(
     *      CPUX86State *env,
     *      int reg,
     *      target_ulong t0)
     * >> might generate exception
     */

    /* 2.1. arg2: data to store */
    /* opnd2 might be temp regsiter, so do this fitst */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg2_ir2_opnd,
            opnd2, &latxs_zero_ir2_opnd);

    /* 2.2. arg0: env */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);

    /* 2.3 arg1: debug register number */
    int drn = latxs_ir1_opnd_get_dr_num(opnd1);
    latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg1_ir2_opnd,
            &latxs_zero_ir2_opnd, drn);

    /* 2.4 call helper */
    latxs_tr_gen_call_to_helper((ADDR)helper_set_dr);

    /* 3. restore context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(cfg);
}

/*
 * Load the value from IR1_OPND to IR2_OPND
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
void latxs_load_ir1_to_ir2(IR2_OPND *opnd2,
        IR1_OPND *opnd1, EXMode em)
{
    lsassert(latxs_ir2_opnd_is_gpr(opnd2));

    switch (ir1_opnd_type(opnd1)) {
    case X86_OP_IMM:
        latxs_load_ir1_imm_to_ir2(opnd2, opnd1, em);
        break;
    case X86_OP_REG:  {
        if (ir1_opnd_is_gpr(opnd1)) {
            latxs_load_ir1_gpr_to_ir2(opnd2, opnd1, em);
            break;
        } else if (ir1_opnd_is_seg(opnd1)) {
            latxs_load_ir1_seg_to_ir2(opnd2, opnd1);
            break;
        } else if (latxs_ir1_opnd_is_cr(opnd1)) {
            latxs_load_ir1_cr_to_ir2(opnd2, opnd1);
            break;
        } else if (latxs_ir1_opnd_is_dr(opnd1)) {
            latxs_load_ir1_dr_to_ir2(opnd2, opnd1);
            break;
        } else if (ir1_opnd_is_mmx(opnd1)) {
            latxs_load_ir1_mmx_to_ir2(opnd2, opnd1, em);
            break;
        } else if (ir1_opnd_is_xmm(opnd1)) {
            latxs_load_ir1_xmm_to_ir2(opnd2, opnd1, em);
            break;
        } else {
            lsassert(0);
        }
        break;
    }
    case X86_OP_MEM:
        latxs_load_ir1_mem_to_ir2(opnd2, opnd1, em, -1);
        break;
    default:
        lsassert(0);
        break;
    }

    return;
}

void latxs_load_eflags_cf_to_ir2(IR2_OPND *opnd2)
{
    lsassert(latxs_ir2_opnd_is_gpr(opnd2));

    latxs_append_ir2_opnd1i(LISA_X86MFFLAG, opnd2, 0x1);

    if (option_by_hand) {
        latxs_ir2_opnd_set_emb(opnd2, EXMode_Z, 8);
    }

    return;
}

void latxs_store_ir2_to_ir1_gpr(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    if (option_by_hand) {
        latxs_store_ir2_to_ir1_gpr_em(opnd2, opnd1);
        return;
    }

    lsassert(ir1_opnd_is_gpr(opnd1) && latxs_ir2_opnd_is_gpr(opnd2));

    int gpr_num = ir1_opnd_base_reg_num(opnd1);
    IR2_OPND gpr_opnd = latxs_ra_alloc_gpr(gpr_num);

    /* 1. 32 bits gpr needs SHIFT operation to handle the extension mode */
    if (ir1_opnd_size(opnd1) == 32) {
#ifdef TARGET_X86_64
        latxs_append_ir2_opnd2_(lisa_mov32z, &gpr_opnd, opnd2);
#else
        if (!latxs_ir2_opnd_cmp(opnd2, &gpr_opnd)) {
            latxs_append_ir2_opnd3(LISA_OR, &gpr_opnd,
                    opnd2, &latxs_zero_ir2_opnd);
        }
#endif
    } else if (ir1_opnd_size(opnd1) == 64) {
        if (!latxs_ir2_opnd_cmp(opnd2, &gpr_opnd)) {
            latxs_append_ir2_opnd3(LISA_OR, &gpr_opnd,
                    opnd2, &latxs_zero_ir2_opnd);
        }
    } else if (ir1_opnd_size(opnd1) == 16) {
        latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &gpr_opnd, opnd2, 15, 0);
    } else if (ir1_opnd_is_8h(opnd1)) {
        latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &gpr_opnd, opnd2, 15, 8);
    } else {
        latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &gpr_opnd, opnd2,  7, 0);
    }
}

void latxs_store_ir2_to_ir1_mem(IR2_OPND *value_opnd,
        IR1_OPND *opnd1, int addr_size)
{
    lsassert(latxs_ir2_opnd_is_gpr(value_opnd));

    IR2_OPND mem_opnd;

    if (addr_size < 0) {
        addr_size = latxs_ir1_addr_size(lsenv->tr_data->curr_ir1_inst);
    }

    latxs_convert_mem_opnd(&mem_opnd, opnd1, addr_size);

    IR2_OPCODE mem_opcode = LISA_INVALID;

    switch (ir1_opnd_size(opnd1)) {
    case 128:
        mem_opcode = LISA_ST_D;
        break;
    case 64:
        mem_opcode = LISA_ST_D;
        break;
    case 32:
        mem_opcode = LISA_ST_W;
        break;
    case 16:
        mem_opcode = LISA_ST_H;
        break;
    case 8:
        mem_opcode = LISA_ST_B;
        break;
    default:
        lsassertm(0, "store ir2 to ir1 mem should not reach here.\n");
        break;
    }

    gen_ldst_softmmu_helper(mem_opcode, value_opnd, &mem_opnd, true);

    latxs_ra_free_temp(&mem_opnd);

    return;
}

/* sys-mode mov to seg */
void latxs_store_ir2_to_ir1_seg(IR2_OPND *seg_value_opnd, IR1_OPND *opnd1)
{
    lsassert(latxs_ir2_opnd_is_gpr(seg_value_opnd));

    int seg_num = ir1_opnd_base_reg_num(opnd1);

    TRANSLATION_DATA *td = lsenv->tr_data;

    /*
     * Intel SDM Vol.3A 6.8.3 states;
     * "Any single-step trap that would be delivered following the MOV to SS
     * instruction or POP to SS instruction (because EFLAGS.TF is 1) is
     * suppressed."
     */
    if (seg_num == ss_index) {
        td->sys.tf = 0;
    }

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
    IR2_OPND _selector_reg = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2_(lisa_mov16z, &_selector_reg, seg_value_opnd);
    latxs_append_ir2_opnd2i(LISA_ST_W, &_selector_reg,
            &latxs_env_ir2_opnd,
            lsenv_offset_of_seg_selector(lsenv, seg_num));
    latxs_ra_free_temp(&_selector_reg);

    /* 0.2 base = 20-bits = selector << 4 */
    IR2_OPND _base_reg = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_SLLI_D, &_base_reg, &_selector_reg, 0x4);
    latxs_append_ir2_opnd2i(LISA_ST_W, &_base_reg,
            &latxs_env_ir2_opnd,
            lsenv_offset_of_seg_base(lsenv, seg_num));
    latxs_ra_free_temp(&_base_reg);

    goto _GEN_EOB_;

_NORMAL_MODE_:
    /*
     * seg_num = ir1_opnd_base_reg_num(opnd1);
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
     *                               R_TR = 7, } X86Seg
     */
    lsassert(seg_num != cs_index);

    /* 0. save native context */
    latxs_tr_gen_call_to_helper_prologue_cfg(cfg);

    /*
     * target/i386/seg_helper.c
     * void helper_load_seg(
     *      CPUX86State *env,
     *      int seg_reg,
     *      int selector)
     */

    /* arg2: selector = seg_value_opnd */
    IR2_OPND seg_tmp = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2_(lisa_mov16z, &seg_tmp, seg_value_opnd);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg2_ir2_opnd, &seg_tmp);
    latxs_ra_free_temp(&seg_tmp);

    /* arg0: env*/
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd);

    /* arg1: segment register numeber */
    latxs_append_ir2_opnd2i(LISA_ORI, &latxs_arg1_ir2_opnd,
    /* CS is checked in tr_excp.c */
            &latxs_zero_ir2_opnd, seg_num);

    /* call to helper */
    latxs_tr_gen_call_to_helper((ADDR)helper_load_seg);

    /* 8. restore native context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(cfg);

_GEN_EOB_:

    if (seg_num == ss_index) {
        td->recheck_tf  = 0;
        td->inhibit_irq = 1;
    }

    return;
}

/*
 * Store the value from IR2_OPND to IR1_OPND
 *
 * @opnd2: IR2_OPND_IREG
 * @opnd1: chould be - GPR
 *                   - Segment selector
 *                   - CR, DR
 *                   - MMX, XMM
 *                   - Memory
 *
 * TODO remove xmm hi
 */
void latxs_store_ir2_to_ir1(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    lsassert(latxs_ir2_opnd_is_gpr(opnd2));

    switch (ir1_opnd_type(opnd1)) {
    case X86_OP_REG:  {
        if (ir1_opnd_is_gpr(opnd1)) {
            if (option_by_hand) {
                latxs_store_ir2_to_ir1_gpr_em(opnd2, opnd1);
            } else {
                latxs_store_ir2_to_ir1_gpr(opnd2, opnd1);
            }
            break;
        } else if (ir1_opnd_is_seg(opnd1)) {
            latxs_store_ir2_to_ir1_seg(opnd2, opnd1);
            break;
        } else if (latxs_ir1_opnd_is_cr(opnd1)) {
            latxs_store_ir2_to_ir1_cr(opnd2, opnd1);
            break;
        } else if (latxs_ir1_opnd_is_dr(opnd1)) {
            latxs_store_ir2_to_ir1_dr(opnd2, opnd1);
            break;
        } else if (ir1_opnd_is_mmx(opnd1)) {
            int ir1_reg = ir1_opnd_base_reg_num(opnd1);
            IR2_OPND mmx_opnd = latxs_ra_alloc_mmx(ir1_reg);
            latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &mmx_opnd, opnd2);
            break;
        } else if (ir1_opnd_is_xmm(opnd1)) {
            lsassertm(0, "%s to be implemented in LoongArch.\n", __func__);
            break;
        } else {
            lsassert(0);
        }
        break;
    }
    case X86_OP_MEM:
        latxs_store_ir2_to_ir1_mem(opnd2, opnd1, -1);
        break;
    default:
        lsassert(0);
        break;
    }

    return;
}

static void latxs_load_64_bit_freg_from_ir1_80_bit_mem(
        IR2_OPND *opnd2, IR2_OPND *mem_opnd)
{
    /* 1. load sign and exponent */
    IR2_OPND ir2_sign_exp = latxs_ra_alloc_itemp();
    IR2_OPND ir2_fraction = latxs_ra_alloc_itemp();

    int mem_base = latxs_ir2_opnd_reg(mem_opnd);
    int mem_off  = latxs_ir2_opnd_offset(mem_opnd);
    IR2_OPND mem_sign_exp;
    latxs_ir2_opnd_build_mem(&mem_sign_exp, mem_base, mem_off + 8);

    gen_ldst_softmmu_helper(LISA_LD_HU, &ir2_sign_exp, &mem_sign_exp, 1);
    gen_ldst_softmmu_helper(LISA_LD_D,  &ir2_fraction,  mem_opnd,     1);

    IR2_OPND f_ir2_sign_exp = latxs_ra_alloc_ftemp();
    IR2_OPND f_ir2_fraction = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2(LISA_MOVGR2FR_W, &f_ir2_sign_exp, &ir2_sign_exp);
    latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &f_ir2_fraction, &ir2_fraction);
    latxs_ra_free_temp(&ir2_sign_exp);
    latxs_ra_free_temp(&ir2_fraction);

    latxs_append_ir2_opnd3(LISA_FCVT_D_LD, opnd2,
            &f_ir2_fraction, &f_ir2_sign_exp);
    latxs_ra_free_temp(&f_ir2_sign_exp);
    latxs_ra_free_temp(&f_ir2_fraction);

    IR2_OPND flag_san = latxs_ra_alloc_itemp();
    IR2_OPND label_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd2(LISA_MOVFCSR2GR, &flag_san, &fcsr_ir2_opnd);
    latxs_append_ir2_opnd2ii(LISA_BSTRPICK_D, &flag_san,
            &flag_san, 28, 28);

    latxs_append_ir2_opnd3(LISA_BEQ, &flag_san,
            &latxs_zero_ir2_opnd, &label_exit);
    latxs_append_ir2_opnd2(LISA_MOVFR2GR_D, &flag_san, opnd2);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &flag_san,
            &latxs_zero_ir2_opnd, 51, 51);
    latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, opnd2, &flag_san);
    latxs_ra_free_temp(&flag_san);

    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);
}

static void latxs_load_freg_from_ir1_mem(IR2_OPND *opnd2,
        IR1_OPND *opnd1, bool is_convert)
{
    IR2_OPND mem_opnd;
    int addr_size = latxs_ir1_addr_size(lsenv->tr_data->curr_ir1_inst);
    latxs_convert_mem_opnd(&mem_opnd, opnd1, addr_size);

    if (ir1_opnd_size(opnd1) == 32) {
        gen_ldst_c1_softmmu_helper(LISA_FLD_S, opnd2, &mem_opnd, 1);
        if (is_convert) {
            latxs_append_ir2_opnd2(LISA_FCVT_D_S, opnd2, opnd2);
        }
    } else if (ir1_opnd_size(opnd1) == 64) {
        gen_ldst_c1_softmmu_helper(LISA_FLD_D, opnd2, &mem_opnd, 1);
    } else if (ir1_opnd_size(opnd1) == 16) {
        IR2_OPND itemp = latxs_ra_alloc_itemp();
        gen_ldst_softmmu_helper(LISA_LD_H, &itemp, &mem_opnd, 1);
        latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, opnd2, &itemp);
        latxs_ra_free_temp(&itemp);
        lsassertm(!is_convert, "convert 16-bit floating point?\n");
    } else if (ir1_opnd_size(opnd1) == 8) {
        IR2_OPND itemp = latxs_ra_alloc_itemp();
        gen_ldst_softmmu_helper(LISA_LD_B, &itemp, &mem_opnd, 1);
        latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, opnd2, &itemp);
        latxs_ra_free_temp(&itemp);
        lsassertm(!is_convert, "convert 8-bit floating point?\n");
    } else if (ir1_opnd_size(opnd1) == 128) {
        latxs_load_freg128_from_ir1_mem(opnd2, opnd1);
    } else {
        latxs_load_64_bit_freg_from_ir1_80_bit_mem(opnd2, &mem_opnd);
    }

    latxs_ra_free_temp(&mem_opnd);
}

static void latxs_load_freg_from_ir1_fpr(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    lsassert(latxs_ir2_opnd_is_fpr(opnd2));
    lsassert(ir1_opnd_is_fpr(opnd1));

    int ir1_reg = ir1_opnd_base_reg_num(opnd1);
    IR2_OPND value_opnd = latxs_ra_alloc_st(ir1_reg);

    if (!latxs_ir2_opnd_cmp(opnd2, &value_opnd)) {
        latxs_append_ir2_opnd2(LISA_FMOV_D, opnd2, &value_opnd);
    }
}

static void latxs_load_freg_from_ir1_mmx(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    lsassert(latxs_ir2_opnd_is_fpr(opnd2));
    lsassert(ir1_opnd_is_mmx(opnd1));

    int ir1_reg = ir1_opnd_base_reg_num(opnd1);
    IR2_OPND mmx_opnd = latxs_ra_alloc_mmx(ir1_reg);

    if (!latxs_ir2_opnd_cmp(opnd2, &mmx_opnd)) {
        latxs_append_ir2_opnd2(LISA_FMOV_D, opnd2, &mmx_opnd);
    }
}

static void latxs_load_freg_from_ir1_xmm(IR2_OPND *opnd2,
        IR1_OPND *opnd1)
{
    lsassertm(0, "%s to be implemented in LoongArch.\n", __func__);
}

IR2_OPND latxs_load_freg_from_ir1_1(IR1_OPND *opnd1, bool is_convert)
{
    switch (ir1_opnd_type(opnd1)) {
    case X86_OP_REG: {
        if (ir1_opnd_is_fpr(opnd1)) {
            int ir1_reg = ir1_opnd_base_reg_num(opnd1);
            return latxs_ra_alloc_st(ir1_reg);
        } else if (ir1_opnd_is_mmx(opnd1)) {
            int ir1_reg = ir1_opnd_base_reg_num(opnd1);
            return latxs_ra_alloc_mmx(ir1_reg);
        } else if (ir1_opnd_is_xmm(opnd1)) {
            int ir1_reg = ir1_opnd_base_reg_num(opnd1);
            return latxs_ra_alloc_xmm(ir1_reg);
        } else if (ir1_opnd_is_ymm(opnd1)) {
            lsassert(0);
        } else {
            lsassert(0);
        }
        break;
    }
    case X86_OP_MEM: {
        IR2_OPND ret_opnd = latxs_ra_alloc_ftemp();
        latxs_load_freg_from_ir1_mem(&ret_opnd, opnd1, is_convert);
        return ret_opnd;
    }
    default:
        lsassert(0);
    }
    return latxs_invalid_ir2_opnd;
}

void latxs_load_freg_from_ir1_2(IR2_OPND *opnd2,
        IR1_OPND *opnd1, bool is_convert)
{
    lsassert(latxs_ir2_opnd_is_fpr(opnd2));

    switch (ir1_opnd_type(opnd1)) {
    case X86_OP_REG: {
        if (ir1_opnd_is_fpr(opnd1)) {
            latxs_load_freg_from_ir1_fpr(opnd2, opnd1);
            return;
        } else if (ir1_opnd_is_mmx(opnd1)) {
            latxs_load_freg_from_ir1_mmx(opnd2, opnd1);
            return;
        } else if (ir1_opnd_is_xmm(opnd1)) {
            latxs_load_freg_from_ir1_xmm(opnd2, opnd1);
            return;
        } else if (ir1_opnd_is_ymm(opnd1)) {
            lsassert(0);
        }
        lsassertm(0, "REG:%s in %s\n", ir1_reg_name(opnd1->reg), __func__);
        break;
    }
    case X86_OP_MEM:
        latxs_load_freg_from_ir1_mem(opnd2, opnd1, is_convert);
        break;
    default:
        lsassert(0);
        break;
    }

    return;
}

void latxs_load_singles_from_ir1_pack(IR2_OPND *single0,
        IR2_OPND *single1, IR1_OPND *opnd1)
{
    lsassertm(0, "%s to be implemented in LoongArch.\n", __func__);
}

void latxs_store_singles_to_ir2_pack(IR2_OPND *single0,
        IR2_OPND *single1, IR2_OPND *pack)
{
    lsassert(latxs_ir2_opnd_is_fpr(single0) &&
             latxs_ir2_opnd_is_fpr(single1) &&
             latxs_ir2_opnd_is_fpr(pack));

    lsassertm(0, "%s to be implemented in LoongArch.\n", __func__);
    return;
}

void latxs_store_64_bit_freg_to_ir1_80_bit_mem(
        IR2_OPND *opnd2, IR2_OPND *mem_opnd)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;

    IR2_OPND f_ir2_sign_exp = latxs_ra_alloc_ftemp();
    IR2_OPND f_ir2_fraction = latxs_ra_alloc_ftemp();

    IR2_OPND flag_san = latxs_ra_alloc_itemp();
    IR2_OPND ir2_sign_exp = latxs_ra_alloc_itemp();
    IR2_OPND ir2_fraction = latxs_ra_alloc_itemp();
    IR2_OPND label_ok = latxs_ir2_opnd_new_label();

    latxs_append_ir2_opnd2(LISA_FCVT_LD_D, &f_ir2_fraction, opnd2);
    latxs_append_ir2_opnd2(LISA_FCVT_UD_D, &f_ir2_sign_exp, opnd2);

    latxs_append_ir2_opnd2(LISA_MOVFCSR2GR, &flag_san, &fcsr_ir2_opnd);
    latxs_append_ir2_opnd2ii(LISA_BSTRPICK_D, &flag_san,
            &flag_san, 28, 28);

    latxs_append_ir2_opnd2(LISA_MOVFR2GR_S, &ir2_sign_exp, &f_ir2_sign_exp);
    latxs_append_ir2_opnd2(LISA_MOVFR2GR_D, &ir2_fraction, &f_ir2_fraction);
    latxs_ra_free_temp(&f_ir2_sign_exp);
    latxs_ra_free_temp(&f_ir2_fraction);

    /* Identify SNAN and write snan to opnd2 */
    latxs_append_ir2_opnd3(LISA_BEQ, &flag_san, zero, &label_ok);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &ir2_fraction, zero, 62, 62);
    latxs_append_ir2_opnd1(LISA_LABEL, &label_ok);
    latxs_ra_free_temp(&flag_san);

    int mem_base = latxs_ir2_opnd_reg(mem_opnd);
    int mem_off  = latxs_ir2_opnd_offset(mem_opnd);
    IR2_OPND mem_sign_exp;
    latxs_ir2_opnd_build_mem(&mem_sign_exp, mem_base, mem_off + 8);

    gen_ldst_softmmu_helper(LISA_ST_H, &ir2_sign_exp, &mem_sign_exp, 1);
    gen_ldst_softmmu_helper(LISA_ST_D, &ir2_fraction,  mem_opnd,     1);

    latxs_ra_free_temp(&ir2_sign_exp);
    latxs_ra_free_temp(&ir2_fraction);
}

static void latxs_store_freg_to_ir1_mem(IR2_OPND *opnd2,
        IR1_OPND *opnd1, bool is_convert)
{
    IR2_OPND mem_opnd;
    int addr_size = latxs_ir1_addr_size(lsenv->tr_data->curr_ir1_inst);
    latxs_convert_mem_opnd(&mem_opnd, opnd1, addr_size);

    int save_temp = 1;

    if (ir1_opnd_size(opnd1) == 32) {
        IR2_OPND ftemp = latxs_ra_alloc_ftemp();
        if (is_convert) {
            latxs_append_ir2_opnd2(LISA_FCVT_S_D, &ftemp, opnd2);
            gen_ldst_c1_softmmu_helper(LISA_FST_S, &ftemp,
                    &mem_opnd, save_temp);
        } else {
            gen_ldst_c1_softmmu_helper(LISA_FST_S, opnd2,
                    &mem_opnd, save_temp);
        }
        latxs_ra_free_temp(&ftemp);
    } else if (ir1_opnd_size(opnd1) == 64) {
        gen_ldst_c1_softmmu_helper(LISA_FST_D, opnd2, &mem_opnd, save_temp);
    } else if (ir1_opnd_size(opnd1) == 128) {
        latxs_store_freg128_to_ir1_mem(opnd2, opnd1);
    } else {
        latxs_store_64_bit_freg_to_ir1_80_bit_mem(opnd2, &mem_opnd);
    }

    latxs_ra_free_temp(&mem_opnd);
}

static void latxs_store_freg_to_ir1_fpr(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    lsassert(latxs_ir2_opnd_is_fpr(opnd2));

    int ir1_reg = ir1_opnd_base_reg_num(opnd1);
    IR2_OPND target_opnd = latxs_ra_alloc_st(ir1_reg);

    if (!latxs_ir2_opnd_cmp(opnd2, &target_opnd)) {
        latxs_append_ir2_opnd2(LISA_FMOV_D, &target_opnd, opnd2);
    }
}

static void latxs_store_freg_to_ir1_mmx(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    lsassert(latxs_ir2_opnd_is_fpr(opnd2));
    lsassert(ir1_opnd_is_mmx(opnd1));

    int ir1_reg = ir1_opnd_base_reg_num(opnd1);

    IR2_OPND mmx_opnd = latxs_ra_alloc_mmx(ir1_reg);

    if (latxs_ir2_opnd_cmp(opnd2, &mmx_opnd)) {
        latxs_append_ir2_opnd2(LISA_FMOV_D, &mmx_opnd, opnd2);
    }
}

static void latxs_store_freg_to_ir1_xmm(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    lsassertm(0, "%s to be implemented in LoongArch.\n", __func__);
}

void latxs_store_freg_to_ir1(IR2_OPND *opnd2,
        IR1_OPND *opnd1, bool is_convert)
{
    lsassert(latxs_ir2_opnd_is_fpr(opnd2));

    switch (ir1_opnd_type(opnd1)) {
    case X86_OP_REG:  {
        if (ir1_opnd_is_fpr(opnd1)) {
            latxs_store_freg_to_ir1_fpr(opnd2, opnd1);
            return;
        } else if (ir1_opnd_is_mmx(opnd1)) {
            latxs_store_freg_to_ir1_mmx(opnd2, opnd1);
            return;
        } else if (ir1_opnd_is_xmm(opnd1)) {
            latxs_store_freg_to_ir1_xmm(opnd2, opnd1);
            return;
        } else if (ir1_opnd_is_ymm(opnd1)) {
            lsassert(0);
        }
        lsassertm(0, "REG:%s\n", ir1_reg_name(opnd1->reg));
        break;
    }
    case X86_OP_MEM:
        latxs_store_freg_to_ir1_mem(opnd2, opnd1, is_convert);
        return;
    default:
        lsassert(0);
        return;
    }
}

void latxs_load_freg128_from_ir1_mem(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    lsassert(ir1_opnd_is_mem(opnd1));
    lsassert(latxs_ir2_opnd_is_fpr(opnd2));

    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd1, -1);

    int new_tmp = 0;
    IR2_OPND mem_no_offset = latxs_convert_mem_ir2_opnd_no_offset(
            &mem_opnd, &new_tmp);
    IR2_OPND mem_base = latxs_ir2_opnd_mem_get_base(&mem_no_offset);
    if (new_tmp) {
        latxs_ra_free_temp(&mem_opnd);
    }

    if (option_fast_fpr_ldst) {
        gen_ldst_softmmu_helper(LISA_VLD, opnd2, &mem_no_offset, 1);
        return;
    }

    static __thread uint64_t freg128_space[4];
    IR2_OPND data_addr = latxs_ra_alloc_itemp();
    latxs_load_imm64_to_ir2(&data_addr, (uint64_t)&freg128_space[0]);

    IR2_OPND data64 = latxs_ra_alloc_itemp();
    gen_ldst_softmmu_helper(LISA_LD_D, &data64, &mem_no_offset, 1);
    latxs_append_ir2_opnd2i(LISA_ST_D, &data64, &data_addr, 0);
    latxs_append_ir2_opnd2i(LISA_ADDI_D, &mem_base, &mem_base, 8);

    gen_ldst_softmmu_helper(LISA_LD_D, &data64, &mem_no_offset, 1);
    latxs_append_ir2_opnd2i(LISA_ST_D, &data64, &data_addr, 8);
    latxs_ra_free_temp(&data64);

    latxs_append_ir2_opnd2i(LISA_VLD, opnd2, &data_addr, 0);
}

void latxs_store_freg128_to_ir1_mem(IR2_OPND *opnd2, IR1_OPND *opnd1)
{
    lsassert(ir1_opnd_is_mem(opnd1));
    lsassert(latxs_ir2_opnd_is_fpr(opnd2));

    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd1, -1);

    int new_tmp = 0;
    IR2_OPND mem_no_offset = latxs_convert_mem_ir2_opnd_no_offset(
            &mem_opnd, &new_tmp);
    IR2_OPND mem_base = latxs_ir2_opnd_mem_get_base(&mem_no_offset);
    if (new_tmp) {
        latxs_ra_free_temp(&mem_opnd);
    }

    if (option_fast_fpr_ldst) {
        gen_ldst_softmmu_helper(LISA_VST, opnd2, &mem_no_offset, 1);
        return;
    }

    static __thread uint64_t freg128_space[4];
    IR2_OPND data_addr = latxs_ra_alloc_itemp();
    latxs_load_imm64_to_ir2(&data_addr, (uint64_t)&freg128_space[0]);

    latxs_append_ir2_opnd2i(LISA_VST, opnd2, &data_addr, 0);

    IR2_OPND data64 = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_D, &data64, &data_addr, 0);
    gen_ldst_softmmu_helper(LISA_ST_D, &data64, &mem_no_offset, 1);
    latxs_append_ir2_opnd2i(LISA_ADDI_D, &mem_base, &mem_base, 8);

    latxs_append_ir2_opnd2i(LISA_LD_D, &data64, &data_addr, 8);
    gen_ldst_softmmu_helper(LISA_ST_D, &data64, &mem_no_offset, 1);

    latxs_ra_free_temp(&data64);
}

IR2_OPND latxs_load_freg128_from_ir1(IR1_OPND *opnd1)
{
    lsassert(ir1_opnd_is_xmm(opnd1) || ir1_opnd_is_mem(opnd1));

    if (ir1_opnd_is_xmm(opnd1)) {
        return latxs_ra_alloc_xmm(ir1_opnd_base_reg_num(opnd1));
    } else if (ir1_opnd_is_mem(opnd1)) {
        IR2_OPND ret_opnd = latxs_ra_alloc_ftemp();
        if (ir1_opnd_size(opnd1) == 128) {
            latxs_load_freg128_from_ir1_mem(&ret_opnd, opnd1);
        } else {
            latxs_load_freg_from_ir1_mem(&ret_opnd, opnd1, false);
        }
        return ret_opnd;
    }

    lsassert(0);
    return latxs_invalid_ir2_opnd;
}

void latxs_reverse_ir2_ireg_opnd_endian(IR2_OPND *opnd, int size_in_bit)
{
    lsassertm(0, "%s to be implemented in LoongArch.\n", __func__);
}
