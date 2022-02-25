#include "../include/common.h"
/*#include "translate.h"*/
#include "../include/reg-alloc.h"
#include "../include/env.h"
#include "../x86tomips-options.h"

void translate_xxxps_helper(IR1_INST *pir1, bool is_xmm_hi,
                            IR2_OPCODE mips_opcode);

void translate_xxxps_helper(
        IR1_INST *pir1,
        bool is_xmm_hi,
        IR2_OPCODE mips_opcode)
{
    lsassertm(0, "SIMD xxxps to be implemented in LoongArch.\n");
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    IR2_OPND dest = load_freg_from_ir1_1(opnd0, is_xmm_hi, true);
//
//    /* 1.1. retrive the 0th and 1th single from half of the xmm operand */
//    IR2_OPND src_single_0  = ra_alloc_ftemp();
//    IR2_OPND src_single_1  = ra_alloc_ftemp();
//    IR2_OPND dest_single_0 = ra_alloc_ftemp();
//    IR2_OPND dest_single_1 = ra_alloc_ftemp();
//
//    load_singles_from_ir1_pack(&src_single_0,  &src_single_1,  opnd1, is_xmm_hi);
//    load_singles_from_ir1_pack(&dest_single_0, &dest_single_1, opnd0, is_xmm_hi);
//
//    /* 1.2. calculate the result of 0th and 1th single */
//    append_ir2_opnd3(mips_opcode, &dest_single_0, &dest_single_0, &src_single_0);
//    append_ir2_opnd3(mips_opcode, &dest_single_1, &dest_single_1, &src_single_1);
//
//    /* 1.3. store the result back to xmm register */
//    store_singles_to_ir2_pack(&dest_single_0, &dest_single_1, &dest);
//
//    ra_free_temp(&src_single_0);
//    ra_free_temp(&src_single_1);
//    ra_free_temp(&dest_single_0);
//    ra_free_temp(&dest_single_1);
}

