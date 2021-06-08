#include "common.h"
#include "translate.h"
#include "reg_alloc.h"
#include "x86tomips-options.h"
#include "flag_lbt.h"

static 
void translate_cmp_byhand_gpr_gpr(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND reg0 = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
    IR2_OPND reg1 = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));

    if (ir1_opnd_is_8h(opnd0)) {
        IR2_OPND tmp0 = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_SRLI_D, &tmp0, &reg0, 8);
        if (ir1_opnd_is_8h(opnd1)) {
            IR2_OPND tmp1 = ra_alloc_itemp();
            append_ir2_opnd2i(LISA_SRLI_D, &tmp1, &reg1, 8);
            append_ir2_opnd2(LISA_X86SUB_B, &tmp0, &tmp1);
        } else {
            append_ir2_opnd2(LISA_X86SUB_B, &tmp0, &reg1);
        }
    } else {
        if (ir1_opnd_is_8h(opnd1)) {
            IR2_OPND tmp1 = ra_alloc_itemp();
            append_ir2_opnd2i(LISA_SRLI_D, &tmp1, &reg1, 8);
            append_ir2_opnd2(LISA_X86SUB_B, &reg0, &tmp1);
        } else {
            int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
            append_ir2_opnd2(LISA_X86SUB_B + (os >> 4), &reg0, &reg1);
        }
    }
}

static 
void translate_cmp_byhand_gpr_imm(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND reg0 = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));

    IR2_OPND reg1;
    if (ir1_opnd_is_imm0(opnd1)) {
        reg1 = zero_ir2_opnd;
    } else {
        reg1 = ra_alloc_itemp();
        load_ir1_imm_to_ir2(&reg1, opnd1, SIGN_EXTENSION);
    }

    if (ir1_opnd_is_8h(opnd0)) {
        IR2_OPND tmp = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_SRLI_D, &tmp, &reg0, 8);
        append_ir2_opnd2(LISA_X86SUB_B, &tmp, &reg1);
    } else {
        int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
        append_ir2_opnd2(LISA_X86SUB_B+ (os >> 4), &reg0, &reg1);
    }
}

static 
void translate_cmp_byhand_gpr_mem(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND reg0 = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));

    IR2_OPND reg1 = ra_alloc_itemp();
    load_ir1_mem_to_ir2(&reg1, opnd1, UNKNOWN_EXTENSION, false, -1);

    if (ir1_opnd_is_8h(opnd0)) {
        IR2_OPND tmp = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_SRLI_D, &tmp, &reg0, 8);
        append_ir2_opnd2(LISA_X86SUB_B, &tmp, &reg1);
    } else {
        int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
        append_ir2_opnd2(LISA_X86SUB_B+ (os >> 4), &reg0, &reg1);
    }
}

static 
void translate_cmp_byhand_mem_gpr(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND reg0 = ra_alloc_itemp();
    load_ir1_mem_to_ir2(&reg0, opnd0, UNKNOWN_EXTENSION, false, -1);

    IR2_OPND reg1 = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));

    if (ir1_opnd_is_8h(opnd1)) {
        IR2_OPND tmp = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_SRLI_D, &tmp, &reg1, 8);
        append_ir2_opnd2(LISA_X86SUB_B, &reg0, &tmp);
    } else {
        int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
        append_ir2_opnd2(LISA_X86SUB_B+ (os >> 4), &reg0, &reg1);
    }
}

static 
void translate_cmp_byhand_mem_imm(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND reg0 = ra_alloc_itemp();
    load_ir1_mem_to_ir2(&reg0, opnd0, UNKNOWN_EXTENSION, false, -1);

    IR2_OPND reg1;
    if (ir1_opnd_is_imm0(opnd1)) {
        reg1 = zero_ir2_opnd;
    } else {
        reg1 = ra_alloc_itemp();
        load_ir1_imm_to_ir2(&reg1, opnd1, SIGN_EXTENSION);
    }

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    append_ir2_opnd2(LISA_X86SUB_B+ (os >> 4), &reg0, &reg1);
}

