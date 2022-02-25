#include "common.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"

/* bool translate_movmskps(IR1_INST * pir1) { return false; } */
/* bool translate_movmskpd(IR1_INST * pir1) { return false; } */
/* bool translate_movdq2q(IR1_INST * pir1) { return false; } */
/* bool translate_movnti(IR1_INST * pir1) { return false; } */
/* bool translate_movntdq(IR1_INST * pir1) { return false; } */
/* bool translate_movntpd(IR1_INST * pir1) { return false; } */
/* bool translate_movntps(IR1_INST * pir1) { return false; } */
/* bool translate_movq2dq(IR1_INST * pir1) { return false; } */
/* bool translate_movntq(IR1_INST * pir1) { return false; } */
/* bool translate_pmovmskb(IR1_INST * pir1) { return false; } */
/* bool translate_maskmovdqu(IR1_INST * pir1) { return false; } */
/* bool translate_maskmovq(IR1_INST *pir1) { return false; } */
/* bool translate_movupd(IR1_INST *pir1)  */
/* bool translate_movdqa(IR1_INST *pir1)  */
/* bool translate_movdqu(IR1_INST *pir1)  */
/* bool translate_movups(IR1_INST *pir1)  */
/* bool translate_movapd(IR1_INST *pir1)  */
/* bool translate_movaps(IR1_INST *pir1)  */
/* bool translate_movhlps(IR1_INST *pir1) */
/* bool translate_movlhps(IR1_INST *pir1) */
/* bool translate_movsd(IR1_INST *pir1)   */
/* bool translate_movss(IR1_INST *pir1)   */
/* bool translate_movhpd(IR1_INST *pir1)  */
/* bool translate_movhps(IR1_INST *pir1)  */
/* bool translate_movlpd(IR1_INST *pir1)  */
/* bool translate_movlps(IR1_INST *pir1)  */

bool translate_movdq2q(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movdq2q(pir1);
#else
    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src = ir1_get_opnd(pir1, 1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1)));
    la_append_ir2_opnd2(LISA_FMOV_D, ra_alloc_mmx(ir1_opnd_base_reg_num(dest)),
                     ra_alloc_xmm(ir1_opnd_base_reg_num(src)));
    // TODO:zero fpu top and tag word
    return true;
#endif
}

bool translate_movmskpd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movmskpd(pir1);
#else
    IR1_OPND* dest = ir1_get_opnd(pir1, 0);
    IR1_OPND* src = ir1_get_opnd(pir1, 1);
    if(ir1_opnd_is_xmm(src)){
        IR2_OPND temp = ra_alloc_ftemp();
        la_append_ir2_opnd2(LISA_VMSKLTZ_D, temp, 
            ra_alloc_xmm(ir1_opnd_base_reg_num(src)));
        la_append_ir2_opnd2_em(LISA_MOVFR2GR_D, 
            ra_alloc_gpr(ir1_opnd_base_reg_num(dest)), temp);
        return true;
    }
    lsassert(0);
    return false;
#endif
}

bool translate_movmskps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movmskps(pir1);
#else
    IR1_OPND* dest = ir1_get_opnd(pir1, 0);
    IR1_OPND* src = ir1_get_opnd(pir1, 1);
    if(ir1_opnd_is_xmm(src)){
        IR2_OPND temp = ra_alloc_ftemp();
        la_append_ir2_opnd2(LISA_VMSKLTZ_W, temp, 
            ra_alloc_xmm(ir1_opnd_base_reg_num(src)));
        la_append_ir2_opnd2_em(LISA_MOVFR2GR_D, 
            ra_alloc_gpr(ir1_opnd_base_reg_num(dest)), temp);
        return true;
    }
    lsassert(0);
    return false;
#endif
}

bool translate_movntdq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movntdq(pir1);
#else
    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src = ir1_get_opnd(pir1, 1);
    if (ir1_opnd_is_xmm(src)) {
        IR2_OPND src_ir2 = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
        store_freg128_to_ir1_mem(src_ir2, dest);
        return true;
    }
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    IR2_OPND src_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, true,
                                           true); /* fill default parameter */
    store_freg_to_ir1(src_lo, ir1_get_opnd(pir1, 0), false,
                      true); /* fill default parameter */
    store_freg_to_ir1(src_hi, ir1_get_opnd(pir1, 0), true,
                      true); /* fill default parameter */
    return true;