bool translate_por(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if(ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = load_freg128_from_ir1(opnd0);
        IR2_OPND src  = load_freg128_from_ir1(opnd1);
        append_ir2_opnd3(LISA_VOR_V, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);

    append_ir2_opnd3(LISA_VOR_V, &dest_lo, &dest_lo, &src_lo);

//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_for, &dest_hi, &dest_hi, &src_hi);
//    }
//
    return true;
}

bool translate_pxor(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR1_OPND *dest = opnd0;
    IR1_OPND *src  = opnd1;
    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_mem(src)) {
        IR2_OPND temp = ra_alloc_ftemp();
        load_freg128_from_ir1_mem(&temp, src);
        IR2_OPND xmm_dest = ra_alloc_xmm(ir1_opnd_base_reg_num(dest));
        append_ir2_opnd3(LISA_VXOR_V, &xmm_dest, &xmm_dest, &temp);
//        append_ir2_opnd3(mips_xorv, &xmm_dest, &xmm_dest, &temp);
        return true;
    }
    else if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_xmm(src)) {
        IR2_OPND xmm_dest = ra_alloc_xmm(ir1_opnd_base_reg_num(dest));
        IR2_OPND xmm_src  = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
        append_ir2_opnd3(LISA_VXOR_V, &xmm_dest, &xmm_dest, &xmm_src);
//        append_ir2_opnd3(mips_xorv, &xmm_dest, &xmm_dest, &xmm_src);
        return true;
    }

    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);

    append_ir2_opnd3(LISA_VXOR_V, &dest_lo, &dest_lo, &src_lo);
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_fxor, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_packsswb(IR1_INST *pir1)
{
    lsassertm(0, "SIMD packsswb to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if (ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            if (ir1_opnd_is_xmm(opnd1) &&
//                (ir1_opnd_base_reg_num(opnd0) ==
//                 ir1_opnd_base_reg_num(opnd1)))
//            {
//                append_ir2_opnd2i(mips_vsrains_sb, &dest, &dest, 0);
//                append_ir2_opnd2ii(mips_vextrinsd, &dest, &dest, 1, 0);
//            }
//            else {
//                IR2_OPND temp = ra_alloc_ftemp();
//                append_ir2_opnd2i(mips_vsrains_sb, &dest, &dest, 0);
//                append_ir2_opnd2i(mips_vsrains_sb, &temp, &src, 0);
//                append_ir2_opnd2ii(mips_vextrinsd, &dest, &temp, 1, 0);
//            }
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) {
//        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src = load_freg_from_ir1_1(opnd1, false, true);
//        append_ir2_opnd3(mips_packsshb, &dest, &dest, &src);
//    }
//    else {
//        if (ir1_opnd_is_xmm(opnd1) &&
//            (ir1_opnd_base_reg_num(opnd0) == ir1_opnd_base_reg_num(opnd1)))
//        {
//            IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//            append_ir2_opnd3(mips_packsshb, &dest_lo, &dest_lo, &dest_hi);
//            store_freg_to_ir1(&dest_lo, opnd0, true, true);
//        }
//        else {
//            IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//            append_ir2_opnd3(mips_packsshb, &dest_lo, &dest_lo, &dest_hi);
//
//            IR2_OPND src_lo = load_freg_from_ir1_1(opnd1, false, true);
//            IR2_OPND src_hi = load_freg_from_ir1_1(opnd1, true, true);
//            append_ir2_opnd3(mips_packsshb, &dest_hi, &src_lo, &src_hi);
//        }
//    }
    return true;
}

bool translate_packssdw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD packssdw to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if (ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src = load_freg128_from_ir1(opnd1);
//            if (ir1_opnd_is_xmm(opnd1) &&
//                (ir1_opnd_base_reg_num(opnd0) ==
//                 ir1_opnd_base_reg_num(opnd1)))
//            {
//                append_ir2_opnd2i(mips_vsrains_sh, &dest, &dest, 0);
//                append_ir2_opnd2ii(mips_vextrinsd, &dest, &dest, 1, 0);
//            }
//            else {
//                IR2_OPND temp = ra_alloc_ftemp();
//                append_ir2_opnd2i(mips_vsrains_sh, &dest, &dest, 0);
//                append_ir2_opnd2i(mips_vsrains_sh, &temp, &src, 0);
//                append_ir2_opnd2ii(mips_vextrinsd, &dest, &temp, 1, 0);
//            }
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) {
//        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src = load_freg_from_ir1_1(opnd1, false, true);
//        append_ir2_opnd3(mips_packsswh, &dest, &dest, &src);
//    }
//    else {
//        if (ir1_opnd_is_xmm(opnd1) &&
//            (ir1_opnd_base_reg_num(opnd0) ==
//             ir1_opnd_base_reg_num(opnd1)))
//        {
//            IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//            append_ir2_opnd3(mips_packsswh, &dest_lo, &dest_lo, &dest_hi);
//
//            store_freg_to_ir1(&dest_lo, opnd0, true, true);
//        }
//        else {
//            IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//            append_ir2_opnd3(mips_packsswh, &dest_lo, &dest_lo, &dest_hi);
//
//            IR2_OPND src_lo = load_freg_from_ir1_1(opnd1, false, true);
//            IR2_OPND src_hi = load_freg_from_ir1_1(opnd1, true, true);
//            append_ir2_opnd3(mips_packsswh, &dest_hi, &src_lo, &src_hi);
//        }
//    }

    return true;
}

bool translate_packuswb(IR1_INST *pir1)
{
    lsassertm(0, "SIMD packuswb to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if (ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src = load_freg128_from_ir1(opnd1);
//            if (ir1_opnd_is_xmm(opnd1) &&
//                (ir1_opnd_base_reg_num(opnd0) ==
//                 ir1_opnd_base_reg_num(opnd1)))
//            {
//                append_ir2_opnd2i(mips_vsrains_ub, &dest, &dest, 0);
//                append_ir2_opnd2ii(mips_vextrinsd, &dest, &dest, 1, 0);
//            } else {
//                append_ir2_opnd2i(mips_vsrains_ub, &dest, &dest, 0);
//                IR2_OPND temp = ra_alloc_ftemp();
//                append_ir2_opnd2i(mips_vsrains_ub, &temp, &src, 0);
//                append_ir2_opnd2ii(mips_vextrinsd, &dest, &temp, 1, 0);
//            }
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) {
//        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src = load_freg_from_ir1_1(opnd1, false, true);
//        append_ir2_opnd3(mips_packushb, &dest, &dest, &src);
//    }
//    else {
//        if (ir1_opnd_is_xmm(opnd1) &&
//            (ir1_opnd_base_reg_num(opnd0) ==
//             ir1_opnd_base_reg_num(opnd1)))
//        {
//            IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//            append_ir2_opnd3(mips_packushb, &dest_lo, &dest_lo, &dest_hi);
//            store_freg_to_ir1(&dest_lo, opnd0, true, true);
//        }
//        else {
//            IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//            append_ir2_opnd3(mips_packushb, &dest_lo, &dest_lo, &dest_hi);
//
//            IR2_OPND src_lo = load_freg_from_ir1_1(opnd1, false, true);
//            IR2_OPND src_hi = load_freg_from_ir1_1(opnd1, true, true);
//            append_ir2_opnd3(mips_packushb, &dest_hi, &src_lo, &src_hi);
//        }
//    }

    return true;
}

bool translate_paddb(IR1_INST *pir1)
{
    lsassertm(0, "SIMD paddb to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_addvb, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_paddb, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_paddb, &dest_hi, &dest_hi, &src_hi);
//    }
//
    return true;
}

bool translate_paddw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD paddw to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_addvh, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_paddh, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_paddh, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_paddd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_addvw, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_paddw, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_paddw, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_paddsb(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_adds_sb, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_paddsb, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_paddsb, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_paddsw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_adds_sh, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_paddsh, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_paddsh, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_paddusb(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_adds_ub, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_paddusb, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_paddusb, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_paddusw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_adds_uh, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_paddush, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_paddush, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_pand(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_andv, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_fand, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_fand, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_pandn(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_vandnv, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_pandn, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_pandn, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_pcmpeqb(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if(ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = load_freg128_from_ir1(opnd0);
        IR2_OPND src  = load_freg128_from_ir1(opnd1);
        append_ir2_opnd3(LISA_VSEQ_B, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);

    append_ir2_opnd3(LISA_VSEQ_B, &dest_lo, &dest_lo, &src_lo);

//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_pcmpeqb, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_pcmpeqw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_ceqh, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_pcmpeqh, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_pcmpeqh, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_pcmpeqd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_ceqw, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_pcmpeqw, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_pcmpeqw, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_pcmpgtb(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//            if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_clt_sb, &dest, &src, &dest);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_pcmpgtb, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_pcmpgtb, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_pcmpgtw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_clt_sh, &dest, &src, &dest);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_pcmpgth, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_pcmpgth, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_pcmpgtd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)){
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_clt_sw, &dest, &src, &dest);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_pcmpgtw, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_pcmpgtw, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_pmaddwd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_dotp_sw, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_pmaddhw, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_pmaddhw, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_pmulhuw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_vmulhi_uh, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) { /* dest mmx */
//        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, true);
//        append_ir2_opnd3(mips_pmulhuh, &dest, &dest, &src);
//    }
//    else {
//        IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//        append_ir2_opnd3(mips_pmulhuh, &dest_lo, &dest_lo, &src_lo);
//
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        append_ir2_opnd3(mips_pmulhuh, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_pmulhw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_vmulhi_sh, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) { /* dest mmx */
//        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, true);
//        append_ir2_opnd3(mips_pmulhh, &dest, &dest, &src);
//    }
//    else {
//        IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//        append_ir2_opnd3(mips_pmulhh, &dest_lo, &dest_lo, &src_lo);
//
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        append_ir2_opnd3(mips_pmulhh, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_pmullw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_mulvh, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) { /* dest mmx */
//        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src = load_freg_from_ir1_1(opnd1, false, true);
//        append_ir2_opnd3(mips_pmullh, &dest, &dest, &src);
//    }
//    else {
//        IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//        append_ir2_opnd3(mips_pmullh, &dest_lo, &dest_lo, &src_lo);
//
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        append_ir2_opnd3(mips_pmullh, &dest_hi, &dest_hi, &src_hi);
//    }
//
    return true;
}

bool translate_psllw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if (ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            if (ir1_opnd_is_xmm(opnd1) ||
//                ir1_opnd_is_mem(opnd1))
//            {
//                IR2_OPND src = load_freg128_from_ir1(opnd1);
//                IR2_OPND temp1 = ra_alloc_ftemp();
//                IR2_OPND temp2 = ra_alloc_ftemp();
//                IR2_OPND temp3 = ra_alloc_ftemp();
//
//                append_ir2_opnd2i(mips_splatid, &temp1, &src, 0);
//                append_ir2_opnd1i(mips_ldid, &temp2, 16);
//                append_ir2_opnd3(mips_clt_ud, &temp3, &temp1, &temp2);
//                append_ir2_opnd2i(mips_splatih, &temp1, &src, 0);
//                append_ir2_opnd3(mips_sllh, &dest, &dest, &temp1);
//                append_ir2_opnd3(mips_andv, &dest, &dest, &temp3);
//            }
//            else if (ir1_opnd_is_imm(opnd1)) {
//                uint8_t imm = ir1_opnd_uimm(opnd1);
//                if (imm > 15) {
//                    append_ir2_opnd3(mips_xorv, &dest, &dest, &dest);
//                } else {
//                    append_ir2_opnd2i(mips_sllih, &dest, &dest, imm);
//                }
//            } else {
//                lsassert(0);
//            }
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) { /* dest mmx */
//        if (ir1_opnd_is_imm(opnd1)) {
//            IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//            uint8 imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 0) + 1);
//
//            if (imm > 15) {
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest);
//            }
//            else {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                load_imm32_to_ir2(&itemp, imm, UNKNOWN_EXTENSION);
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//
//                append_ir2_opnd3(mips_psllh, &dest, &dest, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            }
//        }
//        else {
//            IR2_OPND temp_imm = ra_alloc_itemp();
//            IR2_OPND temp = ra_alloc_itemp();
//            IR2_OPND target_label = ir2_opnd_new_type(IR2_OPND_LABEL);
//            IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, true);
//
//            load_imm32_to_ir2(&temp_imm, 0xff80, SIGN_EXTENSION);
//
//            append_ir2_opnd2(mips_dmfc1, &temp, &src);
//            append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//            append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//            append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest);
//
//            append_ir2_opnd1(mips_label, &target_label);
//            append_ir2_opnd3(mips_psllh, &dest, &dest, &src);
//
//            ra_free_temp(&temp_imm);
//            ra_free_temp(&temp);
//        }
//    }
//    else { /* dest xmm */
//        if (ir1_opnd_is_imm(opnd1)) {
//            uint8 imm = ir1_opnd_uimm(opnd1);
//            IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//            if (imm > 15) {
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_lo);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_hi);
//            }
//            else {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//
//                load_imm32_to_ir2(&itemp, imm, UNKNOWN_EXTENSION);
//
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//                append_ir2_opnd3(mips_psllh, &dest_lo, &dest_lo, &ftemp);
//                append_ir2_opnd3(mips_psllh, &dest_hi, &dest_hi, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            }
//        }
//        else { /* src xmm */
//            if (ir1_opnd_is_xmm(opnd1) &&
//                (ir1_opnd_base_reg_num(opnd0) ==
//                 ir1_opnd_base_reg_num(opnd1)))
//            {
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                IR2_OPND temp_imm = ra_alloc_itemp();
//                IR2_OPND temp = ra_alloc_itemp();
//
//                IR2_OPND target_label = ir2_opnd_new_type( IR2_OPND_LABEL);
//                IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//                IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//                load_imm32_to_ir2(&temp_imm, 0xff80, SIGN_EXTENSION);
//
//                append_ir2_opnd2(mips_dmfc1, &temp, &dest_lo);
//                append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//                append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_lo);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_hi);
//
//                append_ir2_opnd1(mips_label, &target_label);
//                append_ir2_opnd2(mips_mov_d, &ftemp, &dest_lo);
//
//                append_ir2_opnd3(mips_psllh, &dest_lo, &dest_lo, &ftemp);
//
//                append_ir2_opnd3(mips_psllh, &dest_hi, &dest_hi, &ftemp);
//
//                ra_free_temp(&ftemp);
//                ra_free_temp(&temp_imm);
//                ra_free_temp(&temp);
//            }
//            else {
//                IR2_OPND temp_imm = ra_alloc_itemp();
//                IR2_OPND temp = ra_alloc_itemp();
//
//                IR2_OPND target_label = ir2_opnd_new_type( IR2_OPND_LABEL);
//                IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//                IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//                IR2_OPND src_lo = load_freg_from_ir1_1(opnd1, false, true);
//
//                load_imm32_to_ir2(&temp_imm, 0xff80, SIGN_EXTENSION);
//
//                append_ir2_opnd2(mips_dmfc1, &temp, &src_lo);
//                append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//                append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_lo);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_hi);
//
//                append_ir2_opnd1(mips_label, &target_label);
//                append_ir2_opnd3(mips_psllh, &dest_lo, &dest_lo, &src_lo);
//                append_ir2_opnd3(mips_psllh, &dest_hi, &dest_hi, &src_lo);
//
//                ra_free_temp(&temp_imm);
//                ra_free_temp(&temp);
//            }
//        }
//    }

    return true;
}

bool translate_pslld(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if (ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            if (ir1_opnd_is_xmm(opnd1) ||
//                ir1_opnd_is_mem(opnd1))
//            {
//                IR2_OPND src = load_freg128_from_ir1(opnd1);
//                IR2_OPND temp1 = ra_alloc_ftemp();
//                IR2_OPND temp2 = ra_alloc_ftemp();
//                IR2_OPND temp3 = ra_alloc_ftemp();
//
//                append_ir2_opnd2i(mips_splatid, &temp1, &src, 0);
//                append_ir2_opnd1i(mips_ldid, &temp2, 32);
//                append_ir2_opnd3(mips_clt_ud, &temp3, &temp1, &temp2);
//                append_ir2_opnd2i(mips_splatiw, &temp1, &src, 0);
//                append_ir2_opnd3(mips_sllw, &dest, &dest, &temp1);
//                append_ir2_opnd3(mips_andv, &dest, &dest, &temp3);
//            }
//            else if (ir1_opnd_is_imm(opnd1)) {
//                uint8_t imm = ir1_opnd_uimm(opnd1);
//                if (imm > 31) {
//                    append_ir2_opnd3(mips_xorv, &dest, &dest, &dest);
//                } else {
//                    append_ir2_opnd2i(mips_slliw, &dest, &dest, imm);
//                }
//            } else {
//                lsassert(0);
//            }
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) { /* dest mmx */
//        if (ir1_opnd_is_imm(opnd1)) {
//            IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//            uint8 imm = ir1_opnd_uimm(opnd1);
//
//            if (imm > 31) {
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest);
//            } else {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                load_imm32_to_ir2(&itemp, imm, UNKNOWN_EXTENSION);
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//
//                append_ir2_opnd3(mips_psllw, &dest, &dest, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            }
//        } else {
//            IR2_OPND temp_imm = ra_alloc_itemp();
//            IR2_OPND temp = ra_alloc_itemp();
//
//            IR2_OPND target_label = ir2_opnd_new_type(IR2_OPND_LABEL);
//            IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND src = load_freg_from_ir1_1(opnd1, false, true);
//
//            load_imm32_to_ir2(&temp_imm, 0xffffff80, SIGN_EXTENSION);
//
//            append_ir2_opnd2(mips_dmfc1, &temp, &src);
//            append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//            append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//            append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest);
//
//            append_ir2_opnd1(mips_label, &target_label);
//            append_ir2_opnd3(mips_psllw, &dest, &dest, &src);
//
//            ra_free_temp(&temp_imm);
//            ra_free_temp(&temp);
//        }
//    } else {
//        if (ir1_opnd_is_imm(opnd1)) {
//            uint8 imm = ir1_opnd_uimm(opnd1);
//            IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//            if (imm > 31) {
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_lo);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_hi);
//            }
//            else {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                load_imm32_to_ir2(&itemp, imm, UNKNOWN_EXTENSION);
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//
//                append_ir2_opnd3(mips_psllw, &dest_lo, &dest_lo, &ftemp);
//
//                append_ir2_opnd3(mips_psllw, &dest_hi, &dest_hi, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            }
//        } else {
//            if (ir1_opnd_is_xmm(opnd1) &&
//                (ir1_opnd_base_reg_num(opnd0) ==
//                 ir1_opnd_base_reg_num(opnd1)))
//            {
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                IR2_OPND temp_imm = ra_alloc_itemp();
//                IR2_OPND temp = ra_alloc_itemp();
//
//                IR2_OPND target_label = ir2_opnd_new_type( IR2_OPND_LABEL);
//                IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//                IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//                load_imm32_to_ir2(&temp_imm, 0xffffff80, SIGN_EXTENSION);
//
//                append_ir2_opnd2(mips_dmfc1, &temp, &dest_lo);
//                append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//                append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_lo);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_hi);
//
//                append_ir2_opnd1(mips_label, &target_label);
//                append_ir2_opnd2(mips_mov_d, &ftemp, &dest_lo);
//                append_ir2_opnd3(mips_psllw, &dest_lo, &dest_lo, &ftemp);
//                append_ir2_opnd3(mips_psllw, &dest_hi, &dest_hi, &ftemp);
//
//                ra_free_temp(&ftemp);
//                ra_free_temp(&temp_imm);
//                ra_free_temp(&temp);
//            } else {
//                IR2_OPND temp_imm = ra_alloc_itemp();
//                IR2_OPND temp = ra_alloc_itemp();
//                IR2_OPND target_label = ir2_opnd_new_type( IR2_OPND_LABEL);
//
//                IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//                IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//                IR2_OPND src_lo = load_freg_from_ir1_1(opnd1, false, true);
//
//                load_imm32_to_ir2(&temp_imm, 0xff80, SIGN_EXTENSION);
//
//                append_ir2_opnd2(mips_dmfc1, &temp, &src_lo);
//                append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//                append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_lo);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_hi);
//
//                append_ir2_opnd1(mips_label, &target_label);
//                append_ir2_opnd3(mips_psllw, &dest_lo, &dest_lo, &src_lo);
//                append_ir2_opnd3(mips_psllw, &dest_hi, &dest_hi, &src_lo);
//
//                ra_free_temp(&temp_imm);
//                ra_free_temp(&temp);
//            }
//        }
//    }

    return true;
}

bool translate_psllq(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if (ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            if (ir1_opnd_is_xmm(opnd1) ||
//                ir1_opnd_is_mem(opnd1))
//            {
//                IR2_OPND src = load_freg128_from_ir1(opnd1);
//                IR2_OPND temp1 = ra_alloc_ftemp();
//                IR2_OPND temp2 = ra_alloc_ftemp();
//                IR2_OPND temp3 = ra_alloc_ftemp();
//
//                append_ir2_opnd2i(mips_splatid, &temp1, &src, 0);
//                append_ir2_opnd1i(mips_ldid, &temp2, 64);
//                append_ir2_opnd3(mips_clt_ud, &temp3, &temp1, &temp2);
//                append_ir2_opnd2i(mips_splatid, &temp1, &src, 0);
//                append_ir2_opnd3(mips_slld, &dest, &dest, &temp1);
//                append_ir2_opnd3(mips_andv, &dest, &dest, &temp3);
//            }
//            else if (ir1_opnd_is_imm(opnd1)) {
//                uint8_t imm = ir1_opnd_uimm(opnd1);
//                if (imm > 63) {
//                    append_ir2_opnd3(mips_xorv, &dest, &dest, &dest);
//                } else {
//                    append_ir2_opnd2i(mips_sllid, &dest, &dest, imm);
//                }
//            } else {
//                lsassert(0);
//            }
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) { /* dest mmx */
//        if (ir1_opnd_is_imm(opnd1)) {
//            IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//            uint8 imm = ir1_opnd_uimm(opnd1);
//
//            if (imm > 63) {
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest);
//            } else {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                load_imm32_to_ir2(&itemp, imm, UNKNOWN_EXTENSION);
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//
//                append_ir2_opnd3(mips_fdsll, &dest, &dest, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            }
//        } else {
//            IR2_OPND temp_imm = ra_alloc_itemp();
//            IR2_OPND temp = ra_alloc_itemp();
//
//            IR2_OPND target_label = ir2_opnd_new_type(IR2_OPND_LABEL);
//            IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND src = load_freg_from_ir1_1(opnd1, false, true);
//
//            load_imm64_to_ir2(&temp_imm, 0xffffffffffffff80ULL);
//
//            append_ir2_opnd2(mips_dmfc1, &temp, &src);
//            append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//            append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//            append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest);
//
//            append_ir2_opnd1(mips_label, &target_label);
//            append_ir2_opnd3(mips_fdsll, &dest, &dest, &src);
//
//            ra_free_temp(&temp_imm);
//            ra_free_temp(&temp);
//        }
//    } else {
//        if (ir1_opnd_is_imm(opnd1)) {
//            uint8 imm = ir1_opnd_uimm(opnd1);
//            IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//            if (imm > 63) {
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_lo);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_hi);
//            } else {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                load_imm32_to_ir2(&itemp, imm, UNKNOWN_EXTENSION);
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//
//                append_ir2_opnd3(mips_fdsll, &dest_lo, &dest_lo, &ftemp);
//
//                append_ir2_opnd3(mips_fdsll, &dest_hi, &dest_hi, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            }
//        } else {
//            if (ir1_opnd_is_xmm(opnd1) &&
//                (ir1_opnd_base_reg_num(opnd0) ==
//                 ir1_opnd_base_reg_num(opnd1)))
//            {
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                IR2_OPND temp_imm = ra_alloc_itemp();
//                IR2_OPND temp = ra_alloc_itemp();
//
//                IR2_OPND target_label = ir2_opnd_new_type( IR2_OPND_LABEL);
//                IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//                IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//                load_imm64_to_ir2(&temp_imm, 0xffffffffffffff80ULL);
//
//                append_ir2_opnd2(mips_dmfc1, &temp, &dest_lo);
//                append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//                append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_lo);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_hi);
//
//                append_ir2_opnd1(mips_label, &target_label);
//                append_ir2_opnd2(mips_mov_d, &ftemp, &dest_lo);
//                append_ir2_opnd3(mips_fdsll, &dest_lo, &dest_lo, &ftemp);
//                append_ir2_opnd3(mips_fdsll, &dest_hi, &dest_hi, &ftemp);
//
//                ra_free_temp(&ftemp);
//                ra_free_temp(&temp_imm);
//                ra_free_temp(&temp);
//            } else {
//                IR2_OPND temp_imm = ra_alloc_itemp();
//                IR2_OPND temp = ra_alloc_itemp();
//
//                IR2_OPND target_label = ir2_opnd_new_type( IR2_OPND_LABEL);
//                IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//                IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//                IR2_OPND src_lo = load_freg_from_ir1_1(opnd1, false, true);
//
//                load_imm64_to_ir2(&temp_imm, 0xffffffffffffff80ULL);
//
//                append_ir2_opnd2(mips_dmfc1, &temp, &src_lo);
//                append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//                append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_lo);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_hi);
//
//                append_ir2_opnd1(mips_label, &target_label);
//                append_ir2_opnd3(mips_fdsll, &dest_lo, &dest_lo, &src_lo);
//                append_ir2_opnd3(mips_fdsll, &dest_hi, &dest_hi, &src_lo);
//
//                ra_free_temp(&temp_imm);
//                ra_free_temp(&temp);
//            }
//        }
//    }

    return true;
}

bool translate_psrlw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if (ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            if (ir1_opnd_is_xmm(opnd1) ||
//                ir1_opnd_is_mem(opnd1))
//            {
//                IR2_OPND src = load_freg128_from_ir1(opnd1);
//                IR2_OPND temp1 = ra_alloc_ftemp();
//                IR2_OPND temp2 = ra_alloc_ftemp();
//                IR2_OPND temp3 = ra_alloc_ftemp();
//
//                append_ir2_opnd2i(mips_splatid, &temp1, &src, 0);
//                append_ir2_opnd1i(mips_ldid, &temp2, 16);
//                append_ir2_opnd3(mips_clt_ud, &temp3, &temp1, &temp2);
//                append_ir2_opnd2i(mips_splatih, &temp1, &src, 0);
//                append_ir2_opnd3(mips_srlh, &dest, &dest, &temp1);
//                append_ir2_opnd3(mips_andv, &dest, &dest, &temp3);
//            }
//            else if (ir1_opnd_is_imm(opnd1)) {
//                uint8_t imm = ir1_opnd_uimm(opnd1);
//                if (imm > 15) {
//                    append_ir2_opnd3(mips_xorv, &dest, &dest, &dest);
//                } else {
//                    append_ir2_opnd2i(mips_srlih, &dest, &dest, imm);
//                }
//            } else {
//                lsassert(0);
//            }
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) { /* dest mmx */
//        if (ir1_opnd_is_imm(opnd1)) {
//            IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//            uint8 imm = ir1_opnd_uimm(opnd1);
//
//            if (imm > 15) {
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest);
//            } else {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                load_imm32_to_ir2(&itemp, imm, UNKNOWN_EXTENSION);
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//
//                append_ir2_opnd3(mips_psrlh, &dest, &dest, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            }
//        } else {
//            IR2_OPND temp_imm = ra_alloc_itemp();
//            IR2_OPND temp = ra_alloc_itemp();
//
//            IR2_OPND target_label = ir2_opnd_new_type(IR2_OPND_LABEL);
//            IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND src = load_freg_from_ir1_1(opnd1, false, true);
//
//            load_imm32_to_ir2(&temp_imm, 0xff80, SIGN_EXTENSION);
//
//            append_ir2_opnd2(mips_dmfc1, &temp, &src);
//            append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//            append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//            append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest);
//
//            append_ir2_opnd1(mips_label, &target_label);
//            append_ir2_opnd3(mips_psrlh, &dest, &dest, &src);
//
//            ra_free_temp(&temp_imm);
//            ra_free_temp(&temp);
//        }
//    } else { /* dest xmm */
//        if (ir1_opnd_is_imm(opnd1)) {
//            uint8 imm = ir1_opnd_uimm(opnd1);
//            IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//            if (imm > 15) {
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_lo);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_hi);
//            } else {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                load_imm32_to_ir2(&itemp, imm, UNKNOWN_EXTENSION);
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//
//                append_ir2_opnd3(mips_psrlh, &dest_lo, &dest_lo, &ftemp);
//
//                append_ir2_opnd3(mips_psrlh, &dest_hi, &dest_hi, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            }
//        } else { /* src xmm */
//            if (ir1_opnd_is_xmm(opnd1) &&
//                (ir1_opnd_base_reg_num(opnd0) ==
//                 ir1_opnd_base_reg_num(opnd1)))
//            {
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                IR2_OPND temp_imm = ra_alloc_itemp();
//                IR2_OPND temp = ra_alloc_itemp();
//
//                IR2_OPND target_label = ir2_opnd_new_type( IR2_OPND_LABEL);
//                IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//                IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//                load_imm32_to_ir2(&temp_imm, 0xff80, SIGN_EXTENSION);
//
//                append_ir2_opnd2(mips_dmfc1, &temp, &dest_lo);
//                append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//                append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_lo);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_hi);
//
//                append_ir2_opnd1(mips_label, &target_label);
//                append_ir2_opnd2(mips_mov_d, &ftemp, &dest_lo);
//                append_ir2_opnd3(mips_psrlh, &dest_lo, &dest_lo, &ftemp);
//                append_ir2_opnd3(mips_psrlh, &dest_hi, &dest_hi, &ftemp);
//
//                ra_free_temp(&ftemp);
//                ra_free_temp(&temp_imm);
//                ra_free_temp(&temp);
//            } else {
//                IR2_OPND temp_imm = ra_alloc_itemp();
//                IR2_OPND temp = ra_alloc_itemp();
//                IR2_OPND target_label = ir2_opnd_new_type( IR2_OPND_LABEL);
//
//                IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//                IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//                IR2_OPND src_lo = load_freg_from_ir1_1(opnd1, false, true);
//
//                load_imm32_to_ir2(&temp_imm, 0xff80, SIGN_EXTENSION);
//
//                append_ir2_opnd2(mips_dmfc1, &temp, &src_lo);
//                append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//                append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_lo);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_hi);
//
//                append_ir2_opnd1(mips_label, &target_label);
//                append_ir2_opnd3(mips_psrlh, &dest_lo, &dest_lo, &src_lo);
//                append_ir2_opnd3(mips_psrlh, &dest_hi, &dest_hi, &src_lo);
//
//                ra_free_temp(&temp_imm);
//                ra_free_temp(&temp);
//            }
//        }
//    }

    return true;
}

bool translate_psrld(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if (ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            if (ir1_opnd_is_xmm(opnd1) ||
//                ir1_opnd_is_mem(opnd1))
//            {
//                IR2_OPND src = load_freg128_from_ir1(opnd1);
//                IR2_OPND temp1 = ra_alloc_ftemp();
//                IR2_OPND temp2 = ra_alloc_ftemp();
//                IR2_OPND temp3 = ra_alloc_ftemp();
//
//                append_ir2_opnd2i(mips_splatid, &temp1, &src, 0);
//                append_ir2_opnd1i(mips_ldid, &temp2, 32);
//                append_ir2_opnd3(mips_clt_ud, &temp3, &temp1, &temp2);
//                append_ir2_opnd2i(mips_splatiw, &temp1, &src, 0);
//                append_ir2_opnd3(mips_srlw, &dest, &dest, &temp1);
//                append_ir2_opnd3(mips_andv, &dest, &dest, &temp3);
//            }
//            else if (ir1_opnd_is_imm(opnd1)) {
//                uint8_t imm = ir1_opnd_uimm(opnd1);
//                if (imm > 31) {
//                    append_ir2_opnd3(mips_xorv, &dest, &dest, &dest);
//                } else {
//                    append_ir2_opnd2i(mips_srliw, &dest, &dest, imm);
//                }
//            } else {
//                lsassert(0);
//            }
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) { /* dest mmx */
//        if (ir1_opnd_is_imm(opnd1)) {
//            IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//            uint8 imm = ir1_opnd_uimm(opnd1);
//
//            if (imm > 31) {
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest);
//            } else {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                load_imm32_to_ir2(&itemp, imm, UNKNOWN_EXTENSION);
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//
//                append_ir2_opnd3(mips_psrlw, &dest, &dest, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            }
//        } else {
//            IR2_OPND temp_imm = ra_alloc_itemp();
//            IR2_OPND temp = ra_alloc_itemp();
//            IR2_OPND target_label = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//            IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND src = load_freg_from_ir1_1(opnd1, false, true);
//
//            load_imm32_to_ir2(&temp_imm, 0xffffff80, SIGN_EXTENSION);
//
//            append_ir2_opnd2(mips_dmfc1, &temp, &src);
//            append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//            append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//            append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest);
//
//            append_ir2_opnd1(mips_label, &target_label);
//            append_ir2_opnd3(mips_psrlw, &dest, &dest, &src);
//
//            ra_free_temp(&temp_imm);
//            ra_free_temp(&temp);
//        }
//    } else {
//        if (ir1_opnd_is_imm(opnd1)) {
//            uint8 imm = ir1_opnd_uimm(opnd1);
//            IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//            if (imm > 31) {
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_lo);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_hi);
//            }
//            else {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                load_imm32_to_ir2(&itemp, imm, UNKNOWN_EXTENSION);
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//
//                append_ir2_opnd3(mips_psrlw, &dest_lo, &dest_lo, &ftemp);
//
//                append_ir2_opnd3(mips_psrlw, &dest_hi, &dest_hi, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            }
//        } else {
//            if (ir1_opnd_is_xmm(opnd1) &&
//                (ir1_opnd_base_reg_num(opnd0) ==
//                 ir1_opnd_base_reg_num(opnd1)))
//            {
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                IR2_OPND temp_imm = ra_alloc_itemp();
//                IR2_OPND temp = ra_alloc_itemp();
//
//                IR2_OPND target_label = ir2_opnd_new_type( IR2_OPND_LABEL);
//                IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//                IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//                load_imm32_to_ir2(&temp_imm, 0xffffff80, SIGN_EXTENSION);
//
//                append_ir2_opnd2(mips_dmfc1, &temp, &dest_lo);
//                append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//                append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_lo);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_hi);
//
//                append_ir2_opnd1(mips_label, &target_label);
//                append_ir2_opnd2(mips_mov_d, &ftemp, &dest_lo);
//                append_ir2_opnd3(mips_psrlw, &dest_lo, &dest_lo, &ftemp);
//                append_ir2_opnd3(mips_psrlw, &dest_hi, &dest_hi, &ftemp);
//
//                ra_free_temp(&ftemp);
//                ra_free_temp(&temp_imm);
//                ra_free_temp(&temp);
//            } else {
//                IR2_OPND temp_imm = ra_alloc_itemp();
//                IR2_OPND temp = ra_alloc_itemp();
//                IR2_OPND target_label = ir2_opnd_new_type( IR2_OPND_LABEL);
//
//                IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//                IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//                IR2_OPND src_lo = load_freg_from_ir1_1(opnd1, false, true);
//
//                load_imm32_to_ir2(&temp_imm, 0xff80, SIGN_EXTENSION);
//
//                append_ir2_opnd2(mips_dmfc1, &temp, &src_lo);
//                append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//                append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_lo);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_hi);
//
//                append_ir2_opnd1(mips_label, &target_label);
//                append_ir2_opnd3(mips_psrlw, &dest_lo, &dest_lo, &src_lo);
//                append_ir2_opnd3(mips_psrlw, &dest_hi, &dest_hi, &src_lo);
//
//                ra_free_temp(&temp_imm);
//                ra_free_temp(&temp);
//            }
//        }
//    }

    return true;
}

bool translate_psrlq(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if (ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            if (ir1_opnd_is_xmm(opnd1) ||
//                ir1_opnd_is_mem(opnd1))
//            {
//                IR2_OPND src = load_freg128_from_ir1(opnd1);
//                IR2_OPND temp1 = ra_alloc_ftemp();
//                IR2_OPND temp2 = ra_alloc_ftemp();
//                IR2_OPND temp3 = ra_alloc_ftemp();
//
//                append_ir2_opnd2i(mips_splatid, &temp1, &src, 0);
//                append_ir2_opnd1i(mips_ldid, &temp2, 64);
//                append_ir2_opnd3(mips_clt_ud, &temp3, &temp1, &temp2);
//                append_ir2_opnd2i(mips_splatid, &temp1, &src, 0);
//                append_ir2_opnd3(mips_srld, &dest, &dest, &temp1);
//                append_ir2_opnd3(mips_andv, &dest, &dest, &temp3);
//            }
//            else if (ir1_opnd_is_imm(opnd1)) {
//                uint8_t imm = ir1_opnd_uimm(opnd1);
//                if (imm > 63) {
//                    append_ir2_opnd3(mips_xorv, &dest, &dest, &dest);
//                } else {
//                    append_ir2_opnd2i(mips_srlid, &dest, &dest, imm);
//                }
//            } else {
//                lsassert(0);
//            }
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) { /* dest mmx */
//        if (ir1_opnd_is_imm(opnd1)) {
//            IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//            uint8 imm = ir1_opnd_uimm(opnd1);
//
//            if (imm > 63) {
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest);
//            } else {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                load_imm32_to_ir2(&itemp, imm, UNKNOWN_EXTENSION);
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//
//                append_ir2_opnd3(mips_fdsrl, &dest, &dest, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            }
//        } else {
//            IR2_OPND temp_imm = ra_alloc_itemp();
//            IR2_OPND temp = ra_alloc_itemp();
//
//            IR2_OPND target_label = ir2_opnd_new_type(IR2_OPND_LABEL);
//            IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND src = load_freg_from_ir1_1(opnd1, false, true);
//
//            load_imm64_to_ir2(&temp_imm, 0xffffffffffffff80ULL);
//
//            append_ir2_opnd2(mips_dmfc1, &temp, &src);
//            append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//            append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//            append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest);
//
//            append_ir2_opnd1(mips_label, &target_label);
//            append_ir2_opnd3(mips_fdsrl, &dest, &dest, &src);
//
//            ra_free_temp(&temp_imm);
//            ra_free_temp(&temp);
//        }
//    } else {
//        if (ir1_opnd_is_imm(opnd1)) {
//            uint8 imm = ir1_opnd_uimm(opnd1);
//            IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//            if (imm > 63) {
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_lo);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_hi);
//            }
//            else {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                load_imm32_to_ir2(&itemp, imm, UNKNOWN_EXTENSION);
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//
//                append_ir2_opnd3(mips_fdsrl, &dest_lo, &dest_lo, &ftemp);
//
//                append_ir2_opnd3(mips_fdsrl, &dest_hi, &dest_hi, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            }
//        } else {
//            if (ir1_opnd_is_xmm(opnd1) &&
//                (ir1_opnd_base_reg_num(opnd0) ==
//                 ir1_opnd_base_reg_num(opnd1)))
//            {
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                IR2_OPND temp_imm = ra_alloc_itemp();
//                IR2_OPND temp = ra_alloc_itemp();
//
//                IR2_OPND target_label = ir2_opnd_new_type( IR2_OPND_LABEL);
//                IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//                IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//                load_imm64_to_ir2(&temp_imm, 0xffffffffffffff80ULL);
//
//                append_ir2_opnd2(mips_dmfc1, &temp, &dest_lo);
//                append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//                append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_lo);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_hi);
//
//                append_ir2_opnd1(mips_label, &target_label);
//                append_ir2_opnd2(mips_mov_d, &ftemp, &dest_lo);
//                append_ir2_opnd3(mips_fdsrl, &dest_lo, &dest_lo, &ftemp);
//                append_ir2_opnd3(mips_fdsrl, &dest_hi, &dest_hi, &ftemp);
//
//                ra_free_temp(&ftemp);
//                ra_free_temp(&temp_imm);
//                ra_free_temp(&temp);
//            } else {
//                IR2_OPND temp_imm = ra_alloc_itemp();
//                IR2_OPND temp = ra_alloc_itemp();
//                IR2_OPND target_label = ir2_opnd_new_type( IR2_OPND_LABEL);
//
//                IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//                IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//                IR2_OPND src_lo = load_freg_from_ir1_1(opnd1, false, true);
//
//                load_imm64_to_ir2(&temp_imm, 0xffffffffffffff80ULL);
//
//                append_ir2_opnd2(mips_dmfc1, &temp, &src_lo);
//                append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//                append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_lo);
//                append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dest_hi);
//
//                append_ir2_opnd1(mips_label, &target_label);
//                append_ir2_opnd3(mips_fdsrl, &dest_lo, &dest_lo, &src_lo);
//                append_ir2_opnd3(mips_fdsrl, &dest_hi, &dest_hi, &src_lo);
//
//                ra_free_temp(&temp_imm);
//                ra_free_temp(&temp);
//            }
//        }
//    }

    return true;
}

bool translate_psraw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if (ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            if (ir1_opnd_is_xmm(opnd1) ||
//                ir1_opnd_is_mem(opnd1))
//            {
//                IR2_OPND src = load_freg128_from_ir1(opnd1);
//                IR2_OPND temp1 = ra_alloc_ftemp();
//                IR2_OPND temp2 = ra_alloc_ftemp();
//                IR2_OPND temp3 = ra_alloc_ftemp();
//                IR2_OPND temp4 = ra_alloc_ftemp();
//
//                append_ir2_opnd2i(mips_splatid, &temp1, &src, 0);
//                append_ir2_opnd1i(mips_ldid, &temp2, 16);
//                append_ir2_opnd3(mips_clt_ud, &temp3, &temp1, &temp2);
//                append_ir2_opnd2i(mips_splatih, &temp1, &src, 0);
//                append_ir2_opnd2(mips_vsignfillh, &temp4, &dest);
//                append_ir2_opnd3(mips_srah, &dest, &dest, &temp1);
//                append_ir2_opnd3(mips_bmzv, &dest, &temp4, &temp3);
//            }
//            else if (ir1_opnd_is_imm(opnd1)) {
//                uint8_t imm = ir1_opnd_uimm(opnd1);
//                if (imm > 15) {
//                    append_ir2_opnd2(mips_vsignfillh, &dest, &dest);
//                } else {
//                    append_ir2_opnd2i(mips_sraih, &dest, &dest, imm);
//                }
//            } else {
//                lsassert(0);
//            }
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) { /* dest mmx */
//        if (ir1_opnd_is_imm(opnd1)) {
//            IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//            uint8 imm = ir1_opnd_uimm(opnd1);
//
//            if (imm > 15) {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                load_imm32_to_ir2(&itemp, 0x10, UNKNOWN_EXTENSION);
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//
//                append_ir2_opnd3(mips_psrah, &dest, &dest, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            } else {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                load_imm32_to_ir2(&itemp, imm, UNKNOWN_EXTENSION);
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//
//                append_ir2_opnd3(mips_psrah, &dest, &dest, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            }
//        } else {
//            IR2_OPND temp_imm = ra_alloc_itemp();
//            IR2_OPND temp = ra_alloc_itemp();
//            IR2_OPND ftemp = ra_alloc_ftemp();
//            IR2_OPND target_label = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//            IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND src = load_freg_from_ir1_1(opnd1, false, true);
//
//            load_imm32_to_ir2(&temp_imm, 0xff80, SIGN_EXTENSION);
//
//            append_ir2_opnd2(mips_dmfc1, &temp, &src);
//            append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//            append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//
//            load_imm32_to_ir2(&temp_imm, 0x10, UNKNOWN_EXTENSION);
//
//            append_ir2_opnd2(mips_dmtc1, &temp_imm, &ftemp);
//            append_ir2_opnd3(mips_psrah, &dest, &dest, &ftemp);
//
//            append_ir2_opnd1(mips_label, &target_label);
//            append_ir2_opnd3(mips_psrah, &dest, &dest, &src);
//
//            ra_free_temp(&temp_imm);
//            ra_free_temp(&temp);
//            ra_free_temp(&ftemp);
//        }
//    } else { /* dest xmm */
//        if (ir1_opnd_is_imm(opnd1)) {
//            uint8 imm = ir1_opnd_uimm(opnd1);
//            IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//            if (imm > 15) {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                load_imm32_to_ir2(&itemp, 0x10, UNKNOWN_EXTENSION);
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//
//                append_ir2_opnd3(mips_psrah, &dest_lo, &dest_lo, &ftemp);
//
//                append_ir2_opnd3(mips_psrah, &dest_hi, &dest_hi, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            } else {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                load_imm32_to_ir2(&itemp, imm, UNKNOWN_EXTENSION);
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//
//                append_ir2_opnd3(mips_psrah, &dest_lo, &dest_lo, &ftemp);
//
//                append_ir2_opnd3(mips_psrah, &dest_hi, &dest_hi, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            }
//        } else { /* src xmm */
//            if (ir1_opnd_is_xmm(opnd1) &&
//                (ir1_opnd_base_reg_num(opnd0) ==
//                 ir1_opnd_base_reg_num(opnd1)))
//            {
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                IR2_OPND temp_imm = ra_alloc_itemp();
//                IR2_OPND temp = ra_alloc_itemp();
//                IR2_OPND target_label = ir2_opnd_new_type( IR2_OPND_LABEL);
//
//                IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//                IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//                load_imm32_to_ir2(&temp_imm, 0xff80, SIGN_EXTENSION);
//
//                append_ir2_opnd2(mips_dmfc1, &temp, &dest_lo);
//                append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//                append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//
//                load_imm32_to_ir2(&temp_imm, 0x10, UNKNOWN_EXTENSION);
//                append_ir2_opnd2(mips_dmtc1, &temp_imm, &ftemp);
//                append_ir2_opnd3(mips_psrah, &dest_lo, &dest_lo, &ftemp);
//                append_ir2_opnd3(mips_psrah, &dest_hi, &dest_hi, &ftemp);
//
//                append_ir2_opnd1(mips_label, &target_label);
//                append_ir2_opnd2(mips_mov_d, &ftemp, &dest_lo);
//                append_ir2_opnd3(mips_psrah, &dest_lo, &dest_lo, &ftemp);
//                append_ir2_opnd3(mips_psrah, &dest_hi, &dest_hi, &ftemp);
//
//                ra_free_temp(&ftemp);
//                ra_free_temp(&temp_imm);
//                ra_free_temp(&temp);
//            } else {
//                IR2_OPND temp_imm = ra_alloc_itemp();
//                IR2_OPND temp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                IR2_OPND target_label = ir2_opnd_new_type( IR2_OPND_LABEL);
//
//                IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//                IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//                IR2_OPND src_lo = load_freg_from_ir1_1(opnd1, false, true);
//
//                load_imm32_to_ir2(&temp_imm, 0xff80, SIGN_EXTENSION);
//
//                append_ir2_opnd2(mips_dmfc1, &temp, &src_lo);
//                append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//                append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//
//                load_imm32_to_ir2(&temp_imm, 0x10, UNKNOWN_EXTENSION);
//
//                append_ir2_opnd2(mips_dmtc1, &temp_imm, &ftemp);
//                append_ir2_opnd3(mips_psrah, &dest_lo, &dest_lo, &ftemp);
//                append_ir2_opnd3(mips_psrah, &dest_hi, &dest_hi, &ftemp);
//
//                append_ir2_opnd1(mips_label, &target_label);
//                append_ir2_opnd3(mips_psrah, &dest_lo, &dest_lo, &src_lo);
//                append_ir2_opnd3(mips_psrah, &dest_hi, &dest_hi, &src_lo);
//
//                ra_free_temp(&temp_imm);
//                ra_free_temp(&temp);
//                ra_free_temp(&ftemp);
//            }
//        }
//    }
//
    return true;
}

bool translate_psrad(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if (ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            if (ir1_opnd_is_xmm(opnd1) ||
//                ir1_opnd_is_mem(opnd1))
//            {
//                IR2_OPND src = load_freg128_from_ir1(opnd1);
//                IR2_OPND temp1 = ra_alloc_ftemp();
//                IR2_OPND temp2 = ra_alloc_ftemp();
//                IR2_OPND temp3 = ra_alloc_ftemp();
//                IR2_OPND temp4 = ra_alloc_ftemp();
//
//                append_ir2_opnd2i(mips_splatid, &temp1, &src, 0);
//                append_ir2_opnd1i(mips_ldid, &temp2, 32);
//                append_ir2_opnd3(mips_clt_ud, &temp3, &temp1, &temp2);
//                append_ir2_opnd2i(mips_splatih, &temp1, &src, 0);
//                append_ir2_opnd2(mips_vsignfillw, &temp4, &dest);
//                append_ir2_opnd3(mips_sraw, &dest, &dest, &temp1);
//                append_ir2_opnd3(mips_bmzv, &dest, &temp4, &temp3);
//            }
//            else if (ir1_opnd_is_imm(opnd1)) {
//                uint8_t imm = ir1_opnd_uimm(opnd1);
//                if (imm > 31) {
//                    append_ir2_opnd2(mips_vsignfillw, &dest, &dest);
//                } else {
//                    append_ir2_opnd2i(mips_sraiw, &dest, &dest, imm);
//                }
//            } else {
//                lsassert(0);
//            }
//            return true;
//        }
//    }
//    if (ir1_opnd_is_mmx(opnd0)) { /* dest mmx */
//        if (ir1_opnd_is_imm(opnd1)) {
//            IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//            uint8 imm = ir1_opnd_uimm(opnd1);
//
//            if (imm > 31) {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                load_imm32_to_ir2(&itemp, 0x20, UNKNOWN_EXTENSION);
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//
//                append_ir2_opnd3(mips_psraw, &dest, &dest, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            }
//            else {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                load_imm32_to_ir2(&itemp, imm, UNKNOWN_EXTENSION);
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//
//                append_ir2_opnd3(mips_psraw, &dest, &dest, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            }
//        } else {
//            IR2_OPND temp_imm = ra_alloc_itemp();
//            IR2_OPND temp = ra_alloc_itemp();
//            IR2_OPND ftemp = ra_alloc_ftemp();
//            IR2_OPND target_label = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//            IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND src = load_freg_from_ir1_1(opnd1, false, true);
//
//            load_imm32_to_ir2(&temp_imm, 0xffffff80, SIGN_EXTENSION);
//
//            append_ir2_opnd2(mips_dmfc1, &temp, &src);
//            append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//            append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//
//            load_imm32_to_ir2(&temp_imm, 0x20, UNKNOWN_EXTENSION);
//            append_ir2_opnd2(mips_dmtc1, &temp_imm, &ftemp);
//            append_ir2_opnd3(mips_psraw, &dest, &dest, &ftemp);
//
//            append_ir2_opnd1(mips_label, &target_label);
//            append_ir2_opnd3(mips_psraw, &dest, &dest, &src);
//
//            ra_free_temp(&temp_imm);
//            ra_free_temp(&temp);
//            ra_free_temp(&ftemp);
//        }
//    } else { /* dest xmm */
//        if (ir1_opnd_is_imm(opnd1)) {
//            uint8 imm = ir1_opnd_uimm(opnd1);
//            IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//            IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//            if (imm > 31) {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//
//                load_imm32_to_ir2(&itemp, 0x20, UNKNOWN_EXTENSION);
//
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//                append_ir2_opnd3(mips_psraw, &dest_lo, &dest_lo, &ftemp);
//                append_ir2_opnd3(mips_psraw, &dest_hi, &dest_hi, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            } else {
//                IR2_OPND itemp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//
//                load_imm32_to_ir2(&itemp, imm, UNKNOWN_EXTENSION);
//
//                append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//                append_ir2_opnd3(mips_psraw, &dest_lo, &dest_lo, &ftemp);
//                append_ir2_opnd3(mips_psraw, &dest_hi, &dest_hi, &ftemp);
//
//                ra_free_temp(&itemp);
//                ra_free_temp(&ftemp);
//            }
//        } else { /* src xmm */
//            if (ir1_opnd_is_xmm(opnd1) &&
//                (ir1_opnd_base_reg_num(opnd0) ==
//                 ir1_opnd_base_reg_num(opnd1)))
//            {
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                IR2_OPND temp_imm = ra_alloc_itemp();
//                IR2_OPND temp = ra_alloc_itemp();
//                IR2_OPND target_label = ir2_opnd_new_type( IR2_OPND_LABEL);
//
//                IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//                IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//                load_imm32_to_ir2(&temp_imm, 0xffffff80, SIGN_EXTENSION);
//
//                append_ir2_opnd2(mips_dmfc1, &temp, &dest_lo);
//                append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//                append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//
//                load_imm32_to_ir2(&temp_imm, 0x20, UNKNOWN_EXTENSION);
//
//                append_ir2_opnd2(mips_dmtc1, &temp_imm, &ftemp);
//                append_ir2_opnd3(mips_psraw, &dest_lo, &dest_lo, &ftemp);
//                append_ir2_opnd3(mips_psraw, &dest_hi, &dest_hi, &ftemp);
//
//                append_ir2_opnd1(mips_label, &target_label);
//                append_ir2_opnd2(mips_mov_d, &ftemp, &dest_lo);
//                append_ir2_opnd3(mips_psraw, &dest_lo, &dest_lo, &ftemp);
//                append_ir2_opnd3(mips_psraw, &dest_hi, &dest_hi, &ftemp);
//
//                ra_free_temp(&ftemp);
//                ra_free_temp(&temp_imm);
//                ra_free_temp(&temp);
//            } else {
//                IR2_OPND temp_imm = ra_alloc_itemp();
//                IR2_OPND temp = ra_alloc_itemp();
//                IR2_OPND ftemp = ra_alloc_ftemp();
//                IR2_OPND target_label = ir2_opnd_new_type( IR2_OPND_LABEL);
//
//                IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//                IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//                IR2_OPND src_lo = load_freg_from_ir1_1(opnd1, false, true);
//
//                load_imm32_to_ir2(&temp_imm, 0xffffff80, SIGN_EXTENSION);
//
//                append_ir2_opnd2(mips_dmfc1, &temp, &src_lo);
//                append_ir2_opnd3(mips_and, &temp_imm, &temp_imm, &temp);
//                append_ir2_opnd3(mips_beq, &temp_imm, &zero_ir2_opnd, &target_label);
//
//                load_imm32_to_ir2(&temp_imm, 0x20, UNKNOWN_EXTENSION);
//
//                append_ir2_opnd2(mips_dmtc1, &temp_imm, &ftemp);
//                append_ir2_opnd3(mips_psraw, &dest_lo, &dest_lo, &ftemp);
//                append_ir2_opnd3(mips_psraw, &dest_hi, &dest_hi, &ftemp);
//
//                append_ir2_opnd1(mips_label, &target_label);
//                append_ir2_opnd3(mips_psraw, &dest_lo, &dest_lo, &src_lo);
//                append_ir2_opnd3(mips_psraw, &dest_hi, &dest_hi, &src_lo);
//
//                ra_free_temp(&temp_imm);
//                ra_free_temp(&temp);
//                ra_free_temp(&ftemp);
//            }
//        }
//    }
//
    return true;
}

bool translate_psubb(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_subvb, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) {
//        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, true);
//        append_ir2_opnd3(mips_psubb, &dest, &dest, &src);
//    }
//    else {
//        IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//        append_ir2_opnd3(mips_psubb, &dest_lo, &dest_lo, &src_lo);
//
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_psubb, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_psubw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_subvh, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) {
//        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, true);
//        append_ir2_opnd3(mips_psubh, &dest, &dest, &src);
//    }
//    else {
//        IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//        append_ir2_opnd3(mips_psubh, &dest_lo, &dest_lo, &src_lo);
//
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        append_ir2_opnd3(mips_psubh, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_psubd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_subvw, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) {
//        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, true);
//        append_ir2_opnd3(mips_psubw, &dest, &dest, &src);
//    }
//    else {
//        IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//        append_ir2_opnd3(mips_psubw, &dest_lo, &dest_lo, &src_lo);
//
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        append_ir2_opnd3(mips_psubw, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_psubsb(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_subs_sb, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) {
//        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, true);
//        append_ir2_opnd3(mips_psubsb, &dest, &dest, &src);
//    }
//    else {
//        IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//        append_ir2_opnd3(mips_psubsb, &dest_lo, &dest_lo, &src_lo);
//
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        append_ir2_opnd3(mips_psubsb, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_psubsw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_subs_sh, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) {
//        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, true);
//        append_ir2_opnd3(mips_psubsh, &dest, &dest, &src);
//    }
//    else {
//        IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//        append_ir2_opnd3(mips_psubsh, &dest_lo, &dest_lo, &src_lo);
//
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        append_ir2_opnd3(mips_psubsh, &dest_hi, &dest_hi, &src_hi);
//    }
//
    return true;
}

bool translate_psubusb(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_subs_ub, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) {
//        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, true);
//        append_ir2_opnd3(mips_psubusb, &dest, &dest, &src);
//    }
//    else {
//        IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//        append_ir2_opnd3(mips_psubusb, &dest_lo, &dest_lo, &src_lo);
//
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        append_ir2_opnd3(mips_psubusb, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_psubusw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_subs_uh, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) {
//        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, true);
//        append_ir2_opnd3(mips_psubush, &dest, &dest, &src);
//    }
//    else {
//        IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//        append_ir2_opnd3(mips_psubush, &dest_lo, &dest_lo, &src_lo);
//
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        append_ir2_opnd3(mips_psubush, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_punpckhbw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_ilvlb, &dest, &src, &dest);
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) {
//        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, true);
//        append_ir2_opnd3(mips_punpckhbh, &dest, &dest, &src);
//    }
//    else {
//        IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        append_ir2_opnd3(mips_punpcklbh, &dest_lo, &dest_hi, &src_hi);
//        append_ir2_opnd3(mips_punpckhbh, &dest_hi, &dest_hi, &src_hi);
//    }
//
    return true;
}

bool translate_punpckhwd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_ilvlh, &dest, &src, &dest);
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) {
//        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, true);
//        append_ir2_opnd3(mips_punpckhhw, &dest, &dest, &src);
//    }
//    else {
//        IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        append_ir2_opnd3(mips_punpcklhw, &dest_lo, &dest_hi, &src_hi);
//        append_ir2_opnd3(mips_punpckhhw, &dest_hi, &dest_hi, &src_hi);
//    }
//
    return true;
}

bool translate_punpckhdq(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_ilvlw, &dest, &src, &dest);
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) {
//        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, true);
//        append_ir2_opnd3(mips_punpckhwd, &dest, &dest, &src);
//    }
//    else {
//        IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd0 + 1, true, true);
//
//        append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_hi, &src_hi);
//        append_ir2_opnd3(mips_punpckhwd, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_punpcklbw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if(ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = load_freg128_from_ir1(opnd0);
        IR2_OPND src  = load_freg128_from_ir1(opnd1);

        append_ir2_opnd3(LISA_VILVL_B, &dest, &src, &dest);
        return true;
    }

//    if (ir1_opnd_is_mmx(opnd0)) {
        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, false);
        append_ir2_opnd3(LISA_VILVL_B, &dest, &src, &dest);
//    }
//    else {
//        IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//        append_ir2_opnd3(mips_punpckhbh, &dest_hi, &dest_lo, &src_lo);
//        append_ir2_opnd3(mips_punpcklbh, &dest_lo, &dest_lo, &src_lo);
//    }

    return true;
}

bool translate_punpcklwd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//            append_ir2_opnd3(mips_ilvrh, &dest, &src, &dest);
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) {
//        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, false);
//        append_ir2_opnd3(mips_punpcklhw, &dest, &dest, &src);
//    }
//    else {
//        IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//        append_ir2_opnd3(mips_punpckhhw, &dest_hi, &dest_lo, &src_lo);
//        append_ir2_opnd3(mips_punpcklhw, &dest_lo, &dest_lo, &src_lo);
//    }

    return true;
}

bool translate_punpckldq(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)) {
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//            append_ir2_opnd3(mips_ilvrw, &dest, &src, &dest);
//            return true;
//        }
//    }
//
//    if (ir1_opnd_is_mmx(opnd0)) {
//        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, false);
//        append_ir2_opnd3(mips_punpcklwd, &dest, &dest, &src);
//    }
//    else {
//        IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//        append_ir2_opnd3(mips_punpckhwd, &dest_hi, &dest_lo, &src_lo);
//        append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &src_lo);
//    }

    return true;
}

bool translate_addps(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_faddw, &dest, &dest, &src);
//        return true;
//    }
//
//    /* TODO */
//    if (1) {
//        /* if(OPTIONS::use_ps_inst()){ */
//        IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        append_ir2_opnd3(mips_add_ps, &dest_lo, &dest_lo, &src_lo);
//        append_ir2_opnd3(mips_add_ps, &dest_hi, &dest_hi, &src_hi);
//
//    }
//    else {
//        translate_xxxps_helper(pir1, false, mips_add_s);
//        translate_xxxps_helper(pir1, true, mips_add_s);
//    }

    return true;
}

bool translate_addsd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_add_d, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insved, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_add_d, &dest_lo, &dest_lo, &src_lo);

    return true;
}

