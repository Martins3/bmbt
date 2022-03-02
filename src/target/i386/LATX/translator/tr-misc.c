#include "reg-alloc.h"
#include "latx-options.h"
#include "latx-config.h"
#include "shadow-stack.h"
#include "etb.h"
#include "lsenv.h"
#include "flag-lbt.h"
#include "translate.h"

bool translate_endbr32(IR1_INST *pir1) { return true; }
bool translate_endbr64(IR1_INST *pir1) { return true; }

bool translate_bound(IR1_INST *pir1) { return false; }
bool translate_arpl(IR1_INST *pir1) { return false; }
bool translate_cdqe(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cdqe(pir1);
#else
    return false;
#endif
}

bool translate_cqo(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cqo(pir1);
#else
    return false;
#endif
}

bool translate_salc(IR1_INST *pir1) { return false; }

bool translate_lar(IR1_INST *pir1) { return false; }
bool translate_lsl(IR1_INST *pir1) { return false; }
bool translate_syscall(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_syscall(pir1);
#else
    return false;
#endif
}
bool translate_sysret(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_sysret(pir1);
#else
    return false;
#endif
}
bool translate_femms(IR1_INST *pir1) { return false; }
bool translate_getsec(IR1_INST *pir1) { return false; }
bool translate_verr(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_verr(pir1);
#else
    return false;
#endif
}
bool translate_verw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_verw(pir1);
#else
    return false;
#endif
}
bool translate_smsw(IR1_INST *pir1) { return false; }
bool translate_lmsw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_lmsw(pir1);
#else
    return false;
