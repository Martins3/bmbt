#include "common.h"
#include "ir1/ir1.h"
#include "reg_alloc.h"
#include "env.h"
#include "x86tomips-options.h"
#include "flag_lbt.h"

#ifdef CONFIG_SOFTMMU
/* In system-mode, the upper 16-bit of eflags is used.
 * So we should not use mips_andi to operate the eflags. */

/* Clear some bits in the lower 16-bit of elfags.
 *
 * @flag_mask should contains '1' to indicate the bit to clear.
 * All their index shoud be less than 16. */
void tr_gen_eflags_clr_bit_00(IR2_OPND *eflags, uint32_t flag_mask)
{
    IR2_OPND mask = ra_alloc_itemp();

    append_ir2_opnd2i(LISA_ORI, &mask, &zero_ir2_opnd, flag_mask);
    append_ir2_opnd3 (LISA_NOR, &mask, &mask, &zero_ir2_opnd);
    append_ir2_opnd3 (LISA_AND, eflags, eflags, &mask);

    ra_free_temp(&mask);
}

void tr_gen_eflags_clr_bit_10(IR2_OPND *dest, IR2_OPND *eflags, uint32_t flag_mask)
{
    IR2_OPND mask = ra_alloc_itemp();

    append_ir2_opnd2i(LISA_ORI, &mask, &zero_ir2_opnd, flag_mask);
    append_ir2_opnd3 (LISA_NOR, &mask, &mask, &zero_ir2_opnd);
    append_ir2_opnd3 (LISA_AND, dest,  eflags, &mask);
 
    ra_free_temp(&mask);
}
#else
/* In user-mode, it is OK to use mips_andi */
void tr_gen_eflags_clr_bit_00(IR2_OPND *eflags, uint32_t flag_mask)
{
    append_ir2_opnd2i(LISA_ANDI, eflags, eflags, ~flag_mask);
}
void tr_gen_eflags_clr_bit_10(IR2_OPND *dest, IR2_OPND *eflags, uint32_t flag_mask)
{
    append_ir2_opnd2i(LISA_ANDI, dest, eflags, ~flag_mask);
}
#endif

/* Clear some bits in the upper 16-bit of elfgas.
 * To be supported in the future if needed. */
/* static void tr_gen_eflags_clr_bit_01(IR2_OPND *eflags, uint32_t flag_mask) */
/* static void tr_gen_eflags_clr_bit_11(IR2_OPND *eflags, uint32_t flag_mask) */

