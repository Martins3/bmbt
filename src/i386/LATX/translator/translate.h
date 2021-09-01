#ifndef _TRANSLATE_H_
#define _TAANSLATE_H_

#include "../include/common.h"
#include "../ir1/ir1.h"
#include "../ir2/ir2.h"
#include "../include/qemu-def.h"

bool translate_clac(IR1_INST *pir1);
bool translate_stac(IR1_INST *pir1);

bool translate_rdfsbase(IR1_INST *pir1);
bool translate_rdgsbase(IR1_INST *pir1);
bool translate_wrfsbase(IR1_INST *pir1);
bool translate_wrgsbase(IR1_INST *pir1);

bool translate_invalid(IR1_INST *pir1);
bool translate_ud0(IR1_INST *pir1);
bool translate_endbr32(IR1_INST *pir1);

bool translate_add(IR1_INST *pir1);
bool translate_push(IR1_INST *pir1);
bool translate_pop(IR1_INST *pir1);
bool translate_or(IR1_INST *pir1);
bool translate_adc(IR1_INST *pir1);
bool translate_sbb(IR1_INST *pir1);
bool translate_and(IR1_INST *pir1);
bool translate_daa(IR1_INST *pir1);
bool translate_sub(IR1_INST *pir1);
bool translate_das(IR1_INST *pir1);
bool translate_xor(IR1_INST *pir1);
bool translate_aaa(IR1_INST *pir1);
bool translate_cmp(IR1_INST *pir1);
bool translate_aas(IR1_INST *pir1);
bool translate_inc(IR1_INST *pir1);
bool translate_dec(IR1_INST *pir1);
bool translate_pushaw(IR1_INST *pir1);
bool translate_pushal(IR1_INST *pir1);
bool translate_popaw(IR1_INST *pir1);
bool translate_popal(IR1_INST *pir1);
bool translate_bound(IR1_INST *pir1);
bool translate_arpl(IR1_INST *pir1);
bool translate_imul(IR1_INST *pir1);
bool translate_ins(IR1_INST *pir1);
bool translate_outs(IR1_INST *pir1);
bool translate_jo(IR1_INST *pir1);
bool translate_jno(IR1_INST *pir1);
bool translate_jb(IR1_INST *pir1);
bool translate_jae(IR1_INST *pir1);
bool translate_jz(IR1_INST *pir1);
bool translate_jnz(IR1_INST *pir1);
bool translate_jbe(IR1_INST *pir1);
bool translate_ja(IR1_INST *pir1);
bool translate_js(IR1_INST *pir1);
bool translate_jns(IR1_INST *pir1);
bool translate_jp(IR1_INST *pir1);
bool translate_jnp(IR1_INST *pir1);
bool translate_jl(IR1_INST *pir1);
bool translate_jge(IR1_INST *pir1);
bool translate_jle(IR1_INST *pir1);
bool translate_jg(IR1_INST *pir1);
bool translate_test(IR1_INST *pir1);
bool translate_xchg(IR1_INST *pir1);
bool translate_mov(IR1_INST *pir1);
bool translate_lea(IR1_INST *pir1);
bool translate_cbw(IR1_INST *pir1);
bool translate_cwde(IR1_INST *pir1);
bool translate_cdqe(IR1_INST *pir1);
bool translate_cwd(IR1_INST *pir1);
bool translate_cdq(IR1_INST *pir1);
bool translate_cqo(IR1_INST *pir1);
bool translate_call_far(IR1_INST *pir1);
bool translate_pushf(IR1_INST *pir1);
bool translate_popf(IR1_INST *pir1);
bool translate_sahf(IR1_INST *pir1);
bool translate_lahf(IR1_INST *pir1);
bool translate_movs(IR1_INST *pir1);
bool translate_cmps(IR1_INST *pir1);
bool translate_stos(IR1_INST *pir1);
bool translate_lods(IR1_INST *pir1);
bool translate_scas(IR1_INST *pir1);
bool translate_ret(IR1_INST *pir1);
bool translate_les(IR1_INST *pir1);
bool translate_lds(IR1_INST *pir1);
bool translate_enter(IR1_INST *pir1);
bool translate_leave(IR1_INST *pir1);
bool translate_retf(IR1_INST *pir1);
bool translate_int_3(IR1_INST *pir1);
bool translate_int(IR1_INST *pir1);
bool translate_into(IR1_INST *pir1);
bool translate_iret(IR1_INST *pir1);
bool translate_iretd(IR1_INST *pir1);
bool translate_aam(IR1_INST *pir1);
bool translate_aad(IR1_INST *pir1);
bool translate_salc(IR1_INST *pir1);
bool translate_xlat(IR1_INST *pir1);
bool translate_loopnz(IR1_INST *pir1);
bool translate_loopz(IR1_INST *pir1);
bool translate_loop(IR1_INST *pir1);
bool translate_jcxz(IR1_INST *pir1);
bool translate_jecxz(IR1_INST *pir1);
bool translate_jrcxz(IR1_INST *pir1);
bool translate_in(IR1_INST *pir1);
bool translate_out(IR1_INST *pir1);
bool translate_call(IR1_INST *pir1);
bool translate_jmp(IR1_INST *pir1);
bool translate_jmp_far(IR1_INST *pir1);
bool translate_int1(IR1_INST *pir1);
bool translate_hlt(IR1_INST *pir1);
bool translate_cmc(IR1_INST *pir1);
bool translate_clc(IR1_INST *pir1);
bool translate_stc(IR1_INST *pir1);
bool translate_cli(IR1_INST *pir1);
bool translate_sti(IR1_INST *pir1);
bool translate_cld(IR1_INST *pir1);
bool translate_std(IR1_INST *pir1);
bool translate_lar(IR1_INST *pir1);
bool translate_lsl(IR1_INST *pir1);
bool translate_syscall(IR1_INST *pir1);
bool translate_clts(IR1_INST *pir1);
bool translate_sysret(IR1_INST *pir1);
bool translate_invd(IR1_INST *pir1);
bool translate_wbinvd(IR1_INST *pir1);
bool translate_ud2(IR1_INST *pir1);
bool translate_femms(IR1_INST *pir1);
bool translate_nop(IR1_INST *pir1);
bool translate_wrmsr(IR1_INST *pir1);
bool translate_rdtsc(IR1_INST *pir1);
bool translate_rdmsr(IR1_INST *pir1);
bool translate_rdpmc(IR1_INST *pir1);
bool translate_sysenter(IR1_INST *pir1);
bool translate_sysexit(IR1_INST *pir1);
bool translate_getsec(IR1_INST *pir1);
bool translate_cmovo(IR1_INST *pir1);
bool translate_cmovno(IR1_INST *pir1);
bool translate_cmovb(IR1_INST *pir1);
bool translate_cmovae(IR1_INST *pir1);
bool translate_cmovz(IR1_INST *pir1);
bool translate_cmovnz(IR1_INST *pir1);
bool translate_cmovbe(IR1_INST *pir1);
bool translate_cmova(IR1_INST *pir1);
bool translate_cmovs(IR1_INST *pir1);
bool translate_cmovns(IR1_INST *pir1);
bool translate_cmovp(IR1_INST *pir1);
bool translate_cmovnp(IR1_INST *pir1);
bool translate_cmovl(IR1_INST *pir1);
bool translate_cmovge(IR1_INST *pir1);
bool translate_cmovle(IR1_INST *pir1);
bool translate_cmovg(IR1_INST *pir1);
bool translate_seto(IR1_INST *pir1);
bool translate_setno(IR1_INST *pir1);
bool translate_setb(IR1_INST *pir1);
bool translate_setae(IR1_INST *pir1);
bool translate_setz(IR1_INST *pir1);
bool translate_setnz(IR1_INST *pir1);
bool translate_setbe(IR1_INST *pir1);
bool translate_seta(IR1_INST *pir1);
bool translate_sets(IR1_INST *pir1);
bool translate_setns(IR1_INST *pir1);
bool translate_setp(IR1_INST *pir1);
bool translate_setnp(IR1_INST *pir1);
bool translate_setl(IR1_INST *pir1);
bool translate_setge(IR1_INST *pir1);
bool translate_setle(IR1_INST *pir1);
bool translate_setg(IR1_INST *pir1);
bool translate_cpuid(IR1_INST *pir1);
bool translate_btx(IR1_INST *pir1);
bool translate_shld(IR1_INST *pir1);
bool translate_rsm(IR1_INST *pir1);
bool translate_shrd(IR1_INST *pir1);
bool translate_cmpxchg(IR1_INST *pir1);
bool translate_lss(IR1_INST *pir1);
bool translate_lfs(IR1_INST *pir1);
bool translate_lgs(IR1_INST *pir1);
bool translate_movzx(IR1_INST *pir1);
bool translate_bsf(IR1_INST *pir1);
bool translate_movsx(IR1_INST *pir1);
bool translate_xadd(IR1_INST *pir1);
bool translate_movnti(IR1_INST *pir1);
bool translate_bswap(IR1_INST *pir1);
bool translate_rol(IR1_INST *pir1);
bool translate_ror(IR1_INST *pir1);
bool translate_rcl(IR1_INST *pir1);
bool translate_rcr(IR1_INST *pir1);
bool translate_shl(IR1_INST *pir1);
bool translate_shr(IR1_INST *pir1);
bool translate_sal(IR1_INST *pir1);
bool translate_sar(IR1_INST *pir1);
bool translate_fadd(IR1_INST *pir1);
bool translate_fmul(IR1_INST *pir1);
bool translate_fcom(IR1_INST *pir1);
bool translate_fcomp(IR1_INST *pir1);
bool translate_fsub(IR1_INST *pir1);
bool translate_fsubr(IR1_INST *pir1);
bool translate_fdiv(IR1_INST *pir1);
bool translate_fdivr(IR1_INST *pir1);
bool translate_fld(IR1_INST *pir1);
bool translate_fst(IR1_INST *pir1);
bool translate_fstp(IR1_INST *pir1);
bool translate_fldenv(IR1_INST *pir1);
bool translate_fldcw(IR1_INST *pir1);
bool translate_fxch(IR1_INST *pir1);
bool translate_fnop(IR1_INST *pir1);
bool translate_fchs(IR1_INST *pir1);
bool translate_fabs(IR1_INST *pir1);
bool translate_ftst(IR1_INST *pir1);
bool translate_fxam(IR1_INST *pir1);
bool translate_fld1(IR1_INST *pir1);
bool translate_fldl2t(IR1_INST *pir1);
bool translate_fldl2e(IR1_INST *pir1);
bool translate_fldpi(IR1_INST *pir1);
bool translate_fldlg2(IR1_INST *pir1);
bool translate_fldln2(IR1_INST *pir1);
bool translate_fldz(IR1_INST *pir1);
bool translate_f2xm1(IR1_INST *pir1);
bool translate_fyl2x(IR1_INST *pir1);
bool translate_fptan(IR1_INST *pir1);
bool translate_fpatan(IR1_INST *pir1);
bool translate_fxtract(IR1_INST *pir1);
bool translate_fprem1(IR1_INST *pir1);
bool translate_fdecstp(IR1_INST *pir1);
bool translate_fincstp(IR1_INST *pir1);
bool translate_fprem(IR1_INST *pir1);
bool translate_fyl2xp1(IR1_INST *pir1);
bool translate_fsqrt(IR1_INST *pir1);
bool translate_fsincos(IR1_INST *pir1);
bool translate_frndint(IR1_INST *pir1);
bool translate_fscale(IR1_INST *pir1);
bool translate_fsin(IR1_INST *pir1);
bool translate_fcos(IR1_INST *pir1);
bool translate_fiadd(IR1_INST *pir1);
bool translate_fimul(IR1_INST *pir1);
bool translate_ficom(IR1_INST *pir1);
bool translate_ficomp(IR1_INST *pir1);
bool translate_fisub(IR1_INST *pir1);
bool translate_fisubr(IR1_INST *pir1);
bool translate_fidiv(IR1_INST *pir1);
bool translate_fidivr(IR1_INST *pir1);
bool translate_fcmovb(IR1_INST *pir1);
bool translate_fcmove(IR1_INST *pir1);
bool translate_fcmovbe(IR1_INST *pir1);
bool translate_fcmovu(IR1_INST *pir1);
bool translate_fucompp(IR1_INST *pir1);
bool translate_fild(IR1_INST *pir1);
bool translate_fisttp(IR1_INST *pir1);
bool translate_fist(IR1_INST *pir1);
bool translate_fistp(IR1_INST *pir1);
bool translate_fcmovnb(IR1_INST *pir1);
bool translate_fcmovne(IR1_INST *pir1);
bool translate_fcmovnbe(IR1_INST *pir1);
bool translate_fcmovnu(IR1_INST *pir1);
bool translate_feni(IR1_INST *pir1);
bool translate_fedisi(IR1_INST *pir1);
bool translate_fsetpm(IR1_INST *pir1);
bool translate_fucomi(IR1_INST *pir1);
bool translate_fcomi(IR1_INST *pir1);
bool translate_frstor(IR1_INST *pir1);
bool translate_ffree(IR1_INST *pir1);
bool translate_fucom(IR1_INST *pir1);
bool translate_fucomp(IR1_INST *pir1);
bool translate_faddp(IR1_INST *pir1);
bool translate_fmulp(IR1_INST *pir1);
bool translate_fcompp(IR1_INST *pir1);
bool translate_fsubrp(IR1_INST *pir1);
bool translate_fsubp(IR1_INST *pir1);
bool translate_fdivrp(IR1_INST *pir1);
bool translate_fdivp(IR1_INST *pir1);
bool translate_fbld(IR1_INST *pir1);
bool translate_fbstp(IR1_INST *pir1);
bool translate_fucomip(IR1_INST *pir1);
bool translate_fcomip(IR1_INST *pir1);
bool translate_not(IR1_INST *pir1);
bool translate_neg(IR1_INST *pir1);
bool translate_mul(IR1_INST *pir1);
bool translate_div(IR1_INST *pir1);
bool translate_idiv(IR1_INST *pir1);
bool translate_sldt(IR1_INST *pir1);
bool translate_str(IR1_INST *pir1);
bool translate_lldt(IR1_INST *pir1);
bool translate_ltr(IR1_INST *pir1);
bool translate_verr(IR1_INST *pir1);
bool translate_verw(IR1_INST *pir1);
bool translate_sgdt(IR1_INST *pir1);
bool translate_sidt(IR1_INST *pir1);
bool translate_lgdt(IR1_INST *pir1);
bool translate_lidt(IR1_INST *pir1);
bool translate_smsw(IR1_INST *pir1);
bool translate_lmsw(IR1_INST *pir1);
bool translate_invlpg(IR1_INST *pir1);
bool translate_vmcall(IR1_INST *pir1);
bool translate_vmlaunch(IR1_INST *pir1);
bool translate_vmresume(IR1_INST *pir1);
bool translate_vmxoff(IR1_INST *pir1);
bool translate_monitor(IR1_INST *pir1);
bool translate_mwait(IR1_INST *pir1);
bool translate_xgetbv(IR1_INST *pir1);
bool translate_xsetbv(IR1_INST *pir1);
bool translate_vmrun(IR1_INST *pir1);
bool translate_vmmcall(IR1_INST *pir1);
bool translate_vmload(IR1_INST *pir1);
bool translate_vmsave(IR1_INST *pir1);
bool translate_stgi(IR1_INST *pir1);
bool translate_clgi(IR1_INST *pir1);
bool translate_skinit(IR1_INST *pir1);
bool translate_invlpga(IR1_INST *pir1);
bool translate_swapgs(IR1_INST *pir1);
bool translate_rdtscp(IR1_INST *pir1);
bool translate_prefetch(IR1_INST *pir1);
bool translate_prefetchw(IR1_INST *pir1);
bool translate_pi2fw(IR1_INST *pir1);
bool translate_pi2fd(IR1_INST *pir1);
bool translate_pf2iw(IR1_INST *pir1);
bool translate_pf2id(IR1_INST *pir1);
bool translate_pfnacc(IR1_INST *pir1);
bool translate_pfpnacc(IR1_INST *pir1);
bool translate_pfcmpge(IR1_INST *pir1);
bool translate_pfmin(IR1_INST *pir1);
bool translate_pfrcp(IR1_INST *pir1);
bool translate_pfrsqrt(IR1_INST *pir1);
bool translate_pfsub(IR1_INST *pir1);
bool translate_pfadd(IR1_INST *pir1);
bool translate_pfcmpgt(IR1_INST *pir1);
bool translate_pfmax(IR1_INST *pir1);
bool translate_pfrcpit1(IR1_INST *pir1);
bool translate_pfrsqit1(IR1_INST *pir1);
bool translate_pfsubr(IR1_INST *pir1);
bool translate_pfacc(IR1_INST *pir1);
bool translate_pfcmpeq(IR1_INST *pir1);
bool translate_pfmul(IR1_INST *pir1);
bool translate_pfrcpit2(IR1_INST *pir1);
bool translate_pmulhrw(IR1_INST *pir1);
bool translate_pswapd(IR1_INST *pir1);
bool translate_pavgusb(IR1_INST *pir1);
bool translate_movups(IR1_INST *pir1);
bool translate_movupd(IR1_INST *pir1);
bool translate_movss(IR1_INST *pir1);
bool translate_movsd(IR1_INST *pir1);
bool translate_vmovss(IR1_INST *pir1);
bool translate_vmovsd(IR1_INST *pir1);
bool translate_vmovups(IR1_INST *pir1);
bool translate_vmovupd(IR1_INST *pir1);
bool translate_movhlps(IR1_INST *pir1);
bool translate_movlps(IR1_INST *pir1);
bool translate_movlpd(IR1_INST *pir1);
bool translate_movsldup(IR1_INST *pir1);
bool translate_movddup(IR1_INST *pir1);
bool translate_vmovhlps(IR1_INST *pir1);
bool translate_vmovlps(IR1_INST *pir1);
bool translate_vmovlpd(IR1_INST *pir1);
bool translate_vmovsldup(IR1_INST *pir1);
bool translate_vmovddup(IR1_INST *pir1);
bool translate_unpcklps(IR1_INST *pir1);
bool translate_unpcklpd(IR1_INST *pir1);
bool translate_vunpcklps(IR1_INST *pir1);
bool translate_vunpcklpd(IR1_INST *pir1);
bool translate_unpckhps(IR1_INST *pir1);
bool translate_unpckhpd(IR1_INST *pir1);
bool translate_vunpckhps(IR1_INST *pir1);
bool translate_vunpckhpd(IR1_INST *pir1);
bool translate_movlhps(IR1_INST *pir1);
bool translate_movhps(IR1_INST *pir1);
bool translate_movhpd(IR1_INST *pir1);
bool translate_movshdup(IR1_INST *pir1);
bool translate_vmovlhps(IR1_INST *pir1);
bool translate_vmovhps(IR1_INST *pir1);
bool translate_vmovhpd(IR1_INST *pir1);
bool translate_vmovshdup(IR1_INST *pir1);
bool translate_prefetchnta(IR1_INST *pir1);
bool translate_prefetcht0(IR1_INST *pir1);
bool translate_prefetcht1(IR1_INST *pir1);
bool translate_prefetcht2(IR1_INST *pir1);
bool translate_movaps(IR1_INST *pir1);
bool translate_movapd(IR1_INST *pir1);
bool translate_vmovaps(IR1_INST *pir1);
bool translate_vmovapd(IR1_INST *pir1);
bool translate_cvtpi2ps(IR1_INST *pir1);
bool translate_cvtpi2pd(IR1_INST *pir1);
bool translate_cvtsi2ss(IR1_INST *pir1);
bool translate_cvtsi2sd(IR1_INST *pir1);
bool translate_vcvtsi2ss(IR1_INST *pir1);
bool translate_vcvtsi2sd(IR1_INST *pir1);
bool translate_movntps(IR1_INST *pir1);
bool translate_movntpd(IR1_INST *pir1);
bool translate_movntss(IR1_INST *pir1);
bool translate_movntsd(IR1_INST *pir1);
bool translate_vmovntps(IR1_INST *pir1);
bool translate_vmovntpd(IR1_INST *pir1);
bool translate_cvttps2pi(IR1_INST *pir1);
bool translate_cvttpd2pi(IR1_INST *pir1);
bool translate_cvttss2si(IR1_INST *pir1);
bool translate_cvttsd2si(IR1_INST *pir1);
bool translate_vcvttss2si(IR1_INST *pir1);
bool translate_vcvttsd2si(IR1_INST *pir1);
bool translate_cvtps2pi(IR1_INST *pir1);
bool translate_cvtpd2pi(IR1_INST *pir1);
bool translate_cvtss2si(IR1_INST *pir1);
bool translate_cvtsd2si(IR1_INST *pir1);
bool translate_vcvtss2si(IR1_INST *pir1);
bool translate_vcvtsd2si(IR1_INST *pir1);
bool translate_ucomiss(IR1_INST *pir1);
bool translate_ucomisd(IR1_INST *pir1);
bool translate_vucomiss(IR1_INST *pir1);
bool translate_vucomisd(IR1_INST *pir1);
bool translate_comiss(IR1_INST *pir1);
bool translate_comisd(IR1_INST *pir1);
bool translate_vcomiss(IR1_INST *pir1);
bool translate_vcomisd(IR1_INST *pir1);
bool translate_movmskps(IR1_INST *pir1);
bool translate_movmskpd(IR1_INST *pir1);
bool translate_vmovmskps(IR1_INST *pir1);
bool translate_vmovmskpd(IR1_INST *pir1);
bool translate_sqrtps(IR1_INST *pir1);
bool translate_sqrtpd(IR1_INST *pir1);
bool translate_sqrtss(IR1_INST *pir1);
bool translate_sqrtsd(IR1_INST *pir1);
bool translate_vsqrtss(IR1_INST *pir1);
bool translate_vsqrtsd(IR1_INST *pir1);
bool translate_vsqrtps(IR1_INST *pir1);
bool translate_vsqrtpd(IR1_INST *pir1);
bool translate_rsqrtps(IR1_INST *pir1);
bool translate_rsqrtss(IR1_INST *pir1);
bool translate_vrsqrtss(IR1_INST *pir1);
bool translate_vrsqrtps(IR1_INST *pir1);
bool translate_rcpps(IR1_INST *pir1);
bool translate_rcpss(IR1_INST *pir1);
bool translate_vrcpss(IR1_INST *pir1);
bool translate_vrcpps(IR1_INST *pir1);
bool translate_andps(IR1_INST *pir1);
bool translate_andpd(IR1_INST *pir1);
bool translate_vandps(IR1_INST *pir1);
bool translate_vandpd(IR1_INST *pir1);
bool translate_andnps(IR1_INST *pir1);
bool translate_andnpd(IR1_INST *pir1);
bool translate_vandnps(IR1_INST *pir1);
bool translate_vandnpd(IR1_INST *pir1);
bool translate_orps(IR1_INST *pir1);
bool translate_orpd(IR1_INST *pir1);
bool translate_vorps(IR1_INST *pir1);
bool translate_vorpd(IR1_INST *pir1);
bool translate_xorps(IR1_INST *pir1);
bool translate_xorpd(IR1_INST *pir1);
bool translate_vxorps(IR1_INST *pir1);
bool translate_vxorpd(IR1_INST *pir1);
bool translate_addps(IR1_INST *pir1);
bool translate_addpd(IR1_INST *pir1);
bool translate_addss(IR1_INST *pir1);
bool translate_addsd(IR1_INST *pir1);
bool translate_vaddps(IR1_INST *pir1);
bool translate_vaddpd(IR1_INST *pir1);
bool translate_vaddss(IR1_INST *pir1);
bool translate_vaddsd(IR1_INST *pir1);
bool translate_mulps(IR1_INST *pir1);
bool translate_mulpd(IR1_INST *pir1);
bool translate_mulss(IR1_INST *pir1);
bool translate_mulsd(IR1_INST *pir1);
bool translate_vmulps(IR1_INST *pir1);
bool translate_vmulpd(IR1_INST *pir1);
bool translate_vmulss(IR1_INST *pir1);
bool translate_vmulsd(IR1_INST *pir1);
bool translate_cvtps2pd(IR1_INST *pir1);
bool translate_cvtpd2ps(IR1_INST *pir1);
bool translate_cvtss2sd(IR1_INST *pir1);
bool translate_cvtsd2ss(IR1_INST *pir1);
bool translate_vcvtss2sd(IR1_INST *pir1);
bool translate_vcvtsd2ss(IR1_INST *pir1);
bool translate_vcvtps2pd(IR1_INST *pir1);
bool translate_vcvtpd2ps(IR1_INST *pir1);
bool translate_cvtdq2ps(IR1_INST *pir1);
bool translate_cvtps2dq(IR1_INST *pir1);
bool translate_cvttps2dq(IR1_INST *pir1);
bool translate_vcvtdq2ps(IR1_INST *pir1);
bool translate_vcvtps2dq(IR1_INST *pir1);
bool translate_vcvttps2dq(IR1_INST *pir1);
bool translate_subps(IR1_INST *pir1);
bool translate_subpd(IR1_INST *pir1);
bool translate_subss(IR1_INST *pir1);
bool translate_subsd(IR1_INST *pir1);
bool translate_vsubps(IR1_INST *pir1);
bool translate_vsubpd(IR1_INST *pir1);
bool translate_vsubss(IR1_INST *pir1);
bool translate_vsubsd(IR1_INST *pir1);
bool translate_minps(IR1_INST *pir1);
bool translate_minpd(IR1_INST *pir1);
bool translate_minss(IR1_INST *pir1);
bool translate_minsd(IR1_INST *pir1);
bool translate_vminps(IR1_INST *pir1);
bool translate_vminpd(IR1_INST *pir1);
bool translate_vminss(IR1_INST *pir1);
bool translate_vminsd(IR1_INST *pir1);
bool translate_divps(IR1_INST *pir1);
bool translate_divpd(IR1_INST *pir1);
bool translate_divss(IR1_INST *pir1);
bool translate_divsd(IR1_INST *pir1);
bool translate_vdivps(IR1_INST *pir1);
bool translate_vdivpd(IR1_INST *pir1);
bool translate_vdivss(IR1_INST *pir1);
bool translate_vdivsd(IR1_INST *pir1);
bool translate_maxps(IR1_INST *pir1);
bool translate_maxpd(IR1_INST *pir1);
bool translate_maxss(IR1_INST *pir1);
bool translate_maxsd(IR1_INST *pir1);
bool translate_vmaxps(IR1_INST *pir1);
bool translate_vmaxpd(IR1_INST *pir1);
bool translate_vmaxss(IR1_INST *pir1);
bool translate_vmaxsd(IR1_INST *pir1);
bool translate_punpcklbw(IR1_INST *pir1);
bool translate_vpunpcklbw(IR1_INST *pir1);
bool translate_punpcklwd(IR1_INST *pir1);
bool translate_vpunpcklwd(IR1_INST *pir1);
bool translate_punpckldq(IR1_INST *pir1);
bool translate_vpunpckldq(IR1_INST *pir1);
bool translate_packsswb(IR1_INST *pir1);
bool translate_vpacksswb(IR1_INST *pir1);
bool translate_pcmpgtb(IR1_INST *pir1);
bool translate_vpcmpgtb(IR1_INST *pir1);
bool translate_pcmpgtw(IR1_INST *pir1);
bool translate_vpcmpgtw(IR1_INST *pir1);
bool translate_pcmpgtd(IR1_INST *pir1);
bool translate_vpcmpgtd(IR1_INST *pir1);
bool translate_packuswb(IR1_INST *pir1);
bool translate_vpackuswb(IR1_INST *pir1);
bool translate_punpckhbw(IR1_INST *pir1);
bool translate_vpunpckhbw(IR1_INST *pir1);
bool translate_punpckhwd(IR1_INST *pir1);
bool translate_vpunpckhwd(IR1_INST *pir1);
bool translate_punpckhdq(IR1_INST *pir1);
bool translate_vpunpckhdq(IR1_INST *pir1);
bool translate_packssdw(IR1_INST *pir1);
bool translate_vpackssdw(IR1_INST *pir1);
bool translate_punpcklqdq(IR1_INST *pir1);
bool translate_vpunpcklqdq(IR1_INST *pir1);
bool translate_punpckhqdq(IR1_INST *pir1);
bool translate_vpunpckhqdq(IR1_INST *pir1);
bool translate_movd(IR1_INST *pir1);
bool translate_movq(IR1_INST *pir1);
bool translate_vmovd(IR1_INST *pir1);
bool translate_vmovq(IR1_INST *pir1);
bool translate_movdqa(IR1_INST *pir1);
bool translate_movdqu(IR1_INST *pir1);
bool translate_vmovdqa(IR1_INST *pir1);
bool translate_vmovdqu(IR1_INST *pir1);
bool translate_pshufw(IR1_INST *pir1);
bool translate_pshufd(IR1_INST *pir1);
bool translate_pshufhw(IR1_INST *pir1);
bool translate_pshuflw(IR1_INST *pir1);
bool translate_vpshufd(IR1_INST *pir1);
bool translate_vpshufhw(IR1_INST *pir1);
bool translate_vpshuflw(IR1_INST *pir1);
bool translate_pcmpeqb(IR1_INST *pir1);
bool translate_vpcmpeqb(IR1_INST *pir1);
bool translate_pcmpeqw(IR1_INST *pir1);
bool translate_vpcmpeqw(IR1_INST *pir1);
bool translate_pcmpeqd(IR1_INST *pir1);
bool translate_vpcmpeqd(IR1_INST *pir1);
bool translate_emms(IR1_INST *pir1);
bool translate_vzeroupper(IR1_INST *pir1);
bool translate_vzeroall(IR1_INST *pir1);
bool translate_vmread(IR1_INST *pir1);
bool translate_extrq(IR1_INST *pir1);
bool translate_insertq(IR1_INST *pir1);
bool translate_vmwrite(IR1_INST *pir1);
bool translate_haddpd(IR1_INST *pir1);
bool translate_haddps(IR1_INST *pir1);
bool translate_vhaddpd(IR1_INST *pir1);
bool translate_vhaddps(IR1_INST *pir1);
bool translate_hsubpd(IR1_INST *pir1);
bool translate_hsubps(IR1_INST *pir1);
bool translate_vhsubpd(IR1_INST *pir1);
bool translate_vhsubps(IR1_INST *pir1);
bool translate_fxsave(IR1_INST *pir1);
bool translate_fxrstor(IR1_INST *pir1);
bool translate_xave(IR1_INST *pir1);
bool translate_lfence(IR1_INST *pir1);
bool translate_xsave(IR1_INST *pir1);
bool translate_xsaveopt(IR1_INST *pir1);
bool translate_xrstor(IR1_INST *pir1);
bool translate_mfence(IR1_INST *pir1);
bool translate_sfence(IR1_INST *pir1);
bool translate_clflush(IR1_INST *pir1);
bool translate_popcnt(IR1_INST *pir1);
bool translate_bsr(IR1_INST *pir1);
bool translate_lzcnt(IR1_INST *pir1);
bool translate_tzcnt(IR1_INST *pir1);
bool translate_cmpeqps(IR1_INST *pir1);
bool translate_cmpltps(IR1_INST *pir1);
bool translate_cmpleps(IR1_INST *pir1);
bool translate_cmpunordps(IR1_INST *pir1);
bool translate_cmpneqps(IR1_INST *pir1);
bool translate_cmpnltps(IR1_INST *pir1);
bool translate_cmpnleps(IR1_INST *pir1);
bool translate_cmpordps(IR1_INST *pir1);
bool translate_cmpeqpd(IR1_INST *pir1);
bool translate_cmpltpd(IR1_INST *pir1);
bool translate_cmplepd(IR1_INST *pir1);
bool translate_cmpunordpd(IR1_INST *pir1);
bool translate_cmpneqpd(IR1_INST *pir1);
bool translate_cmpnltpd(IR1_INST *pir1);
bool translate_cmpnlepd(IR1_INST *pir1);
bool translate_cmpordpd(IR1_INST *pir1);
bool translate_cmpeqss(IR1_INST *pir1);
bool translate_cmpltss(IR1_INST *pir1);
bool translate_cmpless(IR1_INST *pir1);
bool translate_cmpunordss(IR1_INST *pir1);
bool translate_cmpneqss(IR1_INST *pir1);
bool translate_cmpnltss(IR1_INST *pir1);
bool translate_cmpnless(IR1_INST *pir1);
bool translate_cmpordss(IR1_INST *pir1);
bool translate_cmpeqsd(IR1_INST *pir1);
bool translate_cmpltsd(IR1_INST *pir1);
bool translate_cmplesd(IR1_INST *pir1);
bool translate_cmpunordsd(IR1_INST *pir1);
bool translate_cmpneqsd(IR1_INST *pir1);
bool translate_cmpnltsd(IR1_INST *pir1);
bool translate_cmpnlesd(IR1_INST *pir1);
bool translate_cmpordsd(IR1_INST *pir1);
bool translate_vcmpeqps(IR1_INST *pir1);
bool translate_vcmpltps(IR1_INST *pir1);
bool translate_vcmpleps(IR1_INST *pir1);
bool translate_vcmpunordps(IR1_INST *pir1);
bool translate_vcmpneqps(IR1_INST *pir1);
bool translate_vcmpnltps(IR1_INST *pir1);
bool translate_vcmpnleps(IR1_INST *pir1);
bool translate_vcmpordps(IR1_INST *pir1);
bool translate_vcmpeqpd(IR1_INST *pir1);
bool translate_vcmpltpd(IR1_INST *pir1);
bool translate_vcmplepd(IR1_INST *pir1);
bool translate_vcmpunordpd(IR1_INST *pir1);
bool translate_vcmpneqpd(IR1_INST *pir1);
bool translate_vcmpnltpd(IR1_INST *pir1);
bool translate_vcmpnlepd(IR1_INST *pir1);
bool translate_vcmpordpd(IR1_INST *pir1);
bool translate_vcmpeqss(IR1_INST *pir1);
bool translate_vcmpltss(IR1_INST *pir1);
bool translate_vcmpless(IR1_INST *pir1);
bool translate_vcmpunordss(IR1_INST *pir1);
bool translate_vcmpneqss(IR1_INST *pir1);
bool translate_vcmpnltss(IR1_INST *pir1);
bool translate_vcmpnless(IR1_INST *pir1);
bool translate_vcmpordss(IR1_INST *pir1);
bool translate_vcmpeqsd(IR1_INST *pir1);
bool translate_vcmpltsd(IR1_INST *pir1);
bool translate_vcmplesd(IR1_INST *pir1);
bool translate_vcmpunordsd(IR1_INST *pir1);
bool translate_vcmpneqsd(IR1_INST *pir1);
bool translate_vcmpnltsd(IR1_INST *pir1);
bool translate_vcmpnlesd(IR1_INST *pir1);
bool translate_vcmpordsd(IR1_INST *pir1);
bool translate_pinsrw(IR1_INST *pir1);
bool translate_vpinsrw(IR1_INST *pir1);
bool translate_pextrw(IR1_INST *pir1);
bool translate_vpextrw(IR1_INST *pir1);
bool translate_shufps(IR1_INST *pir1);
bool translate_shufpd(IR1_INST *pir1);
bool translate_vshufps(IR1_INST *pir1);
bool translate_vshufpd(IR1_INST *pir1);
bool translate_cmpxchg8b(IR1_INST *pir1);
bool translate_cmpxchg16b(IR1_INST *pir1);
bool translate_vmptrst(IR1_INST *pir1);
bool translate_addsubpd(IR1_INST *pir1);
bool translate_addsubps(IR1_INST *pir1);
bool translate_vaddsubpd(IR1_INST *pir1);
bool translate_vaddsubps(IR1_INST *pir1);
bool translate_psrlw(IR1_INST *pir1);
bool translate_vpsrlw(IR1_INST *pir1);
bool translate_psrld(IR1_INST *pir1);
bool translate_vpsrld(IR1_INST *pir1);
bool translate_psrlq(IR1_INST *pir1);
bool translate_vpsrlq(IR1_INST *pir1);
bool translate_paddq(IR1_INST *pir1);
bool translate_vpaddq(IR1_INST *pir1);
bool translate_pmullw(IR1_INST *pir1);
bool translate_vpmullw(IR1_INST *pir1);
bool translate_movq2dq(IR1_INST *pir1);
bool translate_movdq2q(IR1_INST *pir1);
bool translate_pmovmskb(IR1_INST *pir1);
bool translate_vpmovmskb(IR1_INST *pir1);
bool translate_psubusb(IR1_INST *pir1);
bool translate_vpsubusb(IR1_INST *pir1);
bool translate_psubusw(IR1_INST *pir1);
bool translate_vpsubusw(IR1_INST *pir1);
bool translate_pminub(IR1_INST *pir1);
bool translate_vpminub(IR1_INST *pir1);
bool translate_pand(IR1_INST *pir1);
bool translate_vpand(IR1_INST *pir1);
bool translate_paddusb(IR1_INST *pir1);
bool translate_vpaddusw(IR1_INST *pir1);
bool translate_paddusw(IR1_INST *pir1);
bool translate_pmaxub(IR1_INST *pir1);
bool translate_vpmaxub(IR1_INST *pir1);
bool translate_pandn(IR1_INST *pir1);
bool translate_vpandn(IR1_INST *pir1);
bool translate_pavgb(IR1_INST *pir1);
bool translate_vpavgb(IR1_INST *pir1);
bool translate_psraw(IR1_INST *pir1);
bool translate_vpsraw(IR1_INST *pir1);
bool translate_psrad(IR1_INST *pir1);
bool translate_vpsrad(IR1_INST *pir1);
bool translate_pavgw(IR1_INST *pir1);
bool translate_vpavgw(IR1_INST *pir1);
bool translate_pmulhuw(IR1_INST *pir1);
bool translate_vpmulhuw(IR1_INST *pir1);
bool translate_pmulhw(IR1_INST *pir1);
bool translate_vpmulhw(IR1_INST *pir1);
bool translate_cvttpd2dq(IR1_INST *pir1);
bool translate_cvtdq2pd(IR1_INST *pir1);
bool translate_cvtpd2dq(IR1_INST *pir1);
bool translate_vcvttpd2dq(IR1_INST *pir1);
bool translate_vcvtdq2pd(IR1_INST *pir1);
bool translate_vcvtpd2dq(IR1_INST *pir1);
bool translate_movntq(IR1_INST *pir1);
bool translate_movntdq(IR1_INST *pir1);
bool translate_vmovntdq(IR1_INST *pir1);
bool translate_psubsb(IR1_INST *pir1);
bool translate_vpsubsb(IR1_INST *pir1);
bool translate_psubsw(IR1_INST *pir1);
bool translate_vpsubsw(IR1_INST *pir1);
bool translate_pminsw(IR1_INST *pir1);
bool translate_vpminsw(IR1_INST *pir1);
bool translate_por(IR1_INST *pir1);
bool translate_vpor(IR1_INST *pir1);
bool translate_paddsb(IR1_INST *pir1);
bool translate_vpaddsb(IR1_INST *pir1);
bool translate_paddsw(IR1_INST *pir1);
bool translate_vpaddsw(IR1_INST *pir1);
bool translate_pmaxsw(IR1_INST *pir1);
bool translate_vpmaxsw(IR1_INST *pir1);
bool translate_pxor(IR1_INST *pir1);
bool translate_vpxor(IR1_INST *pir1);
bool translate_lddqu(IR1_INST *pir1);
bool translate_vlddqu(IR1_INST *pir1);
bool translate_psllw(IR1_INST *pir1);
bool translate_vpsllw(IR1_INST *pir1);
bool translate_pslld(IR1_INST *pir1);
bool translate_vpslld(IR1_INST *pir1);
bool translate_psllq(IR1_INST *pir1);
bool translate_vpsllq(IR1_INST *pir1);
bool translate_pmuludq(IR1_INST *pir1);
bool translate_vpmuludq(IR1_INST *pir1);
bool translate_pmaddwd(IR1_INST *pir1);
bool translate_vpmaddwd(IR1_INST *pir1);
bool translate_psadbw(IR1_INST *pir1);
bool translate_vpsadbw(IR1_INST *pir1);
bool translate_maskmovq(IR1_INST *pir1);
bool translate_maskmovdqu(IR1_INST *pir1);
bool translate_vmaskmovdqu(IR1_INST *pir1);
bool translate_psubb(IR1_INST *pir1);
bool translate_vpsubb(IR1_INST *pir1);
bool translate_psubw(IR1_INST *pir1);
bool translate_vpsubw(IR1_INST *pir1);
bool translate_psubd(IR1_INST *pir1);
bool translate_vpsubd(IR1_INST *pir1);
bool translate_psubq(IR1_INST *pir1);
bool translate_vpsubq(IR1_INST *pir1);
bool translate_paddb(IR1_INST *pir1);
bool translate_vpaddb(IR1_INST *pir1);
bool translate_paddw(IR1_INST *pir1);
bool translate_vpaddw(IR1_INST *pir1);
bool translate_paddd(IR1_INST *pir1);
bool translate_vpaddd(IR1_INST *pir1);
bool translate_fnstenv(IR1_INST *pir1);
bool translate_fstenv(IR1_INST *pir1);
bool translate_fnstcw(IR1_INST *pir1);
bool translate_fstcw(IR1_INST *pir1);
bool translate_fnclex(IR1_INST *pir1);
bool translate_fclex(IR1_INST *pir1);
bool translate_fninit(IR1_INST *pir1);
bool translate_finit(IR1_INST *pir1);
bool translate_fnsave(IR1_INST *pir1);
bool translate_fsave(IR1_INST *pir1);
bool translate_fnstsw(IR1_INST *pir1);
bool translate_fstsw(IR1_INST *pir1);
bool translate_pshufb(IR1_INST *pir1);
bool translate_vpshufb(IR1_INST *pir1);
bool translate_phaddw(IR1_INST *pir1);
bool translate_vphaddw(IR1_INST *pir1);
bool translate_phaddd(IR1_INST *pir1);
bool translate_vphaddd(IR1_INST *pir1);
bool translate_phaddsw(IR1_INST *pir1);
bool translate_vphaddsw(IR1_INST *pir1);
bool translate_pmaddubsw(IR1_INST *pir1);
bool translate_vpmaddubsw(IR1_INST *pir1);
bool translate_phsubw(IR1_INST *pir1);
bool translate_vphsubw(IR1_INST *pir1);
bool translate_phsubd(IR1_INST *pir1);
bool translate_vphsubd(IR1_INST *pir1);
bool translate_phsubsw(IR1_INST *pir1);
bool translate_vphsubsw(IR1_INST *pir1);
bool translate_psignb(IR1_INST *pir1);
bool translate_vpsignb(IR1_INST *pir1);
bool translate_psignw(IR1_INST *pir1);
bool translate_vpsignw(IR1_INST *pir1);
bool translate_psignd(IR1_INST *pir1);
bool translate_vpsignd(IR1_INST *pir1);
bool translate_pmulhrsw(IR1_INST *pir1);
bool translate_vpmulhrsw(IR1_INST *pir1);
bool translate_vpermilps(IR1_INST *pir1);
bool translate_vpermilpd(IR1_INST *pir1);
bool translate_vptestps(IR1_INST *pir1);
bool translate_vptestpd(IR1_INST *pir1);
bool translate_pblendvb(IR1_INST *pir1);
bool translate_blendvps(IR1_INST *pir1);
bool translate_blendvpd(IR1_INST *pir1);
bool translate_ptest(IR1_INST *pir1);
bool translate_vptest(IR1_INST *pir1);
bool translate_vbroadcastss(IR1_INST *pir1);
bool translate_vbroadcastsd(IR1_INST *pir1);
bool translate_vbroadcastf128(IR1_INST *pir1);
bool translate_pabsb(IR1_INST *pir1);
bool translate_vpabsb(IR1_INST *pir1);
bool translate_pabsw(IR1_INST *pir1);
bool translate_vpabsw(IR1_INST *pir1);
bool translate_pabsd(IR1_INST *pir1);
bool translate_vpabsd(IR1_INST *pir1);
bool translate_pmovsxbw(IR1_INST *pir1);
bool translate_vpmovsxbw(IR1_INST *pir1);
bool translate_pmovsxbd(IR1_INST *pir1);
bool translate_vpmovsxbd(IR1_INST *pir1);
bool translate_pmovsxbq(IR1_INST *pir1);
bool translate_vpmovsxbq(IR1_INST *pir1);
bool translate_pmovsxwd(IR1_INST *pir1);
bool translate_vpmovsxwd(IR1_INST *pir1);
bool translate_pmovsxwq(IR1_INST *pir1);
bool translate_vpmovsxwq(IR1_INST *pir1);
bool translate_pmovsxdq(IR1_INST *pir1);
bool translate_vpmovsxdq(IR1_INST *pir1);
bool translate_pmuldq(IR1_INST *pir1);
bool translate_vpmuldq(IR1_INST *pir1);
bool translate_pcmpeqq(IR1_INST *pir1);
bool translate_vpcmpeqq(IR1_INST *pir1);
bool translate_movntdqa(IR1_INST *pir1);
bool translate_vmovntdqa(IR1_INST *pir1);
bool translate_packusdw(IR1_INST *pir1);
bool translate_vpackusdw(IR1_INST *pir1);
bool translate_vmaskmovps(IR1_INST *pir1);
bool translate_vmaskmovpd(IR1_INST *pir1);
bool translate_pmovzxbw(IR1_INST *pir1);
bool translate_vpmovzxbw(IR1_INST *pir1);
bool translate_pmovzxbd(IR1_INST *pir1);
bool translate_vpmovzxbd(IR1_INST *pir1);
bool translate_pmovzxbq(IR1_INST *pir1);
bool translate_vpmovzxbq(IR1_INST *pir1);
bool translate_pmovzxwd(IR1_INST *pir1);
bool translate_vpmovzxwd(IR1_INST *pir1);
bool translate_pmovzxwq(IR1_INST *pir1);
bool translate_vpmovzxwq(IR1_INST *pir1);
bool translate_pmovzxdq(IR1_INST *pir1);
bool translate_vpmovzxdq(IR1_INST *pir1);
bool translate_pcmpgtq(IR1_INST *pir1);
bool translate_vpcmpgtq(IR1_INST *pir1);
bool translate_pminsb(IR1_INST *pir1);
bool translate_vpminsb(IR1_INST *pir1);
bool translate_pminsd(IR1_INST *pir1);
bool translate_vpminsd(IR1_INST *pir1);
bool translate_pminuw(IR1_INST *pir1);
bool translate_vpminuw(IR1_INST *pir1);
bool translate_pminud(IR1_INST *pir1);
bool translate_vpminud(IR1_INST *pir1);
bool translate_pmaxsb(IR1_INST *pir1);
bool translate_vpmaxsb(IR1_INST *pir1);
bool translate_pmaxsd(IR1_INST *pir1);
bool translate_vpmaxsd(IR1_INST *pir1);
bool translate_pmaxuw(IR1_INST *pir1);
bool translate_vpmaxuw(IR1_INST *pir1);
bool translate_pmaxud(IR1_INST *pir1);
bool translate_vpmaxud(IR1_INST *pir1);
bool translate_pmulld(IR1_INST *pir1);
bool translate_vpmulld(IR1_INST *pir1);
bool translate_phminposuw(IR1_INST *pir1);
bool translate_vphminposuw(IR1_INST *pir1);
bool translate_invept(IR1_INST *pir1);
bool translate_invvpid(IR1_INST *pir1);
bool translate_vfmaddsub132ps(IR1_INST *pir1);
bool translate_vfmaddsub132pd(IR1_INST *pir1);
bool translate_vfmsubadd132ps(IR1_INST *pir1);
bool translate_vfmsubadd132pd(IR1_INST *pir1);
bool translate_vfmadd132ps(IR1_INST *pir1);
bool translate_vfmadd132pd(IR1_INST *pir1);
bool translate_vfmadd132ss(IR1_INST *pir1);
bool translate_vfmadd132sd(IR1_INST *pir1);
bool translate_vfmsub132ps(IR1_INST *pir1);
bool translate_vfmsub132pd(IR1_INST *pir1);
bool translate_vfmsub132ss(IR1_INST *pir1);
bool translate_vfmsub132sd(IR1_INST *pir1);
bool translate_vfnmadd132ps(IR1_INST *pir1);
bool translate_vfnmadd132pd(IR1_INST *pir1);
bool translate_vfnmadd132ss(IR1_INST *pir1);
bool translate_vfnmadd132sd(IR1_INST *pir1);
bool translate_vfnmsub132ps(IR1_INST *pir1);
bool translate_vfnmsub132pd(IR1_INST *pir1);
bool translate_vfnmsub132ss(IR1_INST *pir1);
bool translate_vfnmsub132sd(IR1_INST *pir1);
bool translate_vfmaddsub213ps(IR1_INST *pir1);
bool translate_vfmaddsub213pd(IR1_INST *pir1);
bool translate_vfmsubadd213ps(IR1_INST *pir1);
bool translate_vfmsubadd213pd(IR1_INST *pir1);
bool translate_vfmadd213ps(IR1_INST *pir1);
bool translate_vfmadd213pd(IR1_INST *pir1);
bool translate_vfmadd213ss(IR1_INST *pir1);
bool translate_vfmadd213sd(IR1_INST *pir1);
bool translate_vfmsub213ps(IR1_INST *pir1);
bool translate_vfmsub213pd(IR1_INST *pir1);
bool translate_vfmsub213ss(IR1_INST *pir1);
bool translate_vfmsub213sd(IR1_INST *pir1);
bool translate_vfnmadd213ps(IR1_INST *pir1);
bool translate_vfnmadd213pd(IR1_INST *pir1);
bool translate_vfnmadd213ss(IR1_INST *pir1);
bool translate_vfnmadd213sd(IR1_INST *pir1);
bool translate_vfnmsub213ps(IR1_INST *pir1);
bool translate_vfnmsub213pd(IR1_INST *pir1);
bool translate_vfnmsub213ss(IR1_INST *pir1);
bool translate_vfnmsub213sd(IR1_INST *pir1);
bool translate_vfmaddsub231ps(IR1_INST *pir1);
bool translate_vfmaddsub231pd(IR1_INST *pir1);
bool translate_vfmsubadd231ps(IR1_INST *pir1);
bool translate_vfmsubadd231pd(IR1_INST *pir1);
bool translate_vfmadd231ps(IR1_INST *pir1);
bool translate_vfmadd231pd(IR1_INST *pir1);
bool translate_vfmadd231ss(IR1_INST *pir1);
bool translate_vfmadd231sd(IR1_INST *pir1);
bool translate_vfmsub231ps(IR1_INST *pir1);
bool translate_vfmsub231pd(IR1_INST *pir1);
bool translate_vfmsub231ss(IR1_INST *pir1);
bool translate_vfmsub231sd(IR1_INST *pir1);
bool translate_vfnmadd231ps(IR1_INST *pir1);
bool translate_vfnmadd231pd(IR1_INST *pir1);
bool translate_vfnmadd231ss(IR1_INST *pir1);
bool translate_vfnmadd231sd(IR1_INST *pir1);
bool translate_vfnmsub231ps(IR1_INST *pir1);
bool translate_vfnmsub231pd(IR1_INST *pir1);
bool translate_vfnmsub231ss(IR1_INST *pir1);
bool translate_vfnmsub231sd(IR1_INST *pir1);
bool translate_aesimc(IR1_INST *pir1);
bool translate_vaesimc(IR1_INST *pir1);
bool translate_aesenc(IR1_INST *pir1);
bool translate_vaesenc(IR1_INST *pir1);
bool translate_aesenclast(IR1_INST *pir1);
bool translate_vaesenclast(IR1_INST *pir1);
bool translate_aesdec(IR1_INST *pir1);
bool translate_vaesdec(IR1_INST *pir1);
bool translate_aesdeclast(IR1_INST *pir1);
bool translate_vaesdeclast(IR1_INST *pir1);
bool translate_movbe(IR1_INST *pir1);
bool translate_crc32(IR1_INST *pir1);
bool translate_vperm2f128(IR1_INST *pir1);
bool translate_roundps(IR1_INST *pir1);
bool translate_vroundps(IR1_INST *pir1);
bool translate_roundpd(IR1_INST *pir1);
bool translate_vroundpd(IR1_INST *pir1);
bool translate_roundss(IR1_INST *pir1);
bool translate_vroundss(IR1_INST *pir1);
bool translate_roundsd(IR1_INST *pir1);
bool translate_vroundsd(IR1_INST *pir1);
bool translate_blendps(IR1_INST *pir1);
bool translate_vblendps(IR1_INST *pir1);
bool translate_blendpd(IR1_INST *pir1);
bool translate_vblendpd(IR1_INST *pir1);
bool translate_pblendw(IR1_INST *pir1);
bool translate_vpblendvw(IR1_INST *pir1);
bool translate_palignr(IR1_INST *pir1);
bool translate_vpalignr(IR1_INST *pir1);
bool translate_pextrb(IR1_INST *pir1);
bool translate_vpextrb(IR1_INST *pir1);
bool translate_pextrd(IR1_INST *pir1);
bool translate_pextrq(IR1_INST *pir1);
bool translate_vpextrd(IR1_INST *pir1);
bool translate_extractps(IR1_INST *pir1);
bool translate_vextractps(IR1_INST *pir1);
bool translate_vinsertf128(IR1_INST *pir1);
bool translate_vextractf128(IR1_INST *pir1);
bool translate_pinsrb(IR1_INST *pir1);
bool translate_vpinsrb(IR1_INST *pir1);
bool translate_insertps(IR1_INST *pir1);
bool translate_vinsertps(IR1_INST *pir1);
bool translate_pinsrd(IR1_INST *pir1);
bool translate_pinsrq(IR1_INST *pir1);
bool translate_vpinsrd(IR1_INST *pir1);
bool translate_vpinsrq(IR1_INST *pir1);
bool translate_dpps(IR1_INST *pir1);
bool translate_vdpps(IR1_INST *pir1);
bool translate_dppd(IR1_INST *pir1);
bool translate_vdppd(IR1_INST *pir1);
bool translate_mpsadbw(IR1_INST *pir1);
bool translate_vmpsadbw(IR1_INST *pir1);
bool translate_pclmulqdq(IR1_INST *pir1);
bool translate_vpclmulqdq(IR1_INST *pir1);
bool translate_vblendvps(IR1_INST *pir1);
bool translate_vblendvpd(IR1_INST *pir1);
bool translate_vpblendvb(IR1_INST *pir1);
bool translate_pcmpestrm(IR1_INST *pir1);
bool translate_vpcmpestrm(IR1_INST *pir1);
bool translate_pcmpestri(IR1_INST *pir1);
bool translate_vcmpestri(IR1_INST *pir1);
bool translate_pcmpistrm(IR1_INST *pir1);
bool translate_vpcmpistrm(IR1_INST *pir1);
bool translate_pcmpistri(IR1_INST *pir1);
bool translate_vpcmpistri(IR1_INST *pir1);
bool translate_aeskeygenassist(IR1_INST *pir1);
bool translate_vaeskeygenassist(IR1_INST *pir1);
bool translate_psrldq(IR1_INST *pir1);
bool translate_vpsrldq(IR1_INST *pir1);
bool translate_pslldq(IR1_INST *pir1);
bool translate_vpslldq(IR1_INST *pir1);
bool translate_ldmxcsr(IR1_INST *pir1);
bool translate_vldmxcsr(IR1_INST *pir1);
bool translate_stmxcsr(IR1_INST *pir1);
bool translate_vstmxcsr(IR1_INST *pir1);
bool translate_vmptrld(IR1_INST *pir1);
bool translate_vmclear(IR1_INST *pir1);
bool translate_vmxon(IR1_INST *pir1);
bool translate_wait(IR1_INST *pir1);
bool translate_movsxd(IR1_INST *pir1);
bool translate_pause(IR1_INST *pir1);
bool translate_callnext(IR1_INST *pir1);
bool translate_callin(IR1_INST *pir1);
bool translate_jmpin(IR1_INST *pir1);