#endif
}
bool translate_vmcall(IR1_INST *pir1) { return false; }
bool translate_vmlaunch(IR1_INST *pir1) { return false; }
bool translate_vmresume(IR1_INST *pir1) { return false; }
bool translate_vmxoff(IR1_INST *pir1) { return false; }
bool translate_monitor(IR1_INST *pir1) { return false; }
bool translate_mwait(IR1_INST *pir1) { return false; }
bool translate_vmrun(IR1_INST *pir1) { return false; }
bool translate_vmmcall(IR1_INST *pir1) { return false; }
bool translate_vmload(IR1_INST *pir1) { return false; }
bool translate_vmsave(IR1_INST *pir1) { return false; }
bool translate_stgi(IR1_INST *pir1) { return false; }
bool translate_clgi(IR1_INST *pir1) { return false; }
bool translate_skinit(IR1_INST *pir1) { return false; }
bool translate_swapgs(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_swapgs(pir1);
#else
    return false;
#endif
}
bool translate_pi2fw(IR1_INST *pir1) { return false; }
bool translate_pi2fd(IR1_INST *pir1) { return false; }
bool translate_pf2iw(IR1_INST *pir1) { return false; }
bool translate_pf2id(IR1_INST *pir1) { return false; }
bool translate_pfnacc(IR1_INST *pir1) { return false; }
bool translate_pfpnacc(IR1_INST *pir1) { return false; }
bool translate_pfcmpge(IR1_INST *pir1) { return false; }
bool translate_pfmin(IR1_INST *pir1) { return false; }
bool translate_pfrcp(IR1_INST *pir1) { return false; }
bool translate_pfrsqrt(IR1_INST *pir1) { return false; }
bool translate_pfsub(IR1_INST *pir1) { return false; }
bool translate_pfadd(IR1_INST *pir1) { return false; }
bool translate_pfcmpgt(IR1_INST *pir1) { return false; }
bool translate_pfmax(IR1_INST *pir1) { return false; }
bool translate_pfrcpit1(IR1_INST *pir1) { return false; }
bool translate_pfrsqit1(IR1_INST *pir1) { return false; }
bool translate_pfsubr(IR1_INST *pir1) { return false; }
bool translate_pfacc(IR1_INST *pir1) { return false; }
bool translate_pfcmpeq(IR1_INST *pir1) { return false; }
bool translate_pfmul(IR1_INST *pir1) { return false; }
bool translate_pfrcpit2(IR1_INST *pir1) { return false; }
bool translate_pmulhrw(IR1_INST *pir1) { return false; }
bool translate_pswapd(IR1_INST *pir1) { return false; }
bool translate_pavgusb(IR1_INST *pir1) { return false; }
bool translate_vmovss(IR1_INST *pir1) { return false; }
bool translate_vmovsd(IR1_INST *pir1) { return false; }
bool translate_vmovups(IR1_INST *pir1) { return false; }
bool translate_vmovupd(IR1_INST *pir1) { return false; }
bool translate_movsldup(IR1_INST *pir1) { return false; }
bool translate_movddup(IR1_INST *pir1) { return false; }
bool translate_vmovhlps(IR1_INST *pir1) { return false; }
bool translate_vmovlps(IR1_INST *pir1) { return false; }
bool translate_vmovlpd(IR1_INST *pir1) { return false; }
bool translate_vmovsldup(IR1_INST *pir1) { return false; }
bool translate_vmovddup(IR1_INST *pir1) { return false; }
bool translate_vunpcklps(IR1_INST *pir1) { return false; }
bool translate_vunpcklpd(IR1_INST *pir1) { return false; }
bool translate_vunpckhps(IR1_INST *pir1) { return false; }
bool translate_vunpckhpd(IR1_INST *pir1) { return false; }
//bool translate_movshdup(IR1_INST *pir1) { return false; }
bool translate_vmovlhps(IR1_INST *pir1) { return false; }
bool translate_vmovhps(IR1_INST *pir1) { return false; }
bool translate_vmovhpd(IR1_INST *pir1) { return false; }
bool translate_vmovshdup(IR1_INST *pir1) { return false; }
bool translate_vmovaps(IR1_INST *pir1) { return false; }
bool translate_vmovapd(IR1_INST *pir1) { return false; }
bool translate_vcvtsi2ss(IR1_INST *pir1) { return false; }
bool translate_vcvtsi2sd(IR1_INST *pir1) { return false; }
bool translate_movntss(IR1_INST *pir1) { return false; }
bool translate_movntsd(IR1_INST *pir1) { return false; }
bool translate_vmovntps(IR1_INST *pir1) { return false; }
bool translate_vmovntpd(IR1_INST *pir1) { return false; }
/* bool translate_cvttps2pi(IR1_INST *pir1) { return false; } */
/* bool translate_cvttpd2pi(IR1_INST *pir1) { return false; } */
bool translate_vcvttss2si(IR1_INST *pir1) { return false; }
bool translate_vcvttsd2si(IR1_INST *pir1) { return false; }
bool translate_vcvtss2si(IR1_INST *pir1) { return false; }
bool translate_vcvtsd2si(IR1_INST *pir1) { return false; }
bool translate_vucomiss(IR1_INST *pir1) { return false; }
bool translate_vucomisd(IR1_INST *pir1) { return false; }
bool translate_vcomiss(IR1_INST *pir1) { return false; }
bool translate_vcomisd(IR1_INST *pir1) { return false; }
bool translate_vmovmskps(IR1_INST *pir1) { return false; }
bool translate_vmovmskpd(IR1_INST *pir1) { return false; }
bool translate_vsqrtss(IR1_INST *pir1) { return false; }
bool translate_vsqrtsd(IR1_INST *pir1) { return false; }
bool translate_vsqrtps(IR1_INST *pir1) { return false; }
bool translate_vsqrtpd(IR1_INST *pir1) { return false; }
bool translate_vrsqrtss(IR1_INST *pir1) { return false; }
bool translate_vrsqrtps(IR1_INST *pir1) { return false; }
bool translate_vrcpss(IR1_INST *pir1) { return false; }
bool translate_vrcpps(IR1_INST *pir1) { return false; }
bool translate_vandps(IR1_INST *pir1) { return false; }
bool translate_vandpd(IR1_INST *pir1) { return false; }
bool translate_vandnps(IR1_INST *pir1) { return false; }
bool translate_vandnpd(IR1_INST *pir1) { return false; }
bool translate_vorps(IR1_INST *pir1) { return false; }
bool translate_vorpd(IR1_INST *pir1) { return false; }
bool translate_vxorps(IR1_INST *pir1) { return false; }
bool translate_vxorpd(IR1_INST *pir1) { return false; }
bool translate_vaddps(IR1_INST *pir1) { return false; }
bool translate_vaddpd(IR1_INST *pir1) { return false; }
bool translate_vaddss(IR1_INST *pir1) { return false; }
bool translate_vaddsd(IR1_INST *pir1) { return false; }
bool translate_vmulps(IR1_INST *pir1) { return false; }
bool translate_vmulpd(IR1_INST *pir1) { return false; }
bool translate_vmulss(IR1_INST *pir1) { return false; }
bool translate_vmulsd(IR1_INST *pir1) { return false; }
bool translate_vcvtss2sd(IR1_INST *pir1) { return false; }
bool translate_vcvtsd2ss(IR1_INST *pir1) { return false; }
bool translate_vcvtps2pd(IR1_INST *pir1) { return false; }
bool translate_vcvtpd2ps(IR1_INST *pir1) { return false; }
bool translate_vcvtdq2ps(IR1_INST *pir1) { return false; }
bool translate_vcvtps2dq(IR1_INST *pir1) { return false; }
bool translate_vcvttps2dq(IR1_INST *pir1) { return false; }
bool translate_vsubps(IR1_INST *pir1) { return false; }
bool translate_vsubpd(IR1_INST *pir1) { return false; }
bool translate_vsubss(IR1_INST *pir1) { return false; }
bool translate_vsubsd(IR1_INST *pir1) { return false; }
bool translate_vminps(IR1_INST *pir1) { return false; }
bool translate_vminpd(IR1_INST *pir1) { return false; }
bool translate_vminss(IR1_INST *pir1) { return false; }
bool translate_vminsd(IR1_INST *pir1) { return false; }
bool translate_vdivps(IR1_INST *pir1) { return false; }
bool translate_vdivpd(IR1_INST *pir1) { return false; }
bool translate_vdivss(IR1_INST *pir1) { return false; }
bool translate_vdivsd(IR1_INST *pir1) { return false; }
bool translate_vmaxps(IR1_INST *pir1) { return false; }
bool translate_vmaxpd(IR1_INST *pir1) { return false; }
bool translate_vmaxss(IR1_INST *pir1) { return false; }
bool translate_vmaxsd(IR1_INST *pir1) { return false; }
bool translate_vpunpcklbw(IR1_INST *pir1) { return false; }
bool translate_vpunpcklwd(IR1_INST *pir1) { return false; }
bool translate_vpunpckldq(IR1_INST *pir1) { return false; }
bool translate_vpacksswb(IR1_INST *pir1) { return false; }
bool translate_vpcmpgtb(IR1_INST *pir1) { return false; }
bool translate_vpcmpgtw(IR1_INST *pir1) { return false; }
bool translate_vpcmpgtd(IR1_INST *pir1) { return false; }
bool translate_vpackuswb(IR1_INST *pir1) { return false; }
bool translate_vpunpckhbw(IR1_INST *pir1) { return false; }
bool translate_vpunpckhwd(IR1_INST *pir1) { return false; }
bool translate_vpunpckhdq(IR1_INST *pir1) { return false; }
bool translate_vpackssdw(IR1_INST *pir1) { return false; }
bool translate_vpunpcklqdq(IR1_INST *pir1) { return false; }
bool translate_vpunpckhqdq(IR1_INST *pir1) { return false; }
bool translate_vmovd(IR1_INST *pir1) { return false; }
bool translate_vmovq(IR1_INST *pir1) { return false; }
bool translate_vmovdqa(IR1_INST *pir1) { return false; }
bool translate_vmovdqu(IR1_INST *pir1) { return false; }
bool translate_vpshufd(IR1_INST *pir1) { return false; }
bool translate_vpshufhw(IR1_INST *pir1) { return false; }
bool translate_vpshuflw(IR1_INST *pir1) { return false; }
bool translate_vpcmpeqb(IR1_INST *pir1) { return false; }
bool translate_vpcmpeqw(IR1_INST *pir1) { return false; }
bool translate_vpcmpeqd(IR1_INST *pir1) { return false; }
// bool translate_emms(IR1_INST *pir1) { return false; }
bool translate_vzeroupper(IR1_INST *pir1) { return false; }
bool translate_vzeroall(IR1_INST *pir1) { return false; }
bool translate_vmread(IR1_INST *pir1) { return false; }
bool translate_extrq(IR1_INST *pir1) { return false; }
bool translate_insertq(IR1_INST *pir1) { return false; }
bool translate_vmwrite(IR1_INST *pir1) { return false; }
bool translate_haddpd(IR1_INST *pir1) { return false; }
bool translate_haddps(IR1_INST *pir1) { return false; }
bool translate_vhaddpd(IR1_INST *pir1) { return false; }
bool translate_vhaddps(IR1_INST *pir1) { return false; }
bool translate_hsubpd(IR1_INST *pir1) { return false; }
bool translate_hsubps(IR1_INST *pir1) { return false; }
bool translate_vhsubpd(IR1_INST *pir1) { return false; }
bool translate_vhsubps(IR1_INST *pir1) { return false; }
bool translate_xave(IR1_INST *pir1) { return false; }
bool translate_lfence(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_lfence(pir1);
#else
    la_append_ir2_opnd0(LISA_DBAR);
    return true;
#endif
}
bool translate_mfence(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_mfence(pir1);
#else
    la_append_ir2_opnd0(LISA_DBAR);
    return true;
#endif
}
bool translate_sfence(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_sfence(pir1);
#else
    la_append_ir2_opnd0(LISA_DBAR);
    return true;
#endif
}
bool translate_clflush(IR1_INST *pir1) { return false; }
bool translate_popcnt(IR1_INST *pir1) { return false; }
bool translate_lzcnt(IR1_INST *pir1) { return false; }
bool translate_vcmpeqps(IR1_INST *pir1) { return false; }
bool translate_vcmpltps(IR1_INST *pir1) { return false; }
bool translate_vcmpleps(IR1_INST *pir1) { return false; }
bool translate_vcmpunordps(IR1_INST *pir1) { return false; }
bool translate_vcmpneqps(IR1_INST *pir1) { return false; }
bool translate_vcmpnltps(IR1_INST *pir1) { return false; }
bool translate_vcmpnleps(IR1_INST *pir1) { return false; }
bool translate_vcmpordps(IR1_INST *pir1) { return false; }
bool translate_vcmpeqpd(IR1_INST *pir1) { return false; }
bool translate_vcmpltpd(IR1_INST *pir1) { return false; }
bool translate_vcmplepd(IR1_INST *pir1) { return false; }
bool translate_vcmpunordpd(IR1_INST *pir1) { return false; }
bool translate_vcmpneqpd(IR1_INST *pir1) { return false; }
bool translate_vcmpnltpd(IR1_INST *pir1) { return false; }
bool translate_vcmpnlepd(IR1_INST *pir1) { return false; }
bool translate_vcmpordpd(IR1_INST *pir1) { return false; }
bool translate_vcmpeqss(IR1_INST *pir1) { return false; }
bool translate_vcmpltss(IR1_INST *pir1) { return false; }
bool translate_vcmpless(IR1_INST *pir1) { return false; }
bool translate_vcmpunordss(IR1_INST *pir1) { return false; }
bool translate_vcmpneqss(IR1_INST *pir1) { return false; }
bool translate_vcmpnltss(IR1_INST *pir1) { return false; }
bool translate_vcmpnless(IR1_INST *pir1) { return false; }
bool translate_vcmpordss(IR1_INST *pir1) { return false; }
bool translate_vcmpeqsd(IR1_INST *pir1) { return false; }
bool translate_vcmpltsd(IR1_INST *pir1) { return false; }
bool translate_vcmplesd(IR1_INST *pir1) { return false; }
bool translate_vcmpunordsd(IR1_INST *pir1) { return false; }
bool translate_vcmpneqsd(IR1_INST *pir1) { return false; }
bool translate_vcmpnltsd(IR1_INST *pir1) { return false; }
bool translate_vcmpnlesd(IR1_INST *pir1) { return false; }
bool translate_vcmpordsd(IR1_INST *pir1) { return false; }
bool translate_vpinsrw(IR1_INST *pir1) { return false; }
bool translate_vpextrw(IR1_INST *pir1) { return false; }
bool translate_vshufps(IR1_INST *pir1) { return false; }
bool translate_vshufpd(IR1_INST *pir1) { return false; }
bool translate_cmpxchg16b(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cmpxchg16b(pir1);
#else
    return false;
#endif
}
bool translate_vmptrst(IR1_INST *pir1) { return false; }
bool translate_vaddsubpd(IR1_INST *pir1) { return false; }
bool translate_vaddsubps(IR1_INST *pir1) { return false; }
bool translate_vpsrlw(IR1_INST *pir1) { return false; }
bool translate_vpsrld(IR1_INST *pir1) { return false; }
bool translate_vpsrlq(IR1_INST *pir1) { return false; }
bool translate_vpaddq(IR1_INST *pir1) { return false; }
bool translate_vpmullw(IR1_INST *pir1) { return false; }
bool translate_vpmovmskb(IR1_INST *pir1) { return false; }
bool translate_vpsubusb(IR1_INST *pir1) { return false; }
bool translate_vpsubusw(IR1_INST *pir1) { return false; }
bool translate_vpminub(IR1_INST *pir1) { return false; }
bool translate_vpand(IR1_INST *pir1) { return false; }
bool translate_vpaddusw(IR1_INST *pir1) { return false; }
bool translate_vpmaxub(IR1_INST *pir1) { return false; }
bool translate_vpandn(IR1_INST *pir1) { return false; }
bool translate_vpavgb(IR1_INST *pir1) { return false; }
bool translate_vpsraw(IR1_INST *pir1) { return false; }
bool translate_vpsrad(IR1_INST *pir1) { return false; }
bool translate_vpavgw(IR1_INST *pir1) { return false; }
bool translate_vpmulhuw(IR1_INST *pir1) { return false; }
bool translate_vpmulhw(IR1_INST *pir1) { return false; }
//bool translate_cvttpd2dq(IR1_INST *pir1) { return false; }
bool translate_vcvttpd2dq(IR1_INST *pir1) { return false; }
bool translate_vcvtdq2pd(IR1_INST *pir1) { return false; }
bool translate_vcvtpd2dq(IR1_INST *pir1) { return false; }
bool translate_vmovntdq(IR1_INST *pir1) { return false; }
bool translate_vpsubsb(IR1_INST *pir1) { return false; }
bool translate_vpsubsw(IR1_INST *pir1) { return false; }
bool translate_vpminsw(IR1_INST *pir1) { return false; }
bool translate_vpor(IR1_INST *pir1) { return false; }
bool translate_vpaddsb(IR1_INST *pir1) { return false; }
bool translate_vpaddsw(IR1_INST *pir1) { return false; }
bool translate_vpmaxsw(IR1_INST *pir1) { return false; }
bool translate_vpxor(IR1_INST *pir1) { return false; }
bool translate_vlddqu(IR1_INST *pir1) { return false; }
bool translate_vpsllw(IR1_INST *pir1) { return false; }
bool translate_vpslld(IR1_INST *pir1) { return false; }
bool translate_vpsllq(IR1_INST *pir1) { return false; }
bool translate_vpmuludq(IR1_INST *pir1) { return false; }
bool translate_vpmaddwd(IR1_INST *pir1) { return false; }
bool translate_vpsadbw(IR1_INST *pir1) { return false; }
bool translate_vmaskmovdqu(IR1_INST *pir1) { return false; }
bool translate_vpsubb(IR1_INST *pir1) { return false; }
bool translate_vpsubw(IR1_INST *pir1) { return false; }
bool translate_vpsubd(IR1_INST *pir1) { return false; }
bool translate_vpsubq(IR1_INST *pir1) { return false; }
bool translate_vpaddb(IR1_INST *pir1) { return false; }
bool translate_vpaddw(IR1_INST *pir1) { return false; }
bool translate_vpaddd(IR1_INST *pir1) { return false; }
bool translate_fstenv(IR1_INST *pir1) { return false; }
bool translate_fclex(IR1_INST *pir1) { return false; }
// bool translate_fninit(IR1_INST *pir1) { return false; }
bool translate_pshufb(IR1_INST *pir1) { return false; }
bool translate_vpshufb(IR1_INST *pir1) { return false; }
bool translate_phaddw(IR1_INST *pir1) { return false; }
bool translate_vphaddw(IR1_INST *pir1) { return false; }
bool translate_phaddd(IR1_INST *pir1) { return false; }
bool translate_vphaddd(IR1_INST *pir1) { return false; }
bool translate_phaddsw(IR1_INST *pir1) { return false; }
bool translate_vphaddsw(IR1_INST *pir1) { return false; }
bool translate_pmaddubsw(IR1_INST *pir1) { return false; }
bool translate_vpmaddubsw(IR1_INST *pir1) { return false; }
bool translate_phsubw(IR1_INST *pir1) { return false; }
bool translate_vphsubw(IR1_INST *pir1) { return false; }
bool translate_phsubd(IR1_INST *pir1) { return false; }
bool translate_vphsubd(IR1_INST *pir1) { return false; }
bool translate_phsubsw(IR1_INST *pir1) { return false; }
bool translate_vphsubsw(IR1_INST *pir1) { return false; }
bool translate_psignb(IR1_INST *pir1) { return false; }
bool translate_vpsignb(IR1_INST *pir1) { return false; }
bool translate_psignw(IR1_INST *pir1) { return false; }
bool translate_vpsignw(IR1_INST *pir1) { return false; }
bool translate_psignd(IR1_INST *pir1) { return false; }
bool translate_vpsignd(IR1_INST *pir1) { return false; }
bool translate_pmulhrsw(IR1_INST *pir1) { return false; }
bool translate_vpmulhrsw(IR1_INST *pir1) { return false; }
bool translate_vpermilps(IR1_INST *pir1) { return false; }
bool translate_vpermilpd(IR1_INST *pir1) { return false; }
bool translate_vptestps(IR1_INST *pir1) { return false; }
bool translate_vptestpd(IR1_INST *pir1) { return false; }
bool translate_pblendvb(IR1_INST *pir1) { return false; }
bool translate_blendvps(IR1_INST *pir1) { return false; }
bool translate_blendvpd(IR1_INST *pir1) { return false; }
bool translate_ptest(IR1_INST *pir1) { return false; }
bool translate_vptest(IR1_INST *pir1) { return false; }
bool translate_vbroadcastss(IR1_INST *pir1) { return false; }
bool translate_vbroadcastsd(IR1_INST *pir1) { return false; }
bool translate_vbroadcastf128(IR1_INST *pir1) { return false; }
bool translate_pabsb(IR1_INST *pir1) { return false; }
bool translate_vpabsb(IR1_INST *pir1) { return false; }
bool translate_pabsw(IR1_INST *pir1) { return false; }
bool translate_vpabsw(IR1_INST *pir1) { return false; }
bool translate_pabsd(IR1_INST *pir1) { return false; }
bool translate_vpabsd(IR1_INST *pir1) { return false; }
bool translate_pmovsxbw(IR1_INST *pir1) { return false; }
bool translate_vpmovsxbw(IR1_INST *pir1) { return false; }
bool translate_pmovsxbd(IR1_INST *pir1) { return false; }
bool translate_vpmovsxbd(IR1_INST *pir1) { return false; }
bool translate_pmovsxbq(IR1_INST *pir1) { return false; }
bool translate_vpmovsxbq(IR1_INST *pir1) { return false; }
bool translate_pmovsxwd(IR1_INST *pir1) { return false; }
bool translate_vpmovsxwd(IR1_INST *pir1) { return false; }
bool translate_pmovsxwq(IR1_INST *pir1) { return false; }
bool translate_vpmovsxwq(IR1_INST *pir1) { return false; }
bool translate_pmovsxdq(IR1_INST *pir1) { return false; }
bool translate_vpmovsxdq(IR1_INST *pir1) { return false; }
bool translate_pmuldq(IR1_INST *pir1) { return false; }
bool translate_vpmuldq(IR1_INST *pir1) { return false; }
bool translate_pcmpeqq(IR1_INST *pir1) { return false; }
bool translate_vpcmpeqq(IR1_INST *pir1) { return false; }
bool translate_movntdqa(IR1_INST *pir1) { return false; }
bool translate_vmovntdqa(IR1_INST *pir1) { return false; }
bool translate_packusdw(IR1_INST *pir1) { return false; }
bool translate_vpackusdw(IR1_INST *pir1) { return false; }
bool translate_vmaskmovps(IR1_INST *pir1) { return false; }
bool translate_vmaskmovpd(IR1_INST *pir1) { return false; }
bool translate_pmovzxbw(IR1_INST *pir1) { return false; }
bool translate_vpmovzxbw(IR1_INST *pir1) { return false; }
bool translate_pmovzxbd(IR1_INST *pir1) { return false; }
bool translate_vpmovzxbd(IR1_INST *pir1) { return false; }
bool translate_pmovzxbq(IR1_INST *pir1) { return false; }
bool translate_vpmovzxbq(IR1_INST *pir1) { return false; }
bool translate_pmovzxwd(IR1_INST *pir1) { return false; }
bool translate_vpmovzxwd(IR1_INST *pir1) { return false; }
bool translate_pmovzxwq(IR1_INST *pir1) { return false; }
bool translate_vpmovzxwq(IR1_INST *pir1) { return false; }
bool translate_pmovzxdq(IR1_INST *pir1) { return false; }
bool translate_vpmovzxdq(IR1_INST *pir1) { return false; }
bool translate_pcmpgtq(IR1_INST *pir1) { return false; }
bool translate_vpcmpgtq(IR1_INST *pir1) { return false; }
bool translate_pminsb(IR1_INST *pir1) { return false; }
bool translate_vpminsb(IR1_INST *pir1) { return false; }
bool translate_pminsd(IR1_INST *pir1) { return false; }
bool translate_vpminsd(IR1_INST *pir1) { return false; }
bool translate_pminuw(IR1_INST *pir1) { return false; }
bool translate_vpminuw(IR1_INST *pir1) { return false; }
bool translate_pminud(IR1_INST *pir1) { return false; }
bool translate_vpminud(IR1_INST *pir1) { return false; }
bool translate_pmaxsb(IR1_INST *pir1) { return false; }
bool translate_vpmaxsb(IR1_INST *pir1) { return false; }
bool translate_pmaxsd(IR1_INST *pir1) { return false; }
bool translate_vpmaxsd(IR1_INST *pir1) { return false; }
bool translate_pmaxuw(IR1_INST *pir1) { return false; }
bool translate_vpmaxuw(IR1_INST *pir1) { return false; }
bool translate_pmaxud(IR1_INST *pir1) { return false; }
bool translate_vpmaxud(IR1_INST *pir1) { return false; }
bool translate_pmulld(IR1_INST *pir1) { return false; }
bool translate_vpmulld(IR1_INST *pir1) { return false; }
bool translate_phminposuw(IR1_INST *pir1) { return false; }
bool translate_vphminposuw(IR1_INST *pir1) { return false; }
bool translate_invept(IR1_INST *pir1) { return false; }
bool translate_invvpid(IR1_INST *pir1) { return false; }
bool translate_vfmaddsub132ps(IR1_INST *pir1) { return false; }
bool translate_vfmaddsub132pd(IR1_INST *pir1) { return false; }
bool translate_vfmsubadd132ps(IR1_INST *pir1) { return false; }
bool translate_vfmsubadd132pd(IR1_INST *pir1) { return false; }
bool translate_vfmadd132ps(IR1_INST *pir1) { return false; }
bool translate_vfmadd132pd(IR1_INST *pir1) { return false; }
bool translate_vfmadd132ss(IR1_INST *pir1) { return false; }
bool translate_vfmadd132sd(IR1_INST *pir1) { return false; }
bool translate_vfmsub132ps(IR1_INST *pir1) { return false; }
bool translate_vfmsub132pd(IR1_INST *pir1) { return false; }
bool translate_vfmsub132ss(IR1_INST *pir1) { return false; }
bool translate_vfmsub132sd(IR1_INST *pir1) { return false; }
bool translate_vfnmadd132ps(IR1_INST *pir1) { return false; }
bool translate_vfnmadd132pd(IR1_INST *pir1) { return false; }
bool translate_vfnmadd132ss(IR1_INST *pir1) { return false; }
bool translate_vfnmadd132sd(IR1_INST *pir1) { return false; }
bool translate_vfnmsub132ps(IR1_INST *pir1) { return false; }
bool translate_vfnmsub132pd(IR1_INST *pir1) { return false; }
bool translate_vfnmsub132ss(IR1_INST *pir1) { return false; }
bool translate_vfnmsub132sd(IR1_INST *pir1) { return false; }
bool translate_vfmaddsub213ps(IR1_INST *pir1) { return false; }
bool translate_vfmaddsub213pd(IR1_INST *pir1) { return false; }
bool translate_vfmsubadd213ps(IR1_INST *pir1) { return false; }
bool translate_vfmsubadd213pd(IR1_INST *pir1) { return false; }
bool translate_vfmadd213ps(IR1_INST *pir1) { return false; }
bool translate_vfmadd213pd(IR1_INST *pir1) { return false; }
bool translate_vfmadd213ss(IR1_INST *pir1) { return false; }
bool translate_vfmadd213sd(IR1_INST *pir1) { return false; }
bool translate_vfmsub213ps(IR1_INST *pir1) { return false; }
bool translate_vfmsub213pd(IR1_INST *pir1) { return false; }
bool translate_vfmsub213ss(IR1_INST *pir1) { return false; }
bool translate_vfmsub213sd(IR1_INST *pir1) { return false; }
bool translate_vfnmadd213ps(IR1_INST *pir1) { return false; }
bool translate_vfnmadd213pd(IR1_INST *pir1) { return false; }
bool translate_vfnmadd213ss(IR1_INST *pir1) { return false; }
bool translate_vfnmadd213sd(IR1_INST *pir1) { return false; }
bool translate_vfnmsub213ps(IR1_INST *pir1) { return false; }
bool translate_vfnmsub213pd(IR1_INST *pir1) { return false; }
bool translate_vfnmsub213ss(IR1_INST *pir1) { return false; }
bool translate_vfnmsub213sd(IR1_INST *pir1) { return false; }
bool translate_vfmaddsub231ps(IR1_INST *pir1) { return false; }
bool translate_vfmaddsub231pd(IR1_INST *pir1) { return false; }
bool translate_vfmsubadd231ps(IR1_INST *pir1) { return false; }
bool translate_vfmsubadd231pd(IR1_INST *pir1) { return false; }
bool translate_vfmadd231ps(IR1_INST *pir1) { return false; }
bool translate_vfmadd231pd(IR1_INST *pir1) { return false; }
bool translate_vfmadd231ss(IR1_INST *pir1) { return false; }
bool translate_vfmadd231sd(IR1_INST *pir1) { return false; }
bool translate_vfmsub231ps(IR1_INST *pir1) { return false; }
bool translate_vfmsub231pd(IR1_INST *pir1) { return false; }
bool translate_vfmsub231ss(IR1_INST *pir1) { return false; }
bool translate_vfmsub231sd(IR1_INST *pir1) { return false; }
bool translate_vfnmadd231ps(IR1_INST *pir1) { return false; }
bool translate_vfnmadd231pd(IR1_INST *pir1) { return false; }
bool translate_vfnmadd231ss(IR1_INST *pir1) { return false; }
bool translate_vfnmadd231sd(IR1_INST *pir1) { return false; }
bool translate_vfnmsub231ps(IR1_INST *pir1) { return false; }
bool translate_vfnmsub231pd(IR1_INST *pir1) { return false; }
bool translate_vfnmsub231ss(IR1_INST *pir1) { return false; }
bool translate_vfnmsub231sd(IR1_INST *pir1) { return false; }
bool translate_aesimc(IR1_INST *pir1) { return false; }
bool translate_vaesimc(IR1_INST *pir1) { return false; }
bool translate_aesenc(IR1_INST *pir1) { return false; }
bool translate_vaesenc(IR1_INST *pir1) { return false; }
bool translate_aesenclast(IR1_INST *pir1) { return false; }
bool translate_vaesenclast(IR1_INST *pir1) { return false; }
bool translate_aesdec(IR1_INST *pir1) { return false; }
bool translate_vaesdec(IR1_INST *pir1) { return false; }
bool translate_aesdeclast(IR1_INST *pir1) { return false; }
bool translate_vaesdeclast(IR1_INST *pir1) { return false; }
bool translate_movbe(IR1_INST *pir1) { return false; }
bool translate_crc32(IR1_INST *pir1) { return false; }
bool translate_vperm2f128(IR1_INST *pir1) { return false; }
bool translate_roundps(IR1_INST *pir1) { return false; }
bool translate_vroundps(IR1_INST *pir1) { return false; }
bool translate_roundpd(IR1_INST *pir1) { return false; }
bool translate_vroundpd(IR1_INST *pir1) { return false; }
bool translate_roundss(IR1_INST *pir1) { return false; }
bool translate_vroundss(IR1_INST *pir1) { return false; }
bool translate_roundsd(IR1_INST *pir1) { return false; }
bool translate_vroundsd(IR1_INST *pir1) { return false; }
bool translate_blendps(IR1_INST *pir1) { return false; }
bool translate_vblendps(IR1_INST *pir1) { return false; }
bool translate_blendpd(IR1_INST *pir1) { return false; }
bool translate_vblendpd(IR1_INST *pir1) { return false; }
bool translate_pblendw(IR1_INST *pir1) { return false; }
bool translate_vpblendvw(IR1_INST *pir1) { return false; }
bool translate_palignr(IR1_INST *pir1) { return false; }
bool translate_vpalignr(IR1_INST *pir1) { return false; }
bool translate_pextrb(IR1_INST *pir1) { return false; }
bool translate_vpextrb(IR1_INST *pir1) { return false; }
bool translate_pextrd(IR1_INST *pir1) { return false; }
bool translate_pextrq(IR1_INST *pir1) { return false; }
bool translate_vpextrd(IR1_INST *pir1) { return false; }
bool translate_extractps(IR1_INST *pir1) { return false; }
bool translate_vextractps(IR1_INST *pir1) { return false; }
bool translate_vinsertf128(IR1_INST *pir1) { return false; }
bool translate_vextractf128(IR1_INST *pir1) { return false; }
bool translate_pinsrb(IR1_INST *pir1) { return false; }
bool translate_vpinsrb(IR1_INST *pir1) { return false; }
bool translate_insertps(IR1_INST *pir1) { return false; }
bool translate_vinsertps(IR1_INST *pir1) { return false; }
bool translate_pinsrd(IR1_INST *pir1) { return false; }
bool translate_pinsrq(IR1_INST *pir1) { return false; }
bool translate_vpinsrd(IR1_INST *pir1) { return false; }
bool translate_vpinsrq(IR1_INST *pir1) { return false; }
bool translate_dpps(IR1_INST *pir1) { return false; }
bool translate_vdpps(IR1_INST *pir1) { return false; }
bool translate_dppd(IR1_INST *pir1) { return false; }
bool translate_vdppd(IR1_INST *pir1) { return false; }
bool translate_mpsadbw(IR1_INST *pir1) { return false; }
bool translate_vmpsadbw(IR1_INST *pir1) { return false; }
bool translate_pclmulqdq(IR1_INST *pir1) { return false; }
bool translate_vpclmulqdq(IR1_INST *pir1) { return false; }
bool translate_vblendvps(IR1_INST *pir1) { return false; }
bool translate_vblendvpd(IR1_INST *pir1) { return false; }
bool translate_vpblendvb(IR1_INST *pir1) { return false; }
bool translate_pcmpestrm(IR1_INST *pir1) { return false; }
bool translate_vpcmpestrm(IR1_INST *pir1) { return false; }
bool translate_pcmpestri(IR1_INST *pir1) { return false; }
bool translate_vcmpestri(IR1_INST *pir1) { return false; }
bool translate_pcmpistrm(IR1_INST *pir1) { return false; }
bool translate_vpcmpistrm(IR1_INST *pir1) { return false; }
bool translate_pcmpistri(IR1_INST *pir1) { return false; }
bool translate_vpcmpistri(IR1_INST *pir1) { return false; }
bool translate_aeskeygenassist(IR1_INST *pir1) { return false; }
bool translate_vaeskeygenassist(IR1_INST *pir1) { return false; }
bool translate_vpsrldq(IR1_INST *pir1) { return false; }
bool translate_vpslldq(IR1_INST *pir1) { return false; }
bool translate_vldmxcsr(IR1_INST *pir1) { return false; }
bool translate_vstmxcsr(IR1_INST *pir1) { return false; }
bool translate_vmptrld(IR1_INST *pir1) { return false; }
bool translate_vmclear(IR1_INST *pir1) { return false; }
bool translate_vmxon(IR1_INST *pir1) { return false; }
// bool translate_wait(IR1_INST *pir1) { return false; }
bool translate_movsxd(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_movsxd(pir1);
#else
    return false;
#endif
}

