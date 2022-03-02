#include "common.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"

/* bool translate_por(IR1_INST *pir1) { return false; } */
/* bool translate_pxor(IR1_INST *pir1) { return false; } */
/* bool translate_packsswb(IR1_INST *pir1) { return false; } */
/* bool translate_packssdw(IR1_INST *pir1) { return false; } */
/* bool translate_packuswb(IR1_INST *pir1) { return false; } */
/* bool translate_paddb(IR1_INST * pir1) { return false; } */
/* bool translate_paddw(IR1_INST * pir1) { return false; } */
/* bool translate_paddd(IR1_INST * pir1) { return false; } */
/* bool translate_paddsb(IR1_INST * pir1) { return false; } */
/* bool translate_paddsw(IR1_INST * pir1) { return false; } */
/* bool translate_paddusb(IR1_INST * pir1) { return false; } */
/* bool translate_paddusw(IR1_INST * pir1) { return false; } */
/* bool translate_pand(IR1_INST * pir1) { return false; } */
/* bool translate_pandn(IR1_INST * pir1) { return false; } */
/* bool translate_pmaddwd(IR1_INST * pir1) { return false; } */
/* bool translate_pmulhuw(IR1_INST * pir1) { return false; } */
/* bool translate_pmulhw(IR1_INST * pir1) { return false; } */
/* bool translate_pmullw(IR1_INST * pir1) { return false; } */
/* bool translate_psubb(IR1_INST * pir1) { return false; } */
/* bool translate_psubw(IR1_INST * pir1) { return false; } */
/* bool translate_psubd(IR1_INST * pir1) { return false; } */
/* bool translate_psubsb(IR1_INST * pir1) { return false; } */
/* bool translate_psubsw(IR1_INST * pir1) { return false; } */
/* bool translate_psubusb(IR1_INST * pir1) { return false; } */
/* bool translate_psubusw(IR1_INST * pir1) { return false; } */
/* bool translate_punpckhbw(IR1_INST * pir1) { return false; } */
/* bool translate_punpckhwd(IR1_INST * pir1) { return false; } */
/* bool translate_punpckhdq(IR1_INST * pir1) { return false; } */
/* bool translate_punpcklbw(IR1_INST * pir1) { return false; } */
/* bool translate_punpckldq(IR1_INST * pir1) { return false; } */
/* bool translate_punpcklwd(IR1_INST * pir1) { return false; } */
/* bool translate_addsd(IR1_INST * pir1) { return false; } */
/* bool translate_addps(IR1_INST * pir1) { return false; } */
/* bool translate_addss(IR1_INST * pir1) { return false; } */
/* bool translate_andps(IR1_INST * pir1) { return false; } */
/* bool translate_andnps(IR1_INST * pir1) { return false; } */
/* bool translate_andnpd(IR1_INST * pir1) { return false; } */
/* bool translate_divpd(IR1_INST * pir1) { return false; } */
/* bool translate_divps(IR1_INST * pir1) { return false; } */
/* bool translate_divsd(IR1_INST * pir1) { return false; } */
/* bool translate_maxpd(IR1_INST * pir1) { return false; } */
/* bool translate_divss(IR1_INST * pir1) { return false; } */
/* bool translate_maxps(IR1_INST * pir1) { return false; } */
/* bool translate_maxss(IR1_INST * pir1) { return false; } */
/* bool translate_maxsd(IR1_INST * pir1) { return false; } */
/* bool translate_minpd(IR1_INST * pir1) { return false; } */
/* bool translate_minps(IR1_INST * pir1) { return false; } */
/* bool translate_minsd(IR1_INST * pir1) { return false; } */
/* bool translate_minss(IR1_INST * pir1) { return false; } */
/* bool translate_mulpd(IR1_INST * pir1) { return false; } */
/* bool translate_mulps(IR1_INST * pir1) { return false; } */
/* bool translate_mulsd(IR1_INST * pir1) { return false; } */
/* bool translate_mulss(IR1_INST * pir1) { return false; } */
/* bool translate_orpd(IR1_INST * pir1) { return false; } */
/* bool translate_orps(IR1_INST * pir1) { return false; } */
/* bool translate_paddq(IR1_INST * pir1) { return false; } */
/* bool translate_pavgb(IR1_INST * pir1) { return false; } */
/* bool translate_pavgw(IR1_INST * pir1) { return false; } */
/* bool translate_pextrw(IR1_INST * pir1) { return false; } */
/* bool translate_pinsrw(IR1_INST * pir1) { return false; } */
/* bool translate_pmaxsw(IR1_INST * pir1) {    return false; } */
/* bool translate_pmaxub(IR1_INST * pir1) { return false; } */
/* bool translate_pminsw(IR1_INST * pir1) {   return false; } */
/* bool translate_pminub(IR1_INST * pir1) { return false; } */
/* bool translate_pmuludq(IR1_INST * pir1) { return false; } */
/* bool translate_psadbw(IR1_INST * pir1) { return false; } */
/* bool translate_pshufd(IR1_INST * pir1) { return false; } */
/* bool translate_pshufw(IR1_INST * pir1) { return false; } */
/* bool translate_pshuflw(IR1_INST * pir1) { return false; } */
/* bool translate_pshufhw(IR1_INST * pir1) { return false; } */
/* bool translate_psubq(IR1_INST * pir1) { return false; } */
/* bool translate_punpckhqdq(IR1_INST * pir1) { return false; } */
/* bool translate_rcpss(IR1_INST * pir1) { return false; } */
/* bool translate_rcpps(IR1_INST * pir1) { return false; } */
/* bool translate_rsqrtss(IR1_INST * pir1) { return false; } */
/* bool translate_rsqrtps(IR1_INST * pir1) { return false; } */
/* bool translate_sqrtpd(IR1_INST * pir1) { return false; } */
/* bool translate_sqrtps(IR1_INST * pir1) { return false; } */
/* bool translate_addpd(IR1_INST *pir1)      */
/* bool translate_andpd(IR1_INST *pir1)      */
/* bool translate_unpcklps(IR1_INST *pir1)   */
/* bool translate_unpcklpd(IR1_INST *pir1)   */
/* bool translate_unpckhpd(IR1_INST *pir1)   */
/* bool translate_unpckhps(IR1_INST *pir1)   */
/* bool translate_shufps(IR1_INST *pir1)     */
/* bool translate_shufpd(IR1_INST *pir1)     */
/* bool translate_punpcklqdq(IR1_INST *pir1) */
/* bool translate_xorps(IR1_INST *pir1)      */
/* bool translate_xorpd(IR1_INST *pir1)      */
/* bool translate_subss(IR1_INST *pir1)      */
/* bool translate_subsd(IR1_INST *pir1)      */
/* bool translate_subps(IR1_INST *pir1)      */
/* bool translate_subpd(IR1_INST *pir1)      */
/* bool translate_sqrtsd(IR1_INST *pir1)     */
/* bool translate_sqrtss(IR1_INST *pir1)     */
/* bool translate_pause(IR1_INST *pir1)      */