bool translate_addss(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_add_s, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, false);
//
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_add_s, &dest_lo, &dest_lo, &src_lo);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    ra_free_temp(&temp_dest);
    return true;
}

bool translate_andnpd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_vandnv, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    append_ir2_opnd3(mips_pandn, &dest_lo, &dest_lo, &src_lo);
//    append_ir2_opnd3(mips_pandn, &dest_hi, &dest_hi, &src_hi);

    return true;
}

bool translate_andnps(IR1_INST *pir1)
{
    translate_andnpd(pir1);
    return true;
}

bool translate_andps(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_andv, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    append_ir2_opnd3(mips_fand, &dest_lo, &dest_lo, &src_lo);
//    append_ir2_opnd3(mips_fand, &dest_hi, &dest_hi, &src_hi);

    return true;
}

bool translate_cmpeqpd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fceqd, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_eq_d, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1t, &label_true);
//    /* the low 64 bits of dest and src are not equal */
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//    append_ir2_opnd1(mips_b, &label_high);
//    /* the low 64 bits of dest and src are equal */
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* high 64 bits */
//    append_ir2_opnd1(mips_label, &label_high);
//
//    IR2_OPND label_true2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit  = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_eq_d, &dest_hi, &src_hi);
//
//    append_ir2_opnd1(mips_bc1t, &label_true2);
//    /* the high 64 bits of dest and src are not equal */
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    append_ir2_opnd1(mips_b, &label_exit);
//    /* the high 64 bits of dest and src are equal */
//    append_ir2_opnd1(mips_label, &label_true2);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, true);
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&temp_int);
    return true;
}

bool translate_cmpltpd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fcltd, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_lt_d, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1t, &label_true);
//    /* the low 64 bits of dest is not less than that of src */
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    append_ir2_opnd1(mips_b, &label_high);
//    /* the low 64 bits of dest is less than that of src */
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//
//    /* high 64 bits */
//    append_ir2_opnd1(mips_label, &label_high);
//    IR2_OPND label_true2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit  = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_lt_d, &dest_hi, &src_hi);
//
//    append_ir2_opnd1(mips_bc1t, &label_true2);
//    /* the high 64 bits of dest is not less than that of src */
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    append_ir2_opnd1(mips_b, &label_exit);
//    /* the high 64 bits of dest is less than that of src */
//    append_ir2_opnd1(mips_label, &label_true2);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, true);
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&temp_int);
    return true;
}

bool translate_cmplepd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fcled, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_le_d, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1t, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    append_ir2_opnd1(mips_b, &label_high);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* high 64 bits */
//    append_ir2_opnd1(mips_label, &label_high);
//
//    IR2_OPND label_true2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit  = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_le_d, &dest_hi, &src_hi);
//
//    append_ir2_opnd1(mips_bc1t, &label_true2);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true2);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, true);
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&temp_int);
    return true;
}

bool translate_cmpunordpd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fcund, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_un_d, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1t, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    append_ir2_opnd1(mips_b, &label_high);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* high 64 bits */
//    append_ir2_opnd1(mips_label, &label_high);
//
//    IR2_OPND label_true2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit  = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_un_d, &dest_hi, &src_hi);
//
//    append_ir2_opnd1(mips_bc1t, &label_true2);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true2);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, true);
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&temp_int);
    return true;
}

bool translate_cmpneqpd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fcned, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_eq_d, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1f, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    append_ir2_opnd1(mips_b, &label_high);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* high 64 bits */
//    append_ir2_opnd1(mips_label, &label_high);
//    IR2_OPND label_true2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_eq_d, &dest_hi, &src_hi);
//
//    append_ir2_opnd1(mips_bc1f, &label_true2);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true2);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, true);
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&temp_int);
    return true;
}

bool translate_cmpnltpd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fcled, &dest, &src, &dest);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_lt_d, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1f, &label_true);
//    /* the low 64 bits of dest is less than that of src */
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    append_ir2_opnd1(mips_b, &label_high);
//    /* the low 64 bits of dest is not less than that of src */
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* high 64 bits */
//    append_ir2_opnd1(mips_label, &label_high);
//
//    IR2_OPND label_true2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_lt_d, &dest_hi, &src_hi);
//
//    append_ir2_opnd1(mips_bc1f, &label_true2);
//    /* the high 64 bits of dest is less than that of src */
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    append_ir2_opnd1(mips_b, &label_exit);
//    /* the high 64 bits of dest is not less than that of src */
//    append_ir2_opnd1(mips_label, &label_true2);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, true);
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&temp_int);
    return true;
}

bool translate_cmpnlepd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fcltd, &dest, &src, &dest);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_le_d, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1f, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    append_ir2_opnd1(mips_b, &label_high);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* high 64 bits */
//    append_ir2_opnd1(mips_label, &label_high);
//
//    IR2_OPND label_true2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit  = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_le_d, &dest_hi, &src_hi);
//
//    append_ir2_opnd1(mips_bc1f, &label_true2);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true2);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, true);
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&temp_int);
    return true;
}

bool translate_cmpordpd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fcord, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_un_d, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1f, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    append_ir2_opnd1(mips_b, &label_high);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//
//    /* high 64 bits */
//    append_ir2_opnd1(mips_label, &label_high);
//    IR2_OPND label_true2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_un_d, &dest_hi, &src_hi);
//
//    append_ir2_opnd1(mips_bc1f, &label_true2);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true2);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, true);
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&temp_int);
    return true;
}

bool translate_cmpeqps(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fceqw, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    /* save high 32 bits of dest_lo and src_lo */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    IR2_OPND temp_src = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_lo, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* eq? */
//    append_ir2_opnd2(mips_c_eq_s, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1t, &label_true);
//    /* the low 32 bits of dest and src are not equal */
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    append_ir2_opnd1(mips_b, &label_high);
//    /* the low 32 bits of dest and src are equal */
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* the second single */
//    append_ir2_opnd1(mips_label, &label_high);
//
//    IR2_OPND label_true2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_eq_s, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_bc1t, &label_true2);
//    /* the high 32 bits of dest and src are not equal */
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &temp_dest);
//
//    append_ir2_opnd1(mips_b, &label_exit);
//    /* the high 32 bits of dest and src are equal */
//    append_ir2_opnd1(mips_label, &label_true2);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &temp_dest);
//
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    /* high 64 bits */
//    /* save high 32 bits of dest_hi and src_hi */
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_hi, &f32_ir2_opnd);
//
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_hi, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true3 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high1 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* eq? */
//    append_ir2_opnd2(mips_c_eq_s, &dest_hi, &src_hi);
//
//    append_ir2_opnd1(mips_bc1t, &label_true3);
//    /* the low 32 bits of dest and src are not equal */
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    append_ir2_opnd1(mips_b, &label_high1);
//    /* the low 32 bits of dest and src are equal */
//    append_ir2_opnd1(mips_label, &label_true3);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, true);
//    /* the second single */
//    append_ir2_opnd1(mips_label, &label_high1);
//    IR2_OPND label_true4 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_eq_s, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_bc1t, &label_true4);
//    /* the high 32 bits of dest and src are not equal */
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &temp_dest);
//
//    append_ir2_opnd1(mips_b, &label_exit2);
//    /* the high 32 bits of dest and src are equal */
//    append_ir2_opnd1(mips_label, &label_true4);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &temp_dest);
//
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit2);
//    append_ir2_opnd3(mips_punpcklwd, &dest_hi, &dest_hi, &temp_dest);
//
//    ra_free_temp(&temp_int);
//    ra_free_temp(&temp_dest);
//    ra_free_temp(&temp_src);
    return true;
}

bool translate_cmpltps(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fcltw, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    /* save high 32 bits of dest_lo and src_lo */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    IR2_OPND temp_src = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_lo, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* lt? */
//    append_ir2_opnd2(mips_c_lt_s, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1t, &label_true);
//    /* the low 32 bits of dest is not less than that of src */
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    append_ir2_opnd1(mips_b, &label_high);
//    /* the low 32 bits of dest is less than that of src */
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* the second single */
//    append_ir2_opnd1(mips_label, &label_high);
//    IR2_OPND label_true2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_lt_s, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_bc1t, &label_true2);
//    /* the high 32 bits of dest is not less than that of src */
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &temp_dest);
//
//    append_ir2_opnd1(mips_b, &label_exit);
//    /* the high 32 bits of dest is less than that of src */
//    append_ir2_opnd1(mips_label, &label_true2);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &temp_dest);
//
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    /* high 64 bits */
//    /* save high 32 bits of dest_hi and src_hi */
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_hi, &f32_ir2_opnd);
//
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_hi, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true3 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high1 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* lt? */
//    append_ir2_opnd2(mips_c_lt_s, &dest_hi, &src_hi);
//
//    append_ir2_opnd1(mips_bc1t, &label_true3);
//    /* the low 32 bits of dest is not less than that of src */
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    append_ir2_opnd1(mips_b, &label_high1);
//    /* the low 32 bits of dest is less than that of src */
//    append_ir2_opnd1(mips_label, &label_true3);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, true);
//    /* the second single */
//    append_ir2_opnd1(mips_label, &label_high1);
//    IR2_OPND label_true4 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_lt_s, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_bc1t, &label_true4);
//    /* the high 32 bits of dest is not less than that of src */
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &temp_dest);
//
//    append_ir2_opnd1(mips_b, &label_exit2);
//    /* the high 32 bits of dest is less than that of src */
//    append_ir2_opnd1(mips_label, &label_true4);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &temp_dest);
//
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit2);
//    append_ir2_opnd3(mips_punpcklwd, &dest_hi, &dest_hi, &temp_dest);
//
//    ra_free_temp(&temp_int);
//    ra_free_temp(&temp_dest);
//    ra_free_temp(&temp_src);
    return true;
}

bool translate_cmpleps(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fclew, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    /* save high 32 bits of dest_lo and src_lo */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    IR2_OPND temp_src = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_lo, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* le? */
//    append_ir2_opnd2(mips_c_le_s, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1t, &label_true);
//    /* the low 32 bits of dest is not less equal than that of src */
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//    append_ir2_opnd1(mips_b, &label_high);
//    /* the low 32 bits of dest is less equal than that of src */
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* the second single */
//    append_ir2_opnd1(mips_label, &label_high);
//
//    IR2_OPND label_true2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_le_s, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_bc1t, &label_true2);
//    /* the high 32 bits of dest is not less equal than that of src */
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &temp_dest);
//
//    append_ir2_opnd1(mips_b, &label_exit);
//    /* the high 32 bits of dest is less equal than that of src */
//    append_ir2_opnd1(mips_label, &label_true2);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &temp_dest);
//
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    /* high 64 bits */
//    /* save high 32 bits of dest_hi and src_hi */
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_hi, &f32_ir2_opnd);
//
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_hi, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true3 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high1 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* lt? */
//    append_ir2_opnd2(mips_c_le_s, &dest_hi, &src_hi);
//
//    append_ir2_opnd1(mips_bc1t, &label_true3);
//    /* the low 32 bits of dest is not less equal than that of src */
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    append_ir2_opnd1(mips_b, &label_high1);
//    /* the low 32 bits of dest is less equal than that of src */
//    append_ir2_opnd1(mips_label, &label_true3);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, true);
//    /* the second single */
//    append_ir2_opnd1(mips_label, &label_high1);
//
//    IR2_OPND label_true4 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_le_s, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_bc1t, &label_true4);
//    /* the high 32 bits of dest is not less equal than that of src */
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &temp_dest);
//
//    append_ir2_opnd1(mips_b, &label_exit2);
//    /* the high 32 bits of dest is less equal than that of src */
//    append_ir2_opnd1(mips_label, &label_true4);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &temp_dest);
//
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit2);
//    append_ir2_opnd3(mips_punpcklwd, &dest_hi, &dest_hi, &temp_dest);
//
//    ra_free_temp(&temp_int);
//    ra_free_temp(&temp_dest);
//    ra_free_temp(&temp_src);
    return true;
}

bool translate_cmpunordps(IR1_INST *pir1)
{
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fcunw, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    /* save high 32 bits of dest_lo and src_lo */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    IR2_OPND temp_src = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_lo, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* un? */
//    append_ir2_opnd2(mips_c_un_s, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1t, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    append_ir2_opnd1(mips_b, &label_high);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* the second single */
//    append_ir2_opnd1(mips_label, &label_high);
//
//    IR2_OPND label_true2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_un_s, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_bc1t, &label_true2);
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &temp_dest);
//
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true2);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &temp_dest);
//
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    /* high 64 bits */
//    /* save high 32 bits of dest_hi and src_hi */
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_hi, &f32_ir2_opnd);
//
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_hi, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true3 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high1 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* un? */
//    append_ir2_opnd2(mips_c_un_s, &dest_hi, &src_hi);
//
//    append_ir2_opnd1(mips_bc1t, &label_true3);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    append_ir2_opnd1(mips_b, &label_high1);
//    append_ir2_opnd1(mips_label, &label_true3);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, true);
//    /* the second single */
//    append_ir2_opnd1(mips_label, &label_high1);
//
//    IR2_OPND label_true4 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_un_s, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_bc1t, &label_true4);
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &temp_dest);
//
//    append_ir2_opnd1(mips_b, &label_exit2);
//    append_ir2_opnd1(mips_label, &label_true4);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &temp_dest);
//
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit2);
//    append_ir2_opnd3(mips_punpcklwd, &dest_hi, &dest_hi, &temp_dest);
//
//    ra_free_temp(&temp_int);
//    ra_free_temp(&temp_dest);
//    ra_free_temp(&temp_src);
    return true;
}

bool translate_cmpneqps(IR1_INST *pir1)
{
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fcnew, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    /* save high 32 bits of dest_lo and src_lo */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    IR2_OPND temp_src = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_lo, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* eq? */
//    append_ir2_opnd2(mips_c_eq_s, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1f, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    append_ir2_opnd1(mips_b, &label_high);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* the second single */
//    append_ir2_opnd1(mips_label, &label_high);
//
//    IR2_OPND label_true2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_eq_s, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_bc1f, &label_true2);
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &temp_dest);
//
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true2);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &temp_dest);
//
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    /* high 64 bits */
//    /* save high 32 bits of dest_hi and src_hi */
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_hi, &f32_ir2_opnd);
//
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_hi, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true3 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high1 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* eq? */
//    append_ir2_opnd2(mips_c_eq_s, &dest_hi, &src_hi);
//
//    append_ir2_opnd1(mips_bc1f, &label_true3);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    append_ir2_opnd1(mips_b, &label_high1);
//    append_ir2_opnd1(mips_label, &label_true3);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, true);
//    /* the second single */
//    append_ir2_opnd1(mips_label, &label_high1);
//
//    IR2_OPND label_true4 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_eq_s, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_bc1f, &label_true4);
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &temp_dest);
//
//    append_ir2_opnd1(mips_b, &label_exit2);
//    append_ir2_opnd1(mips_label, &label_true4);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &temp_dest);
//
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit2);
//    append_ir2_opnd3(mips_punpcklwd, &dest_hi, &dest_hi, &temp_dest);
//
//    ra_free_temp(&temp_int);
//    ra_free_temp(&temp_dest);
//    ra_free_temp(&temp_src);
    return true;
}

bool translate_cmpnltps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fclew, &dest, &src, &dest);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    /* save high 32 bits of dest_lo and src_lo */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    IR2_OPND temp_src = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_lo, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* lt? */
//    append_ir2_opnd2(mips_c_lt_s, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1f, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    append_ir2_opnd1(mips_b, &label_high);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* the second single */
//    append_ir2_opnd1(mips_label, &label_high);
//
//    IR2_OPND label_true2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_lt_s, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_bc1f, &label_true2);
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &temp_dest);
//
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true2);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &temp_dest);
//
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    /* high 64 bits */
//    /* save high 32 bits of dest_hi and src_hi */
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_hi, &f32_ir2_opnd);
//
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_hi, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true3 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high1 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* lt? */
//    append_ir2_opnd2(mips_c_lt_s, &dest_hi, &src_hi);
//
//    append_ir2_opnd1(mips_bc1f, &label_true3);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    append_ir2_opnd1(mips_b, &label_high1);
//    append_ir2_opnd1(mips_label, &label_true3);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, true);
//    /* the second single */
//    append_ir2_opnd1(mips_label, &label_high1);
//
//    IR2_OPND label_true4 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_lt_s, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_bc1f, &label_true4);
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &temp_dest);
//
//    append_ir2_opnd1(mips_b, &label_exit2);
//    append_ir2_opnd1(mips_label, &label_true4);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &temp_dest);
//
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit2);
//    append_ir2_opnd3(mips_punpcklwd, &dest_hi, &dest_hi, &temp_dest);
//
//    ra_free_temp(&temp_int);
//    ra_free_temp(&temp_dest);
//    ra_free_temp(&temp_src);
    return true;
}

