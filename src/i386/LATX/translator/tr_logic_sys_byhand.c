#include "../include/common.h"
#include "../translator/translate.h"
#include "../include/reg_alloc.h"
#include "../x86tomips-options.h"
#include "../include/flag_lbt.h"

static 
void translate_test_byhand_gpr_gpr(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND reg0 = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
    IR2_OPND reg1 = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));

    if (ir1_opnd_is_8h(opnd0)) {
        IR2_OPND tmp0 = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_SRLI_D, &tmp0, &reg0, 8);
        if (ir1_opnd_is_8h(opnd1)) {
            IR2_OPND tmp1 = ra_alloc_itemp();
            append_ir2_opnd2i(LISA_SRLI_D, &tmp1, &reg1, 8);
            append_ir2_opnd2(LISA_X86AND_B, &tmp0, &tmp1);
        } else {
            append_ir2_opnd2(LISA_X86AND_B, &tmp0, &reg1);
        }
    } else {
        if (ir1_opnd_is_8h(opnd1)) {
            IR2_OPND tmp1 = ra_alloc_itemp();
            append_ir2_opnd2i(LISA_SRLI_D, &tmp1, &reg1, 8);
            append_ir2_opnd2(LISA_X86AND_B, &reg0, &tmp1);
        } else {
            int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
            append_ir2_opnd2(LISA_X86AND_B + (os >> 4), &reg0, &reg1);
        }
    }
}

static 
void translate_test_byhand_gpr_imm(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND reg0 = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));

    IR2_OPND reg1 = ra_alloc_itemp();
    load_ir1_imm_to_ir2(&reg1, opnd1, UNKNOWN_EXTENSION);

    if (ir1_opnd_is_8h(opnd0)) {
        IR2_OPND tmp = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_SRLI_D, &tmp, &reg0, 8);
        append_ir2_opnd2(LISA_X86AND_B, &tmp, &reg1);
    } else {
        int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
        append_ir2_opnd2(LISA_X86AND_B + (os >> 4), &reg0, &reg1);
    }
}

static 
void translate_test_byhand_mem_gpr(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND reg0 = ra_alloc_itemp();
    load_ir1_mem_to_ir2(&reg0, opnd0, UNKNOWN_EXTENSION, false, -1);

    int opnd1_reg = ir1_opnd_base_reg_num(opnd1);
    IR2_OPND reg1 = ra_alloc_gpr(opnd1_reg);

    if (ir1_opnd_is_8h(opnd1)) {
        IR2_OPND tmp = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_SRLI_D, &tmp, &reg1, 8);
        append_ir2_opnd2(LISA_X86AND_B, &reg0, &reg1);
    } else {
        int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
        append_ir2_opnd2(LISA_X86AND_B+ (os >> 4), &reg0, &reg1);
    }
}

static 
void translate_test_byhand_mem_imm(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND reg0 = ra_alloc_itemp();
    IR2_OPND reg1 = ra_alloc_itemp();

    load_ir1_mem_to_ir2(&reg0, opnd0, UNKNOWN_EXTENSION, false, -1);
    load_ir1_imm_to_ir2(&reg1, opnd1, UNKNOWN_EXTENSION);

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    append_ir2_opnd2(LISA_X86AND_B+ (os >> 4), &reg0, &reg1);
}

bool translate_test_byhand(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    /* Four types of test instruction
     * > gpr, gpr/imm
     * > mem, gpr/imm      */

    if (ir1_opnd_is_gpr(opnd0)) {
        if (ir1_opnd_is_gpr(opnd1)) {
            translate_test_byhand_gpr_gpr(opnd0, opnd1);
        } else {
            lsassertm(ir1_opnd_is_imm(opnd1), "[trbh] test opnd1 not imm.\n");
            translate_test_byhand_gpr_imm(opnd0, opnd1);
        }
    } else {
        if (ir1_opnd_is_gpr(opnd1)) {
            translate_test_byhand_mem_gpr(opnd0, opnd1);
        } else {
            lsassertm(ir1_opnd_is_imm(opnd1), "[trbh] test opnd1 not imm.\n");
            translate_test_byhand_mem_imm(opnd0, opnd1);
        }
    }

    return true;
}





