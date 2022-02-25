#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>

void latxs_sys_simd_register_ir1(void)
{
    latxs_register_ir1(X86_INS_POR);
    latxs_register_ir1(X86_INS_PXOR);

    latxs_register_ir1(X86_INS_PACKUSWB);
    latxs_register_ir1(X86_INS_PACKSSWB);
    latxs_register_ir1(X86_INS_PACKSSDW);

    latxs_register_ir1(X86_INS_PADDB);
    latxs_register_ir1(X86_INS_PADDW);
    latxs_register_ir1(X86_INS_PADDD);
    latxs_register_ir1(X86_INS_PADDQ);

    latxs_register_ir1(X86_INS_PADDUSB);
    latxs_register_ir1(X86_INS_PADDUSW);
    latxs_register_ir1(X86_INS_PADDSW);
    latxs_register_ir1(X86_INS_PADDSB);

    latxs_register_ir1(X86_INS_PSUBUSB);
    latxs_register_ir1(X86_INS_PSUBUSW);
    latxs_register_ir1(X86_INS_PSUBSW);
    latxs_register_ir1(X86_INS_PSUBSB);

    latxs_register_ir1(X86_INS_PAND);
    latxs_register_ir1(X86_INS_PANDN);

    latxs_register_ir1(X86_INS_PCMPEQB);
    latxs_register_ir1(X86_INS_PCMPEQW);
    latxs_register_ir1(X86_INS_PCMPEQD);
    latxs_register_ir1(X86_INS_PCMPGTB);
    latxs_register_ir1(X86_INS_PCMPGTW);
    latxs_register_ir1(X86_INS_PCMPGTD);

    latxs_register_ir1(X86_INS_PMULLW);

    latxs_register_ir1(X86_INS_PSLLW);
    latxs_register_ir1(X86_INS_PSLLD);
    latxs_register_ir1(X86_INS_PSLLQ);
    latxs_register_ir1(X86_INS_PSLLDQ);

    latxs_register_ir1(X86_INS_PSRLW);
    latxs_register_ir1(X86_INS_PSRLD);
    latxs_register_ir1(X86_INS_PSRLQ);
    latxs_register_ir1(X86_INS_PSRLDQ);

    latxs_register_ir1(X86_INS_PSRAW);
    latxs_register_ir1(X86_INS_PSRAD);

    latxs_register_ir1(X86_INS_PSUBB);
    latxs_register_ir1(X86_INS_PSUBW);
    latxs_register_ir1(X86_INS_PSUBD);
    latxs_register_ir1(X86_INS_PSUBQ);

    latxs_register_ir1(X86_INS_PUNPCKHBW);
    latxs_register_ir1(X86_INS_PUNPCKHWD);
    latxs_register_ir1(X86_INS_PUNPCKHDQ);
    latxs_register_ir1(X86_INS_PUNPCKLBW);
    latxs_register_ir1(X86_INS_PUNPCKLWD);
    latxs_register_ir1(X86_INS_PUNPCKLDQ);

    latxs_register_ir1(X86_INS_PUNPCKHQDQ);
    latxs_register_ir1(X86_INS_PUNPCKLQDQ);

    latxs_register_ir1(X86_INS_ADDSD);
    latxs_register_ir1(X86_INS_ADDSS);

    latxs_register_ir1(X86_INS_ANDPS);
    latxs_register_ir1(X86_INS_ANDNPS);

    latxs_register_ir1(X86_INS_CMPEQPD);
    latxs_register_ir1(X86_INS_CMPLTPD);
    latxs_register_ir1(X86_INS_CMPLEPD);
    latxs_register_ir1(X86_INS_CMPUNORDPD);
    latxs_register_ir1(X86_INS_CMPNEQPD);
    latxs_register_ir1(X86_INS_CMPNLTPD);
    latxs_register_ir1(X86_INS_CMPNLEPD);
    latxs_register_ir1(X86_INS_CMPORDPD);
    latxs_register_ir1(X86_INS_CMPEQPS);
    latxs_register_ir1(X86_INS_CMPLTPS);
    latxs_register_ir1(X86_INS_CMPLEPS);
    latxs_register_ir1(X86_INS_CMPUNORDPS);
    latxs_register_ir1(X86_INS_CMPNEQPS);
    latxs_register_ir1(X86_INS_CMPNLTPS);
    latxs_register_ir1(X86_INS_CMPNLEPS);
    latxs_register_ir1(X86_INS_CMPORDPS);
    latxs_register_ir1(X86_INS_CMPEQSD);
    latxs_register_ir1(X86_INS_CMPLTSD);
    latxs_register_ir1(X86_INS_CMPLESD);
    latxs_register_ir1(X86_INS_CMPUNORDSD);
    latxs_register_ir1(X86_INS_CMPNEQSD);
    latxs_register_ir1(X86_INS_CMPNLTSD);
    latxs_register_ir1(X86_INS_CMPNLESD);
    latxs_register_ir1(X86_INS_CMPORDSD);
    latxs_register_ir1(X86_INS_CMPEQSS);
    latxs_register_ir1(X86_INS_CMPLTSS);
    latxs_register_ir1(X86_INS_CMPLESS);
    latxs_register_ir1(X86_INS_CMPUNORDSS);
    latxs_register_ir1(X86_INS_CMPNEQSS);
    latxs_register_ir1(X86_INS_CMPNLTSS);
    latxs_register_ir1(X86_INS_CMPNLESS);
    latxs_register_ir1(X86_INS_CMPORDSS);

    latxs_register_ir1(X86_INS_COMISD);
    latxs_register_ir1(X86_INS_COMISS);
    latxs_register_ir1(X86_INS_UCOMISD);
    latxs_register_ir1(X86_INS_UCOMISS);

    latxs_register_ir1(X86_INS_CVTDQ2PD);
    latxs_register_ir1(X86_INS_CVTDQ2PS);
    latxs_register_ir1(X86_INS_CVTPD2PS);
    latxs_register_ir1(X86_INS_CVTSD2SS);
    latxs_register_ir1(X86_INS_CVTSI2SD);
    latxs_register_ir1(X86_INS_CVTSS2SD);
    latxs_register_ir1(X86_INS_CVTTSS2SI);

    latxs_register_ir1(X86_INS_CVTPI2PS);
    latxs_register_ir1(X86_INS_CVTPS2PI);
    latxs_register_ir1(X86_INS_CVTSI2SS);
    latxs_register_ir1(X86_INS_CVTSS2SI);
    latxs_register_ir1(X86_INS_CVTTPS2PI);
    latxs_register_ir1(X86_INS_CVTPI2PD);
    latxs_register_ir1(X86_INS_CVTPD2PI);
    latxs_register_ir1(X86_INS_CVTSD2SI);
    latxs_register_ir1(X86_INS_CVTTPD2PI);
    latxs_register_ir1(X86_INS_CVTPD2DQ);
    latxs_register_ir1(X86_INS_CVTPS2DQ);
    latxs_register_ir1(X86_INS_CVTTPS2DQ);
    latxs_register_ir1(X86_INS_CVTTPD2DQ);

    latxs_register_ir1(X86_INS_DIVPD);
    latxs_register_ir1(X86_INS_DIVPS);
    latxs_register_ir1(X86_INS_DIVSD);
    latxs_register_ir1(X86_INS_DIVSS);

    latxs_register_ir1(X86_INS_MAXPD);
    latxs_register_ir1(X86_INS_MAXPS);
    latxs_register_ir1(X86_INS_MAXSD);
    latxs_register_ir1(X86_INS_MAXSS);
    latxs_register_ir1(X86_INS_MINPD);
    latxs_register_ir1(X86_INS_MINPS);
    latxs_register_ir1(X86_INS_MINSD);
    latxs_register_ir1(X86_INS_MINSS);

    latxs_register_ir1(X86_INS_MOVNTDQ);
    latxs_register_ir1(X86_INS_MOVNTPD);
    latxs_register_ir1(X86_INS_MOVNTPS);
    latxs_register_ir1(X86_INS_MOVNTQ);
    latxs_register_ir1(X86_INS_MOVNTI);

    latxs_register_ir1(X86_INS_MULPD);
    latxs_register_ir1(X86_INS_MULPS);
    latxs_register_ir1(X86_INS_MULSD);
    latxs_register_ir1(X86_INS_MULSS);

    latxs_register_ir1(X86_INS_ORPD);
    latxs_register_ir1(X86_INS_ORPS);

    latxs_register_ir1(X86_INS_PAVGB);
    latxs_register_ir1(X86_INS_PAVGW);

    latxs_register_ir1(X86_INS_PEXTRW);
    latxs_register_ir1(X86_INS_PINSRW);

    latxs_register_ir1(X86_INS_PMAXSW);
    latxs_register_ir1(X86_INS_PMAXUB);
    latxs_register_ir1(X86_INS_PMINSW);
    latxs_register_ir1(X86_INS_PMINUB);

    latxs_register_ir1(X86_INS_PSADBW);
    latxs_register_ir1(X86_INS_PMOVMSKB);
    latxs_register_ir1(X86_INS_MASKMOVQ);
    latxs_register_ir1(X86_INS_MASKMOVDQU);
    latxs_register_ir1(X86_INS_PMULHW);
    latxs_register_ir1(X86_INS_PMULHUW);
    latxs_register_ir1(X86_INS_PMULUDQ);

    latxs_register_ir1(X86_INS_PMADDWD);

    latxs_register_ir1(X86_INS_PSHUFD);
    latxs_register_ir1(X86_INS_PSHUFW);
    latxs_register_ir1(X86_INS_PSHUFLW);
    latxs_register_ir1(X86_INS_PSHUFHW);

    /* SSE */
    latxs_register_ir1(X86_INS_MOVUPD);
    latxs_register_ir1(X86_INS_MOVDQA);
    latxs_register_ir1(X86_INS_MOVDQU);
    latxs_register_ir1(X86_INS_MOVUPS);
    latxs_register_ir1(X86_INS_MOVAPD);
    latxs_register_ir1(X86_INS_MOVAPS);

    latxs_register_ir1(X86_INS_MOVLHPS);
    latxs_register_ir1(X86_INS_MOVHLPS);
    latxs_register_ir1(X86_INS_MOVHPD);
    latxs_register_ir1(X86_INS_MOVHPS);
    latxs_register_ir1(X86_INS_MOVLPD);
    latxs_register_ir1(X86_INS_MOVLPS);

    latxs_register_ir1(X86_INS_MOVQ2DQ);
    latxs_register_ir1(X86_INS_MOVDQ2Q);

    latxs_register_ir1(X86_INS_MOVMSKPS);
    latxs_register_ir1(X86_INS_MOVMSKPD);

    latxs_register_ir1(X86_INS_MOVSD);
    latxs_register_ir1(X86_INS_MOVSS);

    latxs_register_ir1(X86_INS_ADDPS);
    latxs_register_ir1(X86_INS_ADDPD);

    latxs_register_ir1(X86_INS_ANDPD);
    latxs_register_ir1(X86_INS_ANDNPD);

    latxs_register_ir1(X86_INS_UNPCKLPD);
    latxs_register_ir1(X86_INS_UNPCKHPD);
    latxs_register_ir1(X86_INS_UNPCKHPS);
    latxs_register_ir1(X86_INS_UNPCKLPS);

    latxs_register_ir1(X86_INS_SHUFPS);
    latxs_register_ir1(X86_INS_SHUFPD);

    latxs_register_ir1(X86_INS_XORPS);
    latxs_register_ir1(X86_INS_XORPD);

    latxs_register_ir1(X86_INS_SUBSS);
    latxs_register_ir1(X86_INS_SUBSD);
    latxs_register_ir1(X86_INS_SUBPS);
    latxs_register_ir1(X86_INS_SUBPD);

    latxs_register_ir1(X86_INS_RCPPS);
    latxs_register_ir1(X86_INS_RCPSS);

    latxs_register_ir1(X86_INS_SQRTPD);
    latxs_register_ir1(X86_INS_SQRTPS);
    latxs_register_ir1(X86_INS_SQRTSD);
    latxs_register_ir1(X86_INS_SQRTSS);

    latxs_register_ir1(X86_INS_RSQRTPS);
    latxs_register_ir1(X86_INS_RSQRTSS);

    latxs_register_ir1(X86_INS_CVTPS2PD);
    latxs_register_ir1(X86_INS_CVTTSD2SI);
}

