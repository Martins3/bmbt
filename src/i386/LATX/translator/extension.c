#include "../include/common.h"
#include "../include/env.h"
#include "../include/reg_alloc.h"

EXTENSION_MODE ir1_opnd_em(IR1_OPND *opnd)
{
    lsassertm(0, "EM to be implemented in LoongArch.\n");
    return 1;
//    if (ir1_opnd_is_gpr(opnd)) {
//        IR2_OPND ir2_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd));
//        int mips_ireg_num = ir2_opnd_base_reg_num(&ir2_opnd);
//        return lsenv->tr_data->ireg_em[mips_ireg_num];
//    } else
//        return SIGN_EXTENSION;
}

int ir1_opnd_eb(IR1_OPND *opnd)
{
    lsassertm(0, "EM to be implemented in LoongArch.\n");
    return 1;
//    if (ir1_opnd_is_gpr(opnd)) {
//        IR2_OPND ir2_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd));
//        int mips_ireg_num = ir2_opnd_base_reg_num(&ir2_opnd);
//        return lsenv->tr_data->ireg_eb[mips_ireg_num];
//    } else
//        return 32;
}

EXTENSION_MODE ir1_opnd_default_em(IR1_OPND *opnd)
{
    lsassertm(0, "EM to be implemented in LoongArch.\n");
    return 1;
//    lsassert(ir1_opnd_is_gpr(opnd));
//
//#ifdef N64 /* validate address */
//    int x86_gpr_num = ir1_opnd_base_reg_num(opnd);
//
//    if (x86_gpr_num == esp_index)
//        return EM_X86_ADDRESS;
//    else if (x86_gpr_num == ebp_index)
//        return ZERO_EXTENSION;
//    else
//        return SIGN_EXTENSION;
//#else
//    return SIGN_EXTENSION;
//#endif
}

void ir1_opnd_set_em(IR1_OPND *opnd, EXTENSION_MODE e, int bits)
{
    lsassertm(0, "EM to be implemented in LoongArch.\n");
//    lsassert(ir1_opnd_is_gpr(opnd));
//
//    if (bits >= 64 || e == UNKNOWN_EXTENSION) {
//        e = UNKNOWN_EXTENSION;
//        bits = 32;
//    } else if (bits < 0)
//        bits = 0;
//    else if (e == EM_X86_ADDRESS || e == EM_MIPS_ADDRESS)
//        lsassert(bits == 32);
//
//    IR2_OPND ir2_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd));
//    int mips_ireg_num = ir2_opnd_base_reg_num(&ir2_opnd);
//
//    lsenv->tr_data->ireg_em[mips_ireg_num] = e;
//    lsenv->tr_data->ireg_eb[mips_ireg_num] = bits;
}

int ir1_opnd_is_sx(IR1_OPND *opnd, int bits)
{
    lsassertm(0, "EM to be implemented in LoongArch.\n");
    return 1;
//    EXTENSION_MODE em_value = ir1_opnd_em(opnd);
//    int eb_value = ir1_opnd_eb(opnd);
//
//    if (em_value == SIGN_EXTENSION && bits >= eb_value)
//        return true;
//    else if (em_value == ZERO_EXTENSION && bits > eb_value)
//        return true;
//    else
//        return false;
}

int ir1_opnd_is_zx(IR1_OPND *opnd, int bits)
{
    lsassertm(0, "EM to be implemented in LoongArch.\n");
    return 1;
//    EXTENSION_MODE em_value = ir1_opnd_em(opnd);
//    int eb_value = ir1_opnd_eb(opnd);
//
//    if (em_value == ZERO_EXTENSION && bits >= eb_value)
//        return true;
//#ifdef N64 /* validate address */
//    else if (em_value == EM_X86_ADDRESS && bits >= 32)
//        return true;
//#endif
//    else
//        return false;
}

int ir1_opnd_is_bx(IR1_OPND *opnd, int bits)
{
    lsassertm(0, "EM to be implemented in LoongArch.\n");
    return 1;
//    return ir1_opnd_is_sx(opnd, bits) && ir1_opnd_is_zx(opnd, bits);
}

int ir1_opnd_is_ax(IR1_OPND *opnd, int bits)
{
    lsassertm(0, "EM to be implemented in LoongArch.\n");
    return 1;
//    return !ir1_opnd_is_sx(opnd, bits) && !ir1_opnd_is_zx(opnd, bits);
}

