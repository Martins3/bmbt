#include "../include/common.h"
#include "../translator/translate.h"
#include "../include/reg-alloc.h"
#include "../x86tomips-options.h"

bool translate_movupd(IR1_INST *pir1)
{
    translate_movaps(pir1);
    return true;
}

bool translate_movdqa(IR1_INST *pir1)
{
    translate_movaps(pir1);
    return true;
}

bool translate_movdqu(IR1_INST *pir1)
{
    translate_movaps(pir1);
    return true;
}

bool translate_movups(IR1_INST *pir1)
{
    translate_movaps(pir1);
    return true;
}

bool translate_movapd(IR1_INST *pir1)
{
    translate_movaps(pir1);
    return true;
}

bool translate_movaps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif

    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src  = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_mem(src)) {
        IR2_OPND xmm_opnd = ra_alloc_xmm(ir1_opnd_base_reg_num(dest));
        load_freg128_from_ir1_mem(&xmm_opnd, src);
    }
    else if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
        IR2_OPND xmm_opnd = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
        store_freg128_to_ir1_mem(&xmm_opnd, dest);
    }
    else if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_xmm(src)) {
        IR2_OPND xmm_dest = ra_alloc_xmm(ir1_opnd_base_reg_num(dest));
        IR2_OPND xmm_src  = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
        append_ir2_opnd2i(LISA_VORI_B, &xmm_dest, &xmm_src, 0);
        //append_ir2_opnd2(mips_movev, &xmm_dest, &xmm_src);
    } else {
        lsassert(0);
    }

    return true;
}

bool translate_movhlps(IR1_INST *pir1)
{
    lsassertm(0, "SSE: movhlps to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if(option_xmm128map){
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1)));
//        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
//        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//        append_ir2_opnd2ii(mips_vextrinsd, &dest, &src, 0, 1);
//        return true;
//    }
//
//    IR2_OPND src_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, true, true);
//    store_freg_to_ir1(&src_hi, ir1_get_opnd(pir1, 0), false, true);
    return true;
}

bool translate_movlhps(IR1_INST *pir1)
{
    lsassertm(0, "SSE: movlhps to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if(option_xmm128map){
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1)));
//        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
//        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//        append_ir2_opnd2ii(mips_vextrinsd, &dest, &src, 1, 0);
//        return true;
//    }
//
//    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    store_freg_to_ir1(&src_lo, ir1_get_opnd(pir1, 0), true, true);
    return true;
}

bool translate_movsd(IR1_INST *pir1)
{
    if (pir1->info->detail->x86.opcode[0] == 0xa5) {
        return translate_movs(pir1);
    }

#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif

    lsassertm(option_xmm128map, "SIMD not 128map enabled\n");

    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src  = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_mem(src)) {
        IR2_OPND temp = load_freg_from_ir1_1(src, false, false);
        IR2_OPND xmm_dest = ra_alloc_xmm(ir1_opnd_base_reg_num(dest));
        append_ir2_opnd2i(LISA_VCLRSTRI_V, &xmm_dest, &temp, 7);
    }
    else if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
        IR2_OPND xmm_src = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
        store_freg_to_ir1(&xmm_src, dest, false, false);
    }
    else if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_xmm(src)) {
        IR2_OPND xmm_dest = ra_alloc_xmm(ir1_opnd_base_reg_num(dest));
        IR2_OPND xmm_src  = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
        append_ir2_opnd2i(LISA_XVINSVE0_D, &xmm_dest, &xmm_src, 0);
    }

    return true;
}

