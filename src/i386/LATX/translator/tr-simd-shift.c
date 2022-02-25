#include "common.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"

/* bool translate_psllw(IR1_INST * pir1) { return false; } */
/* bool translate_pslld(IR1_INST * pir1) { return false; } */
/* bool translate_psllq(IR1_INST * pir1) { return false; } */
/* bool translate_psrlw(IR1_INST * pir1) { return false; } */
/* bool translate_psrld(IR1_INST * pir1) { return false; } */
/* bool translate_psrlq(IR1_INST * pir1) { return false; } */
/* bool translate_psraw(IR1_INST * pir1) { return false; } */
/* bool translate_psrad(IR1_INST * pir1) { return false; } */
/* bool translate_pslldq(IR1_INST * pir1) { return false; } */
/* bool translate_psrldq(IR1_INST * pir1) { return false; } */

bool translate_psllw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_psllw(pir1);
#else

    if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))) {
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1)) ||
            ir1_opnd_is_mem(ir1_get_opnd(pir1, 1))) {
            IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
            IR2_OPND temp1 = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VREPLVEI_D, temp1, src, 0);
            IR2_OPND temp2 = ra_alloc_ftemp();
            la_append_ir2_opnd1i(LISA_VLDI, temp2, VLDI_IMM_TYPE0(1, 16));
            IR2_OPND temp3 = ra_alloc_ftemp();
            la_append_ir2_opnd3(LISA_VSLT_HU, temp3, temp1, temp2);
            la_append_ir2_opnd2i(LISA_VREPLVEI_H, temp1, src, 0);
            la_append_ir2_opnd3(LISA_VSLL_H, dest, dest, temp1);
            la_append_ir2_opnd3(LISA_VAND_V, dest, dest, temp3);
        } else if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 1))) {
            uint8_t imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 1));
            if (imm > 15) {
                la_append_ir2_opnd3(LISA_VXOR_V, dest, dest, dest);
            } else {
                la_append_ir2_opnd2i(LISA_VSLLI_H, dest, dest, imm);
            }
        } else {
            lsassert(0);
        }
    } else { //mmx
        if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 1)) {
            IR2_OPND dest = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0), false, true); /* fill default parameter */
            uint8 imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 0) + 1);
            if (imm > 15)
                la_append_ir2_opnd3(LISA_VXOR_V, dest, dest, dest);
            else {
                la_append_ir2_opnd2i(LISA_VSLLI_H, dest, dest, imm);
            }
        } else {
            IR2_OPND dest = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0), false, true); /* fill default parameter */
            IR2_OPND src = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0) + 1, false, true); /* fill default parameter */

            IR2_OPND temp1 = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VREPLVEI_D, temp1, src, 0);

            IR2_OPND temp2 = ra_alloc_ftemp();
            la_append_ir2_opnd1i(LISA_VLDI, temp2, VLDI_IMM_TYPE0(1, 16));

            IR2_OPND temp3 = ra_alloc_ftemp();
            la_append_ir2_opnd3(LISA_VSLT_HU, temp3, temp1, temp2);

            la_append_ir2_opnd2i(LISA_VREPLVEI_H, temp1, src, 0);
            la_append_ir2_opnd3(LISA_VSLL_H, dest, dest, temp1);
            la_append_ir2_opnd3(LISA_VAND_V, dest, dest, temp3);
        }
    }
    return true;

#endif
}