int ir1_opnd_is_address(IR1_OPND *opnd)
{
    lsassertm(0, "EM to be implemented in LoongArch.\n");
//#ifdef N64 /* validate address */
//    EXTENSION_MODE em_value = ir1_opnd_em(opnd);
//    return em_value == EM_X86_ADDRESS || em_value == EM_MIPS_ADDRESS;
//#else
    return false;
//#endif
}

int ir1_opnd_is_x86_address(IR1_OPND *opnd)
{
    lsassertm(0, "EM to be implemented in LoongArch.\n");
//#ifdef N64 /* validate address */
//    return ir1_opnd_em(opnd) == EM_X86_ADDRESS;
//#else
    return false;
//#endif
}

int ir1_opnd_is_mips_address(IR1_OPND *opnd)
{
    lsassertm(0, "EM to be implemented in LoongArch.\n");
//#ifdef N64 /* validate address */
//    return ir1_opnd_em(opnd) == EM_MIPS_ADDRESS;
//#else
    return false;
//#endif
}

//EXTENSION_MODE ir2_opnd_em(IR2_OPND *opnd)
//{
//    lsassert(ir2_opnd_is_ireg(opnd));
//    return lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(opnd)];
//}

//int ir2_opnd_eb(IR2_OPND *opnd)
//{
//    lsassert(ir2_opnd_is_ireg(opnd));
//    return lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(opnd)];
//}

//void ir2_opnd_set_em(IR2_OPND *opnd, EXTENSION_MODE e, int bits)
//{
//    if (bits >= 64 || e == UNKNOWN_EXTENSION) {
//        e = UNKNOWN_EXTENSION;
//        bits = 32;
//    } else if (bits < 0) {
//        bits = 0;
//    } else if (e == EM_X86_ADDRESS || e == EM_MIPS_ADDRESS) {
//        lsassert(bits == 32);
//    }
//
//    lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(opnd)] = e;
//    lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(opnd)] = bits;
//}

//int ir2_opnd_is_sx(IR2_OPND *opnd, int bits)
//{
//    EXTENSION_MODE em_value = ir2_opnd_em(opnd);
//    int eb_value = ir2_opnd_eb(opnd);
//
//    if (em_value == SIGN_EXTENSION && bits >= eb_value)
//        return 1;
//    else if (em_value == ZERO_EXTENSION && bits > eb_value)
//        return 1;
//    else
//        return 0;
//}

//int ir2_opnd_is_zx(IR2_OPND *opnd, int bits)
//{
//    EXTENSION_MODE em_value = ir2_opnd_em(opnd);
//    int eb_value = ir2_opnd_eb(opnd);
//    if (em_value == ZERO_EXTENSION && bits >= eb_value)
//        return 1;
//#ifdef N64 /* validate address */
//    else if (em_value == EM_X86_ADDRESS && bits >= 32)
//        return 1;
//#endif
//    else
//        return 0;
//}

//int ir2_opnd_is_bx(IR2_OPND *opnd, int bits)
//{
//    return ir2_opnd_is_sx(opnd, bits) && ir2_opnd_is_zx(opnd, bits);
//}

//int ir2_opnd_is_ax(IR2_OPND *opnd, int bits)
//{
//    return !ir2_opnd_is_sx(opnd, bits) && !ir2_opnd_is_zx(opnd, bits);
//}

//int ir2_opnd_is_address(IR2_OPND *opnd)
//{
//#ifdef N64 /* validate address */
//    EXTENSION_MODE em_value = ir2_opnd_em(opnd);
//    return em_value == EM_X86_ADDRESS || em_value == EM_MIPS_ADDRESS;
//#else
//    return 1;
//#endif
//}

//int ir2_opnd_is_x86_address(IR2_OPND *opnd)
//{
//#ifdef N64 /* validate address */
//    return ir2_opnd_em(opnd) == EM_X86_ADDRESS;
//#else
//    return 0;
//#endif
//}

//int ir2_opnd_is_mips_address(IR2_OPND *opnd)
//{
//#ifdef N64 /* validate address */
//    return ir2_opnd_em(opnd) == EM_MIPS_ADDRESS;
//#else
//    return 0;
//#endif
//}

//void ir2_opnd_set_em_mov(IR2_OPND *opnd, IR2_OPND *src)
//{
//    EXTENSION_MODE em0 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src)];
//    int eb0 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src)];
//
//    ir2_opnd_set_em(opnd, em0, eb0);
//}

