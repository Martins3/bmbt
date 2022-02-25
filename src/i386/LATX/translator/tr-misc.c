#include "../include/common.h"
#include "../include/reg-alloc.h"
#include "../x86tomips-options.h"
#include "../include/env.h"
#include "../x86tomips-options.h"
#include "../include/flag-lbt.h"
#include "../include/error.h"

// IF (CF=1), AL=FF, ELSE, AL=0 (#UD in 64-bit mode)
bool translate_salc(IR1_INST *pir1)
{
    lsassertm(0, "MISC salc to be implemented in LoongArch.\n");
//    IR2_OPND cf_opnd = ra_alloc_itemp();
//    IR2_OPND eax_opnd = ra_alloc_gpr(eax_index);
//    IR2_OPND lable_cf_neq_zero = ir2_opnd_new_type(IR2_OPND_LABEL);
//    IR2_OPND lable_end_of_salc = ir2_opnd_new_type(IR2_OPND_LABEL);
//
//    // get CF from eflags
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mfflag, &cf_opnd, 0x1);
//    } else {
//        append_ir2_opnd2i(mips_andi, &cf_opnd, &eflags_ir2_opnd, CF_BIT);
//    }
//    // if CF=0 then AL=0
//    append_ir2_opnd3(mips_bne, &cf_opnd, &zero_ir2_opnd, &lable_cf_neq_zero);
//    append_ir2_opnd2i(mips_andi, &eax_opnd, &eax_opnd, 0); // imm of mip_andi is 16-bit, exactly the size of AL
//    append_ir2_opnd1(mips_b, &lable_end_of_salc);
//    // else (CF=1) AL=0xff
//    append_ir2_opnd1(mips_label, &lable_cf_neq_zero);
//    append_ir2_opnd2i(mips_ori, &eax_opnd, &eax_opnd, 0xff);
//    // end if-else
//    append_ir2_opnd1(mips_label, &lable_end_of_salc);
    return true;
}

// Set AL to memory byte DS:[(E)BX + unsigned AL].
bool translate_xlat(IR1_INST *pir1)
{
    lsassertm(0, "MISC xlat to be implemented in LoongArch.\n");
//    // DS + BX & 0xffff + AL & 0xff != DS + (BX + AL & 0xff)&0xffff
//    IR2_OPND addr = ra_alloc_itemp();
//    IR2_OPND tmp = ra_alloc_itemp();
//    append_ir2_opnd2i(
//        mips_load_addrx, &addr, &env_ir2_opnd,
//        lsenv_offset_of_seg_base(
//            lsenv, !ir1_get_segment_override_prefix(pir1)
//                       ? ds_index
//                       : ir1_segment_override_prefix_to_index(
//                             ir1_get_segment_override_prefix(pir1))));
//
//    IR2_OPND gpr = ra_alloc_gpr(ebx_index);
//    if (ir1_addr_size(pir1) == 2) {
//        append_ir2_opnd2i(mips_andi, &tmp, &gpr, 0xffff);
//    } else if (ir1_addr_size(pir1) == 4) {
//        append_ir2_opnd3(mips_and, &tmp, &gpr,
//                         &n1_ir2_opnd);
//    } else {
//        lsassert(0);
//    }
//    append_ir2_opnd3(mips_daddu, &addr, &addr, &tmp);
//    gpr = ra_alloc_gpr(eax_index);
//    append_ir2_opnd2i(mips_andi, &tmp, &gpr, 0xff);
//    append_ir2_opnd3(mips_daddu, &addr, &addr, &tmp);
//    append_ir2_opnd3(mips_and, &addr, &addr, &n1_ir2_opnd);
//
//#ifdef CONFIG_SOFTMMU
//    IR2_OPND mem_opnd;
//    ir2_opnd_build2(&mem_opnd, IR2_OPND_MEM, ir2_opnd_base_reg_num(&addr), 0);
//    gen_ldst_softmmu_helper(mips_lbu, &tmp, &mem_opnd, false);
//#else
//    lsassertm(0, "unimplemented xlat in user-mode.\n");
//#endif
//    store_ir2_to_ir1(&tmp, &al_ir1_opnd, false);
//    ra_free_temp(&tmp);
//    ra_free_temp(&addr);
    return true;
}

bool translate_syscall(IR1_INST *pir1)
{
#ifndef CONFIG_SOFTMMU
    lsassertm(0, "i386 not include SYSCALL inst. SYSCALL only validation in x86-64.\n");
#else
    lsassertm_illop(ir1_addr(pir1), 0,
            "i386 not include SYSCALL inst. SYSCALL only validation in x86-64.\n");
#endif
    return true;
}

bool translate_sysret(IR1_INST *pir1)
{
#ifndef CONFIG_SOFTMMU
    lsassertm(0, "i386 not include SYSRET inst. SYSRET only validation in x86-64.\n");
#else
    lsassertm_illop(ir1_addr(pir1), 0,
            "i386 not include SYSRET inst. SYSRET only validation in x86-64.\n");
#endif
    return true;
}

bool translate_femms(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    /* check CPUID_EXT2_3DNOW */
    if (tr_gen_excp_check(pir1)) {
        return true;
    }
#endif
    return translate_emms(pir1);
}

// SMX: Safe Moe Extension not support by i386
bool translate_getsec(IR1_INST *pir1)
{
#ifndef CONFIG_SOFTMMU
    lsassertm(0, "i386 not include GETSET inst. GETSEC only validation in x86-64.\n");
#else
    lsassertm_illop(ir1_addr(pir1), 0,
            "i386 not include GETSET inst. GETSEC only validation in x86-64.\n");
#endif
    return true;
}
// End of SMX

// VMX: Virtual Machine Extension not support by i386
#ifndef CONFIG_SOFTMMU

#define VMX_NOT_SUPPORT_BY_I386 \
    lsassertm(0, "i386 not include GETSET inst. GETSEC only validation in x86-64.\n"); \
    return true;

#else

#define VMX_NOT_SUPPORT_BY_I386 \
    lsassertm_illop(ir1_addr(pir1), 0, \
        "i386 not include GETSET inst. GETSEC only validation in x86-64.\n"); \
    return true;

#endif
bool translate_vmptrld(IR1_INST *pir1) { VMX_NOT_SUPPORT_BY_I386 }
bool translate_vmptrst(IR1_INST *pir1) { VMX_NOT_SUPPORT_BY_I386 }
bool translate_vmcall(IR1_INST *pir1) { VMX_NOT_SUPPORT_BY_I386 }
bool translate_vmlaunch(IR1_INST *pir1) { VMX_NOT_SUPPORT_BY_I386 }
bool translate_vmresume(IR1_INST *pir1) { VMX_NOT_SUPPORT_BY_I386 }
bool translate_vmxoff(IR1_INST *pir1) { VMX_NOT_SUPPORT_BY_I386 }
bool translate_vmclear(IR1_INST *pir1) { VMX_NOT_SUPPORT_BY_I386 }
bool translate_vmxon(IR1_INST *pir1) { VMX_NOT_SUPPORT_BY_I386 }
bool translate_vmread(IR1_INST *pir1) { VMX_NOT_SUPPORT_BY_I386 }
bool translate_vmwrite(IR1_INST *pir1) { VMX_NOT_SUPPORT_BY_I386 }
bool translate_invept(IR1_INST *pir1) { VMX_NOT_SUPPORT_BY_I386 }
bool translate_invvpid(IR1_INST *pir1) { VMX_NOT_SUPPORT_BY_I386 }
// tr_misc.c not include vmfunc
#undef VMX_NOT_SUPPORT_BY_I386
// End of VMX