bool translate_pslld(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pslld(pir1);
#else

    if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))) {
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1)) ||
            ir1_opnd_is_mem(ir1_get_opnd(pir1, 1))) {
            IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
            IR2_OPND temp1 = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VREPLVEI_D, temp1, src, 0);
            IR2_OPND temp2 = ra_alloc_ftemp();
            la_append_ir2_opnd1i(LISA_VLDI, temp2, VLDI_IMM_TYPE0(2, 32));
            IR2_OPND temp3 = ra_alloc_ftemp();
            la_append_ir2_opnd3(LISA_VSLT_WU, temp3, temp1, temp2);
            la_append_ir2_opnd2i(LISA_VREPLVEI_W, temp1, src, 0);
            la_append_ir2_opnd3(LISA_VSLL_W, dest, dest, temp1);
            la_append_ir2_opnd3(LISA_VAND_V, dest, dest, temp3);
        } else if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 1))) {
            uint8_t imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 1));
            if (imm > 31) {
                la_append_ir2_opnd3(LISA_VXOR_V, dest, dest, dest);
            } else {
                la_append_ir2_opnd2i(LISA_VSLLI_W, dest, dest, imm);
            }
        } else {
            lsassert(0);
        }
    } else { //mmx
        if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 1)) {
            IR2_OPND dest = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0), false, true); /* fill default parameter */
            uint8 imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 0) + 1);
            if (imm > 31)
                la_append_ir2_opnd3(LISA_VXOR_V, dest, dest, dest);
            else {
                la_append_ir2_opnd2i(LISA_VSLLI_W, dest, dest, imm);
            }
        } else {
            IR2_OPND dest = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0), false, true); /* fill default parameter */
            IR2_OPND src = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0) + 1, false, true); /* fill default parameter */

            IR2_OPND temp1 = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VREPLVEI_D, temp1, src, 0);

            IR2_OPND temp2 = ra_alloc_ftemp();
            la_append_ir2_opnd1i(LISA_VLDI, temp2, VLDI_IMM_TYPE0(2, 32));

            IR2_OPND temp3 = ra_alloc_ftemp();
            la_append_ir2_opnd3(LISA_VSLT_WU, temp3, temp1, temp2);

            la_append_ir2_opnd2i(LISA_VREPLVEI_W, temp1, src, 0);
            la_append_ir2_opnd3(LISA_VSLL_W, dest, dest, temp1);
            la_append_ir2_opnd3(LISA_VAND_V, dest, dest, temp3);
        }
    }
    return true;

#endif
}

bool translate_psllq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_psllq(pir1);
#else

    if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))) {
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1)) ||
            ir1_opnd_is_mem(ir1_get_opnd(pir1, 1))) {
            IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
            IR2_OPND temp1 = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VREPLVEI_D, temp1, src, 0);
            IR2_OPND temp2 = ra_alloc_ftemp();
            la_append_ir2_opnd1i(LISA_VLDI, temp2, VLDI_IMM_TYPE0(3, 64));
            IR2_OPND temp3 = ra_alloc_ftemp();
            la_append_ir2_opnd3(LISA_VSLT_DU, temp3, temp1, temp2);
            la_append_ir2_opnd2i(LISA_VREPLVEI_D, temp1, src, 0);
            la_append_ir2_opnd3(LISA_VSLL_D, dest, dest, temp1);
            la_append_ir2_opnd3(LISA_VAND_V, dest, dest, temp3);
        } else if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 1))) {
            uint8_t imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 1));
            if (imm > 63) {
                la_append_ir2_opnd3(LISA_VXOR_V, dest, dest, dest);
            } else {
                la_append_ir2_opnd2i(LISA_VSLLI_D, dest, dest, imm);
            }
        } else {
            lsassert(0);
        }
    } else { //mmx
        if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 1)) {
            IR2_OPND dest = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0), false, true); /* fill default parameter */
            uint8 imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 0) + 1);
            if (imm > 63)
                la_append_ir2_opnd3(LISA_VXOR_V, dest, dest, dest);
            else {
                la_append_ir2_opnd2i(LISA_VSLLI_D, dest, dest, imm);
            }
        } else {
            IR2_OPND temp_imm = ra_alloc_itemp();
            IR2_OPND temp = ra_alloc_itemp();
            IR2_OPND target_label = ir2_opnd_new_type(IR2_OPND_LABEL); 
            IR2_OPND dest = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0), false, true); /* fill default parameter */
            IR2_OPND src = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0) + 1, false, true); /* fill default parameter */
            load_ireg_from_imm64(temp_imm, 0xffffffffffffff80ULL);
            la_append_ir2_opnd2(LISA_MOVFR2GR_D, temp, src);
            la_append_ir2_opnd3_em(LISA_AND, temp_imm, temp_imm, temp);
            la_append_ir2_opnd3(LISA_BEQ, temp_imm, zero_ir2_opnd, target_label);
            la_append_ir2_opnd2(LISA_MOVGR2FR_D, dest, zero_ir2_opnd);
            la_append_ir2_opnd1(LISA_LABEL, target_label);
            la_append_ir2_opnd3(LISA_VSLL_D, dest, dest, src);
            ra_free_temp(temp_imm);
            ra_free_temp(temp);
        }
    }
    return true;

