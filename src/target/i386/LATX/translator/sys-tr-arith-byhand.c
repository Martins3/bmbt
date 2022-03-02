#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>

static void latxs_translate_cmp_byhand_gpr_gpr(
        IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND reg0 = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
    IR2_OPND reg1 = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));

    if (ir1_opnd_is_8h(opnd0)) {
        IR2_OPND tmp0 = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_SRLI_D, &tmp0, &reg0, 8);
        if (ir1_opnd_is_8h(opnd1)) {
            IR2_OPND tmp1 = latxs_ra_alloc_itemp();
            latxs_append_ir2_opnd2i(LISA_SRLI_D, &tmp1, &reg1, 8);
            latxs_append_ir2_opnd2(LISA_X86SUB_B, &tmp0, &tmp1);
        } else {
            latxs_append_ir2_opnd2(LISA_X86SUB_B, &tmp0, &reg1);
        }
    } else {
        if (ir1_opnd_is_8h(opnd1)) {
            IR2_OPND tmp1 = latxs_ra_alloc_itemp();
            latxs_append_ir2_opnd2i(LISA_SRLI_D, &tmp1, &reg1, 8);
            latxs_append_ir2_opnd2(LISA_X86SUB_B, &reg0, &tmp1);
        } else {
            int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
            latxs_append_ir2_opnd2(LISA_X86SUB_B + (os >> 4), &reg0, &reg1);
        }
    }
}

static void latxs_translate_cmp_byhand_gpr_imm(
        IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND reg0 = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));

    IR2_OPND reg1;
    if (ir1_opnd_is_imm(opnd1) && ir1_opnd_simm(opnd1) == 0) {
        reg1 = latxs_zero_ir2_opnd;
    } else {
        reg1 = latxs_ra_alloc_itemp();
        latxs_load_ir1_imm_to_ir2(&reg1, opnd1, EXMode_S);
    }

    if (ir1_opnd_is_8h(opnd0)) {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_SRLI_D, &tmp, &reg0, 8);
        latxs_append_ir2_opnd2(LISA_X86SUB_B, &tmp, &reg1);
    } else {
        int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
        latxs_append_ir2_opnd2(LISA_X86SUB_B + (os >> 4), &reg0, &reg1);
    }
}

static void latxs_translate_cmp_byhand_gpr_mem(
        IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND reg0 = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));

    IR2_OPND reg1 = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&reg1, opnd1, EXMode_N, -1);

    if (ir1_opnd_is_8h(opnd0)) {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_SRLI_D, &tmp, &reg0, 8);
        latxs_append_ir2_opnd2(LISA_X86SUB_B, &tmp, &reg1);
    } else {
        int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
        latxs_append_ir2_opnd2(LISA_X86SUB_B + (os >> 4), &reg0, &reg1);
    }
}

static void latxs_translate_cmp_byhand_mem_gpr(
        IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND reg0 = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&reg0, opnd0, EXMode_N, -1);

    IR2_OPND reg1 = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));

    if (ir1_opnd_is_8h(opnd1)) {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_SRLI_D, &tmp, &reg1, 8);
        latxs_append_ir2_opnd2(LISA_X86SUB_B, &reg0, &tmp);
    } else {
        int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
        latxs_append_ir2_opnd2(LISA_X86SUB_B + (os >> 4), &reg0, &reg1);
    }
}

static void latxs_translate_cmp_byhand_mem_imm(
        IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND reg0 = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&reg0, opnd0, EXMode_N, -1);

    IR2_OPND reg1;
    if (ir1_opnd_is_imm(opnd1) && ir1_opnd_simm(opnd1) == 0) {
        reg1 = latxs_zero_ir2_opnd;
    } else {
        reg1 = latxs_ra_alloc_itemp();
        latxs_load_ir1_imm_to_ir2(&reg1, opnd1, EXMode_S);
    }

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    latxs_append_ir2_opnd2(LISA_X86SUB_B + (os >> 4), &reg0, &reg1);
}

