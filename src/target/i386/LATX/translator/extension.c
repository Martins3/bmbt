#include "common.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "translate.h"

EXTENSION_MODE ir1_opnd_em(IR1_OPND *opnd)
{
    if (ir1_opnd_is_gpr(opnd)) {
        IR2_OPND ir2_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd));
        int mips_ireg_num = ir2_opnd_base_reg_num(&ir2_opnd);
        return lsenv->tr_data->ireg_em[mips_ireg_num];
    } else
        return SIGN_EXTENSION;
}

int ir1_opnd_eb(IR1_OPND *opnd)
{
    if (ir1_opnd_is_gpr(opnd)) {
        IR2_OPND ir2_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd));
        int mips_ireg_num = ir2_opnd_base_reg_num(&ir2_opnd);
        return lsenv->tr_data->ireg_eb[mips_ireg_num];
    } else
        return 32;
}

int ir1_opnd_is_sx(IR1_OPND *opnd, int bits)
{
    EXTENSION_MODE em_value = ir1_opnd_em(opnd);
    int eb_value = ir1_opnd_eb(opnd);

    if (em_value == SIGN_EXTENSION && bits >= eb_value)
        return true;
    else if (em_value == ZERO_EXTENSION && bits > eb_value)
        return true;
    else
        return false;
}

int ir1_opnd_is_zx(IR1_OPND *opnd, int bits)
{
    EXTENSION_MODE em_value = ir1_opnd_em(opnd);
    int eb_value = ir1_opnd_eb(opnd);

    if (em_value == ZERO_EXTENSION && bits >= eb_value)
        return true;
#ifdef N64 /* validate address */
    else if (em_value == EM_X86_ADDRESS && bits >= 32)
        return true;
#endif
    else
        return false;
}

int ir1_opnd_is_bx(IR1_OPND *opnd, int bits)
{
    return ir1_opnd_is_sx(opnd, bits) && ir1_opnd_is_zx(opnd, bits);
}

int ir1_opnd_is_ax(IR1_OPND *opnd, int bits)
{
    return !ir1_opnd_is_sx(opnd, bits) && !ir1_opnd_is_zx(opnd, bits);
}

int ir1_opnd_is_x86_address(IR1_OPND *opnd)
{
#ifdef N64 /* validate address */
    return ir1_opnd_em(opnd) == EM_X86_ADDRESS;
#else
    return false;
#endif
}

EXTENSION_MODE ir2_opnd_em(IR2_OPND *opnd)
{
    lsassert(ir2_opnd_is_ireg(opnd));
    return lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(opnd)];
}

int ir2_opnd_eb(IR2_OPND *opnd)
{
    lsassert(ir2_opnd_is_ireg(opnd));
    return lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(opnd)];
}

void ir2_opnd_set_em(IR2_OPND *opnd, EXTENSION_MODE e, int bits)
{
    if (bits >= 64 || e == UNKNOWN_EXTENSION) {
        e = UNKNOWN_EXTENSION;
        bits = 32;
    } else if (bits < 0) {
        bits = 0;
    } else if (e == EM_X86_ADDRESS || e == EM_MIPS_ADDRESS) {
        lsassert(bits == 32);
    }

    lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(opnd)] = e;
    lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(opnd)] = bits;
}

int ir2_opnd_is_sx(IR2_OPND *opnd, int bits)
{
    EXTENSION_MODE em_value = ir2_opnd_em(opnd);
    int eb_value = ir2_opnd_eb(opnd);

    if (em_value == SIGN_EXTENSION && bits >= eb_value)
        return 1;
    else if (em_value == ZERO_EXTENSION && bits > eb_value)
        return 1;
    else
        return 0;
}

int ir2_opnd_is_zx(IR2_OPND *opnd, int bits)
{
    EXTENSION_MODE em_value = ir2_opnd_em(opnd);
    int eb_value = ir2_opnd_eb(opnd);
    if (em_value == ZERO_EXTENSION && bits >= eb_value)
        return 1;
#ifdef N64 /* validate address */
    else if (em_value == EM_X86_ADDRESS && bits >= 32)
        return 1;
#endif
    else
        return 0;
}