#define XMM_REG(o) ir1_opnd_base_reg_num(o)
#define XMM_LOAD128(o) latxs_load_freg128_from_ir1(o)
#define XMM_LOADFP1(o) latxs_load_freg_from_ir1_1(o, true)

#define XMM_EXCP(pir1) do {                             \
    if (latxs_tr_gen_sse_common_excp_check(pir1)) {     \
        return true;                                    \
    }                                                   \
} while (0)

bool latxs_translate_por(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VOR_V, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VOR_V, &dest_lo, &dest_lo, &src_lo);

    return true;
}

bool latxs_translate_pxor(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0) && ir1_opnd_is_mem(opnd1)) {
        IR2_OPND temp = latxs_ra_alloc_ftemp();
        latxs_load_freg128_from_ir1_mem(&temp, opnd1);
        IR2_OPND xmm_dest = latxs_ra_alloc_xmm(XMM_REG(opnd0));
        latxs_append_ir2_opnd3(LISA_VXOR_V, &xmm_dest, &xmm_dest, &temp);
        return true;
    } else if (ir1_opnd_is_xmm(opnd0) && ir1_opnd_is_xmm(opnd1)) {
        IR2_OPND xmm_dest = latxs_ra_alloc_xmm(XMM_REG(opnd0));
        IR2_OPND xmm_src  = latxs_ra_alloc_xmm(XMM_REG(opnd1));
        latxs_append_ir2_opnd3(LISA_VXOR_V, &xmm_dest, &xmm_dest, &xmm_src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);

    latxs_append_ir2_opnd3(LISA_VXOR_V, &dest_lo, &dest_lo, &src_lo);

    return true;
}

bool latxs_translate_packuswb(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        if (ir1_opnd_is_xmm(opnd1) &&
                (XMM_REG(opnd0) == XMM_REG(opnd1))) {
            latxs_append_ir2_opnd2i(LISA_VSSRANI_BU_H, &dest, &dest, 0);
            latxs_append_ir2_opnd2i(LISA_VEXTRINS_D, &dest, &dest,
                    VEXTRINS_IMM_4_0(1, 0));
        } else {
            latxs_append_ir2_opnd2i(LISA_VSSRANI_BU_H, &dest, &dest, 0);
            IR2_OPND temp = latxs_ra_alloc_ftemp();
            latxs_append_ir2_opnd2i(LISA_VSSRANI_BU_H, &temp, &src, 0);
            latxs_append_ir2_opnd2i(LISA_VEXTRINS_D, &dest, &temp,
                    VEXTRINS_IMM_4_0(1, 0));
        }
    } else {
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);

        latxs_append_ir2_opnd3(LISA_VPACKEV_D, &dest, &src, &dest);
        latxs_append_ir2_opnd2i(LISA_VSSRANI_BU_H, &dest, &dest, 0);
    }

    return true;
}

bool latxs_translate_packsswb(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        if (ir1_opnd_is_xmm(opnd1) &&
                (XMM_REG(opnd0) == XMM_REG(opnd1))) {
            latxs_append_ir2_opnd2i(LISA_VSSRANI_B_H, &dest, &dest, 0);
        } else {
            latxs_append_ir2_opnd2i(LISA_VSSRANI_B_H, &dest, &dest, 0);
            IR2_OPND temp = latxs_ra_alloc_ftemp();
            latxs_append_ir2_opnd2i(LISA_VSSRANI_B_H, &temp, &src, 0);
            latxs_append_ir2_opnd2i(LISA_VEXTRINS_D, &dest, &temp,
                    VEXTRINS_IMM_4_0(1, 0));
        }
    } else {
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);

        latxs_append_ir2_opnd3(LISA_VPACKEV_D, &dest, &src, &dest);
        latxs_append_ir2_opnd2i(LISA_VSSRANI_B_H, &dest, &dest, 0);
    }

    return true;
}

bool latxs_translate_packssdw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        if (ir1_opnd_is_xmm(opnd1) &&
                (XMM_REG(opnd0) == XMM_REG(opnd1))) {
            latxs_append_ir2_opnd2i(LISA_VSSRANI_H_W, &dest, &dest, 0);
        } else {
            latxs_append_ir2_opnd2i(LISA_VSSRANI_H_W, &dest, &dest, 0);
            IR2_OPND temp = latxs_ra_alloc_ftemp();
            latxs_append_ir2_opnd2i(LISA_VSSRANI_H_W, &temp, &src, 0);
            latxs_append_ir2_opnd2i(LISA_VEXTRINS_D, &dest, &temp,
                    VEXTRINS_IMM_4_0(1, 0));
        }
    } else {
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);

        latxs_append_ir2_opnd3(LISA_VPACKEV_D, &dest, &src, &dest);
        latxs_append_ir2_opnd2i(LISA_VSSRANI_H_W, &dest, &dest, 0);
    }

    return true;
}

bool latxs_translate_paddb(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VADD_B, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VADD_B, &dest_lo, &dest_lo, &src_lo);

    return true;
}

bool latxs_translate_paddw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VADD_H, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VADD_H, &dest_lo, &dest_lo, &src_lo);
    return true;
}

bool latxs_translate_paddd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VADD_W, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VADD_W, &dest_lo, &dest_lo, &src_lo);
    return true;
}

bool latxs_translate_paddusb(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSADD_BU, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VSADD_BU, &dest_lo, &dest_lo, &src_lo);

    return true;
}

bool latxs_translate_paddusw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSADD_HU, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VSADD_HU, &dest_lo, &dest_lo, &src_lo);

    return true;
}

bool latxs_translate_paddsw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSADD_H, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VSADD_H, &dest_lo, &dest_lo, &src_lo);

    return true;
}

bool latxs_translate_paddsb(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSADD_B, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VSADD_B, &dest_lo, &dest_lo, &src_lo);

    return true;
}

bool latxs_translate_psubsb(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);

    latxs_append_ir2_opnd3(LISA_VSSUB_B, &dest, &dest, &src);

    return true;
}

bool latxs_translate_psubsw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);

    latxs_append_ir2_opnd3(LISA_VSSUB_H, &dest, &dest, &src);

    return true;
}

bool latxs_translate_psubusb(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);

    latxs_append_ir2_opnd3(LISA_VSSUB_BU, &dest, &dest, &src);

    return true;
}

bool latxs_translate_psubusw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);

    latxs_append_ir2_opnd3(LISA_VSSUB_HU, &dest, &dest, &src);

    return true;
}

bool latxs_translate_pand(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VAND_V, &dest_lo, &dest_lo, &src_lo);
    return true;
}

bool latxs_translate_pandn(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VANDN_V, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VANDN_V, &dest_lo, &dest_lo, &src_lo);
    return true;
}

bool latxs_translate_pcmpeqb(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSEQ_B, &dest, &dest, &src);
    } else {
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSEQ_B, &dest, &dest, &src);
    }

    return true;
}

bool latxs_translate_pcmpeqw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSEQ_H, &dest, &dest, &src);
    } else {
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSEQ_H, &dest, &dest, &src);
    }

    return true;
}

bool latxs_translate_pcmpeqd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSEQ_W, &dest, &dest, &src);
    } else {
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSEQ_W, &dest, &dest, &src);
    }

    return true;
}

bool latxs_translate_pcmpgtb(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSLT_B, &dest, &src, &dest);
    } else {
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSLT_B, &dest, &src, &dest);
    }

    return true;
}

bool latxs_translate_pcmpgtw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSLT_H, &dest, &src, &dest);
    } else {
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSLT_H, &dest, &src, &dest);
    }

    return true;
}

bool latxs_translate_pcmpgtd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSLT_W, &dest, &src, &dest);
    } else {
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSLT_W, &dest, &src, &dest);
    }

    return true;
}

bool latxs_translate_pmullw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VMUL_H, &dest, &dest, &src);
    } else {
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VMUL_H, &dest, &dest, &src);
    }

    return true;
}

bool latxs_translate_psllw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    if (ir1_opnd_is_mmx(opnd1) || ir1_opnd_is_xmm(opnd1)
                               || ir1_opnd_is_mem(opnd1)) {
        IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);
        IR2_OPND temp1 = latxs_ra_alloc_ftemp();
        IR2_OPND temp2 = latxs_ra_alloc_ftemp();

        latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
        latxs_append_ir2_opnd2i(LISA_VSLEI_DU, &temp2, &temp1, 15);
        latxs_append_ir2_opnd2i(LISA_VREPLVEI_H, &temp1, &src, 0);
        latxs_append_ir2_opnd3(LISA_VSLL_H, &dest, &dest, &temp1);
        latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &temp2);

        latxs_ra_free_temp(&temp1);
        latxs_ra_free_temp(&temp2);
    } else if (ir1_opnd_is_imm(opnd1)) {
        uint8_t imm = ir1_opnd_uimm(opnd1);
        if (imm > 15) {
            latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
        } else {
            latxs_append_ir2_opnd2i(LISA_VSLLI_H, &dest, &dest, imm);
        }
    } else {
        lsassert(0);
    }
    return true;
}

bool latxs_translate_pslld(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    if (ir1_opnd_is_mmx(opnd1) || ir1_opnd_is_xmm(opnd1)
                               || ir1_opnd_is_mem(opnd1)) {
        IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);
        IR2_OPND temp1 = latxs_ra_alloc_ftemp();
        IR2_OPND temp2 = latxs_ra_alloc_ftemp();

        latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
        latxs_append_ir2_opnd2i(LISA_VSLEI_DU, &temp2, &temp1, 31);
        latxs_append_ir2_opnd2i(LISA_VREPLVEI_W, &temp1, &src, 0);
        latxs_append_ir2_opnd3(LISA_VSLL_W, &dest, &dest, &temp1);
        latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &temp2);

        latxs_ra_free_temp(&temp1);
        latxs_ra_free_temp(&temp2);
    } else if (ir1_opnd_is_imm(opnd1)) {
        uint8_t imm = ir1_opnd_uimm(opnd1);
        if (imm > 31) {
            latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
        } else {
            latxs_append_ir2_opnd2i(LISA_VSLLI_W, &dest, &dest, imm);
        }
    } else {
        lsassert(0);
    }
    return true;
}