//void ir2_opnd_set_em_add(IR2_OPND *opnd, IR2_OPND *src0, EXTENSION_MODE em1,
//                         int eb1)
//{
//    EXTENSION_MODE em0 = ir2_opnd_em(src0);
//    int eb0 = ir2_opnd_eb(src0);
//
//    lsassert(em0 != EM_MIPS_ADDRESS && em1 != EM_MIPS_ADDRESS);
//    if (em0 == EM_X86_ADDRESS) {
//        em0 = ZERO_EXTENSION;
//        eb0 = 32;
//    }
//    if (em1 == EM_X86_ADDRESS) {
//        em1 = ZERO_EXTENSION;
//        eb1 = 32;
//    }
//
//    if (em0 == ZERO_EXTENSION && em1 == ZERO_EXTENSION) {
//        int eb_value = eb0 > eb1 ? (eb0 + 1) : (eb1 + 1);
//        ir2_opnd_set_em(opnd, ZERO_EXTENSION, eb_value);
//        return;
//    } else if (em0 == UNKNOWN_EXTENSION || em1 == UNKNOWN_EXTENSION) {
//        ir2_opnd_set_em(opnd, UNKNOWN_EXTENSION, 32);
//        return;
//    } else {
//        /* now at least one em is SIGN_EXTENSION */
//        if (em0 == ZERO_EXTENSION)
//            eb0++; /* also a sign extension for eb0+1 */
//        if (em1 == ZERO_EXTENSION)
//            eb1++; /* also a sign extension for eb1+1 */
//        int eb_value = eb0 > eb1 ? (eb0 + 1) : (eb1 + 1);
//        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb_value);
//    }
//}

//void ir2_opnd_set_em_add2(IR2_OPND *opnd, IR2_OPND *src0, IR2_OPND *src1)
//{
//    ir2_opnd_set_em_add(opnd, src0, ir2_opnd_em(src1), ir2_opnd_eb(src1));
//}

//void ir2_opnd_set_em_sub(IR2_OPND *opnd, IR2_OPND *src0, EXTENSION_MODE em1,
//                         int eb1)
//{
//    EXTENSION_MODE em0 = ir2_opnd_em(src0);
//    int eb0 = ir2_opnd_eb(src0);
//
//    lsassert(em0 != EM_MIPS_ADDRESS && em1 != EM_MIPS_ADDRESS);
//    if (em0 == EM_X86_ADDRESS) {
//        em0 = ZERO_EXTENSION;
//        eb0 = 32;
//    }
//    if (em1 == EM_X86_ADDRESS) {
//        em1 = ZERO_EXTENSION;
//        eb1 = 32;
//    }
//
//    if (em0 == ZERO_EXTENSION && em1 == ZERO_EXTENSION) {
//        int eb_value = eb0 > eb1 ? (eb0 + 1) : (eb1 + 1);
//        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb_value);
//        return;
//    } else if (em0 == UNKNOWN_EXTENSION || em1 == UNKNOWN_EXTENSION) {
//        ir2_opnd_set_em(opnd, UNKNOWN_EXTENSION, 32);
//        return;
//    } else {
//        /* now at least one em is SIGN_EXTENSION */
//        if (em0 == ZERO_EXTENSION)
//            eb0++; /* also a sign extension for eb0+1 */
//        if (em1 == ZERO_EXTENSION)
//            eb1++; /* also a sign extension for eb1+1 */
//        int eb_value = eb0 > eb1 ? (eb0 + 1) : (eb1 + 1);
//        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb_value);
//    }
//}

//void ir2_opnd_set_em_sub2(IR2_OPND *opnd, IR2_OPND *src0, IR2_OPND *src1)
//{
//    ir2_opnd_set_em_sub(opnd, src0, ir2_opnd_em(src1), ir2_opnd_eb(src1));
//}