/* byhand functions */
bool translate_xor_byhand(IR1_INST *pir1);
bool translate_and_byhand(IR1_INST *pir1);
bool translate_or_byhand(IR1_INST *pir1);

bool translate_neg_byhand(IR1_INST *pir1); /* TODO: sys-mode */
bool translate_not_byhand(IR1_INST *pir1);

bool translate_add_byhand(IR1_INST *pir1);
bool translate_adc_byhand(IR1_INST *pir1);
bool translate_sub_byhand(IR1_INST *pir1);

bool translate_cmp_byhand(IR1_INST *pir1);
bool translate_test_byhand(IR1_INST *pir1);

bool translate_mov_byhand(IR1_INST *pir1);
bool translate_movsx_byhand(IR1_INST *pir1); /* TODO: sys-mode */
bool translate_movzx_byhand(IR1_INST *pir1); /* TODO: sys-mode */

bool translate_inc_byhand(IR1_INST *pir1); /* TODO: sys-mode */
bool translate_dec_byhand(IR1_INST *pir1); /* TODO: sys-mode */

bool translate_rol_byhand(IR1_INST *pir1); /* TODO: sys-mode */
bool translate_ror_byhand(IR1_INST *pir1); /* TODO: sys-mode */

bool translate_shl_byhand(IR1_INST *pir1); /* TODO: sys-mode */
bool translate_sal_byhand(IR1_INST *pir1); /* TODO: sys-mode */
bool translate_sar_byhand(IR1_INST *pir1); /* TODO: sys-mode */
bool translate_shr_byhand(IR1_INST *pir1);