static void generate_cf(IR2_OPND *dest, IR2_OPND *src0, IR2_OPND *src1)
{
    IR1_INST *pir1 = lsenv->tr_data->curr_ir1_inst;

    switch (ir1_opcode(pir1)) {
//    case X86_INS_ADD:
//    case X86_INS_XADD: {
//        IR2_OPND cf_opnd = ra_alloc_itemp();
//        if (ir2_opnd_is_ireg(src1)) {
//            append_ir2_opnd3(mips_sltu, &cf_opnd, dest, src1);
//        } else {
//            append_ir2_opnd2i(mips_sltiu, &cf_opnd, dest, ir2_opnd_imm(src1));
//        }
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, CF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &cf_opnd);
//        ra_free_temp(&cf_opnd);
//        return;
//    }
//    case X86_INS_ADC: {
//        IR2_OPND cf_opnd = ra_alloc_itemp();
//        IR2_OPND src1_add_1 = ra_alloc_itemp();
//        IR2_OPND label_opnd_0 = ir2_opnd_new_type(IR2_OPND_LABEL);
//        if (ir2_opnd_is_imm(src1)) {
//            if (ir2_opnd_imm(src1) + 1 == 0) {
//                append_ir2_opnd2i(mips_andi, &cf_opnd, &eflags_ir2_opnd, 1);
//                append_ir2_opnd3(mips_bne, &cf_opnd, &zero_ir2_opnd, &label_opnd_0);
//            }
//            append_ir2_opnd3(mips_sltu, &cf_opnd, dest, src0);
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, CF_BIT);
//            append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &cf_opnd);
//            append_ir2_opnd1(mips_label, &label_opnd_0);
//            ra_free_temp(&cf_opnd);
//            ra_free_temp(&src1_add_1);
//            return;
//        } else {
//            append_ir2_opnd2i(mips_addiu, &src1_add_1, src1, 1);
//            append_ir2_opnd2(mips_mov64, &cf_opnd, &zero_ir2_opnd);
//            append_ir2_opnd3(mips_movz, &cf_opnd, &eflags_ir2_opnd, &src1_add_1);
//            append_ir2_opnd2i(mips_andi, &cf_opnd, &cf_opnd, 1);
//            append_ir2_opnd3(mips_bne, &cf_opnd, &zero_ir2_opnd, &label_opnd_0);
//            append_ir2_opnd3(mips_sltu, &cf_opnd, dest, src0);
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, CF_BIT);
//            append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &cf_opnd);
//            append_ir2_opnd1(mips_label, &label_opnd_0);
//            ra_free_temp(&cf_opnd);
//            ra_free_temp(&src1_add_1);
//            return;
//        }
//    }
//    case X86_INS_SCASB:
//    case X86_INS_SCASW:
//    case X86_INS_SCASD:
//    case X86_INS_SUB:
//    case X86_INS_CMPSB:
//    case X86_INS_CMPSW:
//    case X86_INS_CMPSD:    
//    case X86_INS_CMP:
//    case X86_INS_CMPXCHG:
//    case X86_INS_DEC: {
//        IR2_OPND cf_opnd = ra_alloc_itemp();
//        append_ir2_opnd3(mips_sltu, &cf_opnd, src0, src1);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, CF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &cf_opnd);
//        ra_free_temp(&cf_opnd);
//        return;
//    }
//    case X86_INS_NEG: {
//        IR2_OPND eflag_cf_0 = ra_alloc_itemp();
//        IR2_OPND ir2_opnd_imm = ir2_opnd_new(IR2_OPND_IMM, 0x1);
//        tr_gen_eflags_clr_bit_10(&eflag_cf_0, &eflags_ir2_opnd, CF_BIT);
//        append_ir2_opnd3(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, &ir2_opnd_imm);
//        append_ir2_opnd3(mips_movz, &eflags_ir2_opnd, &eflag_cf_0, dest);
//        ra_free_temp(&eflag_cf_0);
//        return;
//    }
//    case X86_INS_SBB: {
//        IR2_OPND cf_opnd = ra_alloc_itemp();
//        IR2_OPND src1_add_1 = ra_alloc_itemp();
//        IR2_OPND label_opnd_1 = ir2_opnd_new_type(IR2_OPND_LABEL);
//        append_ir2_opnd2i(mips_addiu, &src1_add_1, src1, 1);
//        append_ir2_opnd2(mips_mov64, &cf_opnd, &zero_ir2_opnd);
//        append_ir2_opnd3(mips_movz, &cf_opnd, &eflags_ir2_opnd, &src1_add_1);
//        append_ir2_opnd2i(mips_andi, &cf_opnd, &cf_opnd, 1);
//        append_ir2_opnd3(mips_bne, &cf_opnd, &zero_ir2_opnd, &label_opnd_1);
//        append_ir2_opnd2i(mips_andi, &cf_opnd, &eflags_ir2_opnd, 1);
//        append_ir2_opnd3(mips_daddu, &src1_add_1, src1, &cf_opnd);
//        append_ir2_opnd3(mips_sltu, &cf_opnd, src0, &src1_add_1);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, CF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &cf_opnd);
//        append_ir2_opnd1(mips_label, &label_opnd_1);
//        ra_free_temp(&cf_opnd);
//        ra_free_temp(&src1_add_1);
//        return;
//    }
//    case X86_INS_OR:
//    case X86_INS_AND:
//    case X86_INS_XOR:
//    case X86_INS_TEST:
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, CF_BIT);
//        return;
//    case X86_INS_RCR:
//    case X86_INS_RCL: {
//        IR2_OPND t_dest_opnd = ra_alloc_itemp();
//        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
//            append_ir2_opnd2i(mips_dsrl32, &t_dest_opnd, dest, 0);
//        else
//            append_ir2_opnd2i(mips_dsrl, &t_dest_opnd, dest, ir1_opnd_size(ir1_get_opnd(pir1, 0)));
//        append_ir2_opnd2i(mips_andi, &t_dest_opnd, &t_dest_opnd, 1);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, CF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &t_dest_opnd);
//        ra_free_temp(&t_dest_opnd);
//        return;
//    }
//    case X86_INS_ROR: {
//        IR2_OPND t_dest_opnd = ra_alloc_itemp();
//        append_ir2_opnd2i(mips_dsrl, &t_dest_opnd, dest, ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
//        append_ir2_opnd2i(mips_andi, &t_dest_opnd, &t_dest_opnd, 1);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, CF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &t_dest_opnd);
//        ra_free_temp(&t_dest_opnd);
//        return;
//    }
//    case X86_INS_SHL:
//    case X86_INS_SAL: {
//        IR2_OPND t_dest_opnd = ra_alloc_itemp();
//        if (ir2_opnd_is_imm(src1))
//            if (ir2_opnd_imm(src1) >= ir1_opnd_size(ir1_get_opnd(pir1, 0))) {
//                ra_free_temp(&t_dest_opnd);
//                return;
//            } else
//                append_ir2_opnd2i(mips_dsll, &t_dest_opnd, src0, ir2_opnd_imm(src1) - 1);
//        else {
//            IR2_OPND tmp_count = ra_alloc_itemp();
//            append_ir2_opnd2i(mips_daddiu, &tmp_count, src1, -1);
//            append_ir2_opnd3(mips_dsllv, &t_dest_opnd, src0, &tmp_count);
//            ra_free_temp(&tmp_count);
//        }
//        append_ir2_opnd2i(mips_dsrl, &t_dest_opnd, &t_dest_opnd,
//                ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1); /* 64bit not handled */
//        append_ir2_opnd2i(mips_andi, &t_dest_opnd, &t_dest_opnd, 1);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, CF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &t_dest_opnd);
//        ra_free_temp(&t_dest_opnd);
//        return;
//    }
//    case X86_INS_ROL: {
//        IR2_OPND t_dest_opnd = ra_alloc_itemp();
//        append_ir2_opnd2i(mips_andi, &t_dest_opnd, dest, 1);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, CF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &t_dest_opnd);
//        ra_free_temp(&t_dest_opnd);
//        return;
//    }
    case X86_INS_SHLD: {
        if (ir2_opnd_is_imm(src1)) {
            IR2_OPND t_dest_opnd = ra_alloc_itemp();
            int count = ir2_opnd_imm(src1) - 1;
            append_ir2_opnd2i(LISA_SLLI_D, &t_dest_opnd, src0, count);
            append_ir2_opnd2i(LISA_SRLI_D, &t_dest_opnd, &t_dest_opnd, ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
            append_ir2_opnd2i(LISA_ANDI,   &t_dest_opnd, &t_dest_opnd, 1);
            if (option_lbt) {
                append_ir2_opnd1i(LISA_X86MTFLAG, &t_dest_opnd, 0x1);
            } else {
                tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, CF_BIT);
                append_ir2_opnd3(LISA_OR, &eflags_ir2_opnd,
                                          &eflags_ir2_opnd, &t_dest_opnd);
            }
            ra_free_temp(&t_dest_opnd);
        } else {
            IR2_OPND t_dest_opnd = ra_alloc_itemp();
            append_ir2_opnd2i(LISA_ADDI_D, &t_dest_opnd, src1, -1);
            append_ir2_opnd3(LISA_SLL_W,   &t_dest_opnd, src0, &t_dest_opnd);
            append_ir2_opnd2i(LISA_SRLI_D, &t_dest_opnd, &t_dest_opnd,
                ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1); /* 64bit not handled */
            append_ir2_opnd2i(LISA_ANDI,   &t_dest_opnd, &t_dest_opnd, 1);
            if (option_lbt) {
                append_ir2_opnd1i(LISA_X86MTFLAG, &t_dest_opnd, 0x1);
            } else {
                tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, CF_BIT);
                append_ir2_opnd3(LISA_OR, &eflags_ir2_opnd,
                                          &eflags_ir2_opnd, &t_dest_opnd);
            }
            ra_free_temp(&t_dest_opnd);
        }
        return;
    }
    case X86_INS_SHRD: {
        if (ir2_opnd_is_imm(src1)) {
            IR2_OPND t_dest_opnd = ra_alloc_itemp();
            int count = ir2_opnd_imm(src1) - 1;
            append_ir2_opnd2i(LISA_SRLI_D, &t_dest_opnd, src0, count);
            append_ir2_opnd2i(LISA_ANDI,   &t_dest_opnd, &t_dest_opnd, 1);
            if (option_lbt) {
                append_ir2_opnd1i(LISA_X86MTFLAG, &t_dest_opnd, 0x1);
            } else {
                tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, CF_BIT);
                append_ir2_opnd3(LISA_OR, &eflags_ir2_opnd,
                                          &eflags_ir2_opnd, &t_dest_opnd);
            }
            ra_free_temp(&t_dest_opnd);
        } else {
            IR2_OPND t_dest_opnd = ra_alloc_itemp();
            append_ir2_opnd2i(LISA_ADDI_D, &t_dest_opnd, src1, -1);
            append_ir2_opnd3 (LISA_SRL_D,  &t_dest_opnd, src0, &t_dest_opnd);
            append_ir2_opnd2i(LISA_ANDI,   &t_dest_opnd, &t_dest_opnd, 1);
            if (option_lbt) {
                append_ir2_opnd1i(LISA_X86MTFLAG, &t_dest_opnd, 0x1);
            } else {
                tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, CF_BIT);
                append_ir2_opnd3(LISA_OR, &eflags_ir2_opnd,
                                          &eflags_ir2_opnd, &t_dest_opnd);
            }
            ra_free_temp(&t_dest_opnd);
        }
        return;
    }