bool translate_ret_without_ss_opt(IR1_INST *pir1);
bool translate_ret_with_ss_opt(IR1_INST *pir1);
void ss_gen_push(IR1_INST *pir1);

static inline void set_CPUX86State_error_code(ENV *lsenv, int error_code)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    cpu->error_code = error_code;
}

bool translate_ins(IR1_INST *pir1) {
#ifdef CONFIG_SOFTMMU
    return latxs_translate_ins(pir1);
#else
    printf("FIXME: the instruction INSD is used, but LATX do not support it.\n");
    return true;
#endif
}

#ifndef CONFIG_SOFTMMU
static inline void set_CPUX86State_exception_is_int(ENV *lsenv, int exception_is_int)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    cpu->exception_is_int = exception_is_int;
}

static inline void set_CPUState_can_do_io(ENV *lsenv, int can_do_io)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    CPUState *cs = env_cpu(cpu);
    cs->can_do_io = can_do_io;
}

static void siglongjmp_cpu_jmp_env(void)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;

    /* siglongjmp will skip the execution of latx_after_exec_tb
     * which is expected to reset top_bias/top
     */
    TranslationBlock *last_tb =
        (TranslationBlock *)lsenv_get_last_executed_tb(lsenv);
#ifdef CONFIG_LATX_DEBUG
    latx_after_exec_trace_tb(cpu, last_tb);