bool translate_cmp_byhand(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    /* 5 types of cmp instruction
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
            translate_cmp_byhand_gpr_gpr(opnd0, opnd1);
        } else if (ir1_opnd_is_imm(opnd1)) {
            translate_cmp_byhand_gpr_imm(opnd0, opnd1);
        } else {
            translate_cmp_byhand_gpr_mem(opnd0, opnd1);
        }
    } else {
        if (ir1_opnd_is_gpr(opnd1)) {
            translate_cmp_byhand_mem_gpr(opnd0, opnd1);
        } else {
            translate_cmp_byhand_mem_imm(opnd0, opnd1);
        }
    }

    return true;
}





static 
void translate_add_sub_byhand_gpr_gpr(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op,
        IR2_OPND   *cf)
{
    IR2_OPND reg0 = convert_gpr_opnd(opnd0, SIGN_EXTENSION);
    IR2_OPND reg1 = convert_gpr_opnd(opnd1, SIGN_EXTENSION);

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    append_ir2_opnd2(lbt_op + (os >> 4), &reg0, &reg1);
//
    if (ir2_opnd_is_reg_temp(&reg0)) {
        append_ir2_opnd3(lisa_op, &reg0, &reg0, &reg1);
        if (cf) append_ir2_opnd3(LISA_ADD_W, &reg0, &reg0, cf);
        store_ir2_to_ir1_gpr(&reg0, opnd0);
    }
    else if (os == 32) {
        append_ir2_opnd3(lisa_op, &reg0, &reg0, &reg1);
        if (cf) append_ir2_opnd3(LISA_ADD_W, &reg0, &reg0, cf);
    }
    else if (ir2_opnd_is_reg_temp(&reg1)) {
        append_ir2_opnd3(lisa_op, &reg1, &reg0, &reg1);
        if (cf) append_ir2_opnd3(LISA_ADD_W, &reg1, &reg1, cf);
        store_ir2_to_ir1_gpr(&reg1, opnd0);
    } else {
        IR2_OPND dest = ra_alloc_itemp();
        append_ir2_opnd3(lisa_op, &dest, &reg0, &reg1);
        if (cf) append_ir2_opnd3(LISA_ADD_W, &dest, &reg1, cf);
        store_ir2_to_ir1_gpr(&dest, opnd0);
    }
}

static 
void translate_add_sub_byhand_gpr_imm(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op,
        IR2_OPND   *cf)
{
//    int32_t imm = ir1_opnd_simm(opnd1);
//    if (int32_in_int16(imm) && !cf) {
//        IR2_OPND dst_reg = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
//        if (ir2_opnd_is_x86_address(&dst_reg)) {
//            IR2_OPND reg1 = ra_alloc_itemp();
//            load_ir1_imm_to_ir2(&reg1, opnd1, SIGN_EXTENSION);
//            append_ir2_opnd2(lbt_op + (ir1_opnd_size(opnd0) >> 4), &dst_reg,
//                             &reg1);
//            if (lisa_op == mips_addu) {
//                append_ir2_opnd2i(mips_addi_addrx, &dst_reg, &dst_reg, imm);
//            } else if (lisa_op == mips_subu) {
//                append_ir2_opnd2i(mips_subi_addrx, &dst_reg, &dst_reg, imm);
//            } else {
//                lsassert(0);
//            }
//            return;
//        }
//    }

    IR2_OPND reg0 = convert_gpr_opnd(opnd0, SIGN_EXTENSION);

    IR2_OPND reg1 = ra_alloc_itemp();
    load_ir1_imm_to_ir2(&reg1, opnd1, SIGN_EXTENSION);

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    append_ir2_opnd2(lbt_op + (os >> 4), &reg0, &reg1);

    if (ir2_opnd_is_reg_temp(&reg0)) {
        append_ir2_opnd3(lisa_op, &reg0, &reg0, &reg1);
        if (cf) append_ir2_opnd3(LISA_ADD_W, &reg0, &reg0, cf);
        store_ir2_to_ir1_gpr(&reg0, opnd0);
    }
    else if (os == 32) {
        append_ir2_opnd3(lisa_op, &reg0, &reg0, &reg1);
        if (cf) append_ir2_opnd3(LISA_ADD_W, &reg0, &reg0, cf);
    } else {
        IR2_OPND dest = ra_alloc_itemp();
        append_ir2_opnd3(lisa_op, &dest, &reg0, &reg1);
        if (cf) append_ir2_opnd3(LISA_ADD_W, &dest, &reg1, cf);
        store_ir2_to_ir1_gpr(&dest, opnd0);
    }
}

static 
void translate_add_sub_byhand_gpr_mem(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op,
        IR2_OPND   *cf)
{
    IR2_OPND reg0 = convert_gpr_opnd(opnd0, SIGN_EXTENSION);

    IR2_OPND reg1 = ra_alloc_itemp();
    load_ir1_mem_to_ir2(&reg1, opnd1, SIGN_EXTENSION, false, -1);

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    append_ir2_opnd2(lbt_op + (os >> 4), &reg0, &reg1);

    if (ir2_opnd_is_reg_temp(&reg0)) {
        append_ir2_opnd3(lisa_op, &reg0, &reg0, &reg1);
        if (cf) append_ir2_opnd3(LISA_ADD_W, &reg0, &reg0, cf);
        store_ir2_to_ir1_gpr(&reg0, opnd0);
    }
    else if (os == 32) {
        append_ir2_opnd3(lisa_op, &reg0, &reg0, &reg1);
        if (cf) append_ir2_opnd3(LISA_ADD_W, &reg0, &reg0, cf);
    } else {
        IR2_OPND dest = ra_alloc_itemp();
        append_ir2_opnd3(lisa_op, &dest, &reg0, &reg1);
        if (cf) append_ir2_opnd3(LISA_ADD_W, &dest, &reg1, cf);
        store_ir2_to_ir1_gpr(&dest, opnd0);
    }
}

static 
void translate_add_sub_byhand_mem_gpr(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op,
        IR2_OPND   *cf)
{
    IR2_OPND reg0 = ra_alloc_itemp();
    load_ir1_mem_to_ir2(&reg0, opnd0, SIGN_EXTENSION, false, -1);

    IR2_OPND reg1 = convert_gpr_opnd(opnd1, SIGN_EXTENSION);

    IR2_OPND dest = ra_alloc_itemp();
    append_ir2_opnd3(lisa_op, &dest, &reg0, &reg1);
    if (cf) {
        append_ir2_opnd3(LISA_ADD_W, &dest, &dest, cf);
        ra_free_temp(cf);
    }
    store_ir2_to_ir1_mem(&dest, opnd0, false, -1);

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    append_ir2_opnd2(lbt_op + (os >> 4), &reg0, &reg1);
}

static 
void translate_add_sub_byhand_mem_imm(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op,
        IR2_OPND   *cf)
{
    IR2_OPND reg0 = ra_alloc_itemp();
    load_ir1_mem_to_ir2(&reg0, opnd0, SIGN_EXTENSION, false, -1);

    IR2_OPND reg1 = ra_alloc_itemp();
    load_ir1_imm_to_ir2(&reg1, opnd1, SIGN_EXTENSION);

    IR2_OPND dest = ra_alloc_itemp();
    append_ir2_opnd3(lisa_op, &dest, &reg0, &reg1);
    if (cf) {
        append_ir2_opnd3(LISA_ADD_W, &dest, &dest, cf);
        ra_free_temp(cf);
    }
    store_ir2_to_ir1_mem(&dest, opnd0, false, -1);

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    append_ir2_opnd2(lbt_op + (os >> 4), &reg0, &reg1);
}

static inline
bool translate_add_sub_byhand(
        IR1_INST   *pir1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op,
        int         is_adc)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND cf;
    if (is_adc) {
        cf = ra_alloc_itemp();
        load_eflags_cf_to_ir2(&cf);
    }

    /* 5 types of xor,and,or instruction
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
            translate_add_sub_byhand_gpr_gpr(opnd0, opnd1,
                    lisa_op, lbt_op, is_adc ? &cf : NULL);
        } else if (ir1_opnd_is_imm(opnd1)) {
            translate_add_sub_byhand_gpr_imm(opnd0, opnd1,
                    lisa_op, lbt_op, is_adc ? &cf : NULL);
        } else {
            translate_add_sub_byhand_gpr_mem(opnd0, opnd1,
                    lisa_op, lbt_op, is_adc ? &cf : NULL);
        }
    } else {
        if (ir1_opnd_is_gpr(opnd1)) {
            translate_add_sub_byhand_mem_gpr(opnd0, opnd1,
                    lisa_op, lbt_op, is_adc ? &cf : NULL);
        } else {
            translate_add_sub_byhand_mem_imm(opnd0, opnd1,
                    lisa_op, lbt_op, is_adc ? &cf : NULL);
        }
    }

    return true;
}





bool translate_add_byhand(IR1_INST *pir1)
{
    return translate_add_sub_byhand(pir1, LISA_ADD_W, LISA_X86ADD_B, 0);
}

bool translate_adc_byhand(IR1_INST *pir1)
{
    return translate_add_sub_byhand(pir1, LISA_ADD_W, LISA_X86ADC_B, 1);
}

bool translate_sub_byhand(IR1_INST *pir1)
{
    return translate_add_sub_byhand(pir1, LISA_SUB_W, LISA_X86SUB_B, 0);
}