bool translate_por(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_por(pir1);
#else

    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VOR_V, dest, dest, src);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    la_append_ir2_opnd3(LISA_VOR_V, dest_lo, dest_lo, src_lo);
    return true;

#endif
}

bool translate_pxor(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pxor(pir1);
#else

    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src = ir1_get_opnd(pir1, 1);
    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_mem(src)) {
        IR2_OPND temp = ra_alloc_ftemp();
        load_freg128_from_ir1_mem(temp, src);
        la_append_ir2_opnd3(LISA_VXOR_V,
                         ra_alloc_xmm(ir1_opnd_base_reg_num(dest)),
                         ra_alloc_xmm(ir1_opnd_base_reg_num(dest)), temp);
        return true;
    } else if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_xmm(src)) {
        la_append_ir2_opnd3(LISA_VXOR_V,
                         ra_alloc_xmm(ir1_opnd_base_reg_num(dest)),
                         ra_alloc_xmm(ir1_opnd_base_reg_num(dest)),
                         ra_alloc_xmm(ir1_opnd_base_reg_num(src)));
        return true;
    }

    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            IS_CONVERT | IS_DEST_MMX);
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           IS_CONVERT | IS_DEST_MMX);
    la_append_ir2_opnd3(LISA_VXOR_V, dest_lo, dest_lo, src_lo);
    return true;

#endif
}

bool translate_packsswb(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_packsswb(pir1);
#else
    if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))) {
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1)) &&
            (ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)) ==
             ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 1)))) {
            la_append_ir2_opnd2i(LISA_VSSRANI_B_H, dest, dest, 0);
        } else {
            la_append_ir2_opnd2i(LISA_VSSRANI_B_H, dest, dest, 0);
            IR2_OPND temp = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VSSRANI_B_H, temp, src, 0);
            la_append_ir2_opnd2i(LISA_VEXTRINS_D, dest, temp, VEXTRINS_IMM_4_0(1, 0));
        }
    } else { //mmx
        IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                             true); /* fill default parameter */
        IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                            true); /* fill default parameter */
        la_append_ir2_opnd3(LISA_VPACKEV_D, dest, src, dest);
        la_append_ir2_opnd2i(LISA_VSSRANI_B_H, dest, dest, 0);
    }
    return true;