bool latxs_translate_psllq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    if (ir1_opnd_is_mmx(opnd1) || ir1_opnd_is_xmm(opnd1)
                               || ir1_opnd_is_mem(opnd1)) {
        IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);
        IR2_OPND temp1 = latxs_ra_alloc_ftemp();
        IR2_OPND temp2 = latxs_ra_alloc_ftemp();
        IR2_OPND temp3 = latxs_ra_alloc_ftemp();

        latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
        latxs_append_ir2_opnd1i(LISA_VLDI, &temp3, VLDI_IMM_TYPE0(3, 63));
        latxs_append_ir2_opnd3(LISA_VSLE_DU, &temp2, &temp1, &temp3);
        latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
        latxs_append_ir2_opnd3(LISA_VSLL_D, &dest, &dest, &temp1);
        latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &temp2);

        latxs_ra_free_temp(&temp1);
        latxs_ra_free_temp(&temp2);
        latxs_ra_free_temp(&temp3);
    } else if (ir1_opnd_is_imm(opnd1)) {
        uint8_t imm = ir1_opnd_uimm(opnd1);
        if (imm > 63) {
            latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
        } else {
            latxs_append_ir2_opnd2i(LISA_VSLLI_D, &dest, &dest, imm);
        }
    } else {
        lsassert(0);
    }
    return true;
}

bool latxs_translate_psrlw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    if (ir1_opnd_is_mmx(opnd1) || ir1_opnd_is_xmm(opnd1)
                               || ir1_opnd_is_mem(opnd1)) {
        IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);
        IR2_OPND temp1 = latxs_ra_alloc_ftemp();
        IR2_OPND temp2 = latxs_ra_alloc_ftemp();

        latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
        latxs_append_ir2_opnd2i(LISA_VSLEI_DU, &temp2, &temp1, 15);
        latxs_append_ir2_opnd2i(LISA_VREPLVEI_H, &temp1, &src, 0);
        latxs_append_ir2_opnd3(LISA_VSRL_H, &dest, &dest, &temp1);
        latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &temp2);

        latxs_ra_free_temp(&temp1);
        latxs_ra_free_temp(&temp2);
    } else if (ir1_opnd_is_imm(opnd1)) {
        uint8_t imm = ir1_opnd_uimm(opnd1);
        if (imm > 15) {
            latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
        } else {
            latxs_append_ir2_opnd2i(LISA_VSRLI_H, &dest, &dest, imm);
        }
    } else {
        lsassert(0);
    }
    return true;
}

bool latxs_translate_psrld(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    if (ir1_opnd_is_mmx(opnd1) || ir1_opnd_is_xmm(opnd1)
                               || ir1_opnd_is_mem(opnd1)) {
        IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);
        IR2_OPND temp1 = latxs_ra_alloc_ftemp();
        IR2_OPND temp2 = latxs_ra_alloc_ftemp();

        latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
        latxs_append_ir2_opnd2i(LISA_VSLEI_DU, &temp2, &temp1, 31);
        latxs_append_ir2_opnd2i(LISA_VREPLVEI_W, &temp1, &src, 0);
        latxs_append_ir2_opnd3(LISA_VSRL_W, &dest, &dest, &temp1);
        latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &temp2);

        latxs_ra_free_temp(&temp1);
        latxs_ra_free_temp(&temp2);
    } else if (ir1_opnd_is_imm(opnd1)) {
        uint8_t imm = ir1_opnd_uimm(opnd1);
        if (imm > 31) {
            latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
        } else {
            latxs_append_ir2_opnd2i(LISA_VSRLI_W, &dest, &dest, imm);
        }
    } else {
        lsassert(0);
    }
    return true;
}

bool latxs_translate_psrlq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    if (ir1_opnd_is_mmx(opnd1) || ir1_opnd_is_xmm(opnd1)
                               || ir1_opnd_is_mem(opnd1)) {
        IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);
        IR2_OPND temp1 = latxs_ra_alloc_ftemp();
        IR2_OPND temp2 = latxs_ra_alloc_ftemp();
        IR2_OPND temp3 = latxs_ra_alloc_ftemp();

        latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
        latxs_append_ir2_opnd1i(LISA_VLDI, &temp3, VLDI_IMM_TYPE0(3, 63));
        latxs_append_ir2_opnd3(LISA_VSLE_DU, &temp2, &temp1, &temp3);
        latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
        latxs_append_ir2_opnd3(LISA_VSRL_D, &dest, &dest, &temp1);
        latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &temp2);

        latxs_ra_free_temp(&temp1);
        latxs_ra_free_temp(&temp2);
        latxs_ra_free_temp(&temp3);
    } else if (ir1_opnd_is_imm(opnd1)) {
        uint8_t imm = ir1_opnd_uimm(opnd1);
        if (imm > 63) {
            latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
        } else {
            latxs_append_ir2_opnd2i(LISA_VSRLI_D, &dest, &dest, imm);
        }
    } else {
        lsassert(0);
    }
    return true;
}

bool latxs_translate_psraw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    if (ir1_opnd_is_mmx(opnd1) || ir1_opnd_is_xmm(opnd1)
                               || ir1_opnd_is_mem(opnd1)) {
        IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);
        IR2_OPND temp1 = latxs_ra_alloc_ftemp();
        IR2_OPND temp2 = latxs_ra_alloc_ftemp();
        IR2_OPND temp3 = latxs_ra_alloc_ftemp();
        IR2_OPND temp4 = latxs_ra_alloc_ftemp();
        latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
        latxs_append_ir2_opnd2i(LISA_VSLTI_DU, &temp2, &temp1, 15);
        latxs_append_ir2_opnd2i(LISA_VSRAI_H, &temp4, &dest, 15);
        latxs_append_ir2_opnd2i(LISA_VREPLVEI_H, &temp3, &src, 0);
        latxs_append_ir2_opnd3(LISA_VSRA_H, &dest, &dest, &temp3);
        latxs_append_ir2_opnd4(LISA_VBITSEL_V, &dest, &temp4, &dest, &temp2);
        latxs_ra_free_temp(&temp1);
        latxs_ra_free_temp(&temp2);
        latxs_ra_free_temp(&temp3);
        latxs_ra_free_temp(&temp4);
    } else if (ir1_opnd_is_imm(opnd1)) {
        uint8_t imm = ir1_opnd_uimm(opnd1);
        if (imm > 15) {
            imm = 15;
        }
        latxs_append_ir2_opnd2i(LISA_VSRAI_H, &dest, &dest, imm);
    } else {
        lsassert(0);
    }
    return true;
}

bool latxs_translate_psrad(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    if (ir1_opnd_is_mmx(opnd1) || ir1_opnd_is_xmm(opnd1)
                               || ir1_opnd_is_mem(opnd1)) {
        IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);
        IR2_OPND temp1 = latxs_ra_alloc_ftemp();
        IR2_OPND temp2 = latxs_ra_alloc_ftemp();
        IR2_OPND temp3 = latxs_ra_alloc_ftemp();
        IR2_OPND temp4 = latxs_ra_alloc_ftemp();
        latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
        latxs_append_ir2_opnd2i(LISA_VSLTI_DU, &temp2, &temp1, 31);
        latxs_append_ir2_opnd2i(LISA_VSRAI_W, &temp4, &dest, 31);
        latxs_append_ir2_opnd2i(LISA_VREPLVEI_W, &temp3, &src, 0);
        latxs_append_ir2_opnd3(LISA_VSRA_W, &dest, &dest, &temp3);
        latxs_append_ir2_opnd4(LISA_VBITSEL_V, &dest, &temp4, &dest, &temp2);
        latxs_ra_free_temp(&temp1);
        latxs_ra_free_temp(&temp2);
        latxs_ra_free_temp(&temp3);
        latxs_ra_free_temp(&temp4);
    } else if (ir1_opnd_is_imm(opnd1)) {
        uint8_t imm = ir1_opnd_uimm(opnd1);
        if (imm > 31) {
            imm = 31;
        }
        latxs_append_ir2_opnd2i(LISA_VSRAI_W, &dest, &dest, imm);
    } else {
        lsassert(0);
    }
    return true;
}


bool latxs_translate_psubb(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSUB_B, &dest, &dest, &src);
    } else { /* mmx */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSUB_B, &dest, &dest, &src);
    }
    return true;
}

bool latxs_translate_psubw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSUB_H, &dest, &dest, &src);
    } else { /* mmx */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSUB_H, &dest, &dest, &src);
    }
    return true;
}

bool latxs_translate_psubd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSUB_W, &dest, &dest, &src);
    } else { /* mmx */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSUB_W, &dest, &dest, &src);
    }
    return true;
}

bool latxs_translate_punpckhbw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PUNPCKHBW xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VILVH_B, &dest, &src, &dest);
    } else {
        /* PUNPCKHBW mm, mm/m64 */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VILVL_B, &dest, &src, &dest);
        latxs_append_ir2_opnd2i(LISA_VEXTRINS_D, &dest, &dest,
                                     VEXTRINS_IMM_4_0(0, 1));
    }

    return true;
}

bool latxs_translate_punpckhwd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PUNPCKHWD xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VILVH_H, &dest, &src, &dest);
    } else {
        /* PUNPCKHWD mm, mm/m64 */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VILVL_H, &dest, &src, &dest);
        latxs_append_ir2_opnd2i(LISA_VEXTRINS_D, &dest, &dest,
                                     VEXTRINS_IMM_4_0(0, 1));
    }

    return true;
}

bool latxs_translate_punpckhdq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PUNPCKHDQ xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VILVH_W, &dest, &src, &dest);
    } else {
        /* PUNPCKHDQ mm, mm/m64 */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VILVL_W, &dest, &src, &dest);
        latxs_append_ir2_opnd2i(LISA_VEXTRINS_D, &dest, &dest,
                                     VEXTRINS_IMM_4_0(0, 1));
    }

    return true;
}

bool latxs_translate_punpcklbw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PUNPCKLBW xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VILVL_B, &dest, &src, &dest);
    } else {
        /* PUNPCKLBW mm, mm/m32 */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = latxs_load_freg_from_ir1_1(opnd1, false);
        latxs_append_ir2_opnd3(LISA_VILVL_B, &dest, &src, &dest);
    }

    return true;
}

bool latxs_translate_punpcklwd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PUNPCKLWD xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VILVL_H, &dest, &src, &dest);
    } else {
        /* PUNPCKLWD mm, mm/m32 */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = latxs_load_freg_from_ir1_1(opnd1, false);
        latxs_append_ir2_opnd3(LISA_VILVL_H, &dest, &src, &dest);
    }

    return true;
}

bool latxs_translate_punpckldq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PUNPCKLQDQ xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VILVL_W, &dest, &src, &dest);
    } else {
        /*PUNPCKLDQ mm, mm/m32  */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = latxs_load_freg_from_ir1_1(opnd1, false);
        latxs_append_ir2_opnd3(LISA_VILVL_W, &dest, &src, &dest);
    }

    return true;
}

bool latxs_translate_addsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();

    latxs_append_ir2_opnd3(LISA_FADD_D, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_addss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();

    latxs_append_ir2_opnd3(LISA_FADD_S, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_andps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &src);
    return true;
}

bool latxs_translate_cmpeqpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &dest, &dest, &src,
                                  FCMP_COND_CEQ);
    return true;
}