bool latxs_translate_cmp_byhand(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    /*
     * 5 types of cmp instruction
     *
     * > gpr, imm # same size or signed(imm8)
     * > gpr, gpr # same size
     * > gpr, mem # same size
     *
     * > mem, imm # same size or signed(imm8)
     * > mem, gpr # same size
     */

    if (ir1_opnd_is_gpr(opnd0)) {
        if (ir1_opnd_is_gpr(opnd1)) {
            latxs_translate_cmp_byhand_gpr_gpr(opnd0, opnd1);
        } else if (ir1_opnd_is_imm(opnd1)) {
            latxs_translate_cmp_byhand_gpr_imm(opnd0, opnd1);
        } else {
            latxs_translate_cmp_byhand_gpr_mem(opnd0, opnd1);
        }
    } else {
        if (ir1_opnd_is_gpr(opnd1)) {
            latxs_translate_cmp_byhand_mem_gpr(opnd0, opnd1);
        } else {
            latxs_translate_cmp_byhand_mem_imm(opnd0, opnd1);
        }
    }

    return true;
}





static void latxs_translate_add_sub_byhand_gpr_gpr(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op)
{
    IR2_OPND reg0 = latxs_convert_gpr_opnd(opnd0, EXMode_S);
    IR2_OPND reg1 = latxs_convert_gpr_opnd(opnd1, EXMode_S);

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    latxs_append_ir2_opnd2(lbt_op + (os >> 4), &reg0, &reg1);

    latxs_append_ir2_opnd3(lisa_op, &reg0, &reg0, &reg1);

    if (latxs_ir2_opnd_is_reg_temp(&reg0) || os != 32) {
        latxs_store_ir2_to_ir1_gpr_em(&reg0, opnd0);
    } else {
        latxs_ir2_opnd_set_emb(&reg0, EXMode_S, 32);
    }
}

static void latxs_translate_adc_sbc_byhand_gpr_gpr(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op)
{
    IR2_OPND reg0 = latxs_convert_gpr_opnd(opnd0, EXMode_N);
    IR2_OPND reg1 = latxs_convert_gpr_opnd(opnd1, EXMode_N);
    IR2_OPND res  = latxs_ra_alloc_itemp();

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    latxs_append_ir2_opnd3(lisa_op + (os >> 4), &res,  &reg0, &reg1);
    latxs_append_ir2_opnd2(lbt_op  + (os >> 4), &reg0, &reg1);

    latxs_store_ir2_to_ir1_gpr_em(&res, opnd0);
}

static void latxs_translate_add_sub_byhand_gpr_imm(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op)
{
    IR2_OPND reg0 = latxs_convert_gpr_opnd(opnd0, EXMode_S);

    IR2_OPND reg1 = latxs_ra_alloc_itemp();
    latxs_load_ir1_imm_to_ir2(&reg1, opnd1, EXMode_S);

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    latxs_append_ir2_opnd2(lbt_op + (os >> 4), &reg0, &reg1);

    latxs_append_ir2_opnd3(lisa_op, &reg0, &reg0, &reg1);

    if (latxs_ir2_opnd_is_reg_temp(&reg0) || os != 32) {
        latxs_store_ir2_to_ir1_gpr_em(&reg0, opnd0);
    } else {
        latxs_ir2_opnd_set_emb(&reg0, EXMode_S, 32);
    }
}

static void latxs_translate_adc_sbc_byhand_gpr_imm(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op)
{
    IR2_OPND reg0 = latxs_convert_gpr_opnd(opnd0, EXMode_N);
    IR2_OPND reg1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_imm_to_ir2(&reg1, opnd1, EXMode_S);

    IR2_OPND res  = latxs_ra_alloc_itemp();
    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    latxs_append_ir2_opnd3(lisa_op + (os >> 4), &res, &reg0, &reg1);
    latxs_append_ir2_opnd2(lbt_op  + (os >> 4), &reg0, &reg1);

    latxs_store_ir2_to_ir1_gpr_em(&res, opnd0);
}

static void latxs_translate_add_sub_byhand_gpr_mem(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op)
{
    IR2_OPND reg0 = latxs_convert_gpr_opnd(opnd0, EXMode_S);

    IR2_OPND reg1 = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&reg1, opnd1, EXMode_S, -1);

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    latxs_append_ir2_opnd2(lbt_op + (os >> 4), &reg0, &reg1);

    latxs_append_ir2_opnd3(lisa_op, &reg0, &reg0, &reg1);

    if (latxs_ir2_opnd_is_reg_temp(&reg0) || os != 32) {
        latxs_store_ir2_to_ir1_gpr_em(&reg0, opnd0);
    } else {
        latxs_ir2_opnd_set_emb(&reg0, EXMode_S, 32);
    }
}