int ir2_opnd_is_bx(IR2_OPND *opnd, int bits)
{
    return ir2_opnd_is_sx(opnd, bits) && ir2_opnd_is_zx(opnd, bits);
}

int ir2_opnd_is_ax(IR2_OPND *opnd, int bits)
{
    return !ir2_opnd_is_sx(opnd, bits) && !ir2_opnd_is_zx(opnd, bits);
}

int ir2_opnd_is_address(IR2_OPND *opnd)
{
#ifdef N64 /* validate address */
    EXTENSION_MODE em_value = ir2_opnd_em(opnd);
    return em_value == EM_X86_ADDRESS || em_value == EM_MIPS_ADDRESS;
#else
    return 1;
#endif
}

int ir2_opnd_is_x86_address(IR2_OPND *opnd)
{
#ifdef N64 /* validate address */
    return ir2_opnd_em(opnd) == EM_X86_ADDRESS;
#else
    return 0;
#endif
}

int ir2_opnd_is_mips_address(IR2_OPND *opnd)
{
#ifdef N64 /* validate address */
    return ir2_opnd_em(opnd) == EM_MIPS_ADDRESS;
#else
    return 0;
#endif
}

void ir2_opnd_set_em_mov(IR2_OPND *opnd, IR2_OPND *src)
{
    EXTENSION_MODE em0 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src)];
    int eb0 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src)];

    ir2_opnd_set_em(opnd, em0, eb0);
}

void ir2_opnd_set_em_add(IR2_OPND *opnd, IR2_OPND *src0, EXTENSION_MODE em1,
                         int eb1)
{
    EXTENSION_MODE em0 = ir2_opnd_em(src0);
    int eb0 = ir2_opnd_eb(src0);

    lsassert(em0 != EM_MIPS_ADDRESS && em1 != EM_MIPS_ADDRESS);
    if (em0 == EM_X86_ADDRESS) {
        em0 = ZERO_EXTENSION;
        eb0 = 32;
    }
    if (em1 == EM_X86_ADDRESS) {
        em1 = ZERO_EXTENSION;
        eb1 = 32;
    }

    if (em0 == ZERO_EXTENSION && em1 == ZERO_EXTENSION) {
        int eb_value = eb0 > eb1 ? (eb0 + 1) : (eb1 + 1);
        ir2_opnd_set_em(opnd, ZERO_EXTENSION, eb_value);
        return;
    } else if (em0 == UNKNOWN_EXTENSION || em1 == UNKNOWN_EXTENSION) {
        ir2_opnd_set_em(opnd, UNKNOWN_EXTENSION, 32);
        return;
    } else {
        /* now at least one em is SIGN_EXTENSION */
        if (em0 == ZERO_EXTENSION)
            eb0++; /* also a sign extension for eb0+1 */
        if (em1 == ZERO_EXTENSION)
            eb1++; /* also a sign extension for eb1+1 */
        int eb_value = eb0 > eb1 ? (eb0 + 1) : (eb1 + 1);
        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb_value);
    }
}

void ir2_opnd_set_em_add2(IR2_OPND *opnd, IR2_OPND *src0, IR2_OPND *src1)
{
    ir2_opnd_set_em_add(opnd, src0, ir2_opnd_em(src1), ir2_opnd_eb(src1));
}

void ir2_opnd_set_em_sub(IR2_OPND *opnd, IR2_OPND *src0, EXTENSION_MODE em1,
                         int eb1)
{
    EXTENSION_MODE em0 = ir2_opnd_em(src0);
    int eb0 = ir2_opnd_eb(src0);

    lsassert(em0 != EM_MIPS_ADDRESS && em1 != EM_MIPS_ADDRESS);
    if (em0 == EM_X86_ADDRESS) {
        em0 = ZERO_EXTENSION;
        eb0 = 32;
    }
    if (em1 == EM_X86_ADDRESS) {
        em1 = ZERO_EXTENSION;
        eb1 = 32;
    }

    if (em0 == ZERO_EXTENSION && em1 == ZERO_EXTENSION) {
        int eb_value = eb0 > eb1 ? (eb0 + 1) : (eb1 + 1);
        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb_value);
        return;
    } else if (em0 == UNKNOWN_EXTENSION || em1 == UNKNOWN_EXTENSION) {
        ir2_opnd_set_em(opnd, UNKNOWN_EXTENSION, 32);
        return;
    } else {
        /* now at least one em is SIGN_EXTENSION */
        if (em0 == ZERO_EXTENSION)
            eb0++; /* also a sign extension for eb0+1 */
        if (em1 == ZERO_EXTENSION)
            eb1++; /* also a sign extension for eb1+1 */
        int eb_value = eb0 > eb1 ? (eb0 + 1) : (eb1 + 1);
        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb_value);
    }
}