bool latxs_translate_cmpltpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &dest, &dest, &src,
                                  FCMP_COND_CLT);
    return true;
}

bool latxs_translate_cmplepd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &dest, &dest, &src,
                                  FCMP_COND_CLE);
    return true;
}

bool latxs_translate_cmpunordpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &dest, &dest, &src,
                                  FCMP_COND_CUN);
    return true;
}

bool latxs_translate_cmpneqpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &dest, &dest, &src,
                                  FCMP_COND_CUNE);
    return true;
}

bool latxs_translate_cmpnltpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &dest, &src, &dest,
                                  FCMP_COND_CULE);
    return true;
}

bool latxs_translate_cmpnlepd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &dest, &src, &dest,
                                  FCMP_COND_CULT);
    return true;
}

bool latxs_translate_cmpordpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &dest, &dest, &src,
                                  FCMP_COND_COR);
    return true;
}

bool latxs_translate_cmpeqps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &dest, &dest, &src,
                                  FCMP_COND_CEQ);
    return true;
}

bool latxs_translate_cmpltps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &dest, &dest, &src,
                                  FCMP_COND_CLT);
    return true;
}

bool latxs_translate_cmpleps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &dest, &dest, &src,
                                  FCMP_COND_CLE);
    return true;
}

bool latxs_translate_cmpunordps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &dest, &dest, &src,
                                  FCMP_COND_CUN);
    return true;
}

bool latxs_translate_cmpneqps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &dest, &dest, &src,
                                  FCMP_COND_CUNE);
    return true;
}

bool latxs_translate_cmpnltps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &dest, &src, &dest,
                                  FCMP_COND_CULE);
    return true;
}

bool latxs_translate_cmpnleps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &dest, &src, &dest,
                                  FCMP_COND_CULT);
    return true;
}

bool latxs_translate_cmpordps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &dest, &dest, &src,
                                  FCMP_COND_COR);
    return true;
}

bool latxs_translate_cmpeqsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &temp, &dest, &src,
                                  FCMP_COND_CEQ);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpltsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &temp, &dest, &src,
                                  FCMP_COND_CLT);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmplesd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &temp, &dest, &src,
                                  FCMP_COND_CLE);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpunordsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &temp, &dest, &src,
                                  FCMP_COND_CUN);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpneqsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &temp, &dest, &src,
                                  FCMP_COND_CUNE);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpnltsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &temp, &src, &dest,
                                  FCMP_COND_CULE);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpnlesd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &temp, &src, &dest,
                                  FCMP_COND_CULT);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpordsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &temp, &dest, &src,
                                  FCMP_COND_COR);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpeqss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &temp, &dest, &src,
                                  FCMP_COND_CEQ);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpltss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &temp, &dest, &src,
                                  FCMP_COND_CLT);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpless(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &temp, &dest, &src,
                                  FCMP_COND_CLE);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpunordss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &temp, &dest, &src,
                                  FCMP_COND_CUN);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpneqss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &temp, &dest, &src,
                                  FCMP_COND_CUNE);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpnltss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &temp, &src, &dest,
                                  FCMP_COND_CULE);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpnless(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &temp, &src, &dest,
                                  FCMP_COND_CULT);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpordss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &temp, &dest, &src,
                                  FCMP_COND_COR);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_comisd(IR1_INST *pir1)
{
    return latxs_translate_ucomisd(pir1);
}

bool latxs_translate_comiss(IR1_INST *pir1)
{
    return latxs_translate_ucomiss(pir1);
}

bool latxs_translate_cvtpi2ps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND fcsr_opnd = latxs_set_fpu_fcsr_rounding_field_by_x86();
    IR2_OPND dest = latxs_load_freg128_from_ir1(opnd0);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2(LISA_VFFINT_S_W, &temp, &src);
    latxs_append_ir2_opnd2i(LISA_VEXTRINS_D, &dest, &temp,
                        VEXTRINS_IMM_4_0(0, 0));
    latxs_set_fpu_fcsr(&fcsr_opnd);
    latxs_ra_free_temp(&temp);
    return true;
}

bool latxs_translate_cvtps2pi(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);
    latxs_append_ir2_opnd2i(LISA_VST, &src, &latxs_env_ir2_opnd,
                            offsetof(CPUX86State, temp_xmm));

    helper_cfg_t cfg = default_helper_cfg;
    int reg_index = ir1_opnd_base_reg_num(opnd0);
    uint64_t dest_addr =
        (uint64_t)&(((CPUX86State *)lsenv->cpu_state)->fpregs[reg_index]);
    uint64_t src_addr =
        (uint64_t)&(((CPUX86State *)lsenv->cpu_state)->temp_xmm);
    latxs_tr_gen_call_to_helper3_u64_cfg((ADDR)helper_cvtps2pi,
                                    dest_addr, src_addr, cfg);
    return true;
}

bool latxs_translate_cvtsi2ss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND fcsr_opnd = latxs_set_fpu_fcsr_rounding_field_by_x86();
    IR2_OPND dest = latxs_load_freg128_from_ir1(opnd0);
    IR2_OPND src = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src, opnd1, EXMode_Z);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &temp, &src);
    if (ir1_opnd_size(opnd1) == 64) {
        latxs_append_ir2_opnd2(LISA_FFINT_S_L, &temp, &temp);
    } else {
        latxs_append_ir2_opnd2(LISA_FFINT_S_W, &temp, &temp);
    }
    latxs_append_ir2_opnd2i(LISA_VEXTRINS_W, &dest, &temp,
                        VEXTRINS_IMM_4_0(0, 0));
    latxs_set_fpu_fcsr(&fcsr_opnd);
    latxs_ra_free_temp(&src);
    latxs_ra_free_temp(&temp);
    return true;
}

bool latxs_translate_cvtss2si(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);
    latxs_append_ir2_opnd2i(LISA_VST, &src, &latxs_env_ir2_opnd,
                            offsetof(CPUX86State, temp_xmm));

    helper_cfg_t cfg = default_helper_cfg;
    uint64_t src_addr =
        (uint64_t)&(((CPUX86State *)lsenv->cpu_state)->temp_xmm);

#ifdef TARGET_X86_64
    if (ir1_opnd_size(opnd0) == 64) {
        latxs_tr_gen_call_to_helper3_u64_cfg((ADDR)helper_cvtss2sq, src_addr, 0,
                                             cfg);
    } else {
        latxs_tr_gen_call_to_helper3_u64_cfg((ADDR)helper_cvtss2si, src_addr, 0,
                                             cfg);
    }
#else
    latxs_tr_gen_call_to_helper3_u64_cfg((ADDR)helper_cvtss2si, src_addr, 0,
                                         cfg);
#endif

    latxs_store_ir2_to_ir1(&latxs_ret0_ir2_opnd, opnd0);
    return true;
}

bool latxs_translate_cvttps2pi(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);
    latxs_append_ir2_opnd2i(LISA_VST, &src, &latxs_env_ir2_opnd,
                            offsetof(CPUX86State, temp_xmm));

    helper_cfg_t cfg = default_helper_cfg;
    int reg_index = ir1_opnd_base_reg_num(opnd0);
    uint64_t dest_addr =
        (uint64_t)&(((CPUX86State *)lsenv->cpu_state)->fpregs[reg_index]);
    uint64_t src_addr =
        (uint64_t)&(((CPUX86State *)lsenv->cpu_state)->temp_xmm);
    latxs_tr_gen_call_to_helper3_u64_cfg((ADDR)helper_cvttps2pi,
                                    dest_addr, src_addr, cfg);
    return true;
}

bool latxs_translate_cvtpi2pd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND fcsr_opnd = latxs_set_fpu_fcsr_rounding_field_by_x86();
    /* TODO:simply */
    IR2_OPND dest = XMM_LOAD128(opnd0);

    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);

    IR2_OPND temp_src = latxs_ra_alloc_ftemp();

    latxs_append_ir2_opnd2i(LISA_VEXTRINS_W, &temp_src, &src,
                                 VEXTRINS_IMM_4_0(0, 0));
    latxs_append_ir2_opnd2(LISA_FFINT_D_W, &temp_src, &temp_src);
    latxs_append_ir2_opnd2i(LISA_VEXTRINS_D, &dest, &temp_src,
                                 VEXTRINS_IMM_4_0(0, 0));

    latxs_append_ir2_opnd2i(LISA_VEXTRINS_W, &temp_src, &src,
                                 VEXTRINS_IMM_4_0(0, 1));
    latxs_append_ir2_opnd2(LISA_FFINT_D_W, &temp_src, &temp_src);
    latxs_append_ir2_opnd2i(LISA_VEXTRINS_D, &dest, &temp_src,
                                 VEXTRINS_IMM_4_0(1, 0));
    latxs_set_fpu_fcsr(&fcsr_opnd);
    return true;
}

bool latxs_translate_cvtpd2pi(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);
    latxs_append_ir2_opnd2i(LISA_VST, &src, &latxs_env_ir2_opnd,
                            offsetof(CPUX86State, temp_xmm));

    helper_cfg_t cfg = default_helper_cfg;
    int reg_index = ir1_opnd_base_reg_num(opnd0);
    uint64_t dest_addr =
        (uint64_t)&(((CPUX86State *)lsenv->cpu_state)->fpregs[reg_index]);
    uint64_t src_addr =
        (uint64_t)&(((CPUX86State *)lsenv->cpu_state)->temp_xmm);
    latxs_tr_gen_call_to_helper3_u64_cfg((ADDR)helper_cvtpd2pi,
                                dest_addr, src_addr, cfg);
    return true;
}

bool latxs_translate_cvtsd2si(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);
    latxs_append_ir2_opnd2i(LISA_VST, &src, &latxs_env_ir2_opnd,
                            offsetof(CPUX86State, temp_xmm));

    helper_cfg_t cfg = default_helper_cfg;
    uint64_t src_addr =
        (uint64_t)&(((CPUX86State *)lsenv->cpu_state)->temp_xmm);
    latxs_tr_gen_call_to_helper3_u64_cfg((ADDR)helper_cvtsd2si, src_addr,
                                        0, cfg);
    latxs_store_ir2_to_ir1(&latxs_ret0_ir2_opnd, opnd0);
    return true;
}

bool latxs_translate_cvttpd2pi(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);
    latxs_append_ir2_opnd2i(LISA_VST, &src, &latxs_env_ir2_opnd,
                            offsetof(CPUX86State, temp_xmm));

    helper_cfg_t cfg = default_helper_cfg;
    int reg_index = ir1_opnd_base_reg_num(opnd0);
    uint64_t dest_addr =
        (uint64_t)&(((CPUX86State *)lsenv->cpu_state)->fpregs[reg_index]);
    uint64_t src_addr =
        (uint64_t)&(((CPUX86State *)lsenv->cpu_state)->temp_xmm);
    latxs_tr_gen_call_to_helper3_u64_cfg((ADDR)helper_cvttpd2pi,
                                dest_addr, src_addr, cfg);
    return true;
}