//    case X86_INS_SHR:
//    case X86_INS_SAR: {
//        IR2_OPND t_dest_opnd = ra_alloc_itemp();
//        if (ir2_opnd_is_imm(src1)) {
//            append_ir2_opnd2i(mips_dsrl, &t_dest_opnd, src0, ir2_opnd_imm(src1) - 1);
//        } else {
//            append_ir2_opnd2i(mips_daddiu, &t_dest_opnd, src1, -1);
//            append_ir2_opnd3(mips_dsrlv, &t_dest_opnd, src0, &t_dest_opnd);
//        }
//        append_ir2_opnd2i(mips_andi, &t_dest_opnd, &t_dest_opnd, 1);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, CF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &t_dest_opnd);
//        ra_free_temp(&t_dest_opnd);
//        return;
//    }
//    case X86_INS_MUL: {
//        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
//            append_ir2_opnd2i(mips_andi, dest, dest, 0xff00);
//        IR2_OPND eflags_ir2_opnd_when_upper_half_is_zero = ra_alloc_itemp_internal();
//        tr_gen_eflags_clr_bit_10(&eflags_ir2_opnd_when_upper_half_is_zero , &eflags_ir2_opnd, CF_BIT | OF_BIT);
//        append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, CF_BIT | OF_BIT);
//        append_ir2_opnd3(mips_movz, &eflags_ir2_opnd, &eflags_ir2_opnd_when_upper_half_is_zero, dest);
//        tr_skip_eflag_calculation(OF_USEDEF_BIT);
//        ra_free_temp(&eflags_ir2_opnd_when_upper_half_is_zero);
//        return;
//    }
//    case X86_INS_IMUL: {
//        IR2_OPND temp = ra_alloc_itemp();
//        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
//            append_ir2_opnd2i(mips_dsra, &temp, dest, 7);
//        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16)
//            append_ir2_opnd2i(mips_dsra, &temp, dest, 15);
//        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
//            append_ir2_opnd2i(mips_dsra, &temp, dest, 31);
//        IR2_OPND eflags_cf_of_cleared = ra_alloc_itemp();
//        tr_gen_eflags_clr_bit_10(&eflags_cf_of_cleared, &eflags_ir2_opnd, CF_BIT | OF_BIT);
//        append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, CF_BIT | OF_BIT);
//        /* if all higher bits are zero */
//        append_ir2_opnd3(mips_movz, &eflags_ir2_opnd, &eflags_cf_of_cleared, &temp);
//        /* if all higher bits are 1 */
//        append_ir2_opnd3(mips_nor, &temp, &temp, &zero_ir2_opnd);
//        append_ir2_opnd3(mips_movz, &eflags_ir2_opnd, &eflags_cf_of_cleared, &temp);
//        tr_skip_eflag_calculation(OF_USEDEF_BIT);
//        ra_free_temp(&temp);
//        ra_free_temp(&eflags_cf_of_cleared);
//        return;
//    }
    default:
        break;
    }

    /* lsenv->tr_data->curr_tb->dump(); */
    lsassertm(0, "%s for %s is not implemented\n", __FUNCTION__,
              ir1_name(ir1_opcode(pir1)));
}
static void generate_pf(IR2_OPND *dest, IR2_OPND *src0, IR2_OPND *src1)
{
    IR1_INST *pir1 = lsenv->tr_data->curr_ir1_inst;

    switch (ir1_opcode(pir1)) {
    case X86_INS_POPCNT:
        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, PF_BIT);
        return;
    default: {
        static char pf_table[256] = {
            4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4, 0, 4, 4, 0, 4, 0,
            0, 4, 4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4,
            0, 4, 4, 0, 4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,

            0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0, 4, 0, 0, 4, 0, 4,
            4, 0, 0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0,
            4, 0, 0, 4, 0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,

            0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0, 4, 0, 0, 4, 0, 4,
            4, 0, 0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0,
            4, 0, 0, 4, 0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,

            4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4, 0, 4, 4, 0, 4, 0,
            0, 4, 4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4,
            0, 4, 4, 0, 4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
        };

        IR2_OPND pf_opnd = ra_alloc_itemp();
        IR2_OPND low_byte = ra_alloc_itemp();
        load_addr_to_ir2(&pf_opnd, (ADDR)pf_table);
        append_ir2_opnd2i(LISA_ANDI, &low_byte, dest, 0xff);
        append_ir2_opnd3(LISA_ADD_D, &low_byte, &pf_opnd, &low_byte);
        append_ir2_opnd2i(LISA_LD_BU, &pf_opnd, &low_byte, 0);
        if (option_lbt) {
            append_ir2_opnd1i(LISA_X86MTFLAG, &pf_opnd, 0x2);
        } else {
            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, PF_BIT);
            append_ir2_opnd3(LISA_OR, &eflags_ir2_opnd,
                                      &eflags_ir2_opnd, &pf_opnd);
        }
        ra_free_temp(&pf_opnd);
        ra_free_temp(&low_byte);
    }
    }
}

static void generate_af(IR2_OPND *dest, IR2_OPND *src0, IR2_OPND *src1)
{
//    IR1_INST *pir1 = lsenv->tr_data->curr_ir1_inst;

//    switch (ir1_opcode(pir1)) {
//    case X86_INS_POPCNT:
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, AF_BIT);
//        return;
//    default: {
        IR2_OPND af_opnd = ra_alloc_itemp();
        if (ir2_opnd_is_imm(src1))
            append_ir2_opnd2i(LISA_XORI, &af_opnd, src0, ir2_opnd_imm(src1));
        else
            append_ir2_opnd3(LISA_XOR, &af_opnd, src0, src1);
        append_ir2_opnd3(LISA_XOR, &af_opnd, &af_opnd, dest);
        append_ir2_opnd2i(LISA_ANDI, &af_opnd, &af_opnd, 0x10);
        if (option_lbt) {
            append_ir2_opnd1i(LISA_X86MTFLAG, &af_opnd, 0x4);
        } else {
            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, AF_BIT);
            append_ir2_opnd3(LISA_OR, &eflags_ir2_opnd,
                                      &eflags_ir2_opnd, &af_opnd);
        }
        ra_free_temp(&af_opnd);
//    }
//    }
}