#endif
    latx_after_exec_rotate_fpu(cpu, last_tb);

    CPUState *cpu_state = env_cpu(cpu);
    siglongjmp(cpu_state->jmp_env, 1);
}

/* Instead save intno in helper_raise_int, we save intno in translate_int. */
static void helper_raise_int(void)
{
    set_CPUX86State_error_code(lsenv, 0);
    set_CPUX86State_exception_is_int(lsenv, 1);
    set_CPUState_can_do_io(lsenv, 1);
    siglongjmp_cpu_jmp_env();
}
#endif

void ss_gen_push(IR1_INST *pir1) {
    IR2_OPND ss_opnd = ra_alloc_ss();
    // 1. store esp onto ss
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    la_append_ir2_opnd2i_em(LISA_STORE_ADDRX, esp_opnd, ss_opnd, (int)offsetof(SS_ITEM, x86_esp));

    /* debug */
    //int debug_type = 0;
    // 2. store callee_addr onto ss
    if (ir1_opcode(pir1) == X86_INS_CALL && !ir1_is_indirect_call(pir1)){
        IR2_OPND callee_addr_opnd = ra_alloc_itemp();
        load_ireg_from_addrx(callee_addr_opnd, ir1_target_addr(pir1));
        la_append_ir2_opnd2i_em(LISA_STORE_ADDRX, callee_addr_opnd, ss_opnd, (int)offsetof(SS_ITEM, x86_callee_addr));
        ra_free_temp(callee_addr_opnd);
        //debug_type = 1;//call
    } else {
        assert(ir1_is_indirect_call(pir1));
        IR2_OPND next_execut_pc = ra_alloc_dbt_arg2();
        la_append_ir2_opnd2i_em(LISA_STORE_ADDRX, next_execut_pc, ss_opnd, (int)offsetof(SS_ITEM, x86_callee_addr));
        //debug_type = 2;//callin
    }

    /* 3. store ret_tb onto ss */
    IR2_OPND ret_tb_opnd = ra_alloc_itemp();
    ETB *etb = etb_find(ir1_addr_next(pir1));
    load_ireg_from_addr(ret_tb_opnd, (ADDR)etb);
    la_append_ir2_opnd2i_em(LISA_STORE_ADDR, ret_tb_opnd, ss_opnd, (int)offsetof(SS_ITEM, return_tb));

    // 4. adjust ss (ss_curr++ )
    la_append_ir2_opnd2i_em(LISA_ADDI_ADDR, ss_opnd, ss_opnd, sizeof(SS_ITEM));

    ra_free_temp(ret_tb_opnd);
    return;
}

bool translate_call(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_call(pir1);
#else
    if(ir1_is_indirect_call(pir1)){
        return translate_callin(pir1);
    }
    /* 1. adjust ssp */
    IR2_OPND esp_opnd = ra_alloc_gpr(4);
    la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, esp_opnd, esp_opnd, -4);

    /* 2. save return address onto stack */
    IR2_OPND return_addr_opnd = ra_alloc_itemp();
    load_ireg_from_addrx(return_addr_opnd, ir1_addr_next(pir1));

    if (cpu_get_guest_base() != 0) {
        IR2_OPND tmp = ra_alloc_itemp();
        IR2_OPND gbase = ra_alloc_guest_base();
        if (!ir2_opnd_is_address(&esp_opnd)) {
            la_append_ir2_opnd2_em(LISA_MOV_ADDRX, esp_opnd, esp_opnd);
        }
        la_append_ir2_opnd3_em(LISA_ADD_ADDR, tmp, esp_opnd, gbase);
        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
        la_append_ir2_opnd2i_em(LISA_STORE_ADDRX, return_addr_opnd, tmp, 0);
        ra_free_temp(tmp);
    } else {
        la_append_ir2_opnd2i_em(LISA_STORE_ADDRX, return_addr_opnd, esp_opnd, 0);
    }

    /* 3. push esp, callee_addr, ret_tb onto shadow stack */
    if(option_shadow_stack)
        ss_gen_push(pir1);

    /* QEMU exit_tb & goto_tb */
    tr_generate_exit_tb(pir1, 0);

    ra_free_temp(return_addr_opnd);
    return true;
#endif
}