static 
void translate_xor_and_or_byhand_gpr_gpr(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op)
{
    IR2_OPND reg0 = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
    IR2_OPND reg1 = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));

    if (ir1_opnd_is_8h(opnd0)) {
        IR2_OPND tmp0 = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_SRLI_D, &tmp0, &reg0, 8);
        if (ir1_opnd_is_8h(opnd1)) {
            /* xor 8H, 8H */
            IR2_OPND tmp1 = ra_alloc_itemp();
            append_ir2_opnd2i(LISA_SRLI_D, &tmp1, &reg1, 8);
            append_ir2_opnd2(lbt_op, &tmp0, &tmp1);
            append_ir2_opnd3(lisa_op, &tmp0, &tmp0, &tmp1);
            store_ir2_to_ir1_gpr(&tmp0, opnd0);
        } else {
            /* xor, 8H, reg8 */
            append_ir2_opnd2(lbt_op, &tmp0, &reg1);
            append_ir2_opnd3(lisa_op, &tmp0, &tmp0, &reg1);
            store_ir2_to_ir1_gpr(&tmp0, opnd0);
        }
    } else {
        if (ir1_opnd_is_8h(opnd1)) {
            /* xor reg8, 8H */
            IR2_OPND tmp1 = ra_alloc_itemp();
            append_ir2_opnd2i(LISA_SRLI_D, &tmp1, &reg1, 8);
            append_ir2_opnd2(lbt_op, &reg0, &tmp1);
            append_ir2_opnd3(lisa_op, &tmp1, &reg0, &tmp1);
            store_ir2_to_ir1_gpr(&tmp1, opnd0);
        } else {
            /* xor reg8/16/32, reg8/16/32 */
            int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
            append_ir2_opnd2(lbt_op+ (os >> 4), &reg0, &reg1);
//            ir2_opnd_set_em(&reg0, UNKNOWN_EXTENSION, 32);
//            ir2_opnd_set_em(&reg1, UNKNOWN_EXTENSION, 32);
            if (os == 32) {
                append_ir2_opnd3(lisa_op, &reg0, &reg0, &reg1);
            } else {
                IR2_OPND tmp0 = ra_alloc_itemp();
                append_ir2_opnd3(lisa_op, &tmp0, &reg0, &reg1);
                store_ir2_to_ir1_gpr(&tmp0, opnd0);
            }
        }
    }
}

static 
void translate_xor_and_or_byhand_gpr_imm(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op)
{
    IR2_OPND reg0 = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));

    IR2_OPND reg1 = ra_alloc_itemp();
    load_ir1_imm_to_ir2(&reg1, opnd1, SIGN_EXTENSION);

    if (ir1_opnd_is_8h(opnd0)) {
        /* xor 8H, imm */
        IR2_OPND tmp0 = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_SRLI_D, &tmp0, &reg0, 8);
        append_ir2_opnd2 (lbt_op, &tmp0, &reg1);
        append_ir2_opnd3 (lisa_op, &tmp0, &tmp0, &reg1);
        store_ir2_to_ir1_gpr(&tmp0, opnd0);
    } else {
        /* xor reg8/16/32, imm */
        int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
        append_ir2_opnd2(lbt_op+ (os >> 4), &reg0, &reg1);
//        ir2_opnd_set_em(&reg0, UNKNOWN_EXTENSION, 32);
        if (os == 32) {
            append_ir2_opnd3(lisa_op, &reg0, &reg0, &reg1);
        } else {
            IR2_OPND tmp0 = ra_alloc_itemp();
            append_ir2_opnd3(lisa_op, &tmp0, &reg0, &reg1);
            store_ir2_to_ir1_gpr(&tmp0, opnd0);
        }
    }
}