#endif
}

bool translate_psrlw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_psrlw(pir1);
#else

    if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))) {
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1)) ||
            ir1_opnd_is_mem(ir1_get_opnd(pir1, 1))) {
            IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
            IR2_OPND temp1 = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VREPLVEI_D, temp1, src, 0);
            IR2_OPND temp2 = ra_alloc_ftemp();
            la_append_ir2_opnd1i(LISA_VLDI, temp2, VLDI_IMM_TYPE0(1, 16));
            IR2_OPND temp3 = ra_alloc_ftemp();
            la_append_ir2_opnd3(LISA_VSLT_HU, temp3, temp1, temp2);
            la_append_ir2_opnd2i(LISA_VREPLVEI_H, temp1, src, 0);
            la_append_ir2_opnd3(LISA_VSRL_H, dest, dest, temp1);
            la_append_ir2_opnd3(LISA_VAND_V, dest, dest, temp3);
        } else if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 1))) {
            uint8_t imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 1));
            if (imm > 15) {
                la_append_ir2_opnd3(LISA_VXOR_V, dest, dest, dest);
            } else {
                la_append_ir2_opnd2i(LISA_VSRLI_H, dest, dest, imm);
            }
        } else {
            lsassert(0);
        }
    } else { //mmx
        if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 1)) {
            IR2_OPND dest = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0), false, true); /* fill default parameter */
            uint8 imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 0) + 1);
            if (imm > 15)
                la_append_ir2_opnd3(LISA_VXOR_V, dest, dest, dest);
            else {
                la_append_ir2_opnd2i(LISA_VSRLI_H, dest, dest, imm);
            }
        } else {
            IR2_OPND dest = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0), false, true); /* fill default parameter */
            IR2_OPND src = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0) + 1, false, true); /* fill default parameter */

            IR2_OPND temp1 = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VREPLVEI_D, temp1, src, 0);

            IR2_OPND temp2 = ra_alloc_ftemp();
            la_append_ir2_opnd1i(LISA_VLDI, temp2, VLDI_IMM_TYPE0(1, 16));

            IR2_OPND temp3 = ra_alloc_ftemp();
            la_append_ir2_opnd3(LISA_VSLT_HU, temp3, temp1, temp2);

            la_append_ir2_opnd2i(LISA_VREPLVEI_H, temp1, src, 0);
            la_append_ir2_opnd3(LISA_VSRL_H, dest, dest, temp1);
            la_append_ir2_opnd3(LISA_VAND_V, dest, dest, temp3);
        }
    }
    return true;

#endif
}