void ir2_opnd_set_em_sub2(IR2_OPND *opnd, IR2_OPND *src0, IR2_OPND *src1)
{
    ir2_opnd_set_em_sub(opnd, src0, ir2_opnd_em(src1), ir2_opnd_eb(src1));
}

void ir2_opnd_set_em_and(IR2_OPND *opnd, IR2_OPND *src0, EXTENSION_MODE em1,
                         int eb1)
{
    EXTENSION_MODE em0 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src0)];
    int eb0 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src0)];

    lsassert(em0 != EM_MIPS_ADDRESS && em1 != EM_MIPS_ADDRESS);
    if (em0 == EM_X86_ADDRESS) {
        em0 = ZERO_EXTENSION;
        eb0 = 32;
    }
    if (em1 == EM_X86_ADDRESS) {
        em1 = ZERO_EXTENSION;
        eb1 = 32;
    }

    if (em0 == ZERO_EXTENSION && em1 == ZERO_EXTENSION) {
        ir2_opnd_set_em(opnd, ZERO_EXTENSION, eb0 > eb1 ? eb1 : eb0);
    } else if (em0 == SIGN_EXTENSION && em1 == SIGN_EXTENSION) {
        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb0 > eb1 ? eb0 : eb1);
    } else if (em0 == ZERO_EXTENSION) {
        ir2_opnd_set_em(opnd, ZERO_EXTENSION, eb0);
    } else if (em1 == ZERO_EXTENSION) {
        ir2_opnd_set_em(opnd, ZERO_EXTENSION, eb1);
    } else {
        ir2_opnd_set_em(opnd, UNKNOWN_EXTENSION, 32);
    }
}

void ir2_opnd_set_em_and2(IR2_OPND *opnd, IR2_OPND *src0, IR2_OPND *src1)
{
    ir2_opnd_set_em_and(opnd, src0, ir2_opnd_em(src1), ir2_opnd_eb(src1));
}

void ir2_opnd_set_em_or(IR2_OPND *opnd, IR2_OPND *src0, EXTENSION_MODE em1,
                        int eb1)
{
    EXTENSION_MODE em0 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src0)];
    int eb0 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src0)];

    lsassert(em0 != EM_MIPS_ADDRESS && em1 != EM_MIPS_ADDRESS);
    if (em0 == EM_X86_ADDRESS) {
        em0 = ZERO_EXTENSION;
        eb0 = 32;
    }
    if (em1 == EM_X86_ADDRESS) {
        em1 = ZERO_EXTENSION;
        eb1 = 32;
    }

    if (em0 == ZERO_EXTENSION && em1 == ZERO_EXTENSION) {
        ir2_opnd_set_em(opnd, ZERO_EXTENSION, eb0 > eb1 ? eb0 : eb1);
    } else if (em0 == SIGN_EXTENSION && em1 == SIGN_EXTENSION) {
        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb0 > eb1 ? eb0 : eb1);
    } else if (em0 == UNKNOWN_EXTENSION || em1 == UNKNOWN_EXTENSION) {
        ir2_opnd_set_em(opnd, UNKNOWN_EXTENSION, 32);
    } else {
        /* now we have one ZERO_EXTENSION and one SIGN_EXTENSION */
        if (em0 == ZERO_EXTENSION)
            eb0++; /* also a sign extension for eb0+1 */
        else
            eb1++; /* also a sign extension for eb1+1 */
        int eb_value = eb0 > eb1 ? eb0 : eb1;
        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb_value);
    }
}