//void ir2_opnd_set_em_and(IR2_OPND *opnd, IR2_OPND *src0, EXTENSION_MODE em1,
//                         int eb1)
//{
//    EXTENSION_MODE em0 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src0)];
//    int eb0 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src0)];
//
//    lsassert(em0 != EM_MIPS_ADDRESS && em1 != EM_MIPS_ADDRESS);
//    if (em0 == EM_X86_ADDRESS) {
//        em0 = ZERO_EXTENSION;
//        eb0 = 32;
//    }
//    if (em1 == EM_X86_ADDRESS) {
//        em1 = ZERO_EXTENSION;
//        eb1 = 32;
//    }
//
//    if (em0 == ZERO_EXTENSION && em1 == ZERO_EXTENSION) {
//        ir2_opnd_set_em(opnd, ZERO_EXTENSION, eb0 > eb1 ? eb1 : eb0);
//    } else if (em0 == SIGN_EXTENSION && em1 == SIGN_EXTENSION) {
//        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb0 > eb1 ? eb0 : eb1);
//    } else if (em0 == ZERO_EXTENSION) {
//        ir2_opnd_set_em(opnd, ZERO_EXTENSION, eb0);
//    } else if (em1 == ZERO_EXTENSION) {
//        ir2_opnd_set_em(opnd, ZERO_EXTENSION, eb1);
//    } else {
//        ir2_opnd_set_em(opnd, UNKNOWN_EXTENSION, 32);
//    }
//}

//void ir2_opnd_set_em_and2(IR2_OPND *opnd, IR2_OPND *src0, IR2_OPND *src1)
//{
//    ir2_opnd_set_em_and(opnd, src0, ir2_opnd_em(src1), ir2_opnd_eb(src1));
//}

//void ir2_opnd_set_em_or(IR2_OPND *opnd, IR2_OPND *src0, EXTENSION_MODE em1,
//                        int eb1)
//{
//    EXTENSION_MODE em0 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src0)];
//    int eb0 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src0)];
//
//    lsassert(em0 != EM_MIPS_ADDRESS && em1 != EM_MIPS_ADDRESS);
//    if (em0 == EM_X86_ADDRESS) {
//        em0 = ZERO_EXTENSION;
//        eb0 = 32;
//    }
//    if (em1 == EM_X86_ADDRESS) {
//        em1 = ZERO_EXTENSION;
//        eb1 = 32;
//    }
//
//    if (em0 == ZERO_EXTENSION && em1 == ZERO_EXTENSION) {
//        ir2_opnd_set_em(opnd, ZERO_EXTENSION, eb0 > eb1 ? eb0 : eb1);
//    } else if (em0 == SIGN_EXTENSION && em1 == SIGN_EXTENSION) {
//        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb0 > eb1 ? eb0 : eb1);
//    } else if (em0 == UNKNOWN_EXTENSION || em1 == UNKNOWN_EXTENSION) {
//        ir2_opnd_set_em(opnd, UNKNOWN_EXTENSION, 32);
//    } else {
//        /* now we have one ZERO_EXTENSION and one SIGN_EXTENSION */
//        if (em0 == ZERO_EXTENSION)
//            eb0++; /* also a sign extension for eb0+1 */
//        else
//            eb1++; /* also a sign extension for eb1+1 */
//        int eb_value = eb0 > eb1 ? eb0 : eb1;
//        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb_value);
//    }
//}

//void ir2_opnd_set_em_or2(IR2_OPND *opnd, IR2_OPND *src0, IR2_OPND *src1)
//{
//    ir2_opnd_set_em_or(opnd, src0, ir2_opnd_em(src1), ir2_opnd_eb(src1));
//}

//void ir2_opnd_set_em_xor(IR2_OPND *opnd, IR2_OPND *src0, EXTENSION_MODE em1,
//                         int eb1)
//{
//    EXTENSION_MODE em0 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src0)];
//    int eb0 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src0)];
//
//    lsassert(em0 != EM_MIPS_ADDRESS && em1 != EM_MIPS_ADDRESS);
//    if (em0 == EM_X86_ADDRESS) {
//        em0 = ZERO_EXTENSION;
//        eb0 = 32;
//    }
//    if (em1 == EM_X86_ADDRESS) {
//        em1 = ZERO_EXTENSION;
//        eb1 = 32;
//    }
//
//    if (em0 == ZERO_EXTENSION && em1 == ZERO_EXTENSION) {
//        ir2_opnd_set_em(opnd, ZERO_EXTENSION, eb0 > eb1 ? eb0 : eb1);
//    } else if (em0 == SIGN_EXTENSION && em1 == SIGN_EXTENSION) {
//        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb0 > eb1 ? eb0 : eb1);
//    } else if (em0 == UNKNOWN_EXTENSION || em1 == UNKNOWN_EXTENSION) {
//        ir2_opnd_set_em(opnd, UNKNOWN_EXTENSION, 32);
//    } else {
//        /* now we have one ZERO_EXTENSION and one SIGN_EXTENSION */
//        if (em0 == ZERO_EXTENSION)
//            eb0++; /* also a sign extension for eb0+1 */
//        else
//            eb1++; /* also a sign extension for eb1+1 */
//        int eb_value = eb0 > eb1 ? eb0 : eb1;
//        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb_value);
//    }
//}