bool translate_movss(IR1_INST *pir1)
{
    lsassertm(0, "movss to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map) {
//        IR1_OPND *dest = ir1_get_opnd(pir1, 0);
//        IR1_OPND *src = ir1_get_opnd(pir1, 1);
//        if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_mem(src)) {
//            IR2_OPND temp = load_freg_from_ir1_1(src, false, false);
//            IR2_OPND xmm_dest = ra_alloc_xmm(ir1_opnd_base_reg_num(dest));
//            append_ir2_opnd2i(mips_vclrstriv, &xmm_dest, &temp, 3);
//            return true;
//        }
//        else if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
//            IR2_OPND xmm_src = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
//            store_freg_to_ir1(&xmm_src, dest, false, false);
//            return true;
//        }
//        else if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_xmm(src)) {
//            IR2_OPND xmm_dest = ra_alloc_xmm(ir1_opnd_base_reg_num(dest));
//            IR2_OPND xmm_src  = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
//            append_ir2_opnd2i(mips_insvew, &xmm_dest, &xmm_src, 0);
//            return true;
//        }
//        if (ir1_opnd_is_xmm(dest) || ir1_opnd_is_xmm(src)){
//            lsassert(0);
//        }
//    }
//
//    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, false);
//
//    if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 0) + 1)) {
//        IR2_OPND dst_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
//        IR2_OPND dst_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), true, true);
//        append_ir2_opnd3(mips_fdsll, &src_lo, &src_lo, &f32_ir2_opnd);
//        append_ir2_opnd3(mips_fdsrl, &dst_lo, &src_lo, &f32_ir2_opnd);
//        append_ir2_opnd2(mips_dmtc1, &zero_ir2_opnd, &dst_hi);
//    } else if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0) + 1) &&
//               ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))) {
//        IR2_OPND temp = ra_alloc_ftemp();
//        IR2_OPND dst_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
//        append_ir2_opnd3(mips_fdsll, &temp, &src_lo, &f32_ir2_opnd);
//        append_ir2_opnd3(mips_fdsrl, &temp, &temp, &f32_ir2_opnd);
//        append_ir2_opnd3(mips_fdsrl, &dst_lo, &dst_lo, &f32_ir2_opnd);
//        append_ir2_opnd3(mips_fdsll, &dst_lo, &dst_lo, &f32_ir2_opnd);
//        append_ir2_opnd3(mips_for, &dst_lo, &dst_lo, &temp);
//        ra_free_temp(&temp);
//    } else {
//        store_freg_to_ir1(&src_lo, ir1_get_opnd(pir1, 0), false, false);
//    }

    return true;
}

bool translate_movhpd(IR1_INST *pir1)
{
    lsassertm(0, "movhpd to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map) {
//        IR1_OPND *dest = ir1_get_opnd(pir1, 0);
//        IR1_OPND *src = ir1_get_opnd(pir1, 1);
//        if (ir1_opnd_is_mem(src) && ir1_opnd_is_xmm(dest)) {
//            IR2_OPND temp = ra_alloc_itemp();
//            load_ir1_to_ir2(&temp, src, ZERO_EXTENSION, false);
//            IR2_OPND xmm_dest = ra_alloc_xmm(ir1_opnd_base_reg_num(dest));
//            append_ir2_opnd2i(mips_insertd, &xmm_dest, &temp, 1);
//            return true;
//        }
//        else if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
//            IR2_OPND temp = ra_alloc_itemp();
//            IR2_OPND xmm_src = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
//            append_ir2_opnd2i(mips_copy_sd, &temp, &xmm_src, 1);
//            store_ir2_to_ir1(&temp, dest, false);
//            return true;
//        }
//        else {
//            lsassert(0);
//        }
//    }
//
//    if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 0) + 1)) {
//        IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//        store_freg_to_ir1(&src_lo, ir1_get_opnd(pir1, 0), true, true);
//    } else {
//        IR2_OPND src_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, true, true);
//        store_freg_to_ir1(&src_hi, ir1_get_opnd(pir1, 0), false, true);
//    }

    return true;
}

bool translate_movhps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif

    translate_movhpd(pir1);
    return true;
}

bool translate_movlpd(IR1_INST *pir1)
{
    lsassertm(0, "movlpd to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map) {
//        IR1_OPND *dest = ir1_get_opnd(pir1, 0);
//        IR1_OPND *src = ir1_get_opnd(pir1, 1);
//        if (ir1_opnd_is_mem(src) && ir1_opnd_is_xmm(dest)) {
//            IR2_OPND temp = ra_alloc_itemp();
//            load_ir1_to_ir2(&temp, src, ZERO_EXTENSION, false);
//            IR2_OPND xmm_dest = ra_alloc_xmm(ir1_opnd_base_reg_num(dest));
//            append_ir2_opnd2i(mips_insertd, &xmm_dest, &temp, 0);
//            return true;
//        }
//        else if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
//            IR2_OPND xmm_src = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
//            store_freg_to_ir1(&xmm_src, dest, false, false);
//            return true;
//        }
//        else {
//            lsassert(0);
//        }
//    }
//
//    if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 0) + 1)) {
//        IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//        store_freg_to_ir1(&src_lo, ir1_get_opnd(pir1, 0), false, true);
//    } else {
//        IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//        store_freg_to_ir1(&src_lo, ir1_get_opnd(pir1, 0), false, true);
//    }

    return true;
}

bool translate_movlps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif

    translate_movlpd(pir1);
    return true;
}

bool translate_addpd(IR1_INST *pir1)
{
    lsassertm(0, "addpd to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
//        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
//        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//        append_ir2_opnd3(mips_faddd, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND src_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, true, true);
//    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    IR2_OPND dst_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), true, true);
//    IR2_OPND dst_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
//    append_ir2_opnd3(mips_add_d, &dst_hi, &dst_hi, &src_hi);
//    append_ir2_opnd3(mips_add_d, &dst_lo, &dst_lo, &src_lo);
//    /*    store_freg_to_ir1(&dst_hi, ir1_get_opnd(pir1, 0), true, true); */
//    /*    store_freg_to_ir1(&dst_lo, ir1_get_opnd(pir1, 0), false, true); */
    return true;
}

bool translate_andpd(IR1_INST *pir1)
{
    lsassertm(0, "andpd to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
//        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
//        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//        append_ir2_opnd3(mips_andv, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND src_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, true, true);
//    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    IR2_OPND dst_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), true, true);
//    IR2_OPND dst_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
//    append_ir2_opnd3(mips_fand, &dst_hi, &dst_hi, &src_hi);
//    append_ir2_opnd3(mips_fand, &dst_lo, &dst_lo, &src_lo);
//    /*    store_freg_to_ir1(&dst_hi, ir1_get_opnd(pir1, 0), true, true); */
//    /*    store_freg_to_ir1(&dst_lo, ir1_get_opnd(pir1, 0), false, true); */
    return true;
}

bool translate_unpcklps(IR1_INST *pir1)
{
    lsassertm(0, "unpcklps to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
//        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
//        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//        append_ir2_opnd3(mips_ilvrw, &dest, &src, &dest);
//        return true;
//    }
//
//    /*    IR2_OPND src_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0)+1, true, true); */
//    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    IR2_OPND dst_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), true, true);
//    IR2_OPND dst_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
//    append_ir2_opnd3(mips_punpckhwd, &dst_hi, &dst_lo, &src_lo);
//    append_ir2_opnd3(mips_punpcklwd, &dst_lo, &dst_lo, &src_lo);
    return true;
}

bool translate_unpcklpd(IR1_INST *pir1)
{
    lsassertm(0, "unpcklpd to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map){
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
//        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
//        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//        append_ir2_opnd2i(mips_shfd, &dest, &src, 0x8);
//        return true;
//    }
//
//    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    store_freg_to_ir1(&src_lo, ir1_get_opnd(pir1, 0), true, true);
//
    return true;
}

bool translate_unpckhpd(IR1_INST *pir1)
{
    lsassertm(0, "unpckhpd to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map){
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
//        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
//        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//        append_ir2_opnd2i(mips_shfd, &dest, &src, 0xd);
//        return true;
//    }
//
//    if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 0) + 1)) {
//        IR2_OPND dst_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), true, true);
//        IR2_OPND dst_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
//        append_ir2_opnd2(mips_mov_d, &dst_lo, &dst_hi);
//        load_freg_from_ir1_2(&dst_hi, ir1_get_opnd(pir1, 0) + 1, true, true);
//
//    } else {
//        IR2_OPND src_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, true, true);
//        IR2_OPND dst_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), true, true);
//        IR2_OPND dst_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
//        append_ir2_opnd2(mips_mov_d, &dst_lo, &dst_hi);
//        if (!ir2_opnd_cmp(&src_hi, &dst_hi)) {
//            append_ir2_opnd2(mips_mov_d, &dst_hi, &src_hi);
//        }
//    }
    return true;
}

bool translate_unpckhps(IR1_INST *pir1)
{
    lsassertm(0, "unpckhps to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
//        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
//        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//        append_ir2_opnd3(mips_ilvlw, &dest, &src, &dest);
//        return true;
//    }
//
//    IR2_OPND src_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, true, true);
//    /*    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0)+1, false, true); */
//    IR2_OPND dst_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), true, true);
//    IR2_OPND dst_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
//    append_ir2_opnd3(mips_punpcklwd, &dst_lo, &dst_hi, &src_hi);
//    append_ir2_opnd3(mips_punpckhwd, &dst_hi, &dst_hi, &src_hi);
    return true;
}

bool translate_shufps(IR1_INST *pir1)
{
    lsassertm(0, "shufps to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
//        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
//        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//        uint64_t imm8 = ir1_opnd_uimm(ir1_get_opnd(pir1, 2));
//        IR2_OPND temp1 = ra_alloc_ftemp();
//        IR2_OPND temp2 = ra_alloc_ftemp();
//        append_ir2_opnd2i(mips_shfw, &temp1, &dest, imm8);
//        append_ir2_opnd2i(mips_shfw, &temp2, &src, imm8 >> 4);
//        append_ir2_opnd3(mips_pckevd, &dest , &temp2, &temp1);
//        return true;
//    }
//
//    IR2_OPND src_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, true, true);
//    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    IR2_OPND dst_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), true, true);
//    IR2_OPND dst_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
//
//    IR2_OPND temp1 = ra_alloc_ftemp();
//    IR2_OPND temp2 = ra_alloc_ftemp();
//    IR2_OPND temp_re = ra_alloc_ftemp();
//
//    uint8 imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 0) + 2);
//    uint8 select = imm & 0x3;
//    switch (select) {
//    case 0:
//        append_ir2_opnd3(mips_fdsll, &temp1, &dst_lo, &f32_ir2_opnd);
//        append_ir2_opnd3(mips_fdsrl, &temp1, &temp1, &f32_ir2_opnd);
//        break;
//    case 1:
//        append_ir2_opnd3(mips_fdsrl, &temp1, &dst_lo, &f32_ir2_opnd);
//        break;
//    case 2:
//        append_ir2_opnd3(mips_fdsll, &temp1, &dst_hi, &f32_ir2_opnd);
//        append_ir2_opnd3(mips_fdsrl, &temp1, &temp1, &f32_ir2_opnd);
//        break;
//    case 3:
//        append_ir2_opnd3(mips_fdsrl, &temp1, &dst_hi, &f32_ir2_opnd);
//        break;
//    default:
//        fprintf(stderr, "1: invalid imm8<0:1> in SHUFPS : %d\n", select);
//        exit(-1);
//    }
//
//    imm = imm >> 2;
//    select = imm & 0x3;
//    switch (select) {
//    case 0:
//        append_ir2_opnd3(mips_fdsll, &temp2, &dst_lo, &f32_ir2_opnd);
//        break;
//    case 1:
//        append_ir2_opnd3(mips_fdsrl, &temp2, &dst_lo, &f32_ir2_opnd);
//        append_ir2_opnd3(mips_fdsll, &temp2, &temp2, &f32_ir2_opnd);
//        break;
//    case 2:
//        append_ir2_opnd3(mips_fdsll, &temp2, &dst_hi, &f32_ir2_opnd);
//        break;
//    case 3:
//        append_ir2_opnd3(mips_fdsrl, &temp2, &dst_hi, &f32_ir2_opnd);
//        append_ir2_opnd3(mips_fdsll, &temp2, &temp2, &f32_ir2_opnd);
//        break;
//    default:
//        fprintf(stderr, "1: invalid imm8<0:1> in SHUFPS : %d\n", select);
//        exit(-1);
//    }
//
//    if (ir2_opnd_cmp(&src_hi, &dst_hi) && ir2_opnd_cmp(&src_lo, &dst_lo)) {
//        append_ir2_opnd3(mips_for, &temp_re, &temp1, &temp2);
//    } else {
//        append_ir2_opnd3(mips_for, &dst_lo, &temp1, &temp2);
//    }
//
//    imm = imm >> 2;
//    select = imm & 0x3;
//    switch (select) {
//    case 0:
//        append_ir2_opnd3(mips_fdsll, &temp1, &src_lo, &f32_ir2_opnd);
//        append_ir2_opnd3(mips_fdsrl, &temp1, &temp1, &f32_ir2_opnd);
//        break;
//    case 1:
//        append_ir2_opnd3(mips_fdsrl, &temp1, &src_lo, &f32_ir2_opnd);
//        break;
//    case 2:
//        append_ir2_opnd3(mips_fdsll, &temp1, &src_hi, &f32_ir2_opnd);
//        append_ir2_opnd3(mips_fdsrl, &temp1, &temp1, &f32_ir2_opnd);
//        break;
//    case 3:
//        append_ir2_opnd3(mips_fdsrl, &temp1, &src_hi, &f32_ir2_opnd);
//        break;
//    default:
//        fprintf(stderr, "1: invalid imm8<0:1> in SHUFPS : %d\n", select);
//        exit(-1);
//    }
//
//    imm = imm >> 2;
//    select = imm & 0x3;
//    switch (select) {
//    case 0:
//        append_ir2_opnd3(mips_fdsll, &temp2, &src_lo, &f32_ir2_opnd);
//        break;
//    case 1:
//        append_ir2_opnd3(mips_fdsrl, &temp2, &src_lo, &f32_ir2_opnd);
//        append_ir2_opnd3(mips_fdsll, &temp2, &temp2, &f32_ir2_opnd);
//        break;
//    case 2:
//        append_ir2_opnd3(mips_fdsll, &temp2, &src_hi, &f32_ir2_opnd);
//        break;
//    case 3:
//        append_ir2_opnd3(mips_fdsrl, &temp2, &src_hi, &f32_ir2_opnd);
//        append_ir2_opnd3(mips_fdsll, &temp2, &temp2, &f32_ir2_opnd);
//        break;
//    default:
//        fprintf(stderr, "1: invalid imm8<0:1> in SHUFPS : %d\n", select);
//        exit(-1);
//    }
//
//    append_ir2_opnd3(mips_for, &dst_hi, &temp1, &temp2);
//    if (ir2_opnd_cmp(&src_hi, &dst_hi) && ir2_opnd_cmp(&src_lo, &dst_lo)) {
//        append_ir2_opnd2(mips_mov_d, &dst_lo, &temp_re);
//    }
//
//    ra_free_temp(&temp1);
//    ra_free_temp(&temp2);
//    ra_free_temp(&temp_re);

    return true;
}

bool translate_shufpd(IR1_INST *pir1)
{
    lsassertm(0, "shufpd to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map){
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
//        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
//        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//        uint8_t imm8 = ir1_opnd_uimm(ir1_get_opnd(pir1, 2));
//        imm8 &= 3;
//        uint8_t shfd_imm8 = 0;
//        if (imm8 == 0){
//            shfd_imm8 = 0x8;
//        }
//        else if(imm8 == 1) {
//            shfd_imm8 = 0x9;
//        }
//        else if(imm8 == 2) {
//            shfd_imm8 = 0xc;
//        }
//        else if(imm8 == 3) {
//            shfd_imm8 = 0xd;
//        }
//        else {
//            lsassert(0);
//        }
//
//        append_ir2_opnd2i(mips_shfd, &dest, &src, shfd_imm8);
//        return true;
//    }
//
//    IR2_OPND src_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, true, true);
//    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    IR2_OPND dst_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), true, true);
//    IR2_OPND dst_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
//
//    IR2_OPND temp1 = ra_alloc_ftemp();
//    bool dest_equal_src = false;
//    if (ir2_opnd_cmp(&src_hi, &dst_hi) && ir2_opnd_cmp(&src_lo, &dst_lo)) {
//        dest_equal_src = true;
//    }
//
//    uint8 imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 0) + 2);
//    uint8 select = imm & 0x3;
//    switch (select) {
//    case 0:
//        append_ir2_opnd2(mips_mov_d, &dst_hi, &src_lo);
//        break;
//    case 1:
//        if (dest_equal_src) {
//            append_ir2_opnd2(mips_mov_d, &temp1, &dst_hi);
//            append_ir2_opnd2(mips_mov_d, &dst_hi, &src_lo);
//            append_ir2_opnd2(mips_mov_d, &src_lo, &temp1);
//        } else {
//            append_ir2_opnd2(mips_mov_d, &dst_lo, &dst_hi);
//            append_ir2_opnd2(mips_mov_d, &dst_hi, &src_lo);
//        }
//        break;
//    case 2:
//        if (dest_equal_src) {
//        } else {
//            append_ir2_opnd2(mips_mov_d, &dst_hi, &src_hi);
//        }
//        break;
//    case 3:
//        if (dest_equal_src) {
//            append_ir2_opnd2(mips_mov_d, &dst_lo, &dst_hi);
//        } else {
//            append_ir2_opnd2(mips_mov_d, &dst_lo, &dst_hi);
//            append_ir2_opnd2(mips_mov_d, &dst_hi, &src_hi);
//        }
//        break;
//
//    default:
//        fprintf(stderr, "1: invalid imm8<0:1> in SHUFPd : %d\n", select);
//        exit(-1);
//    }
//    ra_free_temp(&temp1);
    return true;
}

bool translate_punpcklqdq(IR1_INST *pir1)
{
    lsassertm(0, "punpcklqdq to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
//        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
//        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//        append_ir2_opnd3(mips_ilvrd, &dest, &src, &dest);
//        return true;
//    }
//    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    store_freg_to_ir1(&src_lo, ir1_get_opnd(pir1, 0), true, true);
   return true;
}

bool translate_xorps(IR1_INST *pir1)
{
    lsassertm(0, "xorps to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
//        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
//        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//        append_ir2_opnd3(mips_xorv, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND src_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, true, true);
//    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    IR2_OPND dst_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), true, true);
//    IR2_OPND dst_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
//    append_ir2_opnd3(mips_fxor, &dst_lo, &dst_lo, &src_lo);
//    append_ir2_opnd3(mips_fxor, &dst_hi, &dst_hi, &src_hi);
    return true;
}

bool translate_xorpd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_sse_common_excp_check(pir1)) return true;
#endif

    translate_xorps(pir1);
    return true;
}

bool translate_subss(IR1_INST *pir1)
{
    lsassertm(0, "subss to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
//        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
//        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//        IR2_OPND temp = ra_alloc_ftemp();
//        append_ir2_opnd3(mips_sub_s, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, false);
//    IR2_OPND dst_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
//    IR2_OPND temp1 = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_sub_s, &temp1, &dst_lo, &src_lo);
//    append_ir2_opnd3(mips_fdsrl, &dst_lo, &dst_lo, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_punpcklwd, &dst_lo, &temp1, &dst_lo);
//    ra_free_temp(&temp1);
    return true;
}

bool translate_subsd(IR1_INST *pir1)
{
    lsassertm(0, "subsd to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
//        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
//        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//        IR2_OPND temp = ra_alloc_ftemp();
//        append_ir2_opnd3(mips_sub_d, &temp, &dest, &src);
//        append_ir2_opnd2i(mips_insved, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    IR2_OPND dst_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
//    append_ir2_opnd3(mips_sub_d, &dst_lo, &dst_lo, &src_lo);
//
    return true;
}

bool translate_subps(IR1_INST *pir1)
{
    lsassertm(0, "subps to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
//        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
//        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//        append_ir2_opnd3(mips_fsubw, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dst_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), true, true);
//    IR2_OPND dst_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
//    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    IR2_OPND src_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, true, true);
//    IR2_OPND temp1 = ra_alloc_ftemp();
//    IR2_OPND temp2 = ra_alloc_ftemp();
//    append_ir2_opnd3(mips_fdsll, &temp1, &src_lo, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_fdsrl, &temp1, &temp1, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_fdsll, &temp2, &dst_lo, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_fdsrl, &temp2, &temp2, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_sub_s, &temp1, &temp2, &temp1);
//    append_ir2_opnd3(mips_fdsrl, &temp2, &src_lo, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_fdsrl, &dst_lo, &dst_lo, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_sub_s, &dst_lo, &dst_lo, &temp2);
//    append_ir2_opnd3(mips_fdsll, &dst_lo, &dst_lo, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_for, &dst_lo, &dst_lo, &temp1);
//    append_ir2_opnd3(mips_fdsll, &temp1, &src_hi, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_fdsrl, &temp1, &temp1, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_fdsll, &temp2, &dst_hi, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_fdsrl, &temp2, &temp2, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_sub_s, &temp1, &temp2, &temp1);
//    append_ir2_opnd3(mips_fdsrl, &temp2, &src_hi, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_fdsrl, &dst_hi, &dst_hi, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_sub_s, &dst_hi, &dst_hi, &temp2);
//    append_ir2_opnd3(mips_fdsll, &dst_hi, &dst_hi, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_for, &dst_hi, &dst_hi, &temp1);
//    ra_free_temp(&temp1);
//    ra_free_temp(&temp2);

    return true;
}

bool translate_subpd(IR1_INST *pir1)
{
    lsassertm(0, "subpd to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
//        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
//        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//        append_ir2_opnd3(mips_fsubd, &dest, &dest, &src);
//        return true;
//    }
//
//    IR2_OPND dst_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), true, true);
//    IR2_OPND dst_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
//    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    IR2_OPND src_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, true, true);
//
//    append_ir2_opnd3(mips_sub_d, &dst_lo, &dst_lo, &src_lo);
//    append_ir2_opnd3(mips_sub_d, &dst_hi, &dst_hi, &src_hi);
//
    return true;
}

bool translate_sqrtsd(IR1_INST *pir1)
{
    lsassertm(0, "sqrtsd to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
//        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
//        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//        IR2_OPND temp = ra_alloc_ftemp();
//        append_ir2_opnd2(mips_sqrt_d, &temp, &src);
//        append_ir2_opnd2i(mips_insved, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND dst_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
//    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    append_ir2_opnd2(mips_sqrt_d, &dst_lo, &src_lo);

    return true;
}

bool translate_sqrtss(IR1_INST *pir1)
{
    lsassertm(0, "sqrtss to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
//        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
//        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//        IR2_OPND temp = ra_alloc_ftemp();
//        append_ir2_opnd2(mips_sqrt_s, &temp, &src);
//        append_ir2_opnd2i(mips_insvew, &dest, &temp, 0);
//        return true;
//    }
//
//    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, false);
//    IR2_OPND dst_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
//    IR2_OPND temp1 = ra_alloc_ftemp();
//    append_ir2_opnd2(mips_sqrt_s, &temp1, &src_lo);
//    append_ir2_opnd3(mips_fdsrl, &dst_lo, &dst_lo, &f32_ir2_opnd);
//    append_ir2_opnd3(mips_punpcklwd, &dst_lo, &temp1, &dst_lo);
//    ra_free_temp(&temp1);

    return true;
}

bool translate_cvtps2pd(IR1_INST *pir1)
{
    lsassertm(0, "cvtps2pd to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    if (option_xmm128map) {
//        lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
//        //TODO:simply
//        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
//        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//        IR2_OPND temp = ra_alloc_ftemp();
//        IR2_OPND temp0 = ra_alloc_ftemp();
//        append_ir2_opnd2(mips_cvt_d_s, &temp0, &src);
//        append_ir2_opnd2i(mips_shfw, &temp, &src, 0x55);
//        append_ir2_opnd2(mips_cvt_d_s, &temp, &temp);
//        append_ir2_opnd2i(mips_insved, &dest, &temp, 1);
//        append_ir2_opnd2i(mips_insved, &dest, &temp0, 0);
//        return true;
//    }
//
//    IR2_OPND dst_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), true, true);
//    IR2_OPND dst_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
//    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    append_ir2_opnd3(mips_fdsrl, &dst_hi, &src_lo, &f32_ir2_opnd);
//    append_ir2_opnd2(mips_cvt_d_s, &dst_lo, &src_lo);
//    append_ir2_opnd2(mips_cvt_d_s, &dst_hi, &dst_hi);

    return true;
}

bool translate_cvttsd2si(IR1_INST *pir1)
{
    lsassertm(0, "cvttsd2si to be implemented in LoongArch.\n");
//#ifdef CONFIG_SOFTMMU
//    if (tr_gen_sse_common_excp_check(pir1)) return true;
//#endif
//
//    IR2_OPND src_lo;
//
//    if (option_xmm128map){
//        src_lo = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
//    } else {
//        src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
//    }
//
//    IR2_OPND temp_over_flow = ra_alloc_itemp();
//    IR2_OPND ftemp_over_flow = ra_alloc_ftemp();
//    append_ir2_opnd1i(mips_lui, &temp_over_flow, 0x41e0);
//    append_ir2_opnd2i(mips_dsll32, &temp_over_flow, &temp_over_flow, 0);
//    append_ir2_opnd2(mips_dmtc1, &temp_over_flow, &ftemp_over_flow);
//
//    IR2_OPND temp_under_flow = ra_alloc_itemp();
//    IR2_OPND ftemp_under_flow = ra_alloc_ftemp();
//    append_ir2_opnd1i(mips_lui, &temp_under_flow, 0xc1e);
//    append_ir2_opnd2i(mips_dsll32, &temp_under_flow, &temp_under_flow, 4);
//    append_ir2_opnd2(mips_dmtc1, &temp_under_flow, &ftemp_under_flow);
//
//    /*is unoder?*/
//    IR2_OPND label_for_flow = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2(mips_c_un_d, &src_lo, &src_lo);
//    append_ir2_opnd1(mips_bc1t, &label_for_flow);
//
//    /*is over flow or under flow*/
//    append_ir2_opnd2(mips_c_le_d, &ftemp_over_flow, &src_lo);
//    append_ir2_opnd1(mips_bc1t, &label_for_flow);
//    append_ir2_opnd2(mips_c_lt_d, &src_lo, &ftemp_under_flow);
//    append_ir2_opnd1(mips_bc1t, &label_for_flow);
//    /*not over flow and under flow*/
//    IR2_OPND temp_fcsr = ra_alloc_itemp();
//    /*save fscr in temp_int for reload*/
//    append_ir2_opnd2(mips_cfc1, &temp_fcsr, &fcsr_ir2_opnd);
//    append_ir2_opnd3(mips_or, &temp_under_flow, &temp_under_flow,
//                     &temp_fcsr); /* temp_under_flow save fcsr */
//    /*set fscr for rounding to zero according to x86 operation*/
//    append_ir2_opnd2i(mips_dsrl, &temp_fcsr, &temp_fcsr, 0x2);
//    append_ir2_opnd2i(mips_dsll, &temp_fcsr, &temp_fcsr, 0x2);
//    append_ir2_opnd2i(mips_ori, &temp_fcsr, &temp_fcsr, 0x1);
//    append_ir2_opnd2(mips_ctc1, &temp_fcsr, &fcsr_ir2_opnd);
//
//    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
//        append_ir2_opnd2(mips_cvt_l_d, &ftemp_over_flow, &src_lo);
//    } else {
//        append_ir2_opnd2(mips_cvt_w_d, &ftemp_over_flow, &src_lo);
//    }
//
//    append_ir2_opnd2(mips_dmfc1, &temp_fcsr,
//                     &ftemp_over_flow); /* temp_fscr used as temp_dest */
//    /* reload fcsr */
//    append_ir2_opnd2(mips_ctc1, &temp_under_flow, &fcsr_ir2_opnd);
//
//    IR2_OPND label_for_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd1(mips_b, &label_for_exit);
//    append_ir2_opnd1(mips_label, &label_for_flow);
//    load_imm32_to_ir2(&temp_fcsr, 0x80000000, ZERO_EXTENSION);
//    append_ir2_opnd1(mips_label, &label_for_exit);
//    store_ir2_to_ir1(&temp_fcsr, ir1_get_opnd(pir1, 0), false);
//
//    ra_free_temp(&temp_fcsr);
//    ra_free_temp(&temp_over_flow);
//    ra_free_temp(&temp_under_flow);
//    ra_free_temp(&ftemp_over_flow);
//    ra_free_temp(&ftemp_under_flow);

    return true;
}