#endif
}

bool translate_movnti(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movnti(pir1);
#else
    IR2_OPND src = load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION,
                                      false);   /* fill default parameter */
    store_ireg_to_ir1(src, ir1_get_opnd(pir1, 0), false); /* fill default parameter */
    return true;
#endif
}

bool translate_movntpd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movntpd(pir1);
#else
    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src = ir1_get_opnd(pir1, 1);
    if (ir1_opnd_is_xmm(src)) {
        IR2_OPND src_ir2 = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
        store_freg128_to_ir1_mem(src_ir2, dest);
        return true;
    }
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    IR2_OPND src_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, true,
                                           true); /* fill default parameter */
    store_freg_to_ir1(src_lo, ir1_get_opnd(pir1, 0), false,
                      true); /* fill default parameter */
    store_freg_to_ir1(src_hi, ir1_get_opnd(pir1, 0), true,
                      true); /* fill default parameter */
    return true;
#endif
}

bool translate_movntps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movntps(pir1);
#else
    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src = ir1_get_opnd(pir1, 1);
    if (ir1_opnd_is_xmm(src)) {
        IR2_OPND src_ir2 = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
        store_freg128_to_ir1_mem(src_ir2, dest);
        return true;
    }
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    IR2_OPND src_hi = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, true,
                                           true); /* fill default parameter */
    store_freg_to_ir1(src_lo, ir1_get_opnd(pir1, 0), false,
                      true); /* fill default parameter */
    store_freg_to_ir1(src_hi, ir1_get_opnd(pir1, 0), true,
                      true); /* fill default parameter */
    return true;
#endif
}

bool translate_movntq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movntq(pir1);
#else
    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src = ir1_get_opnd(pir1, 1);
    if (ir1_opnd_is_xmm(src)) {
        IR2_OPND src_ir2 = ra_alloc_xmm(ir1_opnd_base_reg_num(src));
        store_freg_to_ir1(src_ir2, dest, false, false);
        return true;
    }
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    store_freg_to_ir1(src_lo, ir1_get_opnd(pir1, 0), false,
                      true); /* fill default parameter */
    return true;
#endif
}

bool translate_movq2dq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movq2dq(pir1);
#else
    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src = ir1_get_opnd(pir1, 1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    la_append_ir2_opnd2i(LISA_XVPICKVE_D,
                      ra_alloc_xmm(ir1_opnd_base_reg_num(dest)),
                      ra_alloc_mmx(ir1_opnd_base_reg_num(src)), 0);
    //TODO:zero fpu top and tag word
    return true;
#endif
}

bool translate_pmovmskb(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pmovmskb(pir1);
#else
    IR1_OPND* dest = ir1_get_opnd(pir1, 0);
    IR1_OPND* src = ir1_get_opnd(pir1, 1);
    IR2_OPND ftemp = ra_alloc_ftemp();
    if(ir1_opnd_is_xmm(src)){
        la_append_ir2_opnd2(LISA_VMSKLTZ_B, ftemp,
            ra_alloc_xmm(ir1_opnd_base_reg_num(src)));
        la_append_ir2_opnd2_em(LISA_MOVFR2GR_D,
            ra_alloc_gpr(ir1_opnd_base_reg_num(dest)), ftemp);
    } else { //mmx
        IR2_OPND itemp = ra_alloc_itemp();
        la_append_ir2_opnd2(LISA_VMSKLTZ_B, ftemp,
            ra_alloc_mmx(ir1_opnd_base_reg_num(src)));
        la_append_ir2_opnd2_em(LISA_MOVFR2GR_D, itemp, ftemp);
        la_append_ir2_opnd2i_em(LISA_ANDI, itemp, itemp, 0xff);
        store_ireg_to_ir1(itemp, dest, false);
        ra_free_temp(itemp);
        ra_free_temp(ftemp);
    }
    return true;
#endif
}