bool translate_swapgs(IR1_INST *pir1) { return false; } // only support in x86-64
bool translate_prefetch(IR1_INST *pir1) { return true; }
bool translate_prefetchw(IR1_INST *pir1) { return true; }
//// 128-bit and 256-bit media extension currently is partially
//// supported by QEMU (current newest version: 5.0.0-rc4),
//// "partially" refers to those instruction start with "v",
////========= Start of not supported instructions ==========////
bool translate_vmovss(IR1_INST *pir1) { return false; }
bool translate_vmovsd(IR1_INST *pir1) { return false; }
bool translate_vmovups(IR1_INST *pir1) { return false; }
bool translate_vmovupd(IR1_INST *pir1) { return false; }
bool translate_vmovhlps(IR1_INST *pir1) { return false; }
bool translate_vmovlps(IR1_INST *pir1) { return false; }
bool translate_vmovlpd(IR1_INST *pir1) { return false; }
bool translate_vmovsldup(IR1_INST *pir1) { return false; }
bool translate_vmovddup(IR1_INST *pir1) { return false; }
bool translate_vunpcklps(IR1_INST *pir1) { return false; }
bool translate_vunpcklpd(IR1_INST *pir1) { return false; }
bool translate_vunpckhps(IR1_INST *pir1) { return false; }
bool translate_vunpckhpd(IR1_INST *pir1) { return false; }
bool translate_vmovlhps(IR1_INST *pir1) { return false; }
bool translate_vmovhps(IR1_INST *pir1) { return false; }
bool translate_vmovhpd(IR1_INST *pir1) { return false; }
bool translate_vmovshdup(IR1_INST *pir1) { return false; }
bool translate_vmovaps(IR1_INST *pir1) { return false; }
bool translate_vmovapd(IR1_INST *pir1) { return false; }
bool translate_vcvtsi2ss(IR1_INST *pir1) { return false; }
bool translate_vcvtsi2sd(IR1_INST *pir1) { return false; }
bool translate_vmovntps(IR1_INST *pir1) { return false; }
bool translate_vmovntpd(IR1_INST *pir1) { return false; }
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
bool translate_vzeroupper(IR1_INST *pir1) { return false; }
bool translate_vzeroall(IR1_INST *pir1) { return false; }
bool translate_vhaddpd(IR1_INST *pir1) { return false; }
bool translate_vhaddps(IR1_INST *pir1) { return false; }
bool translate_vhsubpd(IR1_INST *pir1) { return false; }
bool translate_vhsubps(IR1_INST *pir1) { return false; }
bool translate_xave(IR1_INST *pir1) { return false; }
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
bool translate_maskmovq(IR1_INST *pir1) { return false; }
bool translate_vmaskmovdqu(IR1_INST *pir1) { return false; }
bool translate_vpsubb(IR1_INST *pir1) { return false; }
bool translate_vpsubw(IR1_INST *pir1) { return false; }
bool translate_vpsubd(IR1_INST *pir1) { return false; }
bool translate_vpsubq(IR1_INST *pir1) { return false; }
bool translate_vpaddb(IR1_INST *pir1) { return false; }
bool translate_vpaddw(IR1_INST *pir1) { return false; }
bool translate_vpaddd(IR1_INST *pir1) { return false; }
bool translate_vpshufb(IR1_INST *pir1) { return false; }
bool translate_vphaddw(IR1_INST *pir1) { return false; }
bool translate_vphaddd(IR1_INST *pir1) { return false; }
bool translate_vphaddsw(IR1_INST *pir1) { return false; }
bool translate_vpmaddubsw(IR1_INST *pir1) { return false; }
bool translate_vphsubw(IR1_INST *pir1) { return false; }
bool translate_vphsubd(IR1_INST *pir1) { return false; }
bool translate_vphsubsw(IR1_INST *pir1) { return false; }
bool translate_vpsignb(IR1_INST *pir1) { return false; }
bool translate_vpsignw(IR1_INST *pir1) { return false; }
bool translate_vpsignd(IR1_INST *pir1) { return false; }
bool translate_vpmulhrsw(IR1_INST *pir1) { return false; }
bool translate_vpermilps(IR1_INST *pir1) { return false; }
bool translate_vpermilpd(IR1_INST *pir1) { return false; }
bool translate_vptestps(IR1_INST *pir1) { return false; }
bool translate_vptestpd(IR1_INST *pir1) { return false; }
bool translate_vptest(IR1_INST *pir1) { return false; }
bool translate_vbroadcastss(IR1_INST *pir1) { return false; }
bool translate_vbroadcastsd(IR1_INST *pir1) { return false; }
bool translate_vbroadcastf128(IR1_INST *pir1) { return false; }
bool translate_vpabsb(IR1_INST *pir1) { return false; }
bool translate_vpabsw(IR1_INST *pir1) { return false; }
bool translate_vpabsd(IR1_INST *pir1) { return false; }
bool translate_vpmovsxbw(IR1_INST *pir1) { return false; }
bool translate_vpmovsxbd(IR1_INST *pir1) { return false; }
bool translate_vpmovsxbq(IR1_INST *pir1) { return false; }
bool translate_vpmovsxwd(IR1_INST *pir1) { return false; }
bool translate_vpmovsxwq(IR1_INST *pir1) { return false; }
bool translate_vpmovsxdq(IR1_INST *pir1) { return false; }
bool translate_vpmuldq(IR1_INST *pir1) { return false; }
bool translate_vpcmpeqq(IR1_INST *pir1) { return false; }
bool translate_movntdqa(IR1_INST *pir1) { return false; }
bool translate_vmovntdqa(IR1_INST *pir1) { return false; }
bool translate_vpackusdw(IR1_INST *pir1) { return false; }
bool translate_vmaskmovps(IR1_INST *pir1) { return false; }
bool translate_vmaskmovpd(IR1_INST *pir1) { return false; }
bool translate_vpmovzxbw(IR1_INST *pir1) { return false; }
bool translate_vpmovzxbd(IR1_INST *pir1) { return false; }
bool translate_vpmovzxbq(IR1_INST *pir1) { return false; }
bool translate_vpmovzxwd(IR1_INST *pir1) { return false; }
bool translate_vpmovzxwq(IR1_INST *pir1) { return false; }
bool translate_vpmovzxdq(IR1_INST *pir1) { return false; }
bool translate_vpcmpgtq(IR1_INST *pir1) { return false; }
bool translate_vpminsb(IR1_INST *pir1) { return false; }
bool translate_vpminsd(IR1_INST *pir1) { return false; }
bool translate_vpminuw(IR1_INST *pir1) { return false; }
bool translate_vpminud(IR1_INST *pir1) { return false; }
bool translate_vpmaxsb(IR1_INST *pir1) { return false; }
bool translate_vpmaxsd(IR1_INST *pir1) { return false; }
bool translate_vpmaxuw(IR1_INST *pir1) { return false; }
bool translate_vpmaxud(IR1_INST *pir1) { return false; }
bool translate_vpmulld(IR1_INST *pir1) { return false; }
bool translate_vphminposuw(IR1_INST *pir1) { return false; }
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
bool translate_vaesimc(IR1_INST *pir1) { return false; }
bool translate_vaesenc(IR1_INST *pir1) { return false; }
bool translate_vaesenclast(IR1_INST *pir1) { return false; }
bool translate_vaesdec(IR1_INST *pir1) { return false; }
bool translate_vaesdeclast(IR1_INST *pir1) { return false; }
bool translate_vperm2f128(IR1_INST *pir1) { return false; }
bool translate_vroundps(IR1_INST *pir1) { return false; }
bool translate_vroundpd(IR1_INST *pir1) { return false; }
bool translate_vroundss(IR1_INST *pir1) { return false; }
bool translate_vroundsd(IR1_INST *pir1) { return false; }
bool translate_vblendps(IR1_INST *pir1) { return false; }
bool translate_vblendpd(IR1_INST *pir1) { return false; }
bool translate_vpblendvw(IR1_INST *pir1) { return false; }
bool translate_vpalignr(IR1_INST *pir1) { return false; }
bool translate_vpextrb(IR1_INST *pir1) { return false; }
bool translate_vpextrd(IR1_INST *pir1) { return false; }
bool translate_vextractps(IR1_INST *pir1) { return false; }
bool translate_vinsertf128(IR1_INST *pir1) { return false; }
bool translate_vextractf128(IR1_INST *pir1) { return false; }
bool translate_vpinsrb(IR1_INST *pir1) { return false; }
bool translate_vinsertps(IR1_INST *pir1) { return false; }
bool translate_vpinsrd(IR1_INST *pir1) { return false; }
bool translate_vpinsrq(IR1_INST *pir1) { return false; }
bool translate_vdpps(IR1_INST *pir1) { return false; }
bool translate_vdppd(IR1_INST *pir1) { return false; }
bool translate_vmpsadbw(IR1_INST *pir1) { return false; }
bool translate_vpclmulqdq(IR1_INST *pir1) { return false; }
bool translate_vblendvps(IR1_INST *pir1) { return false; }
bool translate_vblendvpd(IR1_INST *pir1) { return false; }
bool translate_vpblendvb(IR1_INST *pir1) { return false; }
bool translate_vpcmpestrm(IR1_INST *pir1) { return false; }
bool translate_vcmpestri(IR1_INST *pir1) { return false; }
bool translate_vpcmpistrm(IR1_INST *pir1) { return false; }
bool translate_vpcmpistri(IR1_INST *pir1) { return false; }
bool translate_vaeskeygenassist(IR1_INST *pir1) { return false; }
bool translate_vpsrldq(IR1_INST *pir1) { return false; }
bool translate_vpslldq(IR1_INST *pir1) { return false; }
bool translate_vldmxcsr(IR1_INST *pir1) { return false; }
bool translate_vstmxcsr(IR1_INST *pir1) { return false; }
// extrq and insertq are partially support by qemu only imm8 version
bool translate_extrq(IR1_INST *pir1) { return false; }
bool translate_insertq(IR1_INST *pir1) { return false; }
////========= End of not supported instructions ==========////
// sl: Single-precision floating-point value in Lower 32 bit
// dest[127:0] = src[95:64] || src[95:64] || src[31:0] || src[31:0]
bool translate_movsldup(IR1_INST *pir1)
{
    lsassertm(0, "MISC movsldup to be implemented in LoongArch.\n");
//    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
//    IR1_OPND *src = ir1_get_opnd(pir1, 1);
//
//    IR2_OPND sl0 = ra_alloc_itemp();
//    IR2_OPND sl1 = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&sl0, src, ZERO_EXTENSION, 0); // sl0[63:0] = src[63:0]
//    load_ir1_to_ir2(&sl1, src, ZERO_EXTENSION, 1); // sl1[63:0] = src[127:64]
//
//    IR2_OPND temp0 = ra_alloc_itemp();
//    IR2_OPND temp1 = ra_alloc_itemp();
//
//    append_ir2_opnd2i(mips_dsll32, &temp0, &sl0, 0); // temp0[63:0] = src[31: 0] || 0[31:0]
//    append_ir2_opnd2i(mips_dsll32, &temp1, &sl1, 0); // temp1[63:0] = src[95:64] || 0[31:0]
//
//    append_ir2_opnd2i(mips_dsrl32, &sl0, &temp0, 0); // sl0[63:0] = 0[31:0] || src[31: 0]
//    append_ir2_opnd2i(mips_dsrl32, &sl1, &temp1, 0); // sl1[63:0] = 0[31:0] || src[95:64]
//
//    append_ir2_opnd3(mips_or, &sl0, &sl0, &temp0); // sl0 = src[31: 0] || src[31: 0]
//    append_ir2_opnd3(mips_or, &sl1, &sl1, &temp1); // sl1 = src[95:64] || src[95:64]
//
//    store_ir2_to_ir1(&sl0, dest, 0); // dest[63: 0] = src[31: 0] || src[31: 0]
//    store_ir2_to_ir1(&sl1, dest, 1); // dest[127:0] = src[95:64] || src[95:64]

    return true;
}
// d: Double-precision floating point (64-bit)
// dest[127:0] = src[63:0] || src[63:0]
bool translate_movddup(IR1_INST *pir1)
{
    lsassertm(0, "MISC movddup to be implemented in LoongArch.\n");
//    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
//    IR1_OPND *src = ir1_get_opnd(pir1, 1);
//
//    IR2_OPND d = ra_alloc_itemp();
//    load_ir1_to_ir2(&d, src, ZERO_EXTENSION, 0); // d[63:0] = src[63:0]
//
//    store_ir2_to_ir1(&d, dest, 0); // dest[63 : 0] = src[63:0]
//    store_ir2_to_ir1(&d, dest, 1); // dest[127:64] = src[63:0]
//
    return true;
}
// sh: Single-precision floating-point value in Higher 32 bit
// dest[127:0] = src[127:96] || src[127:96] || src[63:32] || src[63:32]
bool translate_movshdup(IR1_INST *pir1)
{
    lsassertm(0, "MISC movshdup to be implemented in LoongArch.\n");
//    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
//    IR1_OPND *src = ir1_get_opnd(pir1, 1);
//
//    IR2_OPND sl0 = ra_alloc_itemp();
//    IR2_OPND sl1 = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&sl0, src, ZERO_EXTENSION, 0); // sl0[63:0] = src[63:0]
//    load_ir1_to_ir2(&sl1, src, ZERO_EXTENSION, 1); // sl1[63:0] = src[127:64]
//
//    IR2_OPND temp0 = ra_alloc_itemp();
//    IR2_OPND temp1 = ra_alloc_itemp();
//
//    append_ir2_opnd2i(mips_dsrl32, &temp0, &sl0, 0); // temp0[63:0] = 0[31:0] || src[63 :32]
//    append_ir2_opnd2i(mips_dsrl32, &temp1, &sl1, 0); // temp1[63:0] = 0[31:0] || src[127:96]
//
//    append_ir2_opnd2i(mips_dsll32, &sl0, &temp0, 0); // sl0[63:0] = src[63 :32] || 0[31:0]
//    append_ir2_opnd2i(mips_dsll32, &sl1, &temp1, 0); // sl1[63:0] = src[127:96] || 0[31:0]
//
//    append_ir2_opnd3(mips_or, &sl0, &sl0, &temp0); // sl0 = src[63 :32] || src[63 :32]
//    append_ir2_opnd3(mips_or, &sl1, &sl1, &temp1); // sl1 = src[127:96] || src[127:96]
//
//    store_ir2_to_ir1(&sl0, dest, 0); // dest[63: 0] = src[63 :32] || src[63 :32]
//    store_ir2_to_ir1(&sl1, dest, 1); // dest[127:0] = src[127:96] || src[127:96]

    return true;
}
// mem[31:0] = xmm[31:0]
bool translate_movntss(IR1_INST *pir1)
{
    lsassertm(0, "MISC movntss to be implemented in LoongArch.\n");
//    IR1_OPND *mem = ir1_get_opnd(pir1, 0); // size = 4 bytes
//    IR1_OPND *xmm = ir1_get_opnd(pir1, 1);
//
//    lsassert(ir1_opnd_is_mem(mem));
//
//    IR2_OPND temp = ra_alloc_itemp();
//    load_ir1_to_ir2(&temp, xmm, ZERO_EXTENSION, 0); // temp[63;0] = xmm[63:0]
//
//    store_ir2_to_ir1(&temp, mem, 0);
//
    return true;
}
// mem[63:0] = xmm[63:0]
bool translate_movntsd(IR1_INST *pir1)
{
    lsassertm(0, "MISC movntsd to be implemented in LoongArch.\n");
//    IR1_OPND *mem = ir1_get_opnd(pir1, 0); // size = 8 bytes
//    IR1_OPND *xmm = ir1_get_opnd(pir1, 1);
//
//    lsassert(ir1_opnd_is_mem(mem));
//
//    IR2_OPND temp = ra_alloc_itemp();
//    load_ir1_to_ir2(&temp, xmm, ZERO_EXTENSION, 0); // temp[63;0] = xmm[63:0]
//
//    store_ir2_to_ir1(&temp, mem, 0);
//
    return true;
}
// sse_op_table1
bool translate_cvttps2dq(IR1_INST *pir1) { return false; }
bool translate_haddpd(IR1_INST *pir1) { return false; }
bool translate_haddps(IR1_INST *pir1) { return false; }
bool translate_hsubpd(IR1_INST *pir1) { return false; }
bool translate_hsubps(IR1_INST *pir1) { return false; }
bool translate_addsubpd(IR1_INST *pir1) { return false; }
bool translate_addsubps(IR1_INST *pir1) { return false; }
bool translate_cvttpd2dq(IR1_INST *pir1) { return false; }
// end of sse_op_table1
bool translate_clflush(IR1_INST *pir1) { return true; }
bool translate_cmpxchg16b(IR1_INST *pir1) { return false; } // only support in x86-64
// load unaligned from mem128 to xmm128
bool translate_lddqu(IR1_INST *pir1)
{
    lsassertm(0, "MISC lddqu to be implemented in LoongArch.\n");
//    IR1_OPND *dst_ir1 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *src_ir1 = ir1_get_opnd(pir1, 1);
//
//    lsassert(ir1_opnd_is_mem(src_ir1));
//    IR2_OPND src_lo = ra_alloc_itemp();
//    IR2_OPND src_hi = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&src_lo, src_ir1, ZERO_EXTENSION, false);
//    load_ir1_to_ir2(&src_hi, src_ir1, ZERO_EXTENSION, true);
//
//    store_ir2_to_ir1(&src_lo, dst_ir1, false);
//    store_ir2_to_ir1(&src_hi, dst_ir1, true);

    return true;
}
// bool translate_fninit(IR1_INST *pir1) { return false; }
// movbe = mov + reverse endian (little endian to big endian)
// so copy and paste translate_mov's code here
bool translate_movbe(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (tr_gen_excp_check(pir1)) {
        return true;
    }
#endif
    lsassertm(0, "MISC movbe to be implemented in LoongArch.\n");

//    IR2_OPND source_opnd = ra_alloc_itemp();
//    load_ir1_to_ir2(&source_opnd, ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, false);
//
//    reverse_ir2_ireg_opnd_endian(&source_opnd, ir1_opnd_size(ir1_get_opnd(pir1, 1)));
//
//#ifdef CONFIG_SOFTMMU
//
//    lsassert(0);//error usage of CF_USE_ICOUNT
//
//    IR1_OPND* opnd1 = ir1_get_opnd(pir1, 1);
//    TRANSLATION_DATA *td = lsenv->tr_data;
//    TranslationBlock *tb = td->curr_tb;
//    if (tb->cflags & CF_USE_ICOUNT
//        && ir1_opnd_is_cr(opnd1)) {
//        tr_gen_io_start();
//        store_ir2_to_ir1(&source_opnd, ir1_get_opnd(pir1, 0), false);
//        tr_gen_io_end();
//    }
//    else {
//        store_ir2_to_ir1(&source_opnd, ir1_get_opnd(pir1, 0), false);
//    }
//#else
//    store_ir2_to_ir1(&source_opnd, ir1_get_opnd(pir1, 0), false);
//#endif

    return true;
}