bool translate_psrld(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_psrld(pir1);
#else

    if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))) {
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1)) ||
            ir1_opnd_is_mem(ir1_get_opnd(pir1, 1))) {
            IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
            IR2_OPND temp1 = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VREPLVEI_D, temp1, src, 0);
            IR2_OPND temp2 = ra_alloc_ftemp();
            la_append_ir2_opnd1i(LISA_VLDI, temp2, VLDI_IMM_TYPE0(2, 32));
            IR2_OPND temp3 = ra_alloc_ftemp();
            la_append_ir2_opnd3(LISA_VSLT_WU, temp3, temp1, temp2);
            la_append_ir2_opnd2i(LISA_VREPLVEI_W, temp1, src, 0);
            la_append_ir2_opnd3(LISA_VSRL_W, dest, dest, temp1);
            la_append_ir2_opnd3(LISA_VAND_V, dest, dest, temp3);
        } else if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 1))) {
            uint8_t imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 1));
            if (imm > 31) {
                la_append_ir2_opnd3(LISA_VXOR_V, dest, dest, dest);
            } else {
                la_append_ir2_opnd2i(LISA_VSRLI_W, dest, dest, imm);
            }
        } else {
            lsassert(0);
        }
    } else { //mmx
        if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 1)) {
            IR2_OPND dest = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0), false, true); /* fill default parameter */
            uint8 imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 0) + 1);
            if (imm > 31)
                la_append_ir2_opnd3(LISA_VXOR_V, dest, dest, dest);
            else {
                la_append_ir2_opnd2i(LISA_VSRLI_W, dest, dest, imm);
            }
        } else {
            IR2_OPND dest = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0), false, true); /* fill default parameter */
            IR2_OPND src = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0) + 1, false, true); /* fill default parameter */

            IR2_OPND temp1 = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VREPLVEI_D, temp1, src, 0);

            IR2_OPND temp2 = ra_alloc_ftemp();
            la_append_ir2_opnd1i(LISA_VLDI, temp2, VLDI_IMM_TYPE0(2, 32));

            IR2_OPND temp3 = ra_alloc_ftemp();
            la_append_ir2_opnd3(LISA_VSLT_WU, temp3, temp1, temp2);

            la_append_ir2_opnd2i(LISA_VREPLVEI_W, temp1, src, 0);
            la_append_ir2_opnd3(LISA_VSRL_W, dest, dest, temp1);
            la_append_ir2_opnd3(LISA_VAND_V, dest, dest, temp3);
        }
    }
    return true;

#endif
}

bool translate_psrlq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_psrlq(pir1);
#else

    if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))) {
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1)) ||
            ir1_opnd_is_mem(ir1_get_opnd(pir1, 1))) {
            IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
            IR2_OPND temp1 = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VREPLVEI_D, temp1, src, 0);
            IR2_OPND temp2 = ra_alloc_ftemp();
            la_append_ir2_opnd1i(LISA_VLDI, temp2, VLDI_IMM_TYPE0(3, 64));
            IR2_OPND temp3 = ra_alloc_ftemp();
            la_append_ir2_opnd3(LISA_VSLT_DU, temp3, temp1, temp2);
            la_append_ir2_opnd3(LISA_VSRL_D, dest, dest, temp1);
            la_append_ir2_opnd3(LISA_VAND_V, dest, dest, temp3);
        } else if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 1))) {
            uint8_t imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 1));
            if (imm > 63) {
                la_append_ir2_opnd3(LISA_VXOR_V, dest, dest, dest);
            } else {
                la_append_ir2_opnd2i(LISA_VSRLI_D, dest, dest, imm);
            }
        } else {
            lsassert(0);
        }
    } else { //mmx
        if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 1)) {
            IR2_OPND dest = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0), false, true); /* fill default parameter */
            uint8 imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 0) + 1);
            if (imm > 63)
                la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, dest, zero_ir2_opnd);
            else {
                IR2_OPND itemp = ra_alloc_itemp();
                load_ireg_from_imm32(itemp, imm, UNKNOWN_EXTENSION);
                la_append_ir2_opnd2_em(LISA_MOVFR2GR_D, itemp, dest);
                la_append_ir2_opnd2i_em(LISA_SRLI_D, itemp, itemp, imm);
                la_append_ir2_opnd2_em(LISA_MOVGR2FR_D, dest, itemp);
                ra_free_temp(itemp);
            }
        } else {
            IR2_OPND dest = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0), false, true); /* fill default parameter */
            IR2_OPND src = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0) + 1, false, true); /* fill default parameter */

            IR2_OPND temp1 = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VREPLVEI_D, temp1, src, 0);

            IR2_OPND temp2 = ra_alloc_ftemp();
            la_append_ir2_opnd1i(LISA_VLDI, temp2, VLDI_IMM_TYPE0(3, 64));

            IR2_OPND temp3 = ra_alloc_ftemp();
            la_append_ir2_opnd3(LISA_VSLT_DU, temp3, temp1, temp2);

            la_append_ir2_opnd2i(LISA_VREPLVEI_D, temp1, src, 0);
            la_append_ir2_opnd3(LISA_VSRL_D, dest, dest, temp1);
            la_append_ir2_opnd3(LISA_VAND_V, dest, dest, temp3);
        }
    }
    return true;