bool translate_maskmovq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_maskmovq(pir1);
#else
    IR2_OPND src = ra_alloc_ftemp();
    IR2_OPND mask = ra_alloc_ftemp();
    load_freg_from_ir1_2(src, ir1_get_opnd(pir1, 0), IS_DEST_MMX);
    load_freg_from_ir1_2(mask, ir1_get_opnd(pir1, 1), IS_DEST_MMX);
    IR2_OPND zero = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_VXOR_V, zero, zero, zero);
    /*
     * Mapping to LA 23 -> 30
     */
    IR2_OPND base_opnd = ir2_opnd_new(IR2_OPND_IREG, 30);
    lsassert(cpu_get_guest_base() == 0);
    IR2_OPND temp_mask = ra_alloc_ftemp();
    la_append_ir2_opnd2i(LISA_VANDI_B, temp_mask, mask, 0x80);
    IR2_OPND mem_mask = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_VSEQ_B, mem_mask, temp_mask, zero);
    la_append_ir2_opnd3(LISA_VNOR_V, temp_mask, mem_mask, zero);
    IR2_OPND mem_data = ra_alloc_ftemp();
    IR2_OPND xmm_data = ra_alloc_ftemp();
    la_append_ir2_opnd2i(LISA_FLD_D, mem_data, base_opnd, 0);
    la_append_ir2_opnd3(LISA_VAND_V, xmm_data, src, temp_mask);
    la_append_ir2_opnd3(LISA_VAND_V, mem_data, mem_data, mem_mask);
    la_append_ir2_opnd3(LISA_VOR_V, mem_data, mem_data, xmm_data);
    la_append_ir2_opnd2i(LISA_FST_D, mem_data, base_opnd, 0);
    return true;
#endif
}

bool translate_maskmovdqu(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_maskmovdqu(pir1);
#else
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND mask = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND zero = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_VXOR_V, zero, zero, zero);
    /*
     * Mapping to LA 23 -> 30
     */
    IR2_OPND base_opnd = ir2_opnd_new(IR2_OPND_IREG, 30);
    lsassert(cpu_get_guest_base() == 0);
    IR2_OPND temp_mask = ra_alloc_ftemp();
    la_append_ir2_opnd2i(LISA_VANDI_B, temp_mask, mask, 0x80);
    IR2_OPND mem_mask = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_VSEQ_B, mem_mask, temp_mask, zero);
    la_append_ir2_opnd3(LISA_VNOR_V, temp_mask, mem_mask, zero);
    IR2_OPND mem_data = ra_alloc_ftemp();
    IR2_OPND xmm_data = ra_alloc_ftemp();
    la_append_ir2_opnd2i(LISA_VLD, mem_data, base_opnd, 0);
    la_append_ir2_opnd3(LISA_VAND_V, xmm_data, src, temp_mask);
    la_append_ir2_opnd3(LISA_VAND_V, mem_data, mem_data, mem_mask);
    la_append_ir2_opnd3(LISA_VOR_V, mem_data, mem_data, xmm_data);
    la_append_ir2_opnd2i(LISA_VST, mem_data, base_opnd, 0);
    return true;
#endif
}

bool translate_movupd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movupd(pir1);
#else
    translate_movaps(pir1);
    return true;
#endif
}

bool translate_movdqa(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movdqa(pir1);
#else
    translate_movaps(pir1);
    return true;
#endif
}

bool translate_movdqu(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movdqu(pir1);
#else
    translate_movaps(pir1);
    return true;
#endif
}

bool translate_movups(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movups(pir1);
#else
    translate_movaps(pir1);
    return true;
#endif
}

bool translate_movapd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movapd(pir1);
#else
    translate_movaps(pir1);
    return true;
#endif
}
bool translate_lddqu(IR1_INST *pir1)
{
    translate_movaps(pir1);
    return true;
}
bool translate_movaps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movaps(pir1);
#else
    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src = ir1_get_opnd(pir1, 1);
    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_mem(src)) {
        load_freg128_from_ir1_mem(ra_alloc_xmm(ir1_opnd_base_reg_num(dest)),
                                  src);
    } else if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
        store_freg128_to_ir1_mem(ra_alloc_xmm(ir1_opnd_base_reg_num(src)),
                                 dest);
    } else if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_xmm(src)) {
        la_append_ir2_opnd2i(LISA_VORI_B,
                         ra_alloc_xmm(ir1_opnd_base_reg_num(dest)),
                         ra_alloc_xmm(ir1_opnd_base_reg_num(src)), 0);
    } else {
        lsassert(0);
    }
    return true;
#endif
}

bool translate_movhlps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movhlps(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VILVH_D, dest, dest, src);
    return true;