static void generate_zf(IR2_OPND *dest, IR2_OPND *src0, IR2_OPND *src1)
{
    IR1_INST *pir1 = lsenv->tr_data->curr_ir1_inst;

//    switch (ir1_opcode(pir1)) {
//    case X86_INS_POPCNT: {
//        IR2_OPND eflags_clear_zf = ra_alloc_itemp();
//        tr_gen_eflags_clr_bit_10(&eflags_clear_zf, &eflags_ir2_opnd, ZF_BIT);
//        append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, ZF_BIT);
//        append_ir2_opnd3(mips_movz, &eflags_ir2_opnd, &eflags_clear_zf, src0);
//        ra_free_temp(&eflags_clear_zf);
//        return;
//    }
//    default: {
        int opnd_size = ir1_opnd_size(ir1_get_opnd(pir1, 0));
        lsassert(opnd_size == 8 || opnd_size == 16 ||
                 opnd_size == 32);

        IR2_OPND extended_dest_opnd = ra_alloc_itemp();
        switch (opnd_size) {
        case 8:  append_ir2_opnd2_(lisa_mov8z,  &extended_dest_opnd, dest); break;
        case 16: append_ir2_opnd2_(lisa_mov16z, &extended_dest_opnd, dest); break;
        case 32: append_ir2_opnd2_(lisa_mov32z, &extended_dest_opnd, dest); break;
        default: lsassert(0); break;
        }

        IR2_OPND temp_eflags = ra_alloc_itemp();
        if (option_lbt) {
            append_ir2_opnd2i(LISA_ORI, &temp_eflags, &zero_ir2_opnd, 0xfff);

            IR2_OPND is_zero = ir2_opnd_new_label();
            append_ir2_opnd3 (LISA_BEQ, &extended_dest_opnd, &zero_ir2_opnd, &is_zero);
            append_ir2_opnd2_(lisa_mov, &temp_eflags, &zero_ir2_opnd);
            append_ir2_opnd1(LISA_LABEL, &is_zero);
//            append_ir2_opnd3(mips_movn, &temp_eflags, &zero_ir2_opnd, &extended_dest_opnd);

            append_ir2_opnd1i(LISA_X86MTFLAG, &temp_eflags, 0x8);
        } else {
            lsassertm(0, "generate zf not implemented without LBT\n");
//            append_ir2_opnd2i(mips_ori, &temp_eflags, &eflags_ir2_opnd, ZF_BIT);
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, ZF_BIT);
//            append_ir2_opnd3(mips_movz, &eflags_ir2_opnd, &temp_eflags, &extended_dest_opnd);
        }

        ra_free_temp(&temp_eflags);
//    }
//    }
}

static void generate_sf(IR2_OPND *dest, IR2_OPND *src0, IR2_OPND *src1)
{
    IR1_INST *pir1 = lsenv->tr_data->curr_ir1_inst;

//    switch (ir1_opcode(pir1)) {
//    case X86_INS_POPCNT:
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, SF_BIT);
//        return;
//    default: {
        int operation_size = ir1_opnd_size(ir1_get_opnd(pir1, 0));
        IR2_OPND sf_opnd = ra_alloc_itemp();

        if (operation_size > 8) {
            append_ir2_opnd2i(LISA_SRLI_D, &sf_opnd, dest, operation_size - 8);
            append_ir2_opnd2i(LISA_ANDI,   &sf_opnd, &sf_opnd, SF_BIT);
        } else {
            append_ir2_opnd2i(LISA_ANDI,   &sf_opnd, dest, SF_BIT);
        }

        if (option_lbt) {
            append_ir2_opnd1i(LISA_X86MTFLAG, &sf_opnd, 0x10);
        } else {
            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, SF_BIT);
            append_ir2_opnd3(LISA_OR, &eflags_ir2_opnd,
                                      &eflags_ir2_opnd, &sf_opnd);
        }

        ra_free_temp(&sf_opnd);
//    }
//    }
}