bool translate_mul_byhand(IR1_INST *pir1); /* TODO: sys-mode */
bool translate_div_byhand(IR1_INST *pir1); /* TODO: sys-mode */
bool translate_imul_byhand(IR1_INST *pir1); /* TODO: sys-mode */

bool translate_cmpxchg_byhand(IR1_INST *pir1); /* TODO: sys-mode */

#ifdef CONFIG_SOFTMMU
typedef struct helper_cfg_t {
    int sv_allgpr;
    int sv_eflags;
    int cvt_fp80;
} helper_cfg_t;

typedef struct softmmu_slow_path_rcd {
    int tmp_need_save;
    int tmp_mask;

    int is_load;
    IR2_OPCODE op;
    IR2_OPND gpr_ir2_opnd;
    IR2_OPND mem_ir2_opnd;
    int mmu_index;

    int fpu_top;
    ADDR retaddr;

    IR2_OPND label_slow_path;
    IR2_OPND label_exit;
} softmmu_sp_rcd_t;

void td_rcd_softmmu_slow_path(
        IR2_OPCODE op,
        IR2_OPND *gpr_ir2_opnd,
        IR2_OPND *mem_ir2_opnd,
        IR2_OPND *label_slow_path,
        IR2_OPND *label_exit,
        int mmu_index,
        int save_tmp,
        int is_load);