// extract a byte from a xmm reg,
// there are 16 bytes in a xmm reg, so imm4 is enough
bool translate_pextrb(IR1_INST *pir1)
{
    lsassertm(0, "MISC pextrb to be implemented in LoongArch.\n");
//    IR1_OPND *dst = ir1_get_opnd(pir1, 0);
//    IR1_OPND *src = ir1_get_opnd(pir1, 1);
//    int64_t imm4 = ir1_get_opnd(pir1, 2)->imm;
//
//    IR2_OPND xmm_lo_or_hi = ra_alloc_itemp();
//    load_ir1_to_ir2(&xmm_lo_or_hi, src, ZERO_EXTENSION, imm4>7);
//
//    int64_t imm3 = imm4 & 0b111; // the xmm now split into lower 64bits and higher 64bits
//    if(imm3 > 3) // if right shift bit >= 32
//        append_ir2_opnd2i(mips_dsrl32, &xmm_lo_or_hi, &xmm_lo_or_hi, (imm3-4)*8);
//    else // if right shift bit < 32
//        append_ir2_opnd2i(mips_dsrl, &xmm_lo_or_hi, &xmm_lo_or_hi, imm3*8);
//
//    store_ir2_to_ir1(&xmm_lo_or_hi, dst, false);

    return true;
}

// extract 4 bytes from a xmm reg,
// there are 16 bytes in a xmm reg, so imm2 is enough
bool translate_pextrd(IR1_INST *pir1)
{
    lsassertm(0, "MISC pextrd to be implemented in LoongArch.\n");
//    IR1_OPND *dst = ir1_get_opnd(pir1, 0);
//    IR1_OPND *src = ir1_get_opnd(pir1, 1);
//    int64_t imm2 = ir1_get_opnd(pir1, 2)->imm;
//
//    IR2_OPND xmm_lo_or_hi = ra_alloc_itemp();
//    load_ir1_to_ir2(&xmm_lo_or_hi, src, ZERO_EXTENSION, imm2>1);
//
//    int64_t imm1 = imm2 & 0b1; // the xmm now split into lower 64bits and higher 64bits
//    if(imm1) // if right shift bit == 32
//        append_ir2_opnd2i(mips_dsrl32, &xmm_lo_or_hi, &xmm_lo_or_hi, 0);
//
//    store_ir2_to_ir1(&xmm_lo_or_hi, dst, false);

    return true;
}

// extract 8 bytes from a xmm reg,
// there are 16 bytes in a xmm reg, so imm1 is enough
bool translate_pextrq(IR1_INST *pir1)
{
    lsassertm(0, "MISC pextrq to be implemented in LoongArch.\n");
//    IR1_OPND *dst = ir1_get_opnd(pir1, 0);
//    IR1_OPND *src = ir1_get_opnd(pir1, 1);
//    int64_t imm1 = ir1_get_opnd(pir1, 2)->imm;
//
//    IR2_OPND xmm_lo_or_hi = ra_alloc_itemp();
//    load_ir1_to_ir2(&xmm_lo_or_hi, src, ZERO_EXTENSION, imm1);
//
//    store_ir2_to_ir1(&xmm_lo_or_hi, dst, false);

    return true;
}

bool translate_extractps(IR1_INST *pir1)
{
    return false;
}

// insert a byte from mem/gpr to xmm
// take "pinsrb $11, %gpr/%mem, %xmm" for example (AT&T syntax)
// '-' means the value we dont care, 'S' means the byte needed be writed to xmm
bool translate_pinsrb(IR1_INST *pir1)
{
    lsassertm(0, "MISC pinsrb to be implemented in LoongArch.\n");
//    IR1_OPND *dst_ir1 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *src_ir1 = ir1_get_opnd(pir1, 1);
//    int64_t imm4 = ir1_get_opnd(pir1, 2)->imm;
//    int64_t imm3 = imm4 & 0b111;
//
//    IR2_OPND mask_byte_ones = ra_alloc_itemp();
//    IR2_OPND mask_byte_zeros = ra_alloc_itemp();
//    {
//        int64_t temp = (int64_t)0xff << (imm3*8);
//        load_imm64_to_ir2(&mask_byte_ones, temp);
//        append_ir2_opnd3(mips_nor, &mask_byte_zeros, &mask_byte_ones, &zero_ir2_opnd);
//    }                                                                       //        0123456789abcdef
//                                                                            // xmm = "----------------"
//    bool lo_or_hi = imm4>7; // higher 64bits:true, lower 64bits:false
//    IR2_OPND dst = ra_alloc_itemp();
//    load_ir1_to_ir2(&dst, dst_ir1, ZERO_EXTENSION, lo_or_hi);   // dst =         "--------"
//
//    // clear the target byte in xmm
//    append_ir2_opnd3(mips_and, &dst, &dst, &mask_byte_zeros);                  // dst =         "---0----"
//
//    IR2_OPND src = ra_alloc_itemp();
//    load_ir1_to_ir2(&src, src_ir1, ZERO_EXTENSION, false);      // src =         "S-------"
//    // mask the least order byte in src
//    if(imm3 > 3)
//        append_ir2_opnd2i(mips_dsll32, &src, &src, (imm3-4)*8);               // src =         "---S----"
//    else
//        append_ir2_opnd2i(mips_dsll, &src, &src, imm3*8);
//    append_ir2_opnd3(mips_and, &src, &src, &mask_byte_ones);                   // src =         "000S0000"
//
//    append_ir2_opnd3(mips_or, &dst, &dst, &src);                               // dst =         "---S----"
//
//    store_ir2_to_ir1(&dst, dst_ir1, lo_or_hi);                              // xmm = "-----------S----"

    return true;
}