#endif
}

bool translate_packssdw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_packssdw(pir1);
#else
    if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))) {
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1)) &&
            (ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)) ==
             ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 1)))) {
            la_append_ir2_opnd2i(LISA_VSSRANI_H_W, dest, dest, 0);
        } else {
            la_append_ir2_opnd2i(LISA_VSSRANI_H_W, dest, dest, 0);
            IR2_OPND temp = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VSSRANI_H_W, temp, src, 0);
            la_append_ir2_opnd2i(LISA_VEXTRINS_D, dest, temp, VEXTRINS_IMM_4_0(1, 0));
        }
    } else { //mmx
        IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                             true); /* fill default parameter */
        IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                            true); /* fill default parameter */
        la_append_ir2_opnd3(LISA_VPACKEV_D, dest, src, dest);
        la_append_ir2_opnd2i(LISA_VSSRANI_H_W, dest, dest, 0);
    }
    return true;
#endif
}

bool translate_packuswb(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_packuswb(pir1);
#else

    if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))) {
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1)) &&
            (ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)) ==
             ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 1)))) {
            la_append_ir2_opnd2i(LISA_VSSRANI_BU_H, dest, dest, 0);
            la_append_ir2_opnd2i(LISA_VEXTRINS_D, dest, dest, VEXTRINS_IMM_4_0(1, 0));
        } else {
            la_append_ir2_opnd2i(LISA_VSSRANI_BU_H, dest, dest, 0);
            IR2_OPND temp = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VSSRANI_BU_H, temp, src, 0);
            la_append_ir2_opnd2i(LISA_VEXTRINS_D, dest, temp, VEXTRINS_IMM_4_0(1, 0));
        }
    } else { //mmx
        IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                             true); /* fill default parameter */
        IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                            true); /* fill default parameter */
        la_append_ir2_opnd3(LISA_VPACKEV_D, dest, src, dest);
        la_append_ir2_opnd2i(LISA_VSSRANI_BU_H, dest, dest, 0);
    }
    return true;

#endif
}

bool translate_paddb(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_paddb(pir1);
#else

    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VADD_B, dest, dest, src);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    la_append_ir2_opnd3(LISA_VADD_B, dest_lo, dest_lo, src_lo);
    return true;

#endif
}

bool translate_paddw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_paddw(pir1);
#else

    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VADD_H, dest, dest, src);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    la_append_ir2_opnd3(LISA_VADD_H, dest_lo, dest_lo, src_lo);
    return true;

#endif
}

bool translate_paddd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_paddd(pir1);
#else

    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VADD_W, dest, dest, src);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    la_append_ir2_opnd3(LISA_VADD_W, dest_lo, dest_lo, src_lo);
    return true;

#endif
}

bool translate_paddsb(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_paddsb(pir1);
#else
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VSADD_B, dest, dest, src);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    la_append_ir2_opnd3(LISA_VSADD_B, dest_lo, dest_lo, src_lo);
    return true;
#endif
}

bool translate_paddsw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_paddsw(pir1);
#else
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VSADD_H, dest, dest, src);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    la_append_ir2_opnd3(LISA_VSADD_H, dest_lo, dest_lo, src_lo);
    return true;
#endif
}

bool translate_paddusb(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_paddusb(pir1);
#else

    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VSADD_BU, dest, dest, src);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    la_append_ir2_opnd3(LISA_VSADD_BU, dest_lo, dest_lo, src_lo);
    return true;

#endif
}

bool translate_paddusw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_paddusw(pir1);
#else

    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VSADD_HU, dest, dest, src);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    la_append_ir2_opnd3(LISA_VSADD_HU, dest_lo, dest_lo, src_lo);
    return true;

#endif
}

bool translate_pand(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pand(pir1);
#else

    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VAND_V, dest, dest, src);
        return true;
    }

    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    la_append_ir2_opnd3(LISA_VAND_V, dest_lo, dest_lo, src_lo);
    return true;

#endif
}

bool translate_pandn(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pandn(pir1);
#else

    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VANDN_V, dest, dest, src);
        return true;
    }

    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    la_append_ir2_opnd3(LISA_VANDN_V, dest_lo, dest_lo, src_lo);
    return true;

#endif
}