static void latxs_translate_adc_sbc_byhand_gpr_mem(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op)
{
    IR2_OPND reg0 = latxs_convert_gpr_opnd(opnd0, EXMode_N);
    IR2_OPND reg1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_mem_to_ir2(&reg1, opnd1, EXMode_S, -1);

    IR2_OPND res  = latxs_ra_alloc_itemp();
    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    latxs_append_ir2_opnd3(lisa_op + (os >> 4), &res,  &reg0, &reg1);
    latxs_append_ir2_opnd2(lbt_op  + (os >> 4), &reg0, &reg1);

    latxs_store_ir2_to_ir1_gpr_em(&res, opnd0);
}

static void latxs_translate_add_sub_byhand_mem_gpr(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op)
{
    IR2_OPND reg0 = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&reg0, opnd0, EXMode_S, -1);

    IR2_OPND reg1 = latxs_convert_gpr_opnd(opnd1, EXMode_S);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(lisa_op, &dest, &reg0, &reg1);

    latxs_store_ir2_to_ir1_mem(&dest, opnd0, -1);

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    latxs_append_ir2_opnd2(lbt_op + (os >> 4), &reg0, &reg1);
}

static void latxs_translate_adc_sbc_byhand_mem_gpr(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op)
{
    IR2_OPND reg0 = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&reg0, opnd0, EXMode_S, -1);

    IR2_OPND reg1 = latxs_convert_gpr_opnd(opnd1, EXMode_N);

    IR2_OPND res  = latxs_ra_alloc_itemp();
    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */

    latxs_append_ir2_opnd3(lisa_op + (os >> 4), &res, &reg0, &reg1);

    latxs_store_ir2_to_ir1_mem(&res, opnd0, -1);

    latxs_append_ir2_opnd2(lbt_op + (os >> 4), &reg0, &reg1);
}

static void latxs_translate_add_sub_byhand_mem_imm(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op)
{
    IR2_OPND reg0 = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&reg0, opnd0, EXMode_S, -1);

    IR2_OPND reg1 = latxs_ra_alloc_itemp();
    latxs_load_ir1_imm_to_ir2(&reg1, opnd1, EXMode_S);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(lisa_op, &dest, &reg0, &reg1);

    latxs_store_ir2_to_ir1_mem(&dest, opnd0, -1);

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    latxs_append_ir2_opnd2(lbt_op + (os >> 4), &reg0, &reg1);
}

static void latxs_translate_adc_sbc_byhand_mem_imm(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op)
{
    IR2_OPND reg0 = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&reg0, opnd0, EXMode_S, -1);

    IR2_OPND reg1 = latxs_ra_alloc_itemp();
    latxs_load_ir1_imm_to_ir2(&reg1, opnd1, EXMode_S);

    IR2_OPND res  = latxs_ra_alloc_itemp();
    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    latxs_append_ir2_opnd3(lisa_op + (os >> 4), &res, &reg0, &reg1);

    latxs_store_ir2_to_ir1_mem(&res, opnd0, -1);

    latxs_append_ir2_opnd2(lbt_op + (os >> 4), &reg0, &reg1);
}

static inline bool latxs_translate_add_sub_byhand(
        IR1_INST   *pir1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    /*
     * 5 types of xor,and,or instruction
     *
     * > gpr, imm # same size or signed(imm8)
     * > gpr, gpr # same size
     * > gpr, mem # same size
     *
     * > mem, imm # same size or signed(imm8)
     * > mem, gpr # same size
     */

    if (ir1_opnd_is_gpr(opnd0)) {
        if (ir1_opnd_is_gpr(opnd1)) {
            latxs_translate_add_sub_byhand_gpr_gpr(opnd0, opnd1,
                    lisa_op, lbt_op);
        } else if (ir1_opnd_is_imm(opnd1)) {
            latxs_translate_add_sub_byhand_gpr_imm(opnd0, opnd1,
                    lisa_op, lbt_op);
        } else {
            latxs_translate_add_sub_byhand_gpr_mem(opnd0, opnd1,
                    lisa_op, lbt_op);
        }
    } else {
        if (ir1_opnd_is_gpr(opnd1)) {
            latxs_translate_add_sub_byhand_mem_gpr(opnd0, opnd1,
                    lisa_op, lbt_op);
        } else {
            latxs_translate_add_sub_byhand_mem_imm(opnd0, opnd1,
                    lisa_op, lbt_op);
        }
    }

    return true;
}