// insert 4 bytes from mem/xmm to xmm
// imm = count_s || count_d || zmask
// take "insertps 0||3||0b1110, %gpr/%mem, %xmm" for example (AT&T syntax)
// '-' means the value we dont care, 'S' means the byte needed be writed to xmm
bool translate_insertps(IR1_INST *pir1)
{
    lsassertm(0, "MISC insertps to be implemented in LoongArch.\n");
//    IR1_OPND *dst_ir1 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *src_ir1 = ir1_get_opnd(pir1, 1);
//    int64_t imm = ir1_get_opnd(pir1, 2)->imm;
//    int64_t zmask = imm & 0b1111;
//    int64_t count_d = (imm>>4) & 0b11;
//    int64_t count_s = (imm>>6) & 0b11;
//
//    IR2_OPND mask_byte_ones = ra_alloc_itemp();
//    IR2_OPND mask_byte_zeros = ra_alloc_itemp();
//    {
//        int64_t temp = (int64_t)0xffffffff << ((count_d&1)*32);
//        load_imm64_to_ir2(&mask_byte_ones, temp);
//        append_ir2_opnd3(mips_nor, &mask_byte_zeros, &mask_byte_ones, &zero_ir2_opnd);
//    }                                                                       //            0123456789abcdef
//                                                                            // xmm_dst = "----------------"
//    IR2_OPND dst = ra_alloc_itemp();
//    load_ir1_to_ir2(&dst, dst_ir1, ZERO_EXTENSION, count_d>1);  // dst     =         "--------"
//    // clear the target byte in xmm
//    append_ir2_opnd3(mips_and, &dst, &dst, &mask_byte_zeros);                  // dst     =         "----0000"
//                                                                            // xmm_src = "SSSS------------"
//    IR2_OPND src = ra_alloc_itemp();
//    load_ir1_to_ir2(&src, src_ir1, ZERO_EXTENSION, count_s>1);  // src     = "SSSS----"
//    // mask the least order byte in src
//    if((count_d&1)-(count_s&1)>0)
//        append_ir2_opnd2i(mips_dsll32, &src, &src, 0);                        // src     = "----SSSS"
//    else if((count_d&1)-(count_s&1)<0)
//        append_ir2_opnd2i(mips_dsrl32, &src, &src, 0);
//
//    append_ir2_opnd3(mips_and, &src, &src, &mask_byte_ones);                   // src     = "0000SSSS"
//
//    append_ir2_opnd3(mips_or, &dst, &dst, &src);                               // dst     =         "----SSSS"
//
//    store_ir2_to_ir1(&dst, dst_ir1, count_d>1);                             // xmm_dst = "------------SSSS"
//
//    IR2_OPND dst_lo = ra_alloc_itemp();
//    IR2_OPND dst_hi = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&dst_lo, dst_ir1, ZERO_EXTENSION, false);
//    load_ir1_to_ir2(&dst_hi, dst_ir1, ZERO_EXTENSION, true);
//
//    load_imm64_to_ir2(&mask_byte_ones, (int64_t)0xffffffff<<32);
//    append_ir2_opnd3(mips_nor, &mask_byte_zeros, &mask_byte_ones, &zero_ir2_opnd);
//    if(zmask & 0b1)
//        append_ir2_opnd3(mips_and, &dst_lo, &dst_lo, &mask_byte_ones); // clear 1st 32bits
//    if(zmask & 0b10)
//        append_ir2_opnd3(mips_and, &dst_lo, &dst_lo, &mask_byte_zeros); // clear 2nd 32bits
//    if(zmask & 0b100)
//        append_ir2_opnd3(mips_and, &dst_hi, &dst_hi, &mask_byte_ones); // clear 3nd 32bits
//    if(zmask & 0b1000)
//        append_ir2_opnd3(mips_and, &dst_hi, &dst_hi, &mask_byte_zeros); // clear 4th 32bits
//    store_ir2_to_ir1(&dst_lo, dst_ir1, false);
//    store_ir2_to_ir1(&dst_hi, dst_ir1, true);

    return true;
}

// insert 4 bytes from mem/gpr to xmm
// take "pinsrd $3, %gpr/%mem, %xmm" for example (AT&T syntax)
// '-' means the value we dont care, 'S' means the byte needed be writed to xmm
bool translate_pinsrd(IR1_INST *pir1)
{
    lsassertm(0, "MISC pinsrd to be implemented in LoongArch.\n");
//    IR1_OPND *dst_ir1 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *src_ir1 = ir1_get_opnd(pir1, 1);
//    int64_t imm2 = ir1_get_opnd(pir1, 2)->imm;
//    int64_t imm1 = imm2 & 1;
//
//    IR2_OPND mask_byte_ones = ra_alloc_itemp();
//    IR2_OPND mask_byte_zeros = ra_alloc_itemp();
//    {
//        int64_t temp = (int64_t)0xffffffff << (imm1*32);
//        load_imm64_to_ir2(&mask_byte_ones, temp);
//        append_ir2_opnd3(mips_nor, &mask_byte_zeros, &mask_byte_ones, &zero_ir2_opnd);
//    }                                                                       //        0123456789abcdef
//                                                                            // xmm = "----------------"
//    bool lo_or_hi = imm2>1; // higher 64bits:true, lower 64bits:false
//    IR2_OPND dst = ra_alloc_itemp();
//    load_ir1_to_ir2(&dst, dst_ir1, ZERO_EXTENSION, lo_or_hi);   // dst =         "--------"
//    // clear the target byte in xmm
//    append_ir2_opnd3(mips_and, &dst, &dst, &mask_byte_zeros);                  // dst =         "----0000"
//
//    IR2_OPND src = ra_alloc_itemp();
//    load_ir1_to_ir2(&src, src_ir1, ZERO_EXTENSION, false);      // src =         "SSSS----"
//    // mask the least order byte in src
//    if(imm1)
//        append_ir2_opnd2i(mips_dsll32, &src, &src, 0);                        // src =         "----SSSS"
//
//    append_ir2_opnd3(mips_and, &src, &src, &mask_byte_ones);                   // src =         "0000SSSS"
//
//    append_ir2_opnd3(mips_or, &dst, &dst, &src);                               // dst =         "----SSSS"
//
//    store_ir2_to_ir1(&dst, dst_ir1, lo_or_hi);                              // xmm = "------------SSSS"

    return true;
}

// insert 8 bytes from mem/gpr to xmm
// take "pinsrq $1, %gpr/%mem, %xmm" for example (AT&T syntax)
// '-' means the value we dont care, 'S' means the byte needed be writed to xmm
bool translate_pinsrq(IR1_INST *pir1)
{
    lsassertm(0, "MISC pinsrq to be implemented in LoongArch.\n");
//    IR1_OPND *dst_ir1 = ir1_get_opnd(pir1, 0);
//    IR1_OPND *src_ir1 = ir1_get_opnd(pir1, 1);
//    int64_t imm = ir1_get_opnd(pir1, 2)->imm;                               //        0123456789abcdef
//                                                                            // xmm = "----------------"
//    bool lo_or_hi = imm; // higher 64bits:true, lower 64bits:false
//    IR2_OPND dst = ra_alloc_itemp();
//    IR2_OPND src = ra_alloc_itemp();
//
//    load_ir1_to_ir2(&dst, dst_ir1, ZERO_EXTENSION, lo_or_hi);   // dst =         "--------"
//    load_ir1_to_ir2(&src, src_ir1, ZERO_EXTENSION, false);      // src =         "SSSSSSSS"
//
//    append_ir2_opnd2(mips_mov64, &dst, &src);                                 // dst =         "SSSSSSSS"
//    store_ir2_to_ir1(&dst, dst_ir1, lo_or_hi);                              // xmm = "--------SSSSSSSS"
//
    return true;
}

// bool translate_wait(IR1_INST *pir1) { return false; }
bool translate_movsxd(IR1_INST *pir1) { return false; }

bool translate_prefetchnta(IR1_INST *pir1) { return true; }
// bool translate_prefetcht0(IR1_INST *pir1) { return true; }
bool translate_prefetcht1(IR1_INST *pir1) { return true; }
bool translate_prefetcht2(IR1_INST *pir1) { return true; }

#ifndef CONFIG_SOFTMMU
bool translate_ret_without_ss_opt(IR1_INST *pir1);
bool translate_ret_with_ss_opt(IR1_INST *pir1);
void ss_gen_push(IR1_INST *pir1);

void dump_shadow_stack(int debug_type);
void dump_shadow_stack(int debug_type) {
    switch (debug_type) {
        case 1: fprintf(stderr,"%dcall      :",debug_type); break;
        case 2: fprintf(stderr,"%dcallin    :",debug_type); break;
        case 3: fprintf(stderr,"%dret chain :",debug_type); break;
        case 4: fprintf(stderr,"%dret null  :",debug_type); break;
        case 5: fprintf(stderr,"%dret fail  :",debug_type); break;
        default: fprintf(stderr,"%derror    :",debug_type);
    }
    CPUArchState* env = (CPUArchState*)(lsenv->cpu_state);
    SS_ITEM* curr_ss = (SS_ITEM*)(env->vregs[4]);//ss
    for (SS_ITEM* p = ss._ssi_first + 1; p < curr_ss; p++) {
        fprintf(stderr, "%lx ", (uint64_t)p->x86_callee_addr);
    }
    fprintf(stderr, "\n");
}

void ss_gen_push(IR1_INST *pir1) {
    lsassertm(0, "shadow stack to be implemented in LoongArch.\n");
//    IR2_OPND ss_opnd = ra_alloc_ss();
//    // 1. store esp onto ss
//    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
//    append_ir2_opnd2i(mips_store_addrx, &esp_opnd, &ss_opnd, (int)offsetof(SS_ITEM, x86_esp));
//
//    /* debug */
//    //int debug_type = 0;
//    // 2. store callee_addr onto ss
//    if (ir1_opcode(pir1) == X86_INS_CALL && !ir1_is_indirect_call(pir1)){
//        IR2_OPND callee_addr_opnd = ra_alloc_itemp();
//        load_addrx_to_ir2(&callee_addr_opnd, ir1_target_addr(pir1));
//        append_ir2_opnd2i(mips_store_addrx, &callee_addr_opnd, &ss_opnd, (int)offsetof(SS_ITEM, x86_callee_addr));
//        ra_free_temp(&callee_addr_opnd);
//        //debug_type = 1;//call
//    } else {
//        assert(ir1_is_indirect_call(pir1));
//        IR2_OPND next_execut_pc = ra_alloc_dbt_arg2();
//        append_ir2_opnd2i(mips_store_addrx, &next_execut_pc, &ss_opnd, (int)offsetof(SS_ITEM, x86_callee_addr));
//        ra_free_temp(&next_execut_pc);
//        //debug_type = 2;//callin
//    }
//
//    // 3. store ret_tb onto ss
//    IR2_OPND ret_tb_opnd = ra_alloc_itemp();
//    ETB *etb = etb_cache_find(ir1_addr_next(pir1), false);
//    load_addr_to_ir2(&ret_tb_opnd, (ADDR)etb);
//    append_ir2_opnd2i(mips_store_addr, &ret_tb_opnd, &ss_opnd, (int)offsetof(SS_ITEM, return_tb));
//
//    // 4. adjust ss (ss_curr++ )
//    append_ir2_opnd2i(mips_addi_addr, &ss_opnd, &ss_opnd, sizeof(SS_ITEM));
//    /* debug */
//    //tr_save_all_regs_to_env();
//    //IR2_OPND target_opnd = ra_alloc_dbt_arg2();
//    //IR2_OPND param_1 = ir2_opnd_new(IR2_OPND_IREG, 4);
//    //load_ireg_from_addr(&target_opnd, (ADDR)dump_shadow_stack);
//    //append_ir2_opnd1_not_nop(mips_jalr, &target_opnd);
//    //append_ir2_opnd2i(mips_ori, &param_1, &zero_ir2_opnd, debug_type);
//    //tr_load_all_regs_from_env();
//
//
//    ra_free_temp(&ret_tb_opnd);
    return;
}
#endif