bool translate_pmaddwd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pmaddwd(pir1);
#else
    IR2_OPND temp = ra_alloc_ftemp();
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VXOR_V, temp, temp, temp);
        la_append_ir2_opnd3(LISA_VMADDWEV_W_H, temp, dest, src);
        la_append_ir2_opnd3(LISA_VMADDWOD_W_H, temp, dest, src);
        la_append_ir2_opnd2i(LISA_VBSLL_V, dest, temp, 0);
    } else { //mmx
        IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                                true); /* fill default parameter */
        IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                               true); /* fill default parameter */
        la_append_ir2_opnd3(LISA_VXOR_V, temp, temp, temp);
        la_append_ir2_opnd3(LISA_VMADDWEV_W_H, temp, dest_lo, src_lo);
        la_append_ir2_opnd3(LISA_VMADDWOD_W_H, temp, dest_lo, src_lo);
        la_append_ir2_opnd2i(LISA_VBSLL_V, dest_lo, temp, 0);
    }
    return true;
#endif
}

bool translate_pmulhuw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pmulhuw(pir1);
#else
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VMUH_HU, dest, dest, src);
    } else { //mmx
        IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                             true); /* fill default parameter */
        IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                            true); /* fill default parameter */
        la_append_ir2_opnd3(LISA_VMUH_HU, dest, dest, src);
    }
    return true;
#endif
}

bool translate_pmulhw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pmulhw(pir1);
#else
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VMUH_H, dest, dest, src);
    } else { //mmx
        IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                             true); /* fill default parameter */
        IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                            true); /* fill default parameter */
        la_append_ir2_opnd3(LISA_VMUH_H, dest, dest, src);
    }
    return true;
#endif
}

bool translate_pmullw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pmullw(pir1);
#else

    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VMUL_H, dest, dest, src);
    } else { //mmx
        IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                             true); /* fill default parameter */
        IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                            true); /* fill default parameter */
        la_append_ir2_opnd3(LISA_VMUL_H, dest, dest, src);
    }
    return true;

#endif
}

bool translate_psubb(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_psubb(pir1);
#else

    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VSUB_B, dest, dest, src);
    } else { //mmx
        IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                             true); /* fill default parameter */
        IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                            true); /* fill default parameter */
        la_append_ir2_opnd3(LISA_VSUB_B, dest, dest, src);
    }
    return true;

#endif
}

bool translate_psubw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_psubw(pir1);
#else

    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VSUB_H, dest, dest, src);
    } else { //mmx
        IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                             true); /* fill default parameter */
        IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                            true); /* fill default parameter */
        la_append_ir2_opnd3(LISA_VSUB_H, dest, dest, src);
    }
    return true;

#endif
}

bool translate_psubd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_psubd(pir1);
#else

    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VSUB_W, dest, dest, src);
    } else { //mmx
        IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                             true); /* fill default parameter */
        IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                            true); /* fill default parameter */
        la_append_ir2_opnd3(LISA_VSUB_W, dest, dest, src);
    }
    return true;

#endif
}

bool translate_psubsb(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_psubsb(pir1);
#else
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VSSUB_B, dest, dest, src);
        return true;
    } else { //mmx
        IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                             true); /* fill default parameter */
        IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                            true); /* fill default parameter */
        la_append_ir2_opnd3(LISA_VSSUB_B, dest, dest, src);
    }
    return true;
#endif
}

bool translate_psubsw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_psubsw(pir1);
#else
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VSSUB_H, dest, dest, src);
    } else { //mmx
        IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                             true); /* fill default parameter */
        IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                            true); /* fill default parameter */
        la_append_ir2_opnd3(LISA_VSSUB_H, dest, dest, src);
    }
    return true;
#endif
}

bool translate_psubusb(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_psubusb(pir1);
#else
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VSSUB_BU, dest, dest, src);
    } else { //mmx
        IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                             true); /* fill default parameter */
        IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                            true); /* fill default parameter */
        la_append_ir2_opnd3(LISA_VSSUB_BU, dest, dest, src);
    }
    return true;
#endif
}

bool translate_psubusw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_psubusw(pir1);
#else
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VSSUB_HU, dest, dest, src);
    } else { //mmx
        IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                             true); /* fill default parameter */
        IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                            true); /* fill default parameter */
        la_append_ir2_opnd3(LISA_VSSUB_HU, dest, dest, src);
    }
    return true;
#endif
}

bool translate_punpckhbw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_punpckhbw(pir1);
#else

    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VILVH_B, dest, src, dest);
    } else { //mmx
        IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                             true); /* fill default parameter */
        IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                            true); /* fill default parameter */
        la_append_ir2_opnd3(LISA_VILVL_B, dest, src, dest);
        la_append_ir2_opnd2i(LISA_VEXTRINS_D, dest, dest, VEXTRINS_IMM_4_0(0, 1));
    }
    return true;