static inline bool latxs_translate_adc_sbc_byhand(
        IR1_INST   *pir1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    /*
     * 5 types of xor,and,or instruction
     *
     * > gpr, imm # same size or signed(imm8)
     * > gpr, gpr # same size
     * > gpr, mem # same size
     *
     * > mem, imm # same size or signed(imm8)
     * > mem, gpr # same size
     */

    if (ir1_opnd_is_gpr(opnd0)) {
        if (ir1_opnd_is_gpr(opnd1)) {
            latxs_translate_adc_sbc_byhand_gpr_gpr(opnd0, opnd1,
                    lisa_op, lbt_op);
        } else if (ir1_opnd_is_imm(opnd1)) {
            latxs_translate_adc_sbc_byhand_gpr_imm(opnd0, opnd1,
                    lisa_op, lbt_op);
        } else {
            latxs_translate_adc_sbc_byhand_gpr_mem(opnd0, opnd1,
                    lisa_op, lbt_op);
        }
    } else {
        if (ir1_opnd_is_gpr(opnd1)) {
            latxs_translate_adc_sbc_byhand_mem_gpr(opnd0, opnd1,
                    lisa_op, lbt_op);
        } else {
            latxs_translate_adc_sbc_byhand_mem_imm(opnd0, opnd1,
                    lisa_op, lbt_op);
        }
    }

    return true;
}



bool latxs_translate_add_byhand(IR1_INST *pir1)
{
    return latxs_translate_add_sub_byhand(pir1, LISA_ADD_W, LISA_X86ADD_B);
}

bool latxs_translate_adc_byhand(IR1_INST *pir1)
{
    return latxs_translate_adc_sbc_byhand(pir1, LISA_ADC_B, LISA_X86ADC_B);
}

bool latxs_translate_sub_byhand(IR1_INST *pir1)
{
    return latxs_translate_add_sub_byhand(pir1, LISA_SUB_W, LISA_X86SUB_B);
}

bool latxs_translate_sbb_byhand(IR1_INST *pir1)
{
    return latxs_translate_adc_sbc_byhand(pir1, LISA_SBC_B, LISA_X86SBC_B);
}





static void latxs_translate_inc_dec_byhand_gpr(
        IR1_OPND   *opnd0,
        IR2_OPCODE  lbt_op,
        int         addend)
{
    IR2_OPND gpr = latxs_convert_gpr_opnd(opnd0, EXMode_S);

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    latxs_append_ir2_opnd1(lbt_op + (os >> 4), &gpr);

    latxs_append_ir2_opnd2i(LISA_ADDI_W, &gpr, &gpr, addend);

    if (latxs_ir2_opnd_is_reg_temp(&gpr) || os != 32) {
        latxs_store_ir2_to_ir1_gpr_em(&gpr, opnd0);
    } else {
        latxs_ir2_opnd_set_emb(&gpr, EXMode_S, 32);
    }
}

static void latxs_translate_inc_dec_byhand_mem(
        IR1_OPND   *opnd0,
        IR2_OPCODE  lbt_op,
        int         addend)
{
    IR2_OPND tmp = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&tmp, opnd0, EXMode_S, -1);

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */

    IR2_OPND dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ADDI_W, &dest, &tmp, addend);

    latxs_store_ir2_to_ir1_mem(&dest, opnd0, -1);

    latxs_append_ir2_opnd1(lbt_op + (os >> 4), &tmp);
}

static bool latxs_translate_inc_dec_byhand(
        IR1_INST   *pir1,
        IR2_OPCODE  lbt_op,
        int         addend)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    if (ir1_opnd_is_gpr(opnd0)) {
        latxs_translate_inc_dec_byhand_gpr(opnd0, lbt_op, addend);
    } else {
        latxs_translate_inc_dec_byhand_mem(opnd0, lbt_op, addend);
    }

    return true;
}

bool latxs_translate_inc_byhand(IR1_INST *pir1)
{
    return latxs_translate_inc_dec_byhand(pir1, LISA_X86INC_B, 1);
}

bool latxs_translate_dec_byhand(IR1_INST *pir1)
{
    return latxs_translate_inc_dec_byhand(pir1, LISA_X86DEC_B, -1);
}