bool latxs_translate_cvtpd2dq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);
    latxs_append_ir2_opnd2i(LISA_VST, &src, &latxs_env_ir2_opnd,
                            offsetof(CPUX86State, temp_xmm));

    helper_cfg_t cfg = default_helper_cfg;
    int reg_index = ir1_opnd_base_reg_num(opnd0);
    uint64_t dest_addr =
        (uint64_t)&(((CPUX86State *)lsenv->cpu_state)->xmm_regs[reg_index]);
    uint64_t src_addr =
        (uint64_t)&(((CPUX86State *)lsenv->cpu_state)->temp_xmm);
    latxs_tr_gen_call_to_helper3_u64_cfg((ADDR)helper_cvtpd2dq,
                                dest_addr, src_addr, cfg);
    return true;
}

bool latxs_translate_cvtps2dq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);
    latxs_append_ir2_opnd2i(LISA_VST, &src, &latxs_env_ir2_opnd,
                            offsetof(CPUX86State, temp_xmm));

    helper_cfg_t cfg = default_helper_cfg;
    int reg_index = ir1_opnd_base_reg_num(opnd0);
    uint64_t dest_addr =
        (uint64_t)&(((CPUX86State *)lsenv->cpu_state)->xmm_regs[reg_index]);
    uint64_t src_addr =
        (uint64_t)&(((CPUX86State *)lsenv->cpu_state)->temp_xmm);
    latxs_tr_gen_call_to_helper3_u64_cfg((ADDR)helper_cvtps2dq,
                                dest_addr, src_addr, cfg);
    return true;
}

bool latxs_translate_cvttps2dq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);
    latxs_append_ir2_opnd2i(LISA_VST, &src, &latxs_env_ir2_opnd,
                            offsetof(CPUX86State, temp_xmm));

    helper_cfg_t cfg = default_helper_cfg;
    int reg_index = ir1_opnd_base_reg_num(opnd0);
    uint64_t dest_addr =
        (uint64_t)&(((CPUX86State *)lsenv->cpu_state)->xmm_regs[reg_index]);
    uint64_t src_addr =
        (uint64_t)&(((CPUX86State *)lsenv->cpu_state)->temp_xmm);
    latxs_tr_gen_call_to_helper3_u64_cfg((ADDR)helper_cvttps2dq,
                                dest_addr, src_addr, cfg);
    return true;
}

bool latxs_translate_cvttpd2dq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);
    latxs_append_ir2_opnd2i(LISA_VST, &src, &latxs_env_ir2_opnd,
                            offsetof(CPUX86State, temp_xmm));

    helper_cfg_t cfg = default_helper_cfg;
    int reg_index = ir1_opnd_base_reg_num(opnd0);
    uint64_t dest_addr =
        (uint64_t) &(((CPUX86State *)lsenv->cpu_state)->xmm_regs[reg_index]);
    uint64_t src_addr =
        (uint64_t) &(((CPUX86State *)lsenv->cpu_state)->temp_xmm);
    latxs_tr_gen_call_to_helper3_u64_cfg((ADDR)helper_cvttpd2dq,
                                dest_addr, src_addr, cfg);
    return true;
}


bool latxs_translate_cvtdq2pd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND fcsr_opnd = latxs_set_fpu_fcsr_rounding_field_by_x86();
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd2(LISA_VFFINTL_D_W, &dest, &src);
    latxs_set_fpu_fcsr(&fcsr_opnd);
    return true;
}

bool latxs_translate_cvtdq2ps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd2(LISA_VFFINT_S_W, &dest, &src);
    return true;
}

bool latxs_translate_cvtpd2ps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VFCVT_S_D, &dest, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVPICKVE_D, &dest, &dest, 0);
    return true;
}

bool latxs_translate_cvtsd2ss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND fcsr_opnd = latxs_set_fpu_fcsr_rounding_field_by_x86();
    /* TODO:simply */
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);

    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2(LISA_FCVT_S_D, &temp, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);

    latxs_set_fpu_fcsr(&fcsr_opnd);
    return true;
}

bool latxs_translate_cvtsi2sd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND fcsr_opnd = latxs_set_fpu_fcsr_rounding_field_by_x86();
    /* TODO:simply */
    IR2_OPND dest = XMM_LOAD128(opnd0);

    IR2_OPND src = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src, opnd1, EXMode_S);

    IR2_OPND temp_src = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &temp_src, &src);

    if (ir1_opnd_size(opnd0 + 1) == 64) {
        latxs_append_ir2_opnd2(LISA_FFINT_D_L, &temp_src, &temp_src);
    } else {
        latxs_append_ir2_opnd2(LISA_FFINT_D_W, &temp_src, &temp_src);
    }

    latxs_append_ir2_opnd2i(LISA_VEXTRINS_D, &dest, &temp_src,
                                 VEXTRINS_IMM_4_0(0, 0));
    latxs_set_fpu_fcsr(&fcsr_opnd);
    return true;
}

bool latxs_translate_cvtss2sd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));

    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();

    latxs_append_ir2_opnd2(LISA_FCVT_D_S, &temp, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);

    return true;
}

bool latxs_translate_cvttss2si(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *fcc0 = &latxs_fcc0_ir2_opnd;
    IR2_OPND *fcsr = &latxs_fcsr_ir2_opnd;

    IR2_OPND src = XMM_LOAD128(opnd1);

    IR2_OPND src_lo = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2(LISA_FCVT_D_S, &src_lo, &src);

    IR2_OPND temp_over_flow  = latxs_ra_alloc_itemp();
    IR2_OPND ftemp_over_flow = latxs_ra_alloc_ftemp();
    /*
     * if 32 bits, check data is 0x41e0_0000_0000_0000
     * if 64 bits, check data is 0x43e0_0000_0000_0000
     */
    latxs_append_ir2_opnd2i(LISA_LU52I_D, &temp_over_flow, zero,
                            0x3fe + ir1_opnd_size(opnd0));
    latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &ftemp_over_flow,
                                            &temp_over_flow);

    IR2_OPND temp_under_flow  = latxs_ra_alloc_itemp();
    IR2_OPND ftemp_under_flow = latxs_ra_alloc_ftemp();
    /*
     * if 32 bits, check data is 0xc1e0_0000_0000_0000
     * if 64 bits, check data is 0xc3e0_0000_0000_0000
     */
    latxs_append_ir2_opnd2i(LISA_LU52I_D, &temp_under_flow, zero,
                            (0xbfe + ir1_opnd_size(opnd0)) << 20 >> 20);
    latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &ftemp_under_flow,
                                            &temp_under_flow);

    /*is unoder?*/
    IR2_OPND label_for_flow = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0, &src_lo, &src_lo,
                                 FCMP_COND_CUN);
    latxs_append_ir2_opnd2(LISA_BCNEZ, fcc0, &label_for_flow);

    /*is over flow or under flow*/
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0, &ftemp_over_flow,
                        &src_lo, FCMP_COND_CLE);
    latxs_append_ir2_opnd2(LISA_BCNEZ, fcc0, &label_for_flow);

    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0, &src_lo,
              &ftemp_under_flow, FCMP_COND_CLE);
    latxs_append_ir2_opnd2(LISA_BCNEZ, fcc0, &label_for_flow);

    /*not over flow and under flow*/
    IR2_OPND temp_fcsr = latxs_ra_alloc_itemp();

    /*save fscr in temp_int for reload*/
    latxs_append_ir2_opnd2(LISA_MOVFCSR2GR, &temp_fcsr, fcsr);
    latxs_append_ir2_opnd3(LISA_OR, &temp_under_flow,
                                    &temp_under_flow, &temp_fcsr);

    /*set fscr for rounding to zero according to x86 operation*/
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &temp_fcsr, zero, 9, 8);
    latxs_append_ir2_opnd2i(LISA_ORI, &temp_fcsr, &temp_fcsr, 0x1 << 8);
    latxs_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr, &temp_fcsr);

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
        latxs_append_ir2_opnd2(LISA_FTINT_L_D, &ftemp_over_flow, &src_lo);
    } else {
        latxs_append_ir2_opnd2(LISA_FTINT_W_D, &ftemp_over_flow, &src_lo);
    }

    latxs_append_ir2_opnd2(LISA_MOVFR2GR_D, &temp_fcsr,
                                            &ftemp_over_flow);
    /* reload fcsr */
    latxs_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr,
                                            &temp_under_flow);

    IR2_OPND label_for_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd1(LISA_B, &label_for_exit);
    latxs_append_ir2_opnd1(LISA_LABEL, &label_for_flow);
#ifdef TARGET_X86_64
    if (ir1_opnd_size(opnd0) == 64) {
        latxs_load_imm64_to_ir2(&temp_fcsr, 0x8000000000000000UL);
    } else {
        latxs_load_imm32_to_ir2(&temp_fcsr, 0x80000000, EXMode_Z);
    }
#else
    latxs_load_imm32_to_ir2(&temp_fcsr, 0x80000000, EXMode_Z);
#endif
    latxs_append_ir2_opnd1(LISA_LABEL, &label_for_exit);
    latxs_store_ir2_to_ir1(&temp_fcsr, opnd0);

    latxs_ra_free_temp(&temp_fcsr);
    latxs_ra_free_temp(&temp_over_flow);
    latxs_ra_free_temp(&temp_under_flow);
    latxs_ra_free_temp(&ftemp_over_flow);
    latxs_ra_free_temp(&ftemp_under_flow);

    return true;
}

bool latxs_translate_divpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd3(LISA_VFDIV_D, &dest, &dest, &src);
    return true;
}

bool latxs_translate_divps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd3(LISA_VFDIV_S, &dest, &dest, &src);
    return true;
}

bool latxs_translate_divsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_FDIV_D, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_divss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_FDIV_S, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_maxpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VFMAX_D, &dest, &dest, &src);
    return true;
}

bool latxs_translate_maxps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VFMAX_S, &dest, &dest, &src);
    return true;
}

bool latxs_translate_maxsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_VFMAX_D, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_maxss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_VFMAX_S, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_minpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VFMIN_D, &dest, &dest, &src);
    return true;
}

bool latxs_translate_minps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VFMIN_S, &dest, &dest, &src);
    return true;
}

bool latxs_translate_minsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_VFMIN_D, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_minss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_VFMIN_S, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_movmskpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *src = ir1_get_opnd(pir1, 1);
    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    lsassert(ir1_opnd_is_xmm(src));
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    IR2_OPND src_ir2 = latxs_ra_alloc_xmm(XMM_REG(src));
    IR2_OPND dest_ir2 = latxs_ra_alloc_gpr(
        ir1_opnd_base_reg_num(dest));
    latxs_append_ir2_opnd2(LISA_VMSKLTZ_D, &temp, &src_ir2);
    latxs_append_ir2_opnd2(LISA_MOVFR2GR_D, &dest_ir2, &temp);
    return true;
}

bool latxs_translate_movmskps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *src = ir1_get_opnd(pir1, 1);
    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    lsassert(ir1_opnd_is_xmm(src));
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    IR2_OPND src_ir2 = latxs_ra_alloc_xmm(XMM_REG(src));
    IR2_OPND dest_ir2 = latxs_ra_alloc_gpr(
        ir1_opnd_base_reg_num(dest));
    latxs_append_ir2_opnd2(LISA_VMSKLTZ_W, &temp, &src_ir2);
    latxs_append_ir2_opnd2(LISA_MOVFR2GR_D, &dest_ir2, &temp);
    return true;
}

bool latxs_translate_movnti(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src, opnd1, EXMode_N);
    latxs_store_ir2_to_ir1(&src, opnd0);

    return true;
}