#endif
}

bool translate_punpckhwd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_punpckhwd(pir1);
#else

    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VILVH_H, dest, src, dest);
    } else { //mmx
        IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                             true); /* fill default parameter */
        IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                            true); /* fill default parameter */
        la_append_ir2_opnd3(LISA_VILVL_H, dest, src, dest);
        la_append_ir2_opnd2i(LISA_VEXTRINS_D, dest, dest, VEXTRINS_IMM_4_0(0, 1));
    }
    return true;

#endif
}

bool translate_punpckhdq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_punpckhdq(pir1);
#else

    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VILVH_W, dest, src, dest);
    } else {
        IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                             true); /* fill default parameter */
        IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                            true); /* fill default parameter */
        la_append_ir2_opnd3(LISA_VILVL_W, dest, src, dest);
        la_append_ir2_opnd2i(LISA_VEXTRINS_D, dest, dest, VEXTRINS_IMM_4_0(0, 1));
    }
    return true;

#endif
}

bool translate_punpcklbw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_punpcklbw(pir1);
#else

    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VILVL_B, dest, src, dest);
    } else { //mmx
        IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                             true); /* fill default parameter */
        IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, false);
        la_append_ir2_opnd3(LISA_VILVL_B, dest, src, dest);
    }
    return true;

#endif
}

bool translate_punpcklwd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_punpcklwd(pir1);
#else

    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VILVL_H, dest, src, dest);
    } else { //mmx
        IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                             true); /* fill default parameter */
        IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, false);
        la_append_ir2_opnd3(LISA_VILVL_H, dest, src, dest);
    }
    return true;

#endif
}

bool translate_punpckldq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_punpckldq(pir1);
#else

    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VILVL_W, dest, src, dest);
        return true;
    } else { //mmx
        IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                             true); /* fill default parameter */
        IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, false);
        la_append_ir2_opnd3(LISA_VILVL_W, dest, src, dest);
    }
    return true;

#endif
}

bool translate_addps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_addps(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VFADD_S, dest, dest, src);
    return true;
#endif
}

bool translate_addsd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_addsd(pir1);
#else

    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_FADD_D, temp, dest, src);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 0);
    return true;

#endif
}

bool translate_addss(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_addss(pir1);
#else

    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_FADD_S, temp, dest, src);
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp, 0);
    return true;

#endif
}

bool translate_andnpd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_andnpd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VANDN_V, dest, dest, src);
    return true;
#endif
}

bool translate_andnps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_andnps(pir1);
#else
    translate_andnpd(pir1);
    return true;
#endif
}

bool translate_andps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_andps(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VAND_V, dest, dest, src);
    return true;
#endif
}

bool translate_divpd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_divpd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VFDIV_D, dest, dest, src);
    return true;
#endif
}

bool translate_divps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_divps(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VFDIV_S, dest, dest, src);
    return true;
#endif
}

bool translate_divsd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_divsd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_FDIV_D, temp, dest, src);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 0);
    return true;
#endif
}

bool translate_divss(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_divss(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_FDIV_S, temp, dest, src);
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp, 0);
    return true;
#endif
}

bool translate_maxpd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_maxpd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VFMAX_D, dest, dest, src);
    return true;
#endif
}

bool translate_maxps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_maxps(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VFMAX_S, dest, dest, src);
    return true;
#endif
}

bool translate_maxsd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_maxsd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_VFMAX_D, temp, dest, src);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 0);
    return true;
#endif
}

bool translate_maxss(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_maxss(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_VFMAX_S, temp, dest, src);
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp, 0);
    return true;
#endif
}

bool translate_minpd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_minpd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VFMIN_D, dest, dest, src);
    return true;
#endif
}

bool translate_minps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_minps(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VFMIN_S, dest, dest, src);
    return true;
#endif
}

bool translate_minsd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_minsd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_VFMIN_D, temp, dest, src);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 0);
    return true;
#endif
}

bool translate_minss(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_minss(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_VFMIN_S, temp, dest, src);
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp, 0);
    return true;
#endif
}

bool translate_mulpd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_mulpd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VFMUL_D, dest, dest, src);
    return true;
#endif
}

bool translate_mulps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_mulps(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VFMUL_S, dest, dest, src);
    return true;
#endif
}

bool translate_mulsd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_mulsd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_FMUL_D, temp, dest, src);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 0);
    return true;
#endif
}

bool translate_mulss(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_mulss(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_FMUL_S, temp, dest, src);
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp, 0);
    return true;
#endif
}

bool translate_orpd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_orpd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VOR_V, dest, dest, src);
    return true;
#endif
}