static void generate_of(IR2_OPND *dest, IR2_OPND *src0, IR2_OPND *src1)
{
    IR1_INST *pir1 = lsenv->tr_data->curr_ir1_inst;

    switch (ir1_opcode(pir1)) {
//    case X86_INS_ADD:
//    case X86_INS_INC: {
//        if (ir2_opnd_is_imm(src1)) {
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT);
//            /* make sure dest is sign_extension for 8 and 16 bit */
//            IR2_OPND tmp_dest = ra_alloc_itemp();
//            IR2_OPND sx_dest = *dest;
//            if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) != 32 &&
//                !ir2_opnd_is_sx(dest, ir1_opnd_size(ir1_get_opnd(pir1, 0)))) {
//                sx_dest = tmp_dest;
//                append_ir2_opnd2i(mips_sll, &tmp_dest, dest,
//                                  32 - ir1_opnd_size(ir1_get_opnd(pir1, 0)));
//                append_ir2_opnd2i(mips_sra, &tmp_dest, &tmp_dest,
//                                  32 - ir1_opnd_size(ir1_get_opnd(pir1, 0)));
//            }
//
//            IR2_OPND of_opnd = ra_alloc_itemp();
//            if (ir2_opnd_imm(src1) < 0) {
//                /* may only be downward overflow */
//                append_ir2_opnd3(mips_slt, &of_opnd, src0, &sx_dest);
//            } else if (ir2_opnd_imm(src1) > 0) {
//                /* may only be upward overflow */
//                append_ir2_opnd3(mips_slt, &of_opnd, &sx_dest, src0);
//            } else {
//                ra_free_temp(&of_opnd);
//                ra_free_temp(&tmp_dest);
//                return;
//            }
//            append_ir2_opnd2i(mips_dsll, &of_opnd, &of_opnd, 11);
//            append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &of_opnd);
//            ra_free_temp(&of_opnd);
//            ra_free_temp(&tmp_dest);
//            return;
//        } else {
//            IR2_OPND t_sign1 = ra_alloc_itemp();
//            IR2_OPND t_sign2 = ra_alloc_itemp();
//            append_ir2_opnd3(mips_xor, &t_sign1, dest, src0);
//            append_ir2_opnd3(mips_xor, &t_sign2, dest, src1);
//            append_ir2_opnd3(mips_and, &t_sign1, &t_sign1, &t_sign2);
//            if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
//                append_ir2_opnd2i(mips_dsll, &t_sign1, &t_sign1,
//                    OF_BIT_INDEX + 1 - ir1_opnd_size(ir1_get_opnd(pir1, 0)));
//            else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) <= 32)
//                append_ir2_opnd2i(mips_dsrl, &t_sign1, &t_sign1,
//                    ir1_opnd_size(ir1_get_opnd(pir1, 0)) - (OF_BIT_INDEX + 1));
//            else
//                append_ir2_opnd2i(mips_dsrl32, &t_sign1, &t_sign1,
//                    ir1_opnd_size(ir1_get_opnd(pir1, 0)) - (OF_BIT_INDEX + 1 + 32));
//            append_ir2_opnd2i(mips_andi, &t_sign1, &t_sign1, OF_BIT);
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT);
//            append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &t_sign1);
//            ra_free_temp(&t_sign1);
//            ra_free_temp(&t_sign2);
//            return;
//        }
//    }
//    case X86_INS_ADC:
//    case X86_INS_XADD: {
//        IR2_OPND t_sign1 = ra_alloc_itemp();
//        IR2_OPND t_sign2 = ra_alloc_itemp();
//        append_ir2_opnd3(mips_xor, &t_sign1, dest, src0);
//        if (ir2_opnd_is_imm(src1)) {
//            IR2_OPND temp_src1 = ra_alloc_itemp();
//            append_ir2_opnd2i(mips_daddiu, &temp_src1, &zero_ir2_opnd, ir2_opnd_imm(src1));
//            append_ir2_opnd2i(mips_dsll32, &temp_src1, &temp_src1, 16);
//            append_ir2_opnd2i(mips_dsra32, &temp_src1, &temp_src1, 16);
//            append_ir2_opnd3(mips_xor, &t_sign2, dest, &temp_src1);
//            ra_free_temp(&temp_src1);
//        } else
//            append_ir2_opnd3(mips_xor, &t_sign2, dest, src1);
//        append_ir2_opnd3(mips_and, &t_sign1, &t_sign1, &t_sign2);
//        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
//            append_ir2_opnd2i(mips_dsll, &t_sign1, &t_sign1, 4);
//        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) <= 32)
//            append_ir2_opnd2i(mips_dsrl, &t_sign1, &t_sign1,
//                              ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 12);
//        else
//            append_ir2_opnd2i(mips_dsrl32, &t_sign1, &t_sign1,
//                              ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 44);
//        append_ir2_opnd2i(mips_andi, &t_sign1, &t_sign1, 0x800);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &t_sign1);
//        ra_free_temp(&t_sign1);
//        ra_free_temp(&t_sign2);
//        return;
//    }
//    case X86_INS_DEC:
//    case X86_INS_CMP:
//    case X86_INS_SUB: {
//        if (ir2_opnd_is_imm(src1)) {
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT);
//            /* make sure dest is sign_extension for 8 and 16 bit */
//            IR2_OPND tmp_dest = ra_alloc_itemp();
//            IR2_OPND sx_dest = *dest;
//            if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) != 32 &&
//                !ir2_opnd_is_sx(dest, ir1_opnd_size(ir1_get_opnd(pir1, 0)))) {
//                sx_dest = tmp_dest;
//                append_ir2_opnd2i(mips_sll, &tmp_dest, dest,
//                                  32 - ir1_opnd_size(ir1_get_opnd(pir1, 0)));
//                append_ir2_opnd2i(mips_sra, &tmp_dest, &tmp_dest,
//                                  32 - ir1_opnd_size(ir1_get_opnd(pir1, 0)));
//            }
//            IR2_OPND of_opnd = ra_alloc_itemp();
//            if (ir2_opnd_imm(src1) > 0) {
//                /* may only be downward overflow */
//                append_ir2_opnd3(mips_slt, &of_opnd, src0, &sx_dest);
//            } else if (ir2_opnd_imm(src1) < 0) {
//                /* may only be upward overflow */
//                append_ir2_opnd3(mips_slt, &of_opnd, &sx_dest, src0);
//            } else {
//                ra_free_temp(&of_opnd);
//                ra_free_temp(&tmp_dest);
//                return;
//            }
//            append_ir2_opnd2i(mips_dsll, &of_opnd, &of_opnd, OF_BIT_INDEX);
//            append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &of_opnd);
//            ra_free_temp(&of_opnd);
//            ra_free_temp(&tmp_dest);
//            return;
//        } else {
//            IR2_OPND t_sign1 = ra_alloc_itemp();
//            IR2_OPND t_sign2 = ra_alloc_itemp();
//            append_ir2_opnd3(mips_xor, &t_sign1, src0, src1);
//            append_ir2_opnd3(mips_xor, &t_sign2, src0, dest);
//            append_ir2_opnd3(mips_and, &t_sign1, &t_sign1, &t_sign2);
//            if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
//                append_ir2_opnd2i(mips_dsll, &t_sign1, &t_sign1, 4);
//            else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) <= 32)
//                append_ir2_opnd2i(mips_dsrl, &t_sign1, &t_sign1,
//                                  ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 12);
//            else
//                append_ir2_opnd2i(mips_dsrl32, &t_sign1, &t_sign1,
//                                  ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 44);
//            append_ir2_opnd2i(mips_andi, &t_sign1, &t_sign1, 0x800);
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT);
//            append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &t_sign1);
//            ra_free_temp(&t_sign1);
//            ra_free_temp(&t_sign2);
//            return;
//        }
//        break;
//    }
//    case X86_INS_SCASB:
//    case X86_INS_SCASW:
//    case X86_INS_SCASD:    
//    case X86_INS_SBB:
//    case X86_INS_CMPSB:
//    case X86_INS_CMPSW:
//    case X86_INS_CMPSD:
//    case X86_INS_CMPXCHG: {
//        IR2_OPND t_sign1 = ra_alloc_itemp();
//        IR2_OPND t_sign2 = ra_alloc_itemp();
//        append_ir2_opnd3(mips_xor, &t_sign1, src0, src1);
//        append_ir2_opnd3(mips_xor, &t_sign2, src0, dest);
//        append_ir2_opnd3(mips_and, &t_sign1, &t_sign1, &t_sign2);
//        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
//            append_ir2_opnd2i(mips_dsll, &t_sign1, &t_sign1, 4);
//        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) <= 32)
//            append_ir2_opnd2i(mips_dsrl, &t_sign1, &t_sign1,
//                              ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 12);
//        else
//            append_ir2_opnd2i(mips_dsrl32, &t_sign1, &t_sign1,
//                              ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 44);
//
//        append_ir2_opnd2i(mips_andi, &t_sign1, &t_sign1, 0x800);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &t_sign1);
//        ra_free_temp(&t_sign1);
//        ra_free_temp(&t_sign2);
//        return;
//    }
//    case X86_INS_NEG: {
//        IR2_OPND tmp_of = ra_alloc_itemp();
//        append_ir2_opnd3(mips_and, &tmp_of, dest, src1);
//        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
//            append_ir2_opnd2i(mips_dsll, &tmp_of, &tmp_of, 4);
//        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) <= 32)
//            append_ir2_opnd2i(mips_dsrl, &tmp_of, &tmp_of,
//                              ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 12);
//        else
//            append_ir2_opnd2i(mips_dsrl32, &tmp_of, &tmp_of,
//                              ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 44);
//
//        append_ir2_opnd2i(mips_andi, &tmp_of, &tmp_of, 0x800);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &tmp_of);
//        ra_free_temp(&tmp_of);
//        return;
//    }
//    case X86_INS_OR:
//    case X86_INS_AND:
//    case X86_INS_XOR:
//    case X86_INS_TEST:
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT);
//        return;
//    case X86_INS_SHL:
//    case X86_INS_SAL: {
//        IR2_OPND label_temp = ir2_opnd_new_type(IR2_OPND_LABEL);
//        if (ir2_opnd_is_imm(src1)) {
//            if ((ir2_opnd_imm(src1) & 0x1f) != 1) {
//                return;
//            }
//        } else {
//            IR2_OPND t_dest_opnd = ra_alloc_itemp();
//            append_ir2_opnd2i(mips_ori, &t_dest_opnd, &zero_ir2_opnd, 1);
//            append_ir2_opnd3(mips_bne, src1, &t_dest_opnd, &label_temp);
//            ra_free_temp(&t_dest_opnd);
//        }
//
//        IR2_OPND t_of_opnd = ra_alloc_itemp();
//        append_ir2_opnd3(mips_xor, &t_of_opnd, dest, src0);
//        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
//            append_ir2_opnd2i(mips_dsll, &t_of_opnd, &t_of_opnd, 4);
//        else
//            append_ir2_opnd2i(mips_dsrl, &t_of_opnd, &t_of_opnd,
//                              ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 12);
//        append_ir2_opnd2i(mips_andi, &t_of_opnd, &t_of_opnd, OF_BIT);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &t_of_opnd);
//
//        append_ir2_opnd1(mips_label, &label_temp);
//        ra_free_temp(&t_of_opnd);
//        return;
//    }
//    case X86_INS_RCL: {
//        IR2_OPND t_dest_opnd = ra_alloc_itemp();
//        IR2_OPND label_temp = ir2_opnd_new_type(IR2_OPND_LABEL);
//        append_ir2_opnd2i(mips_ori, &t_dest_opnd, &zero_ir2_opnd, 1);
//        append_ir2_opnd3(mips_bne, src1, &t_dest_opnd, &label_temp);
//
//        IR2_OPND t_of_opnd = ra_alloc_itemp();
//        append_ir2_opnd2i(mips_dsrl, &t_of_opnd, src0,
//                          ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
//        append_ir2_opnd2i(mips_dsrl, &t_dest_opnd, src0,
//                          ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 2);
//        append_ir2_opnd3(mips_xor, &t_dest_opnd, &t_dest_opnd, &t_of_opnd);
//        append_ir2_opnd2i(mips_andi, &t_dest_opnd, &t_dest_opnd, 1);
//        append_ir2_opnd2i(mips_dsll, &t_dest_opnd, &t_dest_opnd, OF_BIT_INDEX);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &t_dest_opnd);
//
//        append_ir2_opnd1(mips_label, &label_temp);
//        ra_free_temp(&t_dest_opnd);
//        ra_free_temp(&t_of_opnd);
//        return;
//    }
//    case X86_INS_ROL: {
//        IR2_OPND label_temp = ir2_opnd_new_type(IR2_OPND_LABEL);
//        if (ir2_opnd_is_imm(src1)) {
//            if (ir2_opnd_imm(src1) != 1) {
//                return;
//            }
//        } else {
//            IR2_OPND t_dest_opnd = ra_alloc_itemp();
//            append_ir2_opnd2i(mips_ori, &t_dest_opnd, &zero_ir2_opnd, 1);
//            append_ir2_opnd3(mips_bne, src1, &t_dest_opnd, &label_temp);
//            ra_free_temp(&t_dest_opnd);
//        }
//        IR2_OPND t_of_opnd = ra_alloc_itemp();
//        append_ir2_opnd2i(mips_dsrl, &t_of_opnd, dest,
//                          ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
//        append_ir2_opnd3(mips_xor, &t_of_opnd, dest, &t_of_opnd);
//        append_ir2_opnd2i(mips_andi, &t_of_opnd, &t_of_opnd, 1);
//        append_ir2_opnd2i(mips_dsll, &t_of_opnd, &t_of_opnd, OF_BIT_INDEX);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &t_of_opnd);
//
//        append_ir2_opnd1(mips_label, &label_temp);
//        ra_free_temp(&t_of_opnd);
//        return;
//    }
//    case X86_INS_SAR: {
//        IR2_OPND label_temp = ir2_opnd_new_type(IR2_OPND_LABEL);
//        if (ir2_opnd_is_imm(src1)) {
//            if (ir2_opnd_imm(src1) != 1)
//                return;
//        } else {
//            IR2_OPND t_of_opnd = ra_alloc_itemp();
//            append_ir2_opnd2i(mips_ori, &t_of_opnd, &zero_ir2_opnd, 1);
//            append_ir2_opnd3(mips_bne, src1, &t_of_opnd, &label_temp);
//            ra_free_temp(&t_of_opnd);
//        }
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT);
//        append_ir2_opnd1(mips_label, &label_temp);
//        return;
//    }
//    case X86_INS_ROR: {
//        IR2_OPND label_temp = ir2_opnd_new_type(IR2_OPND_LABEL);
//        if (ir2_opnd_is_imm(src1)) {
//            if (ir2_opnd_imm(src1) != 1) {
//                return;
//            }
//        } else {
//            IR2_OPND t_dest_opnd = ra_alloc_itemp();
//            append_ir2_opnd2i(mips_ori, &t_dest_opnd, &zero_ir2_opnd, 1);
//            append_ir2_opnd3(mips_bne, src1, &t_dest_opnd, &label_temp);
//            ra_free_temp(&t_dest_opnd);
//        }
//        IR2_OPND t_of_opnd = ra_alloc_itemp();
//        append_ir2_opnd2i(mips_dsll, &t_of_opnd, dest, 1);
//        append_ir2_opnd3(mips_xor, &t_of_opnd, &t_of_opnd, dest);
//        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
//            append_ir2_opnd2i(mips_dsll, &t_of_opnd, &t_of_opnd, 4);
//        else
//            append_ir2_opnd2i(mips_dsrl, &t_of_opnd, &t_of_opnd,
//                              ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 12);
//        append_ir2_opnd2i(mips_andi, &t_of_opnd, &t_of_opnd, OF_BIT);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &t_of_opnd);
//
//        append_ir2_opnd1(mips_label, &label_temp);
//        ra_free_temp(&t_of_opnd);
//        return;
//    }
//    case X86_INS_RCR: {
//        IR2_OPND t_dest_opnd = ra_alloc_itemp();
//        IR2_OPND label_temp = ir2_opnd_new_type(IR2_OPND_LABEL);
//        append_ir2_opnd2i(mips_ori, &t_dest_opnd, &zero_ir2_opnd, 1);
//        append_ir2_opnd3(mips_bne, src1, &t_dest_opnd, &label_temp);
//
//        IR2_OPND t_of_opnd = ra_alloc_itemp();
//        append_ir2_opnd2i(mips_dsrl, &t_of_opnd, src0,
//                          ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
//        append_ir2_opnd2i(mips_dsrl, &t_dest_opnd, dest,
//                          ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 1);
//        append_ir2_opnd3(mips_xor, &t_dest_opnd, &t_of_opnd, &t_dest_opnd);
//        append_ir2_opnd2i(mips_andi, &t_dest_opnd, &t_dest_opnd, 1);
//        append_ir2_opnd2i(mips_dsll, &t_dest_opnd, &t_dest_opnd, OF_BIT_INDEX);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &t_dest_opnd);
//
//        append_ir2_opnd1(mips_label, &label_temp);
//        ra_free_temp(&t_dest_opnd);
//        ra_free_temp(&t_of_opnd);
//        return;
//    }
    case X86_INS_SHLD:
    case X86_INS_SHRD: {
        if (ir2_opnd_is_imm(src1)) {
            if (ir2_opnd_imm(src1) != 1) return;

            IR2_OPND t_of_opnd = ra_alloc_itemp();
            IR2_OPND offset = ra_alloc_itemp();

            append_ir2_opnd3(LISA_XOR, &t_of_opnd, src0, dest);
            append_ir2_opnd2i(LISA_SRLI_D, &t_of_opnd, &t_of_opnd,
                              ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 12);

            load_imm32_to_ir2(&offset, 0x800, ZERO_EXTENSION);
            append_ir2_opnd3(LISA_AND, &t_of_opnd, &t_of_opnd, &offset);
//            append_ir2_opnd2i(mips_andi, &t_of_opnd, &t_of_opnd, 0x800);

            if (option_lbt) {
                append_ir2_opnd1i(LISA_X86MTFLAG, &t_of_opnd, 0x20);
            } else {
                tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT);
                append_ir2_opnd3(LISA_OR, &eflags_ir2_opnd,
                                          &eflags_ir2_opnd, &t_of_opnd);
            }

            ra_free_temp(&t_of_opnd);
        } else {
            IR2_OPND t_of_opnd = ra_alloc_itemp();
            IR2_OPND offset = ra_alloc_itemp();
            IR2_OPND label_temp = ir2_opnd_new_label();

            append_ir2_opnd2i(LISA_ORI, &t_of_opnd, &zero_ir2_opnd, 1);
            append_ir2_opnd3(LISA_BNE, src1, &t_of_opnd, &label_temp);

            append_ir2_opnd3(LISA_XOR, &t_of_opnd, src0, dest);
            append_ir2_opnd2i(LISA_SRLI_D, &t_of_opnd, &t_of_opnd,
                              ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 12);

            load_imm32_to_ir2(&offset, 0x800, ZERO_EXTENSION);
            append_ir2_opnd3(LISA_AND, &t_of_opnd, &t_of_opnd, &offset);
//            append_ir2_opnd2i(mips_andi, &t_of_opnd, &t_of_opnd, 0x800);

            if (option_lbt) {
                append_ir2_opnd1i(LISA_X86MTFLAG, &t_of_opnd, 0x20);
            } else {
                tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT);
                append_ir2_opnd3(LISA_OR, &eflags_ir2_opnd,
                                          &eflags_ir2_opnd, &t_of_opnd);
            }

            append_ir2_opnd1(LISA_LABEL, &label_temp);
            ra_free_temp(&t_of_opnd);
        }
        return;
    }