void tr_gen_softmmu_slow_path(void);
#endif

typedef struct TRANSLATION_DATA {
    EXTENSION_MODE
    ireg_em[IR2_ITEMP_MAX]; /* extension mode of the 32 integer registers */
    int8 ireg_eb[IR2_ITEMP_MAX]; /* bits number where the extension starts */
    EXTENSION_MODE hi_em;
    EXTENSION_MODE lo_em;

    TranslationBlock *curr_tb; /* from QEMU */

    /* ir1 */
    IR1_INST *ir1_inst_array;
    int ir1_nr;
    IR1_INST *curr_ir1_inst;
    /* uint8       ir1_dump_threshold[MAX_IR1_NUM_PER_TB]; */

    /* ir2 */
    IR2_INST *ir2_inst_array;
    int ir2_inst_num_max;
    int ir2_inst_num_current;
    int real_ir2_inst_num;

    /* the list of ir2 */
    IR2_INST *first_ir2;
    IR2_INST *last_ir2;

    /* label number */
    int label_num;

#ifdef CONFIG_SOFTMMU
    int max_insns; /* max number of target instruction */
    void *code_highwater; /* to check buffer overflow */

    int slow_path_rcd_max;
    int slow_path_rcd_nr;
    softmmu_sp_rcd_t *slow_path_rcd;

    /* TOP should be saved for precise exception. Especially
     * should be saved before every memory access.
     *
     * But in pratice, the TOP is not changed frequently. And
     * one TB usually does mamny memory access. If the TOP
     * is already saved in the last memory access, then it
     * is OK to not save TOP again.
     *
     * Once the TOP changes, it should be saved again. */
    int is_top_saved;

    /* number of mips for each x86 */
    int x86_ins_idx[MAX_IR1_NUM_PER_TB];
    int x86_ins_mips_nr[MAX_IR1_NUM_PER_TB];
    int x86_ins_nr;
    int x86_ins_size;

    /* flags for system-mode translation */
    struct __sys{
        uint32 flags; /* all execution flags */
        uint32 cflags; /* compile flags*/
        ADDRX  pc;
        ADDRX  cs_base;
        int pe;     /* protected mode */
        int code32; /* 32 bit code segment */
        int ss32;   /* 32 bit stack segment */
        int addseg; /* non zero if either DS/ES/SS have a non zero base */
        int f_st;   /* currently unused */
        int vm86;   /* vm86 mode */
        int cpl;    /* current privilege level */
        int iopl;   /* I/O privilege level */
        int tf;     /* TF cpu flag */
        int mem_index; /* select memory access functions */
        int popl_esp_hack; /* for correct popl with esp base handling */
        int cpuid_features;
        int cpuid_ext_features;
        int cpuid_ext2_features;
        int cpuid_ext3_features;
        int cpuid_7_0_ebx_features;
        int cpuid_xsave_features;
        int bp_hit;
    }sys;

    /* flags for special EOB */
    int need_eob;
    int inhibit_irq;
    int recheck_tf; /* currently not used. it is mainly for debug.*/
    /* Special EOB is ended with a jmp to next instruction.
     * Some special instruction will modify eip in helper function.
     * The the execution will not continue from the next instruction.
     * Set this to ignoe the eip update in generate_exit_tb() of jmp. */
    int ignore_eip_update;
    int ignore_top_update;

    /* context saved flag */
    int in_context_saved;
    int context_save_depth;
    helper_cfg_t helper_cfg;

    /* exit label for icount_decr less than zero */
    IR2_OPND exitreq_label;

    int end_with_exception;
    int dec_icount_inst_id;
#endif

    /* temp register number */
    int itemp_num;
    int ftemp_num;
    uint32_t itemp_mask;
    uint32_t ftemp_mask;
#ifdef REG_ALLOC_ALG_IMM
    uint32_t itemp_mask_bk;
    int itemp_saved;
#endif
#if defined(CONFIG_XTM_PROFILE) && defined(CONFIG_SOFTMMU)
    int curr_ir1_itemp_num;
    int curr_ir1_ftemp_num;
#endif

    int curr_top;               /* top value (changes when translating) */
#ifndef CONFIG_SOFTMMU
    int curr_esp_need_decrease; /* curr_esp need to decrease */
#endif

    uint8 curr_ir1_skipped_eflags; /* these eflag calculation can be skipped */
                                   /* (because of flag pattern, etc) */
} TRANSLATION_DATA;