bool translate_orps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_orps(pir1);
#else
    translate_orpd(pir1);
    return true;
#endif
}

bool translate_paddq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_paddq(pir1);
#else
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VADD_D, dest, dest, src);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            IS_CONVERT | IS_DEST_MMX);
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           IS_CONVERT | IS_DEST_MMX);
    la_append_ir2_opnd3(LISA_VADD_D, dest_lo, dest_lo, src_lo);
    return true;
#endif
}

bool translate_pavgb(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pavgb(pir1);
#else
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VAVGR_BU, dest, dest, src);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    la_append_ir2_opnd3(LISA_VAVGR_BU, dest_lo, dest_lo, src_lo);
    return true;
#endif
}

bool translate_pavgw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pavgw(pir1);
#else
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VAVGR_HU, dest, dest, src);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    la_append_ir2_opnd3(LISA_VAVGR_HU, dest_lo, dest_lo, src_lo);
    return true;
#endif
}

bool translate_pextrw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pextrw(pir1);
#else
    if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1))) {
        uint8_t imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 2));
        imm &= 7;
        la_append_ir2_opnd2i(LISA_VPICKVE2GR_HU,
            ra_alloc_gpr(ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0))),
            ra_alloc_xmm(ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 1))),
            imm);
        return true;
    }
    uint8 imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 0) + 2);
    IR2_OPND src_lo = load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION,
                           false); /* fill default parameter */
    uint8 select = imm & 0x3;
    switch (select) {
    case 0:
        break;
    case 1:
        la_append_ir2_opnd2i(LISA_SRLI_D, src_lo, src_lo, 0x10);
        break;
    case 2:
        la_append_ir2_opnd2i(LISA_SRLI_D, src_lo, src_lo, 0x20);
        break;
    case 3:
        la_append_ir2_opnd2i(LISA_SRLI_D, src_lo, src_lo, 0x30);
        break;
    default:
        fprintf(stderr, "1: invalid imm8<0:1> in PEXTRW : %d\n", select);
        exit(-1);
    }
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, src_lo, src_lo, 15, 0);
    store_ireg_to_ir1(src_lo, ir1_get_opnd(pir1, 0), false); /* fill default */
                                                   /* parameter */
    return true;
#endif
}

bool translate_pinsrw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pinsrw(pir1);
#else
    if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))) {
        uint8_t imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 2));
        imm &= 7;
        IR2_OPND src = load_ireg_from_ir1(ir1_get_opnd(pir1, 1),
                                          UNKNOWN_EXTENSION, false);
        la_append_ir2_opnd2i(LISA_VINSGR2VR_H,
            ra_alloc_xmm(ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0))), src,
            imm);
        return true;
    }
    IR2_OPND src = load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION,
                                      false); /* fill default parameter */
    IR2_OPND ftemp = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_MOVGR2FR_D, ftemp, src);
    uint8 imm8 = ir1_opnd_uimm(ir1_get_opnd(pir1, 0) + 2);

    IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                         true); /* fill default parameter */
    uint8 select = imm8 & 0x3;
    switch (select) {
    case 0:
        la_append_ir2_opnd2i(LISA_VEXTRINS_H, dest, ftemp, VEXTRINS_IMM_4_0(0, 0));
        break;
    case 1:
        la_append_ir2_opnd2i(LISA_VEXTRINS_H, dest, ftemp, VEXTRINS_IMM_4_0(1, 0));
        break;
    case 2:
        la_append_ir2_opnd2i(LISA_VEXTRINS_H, dest, ftemp, VEXTRINS_IMM_4_0(2, 0));
        break;
    case 3:
        la_append_ir2_opnd2i(LISA_VEXTRINS_H, dest, ftemp, VEXTRINS_IMM_4_0(3, 0));
        break;
    default:
        fprintf(stderr, "1: invalid imm8<0:1> in PINSRW : %d\n", select);
        exit(-1);
    }
    ra_free_temp(ftemp);
    return true;
#endif
}

bool translate_pmaxsw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pmaxsw(pir1);
#else
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VMAX_H, dest, dest, src);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    if (ir2_opnd_cmp(&dest_lo, &src_lo))
        return true;
    else
        la_append_ir2_opnd3(LISA_VMAX_H, dest_lo, dest_lo, src_lo);
    return true;
#endif
}

bool translate_pmaxub(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pmaxub(pir1);
#else
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VMAX_BU, dest, dest, src);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    if (ir2_opnd_cmp(&dest_lo, &src_lo))
        return true;
    else
        la_append_ir2_opnd3(LISA_VMAX_BU, dest_lo, dest_lo, src_lo);
    return true;