#endif
}

bool translate_psraw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_psraw(pir1);
#else
    if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))) {
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1)) ||
            ir1_opnd_is_mem(ir1_get_opnd(pir1, 1))) {
            IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
            IR2_OPND temp1 = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VREPLVEI_H, temp1, src, 0);
            IR2_OPND temp2 = ra_alloc_ftemp();
            la_append_ir2_opnd1i(LISA_VLDI, temp2, VLDI_IMM_TYPE0(1, 16));
            IR2_OPND temp3 = ra_alloc_ftemp();
            la_append_ir2_opnd3(LISA_VSLT_HU, temp3, temp1, temp2);
            //la_append_ir2_opnd2i(LISA_VREPLVEI_H, temp1, src, 0);
            IR2_OPND temp4 = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VSRAI_H, temp4, dest, 15);
            la_append_ir2_opnd3(LISA_VSRA_H, dest, dest, temp1);
            la_append_ir2_opnd4(LISA_VBITSEL_V, dest, temp4, dest, temp3);
        } else if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 1))) {
            uint8_t imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 1));
            if (imm > 15)
                imm = 15;
            la_append_ir2_opnd2i(LISA_VSRAI_H, dest, dest, imm);
        } else {
            lsassert(0);
        }
    } else { //mmx
        if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 1)) {
            IR2_OPND dest = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0), false, true); /* fill default parameter */
            uint8 imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 0) + 1);
            if (imm > 15)
                imm = 15;
            la_append_ir2_opnd2i(LISA_VSRAI_H, dest, dest, imm);
        } else {
            IR2_OPND dest = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0), false, true); /* fill default parameter */
            IR2_OPND src = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0) + 1, false, true); /* fill default parameter */

            IR2_OPND temp1 = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VREPLVEI_D, temp1, src, 0);

            IR2_OPND temp2 = ra_alloc_ftemp();
            la_append_ir2_opnd1i(LISA_VLDI, temp2, VLDI_IMM_TYPE0(1, 16));

            IR2_OPND temp3 = ra_alloc_ftemp();
            la_append_ir2_opnd3(LISA_VSLT_HU, temp3, temp1, temp2);

            la_append_ir2_opnd2i(LISA_VREPLVEI_H, temp1, src, 0);
            IR2_OPND temp4 = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VSRAI_H, temp4, dest, 15);
            la_append_ir2_opnd3(LISA_VSRA_H, dest, dest, temp1);
            la_append_ir2_opnd4(LISA_VBITSEL_V, dest, temp4, dest, temp3);
        }
    }
    return true;
#endif
}