/* Main functions for binary translation */
void tr_disasm(TranslationBlock *tb);
IR1_INST *get_ir1_list(ETB* etb, ADDRX pc, int *p_ir1_num);

void tr_init(TranslationBlock *tb);
int  tr_translate_tb(TranslationBlock *tb, int *search_size);
bool tr_ir2_generate(TranslationBlock *tb);
void tr_init_translate_ir1(TranslationBlock *tb, int index);
int  tr_ir2_assemble(void *code_start_addr);
void tr_fini(bool check_the_extension); /* default TRUE */
void tr_adjust_em(void);

void tr_tb_init(TRANSLATION_DATA *t, TranslationBlock *tb);
void tr_ir2_array_init(TRANSLATION_DATA *t);
void tr_ra_init(TRANSLATION_DATA *t);
void tr_em_init(TRANSLATION_DATA *t);
void tr_fpu_init(TRANSLATION_DATA *t, TranslationBlock *tb);
#ifndef CONFIG_SOFTMMU
void tr_flag_pattern_init(TranslationBlock *tb);
#endif

#ifdef CONFIG_SOFTMMU
int tr_check_buffer_overflow(void *, TRANSLATION_DATA *);
#endif

void label_dispose(void *code_cache_addr);

void generate_context_switch_bt_to_native(void *code_buf);
void generate_context_switch_native_to_bt(void *code_buf);