//    case X86_INS_SHR: {
//        IR2_OPND label_temp = ir2_opnd_new_type(IR2_OPND_LABEL);
//        if (ir2_opnd_is_imm(src1)) {
//            if (ir2_opnd_imm(src1) != 1)
//                return;
//        } else {
//            IR2_OPND t_of_opnd = ra_alloc_itemp();
//            append_ir2_opnd2i(mips_ori, &t_of_opnd, &zero_ir2_opnd, 1);
//            append_ir2_opnd3(mips_bne, src1, &t_of_opnd, &label_temp);
//            ra_free_temp(&t_of_opnd);
//        }
//        IR2_OPND of_opnd = ra_alloc_itemp();
//        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
//            append_ir2_opnd2i(mips_sll, &of_opnd, src0, 4);
//        else
//            append_ir2_opnd2i(mips_srl, &of_opnd, src0,
//                              ir1_opnd_size(ir1_get_opnd(pir1, 0)) - 12);
//        append_ir2_opnd2i(mips_andi, &of_opnd, &of_opnd, 0x800);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &of_opnd);
//
//        append_ir2_opnd1(mips_label, &label_temp);
//        ra_free_temp(&of_opnd);
//        return;
//    }
//    case X86_INS_MUL: {
//        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
//            append_ir2_opnd2i(mips_andi, dest, dest, 0xff00);
//        IR2_OPND eflags_ir2_opnd_when_upper_half_is_zero =
//            ra_alloc_itemp_internal();
//        tr_gen_eflags_clr_bit_10(&eflags_ir2_opnd_when_upper_half_is_zero, &eflags_ir2_opnd, CF_BIT | OF_BIT);
//        append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, CF_BIT | OF_BIT);
//        append_ir2_opnd3(mips_movz, &eflags_ir2_opnd,
//                         &eflags_ir2_opnd_when_upper_half_is_zero, dest);
//
//        tr_skip_eflag_calculation(CF_USEDEF_BIT);
//        ra_free_temp(&eflags_ir2_opnd_when_upper_half_is_zero);
//        return;
//    }
//    case X86_INS_IMUL: {
//        IR2_OPND temp = ra_alloc_itemp();
//        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
//            append_ir2_opnd2i(mips_dsra, &temp, dest, 7);
//        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16)
//            append_ir2_opnd2i(mips_dsra, &temp, dest, 15);
//        else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
//            append_ir2_opnd2i(mips_dsra, &temp, dest, 31);
//
//        IR2_OPND eflags_cf_of_cleared = ra_alloc_itemp();
//        tr_gen_eflags_clr_bit_10(&eflags_cf_of_cleared, &eflags_ir2_opnd, CF_BIT | OF_BIT);
//        append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, CF_BIT | OF_BIT);
//        /* if all higher bits are zero */
//        append_ir2_opnd3(mips_movz, &eflags_ir2_opnd, &eflags_cf_of_cleared, &temp);
//        /* if all higher bits are 1 */
//        append_ir2_opnd3(mips_nor, &temp, &temp, &zero_ir2_opnd);
//        append_ir2_opnd3(mips_movz, &eflags_ir2_opnd, &eflags_cf_of_cleared, &temp);
//        tr_skip_eflag_calculation(CF_USEDEF_BIT);
//        ra_free_temp(&temp);
//        ra_free_temp(&eflags_cf_of_cleared);
//        return;
//    }
    default:
        break;
    }

    /* lsenv->tr_data->curr_tb->dump(); */
    lsassertm(0, "%s for %s is not implemented\n", __FUNCTION__,
              ir1_name(ir1_opcode(pir1)));
}