void ir2_opnd_set_em_or2(IR2_OPND *opnd, IR2_OPND *src0, IR2_OPND *src1)
{
    ir2_opnd_set_em_or(opnd, src0, ir2_opnd_em(src1), ir2_opnd_eb(src1));
}

void ir2_opnd_set_em_xor(IR2_OPND *opnd, IR2_OPND *src0, EXTENSION_MODE em1,
                         int eb1)
{
    EXTENSION_MODE em0 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src0)];
    int eb0 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src0)];

    lsassert(em0 != EM_MIPS_ADDRESS && em1 != EM_MIPS_ADDRESS);
    if (em0 == EM_X86_ADDRESS) {
        em0 = ZERO_EXTENSION;
        eb0 = 32;
    }
    if (em1 == EM_X86_ADDRESS) {
        em1 = ZERO_EXTENSION;
        eb1 = 32;
    }

    if (em0 == ZERO_EXTENSION && em1 == ZERO_EXTENSION) {
        ir2_opnd_set_em(opnd, ZERO_EXTENSION, eb0 > eb1 ? eb0 : eb1);
    } else if (em0 == SIGN_EXTENSION && em1 == SIGN_EXTENSION) {
        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb0 > eb1 ? eb0 : eb1);
    } else if (em0 == UNKNOWN_EXTENSION || em1 == UNKNOWN_EXTENSION) {
        ir2_opnd_set_em(opnd, UNKNOWN_EXTENSION, 32);
    } else {
        /* now we have one ZERO_EXTENSION and one SIGN_EXTENSION */
        if (em0 == ZERO_EXTENSION)
            eb0++; /* also a sign extension for eb0+1 */
        else
            eb1++; /* also a sign extension for eb1+1 */
        int eb_value = eb0 > eb1 ? eb0 : eb1;
        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb_value);
    }
}

void ir2_opnd_set_em_xor2(IR2_OPND *opnd, IR2_OPND *src0, IR2_OPND *src1)
{
    ir2_opnd_set_em_xor(opnd, src0, ir2_opnd_em(src1), ir2_opnd_eb(src1));
}

void ir2_opnd_set_em_nor(IR2_OPND *opnd, IR2_OPND *src0, EXTENSION_MODE em1,
                         int eb1)
{
    EXTENSION_MODE em0 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src0)];
    int eb0 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src0)];

    lsassert(em0 != EM_MIPS_ADDRESS && em1 != EM_MIPS_ADDRESS);
    if (em0 == EM_X86_ADDRESS) {
        em0 = ZERO_EXTENSION;
        eb0 = 32;
    }
    if (em1 == EM_X86_ADDRESS) {
        em1 = ZERO_EXTENSION;
        eb1 = 32;
    }

    if (em0 == ZERO_EXTENSION && em1 == ZERO_EXTENSION)
        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb0 > eb1 ? eb0 + 1 : eb1 + 1);
    else if (em0 == SIGN_EXTENSION && em1 == SIGN_EXTENSION)
        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb0 > eb1 ? eb0 : eb1);
    else if (em0 == UNKNOWN_EXTENSION || em1 == UNKNOWN_EXTENSION)
        ir2_opnd_set_em(opnd, UNKNOWN_EXTENSION, 32);
    else {
        /* now we have one ZERO_EXTENSION and one SIGN_EXTENSION */
        if (em0 == ZERO_EXTENSION)
            eb0++; /* also a sign extension for eb0+1 */
        else
            eb1++; /* also a sign extension for eb1+1 */
        int eb_value = eb0 > eb1 ? eb0 : eb1;
        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb_value);
    }
}

void ir2_opnd_set_em_nor2(IR2_OPND *opnd, IR2_OPND *src0, IR2_OPND *src1)
{
    ir2_opnd_set_em_nor(opnd, src0, ir2_opnd_em(src1), ir2_opnd_eb(src1));
}