bool translate_callnext(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_callnext(pir1);
#else
    /* 1. load next_instr_addr to tmp_reg */
    IR2_OPND next_addr = ra_alloc_itemp();
    load_ireg_from_addrx(next_addr, ir1_addr_next(pir1));

    /* 2. store next_addr to x86_stack */
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, esp_opnd, esp_opnd, -4);

    if (cpu_get_guest_base() != 0) {
        IR2_OPND tmp = ra_alloc_itemp();
        IR2_OPND gbase = ra_alloc_guest_base();
        if (!ir2_opnd_is_address(&esp_opnd)) {
            la_append_ir2_opnd2_em(LISA_MOV_ADDRX, esp_opnd, esp_opnd);
        }
        la_append_ir2_opnd3_em(LISA_ADD_ADDR, tmp, esp_opnd, gbase);
        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
        la_append_ir2_opnd2i_em(LISA_STORE_ADDRX, next_addr, tmp, 0);
        ra_free_temp(tmp);
    } else {
        la_append_ir2_opnd2i_em(LISA_STORE_ADDRX, next_addr, esp_opnd, 0);
    }

    ra_free_temp(next_addr);

    return true;
#endif
}

bool translate_callin(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_callin(pir1);
#else
    /* 1. set successor x86 address */
    IR2_OPND succ_x86_addr_opnd = ra_alloc_dbt_arg2();
    load_ireg_from_ir1_2(succ_x86_addr_opnd, ir1_get_opnd(pir1, 0), ZERO_EXTENSION,
                         false);

    /*
     * 2. adjust esp
     * NOTE: There is a corner case during wine kernel32 virtual unit test.
     * If esp updated at the begin of the insn, once stw trigger the segv,
     * The esp will be conflict and gcc stack check will be failed.
     * Solutuion is same as TCG did, update esp to temp reg, once store
'    * insn is successful, esp will be updated.
     */
    IR2_OPND esp_opnd = ra_alloc_gpr(4);
    IR2_OPND temp_esp = ra_alloc_itemp();

    la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, temp_esp, esp_opnd, -4);

    /* 3. save return address onto stack */
    IR2_OPND return_addr_opnd = ra_alloc_itemp();
    load_ireg_from_addrx(return_addr_opnd, ir1_addr_next(pir1));

    if (cpu_get_guest_base() != 0) {
        IR2_OPND tmp = ra_alloc_itemp();
        IR2_OPND gbase = ra_alloc_guest_base();
        if (!ir2_opnd_is_address(&esp_opnd)) {
            la_append_ir2_opnd2_em(LISA_MOV_ADDRX, temp_esp, esp_opnd);
        }
        la_append_ir2_opnd3_em(LISA_ADD_ADDR, tmp, temp_esp, gbase);
        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
        la_append_ir2_opnd2i_em(LISA_STORE_ADDRX, return_addr_opnd, tmp, 0);
        ra_free_temp(tmp);
    } else {
        la_append_ir2_opnd2i_em(LISA_STORE_ADDRX, return_addr_opnd, temp_esp, 0);
    }

    la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, esp_opnd, esp_opnd, -4);
    ra_free_temp(return_addr_opnd);
    ra_free_temp(temp_esp);
    /* 4. push esp, callee_addr, ret_tb onto shadow statck */
    if (option_shadow_stack) {
        ss_gen_push(pir1);
    }

    /* 5. adjust em to defaul em */
    tr_adjust_em();

    /* 6. indirect linkage */
    /* env->tr_data->curr_tb->generate_tb_linkage_indirect(); */
    tr_generate_exit_tb(pir1, 0);
    return true;
#endif
}

bool translate_iret(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_iret(pir1);
#else
    /* 1. load ret_addr into $25 */
    IR1_OPND seg_opnd;
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    IR2_OPND eflags_opnd = ra_alloc_eflags();
    IR2_OPND cs_opnd = ra_alloc_itemp();
    IR2_OPND return_addr_opnd = ra_alloc_dbt_arg2();
    load_ireg_from_ir1_2(return_addr_opnd, &esp_mem32_ir1_opnd,
                         UNKNOWN_EXTENSION, false);

    la_append_ir2_opnd2i(LISA_LD_W, cs_opnd, esp_opnd, 4);
    la_append_ir2_opnd2i(LISA_LD_W, eflags_opnd, esp_opnd, 8);
    ir1_opnd_build_reg(&seg_opnd, 16, X86_REG_CS);
    store_ireg_to_ir1_seg(cs_opnd, &seg_opnd);
    ra_free_temp(cs_opnd);

    la_append_ir2_opnd1i(LISA_X86MTFLAG, eflags_opnd, 0x3f);
    /* 2. adjust esp */
    la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, esp_opnd, esp_opnd, 12);

    tr_generate_exit_tb(pir1, 0);

    return true;
#endif
}

bool translate_ret_without_ss_opt(IR1_INST *pir1)
{
    /* 1. load ret_addr into $25 */
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    IR2_OPND return_addr_opnd = ra_alloc_dbt_arg2();
    load_ireg_from_ir1_2(return_addr_opnd, &esp_mem32_ir1_opnd,
                         UNKNOWN_EXTENSION, false);

    /* 2. adjust esp */
    if (pir1 != NULL && ir1_opnd_num(pir1) &&
        ir1_opnd_type(ir1_get_opnd(pir1, 0)) == X86_OP_IMM)
        la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, esp_opnd, esp_opnd,
                          ir1_opnd_uimm(ir1_get_opnd(pir1, 0)) + 4);
    else
        la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, esp_opnd, esp_opnd, 4);

    tr_generate_exit_tb(pir1, 0);

    return true;
}


bool translate_ret_with_ss_opt(IR1_INST *pir1) {
    int esp_change_imm = 4;
    if(pir1!=NULL && ir1_opnd_num(pir1) && ir1_opnd_type(ir1_get_opnd(pir1, 0))==X86_OP_IMM)
        esp_change_imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 0)) + 4;

    /*1. load supposed returning target TB */
    IR2_OPND ss_opnd = ra_alloc_ss();
    IR2_OPND ss_etb = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_LOAD_ADDR, ss_etb, ss_opnd, -(int)sizeof(SS_ITEM)+(int)offsetof(SS_ITEM, return_tb));
    IR2_OPND supposed_tb = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_LOAD_ADDR, supposed_tb, ss_etb, offsetof(ETB, tb));
    ra_free_temp(ss_etb);

    /*2. pop return target address from x86 stack */
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    IR2_OPND return_addr_opnd = ra_alloc_dbt_arg2();
    la_append_ir2_opnd2i_em(LISA_LOAD_ADDRX, return_addr_opnd, esp_opnd, 0);
    IR2_OPND label_return_to_bt = ir2_opnd_new_type(IR2_OPND_LABEL);
    /*2.1 check if TB is alloced */
    la_append_ir2_opnd3(LISA_BEQ, supposed_tb, zero_ir2_opnd, label_return_to_bt);
    la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, esp_opnd, esp_opnd, esp_change_imm); /* esp pop */

    // 3. load supposed returning x86 address
    IR2_OPND supposed_x86_addr = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_LOAD_ADDRX, supposed_x86_addr, supposed_tb, offsetof(TranslationBlock, pc));

    // 4. check if x86 address matches
    IR2_OPND label_match_fail = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BNE, supposed_x86_addr, return_addr_opnd, label_match_fail);
    ra_free_temp(supposed_x86_addr);

    // 5. find next tb,dirctly jmp
    IR2_OPND supposed_native_addr = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_LOAD_ADDR, supposed_native_addr, supposed_tb,
        offsetof(TranslationBlock, tc) + offsetof(struct tb_tc,ptr));
    la_append_ir2_opnd2i_em(LISA_ADDI_ADDR, ss_opnd, ss_opnd, -(int)sizeof(SS_ITEM));

    // 6. check if top_out == top_in, if so, jump to native code, else rotate the fpu
    IR2_OPND last_executed_tb = ra_alloc_dbt_arg1();
    TranslationBlock *curr_tb = lsenv->tr_data->curr_tb;
    load_ireg_from_addr(last_executed_tb, (ADDR)curr_tb);
    IR2_OPND top_out = ra_alloc_itemp();
    IR2_OPND top_in = ra_alloc_itemp();
    IR2_OPND rotate_step = ra_alloc_dbt_arg1();
    IR2_OPND rotate_ret_addr = ra_alloc_dbt_arg2();
    IR2_OPND label_no_rotate = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd2i_em(LISA_LD_BU, top_out, last_executed_tb,
        offsetof(TranslationBlock,_top_out));
    la_append_ir2_opnd2i_em(LISA_LD_BU, top_in, supposed_tb,
        offsetof(TranslationBlock, _top_in));
    la_append_ir2_opnd3(LISA_BEQ, top_out, top_in, label_no_rotate);
    ra_free_temp(supposed_tb);
    //6.1 top_out != top_in, rotate fpu
    la_append_ir2_opnd3(LISA_SUB_W, rotate_step, top_out, top_in);
    la_append_ir2_opnda(LISA_B, native_rotate_fpu_by); //TODO reloc
    la_append_ir2_opnd2_em(LISA_MOV64, rotate_ret_addr, supposed_native_addr);
    ra_free_temp(top_in);
    ra_free_temp(top_out);

    //6.2 top_out == top_in, run next tb directly
    la_append_ir2_opnd1(LISA_LABEL, label_no_rotate);
    la_append_ir2_opnd2i(LISA_JIRL, zero_ir2_opnd, supposed_native_addr, 0);
    ra_free_temp(supposed_native_addr);

    // 7. tb is null, prepare the last executed tb, and jump to the context switch
    la_append_ir2_opnd1(LISA_LABEL, label_return_to_bt);
    la_append_ir2_opnd2i_em(LISA_ADDI_ADDR, ss_opnd, ss_opnd, -(int)sizeof(SS_ITEM));

    IR2_OPND tb_ptr_opnd = ra_alloc_dbt_arg1();
    uint32_t value = (uint64_t)curr_tb;
    if (cpu_get_guest_base() == 0) {
        load_ireg_from_imm32(tb_ptr_opnd, value, ZERO_EXTENSION);
        la_append_ir2_opnda(LISA_B, context_switch_native_to_bt_ret_0);//TODO reloc
    } else {
        load_ireg_from_imm64(tb_ptr_opnd, (ADDR)curr_tb);
        la_append_ir2_opnda(LISA_B, context_switch_native_to_bt_ret_0);//TODO reloc
    }

    // 8. x86 address does not match
    // restore esp, because the adjustment of shadow stack needs the original esp
    la_append_ir2_opnd1(LISA_LABEL, label_match_fail);
    la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, esp_opnd, esp_opnd, -esp_change_imm);
    IR2_OPND esp_change_bytes = ra_alloc_mda();
    la_append_ir2_opnd2i_em(LISA_ADDI_W, esp_change_bytes, zero_ir2_opnd, esp_change_imm);
    // prepare the last executed tb, and jump to the adjustment
    load_ireg_from_imm64(tb_ptr_opnd, (ADDR)curr_tb );
    la_append_ir2_opnda(LISA_B, ss_match_fail_native);//TODO reloc

    return true;
}