static void generate_cf_not_sx(IR2_OPND *dest, IR2_OPND *src0, IR2_OPND *src1)
{
    lsassertm(0, "EFLAGS gen cf not sx  to be implemented in LoongArch.\n");
//    IR1_INST *pir1 = lsenv->tr_data->curr_ir1_inst;
//
//    switch (ir1_opcode(pir1)) {
//    case X86_INS_OR:
//    case X86_INS_AND:
//    case X86_INS_XOR:
//    case X86_INS_TEST:
//    case X86_INS_POPCNT:
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, CF_BIT);
//        return;
//    case X86_INS_ADD:
//    case X86_INS_ADC:
//    case X86_INS_CMP:
//    case X86_INS_SUB: {
//        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8 ||
//                 ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16 ||
//                 ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32);
//        IR2_OPND cf_opnd = ra_alloc_itemp();
//        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) != 32)
//            append_ir2_opnd2i(mips_dsra, &cf_opnd, dest,
//                              ir1_opnd_size(ir1_get_opnd(pir1, 0)));
//        else
//            append_ir2_opnd2i(mips_dsra32, &cf_opnd, dest, 0);
//        if (!ir2_opnd_is_zx(&cf_opnd, 1))
//            append_ir2_opnd2i(mips_andi, &cf_opnd, &cf_opnd, CF_BIT);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, CF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &cf_opnd);
//        ra_free_temp(&cf_opnd);
//        return;
//    }
//    case X86_INS_SHR: {
//        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8 ||
//                 ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16 ||
//                 ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32);
//        IR2_OPND cf_opnd = ra_alloc_itemp();
//        IR2_OPND ir2_opnd_tmp;
//        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, ir2_opnd_imm(src1) - 1);
//        append_ir2_opnd3(mips_srl, &cf_opnd, src0, &ir2_opnd_tmp);
//        append_ir2_opnd2i(mips_andi, &cf_opnd, &cf_opnd, CF_BIT);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, CF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &cf_opnd);
//        ra_free_temp(&cf_opnd);
//        return;
//    }
//    case X86_INS_SHL: {
//        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8 ||
//                 ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16 ||
//                 ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32);
//        IR2_OPND cf_opnd = ra_alloc_itemp();
//        IR2_OPND ir2_opnd_tmp;
//        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, 32 - ir2_opnd_imm(src1));
//        append_ir2_opnd3(mips_srl, &cf_opnd, src0, &ir2_opnd_tmp);
//        append_ir2_opnd2i(mips_andi, &cf_opnd, &cf_opnd, CF_BIT);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, CF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &cf_opnd);
//        ra_free_temp(&cf_opnd);
//        return;
//    }
//    default:
//        break;
//    }
//
//    /* lsenv->tr_data->curr_tb->dump(); */
//    lsassertm(0, "%s for %s is not implemented\n", __FUNCTION__,
//              ir1_name(ir1_opcode(pir1)));
}