static 
void translate_xor_and_or_byhand_gpr_mem(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op)
{
    IR2_OPND reg0 = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));

    IR2_OPND reg1 = ra_alloc_itemp();
    load_ir1_mem_to_ir2(&reg1, opnd1, UNKNOWN_EXTENSION, false, -1);

    if (ir1_opnd_is_8h(opnd0)) {
        /* xor 8H, mem */
        IR2_OPND tmp0 = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_SRLI_D, &tmp0, &reg0, 8);
        append_ir2_opnd2 (lbt_op, &tmp0, &reg1);
        append_ir2_opnd3 (lisa_op, &tmp0, &tmp0, &reg1);
        store_ir2_to_ir1_gpr(&tmp0, opnd0);
    } else {
        /* xor reg8/16/32, mem */
        int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
        append_ir2_opnd2(lbt_op+ (os >> 4), &reg0, &reg1);
//        ir2_opnd_set_em(&reg0, UNKNOWN_EXTENSION, 32);
        if (os == 32) {
            append_ir2_opnd3(lisa_op, &reg0, &reg0, &reg1);
        } else {
            IR2_OPND tmp0 = ra_alloc_itemp();
            append_ir2_opnd3(lisa_op, &tmp0, &reg0, &reg1);
            store_ir2_to_ir1_gpr(&tmp0, opnd0);
        }
    }
}

static 
void translate_xor_and_or_byhand_mem_gpr(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op)
{
    IR2_OPND reg0 = ra_alloc_itemp();
    load_ir1_mem_to_ir2(&reg0, opnd0, UNKNOWN_EXTENSION, false, -1);

    IR2_OPND reg1 = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));

    if (ir1_opnd_is_8h(opnd1)) {
        /* xor mem, 8H */
        IR2_OPND tmp1 = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_SRLI_D, &tmp1, &reg1, 8);

        IR2_OPND dest = ra_alloc_itemp();
        append_ir2_opnd3(lisa_op, &dest, &reg0, &tmp1);
        store_ir2_to_ir1_mem(&dest, opnd0, false, -1);

        append_ir2_opnd2(lbt_op, &reg0, &tmp1);
    } else {
        /* xor mem, reg8/16/32 */
        IR2_OPND dest = ra_alloc_itemp();
//        ir2_opnd_set_em(&reg1, UNKNOWN_EXTENSION, 32);
        append_ir2_opnd3(lisa_op, &dest, &reg0, &reg1);
        store_ir2_to_ir1_mem(&dest, opnd0, false, -1);

        int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
        append_ir2_opnd2(lbt_op + (os >> 4), &reg0, &reg1);
    }
}

static 
void translate_xor_and_or_byhand_mem_imm(
        IR1_OPND   *opnd0,
        IR1_OPND   *opnd1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op)
{
    IR2_OPND reg0 = ra_alloc_itemp();
    load_ir1_mem_to_ir2(&reg0, opnd0, UNKNOWN_EXTENSION, false, -1);

    IR2_OPND reg1 = ra_alloc_itemp();
    load_ir1_imm_to_ir2(&reg1, opnd1, SIGN_EXTENSION);

    IR2_OPND dest = ra_alloc_itemp();
    append_ir2_opnd3(lisa_op, &dest, &reg0, &reg1);
    store_ir2_to_ir1_mem(&dest, opnd0, false, -1);

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    append_ir2_opnd2(lbt_op + (os >> 4), &reg0, &reg1);
}

static inline
bool translate_xor_and_or_byhand(
        IR1_INST   *pir1,
        IR2_OPCODE  lisa_op,
        IR2_OPCODE  lbt_op)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

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
            translate_xor_and_or_byhand_gpr_gpr(opnd0, opnd1, lisa_op, lbt_op);
        } else if (ir1_opnd_is_imm(opnd1)) {
            translate_xor_and_or_byhand_gpr_imm(opnd0, opnd1, lisa_op, lbt_op);
        } else {
            translate_xor_and_or_byhand_gpr_mem(opnd0, opnd1, lisa_op, lbt_op);
        }
    } else {
        if (ir1_opnd_is_gpr(opnd1)) {
            translate_xor_and_or_byhand_mem_gpr(opnd0, opnd1, lisa_op, lbt_op);
        } else {
            translate_xor_and_or_byhand_mem_imm(opnd0, opnd1, lisa_op, lbt_op);
        }
    }

    return true;
}





bool translate_xor_byhand(IR1_INST *pir1)
{
    return translate_xor_and_or_byhand(pir1, LISA_XOR, LISA_X86XOR_B);
}

bool translate_and_byhand(IR1_INST *pir1)
{
    return translate_xor_and_or_byhand(pir1, LISA_AND, LISA_X86AND_B);
}