int generate_native_jmp_glue_all(void *code_buffer);

void ss_generate_match_fail_native_code(void* code_buf);

extern ADDR native_jmp_glue_0;
extern ADDR native_jmp_glue_1;
extern ADDR native_jmp_glue_2;

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_FAST_CS)
int generate_fastcs_jmp_glue_all(void *code_buffer);
extern ADDR jmp_glue_fastcs_fpu_0;
extern ADDR jmp_glue_fastcs_fpu_1;
extern ADDR jmp_glue_fastcs_xmm_0;
extern ADDR jmp_glue_fastcs_xmm_1;
#endif

extern void tr_ir2_optimize(void);
extern void tr_skip_eflag_calculation(int usedef_bits);

/* operand conversion */
void convert_mem_opnd(IR2_OPND*, IR1_OPND *, int a);
void convert_mem_opnd_with_bias(IR2_OPND *, IR1_OPND *, int, int a);
IR2_OPND convert_mem_opnd_with_bias_within_imm_bits(IR1_OPND *opnd1, int bias, int bits);
IR2_OPND convert_gpr_opnd(IR1_OPND *, EXTENSION_MODE);
IR2_OPND load_freg128_from_ir1(IR1_OPND *);
void reverse_ir2_ireg_opnd_endian(IR2_OPND *opnd, int size_in_bit);