bool translate_cmpnleps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fcltw, &dest, &src, &dest);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    /* save high 32 bits of dest_lo and src_lo */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    IR2_OPND temp_src = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_lo, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* le? */
//    append_ir2_opnd2(mips_c_le_s, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1f, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    append_ir2_opnd1(mips_b, &label_high);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* the second single */
//    append_ir2_opnd1(mips_label, &label_high);
//
//    IR2_OPND label_true2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_le_s, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_bc1f, &label_true2);
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &temp_dest);
//
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true2);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &temp_dest);
//
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    /* high 64 bits */
//    /* save high 32 bits of dest_hi and src_hi */
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_hi, &f32_ir2_opnd);
//
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_hi, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true3 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high1 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* lt? */
//    append_ir2_opnd2(mips_c_le_s, &dest_hi, &src_hi);
//
//    append_ir2_opnd1(mips_bc1f, &label_true3);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    append_ir2_opnd1(mips_b, &label_high1);
//    append_ir2_opnd1(mips_label, &label_true3);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, true);
//    /* the second single */
//    append_ir2_opnd1(mips_label, &label_high1);
//
//    IR2_OPND label_true4 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_le_s, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_bc1f, &label_true4);
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &temp_dest);
//
//    append_ir2_opnd1(mips_b, &label_exit2);
//    append_ir2_opnd1(mips_label, &label_true4);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &temp_dest);
//
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit2);
//    append_ir2_opnd3(mips_punpcklwd, &dest_hi, &dest_hi, &temp_dest);
//
//    ra_free_temp(&temp_int);
//    ra_free_temp(&temp_dest);
//    ra_free_temp(&temp_src);
    return true;
}

bool translate_cmpordps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fcorw, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    /* save high 32 bits of dest_lo and src_lo */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    IR2_OPND temp_src = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_lo, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* un? */
//    append_ir2_opnd2(mips_c_un_s, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1f, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    append_ir2_opnd1(mips_b, &label_high);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* the second single */
//    append_ir2_opnd1(mips_label, &label_high);
//
//    IR2_OPND label_true2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_un_s, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_bc1f, &label_true2);
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &temp_dest);
//
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true2);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &temp_dest);
//
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    /* high 64 bits */
//    /* save high 32 bits of dest_hi and src_hi */
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_hi, &f32_ir2_opnd);
//
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_hi, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true3 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high1 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* un? */
//    append_ir2_opnd2(mips_c_un_s, &dest_hi, &src_hi);
//
//    append_ir2_opnd1(mips_bc1f, &label_true3);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    append_ir2_opnd1(mips_b, &label_high1);
//    append_ir2_opnd1(mips_label, &label_true3);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, true);
//    /* the second single */
//    append_ir2_opnd1(mips_label, &label_high1);
//
//    IR2_OPND label_true4 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_un_s, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_bc1f, &label_true4);
//    append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &temp_dest);
//
//    append_ir2_opnd1(mips_b, &label_exit2);
//    append_ir2_opnd1(mips_label, &label_true4);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &temp_dest);
//
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit2);
//    append_ir2_opnd3(mips_punpcklwd, &dest_hi, &dest_hi, &temp_dest);
//
//    ra_free_temp(&temp_int);
//    ra_free_temp(&temp_dest);
//    ra_free_temp(&temp_src);
    return true;
}

bool translate_cmpeqsd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fceqd, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insved, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_eq_d, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1t, &label_true);
//    /* the low 64 bits of dest and src are not equal */
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_b, &label_exit);
//    /* the low 64 bits of dest and src are equal */
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&temp_int);
    return true;
}

bool translate_cmpltsd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fcltd, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insved, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_lt_d, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1t, &label_true);
//    /* the low 64 bits of dest is not less than that of src */
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_b, &label_exit);
//    /* the low 64 bits of dest is less than that of src */
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&temp_int);
    return true;
}

bool translate_cmplesd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fcled, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insved, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_le_d, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1t, &label_true);
//    /* the low 64 bits of dest is not less equal than that of src */
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_b, &label_exit);
//    /* the low 64 bits of dest is less equal than that of src */
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&temp_int);
    return true;
}

bool translate_cmpunordsd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fcund, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insved, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_un_d, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1t, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&temp_int);
    return true;
}

bool translate_cmpneqsd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fcned, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insved, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_eq_d, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1f, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&temp_int);
    return true;
}

bool translate_cmpnltsd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fcltd, &temp, &dest, &src);
//        IR2_OPND zero = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_xorv, &zero, &zero, &zero);
//        append_ir2_opnd3(mips_norv, &temp, &temp, &zero);
//        append_ir2_opnd2i(mips_insved, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_lt_d, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1f, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&temp_int);
    return true;
}

bool translate_cmpnlesd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fcled, &temp, &dest, &src);
//        IR2_OPND zero = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_xorv, &zero, &zero, &zero);
//        append_ir2_opnd3(mips_norv, &temp, &temp, &zero);
//        append_ir2_opnd2i(mips_insved, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_le_d, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1f, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&temp_int);
    return true;
}

bool translate_cmpordsd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fcord, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insved, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_un_d, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1f, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    append_ir2_opnd1(mips_label, &label_exit);
//
//    ra_free_temp(&temp_int);
    return true;
}

bool translate_cmpeqss(IR1_INST *pir1)
{
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fceqw, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, false);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    /* save high 32 bits of dest_lo */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* eq? */
//    append_ir2_opnd2(mips_c_eq_s, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1t, &label_true);
//    /* the low 32 bits of dest and src are not equal */
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//    append_ir2_opnd1(mips_b, &label_exit);
//    /* the low 32 bits of dest and src are equal */
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    ra_free_temp(&temp_int);
//    ra_free_temp(&temp_dest);
    return true;
}

bool translate_cmpltss(IR1_INST *pir1)
{
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fcltw, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, false);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    /* save high 32 bits of dest_lo */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* lt? */
//    append_ir2_opnd2(mips_c_lt_s, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1t, &label_true);
//    /* the low 32 bits of dest is not less than that of src */
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//    append_ir2_opnd1(mips_b, &label_exit);
//    /* the low 32 bits of dest is less than that of src */
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    ra_free_temp(&temp_int);
//    ra_free_temp(&temp_dest);
    return true;
}

bool translate_cmpless(IR1_INST *pir1)
{
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fclew, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, false);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    /* save high 32 bits of dest_lo */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_le_s, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1t, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    ra_free_temp(&temp_int);
//    ra_free_temp(&temp_dest);
    return true;
}

bool translate_cmpunordss(IR1_INST *pir1)
{
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fcunw, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, false);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    /* save high 32 bits of dest_lo */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_un_s, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1t, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    ra_free_temp(&temp_int);
//    ra_free_temp(&temp_dest);
    return true;
}

bool translate_cmpneqss(IR1_INST *pir1)
{
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fcnew, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, false);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    /* save high 32 bits of dest_lo */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_eq_s, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1f, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    ra_free_temp(&temp_int);
//    ra_free_temp(&temp_dest);
    return true;
}

bool translate_cmpnltss(IR1_INST *pir1)
{
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fcltw, &temp, &dest, &src);
//        IR2_OPND zero = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_xorv, &zero, &zero, &zero);
//        append_ir2_opnd3(mips_norv, &temp, &temp, &zero);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, false);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    /* save high 32 bits of dest_lo */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_lt_s, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1f, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    ra_free_temp(&temp_int);
//    ra_free_temp(&temp_dest);
    return true;
}

bool translate_cmpnless(IR1_INST *pir1)
{
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fclew, &temp, &dest, &src);
//        IR2_OPND zero = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_xorv, &zero, &zero, &zero);
//        append_ir2_opnd3(mips_norv, &temp, &temp, &zero);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, false);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    /* save high 32 bits of dest_lo */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_le_s, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1f, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    ra_free_temp(&temp_int);
//    ra_free_temp(&temp_dest);
    return true;
}

bool translate_cmpordss(IR1_INST *pir1)
{
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fcorw, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, false);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//
//    /* low 64 bits */
//    /* save high 32 bits of dest_lo */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    /* the first single scalar operand */
//    IR2_OPND label_true = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_un_s, &dest_lo, &src_lo);
//
//    append_ir2_opnd1(mips_bc1f, &label_true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_true);
//    append_ir2_opnd3(mips_nor, &temp_int, &zero_ir2_opnd, &zero_ir2_opnd);
//
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//    /* merge */
//    append_ir2_opnd1(mips_label, &label_exit);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    ra_free_temp(&temp_int);
//    ra_free_temp(&temp_dest);
    return true;
}

bool translate_comisd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map){
//        lsassert(ir1_opnd_num(pir1) == 2);
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp;
//
//        IR2_OPND label_nun  = ir2_opnd_new_type(IR2_OPND_LABEL);
//        IR2_OPND label_neq  = ir2_opnd_new_type(IR2_OPND_LABEL);
//        IR2_OPND label_gt   = ir2_opnd_new_type(IR2_OPND_LABEL);
//        IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//        if (option_lbt) {
//            temp = ra_alloc_itemp();
//            append_ir2_opnd2i(mips_ori, &temp, &zero_ir2_opnd, 0xfff);
//        }
//
//        /*  case 1: are they unordered? */
//        append_ir2_opnd2(mips_c_un_d, &dest, &src);
//        append_ir2_opnd1(mips_bc1f, &label_nun);
//        /* at least one of the operands is NaN */
//        // set zf,pf,cf = 111
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &temp, 0xb);
//        } else {
//            append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0045);
//        }
//        append_ir2_opnd1(mips_b, &label_exit);
//
//        /* case 2: not unordered. are they equal? */
//        append_ir2_opnd1(mips_label, &label_nun);
//        append_ir2_opnd2(mips_c_eq_d, &dest, &src);
//        append_ir2_opnd1(mips_bc1f, &label_neq);
//        /* two operands are equal */
//        // set zf,pf,cf = 100
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &temp, 0x8);
//            append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0x3);
//        } else {
//            append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0040);
//            /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xfffa);           */
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, PF_BIT | CF_BIT);
//        }
//        append_ir2_opnd1(mips_b, &label_exit);
//
//        /* case 3: not unordered, not equal. less than? */
//        append_ir2_opnd1(mips_label, &label_neq);
//        append_ir2_opnd2(mips_c_lt_d, &dest, &src);
//        append_ir2_opnd1(mips_bc1f, &label_gt);
//        /* less than */
//        // set zf.pf.cf = 001
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0xa);
//            append_ir2_opnd1i(mips_mtflag, &temp, 0x1);
//        } else {
//            append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0001);
//            /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xffbb);           */
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, ZF_BIT | PF_BIT);
//        }
//        append_ir2_opnd1(mips_b, &label_exit);
//
//        /*  not unordered, not equal, not less than, so it's greater than */
//        //set zf,pf,cf = 000
//        append_ir2_opnd1(mips_label, &label_gt);
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0xb);
//        } else {
//            /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xffba);*/
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, ZF_BIT | PF_BIT | CF_BIT);
//        }
//
//        /* exit */
//        //set of,sf,af = 000
//        append_ir2_opnd1(mips_label, &label_exit);
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0x34);
//            ra_free_temp(&temp);
//        } else {
//            /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xf76f);*/
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT | SF_BIT | AF_BIT);
//        }
//
//        return true;
//    }
//
//    lsassert(ir1_opnd_num(pir1) == 2);
//
//    IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, true);
//
//    IR2_OPND temp;
//
//    IR2_OPND label_nun  = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_neq  = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_gt   = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    if (option_lbt) {
//        temp = ra_alloc_itemp();
//        append_ir2_opnd2i(mips_ori, &temp, &zero_ir2_opnd, 0xfff);
//    }
//
//    /*  case 1: are they unordered? */
//    append_ir2_opnd2(mips_c_un_d, &dest, &src);
//    append_ir2_opnd1(mips_bc1f, &label_nun);
//    /* at least one of the operands is NaN */
//    // set zf,pf,cf = 111
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &temp, 0xb);
//    } else {
//        append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0045);
//    }
//    append_ir2_opnd1(mips_b, &label_exit);
//
//    /* case 2: not unordered. are they equal? */
//    append_ir2_opnd1(mips_label, &label_nun);
//    append_ir2_opnd2(mips_c_eq_d, &dest, &src);
//    append_ir2_opnd1(mips_bc1f, &label_neq);
//    /* two operands are equal */
//    // set zf,pf,cf = 100
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &temp, 0x8);
//        append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0x3);
//    } else {
//        append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0040);
//        /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xfffa);           */
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, PF_BIT | CF_BIT);
//    }
//    append_ir2_opnd1(mips_b, &label_exit);
//
//    /* case 3: not unordered, not equal. less than? */
//    append_ir2_opnd1(mips_label, &label_neq);
//    append_ir2_opnd2(mips_c_lt_d, &dest, &src);
//    append_ir2_opnd1(mips_bc1f, &label_gt);
//    /* less than */
//    // set zf.pf.cf = 001
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0xa);
//        append_ir2_opnd1i(mips_mtflag, &temp, 0x1);
//    } else {
//        append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0001);
//        /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xffbb);           */
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, ZF_BIT | PF_BIT);
//    }
//    append_ir2_opnd1(mips_b, &label_exit);
//
//    /*  not unordered, not equal, not less than, so it's greater than */
//    //set zf,pf,cf = 000
//    append_ir2_opnd1(mips_label, &label_gt);
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0xb);
//    } else {
//        /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xffba);*/
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, ZF_BIT | PF_BIT | CF_BIT);
//    }
//
//    /* exit */
//    //set of,sf,af = 000
//    append_ir2_opnd1(mips_label, &label_exit);
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0x34);
//        ra_free_temp(&temp);
//    } else {
//        /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xf76f);*/
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT | SF_BIT | AF_BIT);
//    }
//
    return true;
}

bool translate_comiss(IR1_INST *pir1)
{
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map){
//        lsassert(ir1_opnd_num(pir1) == 2);
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp;
//
//        IR2_OPND label_nun  = ir2_opnd_new_type(IR2_OPND_LABEL);
//        IR2_OPND label_neq  = ir2_opnd_new_type(IR2_OPND_LABEL);
//        IR2_OPND label_gt   = ir2_opnd_new_type(IR2_OPND_LABEL);
//        IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//        if (option_lbt) {
//            temp = ra_alloc_itemp();
//            append_ir2_opnd2i(mips_ori, &temp, &zero_ir2_opnd, 0xfff);
//        }
//
//        /*  case 1: are they unordered? */
//        append_ir2_opnd2(mips_c_un_s, &dest, &src);
//        append_ir2_opnd1(mips_bc1f, &label_nun);
//        /* at least one of the operands is NaN */
//        // set zf,pf,cf = 111
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &temp, 0xb);
//        } else {
//            append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0045);
//        }
//        append_ir2_opnd1(mips_b, &label_exit);
//
//        /* case 2: not unordered. are they equal? */
//        append_ir2_opnd1(mips_label, &label_nun);
//        append_ir2_opnd2(mips_c_eq_s, &dest, &src);
//        append_ir2_opnd1(mips_bc1f, &label_neq);
//        /* two operands are equal */
//        // set zf,pf,cf = 100
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &temp, 0x8);
//            append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0x3);
//        } else {
//            append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0040);
//            /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xfffa);           */
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, PF_BIT | CF_BIT);
//        }
//        append_ir2_opnd1(mips_b, &label_exit);
//
//        /* case 3: not unordered, not equal. less than? */
//        append_ir2_opnd1(mips_label, &label_neq);
//        append_ir2_opnd2(mips_c_lt_s, &dest, &src);
//        append_ir2_opnd1(mips_bc1f, &label_gt);
//        /* less than */
//        // set zf.pf.cf = 001
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0xa);
//            append_ir2_opnd1i(mips_mtflag, &temp, 0x1);
//        } else {
//            append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0001);
//            /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xffbb);           */
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, ZF_BIT | PF_BIT);
//        }
//
//        append_ir2_opnd1(mips_b, &label_exit);
//
//        /* not unordered, not equal, not less than, so it's greater than */
//        //set zf,pf,cf = 000
//        append_ir2_opnd1(mips_label, &label_gt);
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0xb);
//        } else {
//            /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xffba);*/
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, ZF_BIT | PF_BIT | CF_BIT);
//        }
//
//        /* exit */
//        //set of,sf,af = 000
//        append_ir2_opnd1(mips_label, &label_exit);
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0x34);
//            ra_free_temp(&temp);
//        } else {
//            /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xf76f);*/
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT | SF_BIT | AF_BIT);
//        }
//
//        return true;
//    }
//
//    lsassert(ir1_opnd_num(pir1) == 2);
//    IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, false);
//    IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, false);
//    IR2_OPND temp;
//
//    IR2_OPND label_nun  = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_neq  = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_gt   = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    if (option_lbt) {
//        temp = ra_alloc_itemp();
//        append_ir2_opnd2i(mips_ori, &temp, &zero_ir2_opnd, 0xfff);
//    }
//
//    /*  case 1: are they unordered? */
//    append_ir2_opnd2(mips_c_un_s, &dest, &src);
//    append_ir2_opnd1(mips_bc1f, &label_nun);
//    /* at least one of the operands is NaN */
//    // set zf,pf,cf = 111
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &temp, 0xb);
//    } else {
//        append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0045);
//    }
//    append_ir2_opnd1(mips_b, &label_exit);
//
//    /* case 2: not unordered. are they equal? */
//    append_ir2_opnd1(mips_label, &label_nun);
//    append_ir2_opnd2(mips_c_eq_s, &dest, &src);
//    append_ir2_opnd1(mips_bc1f, &label_neq);
//    /* two operands are equal */
//    // set zf,pf,cf = 100
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &temp, 0x8);
//        append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0x3);
//    } else {
//        append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0040);
//        /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xfffa);           */
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, PF_BIT | CF_BIT);
//    }
//    append_ir2_opnd1(mips_b, &label_exit);
//
//    /* case 3: not unordered, not equal. less than? */
//    append_ir2_opnd1(mips_label, &label_neq);
//    append_ir2_opnd2(mips_c_lt_s, &dest, &src);
//    append_ir2_opnd1(mips_bc1f, &label_gt);
//    /* less than */
//    // set zf.pf.cf = 001
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0xa);
//        append_ir2_opnd1i(mips_mtflag, &temp, 0x1);
//    } else {
//        append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0001);
//        /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xffbb);           */
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, ZF_BIT | PF_BIT);
//    }
//    append_ir2_opnd1(mips_b, &label_exit);
//
//    /* not unordered, not equal, not less than, so it's greater than */
//    //set zf,pf,cf = 000
//    append_ir2_opnd1(mips_label, &label_gt);
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0xb);
//    } else {
//        /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xffba);*/
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, ZF_BIT | PF_BIT | CF_BIT);
//    }
//
//    /* exit */
//    //set of,sf,af = 000
//    append_ir2_opnd1(mips_label, &label_exit);
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0x34);
//        ra_free_temp(&temp);
//    } else {
//        /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xf76f);*/
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT | SF_BIT | AF_BIT);
//    }
//
    return true;
}

bool translate_cvtdq2pd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD pxor to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND fcsr_opnd = set_fpu_fcsr_rounding_field_by_x86();
//
//        //TODO:simply
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        IR2_OPND temp  = ra_alloc_ftemp();
//        IR2_OPND temp0 = ra_alloc_ftemp();
//
//        append_ir2_opnd2(mips_cvt_d_w, &temp0, &src);
//        append_ir2_opnd2i(mips_shfw, &temp, &src, 0x55);
//        append_ir2_opnd2(mips_cvt_d_w, &temp, &temp);
//        append_ir2_opnd2i(mips_insved, &dest, &temp, 1);
//        append_ir2_opnd2i(mips_insved, &dest, &temp0, 0);
//
//        set_fpu_fcsr(&fcsr_opnd);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src     = load_freg_from_ir1_1(opnd1, false, true);
//
//    IR2_OPND ftemp_src_lo32 = ra_alloc_ftemp();
//    IR2_OPND ftemp_src_hi32 = ra_alloc_ftemp();
//
//    append_ir2_opnd3(mips_fdsll, &ftemp_src_lo32, &src, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_fdsrl, &ftemp_src_lo32, &ftemp_src_lo32, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_fdsrl, &ftemp_src_hi32, &src, &f32_ir2_opnd);
//    append_ir2_opnd2(mips_cvt_d_w, &dest_lo, &ftemp_src_lo32);
//    append_ir2_opnd2(mips_cvt_d_w, &dest_hi, &ftemp_src_hi32);
//
//    ra_free_temp(&ftemp_src_lo32);
//    ra_free_temp(&ftemp_src_hi32);
    return true;
}

bool translate_cvtdq2ps(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        // TODO:simply
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        IR2_OPND temp  = ra_alloc_ftemp();
//        IR2_OPND temp0 = ra_alloc_ftemp();
//
//        append_ir2_opnd2(mips_cvt_s_w, &temp0, &src);
//        append_ir2_opnd2i(mips_shfw, &temp, &src, 0x55);
//        append_ir2_opnd2(mips_cvt_s_w, &temp, &temp);
//        append_ir2_opnd2i(mips_insvew, &temp0, &temp, 1);
//        append_ir2_opnd2i(mips_shfw, &temp, &src, 0xaa);
//        append_ir2_opnd2(mips_cvt_s_w, &temp, &temp);
//        append_ir2_opnd2i(mips_insvew, &temp0, &temp, 2);
//        append_ir2_opnd2i(mips_shfw, &temp, &src, 0xff);
//        append_ir2_opnd2(mips_cvt_s_w, &temp, &temp);
//        append_ir2_opnd2i(mips_insvew, &temp0, &temp, 3);
//        append_ir2_opnd2(mips_movev, &dest, &temp0);
//
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    /* low 64bit */
//    IR2_OPND ftemp_src_lo32 = ra_alloc_ftemp();
//    IR2_OPND ftemp_src_hi32 = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsll, &ftemp_src_lo32, &src_lo, &f32_ir2_opnd);
//
//    /* ftemp_src_lo32 is low 32bit of src_lo */
//    append_ir2_opnd3(mips_fdsrl, &ftemp_src_lo32, &ftemp_src_lo32, &f32_ir2_opnd);
//
//    /* ftemp_src_hi32 is high 32bit of src_lo */
//    append_ir2_opnd3(mips_fdsrl, &ftemp_src_hi32, &src_lo, &f32_ir2_opnd);
//    append_ir2_opnd2(mips_cvt_s_w, &ftemp_src_lo32, &ftemp_src_lo32);
//    append_ir2_opnd2(mips_cvt_s_w, &ftemp_src_hi32, &ftemp_src_hi32);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &ftemp_src_lo32, &ftemp_src_hi32);
//
//    /* high 64bit */
//    append_ir2_opnd3(mips_fdsll, &ftemp_src_lo32, &src_hi, &f32_ir2_opnd);
//
//    /* ftemp_src_lo32 is low 32bit of src_hi */
//    append_ir2_opnd3(mips_fdsrl, &ftemp_src_lo32, &ftemp_src_lo32, &f32_ir2_opnd);
//
//    /* ftemp_src_hi32 is high 32bit of src_hi */
//    append_ir2_opnd3(mips_fdsrl, &ftemp_src_hi32, &src_hi, &f32_ir2_opnd);
//    append_ir2_opnd2(mips_cvt_s_w, &ftemp_src_lo32, &ftemp_src_lo32);
//    append_ir2_opnd2(mips_cvt_s_w, &ftemp_src_hi32, &ftemp_src_hi32);
//    append_ir2_opnd3(mips_punpcklwd, &dest_hi, &ftemp_src_lo32, &ftemp_src_hi32);
//
//    ra_free_temp(&ftemp_src_lo32);
//    ra_free_temp(&ftemp_src_hi32);
    return true;
}

bool translate_cvtpd2dq(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    IR2_OPND src_lo;
//    IR2_OPND src_hi;
//    IR2_OPND dest_lo;
//
//    if (option_xmm128map){
//        src_lo = load_freg128_from_ir1(opnd1);
//        src_hi = ra_alloc_ftemp();
//        append_ir2_opnd2ii(mips_vextrinsd, &src_hi, &src_lo, 0, 1);
//        dest_lo = load_freg128_from_ir1(opnd0);
//    }
//    else {
//        src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//        src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//        dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    }
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//    /* double for 0x7fffffff.499999 */
//    load_imm64_to_ir2(&temp_int, 0x41dfffffffdffffcULL);
//
//    IR2_OPND ftemp_over_flow = ra_alloc_ftemp();
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_over_flow);
//
//    /* double for 0x80000000.499999 */
//    load_imm64_to_ir2(&temp_int, 0xc1dfffffffdffffcULL);
//
//    IR2_OPND ftemp_under_flow = ra_alloc_ftemp();
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_under_flow);
//
//    IR2_OPND label_for_flow1 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* low 64bit */
//    /* is unorder or overflow or under flow? */
//    IR2_OPND ftemp_src_lo = ra_alloc_ftemp();
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_d, &src_lo, &src_lo);
//    append_ir2_opnd1(mips_bc1t, &label_for_flow1);
//    /* is overflow? */
//    append_ir2_opnd2(mips_c_lt_d, &ftemp_over_flow, &src_lo);
//    append_ir2_opnd1(mips_bc1t, &label_for_flow1);
//    /* is under flow? */
//    append_ir2_opnd2(mips_c_lt_d, &src_lo, &ftemp_under_flow);
//    append_ir2_opnd1(mips_bc1t, &label_for_flow1);
//    /* is normal */
//    append_ir2_opnd2(mips_cvt_w_d, &ftemp_src_lo, &src_lo);
//
//    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_b, &label_high);
//    append_ir2_opnd1(mips_label, &label_for_flow1);
//
//    load_imm32_to_ir2(&temp_int, 0x80000000, UNKNOWN_EXTENSION);
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_src_lo);
//
//    append_ir2_opnd1(mips_label, &label_high);
//    IR2_OPND label_for_flow2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /* high 64bit */
//    /* is unorder or overflow or under flow? */
//    IR2_OPND ftemp_src_hi = ra_alloc_ftemp();
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_d, &src_hi, &src_hi);
//    append_ir2_opnd1(mips_bc1t, &label_for_flow2);
//    /* is overflow? */
//    append_ir2_opnd2(mips_c_lt_d, &ftemp_over_flow, &src_hi);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_flow2);
//    append_ir2_opnd2(mips_c_lt_d, &src_hi, &ftemp_under_flow);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_flow2);
//    /* is normal */
//    append_ir2_opnd2(mips_cvt_w_d, &ftemp_src_hi, &src_hi);
//
//    IR2_OPND label_over = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_b, &label_over);
//
//    append_ir2_opnd1(mips_label, &label_for_flow2);
//    load_imm32_to_ir2(&temp_int, 0x80000000, UNKNOWN_EXTENSION);
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_src_hi);
//
//    append_ir2_opnd1(mips_label, &label_over);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &ftemp_src_lo, &ftemp_src_hi);
//
//    if (option_xmm128map) {
//        append_ir2_opnd2i(mips_vclrstriv, &dest_lo, &dest_lo, 7);
//    } else{
//        store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    }
//
//    ra_free_temp(&ftemp_src_lo);
//    ra_free_temp(&ftemp_src_hi);
//    ra_free_temp(&ftemp_over_flow);
//    ra_free_temp(&ftemp_under_flow);
//    ra_free_temp(&temp_int);
    return true;
}