#ifndef CONFIG_SOFTMMU
/* user-mode call */
bool translate_call(IR1_INST *pir1)
{
    lsassertm(0, "call to be implemented in LoongArch.\n");
//    if(ir1_is_indirect_call(pir1)){
//        return translate_callin(pir1);
//    }
//    else if (ir1_addr_next(pir1) == ir1_target_addr(pir1)) {
//        return translate_callnext(pir1);
//    }
//
//    /* 1. adjust ssp */
//    IR2_OPND esp_opnd = ra_alloc_gpr(4);
//    append_ir2_opnd2i(mips_addi_addrx, &esp_opnd, &esp_opnd, -4);
//
//    /* 2. save return address onto stack */
//    IR2_OPND return_addr_opnd = ra_alloc_itemp();
//    load_addrx_to_ir2(&return_addr_opnd, ir1_addr_next(pir1));
//
//    if (cpu_get_guest_base() != 0) {
//        IR2_OPND tmp = ra_alloc_itemp();
//        IR2_OPND gbase = ra_alloc_guest_base();
//#ifdef N64
//        if (!ir2_opnd_is_address(&esp_opnd)) {
//            append_ir2_opnd2(mips_mov_addrx, &esp_opnd, &esp_opnd);
//        }
//        append_ir2_opnd3(mips_add_addr, &tmp, &esp_opnd, &gbase);
//#else
//        append_ir2_opnd3(mips_addu, &tmp, &esp_opnd, &gbase);
//#endif
//        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
//        append_ir2_opnd2i(mips_store_addrx, &return_addr_opnd, &tmp, 0);
//    } else {
//        append_ir2_opnd2i(mips_store_addrx, &return_addr_opnd, &esp_opnd, 0);
//    }
//
//    /* 3. push esp, callee_addr, ret_tb onto shadow stack */
//    if(option_shadow_stack)
//        ss_gen_push(pir1);
//
//    /* QEMU exit_tb & goto_tb */
//    tr_generate_exit_tb(pir1, 0);
//
//    ra_free_temp(&return_addr_opnd);
    return true;
}
#endif

#ifndef CONFIG_SOFTMMU
/* user-mode call next */
bool translate_callnext(IR1_INST *pir1)
{
    lsassertm(0, "call next to be implemented in LoongArch.\n");
//    /* 1. load next_instr_addr to tmp_reg */
//    IR2_OPND next_addr = ra_alloc_itemp();
//    load_addrx_to_ir2(&next_addr, ir1_addr_next(pir1));
//
//    /* 2. store next_addr to x86_stack */
//    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
//    append_ir2_opnd2i(mips_addi_addrx, &esp_opnd, &esp_opnd, -4);
//
//    if (cpu_get_guest_base() != 0) {
//        IR2_OPND tmp = ra_alloc_itemp();
//        IR2_OPND gbase = ra_alloc_guest_base();
//#ifdef N64
//        if (!ir2_opnd_is_address(&esp_opnd)) {
//            append_ir2_opnd2(mips_mov_addrx, &esp_opnd, &esp_opnd);
//        }
//        append_ir2_opnd3(mips_add_addr, &tmp, &esp_opnd, &gbase);
//#else
//        append_ir2_opnd3(mips_addu, &tmp, &esp_opnd, &gbase);
//#endif
//        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
//        append_ir2_opnd2i(mips_store_addrx, &next_addr, &tmp, 0);
//    } else {
//        append_ir2_opnd2i(mips_store_addrx, &next_addr, &esp_opnd, 0);
//    }
//
//    ra_free_temp(&next_addr);
//
    return true;
}
#endif

#ifndef CONFIG_SOFTMMU
/* user-mode call indirect */
bool translate_callin(IR1_INST *pir1)
{
    lsassertm(0, "callin to be implemented in LoongArch.\n");
//    /* 1. set successor x86 address */
//    IR2_OPND succ_x86_addr_opnd = ra_alloc_dbt_arg2();
//    load_ir1_to_ir2(&succ_x86_addr_opnd, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
//    ra_free_temp(&succ_x86_addr_opnd);
//
//    /* 2. adjust esp */
//    IR2_OPND esp_opnd = ra_alloc_gpr(4);
//    append_ir2_opnd2i(mips_addi_addrx, &esp_opnd, &esp_opnd, -4);
//
//    /* 3. save return address onto stack */
//    IR2_OPND return_addr_opnd = ra_alloc_itemp();
//    load_addrx_to_ir2(&return_addr_opnd, ir1_addr_next(pir1));
//
//    if (cpu_get_guest_base() != 0) {
//        IR2_OPND tmp = ra_alloc_itemp();
//        IR2_OPND gbase = ra_alloc_guest_base();
//#ifdef N64
//        if (!ir2_opnd_is_address(&esp_opnd)) {
//            append_ir2_opnd2(mips_mov_addrx, &esp_opnd, &esp_opnd);
//        }
//        append_ir2_opnd3(mips_add_addr, &tmp, &esp_opnd, &gbase);
//#else
//        append_ir2_opnd3(mips_addu, &tmp, &esp_opnd, &gbase);
//#endif
//        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
//        append_ir2_opnd2i(mips_store_addrx, &return_addr_opnd, &tmp, 0);
//    } else {
//        append_ir2_opnd2i(mips_store_addrx, &return_addr_opnd, &esp_opnd, 0);
//    }
//
//    ra_free_temp(&return_addr_opnd);
//
//    /* 4. push esp, callee_addr, ret_tb onto shadow statck */
//    if(option_shadow_stack)
//         ss_gen_push(pir1);
//
//    /* 5. adjust em to defaul em */
//    tr_adjust_em();
//
//    /* 6. indirect linkage */
//    /* env->tr_data->curr_tb->generate_tb_linkage_indirect(); */
//    tr_generate_exit_tb(pir1, 0);
    return true;
}
#endif

#ifndef CONFIG_SOFTMMU
bool translate_ret_without_ss_opt(IR1_INST *pir1)
{
    lsassertm(0, "shadow stack to be implemented in LoongArch.\n");
//    /* 1. load ret_addr into $25 */
//    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
//    IR2_OPND return_addr_opnd = ra_alloc_dbt_arg2();
//    load_ir1_to_ir2(&return_addr_opnd, &esp_mem32_ir1_opnd, UNKNOWN_EXTENSION, false);
//
//    /* 2. adjust esp */
//    if (pir1 != NULL && ir1_opnd_num(pir1) &&
//        ir1_opnd_type(ir1_get_opnd(pir1, 0)) == X86_OP_IMM)
//        append_ir2_opnd2i(mips_addi_addrx, &esp_opnd, &esp_opnd,
//                          ir1_opnd_uimm(ir1_get_opnd(pir1, 0)) + 4);
//    else
//        append_ir2_opnd2i(mips_addi_addrx, &esp_opnd, &esp_opnd, 4);
//
//    tr_generate_exit_tb(pir1, 0);
//
    return true;
}
#endif


#ifndef CONFIG_SOFTMMU
bool translate_ret_with_ss_opt(IR1_INST *pir1) {
    lsassertm(0, "shadow stack to be implemented in LoongArch.\n");
//    int esp_change_imm = 4;
//    if(pir1!=NULL && ir1_opnd_num(pir1) && ir1_opnd_type(ir1_get_opnd(pir1, 0))==X86_OP_IMM)
//        esp_change_imm = ir1_opnd_uimm(ir1_get_opnd(pir1, 0)) + 4;
//
//    /*1. load supposed returning target TB */
//    IR2_OPND ss_opnd = ra_alloc_ss();
//    IR2_OPND ss_etb = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_load_addr, &ss_etb, &ss_opnd, -(int)sizeof(SS_ITEM)+(int)offsetof(SS_ITEM, return_tb));
//    IR2_OPND supposed_tb = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_load_addr, &supposed_tb, &ss_etb, offsetof(ETB, tb));
//
//    /*2. pop return target address from x86 stack */
//    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
//    IR2_OPND return_addr_opnd = ra_alloc_dbt_arg2();
//    append_ir2_opnd2i(mips_load_addrx, &return_addr_opnd, &esp_opnd, 0);
//    IR2_OPND label_return_to_bt = ir2_opnd_new_type(IR2_OPND_LABEL);
//    /*2.1 check if TB is alloced */
//    append_ir2_opnd3_not_nop(mips_beq, &supposed_tb, &zero_ir2_opnd, &label_return_to_bt);
//    append_ir2_opnd2i(mips_addi_addrx, &esp_opnd, &esp_opnd, esp_change_imm); /* esp pop */
//
//    // 3. load supposed returning x86 address
//    IR2_OPND supposed_x86_addr = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_load_addrx, &supposed_x86_addr, &supposed_tb, offsetof(TranslationBlock, pc));
//
//    // 4. check if x86 address matches
//    IR2_OPND label_match_fail = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd3(mips_bne, &supposed_x86_addr, &return_addr_opnd, &label_match_fail);
//    ra_free_temp(&supposed_x86_addr);
//    ra_free_temp(&return_addr_opnd);
//
//    // 5. find next tb,dirctly jmp
//    IR2_OPND supposed_native_addr = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_load_addr, &supposed_native_addr, &supposed_tb,
//        offsetof(TranslationBlock, tc) + offsetof(struct tb_tc,ptr));
//    append_ir2_opnd2i(mips_addi_addr, &ss_opnd, &ss_opnd, -(int)sizeof(SS_ITEM));
//    /* debug */
//    //tr_save_all_regs_to_env();
//    //IR2_OPND target_opnd = ra_alloc_dbt_arg2();
//    //IR2_OPND param_1 = ir2_opnd_new(IR2_OPND_IREG, 4);
//    //load_ireg_from_addr(&target_opnd, (ADDR)dump_shadow_stack);
//    //append_ir2_opnd1_not_nop(mips_jalr, &target_opnd);
//    //append_ir2_opnd2i(mips_ori, &param_1, &zero_ir2_opnd, 3);//ret && direct chaining
//    //tr_load_all_regs_from_env();
//
//    // 6. check if top_out == top_in, if so, jump to native code, else rotate the fpu
//    IR2_OPND last_executed_tb = ra_alloc_dbt_arg1();
//    TranslationBlock *curr_tb = lsenv->tr_data->curr_tb;
//    load_addr_to_ir2(&last_executed_tb, (ADDR)curr_tb);
//    IR2_OPND top_out = ra_alloc_itemp();
//    IR2_OPND top_in = ra_alloc_itemp();
//    IR2_OPND rotate_step = ra_alloc_dbt_arg1();
//    IR2_OPND rotate_ret_addr = ra_alloc_dbt_arg2();
//    IR2_OPND label_no_rotate = ir2_opnd_new_type(IR2_OPND_LABEL);
//    append_ir2_opnd2i(mips_lbu, &top_out, &last_executed_tb,
//        offsetof(TranslationBlock, extra_tb) + offsetof(ETB,_top_out));
//    append_ir2_opnd2i(mips_lbu, &top_in, &supposed_tb,
//        offsetof(TranslationBlock, extra_tb) + offsetof(ETB,_top_in));
//    append_ir2_opnd3(mips_beq, &top_out, &top_in, &label_no_rotate);
//    ra_free_temp(&supposed_tb);
//    //6.1 top_out != top_in, rotate fpu
//    append_ir2_opnd3(mips_subu, &rotate_step, &top_out, &top_in);
//    append_ir2_opnda_not_nop(mips_j, native_rotate_fpu_by);
//    append_ir2_opnd2(mips_mov64, &rotate_ret_addr, &supposed_native_addr);
//    ra_free_temp(&top_in);
//    ra_free_temp(&top_out);
//
//    //6.2 top_out == top_in, run next tb directly
//    append_ir2_opnd1(mips_label, &label_no_rotate);
//    append_ir2_opnd1(mips_jr, &supposed_native_addr);
//    ra_free_temp(&supposed_native_addr);
//
//    // 7. tb is null, prepare the last executed tb, and jump to the context switch
//    append_ir2_opnd1(mips_label, &label_return_to_bt);
//    append_ir2_opnd2i(mips_addi_addr, &ss_opnd, &ss_opnd, -(int)sizeof(SS_ITEM));
//    /* debug */
//    //tr_save_all_regs_to_env();
//    //load_ireg_from_addr(&target_opnd, (ADDR)dump_shadow_stack);
//    //append_ir2_opnd1_not_nop(mips_jalr, &target_opnd);
//    //append_ir2_opnd2i(mips_ori, &param_1, &zero_ir2_opnd, 4);//ret && direct chaining
//    //tr_load_all_regs_from_env();
//    IR2_OPND tb_ptr_opnd = ra_alloc_dbt_arg1();
//    uint64_t value = (uint64_t)curr_tb;
//    if (cpu_get_guest_base() == 0) {
//        int16 high16 = (int16)((value >> 16) & 0xffff);
//        int16 low16  = (int16)(value & 0xffff);
//        append_ir2_opnd2i(mips_ori, &tb_ptr_opnd, &zero_ir2_opnd, high16);
//        append_ir2_opnd2i(mips_dsll, &tb_ptr_opnd, &tb_ptr_opnd, 16);
//        append_ir2_opnda_not_nop(mips_j, context_switch_native_to_bt_ret_0);
//        append_ir2_opnd2i(mips_ori, &tb_ptr_opnd, &tb_ptr_opnd, low16);
//    } else {
//        load_imm64_to_ir2(&tb_ptr_opnd, (ADDR)curr_tb);
//        append_ir2_opnda_not_nop(mips_j, context_switch_native_to_bt_ret_0);
//    }
//
//    // 8. x86 address does not match
//    // restore esp, because the adjustment of shadow stack needs the original esp
//    append_ir2_opnd1(mips_label, &label_match_fail);
//    append_ir2_opnd2i(mips_addi_addrx, &esp_opnd, &esp_opnd, -esp_change_imm);
//    IR2_OPND esp_change_bytes = ra_alloc_mda();
//    append_ir2_opnd2i(mips_addiu, &esp_change_bytes, &zero_ir2_opnd, esp_change_imm);
//    // prepare the last executed tb, and jump to the adjustment
//    /* debug */
//    //tr_save_all_regs_to_env();
//    //load_ireg_from_addr(&target_opnd, (ADDR)dump_shadow_stack);
//    //append_ir2_opnd1_not_nop(mips_jalr, &target_opnd);
//    //append_ir2_opnd2i(mips_ori, &param_1, &zero_ir2_opnd, 5);//ret && match fail
//    //tr_load_all_regs_from_env();
//
//    load_imm64_to_ir2(&tb_ptr_opnd, (ADDR)curr_tb );
//    append_ir2_opnda(mips_j, ss_match_fail_native);
//    ra_free_temp(&tb_ptr_opnd);

    return true;
}
#endif