//void ir2_opnd_set_em_xor2(IR2_OPND *opnd, IR2_OPND *src0, IR2_OPND *src1)
//{
//    ir2_opnd_set_em_xor(opnd, src0, ir2_opnd_em(src1), ir2_opnd_eb(src1));
//}

//void ir2_opnd_set_em_nor(IR2_OPND *opnd, IR2_OPND *src0, EXTENSION_MODE em1,
//                         int eb1)
//{
//    EXTENSION_MODE em0 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src0)];
//    int eb0 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src0)];
//
//    lsassert(em0 != EM_MIPS_ADDRESS && em1 != EM_MIPS_ADDRESS);
//    if (em0 == EM_X86_ADDRESS) {
//        em0 = ZERO_EXTENSION;
//        eb0 = 32;
//    }
//    if (em1 == EM_X86_ADDRESS) {
//        em1 = ZERO_EXTENSION;
//        eb1 = 32;
//    }
//
//    if (em0 == ZERO_EXTENSION && em1 == ZERO_EXTENSION)
//        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb0 > eb1 ? eb0 + 1 : eb1 + 1);
//    else if (em0 == SIGN_EXTENSION && em1 == SIGN_EXTENSION)
//        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb0 > eb1 ? eb0 : eb1);
//    else if (em0 == UNKNOWN_EXTENSION || em1 == UNKNOWN_EXTENSION)
//        ir2_opnd_set_em(opnd, UNKNOWN_EXTENSION, 32);
//    else {
//        /* now we have one ZERO_EXTENSION and one SIGN_EXTENSION */
//        if (em0 == ZERO_EXTENSION)
//            eb0++; /* also a sign extension for eb0+1 */
//        else
//            eb1++; /* also a sign extension for eb1+1 */
//        int eb_value = eb0 > eb1 ? eb0 : eb1;
//        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb_value);
//    }
//}

//void ir2_opnd_set_em_nor2(IR2_OPND *opnd, IR2_OPND *src0, IR2_OPND *src1)
//{
//    ir2_opnd_set_em_nor(opnd, src0, ir2_opnd_em(src1), ir2_opnd_eb(src1));
//}

//void ir2_opnd_set_em_movcc(IR2_OPND *opnd, IR2_OPND *src0, IR2_OPND *src1)
//{
//    EXTENSION_MODE em0 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src0)];
//    EXTENSION_MODE em1 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src1)];
//    int eb0 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src0)];
//    int eb1 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src1)];
//
//    lsassert(em0 != EM_MIPS_ADDRESS && em1 != EM_MIPS_ADDRESS);
//    if (em0 == EM_X86_ADDRESS) {
//        em0 = ZERO_EXTENSION;
//        eb0 = 32;
//    }
//    if (em1 == EM_X86_ADDRESS) {
//        em1 = ZERO_EXTENSION;
//        eb1 = 32;
//    }
//
//    if (em0 == UNKNOWN_EXTENSION || em1 == UNKNOWN_EXTENSION)
//        ir2_opnd_set_em(opnd, UNKNOWN_EXTENSION, 32);
//    else if (em0 == ZERO_EXTENSION && em1 == ZERO_EXTENSION)
//        ir2_opnd_set_em(opnd, ZERO_EXTENSION, eb0 > eb1 ? eb0 : eb1);
//    else if (em0 == SIGN_EXTENSION && em1 == SIGN_EXTENSION)
//        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb0 > eb1 ? eb0 : eb1);
//    else {
//        /* now we have one ZERO_EXTENSION and one SIGN_EXTENSION */
//        if (em0 == ZERO_EXTENSION)
//            eb0++; /* also a sign extension for eb0+1 */
//        if (em1 == ZERO_EXTENSION)
//            eb1++; /* also a sign extension for eb1+1 */
//        int eb_value = eb0 > eb1 ? eb0 : eb1;
//        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb_value);
//    }
//}