bool latxs_translate_movntdq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd1));

    IR2_OPND src_ir2 = latxs_ra_alloc_xmm(XMM_REG(opnd1));
    latxs_store_freg128_to_ir1_mem(&src_ir2, opnd0);
    return true;
}

bool latxs_translate_movntpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd1));

    IR2_OPND src_ir2 = latxs_ra_alloc_xmm(XMM_REG(opnd1));
    latxs_store_freg128_to_ir1_mem(&src_ir2, opnd0);
    return true;
}

bool latxs_translate_movntps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd1));

    IR2_OPND src_ir2 = latxs_ra_alloc_xmm(XMM_REG(opnd1));
    latxs_store_freg128_to_ir1_mem(&src_ir2, opnd0);
    return true;
}

bool latxs_translate_movntq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_mmx(opnd1));

    IR2_OPND src_ir2 = XMM_LOADFP1(opnd1);
    latxs_store_freg_to_ir1(&src_ir2, opnd0, false);

    return true;
}

bool latxs_translate_mulpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VFMUL_D, &dest, &dest, &src);
    return true;
}

bool latxs_translate_mulps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VFMUL_S, &dest, &dest, &src);
    return true;
}

bool latxs_translate_mulsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_FMUL_D, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_mulss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_FMUL_S, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_orpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VOR_V, &dest, &dest, &src);
    return true;
}

bool latxs_translate_orps(IR1_INST *pir1)
{
    latxs_translate_orpd(pir1);
    return true;
}

bool latxs_translate_paddq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VADD_D, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);

    latxs_append_ir2_opnd3(LISA_VADD_D, &dest_lo, &dest_lo, &src_lo);

    return true;
}

bool latxs_translate_pavgb(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);

    latxs_append_ir2_opnd3(LISA_VAVGR_BU, &dest, &dest, &src);

    return true;
}

bool latxs_translate_pavgw(IR1_INST *pir1)
{

    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);

    latxs_append_ir2_opnd3(LISA_VAVGR_HU, &dest, &dest, &src);

    return true;
}

bool latxs_translate_pextrw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR1_OPND *opnd2 = ir1_get_opnd(pir1, 2);
    uint8_t imm = ir1_opnd_uimm(opnd2);
    IR2_OPND gpr = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
    IR2_OPND src_ir2;
    if (ir1_opnd_is_xmm(opnd1)) {
        imm &= 7;
        src_ir2 = latxs_ra_alloc_xmm(XMM_REG(opnd1));
    } else if (ir1_opnd_is_mmx(opnd1)) {
        imm &= 3;
        src_ir2 = latxs_ra_alloc_mmx(XMM_REG(opnd1));
    } else {
        lsassert(0);
    }

    latxs_append_ir2_opnd2i(LISA_VPICKVE2GR_HU, &gpr, &src_ir2, imm);

    return true;
}

bool latxs_translate_pinsrw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&temp, opnd1, EXMode_N);

    IR1_OPND *opnd2 = ir1_get_opnd(pir1, 2);
    uint8_t imm = ir1_opnd_uimm(opnd2);
    IR2_OPND dest_ir2;
    if (ir1_opnd_is_xmm(opnd0)) {
        imm &= 7;
        dest_ir2 = latxs_ra_alloc_xmm(XMM_REG(opnd0));
    } else if (ir1_opnd_is_mmx(opnd0)) {
        imm &= 3;
        dest_ir2 = latxs_ra_alloc_mmx(XMM_REG(opnd0));
    } else {
        lsassert(0);
    }

    latxs_append_ir2_opnd2i(LISA_VINSGR2VR_H, &dest_ir2, &temp, imm);

    return true;
}

bool latxs_translate_pmaxsw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PMAXSW xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VMAX_H, &dest, &dest, &src);
        return true;
    }

    /* PMAXSW mm1, mm2/m64 */
    IR2_OPND dest = XMM_LOADFP1(opnd0);
    IR2_OPND src = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VMAX_H, &dest, &dest, &src);

    return true;
}

bool latxs_translate_pmaxub(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PMAXUB xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VMAX_BU, &dest, &dest, &src);
        return true;
    }

    /* PMAXUB mm1, mm2/m64 */
    IR2_OPND dest = XMM_LOADFP1(opnd0);
    IR2_OPND src  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VMAX_BU, &dest, &dest, &src);

    return true;
}

bool latxs_translate_pminsw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PMINSW xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VMIN_H, &dest, &dest, &src);
        return true;
    }

    /* PMINSW mm1, mm2/m64 */
    IR2_OPND dest = XMM_LOADFP1(opnd0);
    IR2_OPND src  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VMIN_H, &dest, &dest, &src);

    return true;
}

bool latxs_translate_pminub(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PMINUB xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VMIN_BU, &dest, &dest, &src);
        return true;
    }

    /* PMINUB mm1, mm2/m64 */
    IR2_OPND dest = XMM_LOADFP1(opnd0);
    IR2_OPND src  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VMIN_BU, &dest, &dest, &src);

    return true;
}

bool latxs_translate_psadbw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);

    latxs_append_ir2_opnd3(LISA_VABSD_BU, &dest, &dest, &src);

    if (ir1_opnd_size(opnd0) == 64) {
        latxs_append_ir2_opnd2i(LISA_XVPICKVE_D, &dest, &dest, 0);
    }

    latxs_append_ir2_opnd2(LISA_VHADD8_D_BU, &dest, &dest);

    return true;
}

bool latxs_translate_maskmovdqu(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    /* data */
    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    /* mask */
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);

    IR2_OPND mask = latxs_ra_alloc_ftemp();

    IR2_OPND one_msb = latxs_ra_alloc_itemp();
    IR2_OPND one_byte = latxs_ra_alloc_itemp();

    int addr_size = latxs_ir1_addr_size(pir1);

#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        lsassert(addr_size == 4 || addr_size == 8);
    } else {
        lsassert(addr_size == 2 || addr_size == 4);
    }
#else
    lsassert(addr_size == 2 || addr_size == 4);
#endif

    IR1_OPND mem_ir1_opnd;

    la_append_ir2_opnd2i(LISA_VANDI_B, mask, src, 0x80);

    int byte_num = 16;

    for (size_t i = 0; i < byte_num; i++) {
        latxs_append_ir2_opnd2i(LISA_VPICKVE2GR_BU, &one_msb, &mask, i);
        latxs_append_ir2_opnd2i(LISA_VPICKVE2GR_BU, &one_byte, &dest, i);
        IR2_OPND skip = latxs_ir2_opnd_new_label();
        latxs_append_ir2_opnd3(LISA_BEQ, &one_msb, &latxs_zero_ir2_opnd, &skip);

        latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, 8, X86_REG_DS, X86_REG_EDI,
                                      i, 0, 0);
        latxs_store_ir2_to_ir1_mem(&one_byte, &mem_ir1_opnd, addr_size);

        latxs_append_ir2_opnd1(LISA_LABEL, &skip);
    }

    latxs_ra_free_temp(&mask);
    latxs_ra_free_temp(&one_msb);
    latxs_ra_free_temp(&one_byte);

    return true;
}

bool latxs_translate_maskmovq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    /* data */
    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    /* mask */
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);

    IR2_OPND mask = latxs_ra_alloc_ftemp();

    IR2_OPND one_msb = latxs_ra_alloc_itemp();
    IR2_OPND one_byte = latxs_ra_alloc_itemp();

    int addr_size = latxs_ir1_addr_size(pir1);

#ifdef TARGET_X86_64
    if (lsenv->tr_data->sys.code64) {
        lsassert(addr_size == 4 || addr_size == 8);
    } else {
        lsassert(addr_size == 2 || addr_size == 4);
    }
#else
    lsassert(addr_size == 2 || addr_size == 4);
#endif

    IR1_OPND mem_ir1_opnd;

    la_append_ir2_opnd2i(LISA_VANDI_B, mask, src, 0x80);

    int byte_num = 8;

    for (size_t i = 0; i < byte_num; i++) {
        latxs_append_ir2_opnd2i(LISA_VPICKVE2GR_BU, &one_msb, &mask, i);
        latxs_append_ir2_opnd2i(LISA_VPICKVE2GR_BU, &one_byte, &dest, i);
        IR2_OPND skip = latxs_ir2_opnd_new_label();
        latxs_append_ir2_opnd3(LISA_BEQ, &one_msb, &latxs_zero_ir2_opnd, &skip);

        latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, 8, X86_REG_DS, X86_REG_EDI,
                                      i, 0, 0);
        latxs_store_ir2_to_ir1_mem(&one_byte, &mem_ir1_opnd, addr_size);

        latxs_append_ir2_opnd1(LISA_LABEL, &skip);
    }

    latxs_ra_free_temp(&mask);
    latxs_ra_free_temp(&one_msb);
    latxs_ra_free_temp(&one_byte);

    return true;
}

bool latxs_translate_pmovmskb(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR1_OPND *dest = opnd0;
    IR1_OPND *src  = opnd1;

    if (ir1_opnd_is_xmm(src)) {
        IR2_OPND temp = latxs_ra_alloc_ftemp();
        IR2_OPND xmm_src  = latxs_ra_alloc_xmm(XMM_REG(src));
        IR2_OPND gpr_dest = latxs_ra_alloc_gpr(XMM_REG(dest));

        latxs_append_ir2_opnd2(LISA_VMSKLTZ_B,  &temp, &xmm_src);
        latxs_append_ir2_opnd2(LISA_MOVFR2GR_D, &gpr_dest, &temp);
        return true;
    }

    IR2_OPND ftemp = latxs_ra_alloc_ftemp();
    IR2_OPND itemp = latxs_ra_alloc_itemp();
    IR2_OPND src_opnd = latxs_ra_alloc_mmx(XMM_REG(src));

    latxs_append_ir2_opnd2(LISA_VMSKLTZ_B, &ftemp, &src_opnd);

    latxs_append_ir2_opnd2(LISA_MOVFR2GR_D, &itemp, &ftemp);
    latxs_append_ir2_opnd2i(LISA_ANDI, &itemp, &itemp, 0xff);
    latxs_store_ir2_to_ir1(&itemp, dest);

    latxs_ra_free_temp(&itemp);
    latxs_ra_free_temp(&ftemp);
    return true;
}

bool latxs_translate_pmulhw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);

    latxs_append_ir2_opnd3(LISA_VMUH_H, &dest, &dest, &src);

    return true;
}

bool latxs_translate_pmulhuw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);

    latxs_append_ir2_opnd3(LISA_VMUH_HU, &dest, &dest, &src);

    return true;
}

bool latxs_translate_pmaddwd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);

    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_VXOR_V, &temp, &temp, &temp);
    latxs_append_ir2_opnd3(LISA_VMADDWEV_W_H, &temp, &dest, &src);
    latxs_append_ir2_opnd3(LISA_VMADDWOD_W_H, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_VBSLL_V, &dest, &temp, 0);

    latxs_ra_free_temp(&temp);
    return true;
}

bool latxs_translate_pmuludq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VMULWEV_D_WU, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);

    latxs_append_ir2_opnd3(LISA_VMULWEV_D_WU, &dest_lo,
                                              &dest_lo, &src_lo);
    return true;
}

bool latxs_translate_pshufd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR1_OPND *opnd2 = ir1_get_opnd(pir1, 2);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    uint64_t imm8 = ir1_opnd_uimm(opnd2);

    latxs_append_ir2_opnd2i(LISA_VSHUF4I_W, &dest, &src, imm8);

    return true;
}