#ifndef CONFIG_SOFTMMU
bool translate_ret(IR1_INST *pir1)
{
    if (option_shadow_stack)
        return translate_ret_with_ss_opt(pir1);
    else
        return translate_ret_without_ss_opt(pir1);
}
#endif

bool translate_jmp(IR1_INST *pir1)
{
    if(ir1_is_indirect_jmp(pir1)){
        return translate_jmpin(pir1);
    }
    /* if (env->tr_data->static_translation) */
    /*     env->tr_data->curr_tb->sbt_generate_linkage_jump_target(); */
    /* else */
    /*     env->tr_data->curr_tb->generate_tb_linkage(0); */

    tr_generate_exit_tb(pir1, 1);
    return true;
}

bool translate_jmpin(IR1_INST *pir1)
{
    /* 1. set successor x86 address */
    IR2_OPND next_eip = ra_alloc_dbt_arg2();
    load_ir1_to_ir2(&next_eip, ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
#ifdef CONFIG_SOFTMMU
    if (ir1_opnd_size(ir1_get_opnd(pir1,0)) == 16) {
        append_ir2_opnd2_(lisa_mov16z, &next_eip, &next_eip);
    }
#endif

    /* 2. indirect linkage */
    /* env->tr_data->curr_tb->generate_tb_linkage_indirect(); */
    tr_generate_exit_tb(pir1, 1);
    return true;
}

#ifndef CONFIG_SOFTMMU
bool translate_leave(IR1_INST *pir1)
{
    lsassertm(0, "leave in to be implemented in LoongArch.\n");
//    IR2_OPND rsp_opnd = ra_alloc_gpr(4);
//    IR2_OPND rbp_opnd = ra_alloc_gpr(5);
//    append_ir2_opnd2(mips_mov_addrx, &rsp_opnd, &rbp_opnd);
//
//    if (cpu_get_guest_base() != 0) {
//        IR2_OPND gbase = ra_alloc_guest_base();
//#ifdef N64
//        if (!ir2_opnd_is_address(&rbp_opnd)) {
//            append_ir2_opnd2(mips_mov_addrx, &rbp_opnd, &rbp_opnd);
//        }
//        append_ir2_opnd3(mips_add_addr, &rbp_opnd, &rsp_opnd, &gbase);
//#else
//        append_ir2_opnd3(mips_addu, &rbp_opnd, &rsp_opnd, &gbase);
//#endif
//        ir2_opnd_set_em(&rbp_opnd, EM_MIPS_ADDRESS, 32);
//    }
//    append_ir2_opnd2i(mips_load_addrx, &rbp_opnd, &rbp_opnd, 0);
//    append_ir2_opnd2i(mips_addi_addrx, &rsp_opnd, &rsp_opnd, 4);
//
    return true;
}
#endif

#ifndef CONFIG_SOFTMMU
/* user-mode int: i386 system call */
bool translate_int(IR1_INST *pir1)
{
    lsassertm(0, "int in to be implemented in LoongArch.\n");
//    /* * store registers to env, or if context_switch_native_to_bt is better? */
//    int use_fp = 0;
//    tr_gen_call_to_helper_prologue(use_fp);
//
//    /* * store intno to CPUState */
//    IR2_OPND intno = ra_alloc_itemp();
//    load_addrx_to_ir2(&intno, ir1_get_opnd(pir1, 0)->imm);
//    append_ir2_opnd2i(mips_sw, &intno, &env_ir2_opnd,
//                      lsenv_offset_exception_index(lsenv));
//
//    /* * store next pc to CPUX86State */
//    IR2_OPND next_pc = ra_alloc_itemp();
//    //load_ireg_from_addrx(&next_pc, pir1->_inst_length + pir1->_addr);
//    load_addrx_to_ir2(&next_pc, ir1_addr_next(pir1));
//    append_ir2_opnd2i(mips_store_addrx, &next_pc, &env_ir2_opnd,
//                      lsenv_offset_exception_next_eip(lsenv));
//
//    /* * store curr_tb to last_executed_tb */
//    IR2_OPND tb = ra_alloc_itemp();
//    load_addr_to_ir2(&tb, (ADDR)lsenv->tr_data->curr_tb);
//    append_ir2_opnd2i(mips_store_addr, &tb, &env_ir2_opnd,
//                      lsenv_offset_of_last_executed_tb(lsenv));
//
//    /* * call helper function */
//    IR2_OPND helper_addr_opnd = ra_alloc_dbt_arg2();
//    load_addr_to_ir2(&helper_addr_opnd, (ADDR)helper_raise_int);
//    append_ir2_opnd1(mips_jalr, &helper_addr_opnd);
//    /* * load registers from env */
//    tr_gen_call_to_helper_epilogue(use_fp);
//
    return true;
}
#endif

#ifndef CONFIG_SOFTMMU
bool translate_hlt(IR1_INST *pir1)
{
//    append_ir2_opnd0(mips_nop);
    return true;
}
#endif

#ifndef CONFIG_SOFTMMU
bool translate_cpuid(IR1_INST *pir1)
{
    lsassertm(0, "cpuid to be implemented in LoongArch.\n");
//    /* 1. store registers to env */
//    tr_save_registers_to_env(EAX_USEDEF_BIT, 0, 0, 0, 0, 0x1|options_to_save());
//
//    /* 2. call helper */
//    IR2_OPND helper_addr_opnd = ra_alloc_dbt_arg2();
//    IR2_OPND a0_opnd = ir2_opnd_new(IR2_OPND_IREG, 4);
//
//    load_addr_to_ir2(&helper_addr_opnd, (ADDR)helper_cpuid);
//    append_ir2_opnd2(mips_mov64, &a0_opnd, &env_ir2_opnd);
//    append_ir2_opnd1(mips_jalr, &helper_addr_opnd);
//
//    /* 3. load registers from env */
//    tr_load_registers_from_env(
//        EAX_USEDEF_BIT | ECX_USEDEF_BIT | EDX_USEDEF_BIT | EBX_USEDEF_BIT,
//        0, 0,
//        0, 0, 0x1|options_to_save());
//
    return true;
}
#endif

bool translate_cwd(IR1_INST *pir1)
{
    IR2_OPND value_opnd = ra_alloc_itemp();
    load_ir1_to_ir2(&value_opnd, &ax_ir1_opnd, SIGN_EXTENSION, false);
    append_ir2_opnd2i(LISA_SRAI_W, &value_opnd, &value_opnd, 15);
    store_ir2_to_ir1(&value_opnd, &dx_ir1_opnd, false);
    ra_free_temp(&value_opnd);
    return true;
}

bool translate_cdq(IR1_INST *pir1)
{
#ifdef CONFIG_SOFTMMU
    if (ir1_data_size(pir1) == 16) {
        return translate_cwd(pir1);
    }
#endif

    IR2_OPND eax = ra_alloc_itemp();
    load_ir1_to_ir2(&eax, &eax_ir1_opnd, SIGN_EXTENSION, false);
    IR2_OPND edx = ra_alloc_itemp();
    append_ir2_opnd2_(lisa_mov32s, &edx, &eax);
    append_ir2_opnd2i(LISA_SRAI_W, &edx, &edx, 31);
    store_ir2_to_ir1(&edx, &edx_ir1_opnd, false);
    ra_free_temp(&eax);
    ra_free_temp(&edx);
    return true;
}

bool translate_cqo(IR1_INST *pir1)
{
#ifndef CONFIG_SOFTMMU
    lsassertm(0, "i386 not include CQO inst. CQO only validation in x86-64.\n");
#else
    lsassertm_illop(ir1_addr(pir1), 0,
            "i386 not include CQO inst. CQO only validation in x86-64.\n");
#endif
    return true;
}

bool translate_sahf(IR1_INST *pir1)
{
    lsassertm(0, "sahf to be implemented in LoongArch.\n");
//    IR2_OPND ah = ra_alloc_itemp();
//    load_ir1_to_ir2(&ah, &ah_ir1_opnd, ZERO_EXTENSION, false);
//    if (option_lbt) {
//        append_ir2_opnd2i(mips_ori, &ah, &ah, 0x2);
//        append_ir2_opnd1i(mips_mtflag, &ah, 0x1f);
//    } else {
//        IR2_OPND ah_tmp = ra_alloc_itemp();
//        append_ir2_opnd2i(mips_dsrl, &eflags_ir2_opnd, &eflags_ir2_opnd, 8);
//        append_ir2_opnd2i(mips_dsll, &eflags_ir2_opnd, &eflags_ir2_opnd, 8);
//        append_ir2_opnd2i(mips_andi, &ah_tmp, &ah, 0xd5); /* 1101 0101 */
//        append_ir2_opnd3(mips_or, &eflags_ir2_opnd, &eflags_ir2_opnd, &ah_tmp);
//        append_ir2_opnd2i(mips_ori, &eflags_ir2_opnd, &eflags_ir2_opnd, 0x2);
//    }
//    ra_free_temp(&ah);
    return true;
}

bool translate_lahf(IR1_INST *pir1)
{
    lsassertm(0, "lahf to be implemented in LoongArch.\n");
//    IR2_OPND ah = ra_alloc_itemp();
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mfflag, &ah, 0x1f);
//        append_ir2_opnd2i(mips_ori, &ah, &ah, 0x2);
//    } else {
//        append_ir2_opnd2i(mips_andi, &ah, &eflags_ir2_opnd, 0xd7);
//    }
//    store_ir2_to_ir1(&ah, &ah_ir1_opnd, false);
//    ra_free_temp(&ah);
    return true;
}