static void generate_of_not_sx(IR2_OPND *dest, IR2_OPND *src0, IR2_OPND *src1)
{
    lsassertm(0, "EFLAGS gen of not sx  to be implemented in LoongArch.\n");
//    IR1_INST *pir1 = lsenv->tr_data->curr_ir1_inst;
//
//    switch (ir1_opcode(pir1)) {
//    case X86_INS_OR:
//    case X86_INS_AND:
//    case X86_INS_XOR:
//    case X86_INS_TEST:
//    case X86_INS_POPCNT:
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT);
//        return;
//    case X86_INS_ADD:
//    case X86_INS_SUB: {
//        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32);
//        IR2_OPND of_opnd = ra_alloc_itemp();
//        /* since it is unsigned add/sub, so set OF = bit32 */
//        append_ir2_opnd2i(mips_dsrl, &of_opnd, dest, 32 - OF_BIT_INDEX);
//        append_ir2_opnd2i(mips_andi, &of_opnd, &of_opnd, OF_BIT);
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT);
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &of_opnd);
//        ra_free_temp(&of_opnd);
//        return;
//    }
//    default:
//        break;
//    }
//
//    /* lsenv->tr_data->curr_tb->dump(); */
//    lsassertm(0, "%s for %s is not implemented\n", __FUNCTION__,
//              ir1_name(ir1_opcode(pir1)));
}

void generate_eflag_calculation(
        IR2_OPND *dest, IR2_OPND *src0, IR2_OPND *src1,
        IR1_INST *pir1, bool is_sx)
{
    if (ir1_get_eflag_def(pir1) == 0)
        return;
#ifndef CONFIG_SOFTMMU
    fp_save_dest_opnd(pir1, *dest);
#endif
    if (ir1_need_calculate_any_flag(pir1) == 0 ||
        (option_lbt && generate_eflag_by_lbt(dest, src0, src1, pir1, is_sx)))
        return;

    /* extension mode does not affect pf, af and zf */
    if (ir1_need_calculate_pf(pir1))
        generate_pf(dest, src0, src1);
    if (ir1_need_calculate_af(pir1))
        generate_af(dest, src0, src1);
    if (ir1_need_calculate_zf(pir1))
        generate_zf(dest, src0, src1);
    if (ir1_need_calculate_sf(pir1))
        generate_sf(dest, src0, src1);

    /* calculate cf and of separately */
    if (is_sx) {
        if (ir1_need_calculate_cf(pir1))
            generate_cf(dest, src0, src1);
        if (ir1_need_calculate_of(pir1))
            generate_of(dest, src0, src1);
    } else {
        if (ir1_need_calculate_cf(pir1))
            generate_cf_not_sx(dest, src0, src1);
        if (ir1_need_calculate_of(pir1))
            generate_of_not_sx(dest, src0, src1);
    }
}

void tr_skip_eflag_calculation(int usedef_bits)
{
    BITS_SET(lsenv->tr_data->curr_ir1_skipped_eflags, usedef_bits);
}
bool ir1_need_calculate_cf(IR1_INST *ir1)
{
    return ir1_is_cf_def(ir1) &&
           BITS_ARE_CLEAR(lsenv->tr_data->curr_ir1_skipped_eflags,
                          1 << CF_USEDEF_BIT_INDEX);
}
bool ir1_need_calculate_pf(IR1_INST *ir1)
{
    return ir1_is_pf_def(ir1) &&
           BITS_ARE_CLEAR(lsenv->tr_data->curr_ir1_skipped_eflags,
                          1 << PF_USEDEF_BIT_INDEX);
}
bool ir1_need_calculate_af(IR1_INST *ir1)
{
    return ir1_is_af_def(ir1) &&
           BITS_ARE_CLEAR(lsenv->tr_data->curr_ir1_skipped_eflags,
                          1 << AF_USEDEF_BIT_INDEX);
}
bool ir1_need_calculate_zf(IR1_INST *ir1)
{
    return ir1_is_zf_def(ir1) &&
           BITS_ARE_CLEAR(lsenv->tr_data->curr_ir1_skipped_eflags,
                          1 << ZF_USEDEF_BIT_INDEX);
}
bool ir1_need_calculate_sf(IR1_INST *ir1)
{
    return ir1_is_sf_def(ir1) &&
           BITS_ARE_CLEAR(lsenv->tr_data->curr_ir1_skipped_eflags,
                          1 << SF_USEDEF_BIT_INDEX);
}
bool ir1_need_calculate_of(IR1_INST *ir1)
{
    return ir1_is_of_def(ir1) &&
           BITS_ARE_CLEAR(lsenv->tr_data->curr_ir1_skipped_eflags,
                          1 << OF_USEDEF_BIT_INDEX);
}
bool ir1_need_calculate_any_flag(IR1_INST *ir1)
{
    return (ir1_get_eflag_def(ir1) &
            ~(lsenv->tr_data->curr_ir1_skipped_eflags)) != 0;
}