#endif
}

bool translate_pminsw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pminsw(pir1);
#else
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VMIN_H, dest, dest, src);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    if (ir2_opnd_cmp(&dest_lo, &src_lo))
        return true;
    else
        la_append_ir2_opnd3(LISA_VMIN_H, dest_lo, dest_lo, src_lo);
    return true;
#endif
}

bool translate_pminub(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pminub(pir1);
#else
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VMIN_BU, dest, dest, src);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    if (ir2_opnd_cmp(&dest_lo, &src_lo))
        return true;
    else
        la_append_ir2_opnd3(LISA_VMIN_BU, dest_lo, dest_lo, src_lo);
    return true;
#endif
}

bool translate_pmuludq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pmuludq(pir1);
#else
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VMULWEV_D_WU, dest, dest, src);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            IS_CONVERT | IS_DEST_MMX);
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           IS_CONVERT | IS_DEST_MMX);
    la_append_ir2_opnd3(LISA_VMULWEV_D_WU, dest_lo, dest_lo, src_lo);
    return true;
#endif
}

bool translate_psadbw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_psadbw(pir1);
#else
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VABSD_BU, dest, dest, src);
        la_append_ir2_opnd2(LISA_VHADD8_D_BU, dest, dest);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    la_append_ir2_opnd3(LISA_VABSD_BU, dest_lo, dest_lo, src_lo);
    la_append_ir2_opnd2(LISA_VHADD8_D_BU, dest_lo, dest_lo);
    return true;
#endif
}

bool translate_pshufd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pshufd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    uint64_t imm8 = ir1_opnd_uimm(ir1_get_opnd(pir1, 2));
    la_append_ir2_opnd2i_em(LISA_VSHUF4I_W, dest, src, imm8);
    return true;
#endif
}

bool translate_pshufw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pshufw(pir1);
#else
    IR2_OPND dest = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                         true); /* fill default parameter */
    IR2_OPND src = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, true,
                                        true); /* fill default parameter */
    IR1_OPND *imm8_reg = ir1_get_opnd(pir1, 2);
    uint64_t imm8 = ir1_opnd_uimm(imm8_reg);
    if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 1)) ||
        (ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)) !=
         ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 1)))) {
        la_append_ir2_opnd2i(LISA_VORI_B, dest, src, 0);
    }
    la_append_ir2_opnd2i(LISA_VSHUF4I_H, dest, dest, imm8);
    return true;
#endif
}

bool translate_pshufhw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pshufhw(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR1_OPND *dest_reg = ir1_get_opnd(pir1, 0);
    IR1_OPND *src_reg = ir1_get_opnd(pir1, 1);
    IR1_OPND *imm8_reg = ir1_get_opnd(pir1, 2);

    IR2_OPND dest = load_freg128_from_ir1(dest_reg);
    IR2_OPND src = load_freg128_from_ir1(src_reg);
    IR2_OPND temp = ra_alloc_ftemp();

    uint64_t imm8 = ir1_opnd_uimm(imm8_reg);
    if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 1)) ||
        (ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)) !=
         ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 1)))) {
        la_append_ir2_opnd2i(LISA_VORI_B, dest, src, 0);
    }
    la_append_ir2_opnd2i(LISA_VORI_B, temp, src, 0);
    la_append_ir2_opnd2i(LISA_VSHUF4I_H, dest, dest, imm8);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 0);
    return true;
#endif
}

bool translate_pshuflw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pshuflw(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    uint64_t imm8 = ir1_opnd_uimm(ir1_get_opnd(pir1, 2));
    if (ir1_opnd_is_mem(ir1_get_opnd(pir1, 1)) ||
        (ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0)) !=
         ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 1)))) {
        la_append_ir2_opnd2i(LISA_VORI_B, dest, src, 0);
    }

    la_append_ir2_opnd2i(LISA_VORI_B, temp, src, 0);
    la_append_ir2_opnd2i(LISA_VBSRL_V, temp, temp, 8);
    la_append_ir2_opnd2i(LISA_VSHUF4I_H, dest, dest, imm8);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 1);
    return true;
#endif
}

bool translate_psubq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_psubq(pir1);
#else
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VSUB_D, dest, dest, src);
        return true;
    } else {
        IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                                true); /* fill default parameter */
        IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                               true); /* fill default parameter */
        la_append_ir2_opnd3(LISA_VSUB_D, dest_lo, dest_lo, src_lo);
    }
    return true;
#endif
}

bool translate_punpckhqdq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_punpckhqdq(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VILVH_D, dest, src, dest);
    return true;
#endif
}