bool latxs_translate_pshufw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = XMM_LOADFP1(opnd0);
    IR2_OPND src = XMM_LOADFP1(opnd1);

    IR1_OPND *imm8_reg = ir1_get_opnd(pir1, 2);
    uint64_t imm8 = ir1_opnd_uimm(imm8_reg);
    if (ir1_opnd_is_mem(opnd1) ||
        (XMM_REG(opnd0) != XMM_REG(opnd1))) {
        latxs_append_ir2_opnd2i(LISA_VORI_B, &dest, &src, 0);
    }
    latxs_append_ir2_opnd2i(LISA_VSHUF4I_H, &dest, &dest, imm8);

    return true;
}

bool latxs_translate_pshuflw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);

    IR2_OPND temp = latxs_ra_alloc_ftemp();

    uint64_t imm8 = ir1_opnd_uimm(ir1_get_opnd(pir1, 2));
    if (ir1_opnd_is_mem(opnd1) ||
        (XMM_REG(opnd0) != XMM_REG(opnd1))) {
        latxs_append_ir2_opnd2i(LISA_VORI_B, &dest, &src, 0);
    }

    latxs_append_ir2_opnd2i(LISA_VORI_B, &temp, &src, 0);
    latxs_append_ir2_opnd2i(LISA_VBSRL_V, &temp, &temp, 8);
    latxs_append_ir2_opnd2i(LISA_VSHUF4I_H, &dest, &dest, imm8);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 1);
    return true;
}

bool latxs_translate_pshufhw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);

    IR2_OPND temp = latxs_ra_alloc_ftemp();

    uint64_t imm8 = ir1_opnd_uimm(ir1_get_opnd(pir1, 2));

    latxs_append_ir2_opnd2i(LISA_VORI_B, &temp, &src, 0);
    latxs_append_ir2_opnd2i(LISA_VSHUF4I_H, &dest, &src, imm8);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_pslldq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    uint8_t imm8 = ir1_opnd_uimm(opnd1);
    if (imm8 > 15) {
        latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
    } else if (imm8 == 0) {
        return true;
    } else {
        latxs_append_ir2_opnd2i(LISA_VBSLL_V, &dest, &dest, imm8);
    }
    return true;
}

bool latxs_translate_psrldq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    uint8_t imm8 = ir1_opnd_uimm(opnd1);
    if (imm8 > 15) {
        latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
    } else if (imm8 == 0) {
        return true;
    } else {
        latxs_append_ir2_opnd2i(LISA_VBSRL_V, &dest, &dest, imm8);
    }
    return true;
}

bool latxs_translate_psubq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    if (option_soft_fpu && latxs_ir1_access_mmx(pir1)) {
        latxs_enter_mmx();
    }

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSUB_D, &dest, &dest, &src);
    } else { /* mmx */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSUB_D, &dest, &dest, &src);
    }
    return true;
}

bool latxs_translate_ucomisd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    /**
     * (bit 6)ZF = 1 if EQ || UOR
     * (bit 2)PF = 1 if UOR (= ZF & CF)
     * (bit 0)CF = 1 if LT || UOR
     */
    lsassert(ir1_opnd_num(pir1) == 2);
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src = XMM_LOAD128(opnd1);
    /* 0. set flag = 0 */
    IR2_OPND flag_zf = latxs_ra_alloc_itemp();
    IR2_OPND flag_pf = latxs_ra_alloc_itemp();
    IR2_OPND flag = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2_(lisa_mov, &flag, &latxs_zero_ir2_opnd);

    /* 1. check ZF, are they equal & unordered? */
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, &latxs_fcc0_ir2_opnd, &dest, &src,
                            FCMP_COND_CUEQ);
    latxs_append_ir2_opnd2(LISA_MOVCF2GR, &flag_zf, &latxs_fcc0_ir2_opnd);

    /* 2. check CF, are they less & unordered? */
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, &latxs_fcc2_ir2_opnd, &dest, &src,
                            FCMP_COND_CULT);
    latxs_append_ir2_opnd2(LISA_MOVCF2GR, &flag, &latxs_fcc2_ir2_opnd);

    /* 3. check PF, are they unordered? (= ZF & CF) */
    latxs_append_ir2_opnd3(LISA_AND, &flag_pf, &flag, &flag_zf);

    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &flag, &flag_zf, ZF_BIT_INDEX,
                             ZF_BIT_INDEX);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &flag, &flag_pf, PF_BIT_INDEX,
                             PF_BIT_INDEX);

    /* 4. mov flag to EFLAGS */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &flag, 0x3f);

    latxs_ra_free_temp(&flag_pf);
    latxs_ra_free_temp(&flag_zf);
    latxs_ra_free_temp(&flag);
    return true;
}

bool latxs_translate_ucomiss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    /**
     * (bit 6)ZF = 1 if EQ || UOR
     * (bit 2)PF = 1 if UOR (= ZF & CF)
     * (bit 0)CF = 1 if LT || UOR
     */
    lsassert(ir1_opnd_num(pir1) == 2);
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src = XMM_LOAD128(opnd1);
    /* 0. set flag = 0 */
    IR2_OPND flag_zf = latxs_ra_alloc_itemp();
    IR2_OPND flag_pf = latxs_ra_alloc_itemp();
    IR2_OPND flag = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2_(lisa_mov, &flag, &latxs_zero_ir2_opnd);

    /* 1. check ZF, are they equal & unordered? */
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_S, &latxs_fcc0_ir2_opnd, &dest, &src,
                            FCMP_COND_CUEQ);
    latxs_append_ir2_opnd2(LISA_MOVCF2GR, &flag_zf, &latxs_fcc0_ir2_opnd);

    /* 2. check CF, are they less & unordered? */
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_S, &latxs_fcc2_ir2_opnd, &dest, &src,
                            FCMP_COND_CULT);
    latxs_append_ir2_opnd2(LISA_MOVCF2GR, &flag, &latxs_fcc2_ir2_opnd);

    /* 3. check PF, are they unordered? (= ZF & CF) */
    latxs_append_ir2_opnd3(LISA_AND, &flag_pf, &flag, &flag_zf);

    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &flag, &flag_zf, ZF_BIT_INDEX,
                             ZF_BIT_INDEX);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &flag, &flag_pf, PF_BIT_INDEX,
                             PF_BIT_INDEX);

    /* 4. mov flag to EFLAGS */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &flag, 0x3f);

    latxs_ra_free_temp(&flag_pf);
    latxs_ra_free_temp(&flag_zf);
    latxs_ra_free_temp(&flag);
    return true;
}

bool latxs_translate_movupd(IR1_INST *pir1)
{
    latxs_translate_movaps(pir1);
    return true;
}

bool latxs_translate_movdqa(IR1_INST *pir1)
{
    latxs_translate_movaps(pir1);
    return true;
}

bool latxs_translate_movdqu(IR1_INST *pir1)
{
    latxs_translate_movaps(pir1);
    return true;
}

bool latxs_translate_movups(IR1_INST *pir1)
{
    latxs_translate_movaps(pir1);
    return true;
}

bool latxs_translate_movapd(IR1_INST *pir1)
{
    latxs_translate_movaps(pir1);
    return true;
}

bool latxs_translate_movaps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src  = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_mem(src)) {
        IR2_OPND xmm_opnd = latxs_ra_alloc_xmm(XMM_REG(dest));
        latxs_load_freg128_from_ir1_mem(&xmm_opnd, src);
    } else if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
        IR2_OPND xmm_opnd = latxs_ra_alloc_xmm(XMM_REG(src));
        latxs_store_freg128_to_ir1_mem(&xmm_opnd, dest);
    } else if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_xmm(src)) {
        IR2_OPND xmm_dest = latxs_ra_alloc_xmm(XMM_REG(dest));
        IR2_OPND xmm_src  = latxs_ra_alloc_xmm(XMM_REG(src));
        latxs_append_ir2_opnd2i(LISA_VORI_B, &xmm_dest, &xmm_src, 0);
    } else {
        lsassert(0);
    }

    return true;
}

bool latxs_translate_movlhps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    lsassert(ir1_opnd_is_xmm(opnd1));

    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VILVL_D, &dest, &src, &dest);
    return true;
}

bool latxs_translate_movhlps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    lsassert(ir1_opnd_is_xmm(opnd1));

    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VILVH_D, &dest, &dest, &src);
    return true;
}

bool latxs_translate_movsd(IR1_INST *pir1)
{
    if (pir1->info->detail->x86.opcode[0] == 0xa5) {
        return latxs_translate_movs(pir1);
    }

    XMM_EXCP(pir1);

    lsassertm(option_xmm128map, "SIMD not 128map enabled\n");

    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src  = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_mem(src)) {
        /*
         * MOVSD (128-bit Legacy SSE version: MOVSD XMM1, m64)
         * DEST[63:0] <= SRC[63:0]
         * DEST[127:64] <= 0
         * DEST[MAXVL-1:128] (Unmodified)
         */
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_load_ir1_mem_to_ir2(&temp, src, EXMode_Z, -1);
        IR2_OPND xmm_dest = latxs_ra_alloc_xmm(XMM_REG(dest));
        latxs_append_ir2_opnd1i(LISA_VLDI, &xmm_dest, 0);
        latxs_append_ir2_opnd2i(LISA_VINSGR2VR_D, &xmm_dest, &temp, 0);
    } else if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
        /*
         * MOVSD/VMOVSD
         * (128-bit versions: MOVSD m64, xmm1 or VMOVSD m64, xmm1)
         * DEST[63:0] <= SRC[63:0]
         */
        IR2_OPND xmm_src = latxs_ra_alloc_xmm(XMM_REG(src));
        latxs_store_freg_to_ir1(&xmm_src, dest, false);
    } else if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_xmm(src)) {
        /*
         * MOVSD/VMOVSD
         * (128-bit versions: MOVSD m64, xmm1 or VMOVSD m64, xmm1)
         * DEST[63:0] <= SRC[63:0]
         */
        IR2_OPND xmm_dest = latxs_ra_alloc_xmm(XMM_REG(dest));
        IR2_OPND xmm_src  = latxs_ra_alloc_xmm(XMM_REG(src));
        latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &xmm_dest, &xmm_src, 0);
    }

    return true;
}

bool latxs_translate_movss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src  = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_mem(src)) {
        /*
         * MOVSS
         * (Legacy SSE version when the source operand is memory
         * and the destination is an XMM register)
         * DEST[31:0] <= SRC[31:0]
         * DEST[127:32] <= 0
         * DEST[MAXVL-1:128] (Unmodified)
         */
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_load_ir1_mem_to_ir2(&temp, src, EXMode_Z, -1);
        IR2_OPND xmm_dest = latxs_ra_alloc_xmm(XMM_REG(dest));
        latxs_append_ir2_opnd1i(LISA_VLDI, &xmm_dest, 0);
        latxs_append_ir2_opnd2i(LISA_VINSGR2VR_W, &xmm_dest, &temp, 0);
        return true;
    } else if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
        /*
         * MOVSS/VMOVSS
         * (when the source operand is an XMM register
         * and the destination is memory)
         * DEST[31:0] <= SRC[31:0]
         */
        IR2_OPND xmm_src = latxs_ra_alloc_xmm(XMM_REG(src));
        latxs_store_freg_to_ir1(&xmm_src, dest, false);
        return true;
    } else if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_xmm(src)) {
        /*
         * MOVSS
         * (Legacy SSE version when the source and
         * destination operands are both XMM registers)
         * DEST[31:0] <= SRC[31:0]
         * DEST[MAXVL-1:32] (Unmodified)
         */
        IR2_OPND xmm_dest = latxs_ra_alloc_xmm(XMM_REG(dest));
        IR2_OPND xmm_src  = latxs_ra_alloc_xmm(XMM_REG(src));
        latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &xmm_dest, &xmm_src, 0);
        return true;
    }

    return true;
}