//void ir2_opnd_set_em_dsll(IR2_OPND *opnd, IR2_OPND *src, int shift)
//{
//    EXTENSION_MODE src_em = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src)];
//    int src_eb = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src)];
//
//    lsassert(src_em != EM_MIPS_ADDRESS);
//    if (src_em == EM_X86_ADDRESS) {
//        src_em = ZERO_EXTENSION;
//        src_eb = 32;
//    }
//
//    ir2_opnd_set_em(opnd, src_em, src_eb + shift);
//}

//void ir2_opnd_set_em_dsrl(IR2_OPND *opnd, IR2_OPND *src, int shift)
//{
//    EXTENSION_MODE src_em = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src)];
//    int src_eb = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src)];
//
//    lsassert(src_em != EM_MIPS_ADDRESS);
//    if (src_em == EM_X86_ADDRESS) {
//        src_em = ZERO_EXTENSION;
//        src_eb = 32;
//    }
//
//    if (shift == 0)
//        ir2_opnd_set_em(opnd, src_em, src_eb);
//    else if (src_em == ZERO_EXTENSION)
//        ir2_opnd_set_em(opnd, ZERO_EXTENSION, src_eb - shift);
//    else
//        ir2_opnd_set_em(opnd, ZERO_EXTENSION, 64 - shift);
//}

//void ir2_opnd_set_em_dsra(IR2_OPND *opnd, IR2_OPND *src, int shift)
//{
//    EXTENSION_MODE src_em = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src)];
//    int src_eb = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src)];
//
//    lsassert(src_em != EM_MIPS_ADDRESS);
//    if (src_em == EM_X86_ADDRESS) {
//        src_em = ZERO_EXTENSION;
//        src_eb = 32;
//    }
//
//    if (src_em == UNKNOWN_EXTENSION)
//        ir2_opnd_set_em(opnd, SIGN_EXTENSION, 64 - shift);
//    else
//        ir2_opnd_set_em(opnd, src_em, src_eb - shift);
//}

//void ir2_opnd_set_em_dmult_g(IR2_OPND *opnd, IR2_OPND *src0, IR2_OPND *src1)
//{
//    EXTENSION_MODE em0 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src0)];
//    EXTENSION_MODE em1 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src1)];
//    int eb0 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src0)];
//    int eb1 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src1)];
//
//    lsassert(em0 != EM_MIPS_ADDRESS && em1 != EM_MIPS_ADDRESS);
//    if (em0 == EM_X86_ADDRESS) {
//        em0 = ZERO_EXTENSION;
//        eb0 = 32;
//    }
//    if (em1 == EM_X86_ADDRESS) {
//        em1 = ZERO_EXTENSION;
//        eb1 = 32;
//    }
//
//    if (em0 == UNKNOWN_EXTENSION || em1 == UNKNOWN_EXTENSION)
//        ir2_opnd_set_em(opnd, UNKNOWN_EXTENSION, 32);
//    else {
//        if (em0 == ZERO_EXTENSION) {
//            em0 = SIGN_EXTENSION;
//            eb0++;
//        }
//
//        if (em1 == ZERO_EXTENSION) {
//            em1 = SIGN_EXTENSION;
//            eb1++;
//        }
//
//        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb0 + eb1);
//    }
//}

//void ir2_opnd_set_em_dmultu_g(IR2_OPND *opnd, IR2_OPND *src0, IR2_OPND *src1)
//{
//    EXTENSION_MODE em0 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src0)];
//    EXTENSION_MODE em1 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src1)];
//    int eb0 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src0)];
//    int eb1 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src1)];
//
//    lsassert(em0 != EM_MIPS_ADDRESS && em1 != EM_MIPS_ADDRESS);
//    if (em0 == EM_X86_ADDRESS) {
//        em0 = ZERO_EXTENSION;
//        eb0 = 32;
//    }
//    if (em1 == EM_X86_ADDRESS) {
//        em1 = ZERO_EXTENSION;
//        eb1 = 32;
//    }
//
//    if (em0 == ZERO_EXTENSION && em1 == ZERO_EXTENSION)
//        ir2_opnd_set_em(opnd, ZERO_EXTENSION, eb1 + eb0);
//    else
//        ir2_opnd_set_em(opnd, UNKNOWN_EXTENSION, 32);
//}