static void latxs_translate_neg_byhand_gpr(IR1_OPND *opnd0)
{
    IR2_OPND gpr = latxs_convert_gpr_opnd(opnd0, EXMode_S);

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    latxs_append_ir2_opnd2(LISA_X86SUB_B + (os >> 4),
            &latxs_zero_ir2_opnd, &gpr);

    latxs_append_ir2_opnd3(LISA_SUB_W, &gpr,
            &latxs_zero_ir2_opnd, &gpr);

    if (latxs_ir2_opnd_is_reg_temp(&gpr)) {
        latxs_store_ir2_to_ir1_gpr_em(&gpr, opnd0);
    } else {
        latxs_ir2_opnd_set_emb(&gpr, EXMode_S, os);
    }
}

static void latxs_translate_neg_byhand_mem(IR1_OPND *opnd0)
{
    IR2_OPND tmp = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&tmp, opnd0, EXMode_S, -1);

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */

    IR2_OPND dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SUB_W, &dest,
            &latxs_zero_ir2_opnd, &tmp);

    latxs_store_ir2_to_ir1_mem(&dest, opnd0, -1);

    latxs_append_ir2_opnd2(LISA_X86SUB_B + (os >> 4),
            &latxs_zero_ir2_opnd, &tmp);
}

bool latxs_translate_neg_byhand(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    if (ir1_opnd_is_gpr(opnd0)) {
        latxs_translate_neg_byhand_gpr(opnd0);
    } else {
        latxs_translate_neg_byhand_mem(opnd0);
    }

    return true;
}





static void __latxs_translate_mul_byhand_eax(
        IR2_OPND *data, int os,
        EXMode eax_em,
        IR2_OPCODE lbt_op)
{
    IR2_OPND res = latxs_ra_alloc_itemp();

    IR2_OPND eax;
    switch (os) {
    case 8:
        eax = latxs_convert_gpr_opnd(&al_ir1_opnd, eax_em);
        latxs_append_ir2_opnd2(lbt_op + (os >> 4), data, &eax);

        latxs_append_ir2_opnd3(LISA_MUL_W, &res, data, &eax);

        latxs_store_ir2_to_ir1_gpr_em(&res, &ax_ir1_opnd);
        break;
    case 16:
        eax = latxs_convert_gpr_opnd(&ax_ir1_opnd, eax_em);
        latxs_append_ir2_opnd2(lbt_op + (os >> 4), data, &eax);

        latxs_append_ir2_opnd3(LISA_MUL_W, &res, data, &eax);

        latxs_store_ir2_to_ir1_gpr_em(&res, &ax_ir1_opnd);
        latxs_append_ir2_opnd2i(LISA_SRLI_W, &res, &res, 16);
        latxs_store_ir2_to_ir1_gpr_em(&res, &dx_ir1_opnd);
        break;
    case 32:
        eax = latxs_convert_gpr_opnd(&eax_ir1_opnd, eax_em);
        latxs_append_ir2_opnd2(lbt_op + (os >> 4), data, &eax);

        latxs_append_ir2_opnd3(LISA_MUL_D, &res, data, &eax);

        latxs_store_ir2_to_ir1_gpr_em(&res, &eax_ir1_opnd);
        latxs_append_ir2_opnd2i(LISA_SRLI_D, &res, &res, 32);
        latxs_store_ir2_to_ir1_gpr_em(&res, &edx_ir1_opnd);
        break;
    default:
        lsassert(0);
        break;
    }
}





static void latxs_translate_mul_byhand_gpr(IR1_OPND *opnd0)
{
    int os = ir1_opnd_size(opnd0);
    IR2_OPND gpr = latxs_convert_gpr_opnd(opnd0, EXMode_Z);
    __latxs_translate_mul_byhand_eax(&gpr, os, EXMode_Z, LISA_X86MUL_BU);
}

static void latxs_translate_mul_byhand_mem(IR1_OPND *opnd0)
{
    int os = ir1_opnd_size(opnd0);
    IR2_OPND mem = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&mem, opnd0, EXMode_Z, -1);
    __latxs_translate_mul_byhand_eax(&mem, os, EXMode_Z, LISA_X86MUL_BU);
}

bool latxs_translate_mul_byhand(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    if (ir1_opnd_is_gpr(opnd0)) {
        latxs_translate_mul_byhand_gpr(opnd0);
    } else {
        latxs_translate_mul_byhand_mem(opnd0);
    }

    return true;
}