bool latxs_translate_movhpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src  = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_mem(src) && ir1_opnd_is_xmm(dest)) {
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_load_ir1_to_ir2(&temp, src, EXMode_Z);
        IR2_OPND xmm_dest = latxs_ra_alloc_xmm(XMM_REG(dest));
        latxs_append_ir2_opnd2i(LISA_VINSGR2VR_D, &xmm_dest, &temp, 1);
        return true;
    } else if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
        IR2_OPND temp = latxs_ra_alloc_itemp();
        IR2_OPND xmm_src = latxs_ra_alloc_xmm(XMM_REG(src));
        latxs_append_ir2_opnd2i(LISA_VPICKVE2GR_D, &temp, &xmm_src, 1);
        latxs_store_ir2_to_ir1_mem(&temp, dest, -1);
        return true;
    }

    lsassertm(0, "movhpd not implemented situation.\n");

    return true;
}

bool latxs_translate_movhps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    latxs_translate_movhpd(pir1);
    return true;
}

bool latxs_translate_movlpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src  = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_mem(src) && ir1_opnd_is_xmm(dest)) {
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_load_ir1_to_ir2(&temp, src, EXMode_Z);
        IR2_OPND xmm_dest = latxs_ra_alloc_xmm(XMM_REG(dest));
        latxs_append_ir2_opnd2i(LISA_VINSGR2VR_D, &xmm_dest, &temp, 0);
        return true;
    } else if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
        IR2_OPND xmm_src = latxs_ra_alloc_xmm(XMM_REG(src));
        latxs_store_freg_to_ir1(&xmm_src, dest, false);
        return true;
    }

    lsassertm(0, "movlpd not implemented situation.\n");

    return true;
}

bool latxs_translate_movlps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    latxs_translate_movlpd(pir1);
    return true;
}

bool latxs_translate_movq2dq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);

    latxs_append_ir2_opnd2i(LISA_XVPICKVE_D, &dest, &src, 0);

    return true;
}

bool latxs_translate_movdq2q(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = latxs_load_freg_from_ir1_1(opnd0, false);
    IR2_OPND src = latxs_load_freg_from_ir1_1(opnd1, false);

    latxs_append_ir2_opnd2i(LISA_XVPICKVE_D, &dest, &src, 0);

    return true;
}

bool latxs_translate_addps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd3(LISA_VFADD_S, &dest, &dest, &src);
    return true;
}

bool latxs_translate_addpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd3(LISA_VFADD_D, &dest, &dest, &src);
    return true;
}

bool latxs_translate_andpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &src);
    return true;
}

bool latxs_translate_andnpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd3(LISA_VANDN_V, &dest, &dest, &src);
    return true;
}

bool latxs_translate_andnps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd3(LISA_VANDN_V, &dest, &dest, &src);
    return true;
}

bool latxs_translate_unpcklpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd2i(LISA_VSHUF4I_D, &dest, &src, 0x8);
    return true;
}

bool latxs_translate_unpckhpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd2i(LISA_VSHUF4I_D, &dest, &src, 0xd);
    return true;
}

bool latxs_translate_unpcklps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd3(LISA_VILVL_W, &dest, &src, &dest);
    return true;
}

bool latxs_translate_unpckhps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd3(LISA_VILVH_W, &dest, &src, &dest);
    return true;
}

bool latxs_translate_punpcklqdq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd3(LISA_VILVL_D, &dest, &src, &dest);
    return true;
}

bool latxs_translate_punpckhqdq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd3(LISA_VILVH_D, &dest, &src, &dest);
    return true;
}

bool latxs_translate_shufps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    uint64_t imm8 = ir1_opnd_uimm(ir1_get_opnd(pir1, 2));
    IR2_OPND temp1 = latxs_ra_alloc_ftemp();
    IR2_OPND temp2 = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2i(LISA_VSHUF4I_W, &temp1, &dest, imm8);
    latxs_append_ir2_opnd2i(LISA_VSHUF4I_W, &temp2, &src, imm8 >> 4);
    latxs_append_ir2_opnd3(LISA_VPICKEV_D, &dest, &temp2, &temp1);
    return true;
}

bool latxs_translate_shufpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    uint8_t imm8 = ir1_opnd_uimm(ir1_get_opnd(pir1, 2));
    imm8 &= 3;
    uint8_t shfd_imm8 = 8 | (imm8 & 1) | ((imm8 & 2) << 1);
    latxs_append_ir2_opnd2i(LISA_VSHUF4I_D, &dest, &src, shfd_imm8);
    return true;
}

bool latxs_translate_xorps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &src);
    return true;

}

bool latxs_translate_xorpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    latxs_translate_xorps(pir1);
    return true;
}

bool latxs_translate_subss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_FSUB_S, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_subsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_FSUB_D, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_subps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd3(LISA_VFSUB_S, &dest, &dest, &src);
    return true;
}

bool latxs_translate_subpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd3(LISA_VFSUB_D, &dest, &dest, &src);
    return true;
}

bool latxs_translate_rcpps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd2(LISA_VFRECIP_S, &dest, &src);
    return true;
}

bool latxs_translate_rcpss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2(LISA_VFRECIP_S, &temp, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_sqrtpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd2(LISA_VFSQRT_D, &dest, &src);
    return true;
}

bool latxs_translate_sqrtps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd2(LISA_VFSQRT_S, &dest, &src);
    return true;
}

bool latxs_translate_sqrtsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2(LISA_FSQRT_D, &temp, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_sqrtss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2(LISA_FSQRT_S, &temp, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_rsqrtps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    latxs_append_ir2_opnd2(LISA_VFRSQRT_S, &dest, &src);
    return true;
}

bool latxs_translate_rsqrtss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src  = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2(LISA_FRSQRT_S, &temp, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cvtps2pd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    /* TODO:simply */

    IR2_OPND dest = XMM_LOAD128(ir1_get_opnd(pir1, 0));
    IR2_OPND src = XMM_LOAD128(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    IR2_OPND temp0 = latxs_ra_alloc_ftemp();

    latxs_append_ir2_opnd2(LISA_FCVT_D_S, &temp0, &src);
    latxs_append_ir2_opnd2i(LISA_VSHUF4I_W, &temp, &src, 0x55);
    latxs_append_ir2_opnd2(LISA_FCVT_D_S, &temp, &temp);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 1);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp0, 0);
    return true;
}

bool latxs_translate_cvttsd2si(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *fcc0 = &latxs_fcc0_ir2_opnd;
    IR2_OPND *fcsr = &latxs_fcsr_ir2_opnd;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR2_OPND src_lo = XMM_LOAD128(ir1_get_opnd(pir1, 1));

    IR2_OPND temp_over_flow  = latxs_ra_alloc_itemp();
    IR2_OPND ftemp_over_flow = latxs_ra_alloc_ftemp();

    /*
     * if 32 bits, check data is 0x41e0_0000_0000_0000
     * if 64 bits, check data is 0x43e0_0000_0000_0000
     */
    latxs_append_ir2_opnd2i(LISA_LU52I_D, &temp_over_flow, zero,
                            0x3fe + ir1_opnd_size(opnd0));
    latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &ftemp_over_flow,
                                            &temp_over_flow);

    IR2_OPND temp_under_flow  = latxs_ra_alloc_itemp();
    IR2_OPND ftemp_under_flow = latxs_ra_alloc_ftemp();

    /*
     * if 32 bits, check data is 0xc1e0_0000_0000_0000
     * if 64 bits, check data is 0xc3e0_0000_0000_0000
     */
    latxs_append_ir2_opnd2i(LISA_LU52I_D, &temp_under_flow, zero,
                            (0xbfe + ir1_opnd_size(opnd0)) << 20 >> 20);
    latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &ftemp_under_flow,
                                            &temp_under_flow);

    /*is unoder?*/
    IR2_OPND label_for_flow = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0, &src_lo, &src_lo,
                                 FCMP_COND_CUN);
    latxs_append_ir2_opnd2(LISA_BCNEZ, fcc0, &label_for_flow);

    /*is over flow or under flow*/
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0, &ftemp_over_flow,
                        &src_lo, FCMP_COND_CLE);
    latxs_append_ir2_opnd2(LISA_BCNEZ, fcc0, &label_for_flow);
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0, &src_lo,
              &ftemp_under_flow, FCMP_COND_CLE);
    latxs_append_ir2_opnd2(LISA_BCNEZ, fcc0, &label_for_flow);

    /*not over flow and under flow*/
    IR2_OPND temp_fcsr = latxs_ra_alloc_itemp();
    /*save fscr in temp_int for reload*/
    latxs_append_ir2_opnd2(LISA_MOVFCSR2GR, &temp_fcsr, fcsr);
    latxs_append_ir2_opnd3(LISA_OR, &temp_under_flow,
                                    &temp_under_flow, &temp_fcsr);

    /*set fscr for rounding to zero according to x86 operation*/
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &temp_fcsr, zero, 9, 8);
    latxs_append_ir2_opnd2i(LISA_ORI, &temp_fcsr, &temp_fcsr, 0x1 << 8);
    latxs_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr, &temp_fcsr);

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
        latxs_append_ir2_opnd2(LISA_FTINT_L_D, &ftemp_over_flow, &src_lo);
    } else {
        latxs_append_ir2_opnd2(LISA_FTINT_W_D, &ftemp_over_flow, &src_lo);
    }

    latxs_append_ir2_opnd2(LISA_MOVFR2GR_D, &temp_fcsr, &ftemp_over_flow);
    /* reload fcsr */
    latxs_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr, &temp_under_flow);

    IR2_OPND label_for_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd1(LISA_B, &label_for_exit);
    latxs_append_ir2_opnd1(LISA_LABEL, &label_for_flow);
#ifdef TARGET_X86_64
    if (ir1_opnd_size(opnd0) == 64) {
        latxs_load_imm64_to_ir2(&temp_fcsr, 0x8000000000000000UL);
    } else {
        latxs_load_imm32_to_ir2(&temp_fcsr, 0x80000000, EXMode_Z);
    }
#else
    latxs_load_imm32_to_ir2(&temp_fcsr, 0x80000000, EXMode_Z);
#endif
    latxs_append_ir2_opnd1(LISA_LABEL, &label_for_exit);
    latxs_store_ir2_to_ir1(&temp_fcsr, ir1_get_opnd(pir1, 0));

    latxs_ra_free_temp(&temp_fcsr);
    latxs_ra_free_temp(&temp_over_flow);
    latxs_ra_free_temp(&temp_under_flow);
    latxs_ra_free_temp(&ftemp_over_flow);
    latxs_ra_free_temp(&ftemp_under_flow);

    return true;
}