/* load/store Integer Registers */
void load_imm32_to_ir2(IR2_OPND*, uint32, EXTENSION_MODE);
void load_imm64_to_ir2(IR2_OPND*, uint64);
void load_addrx_to_ir2(IR2_OPND*, ADDRX);
void load_addr_to_ir2(IR2_OPND*, ADDR);

/* Load IR1_OPND to a given IR2_OPND */
void load_ir1_to_ir2(IR2_OPND*, IR1_OPND *, EXTENSION_MODE, bool is_xmm_hi);
void load_ir1_imm_to_ir2(IR2_OPND*, IR1_OPND*, EXTENSION_MODE);
void load_ir1_mem_addr_to_ir2(IR2_OPND*, IR1_OPND*, int addr_size);
void load_ir1_mem_to_ir2(IR2_OPND*, IR1_OPND*, EXTENSION_MODE, bool is_xmm_hi, int addr_size);
void load_ir1_gpr_to_ir2(IR2_OPND*, IR1_OPND*, EXTENSION_MODE);
void load_ir1_mmx_to_ir2(IR2_OPND*, IR1_OPND*, EXTENSION_MODE);
void load_ir1_xmm_to_ir2(IR2_OPND*, IR1_OPND*, EXTENSION_MODE, bool is_xmm_hi);
void load_ir1_seg_to_ir2(IR2_OPND*, IR1_OPND*);
void load_ir1_cr_to_ir2(IR2_OPND*, IR1_OPND*);
void load_ir1_dr_to_ir2(IR2_OPND*, IR1_OPND*);
/* Load IR1_OPND to IR2_OPND, which is allocated inside this function.
 * In this case, IR2_OPND might be one of the mapping register. */
void load_ir1_to_ir2_ptr(IR2_OPND*, IR1_OPND *, EXTENSION_MODE, bool is_xmm_hi);

void store_ir2_to_ir1(IR2_OPND*, IR1_OPND *, bool is_xmm_hi);

void store_ir2_to_ir1_cr(IR2_OPND*, IR1_OPND*);
void store_ir2_to_ir1_dr(IR2_OPND*, IR1_OPND*);
void store_ir2_to_ir1_gpr(IR2_OPND*, IR1_OPND*);
void store_ir2_to_ir1_mem(IR2_OPND*, IR1_OPND*, bool is_xmm_hi, int addr_size);
void store_ir2_to_ir1_seg(IR2_OPND*, IR1_OPND*);

void load_eflags_cf_to_ir2(IR2_OPND *);

#ifndef CONFIG_SOFTMMU
void load_string_step_to_ir2(IR2_OPND *p_step_opnd, IR1_INST *pir1);
#else
void load_string_step_to_ir2(IR2_OPND *p_step_opnd, IR1_INST *pir1, int opidx);
#endif

/* load/store Float-Point registers */
IR2_OPND load_freg_from_ir1_1(IR1_OPND *opnd1, bool is_xmm_hi, bool is_convert);
void load_freg_from_ir1_2(IR2_OPND*, IR1_OPND *, bool is_xmm_hi, bool is_convert);
void store_freg_to_ir1(IR2_OPND*, IR1_OPND *, bool is_xmm_hi, bool is_convert);
void store_64_bit_freg_to_ir1_80_bit_mem(IR2_OPND *opnd2, IR2_OPND *mem_opnd);
void store_freg128_to_ir1_mem(IR2_OPND *opnd2, IR1_OPND *opnd1);
void load_freg128_from_ir1_mem(IR2_OPND *opnd2, IR1_OPND *opnd1);

extern void load_singles_from_ir1_pack(IR2_OPND *single0, IR2_OPND *single1, IR1_OPND *opnd1, bool is_xmm_hi);
extern void store_singles_to_ir2_pack(IR2_OPND *single0, IR2_OPND *single1, IR2_OPND *pack);

/* FPU related functions */
extern void tr_fpu_push(void);
extern void tr_fpu_pop(void);
extern void tr_fpu_inc(void);
extern void tr_fpu_dec(void);
extern void tr_fpu_enable_top_mode(void);
extern void tr_fpu_disable_top_mode(void);

void xtm_fpu_fix_before_exec_tb(CPUX86State *env,
        TranslationBlock *tb);
void xtm_fpu_fix_after_exec_tb(CPUX86State *env,
        TranslationBlock *tb, TranslationBlock *last_tb);
void xtm_fpu_fix_cpu_loop_exit(void);

void convert_fpregs_64_to_x80(void);
void convert_fpregs_x80_to_64(void);

extern IR2_OPND set_fpu_fcsr_rounding_field_by_x86(void);
extern void set_fpu_fcsr(IR2_OPND*);

int generate_native_rotate_fpu_by(void *code_buf);
extern ADDR native_rotate_fpu_by; /* usage: native_rotate_fpu_by(step, return_address) */
extern void rotate_fpu_to_top(int top);
extern void rotate_fpu_by(int step);
extern void rotate_fpu_to_bias(int bias);

/* Special process for extension mode */
extern void em_convert_gpr_to_addrx(IR1_OPND *);
void        em_recognize_address(IR1_INST *);
int         em_validate_address(int);