bool translate_or_byhand(IR1_INST *pir1)
{
    return translate_xor_and_or_byhand(pir1, LISA_OR, LISA_X86OR_B);
}





static
void translate_not_byhand_gpr(IR1_OPND *opnd0)
{
    IR2_OPND reg = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));

    if (ir1_opnd_is_8h(opnd0)) {
        IR2_OPND tmp = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_SRLI_D, &tmp, &reg, 8);
        append_ir2_opnd3(LISA_NOR, &tmp, &tmp, &zero_ir2_opnd);
        store_ir2_to_ir1_gpr(&tmp, opnd0);
    } else {
        int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
//        ir2_opnd_set_em(&reg, UNKNOWN_EXTENSION, 32);
        if (os == 32) {
            append_ir2_opnd3(LISA_NOR, &reg, &reg, &zero_ir2_opnd);
        } else {
            IR2_OPND tmp = ra_alloc_itemp();
            append_ir2_opnd3(LISA_NOR, &tmp, &reg, &zero_ir2_opnd);
            store_ir2_to_ir1_gpr(&tmp, opnd0);
        }
    }
}

static
void translate_not_byhand_mem(IR1_OPND *opnd0)
{
    IR2_OPND mem;
    convert_mem_opnd(&mem, opnd0, -1);

    IR2_OPND tmp = ra_alloc_itemp();
    int os = ir1_opnd_size(opnd0);
    switch (os) {
    case 32: gen_ldst_softmmu_helper(LISA_LD_WU, &tmp, &mem, 1); break;
    case 16: gen_ldst_softmmu_helper(LISA_LD_HU, &tmp, &mem, 1); break;
    case 8:  gen_ldst_softmmu_helper(LISA_LD_BU, &tmp, &mem, 1); break;
    default: lsassert(0); break;
    }

    append_ir2_opnd3(LISA_NOR, &tmp, &tmp, &zero_ir2_opnd);

    switch (os) {
    case 32: gen_ldst_softmmu_helper(LISA_ST_W, &tmp, &mem, 1); break;
    case 16: gen_ldst_softmmu_helper(LISA_ST_H, &tmp, &mem, 1); break;
    case 8:  gen_ldst_softmmu_helper(LISA_ST_B, &tmp, &mem, 1); break;
    default: lsassert(0); break;
    }
}

bool translate_not_byhand(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    /* 2 types of not instruction
     *
     * > gpr
     * > mem
     */

    if (ir1_opnd_is_gpr(opnd0)) {
        translate_not_byhand_gpr(opnd0);
    } else {
        translate_not_byhand_mem(opnd0);
    }

    return true;
}





static 
void translate_shr_byhand_gpr_cl(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND reg1 = ra_alloc_gpr(ecx_index);

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    if (os == 32) {
        IR2_OPND reg0 = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));

        append_ir2_opnd2(LISA_X86SRL_B + (os >> 4), &reg0, &reg1);
        append_ir2_opnd3(LISA_SRL_W, &reg0, &reg0, &reg1);
    } else {
        IR2_OPND reg0 = ra_alloc_itemp();
        load_ir1_gpr_to_ir2(&reg0, opnd0, ZERO_EXTENSION);

        append_ir2_opnd2(LISA_X86SRL_B + (os >> 4), &reg0, &reg1);
        append_ir2_opnd3(LISA_SRL_W, &reg0, &reg0, &reg1);

        store_ir2_to_ir1_gpr(&reg0, opnd0);
    }
}

static 
void translate_shr_byhand_gpr_imm8(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    int num = ir1_opnd_simm(opnd1) & 0x1f;
    if (!num) return;

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    if (os == 32) {
        IR2_OPND reg0 = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));

        append_ir2_opnd1i(LISA_X86SRLI_W, &reg0, num);
        append_ir2_opnd2i(LISA_SRLI_W, &reg0, &reg0, num);
    } else {
        IR2_OPND reg0 = ra_alloc_itemp();
        load_ir1_gpr_to_ir2(&reg0, opnd0, ZERO_EXTENSION);

        IR2_OPND tmp = ra_alloc_itemp();
        append_ir2_opnd2i(LISA_ORI, &tmp, &zero_ir2_opnd, num);
        append_ir2_opnd2(LISA_X86SRL_B + (os >> 4), &reg0, &tmp);
        append_ir2_opnd2i(LISA_SRLI_W, &reg0, &reg0, num);
        
        store_ir2_to_ir1_gpr(&reg0, opnd0);
    }
}