bool translate_ret(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_ret(pir1);
#else
    if (option_shadow_stack) {
        return translate_ret_with_ss_opt(pir1);
    } else {
        return translate_ret_without_ss_opt(pir1);
    }
#endif
}

bool translate_jmp(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jmp(pir1);
#else
    if(ir1_is_indirect_jmp(pir1)){
        return translate_jmpin(pir1);
    }
    /* if (env->tr_data->static_translation) */
    /*     env->tr_data->curr_tb->sbt_generate_linkage_jump_target(); */
    /* else */
    /*     env->tr_data->curr_tb->generate_tb_linkage(0); */

    tr_generate_exit_tb(pir1, 1);
    return true;
#endif
}

bool translate_jmpin(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jmpin(pir1);
#else
    /* 1. set successor x86 address */
    IR2_OPND succ_x86_addr_opnd = ra_alloc_dbt_arg2();
    load_ireg_from_ir1_2(succ_x86_addr_opnd, ir1_get_opnd(pir1, 0), ZERO_EXTENSION,
                         false);

    tr_adjust_em();

    /* 2. indirect linkage */
    /* env->tr_data->curr_tb->generate_tb_linkage_indirect(); */
    tr_generate_exit_tb(pir1, 1);
    return true;
#endif
}

bool translate_leave(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_leave(pir1);
#else
    IR2_OPND rsp_opnd = ra_alloc_gpr(4);
    IR2_OPND rbp_opnd = ra_alloc_gpr(5);

    la_append_ir2_opnd2_em(LISA_MOV_ADDRX, rsp_opnd, rbp_opnd);
    if (cpu_get_guest_base() != 0) {
        IR2_OPND gbase = ra_alloc_guest_base();
        if (!ir2_opnd_is_address(&rbp_opnd)) {
            la_append_ir2_opnd2_em(LISA_MOV_ADDRX, rbp_opnd, rbp_opnd);
        }
        la_append_ir2_opnd3_em(LISA_ADD_ADDR, rbp_opnd, rsp_opnd, gbase);
        ir2_opnd_set_em(&rbp_opnd, EM_MIPS_ADDRESS, 32);
    }
    /*
     * NOTE: if previous insn as below
     * add        ebp, 0x5c
     * Once result of ebp is sign-extension, that will cause segv.
     * To avoid this, invoke esp instead of ebp because ebp is equal to
     * esp with zero extension.
     * This change is win-win because there is no insn increasement.
     */
    la_append_ir2_opnd2i_em(LISA_LOAD_ADDRX, rbp_opnd, rsp_opnd, 0);
    la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, rsp_opnd, rsp_opnd, 4);

    return true;
#endif
}

bool translate_int(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_int(pir1);
#else
    /* save fcsr for native */
    IR2_OPND fcsr_value_opnd = ra_alloc_itemp();
    la_append_ir2_opnd2(LISA_MOVFCSR2GR, fcsr_value_opnd, fcsr_ir2_opnd);
    la_append_ir2_opnd2i(LISA_ST_W, fcsr_value_opnd, env_ir2_opnd,
                          lsenv_offset_of_fcsr(lsenv));

    /* * store registers to env, or if context_switch_native_to_bt is better? */
    tr_save_registers_to_env(0xff, 0xff, 0xff, 0xff, 0x1 | options_to_save());

    /* * store intno to CPUState */
    IR2_OPND intno = ra_alloc_itemp();
    load_ireg_from_addrx(intno, ir1_get_opnd(pir1, 0)->imm);
    la_append_ir2_opnd2i(LISA_ST_W, intno, env_ir2_opnd,
                      lsenv_offset_exception_index(lsenv));
    ra_free_temp(intno);

    /* * store next pc to CPUX86State */
    IR2_OPND next_pc = ra_alloc_itemp();
    //load_ireg_from_addrx(next_pc, pir1->_inst_length + pir1->_addr);
    load_ireg_from_addrx(next_pc, ir1_addr_next(pir1));
    la_append_ir2_opnd2i_em(LISA_STORE_ADDRX, next_pc, env_ir2_opnd,
                      lsenv_offset_exception_next_eip(lsenv));
    ra_free_temp(next_pc);

    /* * store curr_tb to last_executed_tb */
    IR2_OPND tb = ra_alloc_itemp();
    load_ireg_from_addr(tb, (ADDR)lsenv->tr_data->curr_tb);
    la_append_ir2_opnd2i_em(LISA_STORE_ADDR, tb, env_ir2_opnd,
                      lsenv_offset_of_last_executed_tb(lsenv));
    ra_free_temp(tb);

    /* * call helper function */
    IR2_OPND helper_addr_opnd = ra_alloc_dbt_arg2();
    load_ireg_from_addr(helper_addr_opnd, (ADDR)helper_raise_int);

    la_append_ir2_opnd2i(LISA_JIRL, ir2_opnd_new(IR2_OPND_IREG, 1), helper_addr_opnd, 0);
    /* * load registers from env */
    tr_load_registers_from_env(0xff, 0xff, 0xff, 0xff, 0x1|options_to_save());

    /* retore fcsr for native */
    la_append_ir2_opnd2i(LISA_LD_W, fcsr_value_opnd, env_ir2_opnd,
                          lsenv_offset_of_fcsr(lsenv));
    la_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr_ir2_opnd, fcsr_value_opnd);
    ra_free_temp(fcsr_value_opnd);

    return true;
#endif
}

bool translate_hlt(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_hlt(pir1);
#else
    la_append_ir2_opnd2i(LISA_ANDI, zero_ir2_opnd, zero_ir2_opnd, 0);
    return true;
#endif
}

bool translate_rdtsc(IR1_INST *pir1) {
#ifdef CONFIG_SOFTMMU
    return latxs_translate_rdtsc(pir1);
#else
    IR2_OPND ir2_eax = ra_alloc_gpr(ir1_opnd_base_reg_num(&eax_ir1_opnd));
    IR2_OPND ir2_edx = ra_alloc_gpr(ir1_opnd_base_reg_num(&edx_ir1_opnd));
    la_append_ir2_opnd2(LISA_RDTIME_D, ir2_eax, zero_ir2_opnd);
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, ir2_edx, ir2_eax, 63, 32);
    ir2_opnd_set_em(&ir2_eax, UNKNOWN_EXTENSION, 32);
    ir2_opnd_set_em(&ir2_edx, UNKNOWN_EXTENSION, 32);
    return true;
#endif
}

bool translate_rdtscp(IR1_INST *pir1) {
#ifdef CONFIG_SOFTMMU
    return latxs_translate_rdtscp(pir1);
#else
    IR2_OPND ir2_eax = ra_alloc_gpr(ir1_opnd_base_reg_num(&eax_ir1_opnd));
    IR2_OPND ir2_ecx = ra_alloc_gpr(ir1_opnd_base_reg_num(&ecx_ir1_opnd));
    IR2_OPND ir2_edx = ra_alloc_gpr(ir1_opnd_base_reg_num(&edx_ir1_opnd));
    la_append_ir2_opnd2(LISA_RDTIME_D, ir2_eax, ir2_ecx);
    la_append_ir2_opnd2ii(LISA_BSTRPICK_W, ir2_edx, ir2_eax, 63, 32);
    ir2_opnd_set_em(&ir2_eax, UNKNOWN_EXTENSION, 32);
    ir2_opnd_set_em(&ir2_ecx, UNKNOWN_EXTENSION, 32);
    ir2_opnd_set_em(&ir2_edx, UNKNOWN_EXTENSION, 32);
    return true;
#endif
}

bool translate_cpuid(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cpuid(pir1);
#else
    /* 1. store registers to env */
    tr_save_registers_to_env(EAX_USEDEF_BIT, 0, 0, 0, 0x1|options_to_save());

    /* 2. call helper */
    IR2_OPND helper_addr_opnd = ra_alloc_dbt_arg2();
    IR2_OPND a0_opnd = ir2_opnd_new(IR2_OPND_IREG, 4);

    load_ireg_from_addr(helper_addr_opnd, (ADDR)helper_cpuid);
    la_append_ir2_opnd2_em(LISA_MOV64, a0_opnd, env_ir2_opnd);
    la_append_ir2_opnd2i(LISA_JIRL, ir2_opnd_new(IR2_OPND_IREG, 1),
        helper_addr_opnd, 0);

    /* 3. load registers from env */
    tr_load_registers_from_env(
        EAX_USEDEF_BIT | ECX_USEDEF_BIT | EDX_USEDEF_BIT | EBX_USEDEF_BIT, 0, 0,
        0, 0x1|options_to_save());

    return true;
#endif
}

bool translate_cdq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cdq(pir1);
#else
    IR2_OPND eax = ra_alloc_itemp();
    load_ireg_from_ir1_2(eax, &eax_ir1_opnd, SIGN_EXTENSION, false);
    IR2_OPND edx = ra_alloc_itemp();
    la_append_ir2_opnd2_em(LISA_MOV32_SX, edx, eax);
    la_append_ir2_opnd2i_em(LISA_SRAI_W, edx, edx, 31);
    store_ireg_to_ir1(edx, &edx_ir1_opnd, false);
    ra_free_temp(eax);
    ra_free_temp(edx);
    return true;
#endif
}

bool translate_sahf(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_sahf(pir1);
#else
    IR2_OPND ah = ra_alloc_itemp();
    load_ireg_from_ir1_2(ah, &ah_ir1_opnd, ZERO_EXTENSION, false);
    //la_append_ir2_opnd2i_em(LISA_ORI, ah, ah, 0x2);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, ah, 0x1f);
    ra_free_temp(ah);
    return true;
#endif
}

bool translate_lahf(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_lahf(pir1);
#else
    IR2_OPND ah = ra_alloc_itemp();
    la_append_ir2_opnd1i_em(LISA_X86MFFLAG, ah, 0x1f);
    la_append_ir2_opnd2i_em(LISA_ORI, ah, ah, 0x2);
    store_ireg_to_ir1(ah, &ah_ir1_opnd, false);
    ra_free_temp(ah);
    return true;
#endif
}