bool translate_psrad(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_psrad(pir1);
#else
    if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))) {
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1)) ||
            ir1_opnd_is_mem(ir1_get_opnd(pir1, 1))) {
            IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
            IR2_OPND temp1 = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VREPLVEI_D, temp1, src, 0);
            IR2_OPND temp2 = ra_alloc_ftemp();
            la_append_ir2_opnd1i(LISA_VLDI, temp2, VLDI_IMM_TYPE0(2, 32));
            IR2_OPND temp3 = ra_alloc_ftemp();
            la_append_ir2_opnd3(LISA_VSLT_WU, temp3, temp1, temp2);
            la_append_ir2_opnd2i(LISA_VREPLVEI_H, temp1, src, 0);
            IR2_OPND temp4 = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VSRAI_W, temp4, dest, 31);
            la_append_ir2_opnd3(LISA_VSRA_W, dest, dest, temp1);
            la_append_ir2_opnd4(LISA_VBITSEL_V, dest, temp4, dest, temp3);
        } else if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 1))) {
            uint8_t imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 1));
            if (imm > 31)
                imm = 31;
            la_append_ir2_opnd2i(LISA_VSRAI_W, dest, dest, imm);
        } else {
            lsassert(0);
        }
    } else { //mmx
        if (ir1_opnd_is_imm(ir1_get_opnd(pir1, 0) + 1)) {
            IR2_OPND dest = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0), false, true); /* fill default parameter */
            uint8 imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 0) + 1);
            if (imm > 31)
                imm = 31;
            la_append_ir2_opnd2i(LISA_VSRAI_W, dest, dest, imm);
        } else {
            IR2_OPND dest = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0), false, true); /* fill default parameter */
            IR2_OPND src = load_freg_from_ir1_1(
                ir1_get_opnd(pir1, 0) + 1, false, true); /* fill default parameter */
            IR2_OPND temp1 = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VREPLVEI_D, temp1, src, 0);
            IR2_OPND temp2 = ra_alloc_ftemp();

            la_append_ir2_opnd1i(LISA_VLDI, temp2, VLDI_IMM_TYPE0(2, 32));
            IR2_OPND temp3 = ra_alloc_ftemp();
            la_append_ir2_opnd3(LISA_VSLT_WU, temp3, temp1, temp2);
            la_append_ir2_opnd2i(LISA_VREPLVEI_W, temp1, src, 0);
            IR2_OPND temp4 = ra_alloc_ftemp();
            la_append_ir2_opnd2i(LISA_VSRAI_W, temp4, dest, 31);
            la_append_ir2_opnd3(LISA_VSRA_W, dest, dest, temp1);
            la_append_ir2_opnd4(LISA_VBITSEL_V, dest, temp4, dest, temp3);
        }
    }
    return true;
#endif
}

bool translate_pslldq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_pslldq(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    uint8_t imm8 = ir1_opnd_uimm(ir1_get_opnd(pir1, 1));
    if(imm8 > 15){
        la_append_ir2_opnd3(LISA_VXOR_V, dest, dest, dest);
    } else if (imm8 == 0) {
        return true;
    } else {
        la_append_ir2_opnd2i(LISA_VBSLL_V, dest, dest, imm8);
    }
    return true;
#endif
}

bool translate_psrldq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_psrldq(pir1);
#else
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    uint8_t imm8 = ir1_opnd_uimm(ir1_get_opnd(pir1, 1));
    if(imm8 > 15){
        la_append_ir2_opnd3(LISA_VXOR_V, dest, dest, dest);
    } else if (imm8 == 0) {
        return true;
    } else {
        la_append_ir2_opnd2i(LISA_VBSRL_V, dest, dest, imm8);
    }
    return true;
#endif
}


bool translate_addsubpd(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1)) ||
             ir1_opnd_is_mem(ir1_get_opnd(pir1, 1)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp_sub = ra_alloc_ftemp();

    la_append_ir2_opnd3(LISA_VFSUB_D, temp_sub, dest, src);
    la_append_ir2_opnd3(LISA_VFADD_D, dest, dest, src);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp_sub, 0);

    return true;
}

bool translate_addsubps(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 1)) ||
             ir1_opnd_is_mem(ir1_get_opnd(pir1, 1)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp_sub = ra_alloc_ftemp();

    la_append_ir2_opnd3(LISA_VFSUB_S, temp_sub, dest, src);
    la_append_ir2_opnd3(LISA_VFADD_S, dest, dest, src);

   /*
    * Pick temp_sub data to [0:31] [64:95]
    */
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp_sub, 0);
    la_append_ir2_opnd2i(LISA_VBSRL_V, temp_sub, temp_sub, 8);
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp_sub, 2);
    return true;
}