static 
void translate_shr_byhand_mem_cl(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    IR2_OPND reg0 = ra_alloc_itemp();
    IR2_OPND reg1 = ra_alloc_gpr(ecx_index);

    IR2_OPND label_exit = ir2_opnd_new_label();
    append_ir2_opnd2i(LISA_ANDI, &reg0, &reg1, 0x1f);
    append_ir2_opnd3(LISA_BEQ, &reg0, &zero_ir2_opnd, &label_exit);

    IR2_OPND mem;
    convert_mem_opnd(&mem, opnd0, -1);
    int os = ir1_opnd_size(opnd0);
    switch (os) {
    case 32: gen_ldst_softmmu_helper(LISA_LD_W,  &reg0, &mem, 1); break;
    case 16: gen_ldst_softmmu_helper(LISA_LD_HU, &reg0, &mem, 1); break;
    case 8:  gen_ldst_softmmu_helper(LISA_LD_BU, &reg0, &mem, 1); break;
    default: lsassert(0); break;
    }

    IR2_OPND dest = ra_alloc_itemp();
    append_ir2_opnd3(LISA_SRL_W, &dest, &reg0, &reg1);

    switch (os) {
    case 32: gen_ldst_softmmu_helper(LISA_ST_W, &dest, &mem, 1); break;
    case 16: gen_ldst_softmmu_helper(LISA_ST_H, &dest, &mem, 1); break;
    case 8:  gen_ldst_softmmu_helper(LISA_ST_B, &dest, &mem, 1); break;
    default: lsassert(0); break;
    }

    append_ir2_opnd2(LISA_X86SRL_B + (os >> 4), &reg0, &reg1);

    append_ir2_opnd1(LISA_LABEL, &label_exit);
}

static 
void translate_shr_byhand_mem_imm8(IR1_OPND *opnd0, IR1_OPND *opnd1)
{
    int num = ir1_opnd_simm(opnd1) & 0x1f;
    if (!num) return;

    IR2_OPND reg0 = ra_alloc_itemp();

    IR2_OPND mem;
    convert_mem_opnd(&mem, opnd0, -1);
    int os = ir1_opnd_size(opnd0);
    switch (os) {
    case 32: gen_ldst_softmmu_helper(LISA_LD_W,  &reg0, &mem, 1); break;
    case 16: gen_ldst_softmmu_helper(LISA_LD_HU, &reg0, &mem, 1); break;
    case 8:  gen_ldst_softmmu_helper(LISA_LD_BU, &reg0, &mem, 1); break;
    default: lsassert(0); break;
    }

    IR2_OPND dest = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_SRLI_W, &dest, &reg0, num);

    switch (os) {
    case 32: gen_ldst_softmmu_helper(LISA_ST_W, &dest, &mem, 1); break;
    case 16: gen_ldst_softmmu_helper(LISA_ST_H, &dest, &mem, 1); break;
    case 8:  gen_ldst_softmmu_helper(LISA_ST_B, &dest, &mem, 1); break;
    default: lsassert(0); break;
    }

    IR2_OPND tmp = ra_alloc_itemp();
    append_ir2_opnd2i(LISA_ORI, &tmp, &zero_ir2_opnd, num);
    append_ir2_opnd2(LISA_X86SRL_B + (os >> 4), &reg0, &tmp);
}

bool translate_shr_byhand(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    /* 4 types of shr instruction
     *
     * > gpr, imm8
     * > gpr, cl
     * > mem, imm8
     * > mem, cl
     */

    if (ir1_opnd_is_gpr(opnd0)) {
        if (ir1_opnd_is_gpr(opnd1)) {
            translate_shr_byhand_gpr_cl(opnd0, opnd1);
        } else {
            translate_shr_byhand_gpr_imm8(opnd0, opnd1);
        }
    } else {
        if (ir1_opnd_is_gpr(opnd1)) {
            translate_shr_byhand_mem_cl(opnd0, opnd1);
        } else {
            translate_shr_byhand_mem_imm8(opnd0, opnd1);
        }
    }

    return true;
}