#endif
}

bool translate_movshdup(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1)) ||
             ir1_opnd_is_mem(ir1_get_opnd(pir1, 1)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_XVPACKOD_W, dest, src, src);
    return true;
}

bool translate_movlhps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movlhps(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VILVL_D, dest, src, dest);
    return true;
#endif
}

bool translate_movsd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movsd(pir1);
#else
    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src = ir1_get_opnd(pir1, 1);
    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_mem(src)) {
        IR2_OPND temp = load_freg_from_ir1_1(src, false, false);
        la_append_ir2_opnd2i(LISA_XVPICKVE_D,
                          ra_alloc_xmm(ir1_opnd_base_reg_num(dest)), temp,
                          0);
    } else if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
        store_freg_to_ir1(ra_alloc_xmm(ir1_opnd_base_reg_num(src)), dest,
                          false, false);
    } else if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_xmm(src)) {
        la_append_ir2_opnd2i(LISA_XVINSVE0_D,
                          ra_alloc_xmm(ir1_opnd_base_reg_num(dest)),
                          ra_alloc_xmm(ir1_opnd_base_reg_num(src)), 0);
    }
    return true;
#endif
}

bool translate_movss(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movss(pir1);
#else
    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src = ir1_get_opnd(pir1, 1);
    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_mem(src)) {
        IR2_OPND temp = load_freg_from_ir1_1(src, false, false);
        la_append_ir2_opnd2i(LISA_XVPICKVE_W,
                          ra_alloc_xmm(ir1_opnd_base_reg_num(dest)), temp,
                          0);
        return true;
    } else if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
        store_freg_to_ir1(ra_alloc_xmm(ir1_opnd_base_reg_num(src)), dest,
                          false, false);
        return true;
    } else if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_xmm(src)) {
        la_append_ir2_opnd2i(LISA_XVINSVE0_W,
                          ra_alloc_xmm(ir1_opnd_base_reg_num(dest)),
                          ra_alloc_xmm(ir1_opnd_base_reg_num(src)), 0);
        return true;
    }
    if (ir1_opnd_is_xmm(dest) || ir1_opnd_is_xmm(src)){
        lsassert(0);
    }
    return true;
#endif
}

bool translate_movhpd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movhpd(pir1);
#else
    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src = ir1_get_opnd(pir1, 1);
    if (ir1_opnd_is_mem(src) && ir1_opnd_is_xmm(dest)) {
        IR2_OPND temp = load_ireg_from_ir1(src, ZERO_EXTENSION, false);
        la_append_ir2_opnd2i(LISA_VINSGR2VR_D,
                          ra_alloc_xmm(ir1_opnd_base_reg_num(dest)), temp,
                          1);
    } else if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
        IR2_OPND temp = ra_alloc_itemp();
        la_append_ir2_opnd2i(LISA_VPICKVE2GR_D, temp,
                          ra_alloc_xmm(ir1_opnd_base_reg_num(src)), 1);
        store_ireg_to_ir1(temp, dest, false);
    } else {
        lsassert(0);
    }
    return true;
#endif
}

bool translate_movhps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movhps(pir1);
#else
    translate_movhpd(pir1);
    return true;
#endif
}

bool translate_movlpd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movlpd(pir1);
#else
    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src = ir1_get_opnd(pir1, 1);
    if (ir1_opnd_is_mem(src) && ir1_opnd_is_xmm(dest)) {
        IR2_OPND temp = load_ireg_from_ir1(src, ZERO_EXTENSION, false);
        la_append_ir2_opnd2i(LISA_VINSGR2VR_D,
                          ra_alloc_xmm(ir1_opnd_base_reg_num(dest)), temp,
                          0);
        return true;
    } else if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
        store_freg_to_ir1(ra_alloc_xmm(ir1_opnd_base_reg_num(src)), dest, false, false);
        return true;
    } else {
        lsassert(0);
    }

    if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 0) + 1)) {
        IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        store_freg_to_ir1(src_lo, ir1_get_opnd(pir1, 0), false, true);
    } else {
        IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        store_freg_to_ir1(src_lo, ir1_get_opnd(pir1, 0), false, true);
    }

    return true;
#endif
}

bool translate_movlps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movlps(pir1);
#else
    translate_movlpd(pir1);
    return true;
#endif
}