//static void tr_x87_to_mmx(void)
//{
//    //reset top
//    td_fpu_set_top(0);
//    append_ir2_opnd2i(LISA_ST_W, &zero_ir2_opnd, &env_ir2_opnd,
//                      lsenv_offset_of_top(lsenv));
//
//    tr_gen_top_mode_init();
//}

bool translate_cvtpd2pi(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    tr_x87_to_mmx();
//
//    IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//
//    IR2_OPND src_lo;
//    IR2_OPND src_hi;
//    if (option_xmm128map) {
//        src_lo = load_freg128_from_ir1(opnd1);
//        src_hi = ra_alloc_ftemp();
//        append_ir2_opnd2ii(mips_vextrinsd, &src_hi, &src_lo, 0, 1);
//    }
//    else {
//        src_lo = load_freg_from_ir1_1(opnd1, false, true);
//        src_hi = load_freg_from_ir1_1(opnd1, true, true);
//    }
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//    IR2_OPND ftemp_over_flow  = ra_alloc_ftemp();
//    IR2_OPND ftemp_under_flow = ra_alloc_ftemp();
//
//    IR2_OPND label_for_flow1 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_for_flow2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_over = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    /* double for 0x7fffffff.499999 */
//    load_imm64_to_ir2(&temp_int, 0x41dfffffffdffffcULL);
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_over_flow);
//
//    /* double for 0x80000000.499999 */
//    load_imm64_to_ir2(&temp_int, 0xc1dfffffffdffffcULL);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_under_flow);
//
//    /* low 64bit */
//    /* is unorder or overflow or under flow? */
//    IR2_OPND ftemp_src_lo = ra_alloc_ftemp();
//
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_d, &src_lo, &src_lo);
//    append_ir2_opnd1(mips_bc1t, &label_for_flow1);
//    /* is overflow? */
//    append_ir2_opnd2(mips_c_lt_d, &ftemp_over_flow, &src_lo);
//
//    /* is under flow? */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow1);
//    append_ir2_opnd2(mips_c_lt_d, &src_lo, &ftemp_under_flow);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_flow1);
//    append_ir2_opnd2(mips_cvt_w_d, &ftemp_src_lo, &src_lo);
//
//    append_ir2_opnd1(mips_b, &label_high);
//
//    append_ir2_opnd1(mips_label, &label_for_flow1);
//    load_imm32_to_ir2(&temp_int, 0x80000000, UNKNOWN_EXTENSION);
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_src_lo);
//
//    /* high 64bit */
//    append_ir2_opnd1(mips_label, &label_high);
//
//    /* is unorder or overflow or under flow? */
//    IR2_OPND ftemp_src_hi = ra_alloc_ftemp();
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_d, &src_hi, &src_hi);
//    append_ir2_opnd1(mips_bc1t, &label_for_flow2);
//    /* is overflow? */
//    append_ir2_opnd2(mips_c_lt_d, &ftemp_over_flow, &src_hi);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_flow2);
//    /* is under flow? */
//    append_ir2_opnd2(mips_c_lt_d, &src_hi, &ftemp_under_flow);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_flow2);
//    append_ir2_opnd2(mips_cvt_w_d, &ftemp_src_hi, &src_hi);
//
//    append_ir2_opnd1(mips_b, &label_over);
//
//    append_ir2_opnd1(mips_label, &label_for_flow2);
//    load_imm32_to_ir2(&temp_int, 0x80000000, UNKNOWN_EXTENSION);
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_src_hi);
//
//    append_ir2_opnd1(mips_label, &label_over);
//
//    append_ir2_opnd3(mips_punpcklwd, &dest, &ftemp_src_lo, &ftemp_src_hi);
//
//    ra_free_temp(&ftemp_src_lo);
//    ra_free_temp(&ftemp_src_hi);
//    ra_free_temp(&ftemp_over_flow);
//    ra_free_temp(&ftemp_under_flow);
//    ra_free_temp(&temp_int);
    return true;
}

bool translate_cvtpd2ps(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        // TODO:simply
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd2ii(mips_vextrinsd, &temp, &src, 0, 1);
//        append_ir2_opnd2(mips_cvt_s_d, &dest, &src);
//        append_ir2_opnd2(mips_cvt_s_d, &temp, &temp);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 1);
//        append_ir2_opnd2i(mips_vclrstriv, &dest, &dest, 7);
//
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    IR2_OPND ftemp_src_lo = ra_alloc_ftemp();
//    IR2_OPND ftemp_src_hi = ra_alloc_ftemp();
//
//    append_ir2_opnd2(mips_cvt_s_d, &ftemp_src_lo, &src_lo);
//    append_ir2_opnd2(mips_cvt_s_d, &ftemp_src_hi, &src_hi);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &ftemp_src_lo, &ftemp_src_hi);
//
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//
//    ra_free_temp(&ftemp_src_lo);
//    ra_free_temp(&ftemp_src_hi);
    return true;
}

bool translate_cvtpi2ps(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        tr_x87_to_mmx();
//
//        IR2_OPND fcsr_opnd = set_fpu_fcsr_rounding_field_by_x86();
//        //TODO:simply
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, true);
//
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd2(mips_cvt_s_w, &temp, &src);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 0);
//        append_ir2_opnd2i(mips_shfw, &temp, &src, 0x55);
//        append_ir2_opnd2(mips_cvt_s_w, &temp, &temp);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 1);
//
//        set_fpu_fcsr(&fcsr_opnd);
//        return true;
//    }
//
//    /* use mxcsr */
//    tr_x87_to_mmx();
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src     = load_freg_from_ir1_1(opnd1, false, true);
//
//    IR2_OPND fcsr_opnd  = set_fpu_fcsr_rounding_field_by_x86();
//    IR2_OPND ftemp_src1 = ra_alloc_ftemp();
//
//    append_ir2_opnd2(mips_cvt_s_w, &ftemp_src1, &src);
//
//    /* dest_lo are used temporarily store high 32bit of src */
//    append_ir2_opnd3(mips_fdsrl, &dest_lo, &src, &f32_ir2_opnd);
//
//    /* for we try to use regs as less as possible */
//    append_ir2_opnd2(mips_cvt_s_w, &dest_lo, &dest_lo);
//
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &ftemp_src1, &dest_lo);
//
//    /* reload fscr */
//    set_fpu_fcsr(&fcsr_opnd);
//
//    ra_free_temp(&ftemp_src1);
    return true;
}

bool translate_cvtpi2pd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        tr_x87_to_mmx();
//
//        IR2_OPND fcsr_opnd = set_fpu_fcsr_rounding_field_by_x86();
//        //TODO:simply
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, true);
//
//        IR2_OPND temp  = ra_alloc_ftemp();
//        IR2_OPND temp0 = ra_alloc_ftemp();
//
//        append_ir2_opnd2(mips_cvt_d_w, &temp0, &src);
//        append_ir2_opnd2i(mips_shfw, &temp, &src, 0x55);
//        append_ir2_opnd2(mips_cvt_d_w, &temp, &temp);
//        append_ir2_opnd2i(mips_insved, &dest, &temp, 1);
//        append_ir2_opnd2i(mips_insved, &dest, &temp0, 0);
//
//        set_fpu_fcsr(&fcsr_opnd);
//        return true;
//    }
//
//    tr_x87_to_mmx();
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src     = load_freg_from_ir1_1(opnd1, false, true);
//
//    IR2_OPND ftemp_src1 = ra_alloc_ftemp();
//
//    append_ir2_opnd2(mips_cvt_d_w, &dest_lo, &src);
//    append_ir2_opnd3(mips_fdsrl, &ftemp_src1, &src, &f32_ir2_opnd);
//    append_ir2_opnd2(mips_cvt_d_w, &dest_hi, &ftemp_src1);
//
//    ra_free_temp(&ftemp_src1);
    return true;
}

bool translate_cvtps2dq(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    /* the low 64 bits of src */
//    IR2_OPND dest_lo;
//    IR2_OPND src_lo;
//    /* the high 64 bits of src */
//    IR2_OPND dest_hi;
//    IR2_OPND src_hi;
//
//    IR2_OPND real_dest;
//    IR2_OPND real_src;
//
//    if (option_xmm128map) {
//        real_dest = load_freg128_from_ir1(opnd0);
//        real_src  = load_freg128_from_ir1(opnd1);
//
//        dest_lo = ra_alloc_ftemp();
//        src_lo  = ra_alloc_ftemp();
//        src_hi  = ra_alloc_ftemp();
//        dest_hi = ra_alloc_ftemp();
//
//        append_ir2_opnd2ii(mips_vextrinsd, &src_lo, &real_src, 0, 0);
//        append_ir2_opnd2ii(mips_vextrinsd, &src_hi, &real_src, 0, 1);
//    }
//    else {
//        dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//        dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//        src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//    }
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//    IR2_OPND ftemp_over_flow  = ra_alloc_ftemp();
//    IR2_OPND ftemp_under_flow = ra_alloc_ftemp();
//
//    IR2_OPND label_for_flow1 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_for_flow2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_over      = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high      = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    /* double for 0x7fffffff.499999 */
//    load_imm64_to_ir2(&temp_int, 0x41dfffffffdffffcULL);
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_over_flow);
//
//    /* double for 0x80000000.499999 */
//    load_imm64_to_ir2(&temp_int, 0xc1dfffffffdffffcULL);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_under_flow);
//
//    /* the first single scalar operand */
//    /* is unorder or overflow or under flow? */
//    IR2_OPND ftemp_src1 = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsll, &ftemp_src1, &src_lo, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_fdsrl, &ftemp_src1, &ftemp_src1, &f32_ir2_opnd);
//    append_ir2_opnd2(mips_cvt_d_s, &ftemp_src1, &ftemp_src1);
//
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_d, &ftemp_src1, &ftemp_src1);
//
//    /* is overflow? */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow1);
//    append_ir2_opnd2(mips_c_lt_d, &ftemp_over_flow, &ftemp_src1);
//
//    /* is under flow? */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow1);
//    append_ir2_opnd2(mips_c_lt_d, &ftemp_src1, &ftemp_under_flow);
//
//    /* is normal */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow1);
//    append_ir2_opnd2(mips_cvt_w_d, &ftemp_src1, &ftemp_src1);
//
//    append_ir2_opnd1(mips_b, &label_high);
//
//    append_ir2_opnd1(mips_label, &label_for_flow1);
//    load_imm32_to_ir2(&temp_int, 0x80000000, UNKNOWN_EXTENSION);
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_src1);
//
//    append_ir2_opnd1(mips_label, &label_high);
//    /* the second single scalar operand */
//    /* is unorder or overflow or under flow? */
//    IR2_OPND ftemp_src2 = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &ftemp_src2, &src_lo, &f32_ir2_opnd);
//
//    append_ir2_opnd2(mips_cvt_d_s, &ftemp_src2, &ftemp_src2);
//
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_d, &ftemp_src2, &ftemp_src2);
//
//    /* is overflow? */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow2);
//    append_ir2_opnd2(mips_c_lt_d, &ftemp_over_flow, &ftemp_src2);
//
//    /* is under flow? */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow2);
//    append_ir2_opnd2(mips_c_lt_d, &ftemp_src2, &ftemp_under_flow);
//
//    /* is normal */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow2);
//    append_ir2_opnd2(mips_cvt_w_d, &ftemp_src2, &ftemp_src2);
//
//    append_ir2_opnd1(mips_b, &label_over);
//
//    append_ir2_opnd1(mips_label, &label_for_flow2);
//    load_imm32_to_ir2(&temp_int, 0x80000000, UNKNOWN_EXTENSION);
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_src2);
//
//    append_ir2_opnd1(mips_label, &label_over);
//    /* merge */
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &ftemp_src1, &ftemp_src2);
//
//    IR2_OPND label_for_flow3 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_for_flow4 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_over2     = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high2     = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    /* the first single scalar operand */
//    /* is unorder or overflow or under flow? */
//    append_ir2_opnd3(mips_fdsll, &ftemp_src1, &src_hi, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_fdsrl, &ftemp_src1, &ftemp_src1, &f32_ir2_opnd);
//    append_ir2_opnd2(mips_cvt_d_s, &ftemp_src1, &ftemp_src1);
//
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_d, &ftemp_src1, &ftemp_src1);
//
//    /* is overflow? */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow3);
//    append_ir2_opnd2(mips_c_lt_d, &ftemp_over_flow, &ftemp_src1);
//
//    /* is under flow? */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow3);
//    append_ir2_opnd2(mips_c_lt_d, &ftemp_src1, &ftemp_under_flow);
//
//    /* is normal */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow3);
//    append_ir2_opnd2(mips_cvt_w_d, &ftemp_src1, &ftemp_src1);
//
//    append_ir2_opnd1(mips_b, &label_high2);
//
//    append_ir2_opnd1(mips_label, &label_for_flow3);
//
//    load_imm32_to_ir2(&temp_int, 0x80000000, UNKNOWN_EXTENSION);
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_src1);
//
//    append_ir2_opnd1(mips_label, &label_high2);
//    /* the second single scalar operand */
//    /* is unorder or overflow or under flow? */
//    append_ir2_opnd3(mips_fdsrl, &ftemp_src2, &src_hi, &f32_ir2_opnd);
//    append_ir2_opnd2(mips_cvt_d_s, &ftemp_src2, &ftemp_src2);
//
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_d, &ftemp_src2, &ftemp_src2);
//
//    /* is overflow? */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow4);
//    append_ir2_opnd2(mips_c_lt_d, &ftemp_over_flow, &ftemp_src2);
//
//    /* is under flow? */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow4);
//    append_ir2_opnd2(mips_c_lt_d, &ftemp_src2, &ftemp_under_flow);
//
//    /* is normal */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow4);
//    append_ir2_opnd2(mips_cvt_w_d, &ftemp_src2, &ftemp_src2);
//    append_ir2_opnd1(mips_b, &label_over2);
//
//    append_ir2_opnd1(mips_label, &label_for_flow4);
//
//    load_imm32_to_ir2(&temp_int, 0x80000000, UNKNOWN_EXTENSION);
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_src2);
//
//    append_ir2_opnd1(mips_label, &label_over2);
//
//    /* merge */
//    append_ir2_opnd3(mips_punpcklwd, &dest_hi, &ftemp_src1, &ftemp_src2);
//
//    if (option_xmm128map) {
//        append_ir2_opnd2ii(mips_vextrinsd, &real_dest, &dest_lo, 0, 0);
//        append_ir2_opnd2ii(mips_vextrinsd, &real_dest, &dest_hi, 1, 0);
//    }
//
//    ra_free_temp(&ftemp_src1);
//    ra_free_temp(&ftemp_src2);
//    ra_free_temp(&ftemp_over_flow);
//    ra_free_temp(&ftemp_under_flow);
//    ra_free_temp(&temp_int);
    return true;
}

bool translate_cvtps2pi(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    tr_x87_to_mmx();
//
//    IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo;
//
//    if (option_xmm128map){
//        src_lo = load_freg128_from_ir1(opnd1);
//    } else {
//        src_lo = load_freg_from_ir1_1(opnd1, false, true);
//    }
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//    IR2_OPND ftemp_over_flow  = ra_alloc_ftemp();
//    IR2_OPND ftemp_under_flow = ra_alloc_ftemp();
//
//    IR2_OPND label_for_flow1 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_for_flow2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_over      = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_high      = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    /* double for 0x7fffffff.499999 */
//    load_imm64_to_ir2(&temp_int, 0x41dfffffffdffffcULL);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_over_flow);
//
//    /* double for 0x80000000.499999 */
//    load_imm64_to_ir2(&temp_int, 0xc1dfffffffdffffcULL);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_under_flow);
//
//    /* the first single scalar operand */
//    /* is unorder or overflow or under flow? */
//    IR2_OPND ftemp_src1 = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsll, &ftemp_src1, &src_lo, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_fdsrl, &ftemp_src1, &ftemp_src1, &f32_ir2_opnd);
//    append_ir2_opnd2(mips_cvt_d_s, &ftemp_src1, &ftemp_src1);
//
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_d, &ftemp_src1, &ftemp_src1);
//
//    /* is overflow? */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow1);
//    append_ir2_opnd2(mips_c_lt_d, &ftemp_over_flow, &ftemp_src1);
//
//    /* is under flow? */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow1);
//    append_ir2_opnd2(mips_c_lt_d, &ftemp_src1, &ftemp_under_flow);
//
//    /* is normal */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow1);
//    append_ir2_opnd2(mips_cvt_w_d, &ftemp_src1, &ftemp_src1);
//
//    append_ir2_opnd1(mips_b, &label_high);
//
//    append_ir2_opnd1(mips_label, &label_for_flow1);
//    load_imm32_to_ir2(&temp_int, 0x80000000, UNKNOWN_EXTENSION);
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_src1);
//
//    append_ir2_opnd1(mips_label, &label_high);
//    /* the second single scalar operand */
//    /* is unorder or overflow or under flow? */
//    IR2_OPND ftemp_src2 = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &ftemp_src2, &src_lo, &f32_ir2_opnd);
//
//    append_ir2_opnd2(mips_cvt_d_s, &ftemp_src2, &ftemp_src2);
//
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_d, &ftemp_src2, &ftemp_src2);
//
//    /* is overflow? */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow2);
//    append_ir2_opnd2(mips_c_lt_d, &ftemp_over_flow, &ftemp_src2);
//
//    /* is under flow? */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow2);
//    append_ir2_opnd2(mips_c_lt_d, &ftemp_src2, &ftemp_under_flow);
//
//    /* is normal */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow2);
//    append_ir2_opnd2(mips_cvt_w_d, &ftemp_src2, &ftemp_src2);
//
//    append_ir2_opnd1(mips_b, &label_over);
//
//    append_ir2_opnd1(mips_label, &label_for_flow2);
//    load_imm32_to_ir2(&temp_int, 0x80000000, UNKNOWN_EXTENSION);
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_src2);
//
//    append_ir2_opnd1(mips_label, &label_over);
//
//    /* merge */
//    append_ir2_opnd3(mips_punpcklwd, &dest, &ftemp_src1, &ftemp_src2);
//
//    ra_free_temp(&ftemp_src1);
//    ra_free_temp(&ftemp_src2);
//    ra_free_temp(&ftemp_over_flow);
//    ra_free_temp(&ftemp_under_flow);
//    ra_free_temp(&temp_int);
    return true;
}

bool translate_cvtsd2si(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    IR2_OPND src_lo;
//    if (option_xmm128map){
//        src_lo = load_freg128_from_ir1(opnd1);
//    } else {
//        src_lo = load_freg_from_ir1_1(opnd1, false, true);
//    }
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//    IR2_OPND ftemp_over_flow = ra_alloc_ftemp();
//
//    /* double for 0x7fffffff.499999 */
//    load_imm64_to_ir2(&temp_int, 0x41dfffffffdffffcULL);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_over_flow);
//
//    IR2_OPND ftemp_under_flow = ra_alloc_ftemp();
//
//    /* double for 0x80000000.499999 */
//    load_imm64_to_ir2(&temp_int, 0xc1dfffffffdffffcULL);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_under_flow);
//
//    IR2_OPND label_for_flow = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_d, &src_lo, &src_lo);
//
//    /* is overflow? */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow);
//    append_ir2_opnd2(mips_c_lt_d, &ftemp_over_flow, &src_lo);
//
//    /* is under flow? */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow);
//    append_ir2_opnd2(mips_c_lt_d, &src_lo, &ftemp_under_flow);
//
//    /* is normal */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow);
//    IR2_OPND fcsr_opnd = set_fpu_fcsr_rounding_field_by_x86();
//
//    if (ir1_opnd_size(opnd0) == 64) {
//        append_ir2_opnd2(mips_cvt_l_d, &ftemp_over_flow, &src_lo);
//    } else {
//        append_ir2_opnd2(mips_cvt_w_d, &ftemp_over_flow, &src_lo);
//    }
//
//    /* temp_int is used for saving temp_dest */
//    append_ir2_opnd2(mips_dmfc1, &temp_int, &ftemp_over_flow);
//
//    /* reload fscr */
//    set_fpu_fcsr(&fcsr_opnd);
//
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_for_flow);
//    load_imm32_to_ir2(&temp_int, 0x80000000, UNKNOWN_EXTENSION);
//    append_ir2_opnd1(mips_label, &label_exit);
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//
//    ra_free_temp(&temp_int);
//    ra_free_temp(&ftemp_over_flow);
//    ra_free_temp(&ftemp_under_flow);
    return true;
}

bool translate_cvtsd2ss(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND fcsr_opnd = set_fpu_fcsr_rounding_field_by_x86();
//        //TODO:simply
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src = load_freg128_from_ir1(opnd1);
//
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd2(mips_cvt_s_d, &temp, &src);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 0);
//
//        set_fpu_fcsr(&fcsr_opnd);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src     = load_freg_from_ir1_1(opnd1, false, true);
//
//    IR2_OPND fcsr_opnd = set_fpu_fcsr_rounding_field_by_x86();
//    IR2_OPND temp_src  = ra_alloc_ftemp();
//
//    append_ir2_opnd2(mips_cvt_s_d, &temp_src, &src);
//    set_fpu_fcsr(&fcsr_opnd);
//
//    append_ir2_opnd3(mips_fdsrl, &dest_lo, &dest_lo, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &temp_src, &dest_lo);
//
//    ra_free_temp(&temp_src);
    return true;
}

bool translate_cvtsi2sd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND fcsr_opnd = set_fpu_fcsr_rounding_field_by_x86();
//        //TODO:simply
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = ra_alloc_itemp();
//
//        load_ir1_to_ir2(&src, opnd1, UNKNOWN_EXTENSION, false);
//
//        IR2_OPND temp_src = ra_alloc_ftemp();
//        append_ir2_opnd2(mips_dmtc1, &src, &temp_src);
//
//        if (ir1_opnd_size(opnd1) == 64) {
//            append_ir2_opnd2(mips_cvt_d_l, &temp_src, &temp_src);
//        } else {
//            append_ir2_opnd2(mips_cvt_d_w, &temp_src, &temp_src);
//        }
//
//        append_ir2_opnd2(mips_insved, &dest, &temp_src);
//
//        set_fpu_fcsr(&fcsr_opnd);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src, opnd1, UNKNOWN_EXTENSION, false);
//
//    IR2_OPND temp_src = ra_alloc_ftemp();
//
//    append_ir2_opnd2(mips_dmtc1, &src, &temp_src);
//    if (ir1_opnd_size(opnd1) == 64) {
//        append_ir2_opnd2(mips_cvt_d_l, &dest_lo, &temp_src);
//    } else {
//        append_ir2_opnd2(mips_cvt_d_w, &dest_lo, &temp_src);
//    }
//
//    ra_free_temp(&temp_src);
    return true;
}

bool translate_cvtsi2ss(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND fcsr_opnd = set_fpu_fcsr_rounding_field_by_x86();
//        //TODO:simply
//        //
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src = ra_alloc_itemp();
//
//        load_ir1_to_ir2(&src, opnd1, UNKNOWN_EXTENSION, false);
//
//        IR2_OPND temp_src = ra_alloc_ftemp();
//
//        append_ir2_opnd2(mips_dmtc1, &src, &temp_src);
//        if (ir1_opnd_size(opnd1) == 64) {
//            append_ir2_opnd2(mips_cvt_s_l, &temp_src, &temp_src);
//        } else {
//            append_ir2_opnd2(mips_cvt_s_w, &temp_src, &temp_src);
//        }
//
//        append_ir2_opnd2(mips_insvew, &dest, &temp_src);
//
//        set_fpu_fcsr(&fcsr_opnd);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src, opnd1, UNKNOWN_EXTENSION, false);
//
//    IR2_OPND temp_src = ra_alloc_ftemp();
//
//    append_ir2_opnd2(mips_dmtc1, &src, &temp_src);
//    IR2_OPND fcsr_opnd = set_fpu_fcsr_rounding_field_by_x86();
//
//    if (ir1_opnd_size(opnd1) == 64) {
//        append_ir2_opnd2(mips_cvt_s_l, &temp_src, &temp_src);
//    } else {
//        append_ir2_opnd2(mips_cvt_s_w, &temp_src, &temp_src);
//    }
//
//    set_fpu_fcsr(&fcsr_opnd);
//    append_ir2_opnd3(mips_fdsrl, &dest_lo, &dest_lo, &f32_ir2_opnd);
//
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &temp_src, &dest_lo);
//
//    ra_free_temp(&temp_src);
    return true;
}

bool translate_cvtss2sd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        // TODO:simply
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd2(mips_cvt_d_s, &temp, &src);
//        append_ir2_opnd2i(mips_insved, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo     = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_low_32b = load_freg_from_ir1_1(opnd1, false, false);
//
//    append_ir2_opnd2(mips_cvt_d_s, &dest_lo, &src_low_32b);
//
    return true;
}

bool translate_cvtss2si(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    IR2_OPND src_lo;
//    if (option_xmm128map){
//        src_lo = load_freg128_from_ir1(opnd1);
//    } else {
//        src_lo = load_freg_from_ir1_1(opnd1, false, false);
//    }
//
//    IR2_OPND temp_src = ra_alloc_ftemp();
//    /* only use xmm low 32bit */
//    append_ir2_opnd2(mips_cvt_d_s, &temp_src, &src_lo);
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//    IR2_OPND ftemp_over_flow = ra_alloc_ftemp();
//
//    /* double for 0x7fffffff */
//    load_imm64_to_ir2(&temp_int, 0x41dfffffffe00000ULL);
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_over_flow);
//
//    IR2_OPND ftemp_under_flow = ra_alloc_ftemp();
//
//    /* double for 0x80000000 */
//    load_imm64_to_ir2(&temp_int, 0xc1e0000000000000ULL);
//
//    append_ir2_opnd2(mips_dmtc1, &temp_int, &ftemp_under_flow);
//
//    IR2_OPND label_for_flow = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_d, &temp_src, &temp_src);
//
//    /* is overflow? */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow);
//    append_ir2_opnd2(mips_c_lt_d, &ftemp_over_flow, &temp_src);
//
//    /* is under flow? */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow);
//    append_ir2_opnd2(mips_c_lt_d, &temp_src, &ftemp_under_flow);
//
//    /* is normal ! */
//    append_ir2_opnd1(mips_bc1t, &label_for_flow);
//
//    IR2_OPND fcsr_opnd = set_fpu_fcsr_rounding_field_by_x86();
//    if (ir1_opnd_size(opnd0) == 64) {
//        append_ir2_opnd2(mips_cvt_l_d, &ftemp_over_flow, &temp_src);
//    } else {
//        append_ir2_opnd2(mips_cvt_w_d, &ftemp_over_flow, &temp_src);
//    }
//
//    /* temp_int is used for saving temp_dest */
//    append_ir2_opnd2(mips_dmfc1, &temp_int, &ftemp_over_flow);
//
//    /* reload fscr */
//    set_fpu_fcsr(&fcsr_opnd);
//
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_b, &label_exit);
//    append_ir2_opnd1(mips_label, &label_for_flow);
//    load_imm32_to_ir2(&temp_int, 0x80000000, UNKNOWN_EXTENSION);
//    append_ir2_opnd1(mips_label, &label_exit);
//    store_ir2_to_ir1(&temp_int, opnd0, false);
//
//    ra_free_temp(&temp_int);
//    ra_free_temp(&temp_src);
//    ra_free_temp(&ftemp_over_flow);
//    ra_free_temp(&ftemp_under_flow);
    return true;
}