bool translate_cmc(IR1_INST *pir1)
{
    lsassertm(0, "cmc to be implemented in LoongArch.\n");
//    IR2_OPND temp = ra_alloc_itemp();
//    if (option_lbt) {
//        append_ir2_opnd1i(mips_mfflag, &temp, 0x1);
//        append_ir2_opnd2i(mips_xori, &temp, &temp, 0x1);
//        append_ir2_opnd1i(mips_mtflag, &temp, 0x1);
//    } else {
//        append_ir2_opnd2i(mips_ori, &temp, &zero_ir2_opnd, 1);
//        append_ir2_opnd3(mips_xor, &eflags_ir2_opnd, &eflags_ir2_opnd, &temp);
//    }
//    ra_free_temp(&temp);
    return true;
}

bool translate_cbw(IR1_INST *pir1)
{
    IR2_OPND value_opnd = ra_alloc_itemp();
    load_ir1_gpr_to_ir2(&value_opnd, &al_ir1_opnd, SIGN_EXTENSION);
    store_ir2_to_ir1(&value_opnd, &ax_ir1_opnd, false);

    return true;
}

bool translate_cwde(IR1_INST *pir1)
{
    /* EAX = signed extension(ax) */
#ifndef CONFIG_SOFTMMU
    /* In user-mode, it is always 32-bit */
    IR2_OPND eax_opnd = ra_alloc_gpr(eax_index);
    append_ir2_opnd2_(lisa_mov16s, &eax_opnd, &eax_opnd);
    //append_ir2_opnd2i(LISA_SLLI_W, &eax_opnd, &eax_opnd, 16);
    //append_ir2_opnd2i(LISA_SRAI_W, &eax_opnd, &eax_opnd, 16);
#else
    /* In system-mode, because the capstone will always
     * generate CWDE ignoring the opnd size.
     * So we must do it by ourself. */
    int data_size = ir1_data_size(pir1);
    if (data_size == 16) {
        /* AX = signed extension(al) */
        IR2_OPND al_opnd = ra_alloc_itemp();
        load_ir1_gpr_to_ir2(&al_opnd, &al_ir1_opnd, SIGN_EXTENSION);
        store_ir2_to_ir1(&al_opnd, &ax_ir1_opnd, false);
    }
    else if (data_size == 32) {
        /* EAX = signed extension(ax) */
        IR2_OPND eax_opnd = ra_alloc_gpr(eax_index);
        append_ir2_opnd2_(lisa_mov16s, &eax_opnd, &eax_opnd);
        //append_ir2_opnd2i(mips_sll, &eax_opnd, &eax_opnd, 16);
        //append_ir2_opnd2i(mips_sra, &eax_opnd, &eax_opnd, 16);
    } else {
        lsassertm(0, "unknown data size of cwde.\n");
    }
#endif

    return true;
}

bool translate_cdqe(IR1_INST *pir1)
{
#ifndef CONFIG_SOFTMMU
    lsassertm(0, "i386 not include CDQE inst. CDQE only validation in x86-64.\n");
#else
    lsassertm_illop(ir1_addr(pir1), 0,
            "i386 not include CDQE inst. CDQE only validation in x86-64.\n");
#endif
    return true;
}

#ifndef CONFIG_SOFTMMU
/* fnsave in user-mode */
bool translate_fnsave(IR1_INST *pir1)
{
    lsassertm(0, "fnsave to be implemented in LoongArch.\n");
//    /* store x87 state information */
//    IR2_OPND value = ra_alloc_itemp();
//    IR2_OPND temp = ra_alloc_itemp();
//    load_imm32_to_ir2(&temp, 0xffff0000ULL, UNKNOWN_EXTENSION);
//    IR2_OPND mem_opnd;
//    convert_mem_opnd(&mem_opnd, ir1_get_opnd(pir1, 0), -1);
//    int base_reg_num = ir2_opnd_base_reg_num(&mem_opnd);
//    int offset = ir2_opnd_imm(&mem_opnd);
//    if (cpu_get_guest_base() != 0) {
//        IR2_OPND gbase = ra_alloc_guest_base();
//        IR2_OPND tmp = ra_alloc_itemp();
//        IR2_OPND mem = ir2_opnd_new(IR2_OPND_IREG, base_reg_num);
//#ifdef N64
//        if (!ir2_opnd_is_address(&mem)) {
//            append_ir2_opnd2(mips_mov_addrx, &mem, &mem);
//        }
//        append_ir2_opnd3(mips_add_addr, &tmp, &mem, &gbase);
//#else
//        append_ir2_opnd3(mips_addu, &tmp, &mem, &gbase);
//#endif
//        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
//        base_reg_num = ir2_opnd_base_reg_num(&tmp);
//        ir2_opnd_mem_set_base_ireg(&mem_opnd, &tmp);
//    }
//
//    append_ir2_opnd2i(mips_lh, &value, &env_ir2_opnd,
//                      lsenv_offset_of_control_word(lsenv)); /* control_word */
//    append_ir2_opnd3(mips_or, &value, &temp, &value);
//    append_ir2_opnd2(mips_sw, &value, &mem_opnd);
//
//    assert(offset + 108 <= 32767);
//
//    append_ir2_opnd2i(mips_lh, &value, &env_ir2_opnd,
//                      lsenv_offset_of_status_word(lsenv)); /* status_word */
//    append_ir2_opnd3(mips_or, &value, &temp, &value);
//    IR2_OPND mem_opnd_plus_4 = ir2_opnd_new2(IR2_OPND_MEM, base_reg_num, offset + 4);
//    append_ir2_opnd2(mips_sw, &value, &mem_opnd_plus_4);
//
//    append_ir2_opnd2i(mips_lh, &value, &env_ir2_opnd,
//                      lsenv_offset_of_tag_word(lsenv)); /* tag_word */
//
//    append_ir2_opnd3(mips_or, &value, &temp, &value);
//    /* dispose tag word */
//    IR2_OPND temp_1 = ra_alloc_itemp();
//    load_imm32_to_ir2(&temp_1, 0xffff0000ULL, UNKNOWN_EXTENSION);
//    append_ir2_opnd3(mips_and, &value, &value, &temp_1);
//    IR2_OPND mem_opnd_plus_8 = ir2_opnd_new2(IR2_OPND_MEM, base_reg_num, offset + 8);
//    append_ir2_opnd2(mips_sw, &value, &mem_opnd_plus_8);
//
//    /* append_ir2(mips_lw, value, env_ir2_opnd, */
//    /* env->offset_of_instruction_pointer_offset()); append_ir2(mips_sw, value,
//     */
//    /* ir2_opnd_new2(IR2_OPND_MEM, base_reg_num, offset+12)); */
//
//    /* append_ir2(mips_lw, value, env_ir2_opnd, */
//    /* env->offset_of_selector_opcode()); append_ir2(mips_sw, value, */
//    /* ir2_opnd_new2(IR2_OPND_MEM, base_reg_num, offset+16)); */
//
//    IR2_OPND mem_opnd_plus_24 = ir2_opnd_new2(IR2_OPND_MEM, base_reg_num, offset + 24);
//    append_ir2_opnd2(mips_sw, &temp, &mem_opnd_plus_24);
//
//    /* store x87 registers stack */
//    int i;
//    for (i = 0; i < 7; i++) {
//        IR2_OPND st = ra_alloc_st(i);
//        IR2_OPND mem_opnd = ir2_opnd_new2(IR2_OPND_MEM, base_reg_num, offset + 28 + 10 * i);
//        store_64_bit_freg_to_ir1_80_bit_mem(&st, &mem_opnd);
//
//    }
//
//    /* initialize FPU state */
//    load_imm32_to_ir2(&temp, 0xffff037fULL, UNKNOWN_EXTENSION);
//    append_ir2_opnd2i(mips_sw, &temp, &env_ir2_opnd,
//                      lsenv_offset_of_control_word(lsenv));
//    load_imm32_to_ir2(&temp, 0x0000ffffULL, UNKNOWN_EXTENSION);
//    append_ir2_opnd2i(mips_sw, &temp, &env_ir2_opnd,
//                      lsenv_offset_of_tag_word(lsenv));
//    //clear top
//    append_ir2_opnd2i(mips_sw, &zero_ir2_opnd, &env_ir2_opnd,
//                      lsenv_offset_of_top(lsenv));
//
//    tr_gen_top_mode_init();

    return true;
}
#endif

bool translate_fsave(IR1_INST *pir1) { return translate_fnsave(pir1); }

bool translate_prefetcht0(IR1_INST *pir1)
{
#ifndef CONFIG_SOFTMMU
    lsassertm(0, "prefetch0 to be implemented in LoongArch.\n");
//    IR2_OPND mem;
//    convert_mem_opnd(&mem, ir1_get_opnd(pir1, 0), -1);
//    append_ir2_opnd1i(mips_pref, &mem, 0);
#else
    /* nothing to do in system mode */
#endif
    return true;
}

bool translate_emms(IR1_INST *pir1)
{
    lsassertm(0, "emms to be implemented in LoongArch.\n");
//    // QEMU implements emms by helper func, which is simple, so I choose to implements it by mips assembly.
//    // void helper_emms(CPUX86State *env)
//    // {
//    //     /* set to empty state */
//    //     *(uint32_t *)(env->fptags) = 0x01010101;
//    //     *(uint32_t *)(env->fptags + 4) = 0x01010101;
//    // }
//
//    // uint8_t fptags[8];   /* 0 = valid, 1 = empty */
//    IR2_OPND empty = ra_alloc_itemp();
//    IR2_OPND fptags0_3 = ir2_opnd_new2(IR2_OPND_MEM, ir2_opnd_base_reg_num(&env_ir2_opnd), lsenv_offset_of_fptags(lsenv));
//    IR2_OPND fptags4_7 = ir2_opnd_new2(IR2_OPND_MEM, ir2_opnd_base_reg_num(&env_ir2_opnd), lsenv_offset_of_fptags(lsenv)+4);
//
//    // empty = 0x0101 0101
//    append_ir2_opnd2i(mips_ori, &empty, &zero_ir2_opnd, 0x0101);
//    append_ir2_opnd2i(mips_sll, &empty, &empty, 16); // imm5
//    append_ir2_opnd2i(mips_ori, &empty, &empty, 0x0101);
//
//    // store [empty, empty] to fptags[8]
//    append_ir2_opnd2(mips_sw, &empty, &fptags0_3);
//    append_ir2_opnd2(mips_sw, &empty, &fptags4_7);
    return true;
}

bool translate_endbr32(IR1_INST *pir1) { return true; }