void ir2_opnd_set_em_movcc(IR2_OPND *opnd, IR2_OPND *src0, IR2_OPND *src1)
{
    EXTENSION_MODE em0 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src0)];
    EXTENSION_MODE em1 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src1)];
    int eb0 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src0)];
    int eb1 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src1)];

    lsassert(em0 != EM_MIPS_ADDRESS && em1 != EM_MIPS_ADDRESS);
    if (em0 == EM_X86_ADDRESS) {
        em0 = ZERO_EXTENSION;
        eb0 = 32;
    }
    if (em1 == EM_X86_ADDRESS) {
        em1 = ZERO_EXTENSION;
        eb1 = 32;
    }

    if (em0 == UNKNOWN_EXTENSION || em1 == UNKNOWN_EXTENSION)
        ir2_opnd_set_em(opnd, UNKNOWN_EXTENSION, 32);
    else if (em0 == ZERO_EXTENSION && em1 == ZERO_EXTENSION)
        ir2_opnd_set_em(opnd, ZERO_EXTENSION, eb0 > eb1 ? eb0 : eb1);
    else if (em0 == SIGN_EXTENSION && em1 == SIGN_EXTENSION)
        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb0 > eb1 ? eb0 : eb1);
    else {
        /* now we have one ZERO_EXTENSION and one SIGN_EXTENSION */
        if (em0 == ZERO_EXTENSION)
            eb0++; /* also a sign extension for eb0+1 */
        if (em1 == ZERO_EXTENSION)
            eb1++; /* also a sign extension for eb1+1 */
        int eb_value = eb0 > eb1 ? eb0 : eb1;
        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb_value);
    }
}

void ir2_opnd_set_em_dsll(IR2_OPND *opnd, IR2_OPND *src, int shift)
{
    EXTENSION_MODE src_em = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src)];
    int src_eb = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src)];

    lsassert(src_em != EM_MIPS_ADDRESS);
    if (src_em == EM_X86_ADDRESS) {
        src_em = ZERO_EXTENSION;
        src_eb = 32;
    }

    ir2_opnd_set_em(opnd, src_em, src_eb + shift);
}

void ir2_opnd_set_em_dsrl(IR2_OPND *opnd, IR2_OPND *src, int shift)
{
    EXTENSION_MODE src_em = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src)];
    int src_eb = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src)];

    lsassert(src_em != EM_MIPS_ADDRESS);
    if (src_em == EM_X86_ADDRESS) {
        src_em = ZERO_EXTENSION;
        src_eb = 32;
    }

    if (shift == 0)
        ir2_opnd_set_em(opnd, src_em, src_eb);
    else if (src_em == ZERO_EXTENSION)
        ir2_opnd_set_em(opnd, ZERO_EXTENSION, src_eb - shift);
    else
        ir2_opnd_set_em(opnd, ZERO_EXTENSION, 64 - shift);
}

void ir2_opnd_set_em_dsra(IR2_OPND *opnd, IR2_OPND *src, int shift)
{
    EXTENSION_MODE src_em = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src)];
    int src_eb = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src)];

    lsassert(src_em != EM_MIPS_ADDRESS);
    if (src_em == EM_X86_ADDRESS) {
        src_em = ZERO_EXTENSION;
        src_eb = 32;
    }

    if (src_em == UNKNOWN_EXTENSION)
        ir2_opnd_set_em(opnd, SIGN_EXTENSION, 64 - shift);
    else
        ir2_opnd_set_em(opnd, src_em, src_eb - shift);
}

void ir2_opnd_set_em_dmult_g(IR2_OPND *opnd, IR2_OPND *src0, IR2_OPND *src1)
{
    EXTENSION_MODE em0 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src0)];
    EXTENSION_MODE em1 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src1)];
    int eb0 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src0)];
    int eb1 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src1)];

    lsassert(em0 != EM_MIPS_ADDRESS && em1 != EM_MIPS_ADDRESS);
    if (em0 == EM_X86_ADDRESS) {
        em0 = ZERO_EXTENSION;
        eb0 = 32;
    }
    if (em1 == EM_X86_ADDRESS) {
        em1 = ZERO_EXTENSION;
        eb1 = 32;
    }

    if (em0 == UNKNOWN_EXTENSION || em1 == UNKNOWN_EXTENSION)
        ir2_opnd_set_em(opnd, UNKNOWN_EXTENSION, 32);
    else {
        if (em0 == ZERO_EXTENSION) {
            em0 = SIGN_EXTENSION;
            eb0++;
        }

        if (em1 == ZERO_EXTENSION) {
            em1 = SIGN_EXTENSION;
            eb1++;
        }

        ir2_opnd_set_em(opnd, SIGN_EXTENSION, eb0 + eb1);
    }
}