bool translate_cvttss2si(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    IR2_OPND src_lo;
//    if (option_xmm128map){
//        src_lo = load_freg128_from_ir1(opnd1);
//    } else {
//        src_lo = load_freg_from_ir1_1(opnd1, false, false);
//    }
//
//    IR2_OPND temp_src = ra_alloc_ftemp();
//    /* only use xmm low 32bit */
//    append_ir2_opnd2(mips_cvt_d_s, &temp_src, &src_lo);
//
//    IR2_OPND temp_over_flow  = ra_alloc_itemp();
//    IR2_OPND ftemp_over_flow = ra_alloc_ftemp();
//
//    append_ir2_opnd1i(mips_lui, &temp_over_flow, 0x41e0);
//    append_ir2_opnd2i(mips_dsll32, &temp_over_flow, &temp_over_flow, 0);
//    append_ir2_opnd2(mips_dmtc1, &temp_over_flow, &ftemp_over_flow);
//
//    IR2_OPND temp_under_flow  = ra_alloc_itemp();
//    IR2_OPND ftemp_under_flow = ra_alloc_ftemp();
//
//    append_ir2_opnd1i(mips_lui, &temp_under_flow, 0xc1e);
//    append_ir2_opnd2i(mips_dsll32, &temp_under_flow, &temp_under_flow, 4);
//    append_ir2_opnd2(mips_dmtc1, &temp_under_flow, &ftemp_under_flow);
//
//    IR2_OPND label_for_flow = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd2(mips_c_un_d, &temp_src, &temp_src);
//
//    /*is unoder?*/
//    append_ir2_opnd1(mips_bc1t, &label_for_flow);
//
//    append_ir2_opnd2(mips_c_le_d, &ftemp_over_flow, &temp_src);
//
//    /*is over flow or under flow*/
//    append_ir2_opnd1(mips_bc1t, &label_for_flow);
//
//    append_ir2_opnd2(mips_c_lt_d, &temp_src, &ftemp_under_flow);
//
//    /*not over flow and under flow*/
//    append_ir2_opnd1(mips_bc1t, &label_for_flow);
//
//    IR2_OPND temp_fcsr = ra_alloc_itemp();
//
//    /*save fscr in temp_int for reload*/
//    append_ir2_opnd2(mips_cfc1, &temp_fcsr, &fcsr_ir2_opnd);
//
//    /* temp_under_flow save fcsr */
//    append_ir2_opnd3(mips_or, &temp_under_flow, &temp_under_flow, &temp_fcsr);
//
//    /*set fscr for rounding to zero according to x86 operation*/
//    append_ir2_opnd2i(mips_dsrl, &temp_fcsr, &temp_fcsr, 0x2);
//    append_ir2_opnd2i(mips_dsll, &temp_fcsr, &temp_fcsr, 0x2);
//    append_ir2_opnd2i(mips_ori, &temp_fcsr, &temp_fcsr, 0x1);
//    append_ir2_opnd2(mips_ctc1, &temp_fcsr, &fcsr_ir2_opnd);
//
//    if (ir1_opnd_size(opnd0) == 64) {
//        append_ir2_opnd2(mips_cvt_l_d, &ftemp_over_flow, &temp_src);
//    } else {
//        append_ir2_opnd2(mips_cvt_w_d, &ftemp_over_flow, &temp_src);
//    }
//
//    /* temp_fscr used as temp_dest */
//    append_ir2_opnd2(mips_dmfc1, &temp_fcsr, &ftemp_over_flow);
//
//    /* reload fcsr */
//    append_ir2_opnd2(mips_ctc1, &temp_under_flow, &fcsr_ir2_opnd);
//
//    IR2_OPND label_for_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_b, &label_for_exit);
//    append_ir2_opnd1(mips_label, &label_for_flow);
//    load_imm32_to_ir2(&temp_fcsr, 0x80000000, ZERO_EXTENSION);
//    append_ir2_opnd1(mips_label, &label_for_exit);
//    store_ir2_to_ir1(&temp_fcsr, opnd0, false);
//
//    ra_free_temp(&temp_fcsr);
//    ra_free_temp(&temp_src);
//    ra_free_temp(&temp_over_flow);
//    ra_free_temp(&temp_under_flow);
//    ra_free_temp(&ftemp_over_flow);
//    ra_free_temp(&ftemp_under_flow);
    return true;
}

bool translate_divpd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fdivd, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    append_ir2_opnd3(mips_div_d, &dest_lo, &dest_lo, &src_lo);
//    append_ir2_opnd3(mips_div_d, &dest_hi, &dest_hi, &src_hi);
//
    return true;
}

bool translate_divps(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fdivw, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    IR2_OPND temp_src  = ra_alloc_ftemp();
//
//    /* the low 64 bit of xmm/m128 */
//
//    /* the high 32bit of dest_lo save in temp_dest */
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    /* the high 32bit of src_lo save in temp_src */
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_lo, &f32_ir2_opnd);
//
//    append_ir2_opnd3(mips_div_s, &dest_lo, &dest_lo, &src_lo);
//    append_ir2_opnd3(mips_div_s, &temp_dest, &temp_dest, &temp_src);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    /* the high 64 bit of xmm/m128 */
//
//    /* the high 32bit of dest_hi save in temp_dest */
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_hi, &f32_ir2_opnd);
//    /* the high 32bit of src_hi save in temp_src */
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_hi, &f32_ir2_opnd);
//
//    append_ir2_opnd3(mips_div_s, &dest_hi, &dest_hi, &src_hi);
//    append_ir2_opnd3(mips_div_s, &temp_dest, &temp_dest, &temp_src);
//    append_ir2_opnd3(mips_punpcklwd, &dest_hi, &dest_hi, &temp_dest);
//
//    ra_free_temp(&temp_dest);
//    ra_free_temp(&temp_src);
    return true;
}

bool translate_divsd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src = load_freg128_from_ir1(opnd1);
//
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_div_d, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insved, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_div_d, &dest_lo, &dest_lo, &src_lo);
//
    return true;
}

bool translate_divss(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_div_s, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, false);
//
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_div_s, &dest_lo, &dest_lo, &src_lo);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    ra_free_temp(&temp_dest);
    return true;
}

bool translate_maxpd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fmaxd, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    /* low 64 bit */
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_d, &src_lo, &dest_lo);
//
//    IR2_OPND label_for_mov = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_mov);
//    /* src is less than dest */
//    append_ir2_opnd2(mips_c_lt_d, &src_lo, &dest_lo);
//
//    IR2_OPND label_for_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_exit);
//    append_ir2_opnd1(mips_label, &label_for_mov);
//    append_ir2_opnd2(mips_mov_d, &dest_lo, &src_lo);
//    append_ir2_opnd1(mips_label, &label_for_exit);
//
//    /* high 64 bit */
//    append_ir2_opnd2(mips_c_un_d, &src_hi, &dest_hi);
//
//    IR2_OPND label_for_mov2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    /* is unorder? */
//    append_ir2_opnd1(mips_bc1t, &label_for_mov2);
//
//    /* src is less than dest */
//    append_ir2_opnd2(mips_c_lt_d, &src_hi, &dest_hi);
//
//    IR2_OPND label_for_exit2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_exit2);
//    append_ir2_opnd1(mips_label, &label_for_mov2);
//    append_ir2_opnd2(mips_mov_d, &dest_hi, &src_hi);
//    append_ir2_opnd1(mips_label, &label_for_exit2);
//
    return true;
}

bool translate_maxps(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fmaxw, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    /* the first single scalar operand */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    IR2_OPND temp_src = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_lo, &f32_ir2_opnd);
//
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_s, &src_lo, &dest_lo);
//
//    IR2_OPND label_for_mov = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_mov);
//    /* src is less than dest */
//    append_ir2_opnd2(mips_c_lt_s, &src_lo, &dest_lo);
//
//    IR2_OPND label_for_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_exit);
//    append_ir2_opnd1(mips_label, &label_for_mov);
//    append_ir2_opnd2(mips_mov_d, &dest_lo, &src_lo);
//    append_ir2_opnd1(mips_label, &label_for_exit);
//    /* the second single */
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_s, &temp_src, &temp_dest);
//
//    IR2_OPND label_for_mov2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_mov2);
//    /* src is less than dest */
//    append_ir2_opnd2(mips_c_lt_s, &temp_src, &temp_dest);
//
//    IR2_OPND label_for_exit2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_exit2);
//    append_ir2_opnd1(mips_label, &label_for_mov2);
//    append_ir2_opnd2(mips_mov_d, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_label, &label_for_exit2);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    /* the third single */
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_hi, &f32_ir2_opnd);
//
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_hi, &f32_ir2_opnd);
//
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_s, &src_hi, &dest_hi);
//
//    IR2_OPND label_for_mov3 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_mov3);
//    /* src is less than dest */
//    append_ir2_opnd2(mips_c_lt_s, &src_hi, &dest_hi);
//
//    IR2_OPND label_for_exit3 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_exit3);
//    append_ir2_opnd1(mips_label, &label_for_mov3);
//    append_ir2_opnd2(mips_mov_d, &dest_hi, &src_hi);
//    append_ir2_opnd1(mips_label, &label_for_exit3);
//    /* the fourth single */
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_s, &temp_src, &temp_dest);
//
//    IR2_OPND label_for_mov4 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_mov4);
//    /* src is less than dest */
//    append_ir2_opnd2(mips_c_lt_s, &temp_src, &temp_dest);
//
//    IR2_OPND label_for_exit4 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_exit4);
//    append_ir2_opnd1(mips_label, &label_for_mov4);
//    append_ir2_opnd2(mips_mov_d, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_label, &label_for_exit4);
//    append_ir2_opnd3(mips_punpcklwd, &dest_hi, &dest_hi, &temp_dest);
//
//    ra_free_temp(&temp_dest);
//    ra_free_temp(&temp_src);
    return true;
}

bool translate_maxsd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fmaxd, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insved, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_d, &src_lo, &dest_lo);
//
//    IR2_OPND label_for_mov = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_bc1t, &label_for_mov);
//    /* src is less than dest */
//    append_ir2_opnd2(mips_c_lt_d, &src_lo, &dest_lo);
//
//    IR2_OPND label_for_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_exit);
//    append_ir2_opnd1(mips_label, &label_for_mov);
//    append_ir2_opnd2(mips_mov_d, &dest_lo, &src_lo);
//    append_ir2_opnd1(mips_label, &label_for_exit);

    return true;
}

bool translate_maxss(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fmaxw, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, false);
//
//    /* the first single scalar operand */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_s, &src_lo, &dest_lo);
//
//    IR2_OPND label_for_mov = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_mov);
//    /* src is less than dest */
//    append_ir2_opnd2(mips_c_lt_s, &src_lo, &dest_lo);
//
//    IR2_OPND label_for_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_exit);
//    append_ir2_opnd1(mips_label, &label_for_mov);
//    append_ir2_opnd2(mips_mov_d, &dest_lo, &src_lo);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    append_ir2_opnd1(mips_label, &label_for_exit);
//
//    ra_free_temp(&temp_dest);
    return true;
}

bool translate_minpd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fmind, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    /* low 64 bit */
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_d, &src_lo, &dest_lo);
//
//    IR2_OPND label_for_mov = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_mov);
//    /* src is not less than dest */
//    append_ir2_opnd2(mips_c_lt_d, &dest_lo, &src_lo);
//
//    IR2_OPND label_for_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_exit);
//    append_ir2_opnd1(mips_label, &label_for_mov);
//    append_ir2_opnd2(mips_mov_d, &dest_lo, &src_lo);
//    append_ir2_opnd1(mips_label, &label_for_exit);
//    /* high 64 bit */
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_d, &src_hi, &dest_hi);
//
//    IR2_OPND label_for_mov2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_mov2);
//    /* src is less than dest */
//    append_ir2_opnd2(mips_c_lt_d, &dest_hi, &src_hi);
//
//    IR2_OPND label_for_exit2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_exit2);
//    append_ir2_opnd1(mips_label, &label_for_mov2);
//    append_ir2_opnd2(mips_mov_d, &dest_hi, &src_hi);
//    append_ir2_opnd1(mips_label, &label_for_exit2);

    return true;
}

bool translate_minps(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fminw, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    /* the first single scalar operand */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    IR2_OPND temp_src = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_lo, &f32_ir2_opnd);
//
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_s, &src_lo, &dest_lo);
//
//    IR2_OPND label_for_mov = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_mov);
//    /* src is less than dest */
//    append_ir2_opnd2(mips_c_lt_s, &dest_lo, &src_lo);
//
//    IR2_OPND label_for_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_exit);
//    append_ir2_opnd1(mips_label, &label_for_mov);
//    append_ir2_opnd2(mips_mov_d, &dest_lo, &src_lo);
//    append_ir2_opnd1(mips_label, &label_for_exit);
//    /* the second single */
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_s, &temp_src, &temp_dest);
//
//    IR2_OPND label_for_mov2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_mov2);
//    /* src is less than dest */
//    append_ir2_opnd2(mips_c_lt_s, &temp_dest, &temp_src);
//
//    IR2_OPND label_for_exit2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_exit2);
//    append_ir2_opnd1(mips_label, &label_for_mov2);
//    append_ir2_opnd2(mips_mov_d, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_label, &label_for_exit2);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    /* the third single */
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_hi, &f32_ir2_opnd);
//
//    append_ir2_opnd3(mips_fdsrl, &temp_src, &src_hi, &f32_ir2_opnd);
//
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_s, &src_hi, &dest_hi);
//
//    IR2_OPND label_for_mov3 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_mov3);
//    /* src is less than dest */
//    append_ir2_opnd2(mips_c_lt_s, &dest_hi, &src_hi);
//
//    IR2_OPND label_for_exit3 = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_exit3);
//    append_ir2_opnd1(mips_label, &label_for_mov3);
//    append_ir2_opnd2(mips_mov_d, &dest_hi, &src_hi);
//    append_ir2_opnd1(mips_label, &label_for_exit3);
//    /* the fourth single */
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_s, &temp_src, &temp_dest);
//
//    IR2_OPND label_for_mov4 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_bc1t, &label_for_mov4);
//    /* src is less than dest */
//    append_ir2_opnd2(mips_c_lt_s, &temp_dest, &temp_src);
//
//    IR2_OPND label_for_exit4 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_bc1t, &label_for_exit4);
//    append_ir2_opnd1(mips_label, &label_for_mov4);
//    append_ir2_opnd2(mips_mov_d, &temp_dest, &temp_src);
//
//    append_ir2_opnd1(mips_label, &label_for_exit4);
//    append_ir2_opnd3(mips_punpcklwd, &dest_hi, &dest_hi, &temp_dest);
//
//    ra_free_temp(&temp_dest);
//    ra_free_temp(&temp_src);
    return true;
}

bool translate_minsd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fmind, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insved, &dest, &temp, 0);
//
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_d, &src_lo, &dest_lo);
//
//    IR2_OPND label_for_mov = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_mov);
//    /* src is less than dest */
//    append_ir2_opnd2(mips_c_lt_d, &dest_lo, &src_lo);
//
//    IR2_OPND label_for_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_exit);
//    append_ir2_opnd1(mips_label, &label_for_mov);
//    append_ir2_opnd2(mips_mov_d, &dest_lo, &src_lo);
//    append_ir2_opnd1(mips_label, &label_for_exit);
//
    return true;
}

bool translate_minss(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_fminw, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, false);
//
//    /* the first single scalar operand */
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//
//    /* is unorder? */
//    append_ir2_opnd2(mips_c_un_s, &src_lo, &dest_lo);
//
//    IR2_OPND label_for_mov = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_mov);
//    /* src is less than dest */
//    append_ir2_opnd2(mips_c_lt_s, &dest_lo, &src_lo);
//
//    IR2_OPND label_for_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    append_ir2_opnd1(mips_bc1t, &label_for_exit);
//    append_ir2_opnd1(mips_label, &label_for_mov);
//    append_ir2_opnd2(mips_mov_d, &dest_lo, &src_lo);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    append_ir2_opnd1(mips_label, &label_for_exit);
//
//    ra_free_temp(&temp_dest);
    return true;
}

bool translate_movdq2q(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        IR1_OPND *dest = opnd0;
//        IR1_OPND *src = opnd1;
//
//        lsassert(ir1_opnd_is_xmm(opnd1));
//
//        IR2_OPND xmm_dest = ra_alloc_mmx(ir1_opnd_base_reg_num(dest));
//        IR2_OPND xmm_src  = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
//
//        append_ir2_opnd2(mips_mov_d, &xmm_dest, &xmm_src);
//
//        // TODO:zero fpu top and tag word
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//
//    load_freg_from_ir1_2(&dest_lo, opnd1, false, true);
//
    return true;
}

bool translate_movmskpd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if(option_xmm128map){
//        IR1_OPND *dest = opnd0;
//        IR1_OPND *src  = opnd1;
//        if(ir1_opnd_is_xmm(src)){
//            IR2_OPND temp = ra_alloc_ftemp();
//
//            IR2_OPND xmm_src  = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
//            IR2_OPND gpr_dest = ra_alloc_gpr(ir1_opnd_base_reg_num(dest));
//
//            append_ir2_opnd2(mips_vgetsignd, &temp, &xmm_src);
//            append_ir2_opnd2(mips_dmfc1, &gpr_dest, &temp);
//            return true;
//        }
//    }
//
//    IR2_OPND src_lo = ra_alloc_itemp();
//    IR2_OPND src_hi = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src_lo, opnd1, UNKNOWN_EXTENSION, false);
//    load_ir1_to_ir2(&src_hi, opnd1, UNKNOWN_EXTENSION, true);
//
//    append_ir2_opnd2i(mips_dsrl32, &src_lo, &src_lo, 31);
//    append_ir2_opnd2i(mips_dsrl32, &src_hi, &src_hi, 30);
//    append_ir2_opnd2i(mips_andi, &src_hi, &src_hi, 0x2);
//    append_ir2_opnd3(mips_or, &src_lo, &src_lo, &src_hi);
//
//    ra_free_temp(&src_hi);
//
//    store_ir2_to_ir1(&src_lo, opnd0, false);
    return true;
}

bool translate_movmskps(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if(option_xmm128map){
//        IR1_OPND *dest = opnd0;
//        IR1_OPND *src = opnd1;
//
//        if(ir1_opnd_is_xmm(src)){
//            IR2_OPND temp = ra_alloc_ftemp();
//
//            IR2_OPND xmm_src  = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
//            IR2_OPND gpr_dest = ra_alloc_gpr(ir1_opnd_base_reg_num(dest));
//
//            append_ir2_opnd2(mips_vgetsignw, &temp, &xmm_src);
//            append_ir2_opnd2(mips_dmfc1, &gpr_dest, &temp);
//            return true;
//        }
//    }
//
//    IR2_OPND src_lo = ra_alloc_itemp();
//    IR2_OPND src_hi = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src_lo, opnd1, UNKNOWN_EXTENSION, false);
//    load_ir1_to_ir2(&src_hi, opnd1, UNKNOWN_EXTENSION, true);
//
//    IR2_OPND temp_src = ra_alloc_itemp();
//
//    append_ir2_opnd2i(mips_dsrl, &temp_src, &src_lo, 31);
//    append_ir2_opnd2i(mips_andi, &temp_src, &temp_src, 0x1);
//    append_ir2_opnd2i(mips_dsrl32, &src_lo, &src_lo, 30);
//    append_ir2_opnd2i(mips_andi, &src_lo, &src_lo, 0x2);
//    append_ir2_opnd3(mips_or, &src_lo, &src_lo, &temp_src);
//    append_ir2_opnd2i(mips_dsrl, &temp_src, &src_hi, 29);
//    append_ir2_opnd2i(mips_andi, &temp_src, &temp_src, 0x4);
//    append_ir2_opnd3(mips_or, &src_lo, &src_lo, &temp_src);
//
//    append_ir2_opnd2i(mips_dsrl32, &src_hi, &src_hi, 28);
//    append_ir2_opnd2i(mips_andi, &src_hi, &src_hi, 0x8);
//    append_ir2_opnd3(mips_or, &src_lo, &src_lo, &src_hi);
//
//    ra_free_temp(&src_hi);
//
//    store_ir2_to_ir1(&src_lo, opnd0, false);

    return true;
}

bool translate_movntdq(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        IR1_OPND *dest = opnd0;
//        IR1_OPND *src  = opnd1;
//
//        if (ir1_opnd_is_xmm(src)) {
//            IR2_OPND src_ir2 = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
//            store_freg128_to_ir1_mem(&src_ir2, dest);
//            return true;
//        }
//    }
//    IR2_OPND src_lo = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi = load_freg_from_ir1_1(opnd1, true, true);
//
//    store_freg_to_ir1(&src_lo, opnd0, false, true);
//    store_freg_to_ir1(&src_hi, opnd0, true, true);
//
    return true;
}

bool translate_movnti(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    IR2_OPND src = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src, opnd1, UNKNOWN_EXTENSION, false);
//    store_ir2_to_ir1(&src, opnd0, false);
//
    return true;
}

bool translate_movntpd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        IR1_OPND *dest = opnd0;
//        IR1_OPND *src  = opnd1;
//
//        if (ir1_opnd_is_xmm(src)) {
//            IR2_OPND src_ir2 = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
//            store_freg128_to_ir1_mem(&src_ir2, dest);
//            return true;
//        }
//    }
//
//    IR2_OPND src_lo = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi = load_freg_from_ir1_1(opnd1, true, true);
//
//    store_freg_to_ir1(&src_lo, opnd0, false, true);
//    store_freg_to_ir1(&src_hi, opnd0, true, true);
//
    return true;
}

bool translate_movntps(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        IR1_OPND *dest = opnd0;
//        IR1_OPND *src  = opnd1;
//
//        if (ir1_opnd_is_xmm(src)) {
//            IR2_OPND src_ir2 = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
//            store_freg128_to_ir1_mem(&src_ir2, dest);
//            return true;
//        }
//    }
//
//    IR2_OPND src_lo = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi = load_freg_from_ir1_1(opnd1, true, true);
//
//    store_freg_to_ir1(&src_lo, opnd0, false, true);
//    store_freg_to_ir1(&src_hi, opnd0, true, true);
//
    return true;
}

bool translate_movntq(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        IR1_OPND *dest = opnd0;
//        IR1_OPND *src  = opnd1;
//
//        if (ir1_opnd_is_xmm(src)) {
//            IR2_OPND src_ir2 = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
//            store_freg_to_ir1(&src_ir2, dest, false, false);
//            return true;
//        }
//    }
//
//    IR2_OPND src_lo = load_freg_from_ir1_1(opnd1, false, true);
//    store_freg_to_ir1(&src_lo, opnd0, false, true);

    return true;
}

bool translate_movq2dq(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        IR1_OPND *dest = opnd0;
//        IR1_OPND *src  = opnd1;
//
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND xmm_dest = ra_alloc_xmm(ir1_opnd_base_reg_num(dest));
//        IR2_OPND mmx_src  = ra_alloc_mmx(ir1_opnd_base_reg_num(src));
//        append_ir2_opnd2i(mips_vclrstriv, &xmm_dest, &mmx_src, 7);
//
//        //TODO:zero fpu top and tag word
//        return true;
//    }
//
//    IR2_OPND src = load_freg_from_ir1_1(opnd1, false, true);
//    store_freg_to_ir1(&src, opnd0, false, true);
//    store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);

    return true;
}

bool translate_mulpd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fmuld, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    append_ir2_opnd3(mips_mul_d, &dest_lo, &dest_lo, &src_lo);
//    append_ir2_opnd3(mips_mul_d, &dest_hi, &dest_hi, &src_hi);

    return true;
}

bool translate_mulps(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_fmulw, &dest, &dest, &src);
//        return true;
//    }
//
//    if (1) {
//        IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        append_ir2_opnd3(mips_mul_ps, &dest_lo, &dest_lo, &src_lo);
//        append_ir2_opnd3(mips_mul_ps, &dest_hi, &dest_hi, &src_hi);
//    }
//    else {
//        translate_xxxps_helper(pir1, false, mips_mul_s);
//        translate_xxxps_helper(pir1, true, mips_mul_s);
//    }

    return true;
}

bool translate_mulsd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_mul_d, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insved, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_mul_d, &dest_lo, &dest_lo, &src_lo);

    return true;
}

bool translate_mulss(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_mul_s, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, false);
//
//    IR2_OPND temp_dest = ra_alloc_ftemp();
//
//    append_ir2_opnd3(mips_fdsrl, &temp_dest, &dest_lo, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_mul_s, &dest_lo, &dest_lo, &src_lo);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &temp_dest);
//
//    ra_free_temp(&temp_dest);
    return true;
}

bool translate_orpd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_orv, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    append_ir2_opnd3(mips_for, &dest_lo, &dest_lo, &src_lo);
//    append_ir2_opnd3(mips_for, &dest_hi, &dest_hi, &src_hi);

    return true;
}

bool translate_orps(IR1_INST *pir1)
{
    translate_orpd(pir1);
    return true;
}