#ifndef CONFIG_SOFTMMU
bool translate_invalid(IR1_INST *pir1) { return false; }
bool translate_ud0(IR1_INST *pir1) { return false; }
bool translate_jmp_far(IR1_INST *pir1) { return false; }
bool translate_retf(IR1_INST *pir1) { return false; }
bool translate_call_far(IR1_INST *pir1) { return false; }
bool translate_iret(IR1_INST *pir1) { return false; }
bool translate_iretd(IR1_INST *pir1) { return false; }
bool translate_cli(IR1_INST *pir1) { return false; }
bool translate_sti(IR1_INST *pir1) { return false; }
bool translate_in(IR1_INST *pir1) { return false; }
bool translate_out(IR1_INST *pir1) { return false; }
bool translate_ins(IR1_INST *pir1) { return false; }
bool translate_outs(IR1_INST *pir1) { return false; }
bool translate_lidt(IR1_INST *pir1) { return false; }
bool translate_sidt(IR1_INST *pir1) { return false; }
bool translate_lgdt(IR1_INST *pir1) { return false; }
bool translate_sgdt(IR1_INST *pir1) { return false; }
bool translate_lldt(IR1_INST *pir1) { return false; }
bool translate_sldt(IR1_INST *pir1) { return false; }
bool translate_ltr(IR1_INST *pir1) { return false; }
bool translate_str(IR1_INST *pir1) { return false; }
bool translate_clts(IR1_INST *pir1) { return false; }
bool translate_verr(IR1_INST *pir1) { return false; }
bool translate_verw(IR1_INST *pir1) { return false; }
bool translate_lmsw(IR1_INST *pir1) { return false; }
bool translate_smsw(IR1_INST *pir1) { return false; }
bool translate_pushaw(IR1_INST *pir1) { return false; }
bool translate_pushal(IR1_INST *pir1) { return false; }
bool translate_popaw(IR1_INST *pir1) { return false; }
bool translate_popal(IR1_INST *pir1) { return false; }
bool translate_lds(IR1_INST *pir1) { return false; }
bool translate_les(IR1_INST *pir1) { return false; }
bool translate_lfs(IR1_INST *pir1) { return false; }
bool translate_lgs(IR1_INST *pir1) { return false; }
bool translate_lss(IR1_INST *pir1) { return false; }
// AMD's Virtualization: SVM.
bool translate_vmrun(IR1_INST *pir1) { return false; }
bool translate_vmsave(IR1_INST *pir1) { return false; }
bool translate_vmload(IR1_INST *pir1) { return false; }
bool translate_stgi(IR1_INST *pir1) { return false; }
bool translate_clgi(IR1_INST *pir1) { return false; }
bool translate_vmmcall(IR1_INST *pir1) { return false; }
bool translate_skinit(IR1_INST *pir1) { return false; }
bool translate_invlpg(IR1_INST *pir1) { return false; }
bool translate_invlpga(IR1_INST *pir1) { return false; }
bool translate_wrmsr(IR1_INST *pir1) { return false; }
bool translate_rdmsr(IR1_INST *pir1) { return false; }
bool translate_sysenter(IR1_INST *pir1) { return false; }
bool translate_sysexit(IR1_INST *pir1) { return false; }
bool translate_clac(IR1_INST *pir1) { return false; }
bool translate_stac(IR1_INST *pir1) { return false; }
bool translate_lar(IR1_INST *pir1) { return false; }
bool translate_lsl(IR1_INST *pir1) { return false; }
bool translate_cmpxchg8b(IR1_INST *pir1) { return false; }
bool translate_daa(IR1_INST *pir1) { return false; }
bool translate_aaa(IR1_INST *pir1) { return false; }
bool translate_aas(IR1_INST *pir1) { return false; }
bool translate_das(IR1_INST *pir1) { return false; }
bool translate_aam(IR1_INST *pir1) { return false; }
bool translate_aad(IR1_INST *pir1) { return false; }
bool translate_bound(IR1_INST *pir1) { return false; }
bool translate_rsm(IR1_INST *pir1) { return false; }
bool translate_invd(IR1_INST *pir1) { return false; }
bool translate_wbinvd(IR1_INST *pir1) { return false; }
bool translate_arpl(IR1_INST *pir1) { return false; }
bool translate_nop(IR1_INST *pir1) { return true; }
bool translate_rdtsc(IR1_INST *pir1) { return true; }
bool translate_rdtscp(IR1_INST *pir1) { return false; }
bool translate_rdpmc(IR1_INST *pir1) { return false; }
bool translate_wait(IR1_INST *pir1) { return true; }
bool translate_enter(IR1_INST *pir1) { return false; }
bool translate_rdfsbase(IR1_INST *pir1) { return false; }
bool translate_rdgsbase(IR1_INST *pir1) { return false; }
bool translate_wrfsbase(IR1_INST *pir1) { return false; }
bool translate_wrgsbase(IR1_INST *pir1) { return false; }
bool translate_xsave(IR1_INST *pir1) { return false; }
bool translate_xsaveopt(IR1_INST *pir1) { return false; }
bool translate_xrstor(IR1_INST *pir1) { return false; }
bool translate_xgetbv(IR1_INST *pir1) { return false; }
bool translate_xsetbv(IR1_INST *pir1) { return false; }
bool translate_int_3(IR1_INST *pir1) { return false; }
bool translate_into(IR1_INST *pir1) { return false; }
bool translate_int1(IR1_INST *pir1) { return false; }
bool translate_lfence(IR1_INST *pir1) { return true; }
bool translate_mfence(IR1_INST *pir1) { return true; }
bool translate_sfence(IR1_INST *pir1) { return true; }
bool translate_lzcnt(IR1_INST *pir1) { return false; }
bool translate_tzcnt(IR1_INST *pir1) { return false; }
bool translate_monitor(IR1_INST *pir1) { return false; }
bool translate_mwait(IR1_INST *pir1) { return false; }
// AMD's NOW3D Extension
// Wikipedia: AMD announced on August 2010 that support for 3DNow would be dropped in future AMD processors
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
bool translate_cvttps2pi(IR1_INST *pir1) { return false; }
bool translate_cvttpd2pi(IR1_INST *pir1) { return false; }
bool translate_popcnt(IR1_INST *pir1) { return false; }
bool translate_fstenv(IR1_INST *pir1) { return false; }
bool translate_fclex(IR1_INST *pir1) { return false; }
// sse_op_table6
/// CPUID_EXT_SSSE3
bool translate_pshufb(IR1_INST *pir1) { return false; }
bool translate_phaddw(IR1_INST *pir1) { return false; }
bool translate_phaddd(IR1_INST *pir1) { return false; }
bool translate_phaddsw(IR1_INST *pir1) { return false; }
bool translate_pmaddubsw(IR1_INST *pir1) { return false; }
bool translate_phsubw(IR1_INST *pir1) { return false; }
bool translate_phsubd(IR1_INST *pir1) { return false; }
bool translate_phsubsw(IR1_INST *pir1) { return false; }
bool translate_psignb(IR1_INST *pir1) { return false; }
bool translate_psignw(IR1_INST *pir1) { return false; }
bool translate_psignd(IR1_INST *pir1) { return false; }
bool translate_pmulhrsw(IR1_INST *pir1) { return false; }
bool translate_pabsb(IR1_INST *pir1) { return false; }
bool translate_pabsw(IR1_INST *pir1) { return false; }
bool translate_pabsd(IR1_INST *pir1) { return false; }
/// CPUID_EXT_SSE41
bool translate_pblendvb(IR1_INST *pir1) { return false; }
bool translate_blendvps(IR1_INST *pir1) { return false; }
bool translate_blendvpd(IR1_INST *pir1) { return false; }
bool translate_ptest(IR1_INST *pir1) { return false; }
bool translate_pmovsxbw(IR1_INST *pir1) { return false; }
bool translate_pmovsxbd(IR1_INST *pir1) { return false; }
bool translate_pmovsxbq(IR1_INST *pir1) { return false; }
bool translate_pmovsxwd(IR1_INST *pir1) { return false; }
bool translate_pmovsxwq(IR1_INST *pir1) { return false; }
bool translate_pmovsxdq(IR1_INST *pir1) { return false; }
bool translate_pmuldq(IR1_INST *pir1) { return false; }
bool translate_pcmpeqq(IR1_INST *pir1) { return false; }
bool translate_packusdw(IR1_INST *pir1) { return false; }
bool translate_pmovzxbw(IR1_INST *pir1) { return false; }
bool translate_pmovzxbd(IR1_INST *pir1) { return false; }
bool translate_pmovzxbq(IR1_INST *pir1) { return false; }
bool translate_pmovzxwd(IR1_INST *pir1) { return false; }
bool translate_pmovzxwq(IR1_INST *pir1) { return false; }
bool translate_pmovzxdq(IR1_INST *pir1) { return false; }
bool translate_pminsb(IR1_INST *pir1) { return false; }
bool translate_pminsd(IR1_INST *pir1) { return false; }
bool translate_pminuw(IR1_INST *pir1) { return false; }
bool translate_pminud(IR1_INST *pir1) { return false; }
bool translate_pmaxsb(IR1_INST *pir1) { return false; }
bool translate_pmaxsd(IR1_INST *pir1) { return false; }
bool translate_pmaxuw(IR1_INST *pir1) { return false; }
bool translate_pmaxud(IR1_INST *pir1) { return false; }
bool translate_pmulld(IR1_INST *pir1) { return false; }
bool translate_phminposuw(IR1_INST *pir1) { return false; }
/// CPUID_EXT_SSE42
bool translate_pcmpgtq(IR1_INST *pir1) { return false; }
/// CPUID_EXT_AES
bool translate_aesimc(IR1_INST *pir1) { return false; }
bool translate_aesenc(IR1_INST *pir1) { return false; }
bool translate_aesenclast(IR1_INST *pir1) { return false; }
bool translate_aesdec(IR1_INST *pir1) { return false; }
bool translate_aesdeclast(IR1_INST *pir1) { return false; }
// end of sse_op_table6
bool translate_crc32(IR1_INST *pir1) { return false; }
// sse_op_table7
bool translate_roundps(IR1_INST *pir1) { return false; }
bool translate_roundpd(IR1_INST *pir1) { return false; }
bool translate_roundss(IR1_INST *pir1) { return false; }
bool translate_roundsd(IR1_INST *pir1) { return false; }
bool translate_blendps(IR1_INST *pir1) { return false; }
bool translate_blendpd(IR1_INST *pir1) { return false; }
bool translate_pblendw(IR1_INST *pir1) { return false; }
bool translate_palignr(IR1_INST *pir1) { return false; }
bool translate_dpps(IR1_INST *pir1) { return false; }
bool translate_dppd(IR1_INST *pir1) { return false; }
bool translate_mpsadbw(IR1_INST *pir1) { return false; }
bool translate_pclmulqdq(IR1_INST *pir1) { return false; }
bool translate_pcmpestrm(IR1_INST *pir1) { return false; }
bool translate_pcmpestri(IR1_INST *pir1) { return false; }
bool translate_pcmpistrm(IR1_INST *pir1) { return false; }
bool translate_pcmpistri(IR1_INST *pir1) { return false; }
bool translate_aeskeygenassist(IR1_INST *pir1) { return false; }
// end of sse_op_table7
#endif