void ir2_opnd_set_em_dmultu_g(IR2_OPND *opnd, IR2_OPND *src0, IR2_OPND *src1)
{
    EXTENSION_MODE em0 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src0)];
    EXTENSION_MODE em1 = lsenv->tr_data->ireg_em[ir2_opnd_base_reg_num(src1)];
    int eb0 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src0)];
    int eb1 = lsenv->tr_data->ireg_eb[ir2_opnd_base_reg_num(src1)];

    lsassert(em0 != EM_MIPS_ADDRESS && em1 != EM_MIPS_ADDRESS);
    if (em0 == EM_X86_ADDRESS) {
        em0 = ZERO_EXTENSION;
        eb0 = 32;
    }
    if (em1 == EM_X86_ADDRESS) {
        em1 = ZERO_EXTENSION;
        eb1 = 32;
    }

    if (em0 == ZERO_EXTENSION && em1 == ZERO_EXTENSION)
        ir2_opnd_set_em(opnd, ZERO_EXTENSION, eb1 + eb0);
    else
        ir2_opnd_set_em(opnd, UNKNOWN_EXTENSION, 32);
}

void em_convert_gpr_to_addrx(IR1_OPND *opnd1)
{
    lsassert(ir1_opnd_is_gpr(opnd1));
    IR2_OPND ireg_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd1));
    la_append_ir2_opnd2_em(LISA_MOV_ADDRX, ireg_opnd,
                     ireg_opnd); /* replace by shell script */
}

static int emi_validate_address_flow(int base_ireg_num) {
    IR2_OPND old_opnd = ir2_opnd_new(IR2_OPND_IREG, base_ireg_num);
    IR2_OPND new_opnd = old_opnd;
    if (!ir2_opnd_is_address(&old_opnd)) {
        la_append_ir2_opnd2_em(LISA_MOV_ADDRX, new_opnd, old_opnd);
    }
    return ir2_opnd_base_reg_num(&new_opnd);
}

int em_validate_address(int base_ireg_num) {
#ifdef N64  // validate address
    /*
    if (env->is_in_mda_process())
        return base_ireg_num;
    */
    return emi_validate_address_flow(base_ireg_num);
#else
    return base_ireg_num;
#endif
}

#ifdef CONFIG_SOFTMMU

void latxs_tr_em_init(TRANSLATION_DATA *td)
{
    int i = 0;
    for (i = 0; i < CPU_NB_REGS; ++i) {
        td->reg_exmode[i] = EXMode_S;
        td->reg_exbits[i] = 32;
    }
}

EXMode latxs_td_get_reg_extm(int gpr)
{
    lsassert(gpr < CPU_NB_REGS);
    return lsenv->tr_data->reg_exmode[gpr];
}

EXBits latxs_td_get_reg_extb(int gpr)
{
    lsassert(gpr < CPU_NB_REGS);
    return lsenv->tr_data->reg_exbits[gpr];
}

void latxs_td_set_reg_extm(int gpr, EXMode em)
{
    lsassert(gpr < CPU_NB_REGS);
    lsenv->tr_data->reg_exmode[gpr] = em;
}

void latxs_td_set_reg_extb(int gpr, EXBits eb)
{
    lsassert(gpr < CPU_NB_REGS);
    lsassert(eb <= 32);
    lsenv->tr_data->reg_exbits[gpr] = eb;
}

void latxs_td_set_reg_extmb(int gpr, EXMode em, EXBits eb)
{
    lsassert(gpr < CPU_NB_REGS);
    lsassert(eb <= 32);
    lsenv->tr_data->reg_exmode[gpr] = em;
    lsenv->tr_data->reg_exbits[gpr] = eb;
}

#endif