static void latxs_translate_imul_byhand_1(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR/MEM */

    int os = ir1_opnd_size(opnd0);
    if (ir1_opnd_is_gpr(opnd0)) {
        IR2_OPND gpr = latxs_convert_gpr_opnd(opnd0, EXMode_S);
        __latxs_translate_mul_byhand_eax(&gpr, os, EXMode_S, LISA_X86MUL_B);
    } else {
        IR2_OPND mem = latxs_ra_alloc_itemp();
        latxs_load_ir1_mem_to_ir2(&mem, opnd0, EXMode_S, -1);
        __latxs_translate_mul_byhand_eax(&mem, os, EXMode_S, LISA_X86MUL_B);
        latxs_ra_free_temp(&mem);
    }
}

static void latxs_translate_imul_byhand_2(IR1_INST *pir1)
{
    /* DEST <- truncated(DEST * SRC) */
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int os = ir1_opnd_size(opnd0);

    IR2_OPND dest = latxs_convert_gpr_opnd(opnd0, EXMode_S);
    IR2_OPND src;
    if (ir1_opnd_is_gpr(opnd1)) {
        src = latxs_convert_gpr_opnd(opnd1, EXMode_S);
    } else if (ir1_opnd_is_mem(opnd1)) {
        src = latxs_ra_alloc_itemp();
        latxs_load_ir1_mem_to_ir2(&src, opnd1, EXMode_S, -1);
    } else if (ir1_opnd_is_imm(opnd1)) {
        src = latxs_ra_alloc_itemp();
        latxs_load_ir1_imm_to_ir2(&src, opnd1, EXMode_S);
    } else {
        lsassert(0);
    }

    latxs_append_ir2_opnd2(LISA_X86MUL_B + (os >> 4), &dest, &src);

    IR2_OPND res = latxs_ra_alloc_itemp();
    if (os == 32) {
        latxs_append_ir2_opnd3(LISA_MUL_D, &res, &dest, &src);
    } else {
        latxs_append_ir2_opnd3(LISA_MUL_W, &res, &dest, &src);
    }

    latxs_store_ir2_to_ir1_gpr_em(&res, opnd0);
}

static void latxs_translate_imul_byhand_3(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */
    IR1_OPND *opnd2 = ir1_get_opnd(pir1, 2); /* imm */

    IR2_OPND res, src1, src2;

    int os = ir1_opnd_size(opnd0);

    if (ir1_opnd_is_gpr(opnd1)) {
        src1 = latxs_convert_gpr_opnd(opnd1, EXMode_S);
    } else {
        src1 = latxs_ra_alloc_itemp();
        latxs_load_ir1_mem_to_ir2(&src1, opnd1, EXMode_S, -1);
    }

    src2 = latxs_ra_alloc_itemp();
    latxs_load_ir1_imm_to_ir2(&src2, opnd2, EXMode_S);

    latxs_append_ir2_opnd2(LISA_X86MUL_B + (os >> 4), &src1, &src2);

    res = latxs_ra_alloc_itemp();
    if (os == 32) {
        latxs_append_ir2_opnd3(LISA_MUL_D, &res, &src1, &src2);
    } else {
        latxs_append_ir2_opnd3(LISA_MUL_W, &res, &src1, &src2);
    }

    latxs_store_ir2_to_ir1_gpr_em(&res, opnd0);
}

bool latxs_translate_imul_byhand(IR1_INST *pir1)
{
    switch (ir1_opnd_num(pir1)) {
    case 1:
        latxs_translate_imul_byhand_1(pir1);
        break;
    case 2:
        latxs_translate_imul_byhand_2(pir1);
        break;
    case 3:
        latxs_translate_imul_byhand_3(pir1);
        break;
    default:
        lsassert(0);
        break;
    }

    return true;
}