bool translate_loopnz(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_loopnz(pir1);
#else
    IR2_OPND ir2_xcx;
    if (sizeof(ADDRX) == 4) {
        ir2_xcx = ra_alloc_gpr(ir1_opnd_base_reg_num(&ecx_ir1_opnd));
        la_append_ir2_opnd2i_em(LISA_ADDI_W, ir2_xcx, ir2_xcx, -1);
    } else {
        ir2_xcx = ra_alloc_gpr(ir1_opnd_base_reg_num(&rcx_ir1_opnd));
        la_append_ir2_opnd2i_em(LISA_ADDI_D, ir2_xcx, ir2_xcx, -1);
    }
    IR2_OPND target_label = ir2_opnd_new_type(IR2_OPND_LABEL);

    IR2_OPND temp_zf = ra_alloc_itemp();

    get_eflag_condition(&temp_zf, pir1);

    //By now, if zf =0, then temp_zf =0 else temp_zf = 0xFFFFFFFFFFFFFFFF
    la_append_ir2_opnd3_em(LISA_NOR, temp_zf, temp_zf, zero_ir2_opnd);
    la_append_ir2_opnd3_em(LISA_AND, temp_zf, temp_zf, ir2_xcx);
    la_append_ir2_opnd3(LISA_BNE, temp_zf, zero_ir2_opnd, target_label);
    ra_free_temp(temp_zf);
    /* env->tr_data->curr_tb->generate_tb_linkage(0); */
    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label);
    /* env->tr_data->curr_tb->generate_tb_linkage(1); */
    tr_generate_exit_tb(pir1, 1);
    return true;
#endif
}

bool translate_loopz(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_loopz(pir1);
#else
    IR2_OPND ir2_xcx;
    if (sizeof(ADDRX) == 4) {
        ir2_xcx = ra_alloc_gpr(ir1_opnd_base_reg_num(&ecx_ir1_opnd));
        la_append_ir2_opnd2i_em(LISA_ADDI_W, ir2_xcx, ir2_xcx, -1);
    } else {
        ir2_xcx = ra_alloc_gpr(ir1_opnd_base_reg_num(&rcx_ir1_opnd));
        la_append_ir2_opnd2i_em(LISA_ADDI_D, ir2_xcx, ir2_xcx, -1);
    }
    IR2_OPND target_label = ir2_opnd_new_type(IR2_OPND_LABEL);

    IR2_OPND temp_zf = ra_alloc_itemp();

    get_eflag_condition(&temp_zf, pir1);

    //By now, if zf =0, then temp_zf =0 else temp_zf = 0xFFFFFFFFFFFFFFFF
    la_append_ir2_opnd3_em(LISA_AND, temp_zf, temp_zf, ir2_xcx);
    la_append_ir2_opnd3(LISA_BNE, temp_zf, zero_ir2_opnd, target_label);
    ra_free_temp(temp_zf);
    /* env->tr_data->curr_tb->generate_tb_linkage(0); */
    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label);
    /* env->tr_data->curr_tb->generate_tb_linkage(1); */
    tr_generate_exit_tb(pir1, 1);
    return true;
#endif
}

bool translate_loop(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_loop(pir1);
#else
    IR2_OPND ir2_xcx;
    if (sizeof(ADDRX) == 4) {
        ir2_xcx = ra_alloc_gpr(ir1_opnd_base_reg_num(&ecx_ir1_opnd));
        la_append_ir2_opnd2i_em(LISA_ADDI_W, ir2_xcx, ir2_xcx, -1);
    } else {
        ir2_xcx = ra_alloc_gpr(ir1_opnd_base_reg_num(&rcx_ir1_opnd));
        la_append_ir2_opnd2i_em(LISA_ADDI_D, ir2_xcx, ir2_xcx, -1);
    }
    IR2_OPND target_label = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd3(LISA_BNE, ir2_xcx, zero_ir2_opnd, target_label);
    /* env->tr_data->curr_tb->generate_tb_linkage(0); */
    tr_generate_exit_tb(pir1, 0);
    la_append_ir2_opnd1(LISA_LABEL, target_label);
    /* env->tr_data->curr_tb->generate_tb_linkage(1); */
    tr_generate_exit_tb(pir1, 1);
    return true;
#endif
}

bool translate_cmc(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cmc(pir1);
#else
    IR2_OPND temp = ra_alloc_itemp();
    la_append_ir2_opnd1i(LISA_X86MFFLAG, temp, 0x1);
    la_append_ir2_opnd2i(LISA_XORI, temp, temp, 0x1);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, temp, 0x1);
    ra_free_temp(temp);
    return true;
#endif
}

bool translate_cbw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cbw(pir1);
#else
    IR2_OPND value_opnd =
        load_ireg_from_ir1(&al_ir1_opnd, SIGN_EXTENSION, false);
    store_ireg_to_ir1(value_opnd, &ax_ir1_opnd, false);

    return true;
#endif
}

bool translate_cwde(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cwde(pir1);
#else
    IR2_OPND eax_opnd = ra_alloc_gpr(eax_index);
    if (!ir2_opnd_is_sx(&eax_opnd, 16)) {
        la_append_ir2_opnd2i_em(LISA_SLLI_W, eax_opnd, eax_opnd, 16);
        la_append_ir2_opnd2i_em(LISA_SRAI_W, eax_opnd, eax_opnd, 16);
    }

    return true;
#endif
}

bool translate_fnsave(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_fnsave(pir1);
#else
    printf("FIXME:fnsave/fsave didn't verify in 100, please investigate this insn first if you are in trouble\n");
    /* store x87 state information */
    IR2_OPND value = ra_alloc_itemp();
    IR2_OPND temp = ra_alloc_itemp();
    load_ireg_from_imm32(temp, 0xffff0000ULL, UNKNOWN_EXTENSION);

    /* mem_opnd is not supported in ir2 assemble */
    /* convert mem_opnd to ireg_opnd */

    IR1_OPND* opnd1 = ir1_get_opnd(pir1, 0);
    IR2_OPND mem_opnd = convert_mem_opnd_with_no_offset(opnd1);
    mem_opnd._type = IR2_OPND_IREG;
    int base_reg_num = mem_opnd._reg_num;
    int offset = mem_opnd._imm16;

    if (cpu_get_guest_base() != 0) {
        IR2_OPND gbase = ra_alloc_guest_base();
        IR2_OPND tmp = ra_alloc_itemp();
        IR2_OPND mem = ir2_opnd_new(IR2_OPND_IREG, mem_opnd._reg_num);
        if (!ir2_opnd_is_address(&mem)) {
            la_append_ir2_opnd2_em(LISA_MOV_ADDRX, mem, mem);
        }
        la_append_ir2_opnd3_em(LISA_ADD_ADDR, tmp, mem, gbase);
        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
        base_reg_num = mem_opnd._reg_num = ir2_opnd_base_reg_num(&tmp);
    }

    la_append_ir2_opnd2i_em(LISA_LD_H, value, env_ir2_opnd,
                      lsenv_offset_of_control_word(lsenv)); /* control_word */
    la_append_ir2_opnd3_em(LISA_OR, value, temp, value);
    la_append_ir2_opnd2i(LISA_ST_W, value, mem_opnd, offset);

    lsassert(offset + 108 <= 2047);

    update_sw_by_fcsr(value);
    la_append_ir2_opnd3_em(LISA_OR, value, temp, value);
    la_append_ir2_opnd2i(LISA_ST_W, value, mem_opnd, offset + 4);

    la_append_ir2_opnd2i_em(LISA_LD_H, value, env_ir2_opnd,
                      lsenv_offset_of_tag_word(lsenv)); /* tag_word */

    la_append_ir2_opnd3_em(LISA_OR, value, temp, value);
    /* dispose tag word */
    IR2_OPND temp_1 = ra_alloc_itemp();
    load_ireg_from_imm32(temp_1, 0xffff0000ULL, UNKNOWN_EXTENSION);
    la_append_ir2_opnd3_em(LISA_AND, value, value, temp_1);
    ra_free_temp(temp_1);
    la_append_ir2_opnd2i(LISA_ST_W, value, mem_opnd, offset + 8);
    ra_free_temp(value);

    la_append_ir2_opnd2i(LISA_ST_W, temp, mem_opnd, offset + 24);
    ra_free_temp(temp);

    /* store x87 registers stack */
    int i;
    for (i = 0; i <= 7; i++) {
        IR2_OPND st = ra_alloc_st(i);
        store_64_bit_freg_to_ir1_80_bit_mem(
            st,
            ir2_opnd_new2(IR2_OPND_MEM, base_reg_num, offset + 28 + 10 * i));
    }

    /* reset SR and CR */
    translate_fninit(pir1);
    return true;
#endif
}

bool translate_fsave(IR1_INST *pir1)
{
    /*
    * FIXME: checking for pending unmasked exceptions?
    */
    return translate_fnsave(pir1);
}

bool translate_frstor(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_frstor(pir1);
#else
    /*
    * Loads the FPU state (operating environment and register stack)
    * from the memory area specified with the source
    * operand. This state data is typically written to the specified
    * memory location by a previous FSAVE/FNSAVE instruction.
    * FPUControlWord ← SRC[FPUControlWord];
    * FPUStatusWord ← SRC[FPUStatusWord];
    * FPUTagWord ← SRC[FPUTagWord];
    * FPUDataPointer ← SRC[FPUDataPointer];
    * FPUInstructionPointer ← SRC[FPUInstructionPointer];
    * FPULastInstructionOpcode ← SRC[FPULastInstructionOpcode];
    * ST(0) ← SRC[ST(0)];
    * ST(1) ← SRC[ST(1)];
    * ST(2) ← SRC[ST(2)];
    * ST(3) ← SRC[ST(3)];
    * ST(4) ← SRC[ST(4)];
    * ST(5) ← SRC[ST(5)];
    * ST(6) ← SRC[ST(6)];
    * ST(7) ← SRC[ST(7)];
     */
    printf("FIXME:frstor didn't verify in 100, please investigate this insn first if you are in trouble\n");
    IR2_OPND value = ra_alloc_itemp();
    IR2_OPND temp = ra_alloc_itemp();
    load_ireg_from_imm32(temp, 0xffff0000ULL, UNKNOWN_EXTENSION);

    /* mem_opnd is not supported in ir2 assemble */
    /* convert mem_opnd to ireg_opnd */

    IR1_OPND* opnd1 = ir1_get_opnd(pir1, 0);
    IR2_OPND mem_opnd = convert_mem_opnd_with_no_offset(opnd1);
    mem_opnd._type = IR2_OPND_IREG;
    int offset = mem_opnd._imm16;

    if (cpu_get_guest_base() != 0) {
        IR2_OPND gbase = ra_alloc_guest_base();
        IR2_OPND tmp = ra_alloc_itemp();
        IR2_OPND mem = ir2_opnd_new(IR2_OPND_IREG, mem_opnd._reg_num);
        if (!ir2_opnd_is_address(&mem)) {
            la_append_ir2_opnd2_em(LISA_MOV_ADDRX, mem, mem);
          }
        la_append_ir2_opnd3_em(LISA_ADD_ADDR, tmp, mem, gbase);
        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
     }

    lsassert(offset + 108 <= 2047);

    la_append_ir2_opnd2i_em(LISA_LD_W, value, mem_opnd, offset);
    la_append_ir2_opnd3_em(LISA_AND, value, n1_ir2_opnd, value);
    la_append_ir2_opnd2i(LISA_ST_H, value, env_ir2_opnd,
                      lsenv_offset_of_control_word(lsenv)); /* control_word */

    la_append_ir2_opnd2i_em(LISA_LD_W, value, mem_opnd, offset + 4);
    la_append_ir2_opnd3_em(LISA_AND, value, n1_ir2_opnd, value);
    la_append_ir2_opnd2i(LISA_ST_H, value, env_ir2_opnd,
                      lsenv_offset_of_status_word(lsenv)); /* status_word */

    /* tag word */
    la_append_ir2_opnd2i_em(LISA_LD_D, value, mem_opnd, offset + 8);
    la_append_ir2_opnd2i(LISA_ST_D, value, env_ir2_opnd,
                      lsenv_offset_of_tag_word(lsenv));

    for (int i = 0; i <= 7; i++) {
        IR2_OPND st = ra_alloc_st(i);
        load_64_bit_freg_from_ir1_80_bit_mem(
            st, mem_opnd, offset + 28 + 10 * i);
    }
    return true;
#endif
}