bool translate_paddq(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)){
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            append_ir2_opnd3(mips_addvd, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_paddd, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        append_ir2_opnd3(mips_paddd, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_pavgb(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)){
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//            append_ir2_opnd3(mips_aver_ub, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_pavgb, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        append_ir2_opnd3(mips_pavgb, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_pavgw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)){
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//            append_ir2_opnd3(mips_aver_uh, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_pavgh, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        append_ir2_opnd3(mips_pavgh, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_pextrw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//    IR1_OPND *opnd2 = ir1_get_opnd(pir1, 2);
//
//    if (option_xmm128map) {
//        if (ir1_opnd_is_xmm(opnd1)) {
//            uint8_t imm = ir1_opnd_uimm(opnd2);
//            imm &= 7;
//
//            IR2_OPND gpr_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
//            IR2_OPND xmm_opnd = ra_alloc_xmm(ir1_opnd_base_reg_num(opnd1));
//
//            append_ir2_opnd2i(mips_copy_uh, &gpr_opnd, &xmm_opnd, imm);
//            return true;
//        }
//    }
//
//    uint8 imm = ir1_opnd_uimm(opnd2);
//
//    if (ir1_opnd_is_mmx(opnd1)) {
//        IR2_OPND src_lo = ra_alloc_itemp();
//        load_ir1_to_ir2(&src_lo, opnd1, UNKNOWN_EXTENSION, false);
//        uint8 select = imm & 0x3;
//
//        switch (select) {
//        case 0: break;
//        case 1: append_ir2_opnd2i(mips_dsrl, &src_lo, &src_lo, 0x10); break;
//        case 2: append_ir2_opnd2i(mips_dsrl32, &src_lo, &src_lo, 0x0); break;
//        case 3: append_ir2_opnd2i(mips_dsrl32, &src_lo, &src_lo, 0x10); break;
//        default:
//            fprintf(stderr, "1: invalid imm8<0:1> in PEXTRW : %d\n", select);
//            exit(-1);
//        }
//
//        append_ir2_opnd2i(mips_andi, &src_lo, &src_lo, 0xffff);
//        store_ir2_to_ir1(&src_lo, opnd0, false);
//    } else {
//        uint8 select = imm & 0x7;
//
//        if (select <= 3) {
//            IR2_OPND src_lo = ra_alloc_itemp();
//            load_ir1_to_ir2(&src_lo, opnd1, UNKNOWN_EXTENSION, false);
//            switch (select) {
//            case 0: break;
//            case 1: append_ir2_opnd2i(mips_dsrl, &src_lo, &src_lo, 0x10); break;
//            case 2: append_ir2_opnd2i(mips_dsrl32, &src_lo, &src_lo, 0x0); break;
//            case 3: append_ir2_opnd2i(mips_dsrl32, &src_lo, &src_lo, 0x10); break;
//            default:
//                fprintf(stderr, "1: invalid imm8<0:2> in PEXTRW : %d\n", select);
//                exit(-1);
//            }
//            append_ir2_opnd2i(mips_andi, &src_lo, &src_lo, 0xffff);
//            store_ir2_to_ir1(&src_lo, opnd0, false);
//        } else {
//            lsassert(select >= 4 && select <= 7);
//
//            IR2_OPND src_hi = ra_alloc_itemp();
//            load_ir1_to_ir2(&src_hi, opnd1, UNKNOWN_EXTENSION, true);
//            switch (select) {
//            case 4: break;
//            case 5: append_ir2_opnd2i(mips_dsrl, &src_hi, &src_hi, 0x10); break;
//            case 6: append_ir2_opnd2i(mips_dsrl32, &src_hi, &src_hi, 0x0); break;
//            case 7: append_ir2_opnd2i(mips_dsrl32, &src_hi, &src_hi, 0x10); break;
//            default:
//                fprintf(stderr, "1: invalid imm8<0:2> in PEXTRW : %d\n", select);
//                exit(-1);
//            }
//            append_ir2_opnd2i(mips_andi, &src_hi, &src_hi, 0xffff);
//            store_ir2_to_ir1(&src_hi, opnd0, false);
//        }
//    }
//
    return true;
}

bool translate_pinsrw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//    IR1_OPND *opnd2 = ir1_get_opnd(pir1, 2);
//
//    if (option_xmm128map) {
//        if (ir1_opnd_is_xmm(opnd0)) {
//            uint8_t imm = ir1_opnd_uimm(opnd2);
//            imm &= 7;
//
//            IR2_OPND src = ra_alloc_itemp();
//            load_ir1_to_ir2(&src, opnd1, UNKNOWN_EXTENSION, false);
//
//            IR2_OPND xmm_opnd = ra_alloc_xmm(ir1_opnd_base_reg_num(opnd0));
//            append_ir2_opnd2i(mips_inserth, &xmm_opnd, &src, imm);
//            return true;
//        }
//    }
//
//    IR2_OPND src   = ra_alloc_itemp();
//    IR2_OPND ftemp = ra_alloc_ftemp();
//
//    load_ir1_to_ir2(&src, opnd1, UNKNOWN_EXTENSION, false);
//    append_ir2_opnd2(mips_dmtc1, &src, &ftemp);
//
//    uint8 imm8 = ir1_opnd_uimm(opnd2);
//
//    if (ir1_opnd_is_mmx(opnd0)) {
//        IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//        uint8 select = imm8 & 0x3;
//
//        switch (select) {
//        case 0: append_ir2_opnd3(mips_pinsrh0, &dest, &dest, &ftemp); break;
//        case 1: append_ir2_opnd3(mips_pinsrh1, &dest, &dest, &ftemp); break;
//        case 2: append_ir2_opnd3(mips_pinsrh2, &dest, &dest, &ftemp); break;
//        case 3: append_ir2_opnd3(mips_pinsrh3, &dest, &dest, &ftemp); break;
//        default:
//            fprintf(stderr, "1: invalid imm8<0:1> in PINSRW : %d\n", select);
//            exit(-1);
//        }
//    } else {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        uint8 select = imm8 & 0x7;
//        if (select <= 3) {
//            IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//
//            switch (select) {
//            case 0: append_ir2_opnd3(mips_pinsrh0, &dest_lo, &dest_lo, &ftemp); break;
//            case 1: append_ir2_opnd3(mips_pinsrh1, &dest_lo, &dest_lo, &ftemp); break;
//            case 2: append_ir2_opnd3(mips_pinsrh2, &dest_lo, &dest_lo, &ftemp); break;
//            case 3: append_ir2_opnd3(mips_pinsrh3, &dest_lo, &dest_lo, &ftemp); break;
//            default:
//                fprintf(stderr, "1: invalid imm8<0:2> in PINSRW : %d\n", select);
//                exit(-1);
//            }
//        } else {
//            lsassert(select >= 4 && select <= 7);
//
//            IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//            switch (select) {
//            case 4: append_ir2_opnd3(mips_pinsrh0, &dest_hi, &dest_hi, &ftemp); break;
//            case 5: append_ir2_opnd3(mips_pinsrh1, &dest_hi, &dest_hi, &ftemp); break;
//            case 6: append_ir2_opnd3(mips_pinsrh2, &dest_hi, &dest_hi, &ftemp); break;
//            case 7: append_ir2_opnd3(mips_pinsrh3, &dest_hi, &dest_hi, &ftemp); break;
//            default:
//                fprintf(stderr, "1: invalid imm8<0:2> in PINSRW : %d\n", select);
//                exit(-1);
//            }
//        }
//    }
//
//    ra_free_temp(&ftemp);
    return true;
}

bool translate_pmaxsw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)){
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src = load_freg128_from_ir1(opnd1);
//
//            append_ir2_opnd3(mips_max_sh, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo = load_freg_from_ir1_1(opnd1, false, true);
//
//    if (ir2_opnd_cmp(&dest_lo, &src_lo)) {
//        return true;
//    } else {
//        append_ir2_opnd3(mips_pmaxsh, &dest_lo, &dest_lo, &src_lo);
//    }
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi = load_freg_from_ir1_1(opnd1, true, true);
//
//        if (ir2_opnd_cmp(&dest_hi, &src_hi)) {
//            return true;
//        } else {
//            append_ir2_opnd3(mips_pmaxsh, &dest_hi, &dest_hi, &src_hi);
//        }
//    }
//
    return true;
}

bool translate_pmaxub(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)){
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//            append_ir2_opnd3(mips_max_ub, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    if (ir2_opnd_cmp(&dest_lo, &src_lo)) {
//        return true;
//    } else {
//        append_ir2_opnd3(mips_pmaxub, &dest_lo, &dest_lo, &src_lo);
//    }
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        if (ir2_opnd_cmp(&dest_hi, &src_hi)) {
//            return true;
//        } else {
//            append_ir2_opnd3(mips_pmaxub, &dest_hi, &dest_hi, &src_hi);
//        }
//    }
//
    return true;
}

bool translate_pminsw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)){
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//            append_ir2_opnd3(mips_min_sh, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    if (ir2_opnd_cmp(&dest_lo, &src_lo)) {
//        return true;
//    } else {
//        append_ir2_opnd3(mips_pminsh, &dest_lo, &dest_lo, &src_lo);
//    }
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        if (ir2_opnd_cmp(&dest_hi, &src_hi)) {
//            return true;
//        } else {
//            append_ir2_opnd3(mips_pminsh, &dest_hi, &dest_hi, &src_hi);
//        }
//    }

    return true;
}

bool translate_pminub(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)){
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//            append_ir2_opnd3(mips_min_ub, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    if (ir2_opnd_cmp(&dest_lo, &src_lo)) {
//        return true;
//    } else {
//        append_ir2_opnd3(mips_pminub, &dest_lo, &dest_lo, &src_lo);
//    }
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        if (ir2_opnd_cmp(&dest_hi, &src_hi)) {
//            return true;
//        } else {
//            append_ir2_opnd3(mips_pminub, &dest_hi, &dest_hi, &src_hi);
//        }
//    }

    return true;
}

bool translate_pmovmskb(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR1_OPND *dest = opnd0;
    IR1_OPND *src  = opnd1;

    if(ir1_opnd_is_xmm(src)){
        IR2_OPND temp = ra_alloc_ftemp();
        IR2_OPND xmm_src  = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
        IR2_OPND gpr_dest = ra_alloc_gpr(ir1_opnd_base_reg_num(dest));

        append_ir2_opnd2(LISA_VMSKLTZ_B,  &temp, &xmm_src);
        append_ir2_opnd2(LISA_MOVFR2GR_D, &gpr_dest, &temp);
        return true;
    }

//    IR2_OPND src_lo = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND ftemp  = ra_alloc_ftemp();
//
//    if (ir1_opnd_is_mmx(opnd1)) {
//        append_ir2_opnd2(mips_pmovmskb, &ftemp, &src_lo);
//    }
//    else {
//        lsassert(ir1_opnd_is_xmm(opnd1));
//
//        IR2_OPND src_hi = load_freg_from_ir1_1(opnd1, true, true);
//        IR2_OPND ftemp2 = ra_alloc_ftemp();
//
//        append_ir2_opnd2(mips_pmovmskb, &ftemp, &src_lo);
//        append_ir2_opnd2(mips_pmovmskb, &ftemp2, &src_hi);
//        append_ir2_opnd3(mips_punpcklbh, &ftemp, &ftemp, &ftemp2);
//
//        ra_free_temp(&ftemp2);
//    }

    IR2_OPND ftemp = ra_alloc_ftemp();
    IR2_OPND itemp = ra_alloc_itemp();
    IR2_OPND src_opnd = ra_alloc_mmx(ir1_opnd_base_reg_num(src));

    append_ir2_opnd2(LISA_VMSKLTZ_B, &ftemp, &src_opnd);

    append_ir2_opnd2(LISA_MOVFR2GR_D, &itemp, &ftemp);
    append_ir2_opnd2i(LISA_ANDI, &itemp, &itemp, 0xff);
    store_ir2_to_ir1(&itemp, dest, false);

    ra_free_temp(&itemp);
    ra_free_temp(&ftemp);
    return true;
}

bool translate_pmuludq(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)){
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            IR2_OPND temp = ra_alloc_ftemp();
//
//            append_ir2_opnd3(mips_ilvevw, &temp, &src, &src);
//            append_ir2_opnd3(mips_vmulx_ud, &dest, &temp, &dest);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_pmuluw, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        append_ir2_opnd3(mips_pmuluw, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_psadbw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)){
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//            IR2_OPND temp = ra_alloc_ftemp();
//
//            append_ir2_opnd3(mips_asub_ub, &temp, &dest, &src);
//            append_ir2_opnd2(mips_vacc8b_ud, &dest, &temp);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_pasubub, &dest_lo, &dest_lo, &src_lo);
//    append_ir2_opnd2(mips_biadd, &dest_lo, &dest_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        append_ir2_opnd3(mips_pasubub, &dest_hi, &dest_hi, &src_hi);
//        append_ir2_opnd2(mips_biadd, &dest_hi, &dest_hi);
//    }

    return true;
}

bool translate_pshufd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR1_OPND *opnd2 = ir1_get_opnd(pir1, 2);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = load_freg128_from_ir1(opnd0);
    IR2_OPND src  = load_freg128_from_ir1(opnd1);
    uint64_t imm8 = ir1_opnd_uimm(opnd2);

    append_ir2_opnd2i(LISA_VSHUF4I_W, &dest, &src, imm8);
    //append_ir2_opnd2i(mips_shfw, &dest, &src, imm8);

    return true;
}

bool translate_pshufw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//    IR1_OPND *opnd2 = ir1_get_opnd(pir1, 2);
//
//    IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src  = load_freg_from_ir1_1(opnd1, true, true);
//    IR2_OPND sel  = ra_alloc_itemp();
//
//    IR2_OPND ftemp_sel = ra_alloc_ftemp();
//
//    load_ir1_to_ir2(&sel, opnd2, UNKNOWN_EXTENSION, false);
//
//    append_ir2_opnd2(mips_dmtc1, &sel, &ftemp_sel);
//    append_ir2_opnd3(mips_pshufh, &dest, &src, &ftemp_sel);

    return true;
}

bool translate_pshufhw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//    IR1_OPND *opnd2 = ir1_get_opnd(pir1, 2);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        uint64_t imm8 = ir1_opnd_uimm(opnd2);
//
//        if (ir1_opnd_is_mem(opnd1) ||
//            (ir1_opnd_base_reg_num(opnd0) !=
//             ir1_opnd_base_reg_num(opnd1)))
//        {
//            append_ir2_opnd2(mips_movev, &dest, &src);
//        }
//
//        append_ir2_opnd2i(mips_vshufilh, &dest, &dest, imm8);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    load_freg_from_ir1_2(&dest_lo, opnd1, false, true);
//
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_hi = load_freg_from_ir1_1(opnd1, true, true);
//
//    IR2_OPND sel = ra_alloc_itemp();
//    load_ir1_to_ir2(&sel, opnd2, UNKNOWN_EXTENSION, false);
//
//    IR2_OPND ftemp_sel = ra_alloc_ftemp();
//    append_ir2_opnd2(mips_dmtc1, &sel, &ftemp_sel);
//    append_ir2_opnd3(mips_pshufh, &dest_hi, &src_hi, &ftemp_sel);
//
//    ra_free_temp(&ftemp_sel);
    return true;
}

bool translate_pshuflw(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//    IR1_OPND *opnd2 = ir1_get_opnd(pir1, 2);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        uint64_t imm8 = ir1_opnd_uimm(opnd2);
//
//        if (ir1_opnd_is_mem(opnd1) ||
//            (ir1_opnd_base_reg_num(opnd0) !=
//             ir1_opnd_base_reg_num(opnd1)))
//        {
//            append_ir2_opnd2(mips_movev, &dest, &src);
//        }
//
//        append_ir2_opnd2i(mips_vshufirh, &dest, &dest, imm8);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//    load_freg_from_ir1_2(&dest_hi, opnd1, true, true);
//
//    IR2_OPND sel = ra_alloc_itemp();
//    load_ir1_to_ir2(&sel, opnd2, UNKNOWN_EXTENSION, false);
//
//    IR2_OPND ftemp_sel = ra_alloc_ftemp();
//    append_ir2_opnd2(mips_dmtc1, &sel, &ftemp_sel);
//    append_ir2_opnd3(mips_pshufh, &dest_lo, &src_lo, &ftemp_sel);
//
//    ra_free_temp(&ftemp_sel);
    return true;
}

bool translate_pslldq(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        uint8_t imm8 = ir1_opnd_uimm(opnd1);
//
//        if(imm8 > 15) {
//            append_ir2_opnd3(mips_xorv, &dest, &dest, &dest);
//        }
//        else if(imm8 == 0) {
//            return true;
//        }
//        else {
//            IR2_OPND zero = ra_alloc_ftemp();
//            append_ir2_opnd3(mips_xorv, &zero, &zero, &zero);
//            append_ir2_opnd2i(mips_sldib, &dest, &zero, 16 - imm8);
//        }
//
//        return true;
//    }
//
//    IR2_OPND dest_lo = ra_alloc_itemp();
//    IR2_OPND dest_hi = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&dest_lo, opnd0, UNKNOWN_EXTENSION, false);
//    load_ir1_to_ir2(&dest_hi, opnd0, UNKNOWN_EXTENSION, true);
//
//    uint8 imm8 = ir1_opnd_uimm(opnd1);
//    if (imm8 > 15) {
//        store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//        store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    }
//    else if (imm8 > 8) {
//        imm8 -= 8;
//        imm8 *= 8;
//        if (imm8 < 32) {
//            append_ir2_opnd2i(mips_dsll, &dest_lo, &dest_lo, imm8);
//        } else {
//            imm8 -= 32;
//            append_ir2_opnd2i(mips_dsll32, &dest_lo, &dest_lo, imm8);
//        }
//
//        store_ir2_to_ir1(&dest_lo, opnd0, true);
//        store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//    }
//    else if (imm8 == 8) {
//        store_ir2_to_ir1(&dest_lo, opnd0, true);
//        store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//    }
//    else if (imm8 == 0) {
//        return true;
//    }
//    else { /* imm8 > 0 && imm8 < 8 */
//        IR2_OPND itemp = ra_alloc_itemp();
//        if (imm8 < 4) {
//            uint8 imm = 4 - imm8;
//            imm8 *= 8;
//            imm *= 8;
//            append_ir2_opnd2i(mips_dsrl32, &itemp, &dest_lo, imm);
//            append_ir2_opnd2i(mips_dsll, &dest_lo, &dest_lo, imm8);
//            append_ir2_opnd2i(mips_dsll, &dest_hi, &dest_hi, imm8);
//        }
//        else if(imm8 == 4){
//            append_ir2_opnd2i(mips_dsrl32, &itemp, &dest_lo, 0);
//            append_ir2_opnd2i(mips_dsll32, &dest_lo, &dest_lo, 0);
//            append_ir2_opnd2i(mips_dsll32, &dest_hi, &dest_hi, 0);
//        }
//        else {                     /* imm8 > 4 && imm8 < 8 */
//            uint8 imm = 8 - imm8;
//            imm8 -= 4;
//            imm8 *= 8;
//            imm *= 8;
//            append_ir2_opnd2i(mips_dsrl, &itemp, &dest_lo, imm);
//            append_ir2_opnd2i(mips_dsll32, &dest_lo, &dest_lo, imm8);
//            append_ir2_opnd2i(mips_dsll32, &dest_hi, &dest_hi, imm8);
//        }
//        append_ir2_opnd3(mips_or, &dest_hi, &dest_hi, &itemp);
//
//        store_ir2_to_ir1(&dest_lo, opnd0, false);
//        store_ir2_to_ir1(&dest_hi, opnd0, true);
//
//        ra_free_temp(&itemp);
//    }
    return true;
}

bool translate_psrldq(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//
//        uint8_t imm8 = ir1_opnd_uimm(opnd1);
//        if(imm8 > 15){
//            append_ir2_opnd3(mips_xorv, &dest, &dest, &dest);
//        }
//        else if(imm8 == 0){
//            return true;
//        } else {
//            append_ir2_opnd2i(mips_sldib, &dest, &dest, imm8);
//            append_ir2_opnd2i(mips_vclrstriv, &dest, &dest, 16 - imm8);
//        }
//
//        return true;
//    }
//
//    IR2_OPND dest_lo = ra_alloc_itemp();
//    IR2_OPND dest_hi = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&dest_lo, opnd0, UNKNOWN_EXTENSION, false);
//    load_ir1_to_ir2(&dest_hi, opnd0, UNKNOWN_EXTENSION, true);
//
//    uint8 imm8 = ir1_opnd_uimm(opnd1);
//    if (imm8 > 15) {
//        store_ir2_to_ir1(&zero_ir2_opnd, opnd0, false);
//        store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    }
//    else if (imm8 > 8) {
//        imm8 -= 8;
//        imm8 *= 8;
//        if (imm8 < 32) {
//            append_ir2_opnd2i(mips_dsrl, &dest_hi, &dest_hi, imm8);
//        }
//        else {
//            imm8 -= 32;
//            append_ir2_opnd2i(mips_dsrl32, &dest_hi, &dest_hi, imm8);
//        }
//
//        store_ir2_to_ir1(&dest_hi, opnd0, false);
//        store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    }
//    else if (imm8 == 8) {
//        store_ir2_to_ir1(&dest_hi, opnd0, false);
//        store_ir2_to_ir1(&zero_ir2_opnd, opnd0, true);
//    }
//    else if (imm8 == 0) {
//        return true;
//    }
//    else { /* imm8 > 0 && imm8 < 8 */
//        IR2_OPND itemp = ra_alloc_itemp();
//
//        if (imm8 < 4) {
//            uint8 imm = 4 - imm8;
//            imm8 *= 8;
//            imm *= 8;
//            append_ir2_opnd2i(mips_dsll32, &itemp, &dest_hi, imm);
//            append_ir2_opnd2i(mips_dsrl, &dest_hi, &dest_hi, imm8);
//            append_ir2_opnd2i(mips_dsrl, &dest_lo, &dest_lo, imm8);
//        }
//        else if (imm8 > 4) {       /* imm8 > 4 && imm8 < 8 */
//            uint8 imm = 8 - imm8;
//            imm8 -= 4;
//            imm8 *= 8;
//            imm *= 8;
//            append_ir2_opnd2i(mips_dsll, &itemp, &dest_hi, imm);
//            append_ir2_opnd2i(mips_dsrl32, &dest_hi, &dest_hi, imm8);
//            append_ir2_opnd2i(mips_dsrl32, &dest_lo, &dest_lo, imm8);
//        }
//        else if (imm8 == 4) {
//            append_ir2_opnd2i(mips_dsll32, &itemp, &dest_hi, 0);
//            append_ir2_opnd2i(mips_dsrl32, &dest_hi, &dest_hi, 0);
//            append_ir2_opnd2i(mips_dsrl32, &dest_lo, &dest_lo, 0);
//        }
//
//        append_ir2_opnd3(mips_or, &dest_lo, &dest_lo, &itemp);
//
//        store_ir2_to_ir1(&dest_lo, opnd0, false);
//        store_ir2_to_ir1(&dest_hi, opnd0, true);
//
//        ra_free_temp(&itemp);
//    }

    return true;
}

bool translate_psubq(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        if(ir1_opnd_is_xmm(opnd0)){
//            IR2_OPND dest = load_freg128_from_ir1(opnd0);
//            IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//            append_ir2_opnd3(mips_subvd, &dest, &dest, &src);
//            return true;
//        }
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd3(mips_psubd, &dest_lo, &dest_lo, &src_lo);
//
//    if (ir1_opnd_is_xmm(opnd0)) {
//        IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//        IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//        append_ir2_opnd3(mips_psubd, &dest_hi, &dest_hi, &src_hi);
//    }

    return true;
}

bool translate_punpckhqdq(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd3(mips_ilvld, &dest, &src, &dest);
//        return true;
//    }
//
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//
//    store_freg_to_ir1(&dest_hi, opnd0, false, true);
//    load_freg_from_ir1_2(&dest_hi, opnd1, true, true);

    return true;
}

bool translate_rcpss(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//        append_ir2_opnd2(mips_frcpw, &temp, &src);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo = load_freg_from_ir1_1(opnd1, false, false);
//
//    /* load single value of 1.0 */
//    IR2_OPND itemp = ra_alloc_itemp();
//    IR2_OPND ftemp = ra_alloc_ftemp();
//
//    load_imm32_to_ir2(&itemp, 0x3f800000, UNKNOWN_EXTENSION);
//    append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//    /* save high 32 bits of dest_lo */
//    IR2_OPND ftemp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &ftemp_dest, &dest_lo, &f32_ir2_opnd);
//
//    /* compute */
//    append_ir2_opnd3(mips_div_s, &dest_lo, &ftemp, &src_lo);
//
//    /* merge */
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &ftemp_dest);
//
//    ra_free_temp(&itemp);
//    ra_free_temp(&ftemp);
//    ra_free_temp(&ftemp_dest);
    return true;
}

bool translate_rcpps(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src = load_freg128_from_ir1(opnd1);
//        append_ir2_opnd2(mips_frcpw, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    /* load single value of 1.0 */
//    IR2_OPND itemp = ra_alloc_itemp();
//    IR2_OPND ftemp = ra_alloc_ftemp();
//
//    load_imm32_to_ir2(&itemp, 0x3f800000, UNKNOWN_EXTENSION);
//    append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//    /* save high 32 bits of src_lo */
//    IR2_OPND ftemp_src = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &ftemp_src, &src_lo, &f32_ir2_opnd);
//
//    /* compute */
//    append_ir2_opnd3(mips_div_s, &dest_lo, &ftemp, &src_lo);
//    append_ir2_opnd3(mips_div_s, &ftemp_src, &ftemp, &ftemp_src);
//
//    /* merge */
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &ftemp_src);
//
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    /* save high 32 bits of src_hi */
//    append_ir2_opnd3(mips_fdsrl, &ftemp_src, &src_hi, &f32_ir2_opnd);
//
//    /* compute */
//    append_ir2_opnd3(mips_div_s, &dest_hi, &ftemp, &src_hi);
//    append_ir2_opnd3(mips_div_s, &ftemp_src, &ftemp, &ftemp_src);
//
//    /* merge */
//    append_ir2_opnd3(mips_punpcklwd, &dest_hi, &dest_hi, &ftemp_src);
//
//    ra_free_temp(&itemp);
//    ra_free_temp(&ftemp);
//    ra_free_temp(&ftemp_src);
    return true;
}

bool translate_rsqrtss(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src = load_freg128_from_ir1(opnd1);
//        IR2_OPND temp = ra_alloc_ftemp();
//        append_ir2_opnd2(mips_rsqrt_s, &temp, &src);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, false);
//
//    /* load single value of 1.0 */
//    IR2_OPND itemp = ra_alloc_itemp();
//    IR2_OPND ftemp = ra_alloc_ftemp();
//    load_imm32_to_ir2(&itemp, 0x3f800000, UNKNOWN_EXTENSION);
//    append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//    /* save high 32 bits of dest_lo */
//    IR2_OPND ftemp_dest = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &ftemp_dest, &dest_lo, &f32_ir2_opnd);
//
//    /* compute */
//    append_ir2_opnd2(mips_sqrt_s, &dest_lo, &src_lo);
//    append_ir2_opnd3(mips_div_s, &dest_lo, &ftemp, &dest_lo);
//
//    /* merge */
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &ftemp_dest);
//
//    ra_free_temp(&itemp);
//    ra_free_temp(&ftemp);
//    ra_free_temp(&ftemp_dest);
    return true;
}