static void __latxs_translate_div_byhand(
        IR1_INST *pir1, int os,
        IR2_OPND *data, int is_idiv)
{
    IR2_OPND div_res = latxs_ra_alloc_itemp();
    IR2_OPND div_mod = latxs_ra_alloc_itemp();

    IR2_OPND src, eax, edx;

    EXMode em = is_idiv ? EXMode_S : EXMode_Z;
    IR2_OPCODE div_op = is_idiv ? LISA_DIV_D : LISA_DIV_DU;
    IR2_OPCODE mod_op = is_idiv ? LISA_MOD_D : LISA_MOD_DU;
    /*
     * Why we do NOT use LISA_DIV_W(U) with x86 GPR directly:
     *
     *   > For 32 operand size, the calculation is EDX:EAX / data,
     *     which is a 64-bit div operation
     *
     *   > For 8/16 operand size, we could use the GPR directly
     *     only if the Extension Bits of GPR is less than or equal to
     *     the operand size AND the Extension Mode is of GPR is sign/zero.
     *     If so, the latxs_convert_gpr_opnd() will return the GPR directly.
     */

    switch (os) {
    case  8: /* AX / data */
        src = latxs_convert_gpr_opnd(&ax_ir1_opnd, em);

        latxs_append_ir2_opnd3(div_op, &div_res, &src, data);
        latxs_append_ir2_opnd3(mod_op, &div_mod, &src, data);

        latxs_tr_gen_div_result_check(pir1, div_res, 8, is_idiv);

        latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &div_res, &div_mod, 15, 8);
        latxs_store_ir2_to_ir1_gpr_em(&div_res, &ax_ir1_opnd);
        break;
    case 16: /* DX:AX / data */
        eax = latxs_convert_gpr_opnd(&ax_ir1_opnd, EXMode_Z);
        edx = latxs_convert_gpr_opnd(&dx_ir1_opnd, em);
        src = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &src, &edx, 16);
        latxs_append_ir2_opnd3(LISA_OR,      &src, &src, &eax);

        latxs_append_ir2_opnd3(div_op, &div_res, &src, data);
        latxs_append_ir2_opnd3(mod_op, &div_mod, &src, data);
        latxs_ra_free_temp(&src);

        latxs_tr_gen_div_result_check(pir1, div_res, 16, is_idiv);

        latxs_store_ir2_to_ir1_gpr_em(&div_mod, &dx_ir1_opnd);
        latxs_store_ir2_to_ir1_gpr_em(&div_res, &ax_ir1_opnd);

        /* convert gpr opnd might return temp */
        latxs_ra_free_temp(&eax);
        latxs_ra_free_temp(&edx);
        break;
    case 32: /* EDX:EAX / data */
        eax = latxs_convert_gpr_opnd(&eax_ir1_opnd, EXMode_Z);
        edx = latxs_convert_gpr_opnd(&edx_ir1_opnd, em);
        src = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &src, &edx, 32);
        latxs_append_ir2_opnd3(LISA_OR,      &src, &src, &eax);

        latxs_append_ir2_opnd3(div_op, &div_res, &src, data);
        latxs_append_ir2_opnd3(mod_op, &div_mod, &src, data);
        latxs_ra_free_temp(&src);

        latxs_tr_gen_div_result_check(pir1, div_res, 32, is_idiv);

        latxs_store_ir2_to_ir1_gpr_em(&div_mod, &edx_ir1_opnd);
        latxs_store_ir2_to_ir1_gpr_em(&div_res, &eax_ir1_opnd);

        /* convert gpr opnd might return temp */
        latxs_ra_free_temp(&eax);
        latxs_ra_free_temp(&edx);
        break;
    default:
        lsassert(0);
        break;
    }

    latxs_ra_free_temp(&div_res);
    latxs_ra_free_temp(&div_mod);
}

static bool latxs_translate_div_byhand_check_zero(
        IR1_INST *pir1, int os,
        IR2_OPND *data, int is_idiv)
{
    IR2_OPND label_not_zero = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd2(LISA_BNEZ, data, &label_not_zero);
    latxs_tr_gen_excp_divz(pir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &label_not_zero);

    __latxs_translate_div_byhand(pir1, os, data, is_idiv);
    return true;
}

bool latxs_translate_div_byhand(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR/MEM */
    int os = ir1_opnd_size(opnd0);

    IR2_OPND data;
    if (ir1_opnd_is_gpr(opnd0)) {
        data = latxs_convert_gpr_opnd(opnd0, EXMode_Z);
    } else {
        data = latxs_ra_alloc_itemp();
        latxs_load_ir1_mem_to_ir2(&data, opnd0, EXMode_Z, -1);
    }

    return latxs_translate_div_byhand_check_zero(pir1, os, &data, 0);
}

bool latxs_translate_idiv_byhand(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR/MEM */
    int os = ir1_opnd_size(opnd0);

    IR2_OPND data;
    if (ir1_opnd_is_gpr(opnd0)) {
        data = latxs_convert_gpr_opnd(opnd0, EXMode_S);
    } else {
        data = latxs_ra_alloc_itemp();
        latxs_load_ir1_mem_to_ir2(&data, opnd0, EXMode_S, -1);
    }

    return latxs_translate_div_byhand_check_zero(pir1, os, &data, 1);
}