/* Generate codes for tblink */
void tr_generate_exit_tb(IR1_INST *branch, int succ_id);
void tr_gen_exit_tb_load_tb_addr(IR2_OPND *tbptr, ADDR tb_addr);
void tr_gen_exit_tb_j_tb_link(TranslationBlock *tb, int succ_id);
void tr_gen_exit_tb_load_next_eip(int reload_eip_from_env,
        IR2_OPND *eip_opnd, ADDRX eip, int opnd_size);
void tr_gen_exit_tb_j_context_switch(IR2_OPND *tbptr, int can_link, int succ_id);

void tr_gen_eob_if_tb_too_large(IR1_INST *pir1);
#ifdef CONFIG_SOFTMMU
/* Do cross-page TB-Link check in system mode  */
extern ADDR native_jmp_glue_cpc_0;
extern ADDR native_jmp_glue_cpc_1;
int xtm_tb_need_cpc(TranslationBlock *tb, IR1_INST *jmp, int n);
int generate_jmp_glue_cpc(void *code_buffer, int n);
#endif

/* Generate eflags calculation */
void generate_eflag_calculation(IR2_OPND*, IR2_OPND*, IR2_OPND*, IR1_INST *, bool has_address);

/* Functions for native EFLAGS processing */
void tr_gen_eflags_clr_bit_00(IR2_OPND *eflags, uint32_t flag_mask);
void tr_gen_eflags_clr_bit_10(IR2_OPND *dest, IR2_OPND *eflags, uint32_t flag_mask);

/* Functions to generate call to helper function (mainly in user-mode) */
extern void tr_gen_call_to_helper_prologue(int use_fp);
extern void tr_gen_call_to_helper_epilogue(int use_fp);
extern void tr_gen_call_to_helper0(ADDR func);
extern void tr_gen_call_to_helper1(ADDR func, int use_fp);
extern void tr_gen_call_to_helper2(ADDR func, int arg2);

extern int GPR_USEDEF_TO_SAVE;
extern int FPR_USEDEF_TO_SAVE;
extern int XMM_LO_USEDEF_TO_SAVE;
extern int XMM_HI_USEDEF_TO_SAVE;

void tr_save_registers_to_env(
        uint8_t gpr_to_save,
        uint8_t fpr_to_save, int save_top,
        uint8_t xmm_lo_to_save,
        uint8_t xmm_hi_to_save,
        uint8_t vreg_to_save);

void tr_gen_static_save_registers_to_env(
        uint8_t gpr_to_save,
        uint8_t fpr_to_save,
        uint8_t xmm_lo_to_save,
        uint8_t xmm_hi_to_save,
        uint8_t vreg_to_save);

void tr_save_gprs_to_env(uint8_t mask);
void tr_save_fprs_to_env(uint8_t mask, int save_top);
void tr_save_xmms_to_env(uint8_t lo_mask, uint8_t hi_mask);
void tr_save_vreg_to_env(uint8_t mask);

void tr_gen_save_curr_top(void); /* write curr_top into env->fpstt */
void td_fpu_set_top(int ctop);
int  td_fpu_get_top(void);

/* Loongson LSFPU */
void tr_gen_top_mode_init(void);
void tr_load_lstop_from_env(IR2_OPND *top);
void tr_save_lstop_to_env(IR2_OPND *top);

/* Loongson CAM */
#ifndef _XTM_CAM_CLEAR_FUNC_
#define _XTM_CAM_CLEAR_FUNC_
extern uint64_t cam_clear_func;
#endif
extern ADDR cam_insert_func;
extern ADDR cam_lookup_k2v;
extern ADDR cam_lookup_k2i;
int  cam_enabled(void);
void cam_insert(TranslationBlock *tb);
int generate_native_cam(void *code_buffer);

/* Native Jmp Cache lookup */
extern ADDR njc_lookup_tb;
int generate_native_jmp_cache(void *code_buffer);

void tr_load_registers_from_env(
        uint8_t gpr_to_load,
        uint8_t fpr_to_load, int load_top,
        uint8_t xmm_lo_to_load,
        uint8_t xmm_hi_to_load,
        uint8_t vreg_to_load);

void tr_gen_static_load_registers_from_env(
        uint8_t gpr_to_load,
        uint8_t fpr_to_load,
        uint8_t xmm_lo_to_load,
        uint8_t xmm_hi_to_load,
        uint8_t vreg_to_load);

void tr_load_gprs_from_env(uint8_t mask);
void tr_load_fprs_from_env(uint8_t mask, int load_top);
void tr_load_xmms_from_env(uint8_t lo_mask, uint8_t hi_mask);
void tr_load_vreg_from_env(uint8_t mask);

IR2_INST *tr_gen_call_to_helper(ADDR);

#ifdef CONFIG_SOFTMMU

/* Functions to fit the QEMU's softmmu */
void gen_ldst_softmmu_helper(
        IR2_OPCODE op,
        IR2_OPND *opnd_gpr,
        IR2_OPND *opnd_mem,
        int save_temp);
void softmmu_fixup_em(IR2_OPCODE op, IR2_OPND *opnd_gpr);

void gen_ldst_c1_softmmu_helper(
        IR2_OPCODE op,
        IR2_OPND *opnd_fpr,
        IR2_OPND *opnd_mem,
        int save_temp);

/* Atomic version for softmmu */
void gen_helper_atomic(IR1_OPCODE op, IR2_OPND *opr1, IR2_OPND *opr2, int save_temp);

/* Nested Context Save/Restore */
#define NATIVE_CONTEXT_OUT  0
#define NATIVE_CONTEXT_IN   1
#define NATIVE_CONTEXT_DIFF 2
int context_save_state(helper_cfg_t cfg);

/* Convert IR2_OPND_MEM */
IR2_OPND convert_mem_ir2_opnd_plus_2(IR2_OPND *mem);
IR2_OPND convert_mem_ir2_opnd_plus_4(IR2_OPND *mem);
IR2_OPND convert_mem_ir2_opnd_no_offset(IR2_OPND *mem, int *newtmp);

/* More translation functions in system-mode */
void tr_sys_init(TranslationBlock *tb, int max_insns, void *code_highwater);
int  tb_encode_search(TranslationBlock *tb, uint8 *block);
void tr_gen_eob(void);
void tr_gen_sys_eob(IR1_INST *pir1);

/* Functions to check and generate exception */
int tr_gen_excp_check(IR1_INST *pir1);
int tr_gen_fp_common_excp_check(IR1_INST *pir1);
int tr_gen_sse_common_excp_check(IR1_INST *pir1);

void tr_gen_excp_illegal_op(IR1_INST *ir1, int end);
void tr_gen_excp_gp(IR1_INST *ir1, int end);
void tr_gen_excp_prex(IR1_INST *ir1, int end);
void tr_gen_excp_divz(IR1_INST *ir1, int end);

void tr_gen_infinite_loop(void);

/* Functions to generate call to helper functgion in system-mode */
extern helper_cfg_t all_helper_cfg;
extern helper_cfg_t zero_helper_cfg;
extern helper_cfg_t default_helper_cfg;
int cmp_helper_cfg(helper_cfg_t cfg1, helper_cfg_t cfg2);

void tr_sys_gen_call_to_helper_prologue_cfg(helper_cfg_t cfg);
void tr_sys_gen_call_to_helper_epilogue_cfg(helper_cfg_t cfg);
void tr_sys_gen_call_to_helper0_cfg(ADDR func, helper_cfg_t cfg);
void tr_sys_gen_call_to_helper1_cfg(ADDR func, helper_cfg_t cfg);
void tr_sys_gen_call_to_helper2_cfg(ADDR func, int arg, helper_cfg_t cfg);
void tr_sys_gen_call_to_helper3_cfg(ADDR func, int arg2, int arg3, helper_cfg_t cfg);
void tr_sys_gen_call_to_helper3_u64_cfg(ADDR func, uint64 arg2, uint64 arg3, helper_cfg_t cfg);
void tr_sys_gen_call_to_helper4_u64_cfg(ADDR func, uint64 arg2, uint64 arg3, uint64 arg4, helper_cfg_t cfg);

int  staticcs_enabled(void);
void generate_static_sys_helper_prologue_cfg(helper_cfg_t cfg);
void generate_static_sys_helper_epilogue_cfg(helper_cfg_t cfg);
void tr_gen_static_save_eflags(void);
void tr_gen_static_load_eflags(int simple);

void tr_cvt_fp64_to_80(void);
void tr_cvt_fp80_to_64(void);

void tr_save_eflags(void);
void tr_load_eflags(int simple);

void tr_save_temp_register(void);
void tr_restore_temp_register(void);
void tr_save_temp_register_mask(int mask);
void tr_restore_temp_register_mask(int mask);

void tr_sys_gen_call_to_helper_prologue(int all, int eflags);
void tr_sys_gen_call_to_helper_epilogue(int all, int eflags);
void tr_sys_gen_call_to_helper0(ADDR func, int all, int eflags);
void tr_sys_gen_call_to_helper1(ADDR func, int all, int eflags);
void tr_sys_gen_call_to_helper2(ADDR func, int arg, int all, int eflags);
void tr_sys_gen_call_to_helper3(ADDR func, int arg2, int arg3, int all, int eflags);
void tr_sys_gen_call_to_helper3_u64(ADDR func, uint64 arg2, uint64 arg3, int all, int eflags);
void tr_sys_gen_call_to_helper4_u64(ADDR func, uint64 arg2, uint64 arg3, uint64 arg4, int all_gpr, int eflags);

/* These are used for icount decrease */
void tr_gen_tb_start(void);
void tr_gen_tb_end(void);

/* These are used for IO instructions
 * > translate_in/out
 * > translate_ins/outs
 *
 * io_check(): checking the validation of IO instruction
 * io_bpt(): checking the CPU breakpoint
 * io_strat(): set CPUState.can_do_io = 1
 * io_end()  : set CPUState.can_do_io = 0 */
void tr_gen_io_check(IR1_INST *ir1, IR1_OPND *io, int data_size);
void tr_gen_io_bpt(IR1_INST *ir1, IR1_OPND *port, int size);
void tr_gen_io_start(void);
void tr_gen_io_end(void);

/* Functions to simply the translation */
void tr_gen_save_curr_eip(void);
void tr_gen_save_next_eip(void);
void tr_gen_fwait(void);

/* Special functions needed in system-mode */
int get_sys_stack_addr_size(void);

#endif

#endif