bool translate_rsqrtps(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src = load_freg128_from_ir1(opnd1);
//        append_ir2_opnd2(mips_frsqrtw, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    /* load single value of 1.0 */
//    IR2_OPND itemp = ra_alloc_itemp();
//    IR2_OPND ftemp = ra_alloc_ftemp();
//
//    load_imm32_to_ir2(&itemp, 0x3f800000, UNKNOWN_EXTENSION);
//    append_ir2_opnd2(mips_dmtc1, &itemp, &ftemp);
//    /* save high 32 bits of src_lo */
//    IR2_OPND ftemp_src = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsrl, &ftemp_src, &src_lo, &f32_ir2_opnd);
//
//    /* compute low 32 bits of src_lo */
//    append_ir2_opnd2(mips_sqrt_s, &dest_lo, &src_lo);
//    append_ir2_opnd3(mips_div_s, &dest_lo, &ftemp, &dest_lo);
//
//    /* compute high 32 bits of src_lo */
//    append_ir2_opnd2(mips_sqrt_s, &ftemp_src, &ftemp_src);
//    append_ir2_opnd3(mips_div_s, &ftemp_src, &ftemp, &ftemp_src);
//
//    /* merge */
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &ftemp_src);
//
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    /* save high 32 bits of src_hi */
//    append_ir2_opnd3(mips_fdsrl, &ftemp_src, &src_hi, &f32_ir2_opnd);
//
//    /* compute low 32 bits of src_hi */
//    append_ir2_opnd2(mips_sqrt_s, &dest_hi, &src_hi);
//    append_ir2_opnd3(mips_div_s, &dest_hi, &ftemp, &dest_hi);
//
//    /* compute high 32 bits of src_lo */
//    append_ir2_opnd2(mips_sqrt_s, &ftemp_src, &ftemp_src);
//    append_ir2_opnd3(mips_div_s, &ftemp_src, &ftemp, &ftemp_src);
//
//    /* merge */
//    append_ir2_opnd3(mips_punpcklwd, &dest_hi, &dest_hi, &ftemp_src);
//
//    ra_free_temp(&itemp);
//    ra_free_temp(&ftemp);
//    ra_free_temp(&ftemp_src);
    return true;
}

bool translate_sqrtpd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd2(mips_fsqrtd, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    append_ir2_opnd2(mips_sqrt_d, &dest_lo, &src_lo);
//
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    append_ir2_opnd2(mips_sqrt_d, &dest_hi, &src_hi);

    return true;
}

bool translate_sqrtps(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(opnd0));
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        append_ir2_opnd2(mips_fsqrtw, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dest_lo = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src_lo  = load_freg_from_ir1_1(opnd1, false, true);
//
//    IR2_OPND ftemp = ra_alloc_ftemp();
//
//    append_ir2_opnd3(mips_fdsrl, &ftemp, &src_lo, &f32_ir2_opnd);
//    append_ir2_opnd2(mips_sqrt_s, &dest_lo, &src_lo);
//    append_ir2_opnd2(mips_sqrt_s, &ftemp, &ftemp);
//    append_ir2_opnd3(mips_punpcklwd, &dest_lo, &dest_lo, &ftemp);
//
//    IR2_OPND dest_hi = load_freg_from_ir1_1(opnd0, true, true);
//    IR2_OPND src_hi  = load_freg_from_ir1_1(opnd1, true, true);
//
//    append_ir2_opnd3(mips_fdsrl, &ftemp, &src_hi, &f32_ir2_opnd);
//    append_ir2_opnd2(mips_sqrt_s, &dest_hi, &src_hi);
//    append_ir2_opnd2(mips_sqrt_s, &ftemp, &ftemp);
//    append_ir2_opnd3(mips_punpcklwd, &dest_hi, &dest_hi, &ftemp);
//
//    ra_free_temp(&ftemp);
    return true;
}

bool translate_ucomisd(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map){
//        lsassert(ir1_opnd_num(pir1) == 2);
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        IR2_OPND temp;
//
//        IR2_OPND label_nun  = ir2_opnd_new_type(IR2_OPND_LABEL);
//        IR2_OPND label_neq  = ir2_opnd_new_type(IR2_OPND_LABEL);
//        IR2_OPND label_gt   = ir2_opnd_new_type(IR2_OPND_LABEL);
//        IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//        if (option_lbt) {
//            temp = ra_alloc_itemp();
//            append_ir2_opnd2i(mips_ori, &temp, &zero_ir2_opnd, 0xfff);
//        }
//
//        /*  case 1: are they unordered? */
//        append_ir2_opnd2(mips_c_un_d, &dest, &src);
//        append_ir2_opnd1(mips_bc1t, &label_nun);
//        /* at least one of the operands is NaN */
//        // set zf,pf,cf = 111
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &temp, 0xb);
//        } else {
//            append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0045);
//        }
//        append_ir2_opnd1(mips_b, &label_exit);
//
//        /* case 2: not unordered. are they equal? */
//        append_ir2_opnd1(mips_label, &label_nun);
//        append_ir2_opnd2(mips_c_eq_d, &dest, &src);
//        append_ir2_opnd1(mips_bc1t, &label_neq);
//        /* two operands are equal */
//        // set zf,pf,cf = 100
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &temp, 0x8);
//            append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0x3);
//        } else {
//            append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0040);
//            /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xfffa);           */
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, PF_BIT | CF_BIT);
//        }
//        append_ir2_opnd1(mips_b, &label_exit);
//
//        /* case 3: not unordered, not equal. less than? */
//        append_ir2_opnd1(mips_label, &label_neq);
//        append_ir2_opnd2(mips_c_lt_d, &dest, &src);
//        append_ir2_opnd1(mips_bc1t, &label_gt);
//        /* less than */
//        // set zf.pf.cf = 001
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0xa);
//            append_ir2_opnd1i(mips_mtflag, &temp, 0x1);
//        } else {
//            append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0001);
//            /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xffbb);           */
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, ZF_BIT | PF_BIT);
//        }
//        append_ir2_opnd1(mips_b, &label_exit);
//
//        /* not unordered, not equal, not less than, so it's greater than */
//        //set zf,pf,cf = 000
//        append_ir2_opnd1(mips_label, &label_gt);
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0xb);
//        } else {
//            /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xffba);*/
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, ZF_BIT | PF_BIT | CF_BIT);
//        }
//
//        /* exit */
//        //set of,sf,af = 000
//        append_ir2_opnd1(mips_label, &label_exit);
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0x34);
//            ra_free_temp(&temp);
//        } else {
//            /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xf76f);*/
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT | SF_BIT | AF_BIT);
//        }
//
//        return true;
//    }
//
//    lsassert(ir1_opnd_num(pir1) == 2);
//
//    IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, true);
//    IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, true);
//
//    IR2_OPND temp;
//
//    IR2_OPND label_nun  = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_neq  = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_gt   = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    if (option_lbt) {
//        temp = ra_alloc_itemp();
//        append_ir2_opnd2i(mips_ori, &temp, &zero_ir2_opnd, 0xfff);
//    }
//
//    /*  case 1: are they unordered? */
//    append_ir2_opnd2(mips_c_un_d, &dest, &src);
//    append_ir2_opnd1(mips_bc1t, &label_nun);
//    /* at least one of the operands is NaN */
//    // set zf,pf,cf = 111
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &temp, 0xb);
//    } else {
//        append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0045);
//    }
//    append_ir2_opnd1(mips_b, &label_exit);
//
//    /* case 2: not unordered. are they equal? */
//    append_ir2_opnd1(mips_label, &label_nun);
//    append_ir2_opnd2(mips_c_eq_d, &dest, &src);
//    append_ir2_opnd1(mips_bc1t, &label_neq);
//    /* two operands are equal */
//    // set zf,pf,cf = 100
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &temp, 0x8);
//        append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0x3);
//    } else {
//        append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0040);
//        /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xfffa);           */
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, PF_BIT | CF_BIT);
//    }
//    append_ir2_opnd1(mips_b, &label_exit);
//
//    /* case 3: not unordered, not equal. less than? */
//    append_ir2_opnd1(mips_label, &label_neq);
//    append_ir2_opnd2(mips_c_lt_d, &dest, &src);
//    append_ir2_opnd1(mips_bc1t, &label_gt);
//    /* less than */
//    // set zf.pf.cf = 001
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0xa);
//        append_ir2_opnd1i(mips_mtflag, &temp, 0x1);
//    } else {
//        append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0001);
//        /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xffbb);           */
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, ZF_BIT | PF_BIT);
//    }
//    append_ir2_opnd1(mips_b, &label_exit);
//
//    /* not unordered, not equal, not less than, so it's greater than */
//    //set zf,pf,cf = 000
//    append_ir2_opnd1(mips_label, &label_gt);
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0xb);
//    } else {
//        /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xffba);*/
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, ZF_BIT | PF_BIT | CF_BIT);
//    }
//
//    /* exit */
//    //set of,sf,af = 000
//    append_ir2_opnd1(mips_label, &label_exit);
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0x34);
//        ra_free_temp(&temp);
//    } else {
//        /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xf76f);*/
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT | SF_BIT | AF_BIT);
//    }
//
    return true;
}

bool translate_ucomiss(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map){
//        lsassert(ir1_opnd_num(pir1) == 2);
//
//        IR2_OPND dest = load_freg128_from_ir1(opnd0);
//        IR2_OPND src  = load_freg128_from_ir1(opnd1);
//
//        IR2_OPND temp;
//
//        IR2_OPND label_nun  = ir2_opnd_new_type(IR2_OPND_LABEL);
//        IR2_OPND label_neq  = ir2_opnd_new_type(IR2_OPND_LABEL);
//        IR2_OPND label_gt   = ir2_opnd_new_type(IR2_OPND_LABEL);
//        IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//        if (option_lbt) {
//            temp = ra_alloc_itemp();
//            append_ir2_opnd2i(mips_ori, &temp, &zero_ir2_opnd, 0xfff);
//        }
//
//        /*  case 1: are they unordered? */
//        append_ir2_opnd2(mips_c_un_s, &dest, &src);
//        append_ir2_opnd1(mips_bc1t, &label_nun);
//        /* at least one of the operands is NaN */
//        // set zf,pf,cf = 111
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &temp, 0xb);
//        } else {
//            append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0045);
//        }
//        append_ir2_opnd1(mips_b, &label_exit);
//
//        /* case 2: not unordered. are they equal? */
//        append_ir2_opnd1(mips_label, &label_nun);
//        append_ir2_opnd2(mips_c_eq_s, &dest, &src);
//        append_ir2_opnd1(mips_bc1t, &label_neq);
//        /* two operands are equal */
//        // set zf,pf,cf = 100
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &temp, 0x8);
//            append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0x3);
//        } else {
//            append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0040);
//            /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xfffa);           */
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, PF_BIT | CF_BIT);
//        }
//        append_ir2_opnd1(mips_b, &label_exit);
//
//        /* case 3: not unordered, not equal. less than? */
//        append_ir2_opnd1(mips_label, &label_neq);
//        append_ir2_opnd2(mips_c_lt_s, &dest, &src);
//        append_ir2_opnd1(mips_bc1t, &label_gt);
//        /* less than */
//        // set zf.pf.cf = 001
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0xa);
//            append_ir2_opnd1i(mips_mtflag, &temp, 0x1);
//        } else {
//            append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0001);
//            /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xffbb);           */
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, ZF_BIT | PF_BIT);
//        }
//        append_ir2_opnd1(mips_b, &label_exit);
//
//        /* not unordered, not equal, not less than, so it's greater than */
//        //set zf,pf,cf = 000
//        append_ir2_opnd1(mips_label, &label_gt);
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0xb);
//        } else {
//            /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xffba);*/
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, ZF_BIT | PF_BIT | CF_BIT);
//        }
//
//        /* exit*/
//        //set of,sf,af = 000
//        append_ir2_opnd1(mips_label, &label_exit);
//        if (option_lbt) {
//            append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0x34);
//            ra_free_temp(&temp);
//        } else {
//            /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xf76f);*/
//            tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT | SF_BIT | AF_BIT);
//        }
//
//        return true;
//    }
//
//    lsassert(ir1_opnd_num(pir1) == 2);
//    IR2_OPND dest = load_freg_from_ir1_1(opnd0, false, false);
//    IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, false);
//
//    IR2_OPND temp;
//
//    IR2_OPND label_nun  = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_neq  = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_gt   = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    if (option_lbt) {
//        temp = ra_alloc_itemp();
//        append_ir2_opnd2i(mips_ori, &temp, &zero_ir2_opnd, 0xfff);
//    }
//
//    /*  case 1: are they unordered? */
//    append_ir2_opnd2(mips_c_un_s, &dest, &src);
//    append_ir2_opnd1(mips_bc1t, &label_nun);
//    /* at least one of the operands is NaN */
//    // set zf,pf,cf = 111
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &temp, 0xb);
//    } else {
//        append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0045);
//    }
//    append_ir2_opnd1(mips_b, &label_exit);
//
//    /* case 2: not unordered. are they equal? */
//    append_ir2_opnd1(mips_label, &label_nun);
//    append_ir2_opnd2(mips_c_eq_s, &dest, &src);
//    append_ir2_opnd1(mips_bc1t, &label_neq);
//    /* two operands are equal */
//    // set zf,pf,cf = 100
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &temp, 0x8);
//        append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0x3);
//    } else {
//        append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0040);
//        /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xfffa);           */
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, PF_BIT | CF_BIT);
//    }
//    append_ir2_opnd1(mips_b, &label_exit);
//
//    /* case 3: not unordered, not equal. less than? */
//    append_ir2_opnd1(mips_label, &label_neq);
//    append_ir2_opnd2(mips_c_lt_s, &dest, &src);
//    append_ir2_opnd1(mips_bc1t, &label_gt);
//    /* less than */
//    // set zf.pf.cf = 001
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0xa);
//        append_ir2_opnd1i(mips_mtflag, &temp, 0x1);
//    } else {
//        append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x0001);
//        /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xffbb);           */
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, ZF_BIT | PF_BIT);
//    }
//    append_ir2_opnd1(mips_b, &label_exit);
//
//    /* not unordered, not equal, not less than, so it's greater than */
//    //set zf,pf,cf = 000
//    append_ir2_opnd1(mips_label, &label_gt);
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0xb);
//    } else {
//        /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xffba);*/
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, ZF_BIT | PF_BIT | CF_BIT);
//    }
//
//    /* exit*/
//    //set of,sf,af = 000
//    append_ir2_opnd1(mips_label, &label_exit);
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mtflag, &zero_ir2_opnd, 0x34);
//        ra_free_temp(&temp);
//    } else {
//        /*append_ir2_opnd2i(mips_andi, &eflags_ir2_opnd, &eflags_ir2_opnd, 0xf76f);*/
//        tr_gen_eflags_clr_bit_00(&eflags_ir2_opnd, OF_BIT | SF_BIT | AF_BIT);
//    }
//
    return true;
}

bool translate_maskmovdqu(IR1_INST *pir1)
{
    lsassertm(0, "SIMD to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
//
//    if (option_xmm128map){
//        IR2_OPND src = load_freg128_from_ir1(opnd0);
//        IR2_OPND mask = load_freg128_from_ir1(opnd1);
//        IR2_OPND zero = ra_alloc_ftemp();
//        IR2_OPND temp_mask = ra_alloc_ftemp();
//        IR2_OPND mem_mask = ra_alloc_ftemp();
//        IR2_OPND mem_data = ra_alloc_ftemp();
//        IR2_OPND xmm_data = ra_alloc_ftemp();
//
//        append_ir2_opnd3(mips_xorv, &zero, &zero, &zero);
//
//        IR2_OPND base_opnd = ir2_opnd_new(IR2_OPND_IREG, 23);
//
//        lsassert(cpu_get_guest_base() == 0);
//
//        append_ir2_opnd2i(mips_andib, &temp_mask, &mask, 0x80);
//        append_ir2_opnd3(mips_ceqb, &mem_mask, &temp_mask, &zero);
//        append_ir2_opnd3(mips_norv, &temp_mask, &mem_mask, &zero);
//        append_ir2_opnd2i(mips_ldb, &mem_data, &base_opnd, 0);
//        append_ir2_opnd3(mips_andv, &xmm_data, &src, &temp_mask);
//        append_ir2_opnd3(mips_andv, &mem_data, &mem_data, &mem_mask);
//        append_ir2_opnd3(mips_orv, &mem_data, &mem_data, &xmm_data);
//        append_ir2_opnd2i(mips_stb, &mem_data, &base_opnd, 0);
//
//        return true;
//    }
//
//    IR2_OPND src_lo = ra_alloc_itemp();
//    IR2_OPND mask_hi = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src_lo, opnd0, UNKNOWN_EXTENSION, false);
//    load_ir1_to_ir2(&mask_hi, opnd1, UNKNOWN_EXTENSION, true);
//
//    IR2_OPND temp_src = ra_alloc_itemp();
//    IR2_OPND base_opnd = ir2_opnd_new(IR2_OPND_IREG, 23);
//    /* current code cannot recover a mapped register from EM_MIPS_ADDRESS
//       back into SIGN_EXTENSION. So we use a temp one
//     */
//    IR2_OPND tmp_opnd = ra_alloc_itemp();
//    if (cpu_get_guest_base() != 0) {
//        /*memory access should add guest_base */
//        IR2_OPND guest_base_opnd = ra_alloc_guest_base();
//        if (!ir2_opnd_is_address(&base_opnd)) {
//            append_ir2_opnd2(mips_mov_addrx, &base_opnd, &base_opnd);
//        }
//        append_ir2_opnd3(mips_add_addr, &tmp_opnd, &base_opnd, &guest_base_opnd);
//        ir2_opnd_set_em(&tmp_opnd, EM_MIPS_ADDRESS, 32);
//    } else {
//        tmp_opnd = base_opnd;
//    }
//
//    IR2_OPND temp_int = ra_alloc_itemp();
//    load_imm64_to_ir2(&temp_int, 0x80);
//
//    IR2_OPND temp_mask = ra_alloc_itemp();
//    IR2_OPND mask_lo = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&mask_lo, opnd1, UNKNOWN_EXTENSION, false);
//    append_ir2_opnd3(mips_and, &temp_mask, &mask_lo, &temp_int);
//
//    IR2_OPND label_next = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &temp_mask, &zero_ir2_opnd, &label_next);
//
//    append_ir2_opnd2i(mips_sb, &src_lo, &tmp_opnd, 0); /* replace by shell */
//                                                      /* script */
//    append_ir2_opnd1(mips_label, &label_next);
//    load_imm64_to_ir2(&temp_int, 0x8000);
//    append_ir2_opnd3(mips_and, &temp_mask, &mask_lo, &temp_int);
//
//    IR2_OPND label_next1 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &temp_mask, &zero_ir2_opnd, &label_next1);
//
//    append_ir2_opnd2i(mips_dsrl, &temp_src, &src_lo, 8);
//    append_ir2_opnd2i(mips_sb, &temp_src, &tmp_opnd, 1);
//
//    append_ir2_opnd1(mips_label, &label_next1);
//    load_imm64_to_ir2(&temp_int, 0x800000);
//    append_ir2_opnd3(mips_and, &temp_mask, &mask_lo, &temp_int);
//
//    IR2_OPND label_next2 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &temp_mask, &zero_ir2_opnd, &label_next2);
//
//    append_ir2_opnd2i(mips_dsrl, &temp_src, &src_lo, 16);
//    append_ir2_opnd2i(mips_sb, &temp_src, &tmp_opnd, 2);
//
//    append_ir2_opnd1(mips_label, &label_next2);
//    load_imm64_to_ir2(&temp_int, 0x80000000);
//    append_ir2_opnd3(mips_and, &temp_mask, &mask_lo, &temp_int);
//
//    IR2_OPND label_next3 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &temp_mask, &zero_ir2_opnd, &label_next3);
//
//    append_ir2_opnd2i(mips_dsrl, &temp_src, &src_lo, 24);
//    append_ir2_opnd2i(mips_sb, &temp_src, &tmp_opnd, 3);
//
//    append_ir2_opnd1(mips_label, &label_next3);
//    load_imm64_to_ir2(&temp_int, 0x8000000000LL);
//    append_ir2_opnd3(mips_and, &temp_mask, &mask_lo, &temp_int);
//
//    IR2_OPND label_next4 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &temp_mask, &zero_ir2_opnd, &label_next4);
//
//    append_ir2_opnd2i(mips_dsrl32, &temp_src, &src_lo, 0);
//    append_ir2_opnd2i(mips_sb, &temp_src, &tmp_opnd, 4);
//
//    append_ir2_opnd1(mips_label, &label_next4);
//    load_imm64_to_ir2(&temp_int, 0x800000000000LL);
//    append_ir2_opnd3(mips_and, &temp_mask, &mask_lo, &temp_int);
//
//    IR2_OPND label_next5 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &temp_mask, &zero_ir2_opnd, &label_next5);
//
//    append_ir2_opnd2i(mips_dsrl32, &temp_src, &src_lo, 8);
//    append_ir2_opnd2i(mips_sb, &temp_src, &tmp_opnd, 5);
//
//    append_ir2_opnd1(mips_label, &label_next5);
//    load_imm64_to_ir2(&temp_int, 0x80000000000000LL);
//    append_ir2_opnd3(mips_and, &temp_mask, &mask_lo, &temp_int);
//
//    IR2_OPND label_next6 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &temp_mask, &zero_ir2_opnd, &label_next6);
//
//    append_ir2_opnd2i(mips_dsrl32, &temp_src, &src_lo, 16);
//    append_ir2_opnd2i(mips_sb, &temp_src, &tmp_opnd, 6);
//
//    append_ir2_opnd1(mips_label, &label_next6);
//    load_imm64_to_ir2(&temp_int, 0x8000000000000000LL);
//    append_ir2_opnd3(mips_and, &temp_mask, &mask_lo, &temp_int);
//
//    IR2_OPND label_next7 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &temp_mask, &zero_ir2_opnd, &label_next7);
//
//    append_ir2_opnd2i(mips_dsrl32, &temp_src, &src_lo, 24);
//    append_ir2_opnd2i(mips_sb, &temp_src, &tmp_opnd, 7);
//    append_ir2_opnd1(mips_label, &label_next7);
//    load_imm64_to_ir2(&temp_int, 0x80);
//    append_ir2_opnd3(mips_and, &temp_mask, &mask_hi, &temp_int);
//
//    IR2_OPND src_hi = ra_alloc_itemp();
//    load_ir1_to_ir2(&src_hi, opnd0, UNKNOWN_EXTENSION, true);
//
//    IR2_OPND label_next8 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &temp_mask, &zero_ir2_opnd, &label_next8);
//
//    append_ir2_opnd2i(mips_sb, &src_hi, &tmp_opnd, 8);
//
//    append_ir2_opnd1(mips_label, &label_next8);
//    load_imm64_to_ir2(&temp_int, 0x8000);
//    append_ir2_opnd3(mips_and, &temp_mask, &mask_hi, &temp_int);
//
//    IR2_OPND label_next9 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &temp_mask, &zero_ir2_opnd, &label_next9);
//
//    append_ir2_opnd2i(mips_dsrl, &temp_src, &src_hi, 8);
//    append_ir2_opnd2i(mips_sb, &temp_src, &tmp_opnd, 9);
//
//    append_ir2_opnd1(mips_label, &label_next9);
//    load_imm64_to_ir2(&temp_int, 0x800000);
//    append_ir2_opnd3(mips_and, &temp_mask, &mask_hi, &temp_int);
//
//    IR2_OPND label_next10 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &temp_mask, &zero_ir2_opnd, &label_next10);
//
//    append_ir2_opnd2i(mips_dsrl, &temp_src, &src_hi, 16);
//    append_ir2_opnd2i(mips_sb, &temp_src, &tmp_opnd, 10);
//    append_ir2_opnd1(mips_label, &label_next10);
//    load_imm64_to_ir2(&temp_int, 0x80000000);
//    append_ir2_opnd3(mips_and, &temp_mask, &mask_hi, &temp_int);
//
//    IR2_OPND label_next11 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &temp_mask, &zero_ir2_opnd, &label_next11);
//
//    append_ir2_opnd2i(mips_dsrl, &temp_src, &src_hi, 24);
//    append_ir2_opnd2i(mips_sb, &temp_src, &tmp_opnd, 11);
//    append_ir2_opnd1(mips_label, &label_next11);
//    load_imm64_to_ir2(&temp_int, 0x8000000000LL);
//    append_ir2_opnd3(mips_and, &temp_mask, &mask_hi, &temp_int);
//
//    IR2_OPND label_next12 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &temp_mask, &zero_ir2_opnd, &label_next12);
//
//    append_ir2_opnd2i(mips_dsrl32, &temp_src, &src_hi, 0);
//    append_ir2_opnd2i(mips_sb, &temp_src, &tmp_opnd, 12);
//    append_ir2_opnd1(mips_label, &label_next12);
//    load_imm64_to_ir2(&temp_int, 0x800000000000LL);
//    append_ir2_opnd3(mips_and, &temp_mask, &mask_hi, &temp_int);
//
//    IR2_OPND label_next13 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &temp_mask, &zero_ir2_opnd, &label_next13);
//
//    append_ir2_opnd2i(mips_dsrl32, &temp_src, &src_hi, 8);
//    append_ir2_opnd2i(mips_sb, &temp_src, &tmp_opnd, 13);
//    append_ir2_opnd1(mips_label, &label_next13);
//    load_imm64_to_ir2(&temp_int, 0x80000000000000LL);
//    append_ir2_opnd3(mips_and, &temp_mask, &mask_hi, &temp_int);
//
//    IR2_OPND label_next14 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &temp_mask, &zero_ir2_opnd, &label_next14);
//
//    append_ir2_opnd2i(mips_dsrl32, &temp_src, &src_hi, 16);
//    append_ir2_opnd2i(mips_sb, &temp_src, &tmp_opnd, 14);
//
//    append_ir2_opnd1(mips_label, &label_next14);
//    load_imm64_to_ir2(&temp_int, 0x8000000000000000LL);
//    append_ir2_opnd3(mips_and, &temp_mask, &mask_hi, &temp_int);
//
//    IR2_OPND label_next15 = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_beq, &temp_mask, &zero_ir2_opnd, &label_next15);
//
//    append_ir2_opnd2i(mips_dsrl32, &temp_src, &src_hi, 24);
//    append_ir2_opnd2i(mips_sb, &temp_src, &tmp_opnd, 15);
//
//    append_ir2_opnd1(mips_label, &label_next15);

    return true;
}

#ifndef CONFIG_SOFTMMU
bool translate_pause(IR1_INST *pir1) { return true; }
#endif