bool translate_rcpss(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_rcpss(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_VFRECIP_S, temp, src);
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp, 0);
    return true;
#endif
}

bool translate_rcpps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_rcpps(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd2(LISA_VFRECIP_S, dest, src);
    return true;
#endif
}

bool translate_rsqrtss(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_rsqrtss(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FRSQRT_S, temp, src);
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp, 0);
    return true;
#endif
}

bool translate_rsqrtps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_rsqrtps(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd2(LISA_VFRSQRT_S, dest, src);
    return true;
#endif
}

bool translate_sqrtpd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_sqrtpd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));

    IR2_OPND temp = ra_alloc_ftemp();
    IR2_OPND temp0 = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FSQRT_D, temp0, src);
    la_append_ir2_opnd2i(LISA_VSHUF4I_W, temp, src, 0xee);
    la_append_ir2_opnd2(LISA_FSQRT_D, temp, temp);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 1);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp0, 0);

    return true;
#endif
}

bool translate_sqrtps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_sqrtps(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));

    IR2_OPND temp = ra_alloc_ftemp();
    IR2_OPND temp0 = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_VFSQRT_S, temp0, src);
    la_append_ir2_opnd2i(LISA_VSHUF4I_W, temp, src, 0xee);
    la_append_ir2_opnd2(LISA_VFSQRT_S, temp, temp);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 1);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp0, 0);

    return true;
#endif
}

bool translate_addpd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_addpd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VFADD_D, dest, dest, src);
    return true;
#endif
}

bool translate_andpd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_andpd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VAND_V, dest, dest, src);
    return true;
#endif
}

bool translate_unpcklps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_unpcklps(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VILVL_W, dest, src, dest);
    return true;
#endif
}

bool translate_unpcklpd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_unpcklpd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd2i(LISA_VSHUF4I_D, dest, src, 0x8);
    return true;
#endif
}

bool translate_unpckhpd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_unpckhpd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd2i(LISA_VSHUF4I_D, dest, src, 0xd);
    return true;
#endif
}

bool translate_unpckhps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_unpckhps(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VILVH_W, dest, src, dest);
    return true;
#endif
}

bool translate_shufps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_shufps(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    uint64_t imm8 = ir1_opnd_uimm(ir1_get_opnd(pir1, 2));
    IR2_OPND temp1 = ra_alloc_ftemp();
    IR2_OPND temp2 = ra_alloc_ftemp();
    la_append_ir2_opnd2i(LISA_VSHUF4I_W, temp1, dest, imm8);
    la_append_ir2_opnd2i(LISA_VSHUF4I_W, temp2, src, imm8 >> 4);
    la_append_ir2_opnd3(LISA_VPICKEV_D, dest , temp2, temp1);
    return true;
#endif
}

bool translate_shufpd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_shufpd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    uint8_t imm8 = ir1_opnd_uimm(ir1_get_opnd(pir1, 2));
    imm8 &= 3;
    uint8_t shfd_imm8 = 0; 
    if (imm8 == 0){
        shfd_imm8 = 0x8;
    }
    else if(imm8 == 1) {
        shfd_imm8 = 0x9;
    }
    else if(imm8 == 2) {
        shfd_imm8 = 0xc;
    }
    else if(imm8 == 3) {
        shfd_imm8 = 0xd;
    }
    else {
        lsassert(0);
    }

    la_append_ir2_opnd2i(LISA_VSHUF4I_D, dest, src, shfd_imm8);
    return true;
#endif
}

bool translate_punpcklqdq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_punpcklqdq(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VILVL_D, dest, src, dest);
    return true;
#endif
}

bool translate_xorps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_xorps(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VXOR_V, dest, dest, src);
    return true;
#endif
}

bool translate_xorpd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_xorpd(pir1);
#else
    translate_xorps(pir1);
    return true;
#endif
}

bool translate_subss(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_subss(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_FSUB_S, temp, dest, src);
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp, 0);
    return true;
#endif
}

bool translate_subsd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_subsd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_FSUB_D, temp, dest, src);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 0);
    return true;
#endif
}

bool translate_subps(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_subps(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VFSUB_S, dest, dest, src);
    return true;
#endif
}

bool translate_subpd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_subpd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3(LISA_VFSUB_D, dest, dest, src);
    return true;
#endif
}

bool translate_sqrtsd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_sqrtsd(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FSQRT_D, temp, src);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 0);
    return true;
#endif
}

bool translate_sqrtss(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_sqrtss(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd2(LISA_FSQRT_S, temp, src);
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp, 0);
    return true;
#endif
}

bool translate_pause(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pause(pir1);
#else
    return true;
#endif
}