bool translate_prefetcht0(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_prefetcht0(pir1);
#else
    IR2_OPND mem = convert_mem_opnd(ir1_get_opnd(pir1, 0));
    mem._type = IR2_OPND_IREG;
    /* if the value of zero_imm_opnd is 0, it is load.
     * if the value of zero_imm_opnd is 8, it is store.
     * if the value of zero_imm_opnd is others, it is invalid.
     */
    int imm = 0;
    IR2_OPND zero_imm_opnd = create_ir2_opnd(IR2_OPND_IMMH, imm);

    la_append_ir2_opnd2i(LISA_PRELD, zero_imm_opnd , mem, mem._imm16);
    return true;
#endif
}

bool translate_prefetchw(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_prefetchw(pir1);
#else
    IR2_OPND mem = convert_mem_opnd(ir1_get_opnd(pir1, 0));
    la_append_ir2_opnd1i(LISA_PRELDX, mem, 0);
    return true;
#endif
}

bool translate_emms(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_emms(pir1);
#else
    // TODO: do something?
    return true;
#endif
}

bool translate_jmp_far(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_jmp_far(pir1);
#else
    return false;
#endif
}

bool translate_cli(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_cli(pir1);
#else
    return false;
#endif
}

bool translate_sti(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_sti(pir1);
#else
    return false;
#endif
}

bool translate_in(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_in(pir1);
#else
    return false;
#endif
}

bool translate_out(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    return latxs_translate_out(pir1);
#else
    return false;
#endif
}

#ifndef CONFIG_SOFTMMU

bool translate_lidt(IR1_INST *pir1) { return false; }
bool translate_sidt(IR1_INST *pir1) { return false; }
bool translate_lgdt(IR1_INST *pir1) { return false; }
bool translate_sgdt(IR1_INST *pir1) { return false; }
bool translate_lldt(IR1_INST *pir1) { return false; }
bool translate_sldt(IR1_INST *pir1) { return false; }
bool translate_ltr(IR1_INST *pir1) { return false; }
bool translate_str(IR1_INST *pir1) { return false; }

bool translate_outs(IR1_INST *pir1) { return false; }
bool translate_rsm(IR1_INST *pir1) { return false; }
bool translate_wbinvd(IR1_INST *pir1) { return false; }
bool translate_invd(IR1_INST *pir1) { return false; }
bool translate_nop(IR1_INST *pir1) { return true; }
bool translate_int1(IR1_INST *pir1) { return false; }
bool translate_int_3(IR1_INST *pir1) { return false; }
bool translate_into(IR1_INST *pir1) { return false; }
bool translate_retf(IR1_INST *pir1) { return false; }
bool translate_cwd(IR1_INST *pir1) { return false; }
bool translate_call_far(IR1_INST *pir1) { return false; }
bool translate_clac(IR1_INST *pir1) { return false; }
bool translate_stac(IR1_INST *pir1) { return false; }
bool translate_xgetbv(IR1_INST *pir1) { return false; }
bool translate_xsetbv(IR1_INST *pir1) { return false; }
bool translate_xsave(IR1_INST *pir1) { return false; }
bool translate_xrstor(IR1_INST *pir1) { return false; }
bool translate_xsaveopt(IR1_INST *pir1) { return false; }

bool translate_lds(IR1_INST *pir1) { return false; }
bool translate_les(IR1_INST *pir1) { return false; }
bool translate_lfs(IR1_INST *pir1) { return false; }
bool translate_lgs(IR1_INST *pir1) { return false; }
bool translate_lss(IR1_INST *pir1) { return false; }
bool translate_clts(IR1_INST *pir1) { return false; }
bool translate_enter(IR1_INST *pir1) { return false; }
bool translate_wrmsr(IR1_INST *pir1) { return false; }
bool translate_rdmsr(IR1_INST *pir1) { return false; }
bool translate_rdpmc(IR1_INST *pir1) { return false; }
bool translate_invlpg(IR1_INST *pir1) { return false; }
bool translate_invlpga(IR1_INST *pir1) { return false; }
bool translate_sysenter(IR1_INST *pir1) { return false; }
bool translate_sysexit(IR1_INST *pir1) { return false; }
bool translate_prefetchnta(IR1_INST *pir1) { return true; }
bool translate_prefetcht1(IR1_INST *pir1) { return true; }
bool translate_prefetcht2(IR1_INST *pir1) { return true; }
bool translate_prefetch(IR1_INST *pir1) { return true; }

bool translate_invalid(IR1_INST *pir1) { return false; }
bool translate_ud0(IR1_INST *pir1) { return false; }

bool translate_cvttps2pi(IR1_INST *pir1) { return false; }
bool translate_cvttpd2pi(IR1_INST *pir1) { return false; }

bool translate_xlat(IR1_INST *pir1) { return false; }

#else

bool translate_lidt(IR1_INST *pir1) { return latxs_translate_lidt(pir1); }
bool translate_sidt(IR1_INST *pir1) { return latxs_translate_sidt(pir1); }
bool translate_lgdt(IR1_INST *pir1) { return latxs_translate_lgdt(pir1); }
bool translate_sgdt(IR1_INST *pir1) { return latxs_translate_sgdt(pir1); }
bool translate_lldt(IR1_INST *pir1) { return latxs_translate_lldt(pir1); }
bool translate_sldt(IR1_INST *pir1) { return latxs_translate_sldt(pir1); }
bool translate_ltr(IR1_INST *pir1) { return latxs_translate_ltr(pir1); }
bool translate_str(IR1_INST *pir1) { return latxs_translate_str(pir1); }

bool translate_outs(IR1_INST *pir1) { return latxs_translate_outs(pir1); }
bool translate_rsm(IR1_INST *pir1) { return latxs_translate_rsm(pir1); }
bool translate_wbinvd(IR1_INST *pir1) { return latxs_translate_wbinvd(pir1); }
bool translate_invd(IR1_INST *pir1) { return latxs_translate_invd(pir1); }
bool translate_nop(IR1_INST *pir1) { return latxs_translate_nop(pir1); }
bool translate_int1(IR1_INST *pir1) { return latxs_translate_int1(pir1); }
bool translate_int_3(IR1_INST *pir1) { return latxs_translate_int_3(pir1); }
bool translate_into(IR1_INST *pir1) { return latxs_translate_into(pir1); }
bool translate_retf(IR1_INST *pir1) { return latxs_translate_retf(pir1); }
bool translate_cwd(IR1_INST *pir1) { return latxs_translate_cwd(pir1); }
bool translate_call_far(IR1_INST *pir1) { return latxs_translate_lcall(pir1); }
bool translate_clac(IR1_INST *pir1) { return latxs_translate_clac(pir1); }
bool translate_stac(IR1_INST *pir1) { return latxs_translate_stac(pir1); }
bool translate_xgetbv(IR1_INST *pir1) { return latxs_translate_xgetbv(pir1); }
bool translate_xsetbv(IR1_INST *pir1) { return latxs_translate_xsetbv(pir1); }
bool translate_xsave(IR1_INST *pir1) { return latxs_translate_xsave(pir1); }
bool translate_xrstor(IR1_INST *pir1) { return latxs_translate_xrstor(pir1); }
bool translate_xsaveopt(IR1_INST *pir1)
{
    return latxs_translate_xsaveopt(pir1);
}
bool translate_xlat(IR1_INST *pir1) { return latxs_translate_xlat(pir1); }

bool translate_lds(IR1_INST *pir1) { return latxs_translate_lds(pir1); }
bool translate_les(IR1_INST *pir1) { return latxs_translate_les(pir1); }
bool translate_lfs(IR1_INST *pir1) { return latxs_translate_lfs(pir1); }
bool translate_lgs(IR1_INST *pir1) { return latxs_translate_lgs(pir1); }
bool translate_lss(IR1_INST *pir1) { return latxs_translate_lss(pir1); }
bool translate_clts(IR1_INST *pir1) { return latxs_translate_clts(pir1); }
bool translate_enter(IR1_INST *pir1) { return latxs_translate_enter(pir1); }
bool translate_wrmsr(IR1_INST *pir1) { return latxs_translate_wrmsr(pir1); }
bool translate_rdmsr(IR1_INST *pir1) { return latxs_translate_rdmsr(pir1); }
bool translate_rdpmc(IR1_INST *pir1) { return latxs_translate_rdpmc(pir1); }
bool translate_invlpg(IR1_INST *pir1) { return latxs_translate_invlpg(pir1); }
bool translate_invlpga(IR1_INST *pir1) { return latxs_translate_invlpga(pir1); }
bool translate_sysenter(IR1_INST *pir1)
{
    return latxs_translate_sysenter(pir1);
}
bool translate_sysexit(IR1_INST *pir1)
{
    return latxs_translate_sysexit(pir1);
}
bool translate_prefetchnta(IR1_INST *pir1)
{
    return latxs_translate_prefetchnta(pir1);
}
bool translate_prefetcht1(IR1_INST *pir1)
{
    return latxs_translate_prefetcht1(pir1);
}
bool translate_prefetcht2(IR1_INST *pir1)
{
    return latxs_translate_prefetcht2(pir1);
}
bool translate_prefetch(IR1_INST *pir1)
{
    return latxs_translate_prefetch(pir1);
}

bool translate_invalid(IR1_INST *pir1)
{
    return latxs_translate_invalid(pir1);
}
bool translate_ud0(IR1_INST *pir1)
{
    return latxs_translate_ud0(pir1);
}

bool translate_cvttps2pi(IR1_INST *pir1)
{
    return latxs_translate_cvttps2pi(pir1);
}
bool translate_cvttpd2pi(IR1_INST *pir1)
{
    return latxs_translate_cvttpd2pi(pir1);
}

#endif