void em_convert_gpr_to_addrx(IR1_OPND *opnd1)
{
    lsassertm(0, "EM to be implemented in LoongArch.\n");
//    lsassert(ir1_opnd_is_gpr(opnd1));
//    IR2_OPND ireg_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
//    append_ir2_opnd2(mips_mov_addrx, &ireg_opnd, &ireg_opnd);
}

//#ifdef N64 /* validate address */
//static void emi_recognize_address(IR1_OPND *opnd1)
//{
//    if (ir1_opnd_type(opnd1) !=
//        X86_OP_MEM || opnd1->mem.segment != X86_REG_INVALID) /* exclude segment memory operands */
//        return;
//    if (ir1_opnd_simm(opnd1) < -4096 || ir1_opnd_simm(opnd1) > 4096)
//        return;
//
//    if (ir1_opnd_has_base(opnd1)) {
//        IR2_OPND base_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
//        if (ir1_opnd_has_index(opnd1)) {
//            if (ir1_opnd_scale(opnd1) != 1) {
//                append_ir2_opnd2(mips_mov_addrx, &base_opnd, &base_opnd);
//            } else {
//                /* cannot determine whether base or index is address */
//            }
//        } else {
//            append_ir2_opnd2(mips_mov_addrx, &base_opnd, &base_opnd);
//        }
//    } else {
//        if (ir1_opnd_has_index(opnd1)) {
//            IR2_OPND index_opnd = ra_alloc_gpr(ir1_opnd_index_reg_num(opnd1));
//            append_ir2_opnd2(mips_mov_addrx, &index_opnd, &index_opnd);
//        } else {
//            /* no base and index */
//        }
//    }
//}
//#endif

void em_recognize_address(IR1_INST *pir1)
{
    lsassertm(0, "EM to be implemented in LoongArch.\n");
//#ifdef N64
//    if (ir1_opcode(pir1) == X86_INS_LEA)
//        return;
//
//    IR1_OPND *opnd1;
//    int i = 0;
//    while (1) {
//        opnd1 = ir1_get_src_opnd(pir1, i);
//        if (opnd1 == NULL) {
//            break;
//        }
//        i++;
//        emi_recognize_address(opnd1);
//    };
//    i = 0;
//    while (1) {
//        opnd1 = ir1_get_dest_opnd(pir1, i);
//        if (opnd1 == NULL) {
//            break;
//        }
//        i++;
//        emi_recognize_address(opnd1);
//    };
//
//#else
//    return;
//#endif
}

//static int emi_validate_address_flow(int base_ireg_num)
//{
//    IR2_OPND old_opnd = ir2_opnd_new(IR2_OPND_IREG, base_ireg_num);
//    IR2_OPND new_opnd = old_opnd;
//    if (!ir2_opnd_is_address(&old_opnd)) {
//        append_ir2_opnd2(mips_mov_addrx, &new_opnd, &old_opnd);
//    }
//    return ir2_opnd_base_reg_num(&new_opnd);
//}


int em_validate_address(int base_ireg_num)
{
    lsassertm(0, "EM to be implemented in LoongArch.\n");
//#ifdef N64  // validate address
//    return emi_validate_address_flow(base_ireg_num);
//#else
    return base_ireg_num;
//#endif
}

#ifdef CONFIG_SOFTMMU

void softmmu_fixup_em(IR2_OPCODE op, IR2_OPND *opnd_gpr)
{
    lsassertm(0, "EM to be implemented in LoongArch.\n");
//    switch (op) {
//        case mips_ld:  ir2_opnd_set_em(opnd_gpr, UNKNOWN_EXTENSION, 32); break;
//        case mips_lw:  ir2_opnd_set_em(opnd_gpr, SIGN_EXTENSION,    32); break;
//        case mips_lh:  ir2_opnd_set_em(opnd_gpr, SIGN_EXTENSION,    16); break;
//        case mips_lb:  ir2_opnd_set_em(opnd_gpr, SIGN_EXTENSION,     8); break;
//        case mips_lwu: ir2_opnd_set_em(opnd_gpr, ZERO_EXTENSION,    32); break;
//        case mips_lhu: ir2_opnd_set_em(opnd_gpr, ZERO_EXTENSION,    16); break;
//        case mips_lbu: ir2_opnd_set_em(opnd_gpr, ZERO_EXTENSION,     8); break;
//        default: break;
//    }
}

#endif
