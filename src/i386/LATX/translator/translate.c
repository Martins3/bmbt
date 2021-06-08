#include "common.h"
#include "env.h"
#include "etb.h"
#include "ir2/ir2.h"
#include "ir1/ir1.h"
#include "reg_alloc.h"

#include "flag_lbt.h"
#include "x86tomips-options.h"
#include "fpu/softfloat.h"

#include "ibtc.h"
#include "profile.h"
#include "flag_pattern.h"
#include "shadow_stack.h"

#ifdef CONFIG_SOFTMMU
#include "debug/lockstep.h"
#  ifdef CONFIG_XTM_PROFILE
#  include "x86tomips-profile-sys.h"
#  endif
#endif

#include <string.h>

#ifdef CONFIG_SOFTMMU
extern int debug_tb;
extern unsigned long debug_pc_start;
extern unsigned long debug_pc_stop;
#endif

bool ir1_translate(IR1_INST *ir1)
{
    static bool (*translate_functions[])(IR1_INST *) = {
        //implemented , but not in X86_INS_...        
        //lods,pusha,finit,popa,fstenv,xlat,fclex,movs,jmp_far,fsave,call_far,fstcw,
        translate_invalid,   //X86_INS_INVALID = 0,
        translate_aaa,       //X86_INS_AAA,
        translate_aad,       //X86_INS_AAD,
        translate_aam,       //X86_INS_AAM,
        translate_aas,       //X86_INS_AAS,
        translate_fabs,      //X86_INS_FABS,
        translate_adc,       //X86_INS_ADC,
        NULL,                //X86_INS_ADCX,
        translate_add,       //X86_INS_ADD,
        translate_addpd,     //X86_INS_ADDPD,
        translate_addps,     //X86_INS_ADDPS,
        translate_addsd,     //X86_INS_ADDSD,
        translate_addss,     //X86_INS_ADDSS,
        translate_addsubpd,  //X86_INS_ADDSUBPD,
        translate_addsubps,  //X86_INS_ADDSUBPS,
        translate_fadd,      //X86_INS_FADD,
        translate_fiadd,     //X86_INS_FIADD,
        translate_faddp,     //X86_INS_FADDP,
        NULL,                //X86_INS_ADOX,
        translate_aesdeclast,//X86_INS_AESDECLAST,
        translate_aesdec,    //X86_INS_AESDEC,
        translate_aesenclast,//X86_INS_AESENCLAST,
        translate_aesenc,    //X86_INS_AESENC,
        translate_aesimc,    //X86_INS_AESIMC,
        translate_aeskeygenassist,//X86_INS_AESKEYGENASSIST,
        translate_and,       //X86_INS_AND,
        NULL,                //X86_INS_ANDN,
        translate_andnpd,    //X86_INS_ANDNPD,
        translate_andnps,    //X86_INS_ANDNPS,
        translate_andpd,     //X86_INS_ANDPD,
        translate_andps,     //X86_INS_ANDPS,
        translate_arpl,      //X86_INS_ARPL,
        NULL,                //X86_INS_BEXTR,
        NULL,                //X86_INS_BLCFILL,
        NULL,                //X86_INS_BLCI,
        NULL,                //X86_INS_BLCIC,
        NULL,                //X86_INS_BLCMSK,
        NULL,                //X86_INS_BLCS,
        translate_blendpd,   //X86_INS_BLENDPD,
        translate_blendps,   //X86_INS_BLENDPS,
        translate_blendvpd,  //X86_INS_BLENDVPD,
        translate_blendvps,  //X86_INS_BLENDVPS,
        NULL,                //X86_INS_BLSFILL,
        NULL,                //X86_INS_BLSI,
        NULL,                //X86_INS_BLSIC,
        NULL,                //X86_INS_BLSMSK,
        NULL,                //X86_INS_BLSR,
        translate_bound,     //X86_INS_BOUND,
        translate_bsf,       //X86_INS_BSF,
        translate_bsr,       //X86_INS_BSR,
        translate_bswap,     //X86_INS_BSWAP,
        translate_btx,       //X86_INS_BT,
        translate_btx,       //X86_INS_BTC,
        translate_btx,       //X86_INS_BTR,
        translate_btx,       //X86_INS_BTS,
        NULL,                //X86_INS_BZHI,
        translate_call,      //X86_INS_CALL,
        translate_cbw,       //X86_INS_CBW,
        translate_cdq,       //X86_INS_CDQ,
        translate_cdqe,      //X86_INS_CDQE,
        translate_fchs,      //X86_INS_FCHS,
        translate_clac,      //X86_INS_CLAC,
        translate_clc,       //X86_INS_CLC,
        translate_cld,       //X86_INS_CLD,
        translate_clflush,   //X86_INS_CLFLUSH,
        NULL,                //X86_INS_CLFLUSHOPT,
        translate_clgi,      //X86_INS_CLGI,
        translate_cli,       //X86_INS_CLI,
        translate_clts,      //X86_INS_CLTS,
        NULL,                //X86_INS_CLWB,
        translate_cmc,       //X86_INS_CMC,
        translate_cmova,     //X86_INS_CMOVA,
        translate_cmovae,    //X86_INS_CMOVAE,
        translate_cmovb,     //X86_INS_CMOVB,
        translate_cmovbe,    //X86_INS_CMOVBE,
        translate_fcmovbe,   //X86_INS_FCMOVBE,
        translate_fcmovb,    //X86_INS_FCMOVB,
        translate_cmovz,     //X86_INS_CMOVE,
        translate_fcmove,    //X86_INS_FCMOVE,
        translate_cmovg,     //X86_INS_CMOVG,
        translate_cmovge,    //X86_INS_CMOVGE,
        translate_cmovl,     //X86_INS_CMOVL,
        translate_cmovle,    //X86_INS_CMOVLE,
        translate_fcmovnbe,  //X86_INS_FCMOVNBE,
        translate_fcmovnb,   //X86_INS_FCMOVNB,
        translate_cmovnz,    //X86_INS_CMOVNE,
        translate_fcmovne,   //X86_INS_FCMOVNE,
        translate_cmovno,    //X86_INS_CMOVNO,
        translate_cmovnp,    //X86_INS_CMOVNP,
        translate_fcmovnu,   //X86_INS_FCMOVNU,
        translate_cmovns,    //X86_INS_CMOVNS,
        translate_cmovo,     //X86_INS_CMOVO,
        translate_cmovp,     //X86_INS_CMOVP,
        translate_fcmovu,    //X86_INS_FCMOVU,
        translate_cmovs,     //X86_INS_CMOVS,
        translate_cmp,       //X86_INS_CMP,
        translate_cmps,      //X86_INS_CMPSB,
        NULL,                //X86_INS_CMPSQ,
        translate_cmps,                //X86_INS_CMPSW,
        translate_cmpxchg16b,//X86_INS_CMPXCHG16B,
        translate_cmpxchg,   //X86_INS_CMPXCHG,
        translate_cmpxchg8b, //X86_INS_CMPXCHG8B,
        translate_comisd,    //X86_INS_COMISD,
        translate_comiss,    //X86_INS_COMISS,
        translate_fcomp,     //X86_INS_FCOMP,
        translate_fcomip,    //X86_INS_FCOMIP,
        translate_fcomi,     //X86_INS_FCOMI,
        translate_fcom,      //X86_INS_FCOM,
        translate_fcos,      //X86_INS_FCOS,
        translate_cpuid,     //X86_INS_CPUID,
        translate_cqo,       //X86_INS_CQO,
        translate_crc32,     //X86_INS_CRC32,
        translate_cvtdq2pd,  //X86_INS_CVTDQ2PD,
        translate_cvtdq2ps,  //X86_INS_CVTDQ2PS,
        translate_cvtpd2dq,  //X86_INS_CVTPD2DQ,
        translate_cvtpd2ps,  //X86_INS_CVTPD2PS,
        translate_cvtps2dq,  //X86_INS_CVTPS2DQ,
        translate_cvtps2pd,  //X86_INS_CVTPS2PD,
        translate_cvtsd2si,  //X86_INS_CVTSD2SI,
        translate_cvtsd2ss,  //X86_INS_CVTSD2SS,
        translate_cvtsi2sd,  //X86_INS_CVTSI2SD,
        translate_cvtsi2ss,  //X86_INS_CVTSI2SS,
        translate_cvtss2sd,  //X86_INS_CVTSS2SD,
        translate_cvtss2si,  //X86_INS_CVTSS2SI,
        translate_cvttpd2dq, //X86_INS_CVTTPD2DQ,
        translate_cvttps2dq, //X86_INS_CVTTPS2DQ,
        translate_cvttsd2si, //X86_INS_CVTTSD2SI,
        translate_cvttss2si, //X86_INS_CVTTSS2SI,
        translate_cwd,       //X86_INS_CWD,
        translate_cwde,      //X86_INS_CWDE,
        translate_daa,       //X86_INS_DAA,
        translate_das,       //X86_INS_DAS,
        NULL,                //X86_INS_DATA16,
        translate_dec,       //X86_INS_DEC,
        translate_div,       //X86_INS_DIV,
        translate_divpd,     //X86_INS_DIVPD,
        translate_divps,     //X86_INS_DIVPS,
        translate_fdivr,     //X86_INS_FDIVR,
        translate_fidivr,    //X86_INS_FIDIVR,
        translate_fdivrp,    //X86_INS_FDIVRP,
        translate_divsd,     //X86_INS_DIVSD,
        translate_divss,     //X86_INS_DIVSS,
        translate_fdiv,      //X86_INS_FDIV,
        translate_fidiv,     //X86_INS_FIDIV,
        translate_fdivp,     //X86_INS_FDIVP,
        translate_dppd,      //X86_INS_DPPD,
        translate_dpps,      //X86_INS_DPPS,
        translate_ret,       //X86_INS_RET,
        NULL,                //X86_INS_ENCLS,
        NULL,                //X86_INS_ENCLU,
        translate_enter,     //X86_INS_ENTER,
        translate_extractps, //X86_INS_EXTRACTPS,
        translate_extrq,     //X86_INS_EXTRQ,
        translate_f2xm1,     //X86_INS_F2XM1,
        translate_call_far,  //X86_INS_LCALL,
        translate_jmp_far,   //X86_INS_LJMP,
        translate_fbld,      //X86_INS_FBLD,
        translate_fbstp,     //X86_INS_FBSTP,
        translate_fcompp,    //X86_INS_FCOMPP,
        translate_fdecstp,   //X86_INS_FDECSTP,
        translate_femms,     //X86_INS_FEMMS,
        translate_ffree,     //X86_INS_FFREE,
        translate_ficom,     //X86_INS_FICOM,
        translate_ficomp,    //X86_INS_FICOMP,
        translate_fincstp,   //X86_INS_FINCSTP,
        translate_fldcw,     //X86_INS_FLDCW,
        translate_fldenv,    //X86_INS_FLDENV,
        translate_fldl2e,    //X86_INS_FLDL2E,
        translate_fldl2t,    //X86_INS_FLDL2T,
        translate_fldlg2,    //X86_INS_FLDLG2,
        translate_fldln2,    //X86_INS_FLDLN2,
        translate_fldpi,     //X86_INS_FLDPI,
        translate_fnclex,    //X86_INS_FNCLEX,
        translate_fninit,    //X86_INS_FNINIT,
        translate_fnop,      //X86_INS_FNOP,
        translate_fnstcw,    //X86_INS_FNSTCW,
        translate_fnstsw,    //X86_INS_FNSTSW,
        translate_fpatan,    //X86_INS_FPATAN,
        translate_fprem,     //X86_INS_FPREM,
        translate_fprem1,    //X86_INS_FPREM1,
        translate_fptan,     //X86_INS_FPTAN,
        NULL,                //X86_INS_FFREEP,
        translate_frndint,   //X86_INS_FRNDINT,
        translate_frstor,    //X86_INS_FRSTOR,
        translate_fnsave,    //X86_INS_FNSAVE,
        translate_fscale,    //X86_INS_FSCALE,
        translate_fsetpm,    //X86_INS_FSETPM,
        translate_fsincos,   //X86_INS_FSINCOS,
        translate_fnstenv,   //X86_INS_FNSTENV,
        translate_fxam,      //X86_INS_FXAM,
        translate_fxrstor,   //X86_INS_FXRSTOR,
        NULL,                //X86_INS_FXRSTOR64,
        translate_fxsave,    //X86_INS_FXSAVE,
        NULL,                //X86_INS_FXSAVE64,
        translate_fxtract,   //X86_INS_FXTRACT,
        translate_fyl2x,     //X86_INS_FYL2X,
        translate_fyl2xp1,   //X86_INS_FYL2XP1,
        translate_movapd,    //X86_INS_MOVAPD,
        translate_movaps,    //X86_INS_MOVAPS,
        translate_orpd,      //X86_INS_ORPD,
        translate_orps,      //X86_INS_ORPS,
        translate_vmovapd,   //X86_INS_VMOVAPD,
        translate_vmovaps,   //X86_INS_VMOVAPS,
        translate_xorpd,     //X86_INS_XORPD,
        translate_xorps,     //X86_INS_XORPS,
        translate_getsec,    //X86_INS_GETSEC,
        translate_haddpd,    //X86_INS_HADDPD,
        translate_haddps,    //X86_INS_HADDPS,
        translate_hlt,       //X86_INS_HLT,
        translate_hsubpd,    //X86_INS_HSUBPD,
        translate_hsubps,    //X86_INS_HSUBPS,
        translate_idiv,      //X86_INS_IDIV,
        translate_fild,      //X86_INS_FILD,
        translate_imul,      //X86_INS_IMUL,
        translate_in,        //X86_INS_IN,
        translate_inc,       //X86_INS_INC,
        translate_ins,       //X86_INS_INSB,
        translate_insertps,  //X86_INS_INSERTPS,
        translate_insertq,   //X86_INS_INSERTQ,
        translate_ins,       //X86_INS_INSD,
        translate_ins,       //X86_INS_INSW,
        translate_int,       //X86_INS_INT,
        translate_int1,      //X86_INS_INT1,
        translate_int_3,     //X86_INS_INT3,
        translate_into,      //X86_INS_INTO,
        translate_invd,      //X86_INS_INVD,
        translate_invept,    //X86_INS_INVEPT,
        translate_invlpg,    //X86_INS_INVLPG,
        translate_invlpga,   //X86_INS_INVLPGA,
        NULL,                //X86_INS_INVPCID,
        translate_invvpid,   //X86_INS_INVVPID,
        translate_iret,      //X86_INS_IRET,
        translate_iretd,     //X86_INS_IRETD,
        NULL,                //X86_INS_IRETQ,
        translate_fisttp,    //X86_INS_FISTTP,
        translate_fist,      //X86_INS_FIST,
        translate_fistp,     //X86_INS_FISTP,
        translate_ucomisd,   //X86_INS_UCOMISD,
        translate_ucomiss,   //X86_INS_UCOMISS,
        translate_vcomisd,   //X86_INS_VCOMISD,
        translate_vcomiss,   //X86_INS_VCOMISS,
        translate_vcvtsd2ss, //X86_INS_VCVTSD2SS,
        translate_vcvtsi2sd, //X86_INS_VCVTSI2SD,
        translate_vcvtsi2ss, //X86_INS_VCVTSI2SS,
        translate_vcvtss2sd, //X86_INS_VCVTSS2SD,
        translate_vcvttsd2si,//X86_INS_VCVTTSD2SI,
        NULL,                //X86_INS_VCVTTSD2USI,
        translate_vcvttss2si,//X86_INS_VCVTTSS2SI,
        NULL,                //X86_INS_VCVTTSS2USI,
        NULL,                //X86_INS_VCVTUSI2SD,
        NULL,                //X86_INS_VCVTUSI2SS,
        translate_vucomisd,  //X86_INS_VUCOMISD,
        translate_vucomiss,  //X86_INS_VUCOMISS,
        translate_jae,       //X86_INS_JAE,
        translate_ja,        //X86_INS_JA,
        translate_jbe,       //X86_INS_JBE,
        translate_jb,        //X86_INS_JB,
        translate_jcxz,      //X86_INS_JCXZ,
        translate_jecxz,     //X86_INS_JECXZ,
        translate_jz,                //X86_INS_JE,
        translate_jge,       //X86_INS_JGE,
        translate_jg,        //X86_INS_JG,
        translate_jle,       //X86_INS_JLE,
        translate_jl,        //X86_INS_JL,
        translate_jmp,       //X86_INS_JMP,
        translate_jnz,                //X86_INS_JNE,
        translate_jno,       //X86_INS_JNO,
        translate_jnp,       //X86_INS_JNP,
        translate_jns,       //X86_INS_JNS,
        translate_jo,        //X86_INS_JO,
        translate_jp,        //X86_INS_JP,
        translate_jrcxz,     //X86_INS_JRCXZ,
        translate_js,        //X86_INS_JS,
        NULL,                //X86_INS_KANDB,
        NULL,                //X86_INS_KANDD,
        NULL,                //X86_INS_KANDNB,
        NULL,                //X86_INS_KANDND,
        NULL,                //X86_INS_KANDNQ,
        NULL,                //X86_INS_KANDNW,
        NULL,                //X86_INS_KANDQ,
        NULL,                //X86_INS_KANDW,
        NULL,                //X86_INS_KMOVB,
        NULL,                //X86_INS_KMOVD,
        NULL,                //X86_INS_KMOVQ,
        NULL,                //X86_INS_KMOVW,
        NULL,                //X86_INS_KNOTB,
        NULL,                //X86_INS_KNOTD,
        NULL,                //X86_INS_KNOTQ,
        NULL,                //X86_INS_KNOTW,
        NULL,                //X86_INS_KORB,
        NULL,                //X86_INS_KORD,
        NULL,                //X86_INS_KORQ,
        NULL,                //X86_INS_KORTESTB,
        NULL,                //X86_INS_KORTESTD,
        NULL,                //X86_INS_KORTESTQ,
        NULL,                //X86_INS_KORTESTW,
        NULL,                //X86_INS_KORW,
        NULL,                //X86_INS_KSHIFTLB,
        NULL,                //X86_INS_KSHIFTLD,
        NULL,                //X86_INS_KSHIFTLQ,
        NULL,                //X86_INS_KSHIFTLW,
        NULL,                //X86_INS_KSHIFTRB,
        NULL,                //X86_INS_KSHIFTRD,
        NULL,                //X86_INS_KSHIFTRQ,
        NULL,                //X86_INS_KSHIFTRW,
        NULL,                //X86_INS_KUNPCKBW,
        NULL,                //X86_INS_KXNORB,
        NULL,                //X86_INS_KXNORD,
        NULL,                //X86_INS_KXNORQ,
        NULL,                //X86_INS_KXNORW,
        NULL,                //X86_INS_KXORB,
        NULL,                //X86_INS_KXORD,
        NULL,                //X86_INS_KXORQ,
        NULL,                //X86_INS_KXORW,
        translate_lahf,      //X86_INS_LAHF,
        translate_lar,       //X86_INS_LAR,
        translate_lddqu,     //X86_INS_LDDQU,
        translate_ldmxcsr,   //X86_INS_LDMXCSR,
        translate_lds,       //X86_INS_LDS,
        translate_fldz,      //X86_INS_FLDZ,
        translate_fld1,      //X86_INS_FLD1,
        translate_fld,       //X86_INS_FLD,
        translate_lea,       //X86_INS_LEA,
        translate_leave,     //X86_INS_LEAVE,
        translate_les,       //X86_INS_LES,
        translate_lfence,    //X86_INS_LFENCE,
        translate_lfs,       //X86_INS_LFS,
        translate_lgdt,      //X86_INS_LGDT,
        translate_lgs,       //X86_INS_LGS,
        translate_lidt,      //X86_INS_LIDT,
        translate_lldt,      //X86_INS_LLDT,
        translate_lmsw,      //X86_INS_LMSW,
        translate_or,        //X86_INS_OR,
        translate_sub,       //X86_INS_SUB,
        translate_xor,       //X86_INS_XOR,
        translate_lods,                //X86_INS_LODSB,
        translate_lods,                //X86_INS_LODSD,
        NULL,                //X86_INS_LODSQ,
        translate_lods,                //X86_INS_LODSW,
        translate_loop,      //X86_INS_LOOP,
        translate_loopz,     //X86_INS_LOOPE,
        translate_loopnz,    //X86_INS_LOOPNE,
        translate_retf,      //X86_INS_RETF,
        NULL,                //X86_INS_RETFQ,
        translate_lsl,       //X86_INS_LSL,
        translate_lss,       //X86_INS_LSS,
        translate_ltr,       //X86_INS_LTR,
        translate_xadd,      //X86_INS_XADD,
        translate_lzcnt,     //X86_INS_LZCNT,
        translate_maskmovdqu,//X86_INS_MASKMOVDQU,
        translate_maxpd,     //X86_INS_MAXPD,
        translate_maxps,     //X86_INS_MAXPS,
        translate_maxsd,     //X86_INS_MAXSD,
        translate_maxss,     //X86_INS_MAXSS,
        translate_mfence,    //X86_INS_MFENCE,
        translate_minpd,     //X86_INS_MINPD,
        translate_minps,     //X86_INS_MINPS,
        translate_minsd,     //X86_INS_MINSD,
        translate_minss,     //X86_INS_MINSS,
        translate_cvtpd2pi,  //X86_INS_CVTPD2PI,
        translate_cvtpi2pd,  //X86_INS_CVTPI2PD,
        translate_cvtpi2ps,  //X86_INS_CVTPI2PS,
        translate_cvtps2pi,  //X86_INS_CVTPS2PI,
        translate_cvttpd2pi, //X86_INS_CVTTPD2PI,
        translate_cvttps2pi, //X86_INS_CVTTPS2PI,
        translate_emms,      //X86_INS_EMMS,
        translate_maskmovq,  //X86_INS_MASKMOVQ,
        translate_movd,      //X86_INS_MOVD,
        translate_movdq2q,   //X86_INS_MOVDQ2Q,
        translate_movntq,    //X86_INS_MOVNTQ,
        translate_movq2dq,   //X86_INS_MOVQ2DQ,
        translate_movq,      //X86_INS_MOVQ,
        translate_pabsb,     //X86_INS_PABSB,
        translate_pabsd,     //X86_INS_PABSD,
        translate_pabsw,     //X86_INS_PABSW,
        translate_packssdw,  //X86_INS_PACKSSDW,
        translate_packsswb,  //X86_INS_PACKSSWB,
        translate_packuswb,  //X86_INS_PACKUSWB,
        translate_paddb,     //X86_INS_PADDB,
        translate_paddd,     //X86_INS_PADDD,
        translate_paddq,     //X86_INS_PADDQ,
        translate_paddsb,    //X86_INS_PADDSB,
        translate_paddsw,    //X86_INS_PADDSW,
        translate_paddusb,   //X86_INS_PADDUSB,
        translate_paddusw,   //X86_INS_PADDUSW,
        translate_paddw,     //X86_INS_PADDW,
        translate_palignr,   //X86_INS_PALIGNR,
        translate_pandn,     //X86_INS_PANDN,
        translate_pand,      //X86_INS_PAND,
        translate_pavgb,     //X86_INS_PAVGB,
        translate_pavgw,     //X86_INS_PAVGW,
        translate_pcmpeqb,   //X86_INS_PCMPEQB,
        translate_pcmpeqd,   //X86_INS_PCMPEQD,
        translate_pcmpeqw,   //X86_INS_PCMPEQW,
        translate_pcmpgtb,   //X86_INS_PCMPGTB,
        translate_pcmpgtd,   //X86_INS_PCMPGTD,
        translate_pcmpgtw,   //X86_INS_PCMPGTW,
        translate_pextrw,    //X86_INS_PEXTRW,
        translate_phaddsw,   //X86_INS_PHADDSW,
        translate_phaddw,    //X86_INS_PHADDW,
        translate_phaddd,    //X86_INS_PHADDD,
        translate_phsubd,    //X86_INS_PHSUBD,
        translate_phsubsw,   //X86_INS_PHSUBSW,
        translate_phsubw,    //X86_INS_PHSUBW,
        translate_pinsrw,    //X86_INS_PINSRW,
        translate_pmaddubsw, //X86_INS_PMADDUBSW,
        translate_pmaddwd,   //X86_INS_PMADDWD,
        translate_pmaxsw,    //X86_INS_PMAXSW,
        translate_pmaxub,    //X86_INS_PMAXUB,
        translate_pminsw,    //X86_INS_PMINSW,
        translate_pminub,    //X86_INS_PMINUB,
        translate_pmovmskb,  //X86_INS_PMOVMSKB,
        translate_pmulhrsw,  //X86_INS_PMULHRSW,
        translate_pmulhuw,   //X86_INS_PMULHUW,
        translate_pmulhw,    //X86_INS_PMULHW,
        translate_pmullw,    //X86_INS_PMULLW,
        translate_pmuludq,   //X86_INS_PMULUDQ,
        translate_por,       //X86_INS_POR,
        translate_psadbw,    //X86_INS_PSADBW,
        translate_pshufb,    //X86_INS_PSHUFB,
        translate_pshufw,    //X86_INS_PSHUFW,
        translate_psignb,    //X86_INS_PSIGNB,
        translate_psignd,    //X86_INS_PSIGND,
        translate_psignw,    //X86_INS_PSIGNW,
        translate_pslld,     //X86_INS_PSLLD,
        translate_psllq,     //X86_INS_PSLLQ,
        translate_psllw,     //X86_INS_PSLLW,
        translate_psrad,     //X86_INS_PSRAD,
        translate_psraw,     //X86_INS_PSRAW,
        translate_psrld,     //X86_INS_PSRLD,
        translate_psrlq,     //X86_INS_PSRLQ,
        translate_psrlw,     //X86_INS_PSRLW,
        translate_psubb,     //X86_INS_PSUBB,
        translate_psubd,     //X86_INS_PSUBD,
        translate_psubq,     //X86_INS_PSUBQ,
        translate_psubsb,    //X86_INS_PSUBSB,
        translate_psubsw,    //X86_INS_PSUBSW,
        translate_psubusb,   //X86_INS_PSUBUSB,
        translate_psubusw,   //X86_INS_PSUBUSW,
        translate_psubw,     //X86_INS_PSUBW,
        translate_punpckhbw, //X86_INS_PUNPCKHBW,
        translate_punpckhdq, //X86_INS_PUNPCKHDQ,
        translate_punpckhwd, //X86_INS_PUNPCKHWD,
        translate_punpcklbw, //X86_INS_PUNPCKLBW,
        translate_punpckldq, //X86_INS_PUNPCKLDQ,
        translate_punpcklwd, //X86_INS_PUNPCKLWD,
        translate_pxor,      //X86_INS_PXOR,
        translate_monitor,   //X86_INS_MONITOR,
        NULL,                //X86_INS_MONTMUL,
        translate_mov,       //X86_INS_MOV,
        NULL,                //X86_INS_MOVABS,
        translate_movbe,     //X86_INS_MOVBE,
        translate_movddup,   //X86_INS_MOVDDUP,
        translate_movdqa,    //X86_INS_MOVDQA,
        translate_movdqu,    //X86_INS_MOVDQU,
        translate_movhlps,   //X86_INS_MOVHLPS,
        translate_movhpd,    //X86_INS_MOVHPD,
        translate_movhps,    //X86_INS_MOVHPS,
        translate_movlhps,   //X86_INS_MOVLHPS,
        translate_movlpd,    //X86_INS_MOVLPD,
        translate_movlps,    //X86_INS_MOVLPS,
        translate_movmskpd,  //X86_INS_MOVMSKPD,
        translate_movmskps,  //X86_INS_MOVMSKPS,
        translate_movntdqa,  //X86_INS_MOVNTDQA,
        translate_movntdq,   //X86_INS_MOVNTDQ,
        translate_movnti,    //X86_INS_MOVNTI,
        translate_movntpd,   //X86_INS_MOVNTPD,
        translate_movntps,   //X86_INS_MOVNTPS,
        translate_movntsd,   //X86_INS_MOVNTSD,
        translate_movntss,   //X86_INS_MOVNTSS,
        translate_movs,      //X86_INS_MOVSB,
        translate_movsd,     //X86_INS_MOVSD, movsd(movs) or movsd(sse2)
        translate_movshdup,  //X86_INS_MOVSHDUP,
        translate_movsldup,  //X86_INS_MOVSLDUP,
        NULL,                //X86_INS_MOVSQ,
        translate_movss,     //X86_INS_MOVSS,
        translate_movs,                //X86_INS_MOVSW,
        translate_movsx,     //X86_INS_MOVSX,
        translate_movsxd,    //X86_INS_MOVSXD,
        translate_movupd,    //X86_INS_MOVUPD,
        translate_movups,    //X86_INS_MOVUPS,
        translate_movzx,     //X86_INS_MOVZX,
        translate_mpsadbw,   //X86_INS_MPSADBW,
        translate_mul,       //X86_INS_MUL,
        translate_mulpd,     //X86_INS_MULPD,
        translate_mulps,     //X86_INS_MULPS,
        translate_mulsd,     //X86_INS_MULSD,
        translate_mulss,     //X86_INS_MULSS,
        NULL,                //X86_INS_MULX,
        translate_fmul,      //X86_INS_FMUL,
        translate_fimul,     //X86_INS_FIMUL,
        translate_fmulp,     //X86_INS_FMULP,
        translate_mwait,     //X86_INS_MWAIT,
        translate_neg,       //X86_INS_NEG,
        translate_nop,       //X86_INS_NOP,
        translate_not,       //X86_INS_NOT,
        translate_out,       //X86_INS_OUT,
        translate_outs,      //X86_INS_OUTSB,
        translate_outs,      //X86_INS_OUTSD,
        translate_outs,      //X86_INS_OUTSW,
        translate_packusdw,  //X86_INS_PACKUSDW,
        translate_pause,     //X86_INS_PAUSE,
        translate_pavgusb,   //X86_INS_PAVGUSB,
        translate_pblendvb,  //X86_INS_PBLENDVB,
        translate_pblendw,   //X86_INS_PBLENDW,
        translate_pclmulqdq, //X86_INS_PCLMULQDQ,
        translate_pcmpeqq,   //X86_INS_PCMPEQQ,
        translate_pcmpestri, //X86_INS_PCMPESTRI,
        translate_pcmpestrm, //X86_INS_PCMPESTRM,
        translate_pcmpgtq,   //X86_INS_PCMPGTQ,
        translate_pcmpistri, //X86_INS_PCMPISTRI,
        translate_pcmpistrm, //X86_INS_PCMPISTRM,
        NULL,                //X86_INS_PCOMMIT,
        NULL,                //X86_INS_PDEP,
        NULL,                //X86_INS_PEXT,
        translate_pextrb,    //X86_INS_PEXTRB,
        translate_pextrd,    //X86_INS_PEXTRD,
        translate_pextrq,    //X86_INS_PEXTRQ,
        translate_pf2id,     //X86_INS_PF2ID,
        translate_pf2iw,     //X86_INS_PF2IW,
        translate_pfacc,     //X86_INS_PFACC,
        translate_pfadd,     //X86_INS_PFADD,
        translate_pfcmpeq,   //X86_INS_PFCMPEQ,
        translate_pfcmpge,   //X86_INS_PFCMPGE,
        translate_pfcmpgt,   //X86_INS_PFCMPGT,
        translate_pfmax,     //X86_INS_PFMAX,
        translate_pfmin,     //X86_INS_PFMIN,
        translate_pfmul,     //X86_INS_PFMUL,
        translate_pfnacc,    //X86_INS_PFNACC,
        translate_pfpnacc,   //X86_INS_PFPNACC,
        translate_pfrcpit1,  //X86_INS_PFRCPIT1,
        translate_pfrcpit2,  //X86_INS_PFRCPIT2,
        translate_pfrcp,     //X86_INS_PFRCP,
        translate_pfrsqit1,  //X86_INS_PFRSQIT1,
        translate_pfrsqrt,   //X86_INS_PFRSQRT,
        translate_pfsubr,    //X86_INS_PFSUBR,
        translate_pfsub,     //X86_INS_PFSUB,
        translate_phminposuw,//X86_INS_PHMINPOSUW,
        translate_pi2fd,     //X86_INS_PI2FD,
        translate_pi2fw,     //X86_INS_PI2FW,
        translate_pinsrb,    //X86_INS_PINSRB,
        translate_pinsrd,    //X86_INS_PINSRD,
        translate_pinsrq,    //X86_INS_PINSRQ,
        translate_pmaxsb,    //X86_INS_PMAXSB,
        translate_pmaxsd,    //X86_INS_PMAXSD,
        translate_pmaxud,    //X86_INS_PMAXUD,
        translate_pmaxuw,    //X86_INS_PMAXUW,
        translate_pminsb,    //X86_INS_PMINSB,
        translate_pminsd,    //X86_INS_PMINSD,
        translate_pminud,    //X86_INS_PMINUD,
        translate_pminuw,    //X86_INS_PMINUW,
        translate_pmovsxbd,  //X86_INS_PMOVSXBD,
        translate_pmovsxbq,  //X86_INS_PMOVSXBQ,
        translate_pmovsxbw,  //X86_INS_PMOVSXBW,
        translate_pmovsxdq,  //X86_INS_PMOVSXDQ,
        translate_pmovsxwd,  //X86_INS_PMOVSXWD,
        translate_pmovsxwq,  //X86_INS_PMOVSXWQ,
        translate_pmovzxbd,  //X86_INS_PMOVZXBD,
        translate_pmovzxbq,  //X86_INS_PMOVZXBQ,
        translate_pmovzxbw,  //X86_INS_PMOVZXBW,
        translate_pmovzxdq,  //X86_INS_PMOVZXDQ,
        translate_pmovzxwd,  //X86_INS_PMOVZXWD,
        translate_pmovzxwq,  //X86_INS_PMOVZXWQ,
        translate_pmuldq,    //X86_INS_PMULDQ,
        translate_pmulhrw,   //X86_INS_PMULHRW,
        translate_pmulld,    //X86_INS_PMULLD,
        translate_pop,       //X86_INS_POP,
        translate_popaw,     //X86_INS_POPAW,
        translate_popal,     //X86_INS_POPAL,
        translate_popcnt,    //X86_INS_POPCNT,
        translate_popf,      //X86_INS_POPF,
        translate_popf,                //X86_INS_POPFD,
        NULL,                //X86_INS_POPFQ,
        translate_prefetch,  //X86_INS_PREFETCH,
        translate_prefetchnta,//X86_INS_PREFETCHNTA,
        translate_prefetcht0,//X86_INS_PREFETCHT0,
        translate_prefetcht1,//X86_INS_PREFETCHT1,
        translate_prefetcht2,//X86_INS_PREFETCHT2,
        translate_prefetchw, //X86_INS_PREFETCHW,
        translate_pshufd,    //X86_INS_PSHUFD,
        translate_pshufhw,   //X86_INS_PSHUFHW,
        translate_pshuflw,   //X86_INS_PSHUFLW,
        translate_pslldq,    //X86_INS_PSLLDQ,
        translate_psrldq,    //X86_INS_PSRLDQ,
        translate_pswapd,    //X86_INS_PSWAPD,
        translate_ptest,     //X86_INS_PTEST,
        translate_punpckhqdq,//X86_INS_PUNPCKHQDQ,
        translate_punpcklqdq,//X86_INS_PUNPCKLQDQ,
        translate_push,      //X86_INS_PUSH,
        translate_pushaw,    //X86_INS_PUSHAW,
        translate_pushal,    //X86_INS_PUSHAL,
        translate_pushf,     //X86_INS_PUSHF,
        translate_pushf,                //X86_INS_PUSHFD,
        NULL,                //X86_INS_PUSHFQ,
        translate_rcl,       //X86_INS_RCL,
        translate_rcpps,     //X86_INS_RCPPS,
        translate_rcpss,     //X86_INS_RCPSS,
        translate_rcr,       //X86_INS_RCR,
        translate_rdfsbase,  //X86_INS_RDFSBASE,
        translate_rdgsbase,  //X86_INS_RDGSBASE,
        translate_rdmsr,     //X86_INS_RDMSR,
        translate_rdpmc,     //X86_INS_RDPMC,
        NULL,                //X86_INS_RDRAND,
        NULL,                //X86_INS_RDSEED,
        translate_rdtsc,     //X86_INS_RDTSC,
        translate_rdtscp,    //X86_INS_RDTSCP,
        translate_rol,       //X86_INS_ROL,
        translate_ror,       //X86_INS_ROR,
        NULL,                //X86_INS_RORX,
        translate_roundpd,   //X86_INS_ROUNDPD,
        translate_roundps,   //X86_INS_ROUNDPS,
        translate_roundsd,   //X86_INS_ROUNDSD,
        translate_roundss,   //X86_INS_ROUNDSS,
        translate_rsm,       //X86_INS_RSM,
        translate_rsqrtps,   //X86_INS_RSQRTPS,
        translate_rsqrtss,   //X86_INS_RSQRTSS,
        translate_sahf,      //X86_INS_SAHF,
        translate_sal,       //X86_INS_SAL,
        translate_salc,      //X86_INS_SALC,
        translate_sar,       //X86_INS_SAR,
        NULL,                //X86_INS_SARX,
        translate_sbb,       //X86_INS_SBB,
        translate_scas,      //X86_INS_SCASB,
        translate_scas,      //X86_INS_SCASD,
        translate_scas,      //X86_INS_SCASQ,
        translate_scas,      //X86_INS_SCASW,
        translate_setae,     //X86_INS_SETAE,
        translate_seta,      //X86_INS_SETA,
        translate_setbe,     //X86_INS_SETBE,
        translate_setb,      //X86_INS_SETB,
        translate_setz,      //X86_INS_SETE,
        translate_setge,     //X86_INS_SETGE,
        translate_setg,      //X86_INS_SETG,
        translate_setle,     //X86_INS_SETLE,
        translate_setl,      //X86_INS_SETL,
        translate_setnz,     //X86_INS_SETNE,
        translate_setno,     //X86_INS_SETNO,
        translate_setnp,     //X86_INS_SETNP,
        translate_setns,     //X86_INS_SETNS,
        translate_seto,      //X86_INS_SETO,
        translate_setp,      //X86_INS_SETP,
        translate_sets,      //X86_INS_SETS,
        translate_sfence,    //X86_INS_SFENCE,
        translate_sgdt,      //X86_INS_SGDT,
        NULL,                //X86_INS_SHA1MSG1,
        NULL,                //X86_INS_SHA1MSG2,
        NULL,                //X86_INS_SHA1NEXTE,
        NULL,                //X86_INS_SHA1RNDS4,
        NULL,                //X86_INS_SHA256MSG1,
        NULL,                //X86_INS_SHA256MSG2,
        NULL,                //X86_INS_SHA256RNDS2,
        translate_shl,       //X86_INS_SHL,
        translate_shld,      //X86_INS_SHLD,
        NULL,                //X86_INS_SHLX,
        translate_shr,       //X86_INS_SHR,
        translate_shrd,      //X86_INS_SHRD,
        NULL,                //X86_INS_SHRX,
        translate_shufpd,    //X86_INS_SHUFPD,
        translate_shufps,    //X86_INS_SHUFPS,
        translate_sidt,      //X86_INS_SIDT,
        translate_fsin,      //X86_INS_FSIN,
        translate_skinit,    //X86_INS_SKINIT,
        translate_sldt,      //X86_INS_SLDT,
        translate_smsw,      //X86_INS_SMSW,
        translate_sqrtpd,    //X86_INS_SQRTPD,
        translate_sqrtps,    //X86_INS_SQRTPS,
        translate_sqrtsd,    //X86_INS_SQRTSD,
        translate_sqrtss,    //X86_INS_SQRTSS,
        translate_fsqrt,     //X86_INS_FSQRT,
        translate_stac,      //X86_INS_STAC,
        translate_stc,       //X86_INS_STC,
        translate_std,       //X86_INS_STD,
        translate_stgi,      //X86_INS_STGI,
        translate_sti,       //X86_INS_STI,
        translate_stmxcsr,   //X86_INS_STMXCSR,
        translate_stos,      //X86_INS_STOSB,
        translate_stos,      //X86_INS_STOSD,
        translate_stos,      //X86_INS_STOSQ,
        translate_stos,      //X86_INS_STOSW,
        translate_str,       //X86_INS_STR,
        translate_fst,       //X86_INS_FST,
        translate_fstp,      //X86_INS_FSTP,
        NULL,                //X86_INS_FSTPNCE,
        translate_fxch,      //X86_INS_FXCH,
        translate_subpd,     //X86_INS_SUBPD,
        translate_subps,     //X86_INS_SUBPS,
        translate_fsubr,     //X86_INS_FSUBR,
        translate_fisubr,    //X86_INS_FISUBR,
        translate_fsubrp,    //X86_INS_FSUBRP,
        translate_subsd,     //X86_INS_SUBSD,
        translate_subss,     //X86_INS_SUBSS,
        translate_fsub,      //X86_INS_FSUB,
        translate_fisub,     //X86_INS_FISUB,
        translate_fsubp,     //X86_INS_FSUBP,
        translate_swapgs,    //X86_INS_SWAPGS,
        translate_syscall,   //X86_INS_SYSCALL,
        translate_sysenter,  //X86_INS_SYSENTER,
        translate_sysexit,   //X86_INS_SYSEXIT,
        translate_sysret,    //X86_INS_SYSRET,
        NULL,                //X86_INS_T1MSKC,
        translate_test,      //X86_INS_TEST,
        translate_ud2,       //X86_INS_UD2,
        translate_ftst,      //X86_INS_FTST,
#ifdef CONFIG_SOFTMMU
        translate_tzcnt,     //X86_INS_TZCNT
#else
        translate_bsf,       //X86_INS_TZCNT // same as master
#endif
        NULL,                //X86_INS_TZMSK,
        translate_fucomip,   //X86_INS_FUCOMIP,
        translate_fucomi,    //X86_INS_FUCOMI,
        translate_fucompp,   //X86_INS_FUCOMPP,
        translate_fucomp,    //X86_INS_FUCOMP,
        translate_fucom,     //X86_INS_FUCOM,
        NULL,                //X86_INS_UD2B,
        translate_unpckhpd,  //X86_INS_UNPCKHPD,
        translate_unpckhps,  //X86_INS_UNPCKHPS,
        translate_unpcklpd,  //X86_INS_UNPCKLPD,
        translate_unpcklps,  //X86_INS_UNPCKLPS,
        translate_vaddpd,    //X86_INS_VADDPD,
        translate_vaddps,    //X86_INS_VADDPS,
        translate_vaddsd,    //X86_INS_VADDSD,
        translate_vaddss,    //X86_INS_VADDSS,
        translate_vaddsubpd, //X86_INS_VADDSUBPD,
        translate_vaddsubps, //X86_INS_VADDSUBPS,
        translate_vaesdeclast,//X86_INS_VAESDECLAST,
        translate_vaesdec,   //X86_INS_VAESDEC,
        translate_vaesenclast,//X86_INS_VAESENCLAST,
        translate_vaesenc,   //X86_INS_VAESENC,
        translate_vaesimc,   //X86_INS_VAESIMC,
        translate_vaeskeygenassist,//X86_INS_VAESKEYGENASSIST,
        NULL,                //X86_INS_VALIGND,
        NULL,                //X86_INS_VALIGNQ,
        translate_vandnpd,   //X86_INS_VANDNPD,
        translate_vandnps,   //X86_INS_VANDNPS,
        translate_vandpd,    //X86_INS_VANDPD,
        translate_vandps,    //X86_INS_VANDPS,
        NULL,                //X86_INS_VBLENDMPD,
        NULL,                //X86_INS_VBLENDMPS,
        translate_vblendpd,  //X86_INS_VBLENDPD,
        translate_vblendps,  //X86_INS_VBLENDPS,
        translate_vblendvpd, //X86_INS_VBLENDVPD,
        translate_vblendvps, //X86_INS_VBLENDVPS,
        translate_vbroadcastf128,//X86_INS_VBROADCASTF128,
        NULL,                //X86_INS_VBROADCASTI32X4,
        NULL,                //X86_INS_VBROADCASTI64X4,
        translate_vbroadcastsd,//X86_INS_VBROADCASTSD,
        translate_vbroadcastss,//X86_INS_VBROADCASTSS,
        NULL,                //X86_INS_VCOMPRESSPD,
        NULL,                //X86_INS_VCOMPRESSPS,
        translate_vcvtdq2pd, //X86_INS_VCVTDQ2PD,
        translate_vcvtdq2ps, //X86_INS_VCVTDQ2PS,
        NULL,                //X86_INS_VCVTPD2DQX,
        translate_vcvtpd2dq, //X86_INS_VCVTPD2DQ,
        NULL,                //X86_INS_VCVTPD2PSX,
        translate_vcvtpd2ps, //X86_INS_VCVTPD2PS,
        NULL,                //X86_INS_VCVTPD2UDQ,
        NULL,                //X86_INS_VCVTPH2PS,
        translate_vcvtps2dq, //X86_INS_VCVTPS2DQ,
        translate_vcvtps2pd, //X86_INS_VCVTPS2PD,
        NULL,                //X86_INS_VCVTPS2PH,
        NULL,                //X86_INS_VCVTPS2UDQ,
        translate_vcvtsd2si, //X86_INS_VCVTSD2SI,
        NULL,                //X86_INS_VCVTSD2USI,
        translate_vcvtss2si, //X86_INS_VCVTSS2SI,
        NULL,                //X86_INS_VCVTSS2USI,
        NULL,                //X86_INS_VCVTTPD2DQX,
        translate_vcvttpd2dq,//X86_INS_VCVTTPD2DQ,
        NULL,                //X86_INS_VCVTTPD2UDQ,
        translate_vcvttps2dq,//X86_INS_VCVTTPS2DQ,
        NULL,                //X86_INS_VCVTTPS2UDQ,
        NULL,                //X86_INS_VCVTUDQ2PD,
        NULL,                //X86_INS_VCVTUDQ2PS,
        translate_vdivpd,    //X86_INS_VDIVPD,
        translate_vdivps,    //X86_INS_VDIVPS,
        translate_vdivsd,    //X86_INS_VDIVSD,
        translate_vdivss,    //X86_INS_VDIVSS,
        translate_vdppd,     //X86_INS_VDPPD,
        translate_vdpps,     //X86_INS_VDPPS,
        translate_verr,      //X86_INS_VERR,
        translate_verw,      //X86_INS_VERW,
        NULL,                //X86_INS_VEXP2PD,
        NULL,                //X86_INS_VEXP2PS,
        NULL,                //X86_INS_VEXPANDPD,
        NULL,                //X86_INS_VEXPANDPS,
        translate_vextractf128,//X86_INS_VEXTRACTF128,
        NULL,                //X86_INS_VEXTRACTF32X4,
        NULL,                //X86_INS_VEXTRACTF64X4,
        NULL,                //X86_INS_VEXTRACTI128,
        NULL,                //X86_INS_VEXTRACTI32X4,
        NULL,                //X86_INS_VEXTRACTI64X4,
        translate_vextractps,//X86_INS_VEXTRACTPS,
        translate_vfmadd132pd,//X86_INS_VFMADD132PD,
        translate_vfmadd132ps,//X86_INS_VFMADD132PS,
        NULL,                //X86_INS_VFMADDPD,
        translate_vfmadd213pd,//X86_INS_VFMADD213PD,
        translate_vfmadd231pd,//X86_INS_VFMADD231PD,
        NULL,                //X86_INS_VFMADDPS,
        translate_vfmadd213ps,//X86_INS_VFMADD213PS,
        translate_vfmadd231ps,//X86_INS_VFMADD231PS,
        NULL,                //X86_INS_VFMADDSD,
        translate_vfmadd213sd,//X86_INS_VFMADD213SD,
        translate_vfmadd132sd,//X86_INS_VFMADD132SD,
        translate_vfmadd231sd,//X86_INS_VFMADD231SD,
        NULL,                //X86_INS_VFMADDSS,
        translate_vfmadd213ss,//X86_INS_VFMADD213SS,
        translate_vfmadd132ss,//X86_INS_VFMADD132SS,
        translate_vfmadd231ss,//X86_INS_VFMADD231SS,
        translate_vfmaddsub132pd,//X86_INS_VFMADDSUB132PD,
        translate_vfmaddsub132ps,//X86_INS_VFMADDSUB132PS,
        NULL,                //X86_INS_VFMADDSUBPD,
        translate_vfmaddsub213pd,//X86_INS_VFMADDSUB213PD,
        translate_vfmaddsub231pd,//X86_INS_VFMADDSUB231PD,
        NULL,                //X86_INS_VFMADDSUBPS,
        translate_vfmaddsub213ps,//X86_INS_VFMADDSUB213PS,
        translate_vfmaddsub231ps,//X86_INS_VFMADDSUB231PS,
        translate_vfmsub132pd,//X86_INS_VFMSUB132PD,
        translate_vfmsub132ps,//X86_INS_VFMSUB132PS,
        translate_vfmsubadd132pd,//X86_INS_VFMSUBADD132PD,
        translate_vfmsubadd132ps,//X86_INS_VFMSUBADD132PS,
        NULL,                //X86_INS_VFMSUBADDPD,
        translate_vfmsubadd213pd,//X86_INS_VFMSUBADD213PD,
        translate_vfmsubadd231pd,//X86_INS_VFMSUBADD231PD,
        NULL,                //X86_INS_VFMSUBADDPS,
        translate_vfmsubadd213ps,//X86_INS_VFMSUBADD213PS,
        translate_vfmsubadd231ps,//X86_INS_VFMSUBADD231PS,
        NULL,                //X86_INS_VFMSUBPD,
        translate_vfmsub213pd,//X86_INS_VFMSUB213PD,
        translate_vfmsub231pd,//X86_INS_VFMSUB231PD,
        NULL,                //X86_INS_VFMSUBPS,
        translate_vfmsub213ps,//X86_INS_VFMSUB213PS,
        translate_vfmsub231ps,//X86_INS_VFMSUB231PS,
        NULL,                //X86_INS_VFMSUBSD,
        translate_vfmsub213sd,//X86_INS_VFMSUB213SD,
        translate_vfmsub132sd,//X86_INS_VFMSUB132SD,
        translate_vfmsub231sd,//X86_INS_VFMSUB231SD,
        NULL,                //X86_INS_VFMSUBSS,
        translate_vfmsub213ss,//X86_INS_VFMSUB213SS,
        translate_vfmsub132ss,//X86_INS_VFMSUB132SS,
        translate_vfmsub231ss,//X86_INS_VFMSUB231SS,
        translate_vfnmadd132pd,//X86_INS_VFNMADD132PD,
        translate_vfnmadd132ps,//X86_INS_VFNMADD132PS,
        NULL,                //X86_INS_VFNMADDPD,
        translate_vfnmadd213pd,//X86_INS_VFNMADD213PD,
        translate_vfnmadd231pd,//X86_INS_VFNMADD231PD,
        NULL,                //X86_INS_VFNMADDPS,
        translate_vfnmadd213ps,//X86_INS_VFNMADD213PS,
        translate_vfnmadd231ps,//X86_INS_VFNMADD231PS,
        NULL,                //X86_INS_VFNMADDSD,
        translate_vfnmadd213sd,//X86_INS_VFNMADD213SD,
        translate_vfnmadd132sd,//X86_INS_VFNMADD132SD,
        translate_vfnmadd231sd,//X86_INS_VFNMADD231SD,
        NULL,                //X86_INS_VFNMADDSS,
        translate_vfnmadd213ss,//X86_INS_VFNMADD213SS,
        translate_vfnmadd132ss,//X86_INS_VFNMADD132SS,
        translate_vfnmadd231ss,//X86_INS_VFNMADD231SS,
        translate_vfnmsub132pd,//X86_INS_VFNMSUB132PD,
        translate_vfnmsub132ps,//X86_INS_VFNMSUB132PS,
        NULL,                //X86_INS_VFNMSUBPD,
        translate_vfnmsub213pd,//X86_INS_VFNMSUB213PD,
        translate_vfnmsub231pd,//X86_INS_VFNMSUB231PD,
        NULL,                //X86_INS_VFNMSUBPS,
        translate_vfnmsub213ps,//X86_INS_VFNMSUB213PS,
        translate_vfnmsub231ps,//X86_INS_VFNMSUB231PS,
        NULL,                //X86_INS_VFNMSUBSD,
        translate_vfnmsub213sd,//X86_INS_VFNMSUB213SD,
        translate_vfnmsub132sd,//X86_INS_VFNMSUB132SD,
        translate_vfnmsub231sd,//X86_INS_VFNMSUB231SD,
        NULL,                //X86_INS_VFNMSUBSS,
        translate_vfnmsub213ss,//X86_INS_VFNMSUB213SS,
        translate_vfnmsub132ss,//X86_INS_VFNMSUB132SS,
        translate_vfnmsub231ss,//X86_INS_VFNMSUB231SS,
        NULL,                //X86_INS_VFRCZPD,
        NULL,                //X86_INS_VFRCZPS,
        NULL,                //X86_INS_VFRCZSD,
        NULL,                //X86_INS_VFRCZSS,
        translate_vorpd,     //X86_INS_VORPD,
        translate_vorps,     //X86_INS_VORPS,
        translate_vxorpd,    //X86_INS_VXORPD,
        translate_vxorps,    //X86_INS_VXORPS,
        NULL,                //X86_INS_VGATHERDPD,
        NULL,                //X86_INS_VGATHERDPS,
        NULL,                //X86_INS_VGATHERPF0DPD,
        NULL,                //X86_INS_VGATHERPF0DPS,
        NULL,                //X86_INS_VGATHERPF0QPD,
        NULL,                //X86_INS_VGATHERPF0QPS,
        NULL,                //X86_INS_VGATHERPF1DPD,
        NULL,                //X86_INS_VGATHERPF1DPS,
        NULL,                //X86_INS_VGATHERPF1QPD,
        NULL,                //X86_INS_VGATHERPF1QPS,
        NULL,                //X86_INS_VGATHERQPD,
        NULL,                //X86_INS_VGATHERQPS,
        translate_vhaddpd,   //X86_INS_VHADDPD,
        translate_vhaddps,   //X86_INS_VHADDPS,
        translate_vhsubpd,   //X86_INS_VHSUBPD,
        translate_vhsubps,   //X86_INS_VHSUBPS,
        translate_vinsertf128,//X86_INS_VINSERTF128,
        NULL,                //X86_INS_VINSERTF32X4,
        NULL,                //X86_INS_VINSERTF32X8,
        NULL,                //X86_INS_VINSERTF64X2,
        NULL,                //X86_INS_VINSERTF64X4,
        NULL,                //X86_INS_VINSERTI128,
        NULL,                //X86_INS_VINSERTI32X4,
        NULL,                //X86_INS_VINSERTI32X8,
        NULL,                //X86_INS_VINSERTI64X2,
        NULL,                //X86_INS_VINSERTI64X4,
        translate_vinsertps, //X86_INS_VINSERTPS,
        translate_vlddqu,    //X86_INS_VLDDQU,
        translate_vldmxcsr,  //X86_INS_VLDMXCSR,
        translate_vmaskmovdqu,//X86_INS_VMASKMOVDQU,
        translate_vmaskmovpd,//X86_INS_VMASKMOVPD,
        translate_vmaskmovps,//X86_INS_VMASKMOVPS,
        translate_vmaxpd,    //X86_INS_VMAXPD,
        translate_vmaxps,    //X86_INS_VMAXPS,
        translate_vmaxsd,    //X86_INS_VMAXSD,
        translate_vmaxss,    //X86_INS_VMAXSS,
        translate_vmcall,    //X86_INS_VMCALL,
        translate_vmclear,   //X86_INS_VMCLEAR,
        NULL,                //X86_INS_VMFUNC,
        translate_vminpd,    //X86_INS_VMINPD,
        translate_vminps,    //X86_INS_VMINPS,
        translate_vminsd,    //X86_INS_VMINSD,
        translate_vminss,    //X86_INS_VMINSS,
        translate_vmlaunch,  //X86_INS_VMLAUNCH,
        translate_vmload,    //X86_INS_VMLOAD,
        translate_vmmcall,   //X86_INS_VMMCALL,
        translate_vmovq,     //X86_INS_VMOVQ,
        translate_vmovddup,  //X86_INS_VMOVDDUP,
        translate_vmovd,     //X86_INS_VMOVD,
        NULL,                //X86_INS_VMOVDQA32,
        NULL,                //X86_INS_VMOVDQA64,
        translate_vmovdqa,   //X86_INS_VMOVDQA,
        NULL,                //X86_INS_VMOVDQU16,
        NULL,                //X86_INS_VMOVDQU32,
        NULL,                //X86_INS_VMOVDQU64,
        NULL,                //X86_INS_VMOVDQU8,
        translate_vmovdqu,   //X86_INS_VMOVDQU,
        translate_vmovhlps,  //X86_INS_VMOVHLPS,
        translate_vmovhpd,   //X86_INS_VMOVHPD,
        translate_vmovhps,   //X86_INS_VMOVHPS,
        translate_vmovlhps,  //X86_INS_VMOVLHPS,
        translate_vmovlpd,   //X86_INS_VMOVLPD,
        translate_vmovlps,   //X86_INS_VMOVLPS,
        translate_vmovmskpd, //X86_INS_VMOVMSKPD,
        translate_vmovmskps, //X86_INS_VMOVMSKPS,
        translate_vmovntdqa, //X86_INS_VMOVNTDQA,
        translate_vmovntdq,  //X86_INS_VMOVNTDQ,
        translate_vmovntpd,  //X86_INS_VMOVNTPD,
        translate_vmovntps,  //X86_INS_VMOVNTPS,
        translate_vmovsd,    //X86_INS_VMOVSD,
        translate_vmovshdup, //X86_INS_VMOVSHDUP,
        translate_vmovsldup, //X86_INS_VMOVSLDUP,
        translate_vmovss,    //X86_INS_VMOVSS,
        translate_vmovupd,   //X86_INS_VMOVUPD,
        translate_vmovups,   //X86_INS_VMOVUPS,
        translate_vmpsadbw,  //X86_INS_VMPSADBW,
        translate_vmptrld,   //X86_INS_VMPTRLD,
        translate_vmptrst,   //X86_INS_VMPTRST,
        translate_vmread,    //X86_INS_VMREAD,
        translate_vmresume,  //X86_INS_VMRESUME,
        translate_vmrun,     //X86_INS_VMRUN,
        translate_vmsave,    //X86_INS_VMSAVE,
        translate_vmulpd,    //X86_INS_VMULPD,
        translate_vmulps,    //X86_INS_VMULPS,
        translate_vmulsd,    //X86_INS_VMULSD,
        translate_vmulss,    //X86_INS_VMULSS,
        translate_vmwrite,   //X86_INS_VMWRITE,
        translate_vmxoff,    //X86_INS_VMXOFF,
        translate_vmxon,     //X86_INS_VMXON,
        translate_vpabsb,    //X86_INS_VPABSB,
        translate_vpabsd,    //X86_INS_VPABSD,
        NULL,                //X86_INS_VPABSQ,
        translate_vpabsw,    //X86_INS_VPABSW,
        translate_vpackssdw, //X86_INS_VPACKSSDW,
        translate_vpacksswb, //X86_INS_VPACKSSWB,
        translate_vpackusdw, //X86_INS_VPACKUSDW,
        translate_vpackuswb, //X86_INS_VPACKUSWB,
        translate_vpaddb,    //X86_INS_VPADDB,
        translate_vpaddd,    //X86_INS_VPADDD,
        translate_vpaddq,    //X86_INS_VPADDQ,
        translate_vpaddsb,   //X86_INS_VPADDSB,
        translate_vpaddsw,   //X86_INS_VPADDSW,
        NULL,                //X86_INS_VPADDUSB,
        translate_vpaddusw,  //X86_INS_VPADDUSW,
        translate_vpaddw,    //X86_INS_VPADDW,
        translate_vpalignr,  //X86_INS_VPALIGNR,
        NULL,                //X86_INS_VPANDD,
        NULL,                //X86_INS_VPANDND,
        NULL,                //X86_INS_VPANDNQ,
        translate_vpandn,    //X86_INS_VPANDN,
        NULL,                //X86_INS_VPANDQ,
        translate_vpand,     //X86_INS_VPAND,
        translate_vpavgb,    //X86_INS_VPAVGB,
        translate_vpavgw,    //X86_INS_VPAVGW,
        NULL,                //X86_INS_VPBLENDD,
        NULL,                //X86_INS_VPBLENDMB,
        NULL,                //X86_INS_VPBLENDMD,
        NULL,                //X86_INS_VPBLENDMQ,
        NULL,                //X86_INS_VPBLENDMW,
        translate_vpblendvb, //X86_INS_VPBLENDVB,
        NULL,                //X86_INS_VPBLENDW,
        NULL,                //X86_INS_VPBROADCASTB,
        NULL,                //X86_INS_VPBROADCASTD,
        NULL,                //X86_INS_VPBROADCASTMB2Q,
        NULL,                //X86_INS_VPBROADCASTMW2D,
        NULL,                //X86_INS_VPBROADCASTQ,
        NULL,                //X86_INS_VPBROADCASTW,
        translate_vpclmulqdq,//X86_INS_VPCLMULQDQ,
        NULL,                //X86_INS_VPCMOV,
        NULL,                //X86_INS_VPCMPB,
        NULL,                //X86_INS_VPCMPD,
        translate_vpcmpeqb,  //X86_INS_VPCMPEQB,
        translate_vpcmpeqd,  //X86_INS_VPCMPEQD,
        translate_vpcmpeqq,  //X86_INS_VPCMPEQQ,
        translate_vpcmpeqw,  //X86_INS_VPCMPEQW,
        NULL,                //X86_INS_VPCMPESTRI,
        translate_vpcmpestrm,//X86_INS_VPCMPESTRM,
        translate_vpcmpgtb,  //X86_INS_VPCMPGTB,
        translate_vpcmpgtd,  //X86_INS_VPCMPGTD,
        translate_vpcmpgtq,  //X86_INS_VPCMPGTQ,
        translate_vpcmpgtw,  //X86_INS_VPCMPGTW,
        translate_vpcmpistri,//X86_INS_VPCMPISTRI,
        translate_vpcmpistrm,//X86_INS_VPCMPISTRM,
        NULL,                //X86_INS_VPCMPQ,
        NULL,                //X86_INS_VPCMPUB,
        NULL,                //X86_INS_VPCMPUD,
        NULL,                //X86_INS_VPCMPUQ,
        NULL,                //X86_INS_VPCMPUW,
        NULL,                //X86_INS_VPCMPW,
        NULL,                //X86_INS_VPCOMB,
        NULL,                //X86_INS_VPCOMD,
        NULL,                //X86_INS_VPCOMPRESSD,
        NULL,                //X86_INS_VPCOMPRESSQ,
        NULL,                //X86_INS_VPCOMQ,
        NULL,                //X86_INS_VPCOMUB,
        NULL,                //X86_INS_VPCOMUD,
        NULL,                //X86_INS_VPCOMUQ,
        NULL,                //X86_INS_VPCOMUW,
        NULL,                //X86_INS_VPCOMW,
        NULL,                //X86_INS_VPCONFLICTD,
        NULL,                //X86_INS_VPCONFLICTQ,
        translate_vperm2f128,//X86_INS_VPERM2F128,
        NULL,                //X86_INS_VPERM2I128,
        NULL,                //X86_INS_VPERMD,
        NULL,                //X86_INS_VPERMI2D,
        NULL,                //X86_INS_VPERMI2PD,
        NULL,                //X86_INS_VPERMI2PS,
        NULL,                //X86_INS_VPERMI2Q,
        NULL,                //X86_INS_VPERMIL2PD,
        NULL,                //X86_INS_VPERMIL2PS,
        translate_vpermilpd, //X86_INS_VPERMILPD,
        translate_vpermilps, //X86_INS_VPERMILPS,
        NULL,                //X86_INS_VPERMPD,
        NULL,                //X86_INS_VPERMPS,
        NULL,                //X86_INS_VPERMQ,
        NULL,                //X86_INS_VPERMT2D,
        NULL,                //X86_INS_VPERMT2PD,
        NULL,                //X86_INS_VPERMT2PS,
        NULL,                //X86_INS_VPERMT2Q,
        NULL,                //X86_INS_VPEXPANDD,
        NULL,                //X86_INS_VPEXPANDQ,
        translate_vpextrb,   //X86_INS_VPEXTRB,
        translate_vpextrd,   //X86_INS_VPEXTRD,
        NULL,                //X86_INS_VPEXTRQ,
        translate_vpextrw,   //X86_INS_VPEXTRW,
        NULL,                //X86_INS_VPGATHERDD,
        NULL,                //X86_INS_VPGATHERDQ,
        NULL,                //X86_INS_VPGATHERQD,
        NULL,                //X86_INS_VPGATHERQQ,
        NULL,                //X86_INS_VPHADDBD,
        NULL,                //X86_INS_VPHADDBQ,
        NULL,                //X86_INS_VPHADDBW,
        NULL,                //X86_INS_VPHADDDQ,
        translate_vphaddd,   //X86_INS_VPHADDD,
        translate_vphaddsw,  //X86_INS_VPHADDSW,
        NULL,                //X86_INS_VPHADDUBD,
        NULL,                //X86_INS_VPHADDUBQ,
        NULL,                //X86_INS_VPHADDUBW,
        NULL,                //X86_INS_VPHADDUDQ,
        NULL,                //X86_INS_VPHADDUWD,
        NULL,                //X86_INS_VPHADDUWQ,
        NULL,                //X86_INS_VPHADDWD,
        NULL,                //X86_INS_VPHADDWQ,
        translate_vphaddw,   //X86_INS_VPHADDW,
        translate_vphminposuw,//X86_INS_VPHMINPOSUW,
        NULL,                //X86_INS_VPHSUBBW,
        NULL,                //X86_INS_VPHSUBDQ,
        translate_vphsubd,   //X86_INS_VPHSUBD,
        translate_vphsubsw,  //X86_INS_VPHSUBSW,
        NULL,                //X86_INS_VPHSUBWD,
        translate_vphsubw,   //X86_INS_VPHSUBW,
        translate_vpinsrb,   //X86_INS_VPINSRB,
        translate_vpinsrd,   //X86_INS_VPINSRD,
        translate_vpinsrq,   //X86_INS_VPINSRQ,
        translate_vpinsrw,   //X86_INS_VPINSRW,
        NULL,                //X86_INS_VPLZCNTD,
        NULL,                //X86_INS_VPLZCNTQ,
        NULL,                //X86_INS_VPMACSDD,
        NULL,                //X86_INS_VPMACSDQH,
        NULL,                //X86_INS_VPMACSDQL,
        NULL,                //X86_INS_VPMACSSDD,
        NULL,                //X86_INS_VPMACSSDQH,
        NULL,                //X86_INS_VPMACSSDQL,
        NULL,                //X86_INS_VPMACSSWD,
        NULL,                //X86_INS_VPMACSSWW,
        NULL,                //X86_INS_VPMACSWD,
        NULL,                //X86_INS_VPMACSWW,
        NULL,                //X86_INS_VPMADCSSWD,
        NULL,                //X86_INS_VPMADCSWD,
        translate_vpmaddubsw,//X86_INS_VPMADDUBSW,
        translate_vpmaddwd,  //X86_INS_VPMADDWD,
        NULL,                //X86_INS_VPMASKMOVD,
        NULL,                //X86_INS_VPMASKMOVQ,
        translate_vpmaxsb,   //X86_INS_VPMAXSB,
        translate_vpmaxsd,   //X86_INS_VPMAXSD,
        NULL,                //X86_INS_VPMAXSQ,
        translate_vpmaxsw,   //X86_INS_VPMAXSW,
        translate_vpmaxub,   //X86_INS_VPMAXUB,
        translate_vpmaxud,   //X86_INS_VPMAXUD,
        NULL,                //X86_INS_VPMAXUQ,
        translate_vpmaxuw,   //X86_INS_VPMAXUW,
        translate_vpminsb,   //X86_INS_VPMINSB,
        translate_vpminsd,   //X86_INS_VPMINSD,
        NULL,                //X86_INS_VPMINSQ,
        translate_vpminsw,   //X86_INS_VPMINSW,
        translate_vpminub,   //X86_INS_VPMINUB,
        translate_vpminud,   //X86_INS_VPMINUD,
        NULL,                //X86_INS_VPMINUQ,
        translate_vpminuw,   //X86_INS_VPMINUW,
        NULL,                //X86_INS_VPMOVDB,
        NULL,                //X86_INS_VPMOVDW,
        NULL,                //X86_INS_VPMOVM2B,
        NULL,                //X86_INS_VPMOVM2D,
        NULL,                //X86_INS_VPMOVM2Q,
        NULL,                //X86_INS_VPMOVM2W,
        translate_vpmovmskb, //X86_INS_VPMOVMSKB,
        NULL,                //X86_INS_VPMOVQB,
        NULL,                //X86_INS_VPMOVQD,
        NULL,                //X86_INS_VPMOVQW,
        NULL,                //X86_INS_VPMOVSDB,
        NULL,                //X86_INS_VPMOVSDW,
        NULL,                //X86_INS_VPMOVSQB,
        NULL,                //X86_INS_VPMOVSQD,
        NULL,                //X86_INS_VPMOVSQW,
        translate_vpmovsxbd, //X86_INS_VPMOVSXBD,
        translate_vpmovsxbq, //X86_INS_VPMOVSXBQ,
        translate_vpmovsxbw, //X86_INS_VPMOVSXBW,
        translate_vpmovsxdq, //X86_INS_VPMOVSXDQ,
        translate_vpmovsxwd, //X86_INS_VPMOVSXWD,
        translate_vpmovsxwq, //X86_INS_VPMOVSXWQ,
        NULL,                //X86_INS_VPMOVUSDB,
        NULL,                //X86_INS_VPMOVUSDW,
        NULL,                //X86_INS_VPMOVUSQB,
        NULL,                //X86_INS_VPMOVUSQD,
        NULL,                //X86_INS_VPMOVUSQW,
        translate_vpmovzxbd, //X86_INS_VPMOVZXBD,
        translate_vpmovzxbq, //X86_INS_VPMOVZXBQ,
        translate_vpmovzxbw, //X86_INS_VPMOVZXBW,
        translate_vpmovzxdq, //X86_INS_VPMOVZXDQ,
        translate_vpmovzxwd, //X86_INS_VPMOVZXWD,
        translate_vpmovzxwq, //X86_INS_VPMOVZXWQ,
        translate_vpmuldq,   //X86_INS_VPMULDQ,
        translate_vpmulhrsw, //X86_INS_VPMULHRSW,
        translate_vpmulhuw,  //X86_INS_VPMULHUW,
        translate_vpmulhw,   //X86_INS_VPMULHW,
        translate_vpmulld,   //X86_INS_VPMULLD,
        NULL,                //X86_INS_VPMULLQ,
        translate_vpmullw,   //X86_INS_VPMULLW,
        translate_vpmuludq,  //X86_INS_VPMULUDQ,
        NULL,                //X86_INS_VPORD,
        NULL,                //X86_INS_VPORQ,
        translate_vpor,      //X86_INS_VPOR,
        NULL,                //X86_INS_VPPERM,
        NULL,                //X86_INS_VPROTB,
        NULL,                //X86_INS_VPROTD,
        NULL,                //X86_INS_VPROTQ,
        NULL,                //X86_INS_VPROTW,
        translate_vpsadbw,   //X86_INS_VPSADBW,
        NULL,                //X86_INS_VPSCATTERDD,
        NULL,                //X86_INS_VPSCATTERDQ,
        NULL,                //X86_INS_VPSCATTERQD,
        NULL,                //X86_INS_VPSCATTERQQ,
        NULL,                //X86_INS_VPSHAB,
        NULL,                //X86_INS_VPSHAD,
        NULL,                //X86_INS_VPSHAQ,
        NULL,                //X86_INS_VPSHAW,
        NULL,                //X86_INS_VPSHLB,
        NULL,                //X86_INS_VPSHLD,
        NULL,                //X86_INS_VPSHLQ,
        NULL,                //X86_INS_VPSHLW,
        translate_vpshufb,   //X86_INS_VPSHUFB,
        translate_vpshufd,   //X86_INS_VPSHUFD,
        translate_vpshufhw,  //X86_INS_VPSHUFHW,
        translate_vpshuflw,  //X86_INS_VPSHUFLW,
        translate_vpsignb,   //X86_INS_VPSIGNB,
        translate_vpsignd,   //X86_INS_VPSIGND,
        translate_vpsignw,   //X86_INS_VPSIGNW,
        translate_vpslldq,   //X86_INS_VPSLLDQ,
        translate_vpslld,    //X86_INS_VPSLLD,
        translate_vpsllq,    //X86_INS_VPSLLQ,
        NULL,                //X86_INS_VPSLLVD,
        NULL,                //X86_INS_VPSLLVQ,
        translate_vpsllw,    //X86_INS_VPSLLW,
        translate_vpsrad,    //X86_INS_VPSRAD,
        NULL,                //X86_INS_VPSRAQ,
        NULL,                //X86_INS_VPSRAVD,
        NULL,                //X86_INS_VPSRAVQ,
        translate_vpsraw,    //X86_INS_VPSRAW,
        translate_vpsrldq,   //X86_INS_VPSRLDQ,
        translate_vpsrld,    //X86_INS_VPSRLD,
        translate_vpsrlq,    //X86_INS_VPSRLQ,
        NULL,                //X86_INS_VPSRLVD,
        NULL,                //X86_INS_VPSRLVQ,
        translate_vpsrlw,    //X86_INS_VPSRLW,
        translate_vpsubb,    //X86_INS_VPSUBB,
        translate_vpsubd,    //X86_INS_VPSUBD,
        translate_vpsubq,    //X86_INS_VPSUBQ,
        translate_vpsubsb,   //X86_INS_VPSUBSB,
        translate_vpsubsw,   //X86_INS_VPSUBSW,
        translate_vpsubusb,  //X86_INS_VPSUBUSB,
        translate_vpsubusw,  //X86_INS_VPSUBUSW,
        translate_vpsubw,    //X86_INS_VPSUBW,
        NULL,                //X86_INS_VPTESTMD,
        NULL,                //X86_INS_VPTESTMQ,
        NULL,                //X86_INS_VPTESTNMD,
        NULL,                //X86_INS_VPTESTNMQ,
        translate_vptest,    //X86_INS_VPTEST,
        translate_vpunpckhbw,//X86_INS_VPUNPCKHBW,
        translate_vpunpckhdq,//X86_INS_VPUNPCKHDQ,
        translate_vpunpckhqdq,//X86_INS_VPUNPCKHQDQ,
        translate_vpunpckhwd,//X86_INS_VPUNPCKHWD,
        translate_vpunpcklbw,//X86_INS_VPUNPCKLBW,
        translate_vpunpckldq,//X86_INS_VPUNPCKLDQ,
        translate_vpunpcklqdq,//X86_INS_VPUNPCKLQDQ,
        translate_vpunpcklwd,//X86_INS_VPUNPCKLWD,
        NULL,                //X86_INS_VPXORD,
        NULL,                //X86_INS_VPXORQ,
        translate_vpxor,     //X86_INS_VPXOR,
        NULL,                //X86_INS_VRCP14PD,
        NULL,                //X86_INS_VRCP14PS,
        NULL,                //X86_INS_VRCP14SD,
        NULL,                //X86_INS_VRCP14SS,
        NULL,                //X86_INS_VRCP28PD,
        NULL,                //X86_INS_VRCP28PS,
        NULL,                //X86_INS_VRCP28SD,
        NULL,                //X86_INS_VRCP28SS,
        translate_vrcpps,    //X86_INS_VRCPPS,
        translate_vrcpss,    //X86_INS_VRCPSS,
        NULL,                //X86_INS_VRNDSCALEPD,
        NULL,                //X86_INS_VRNDSCALEPS,
        NULL,                //X86_INS_VRNDSCALESD,
        NULL,                //X86_INS_VRNDSCALESS,
        translate_vroundpd,  //X86_INS_VROUNDPD,
        translate_vroundps,  //X86_INS_VROUNDPS,
        translate_vroundsd,  //X86_INS_VROUNDSD,
        translate_vroundss,  //X86_INS_VROUNDSS,
        NULL,                //X86_INS_VRSQRT14PD,
        NULL,                //X86_INS_VRSQRT14PS,
        NULL,                //X86_INS_VRSQRT14SD,
        NULL,                //X86_INS_VRSQRT14SS,
        NULL,                //X86_INS_VRSQRT28PD,
        NULL,                //X86_INS_VRSQRT28PS,
        NULL,                //X86_INS_VRSQRT28SD,
        NULL,                //X86_INS_VRSQRT28SS,
        translate_vrsqrtps,  //X86_INS_VRSQRTPS,
        translate_vrsqrtss,  //X86_INS_VRSQRTSS,
        NULL,                //X86_INS_VSCATTERDPD,
        NULL,                //X86_INS_VSCATTERDPS,
        NULL,                //X86_INS_VSCATTERPF0DPD,
        NULL,                //X86_INS_VSCATTERPF0DPS,
        NULL,                //X86_INS_VSCATTERPF0QPD,
        NULL,                //X86_INS_VSCATTERPF0QPS,
        NULL,                //X86_INS_VSCATTERPF1DPD,
        NULL,                //X86_INS_VSCATTERPF1DPS,
        NULL,                //X86_INS_VSCATTERPF1QPD,
        NULL,                //X86_INS_VSCATTERPF1QPS,
        NULL,                //X86_INS_VSCATTERQPD,
        NULL,                //X86_INS_VSCATTERQPS,
        translate_vshufpd,   //X86_INS_VSHUFPD,
        translate_vshufps,   //X86_INS_VSHUFPS,
        translate_vsqrtpd,   //X86_INS_VSQRTPD,
        translate_vsqrtps,   //X86_INS_VSQRTPS,
        translate_vsqrtsd,   //X86_INS_VSQRTSD,
        translate_vsqrtss,   //X86_INS_VSQRTSS,
        translate_vstmxcsr,  //X86_INS_VSTMXCSR,
        translate_vsubpd,    //X86_INS_VSUBPD,
        translate_vsubps,    //X86_INS_VSUBPS,
        translate_vsubsd,    //X86_INS_VSUBSD,
        translate_vsubss,    //X86_INS_VSUBSS,
        NULL,                //X86_INS_VTESTPD,
        NULL,                //X86_INS_VTESTPS,
        translate_vunpckhpd, //X86_INS_VUNPCKHPD,
        translate_vunpckhps, //X86_INS_VUNPCKHPS,
        translate_vunpcklpd, //X86_INS_VUNPCKLPD,
        translate_vunpcklps, //X86_INS_VUNPCKLPS,
        translate_vzeroall,  //X86_INS_VZEROALL,
        translate_vzeroupper,//X86_INS_VZEROUPPER,
        translate_wait,      //X86_INS_WAIT,
        translate_wbinvd,    //X86_INS_WBINVD,
        translate_wrfsbase,  //X86_INS_WRFSBASE,
        translate_wrgsbase,  //X86_INS_WRGSBASE,
        translate_wrmsr,     //X86_INS_WRMSR,
        NULL,                //X86_INS_XABORT,
        NULL,                //X86_INS_XACQUIRE,
        NULL,                //X86_INS_XBEGIN,
        translate_xchg,      //X86_INS_XCHG,
        NULL,                //X86_INS_XCRYPTCBC,
        NULL,                //X86_INS_XCRYPTCFB,
        NULL,                //X86_INS_XCRYPTCTR,
        NULL,                //X86_INS_XCRYPTECB,
        NULL,                //X86_INS_XCRYPTOFB,
        NULL,                //X86_INS_XEND,
        translate_xgetbv,    //X86_INS_XGETBV,
        translate_xlat,                //X86_INS_XLATB,
        NULL,                //X86_INS_XRELEASE,
        translate_xrstor,    //X86_INS_XRSTOR,
        NULL,                //X86_INS_XRSTOR64,
        NULL,                //X86_INS_XRSTORS,
        NULL,                //X86_INS_XRSTORS64,
        translate_xsave,     //X86_INS_XSAVE,
        NULL,                //X86_INS_XSAVE64,
        NULL,                //X86_INS_XSAVEC,
        NULL,                //X86_INS_XSAVEC64,
        translate_xsaveopt,  //X86_INS_XSAVEOPT,
        NULL,                //X86_INS_XSAVEOPT64,
        NULL,                //X86_INS_XSAVES,
        NULL,                //X86_INS_XSAVES64,
        translate_xsetbv,    //X86_INS_XSETBV,
        NULL,                //X86_INS_XSHA1,
        NULL,                //X86_INS_XSHA256,
        NULL,                //X86_INS_XSTORE,
        NULL,                //X86_INS_XTEST,
        NULL,                //X86_INS_FDISI8087_NOP,
        NULL,                //X86_INS_FENI8087_NOP,
        NULL,                //X86_INS_CMPSS,
        translate_cmpeqss,   //X86_INS_CMPEQSS,
        translate_cmpltss,   //X86_INS_CMPLTSS,
        translate_cmpless,   //X86_INS_CMPLESS,
        translate_cmpunordss,//X86_INS_CMPUNORDSS,
        translate_cmpneqss,  //X86_INS_CMPNEQSS,
        translate_cmpnltss,  //X86_INS_CMPNLTSS,
        translate_cmpnless,  //X86_INS_CMPNLESS,
        translate_cmpordss,  //X86_INS_CMPORDSS,
        translate_cmps,                //X86_INS_CMPSD,
        translate_cmpeqsd,   //X86_INS_CMPEQSD,
        translate_cmpltsd,   //X86_INS_CMPLTSD,
        translate_cmplesd,   //X86_INS_CMPLESD,
        translate_cmpunordsd,//X86_INS_CMPUNORDSD,
        translate_cmpneqsd,  //X86_INS_CMPNEQSD,
        translate_cmpnltsd,  //X86_INS_CMPNLTSD,
        translate_cmpnlesd,  //X86_INS_CMPNLESD,
        translate_cmpordsd,  //X86_INS_CMPORDSD,
        NULL,                //X86_INS_CMPPS,
        translate_cmpeqps,   //X86_INS_CMPEQPS,
        translate_cmpltps,   //X86_INS_CMPLTPS,
        translate_cmpleps,   //X86_INS_CMPLEPS,
        translate_cmpunordps,//X86_INS_CMPUNORDPS,
        translate_cmpneqps,  //X86_INS_CMPNEQPS,
        translate_cmpnltps,  //X86_INS_CMPNLTPS,
        translate_cmpnleps,  //X86_INS_CMPNLEPS,
        translate_cmpordps,  //X86_INS_CMPORDPS,
        NULL,                //X86_INS_CMPPD,
        translate_cmpeqpd,   //X86_INS_CMPEQPD,
        translate_cmpltpd,   //X86_INS_CMPLTPD,
        translate_cmplepd,   //X86_INS_CMPLEPD,
        translate_cmpunordpd,//X86_INS_CMPUNORDPD,
        translate_cmpneqpd,  //X86_INS_CMPNEQPD,
        translate_cmpnltpd,  //X86_INS_CMPNLTPD,
        translate_cmpnlepd,  //X86_INS_CMPNLEPD,
        translate_cmpordpd,  //X86_INS_CMPORDPD,
        NULL,                //X86_INS_VCMPSS,
        translate_vcmpeqss,  //X86_INS_VCMPEQSS,
        translate_vcmpltss,  //X86_INS_VCMPLTSS,
        translate_vcmpless,  //X86_INS_VCMPLESS,
        translate_vcmpunordss,//X86_INS_VCMPUNORDSS,
        translate_vcmpneqss, //X86_INS_VCMPNEQSS,
        translate_vcmpnltss, //X86_INS_VCMPNLTSS,
        translate_vcmpnless, //X86_INS_VCMPNLESS,
        translate_vcmpordss, //X86_INS_VCMPORDSS,
        NULL,                //X86_INS_VCMPEQ_UQSS,
        NULL,                //X86_INS_VCMPNGESS,
        NULL,                //X86_INS_VCMPNGTSS,
        NULL,                //X86_INS_VCMPFALSESS,
        NULL,                //X86_INS_VCMPNEQ_OQSS,
        NULL,                //X86_INS_VCMPGESS,
        NULL,                //X86_INS_VCMPGTSS,
        NULL,                //X86_INS_VCMPTRUESS,
        NULL,                //X86_INS_VCMPEQ_OSSS,
        NULL,                //X86_INS_VCMPLT_OQSS,
        NULL,                //X86_INS_VCMPLE_OQSS,
        NULL,                //X86_INS_VCMPUNORD_SSS,
        NULL,                //X86_INS_VCMPNEQ_USSS,
        NULL,                //X86_INS_VCMPNLT_UQSS,
        NULL,                //X86_INS_VCMPNLE_UQSS,
        NULL,                //X86_INS_VCMPORD_SSS,
        NULL,                //X86_INS_VCMPEQ_USSS,
        NULL,                //X86_INS_VCMPNGE_UQSS,
        NULL,                //X86_INS_VCMPNGT_UQSS,
        NULL,                //X86_INS_VCMPFALSE_OSSS,
        NULL,                //X86_INS_VCMPNEQ_OSSS,
        NULL,                //X86_INS_VCMPGE_OQSS,
        NULL,                //X86_INS_VCMPGT_OQSS,
        NULL,                //X86_INS_VCMPTRUE_USSS,
        NULL,                //X86_INS_VCMPSD,
        translate_vcmpeqsd,  //X86_INS_VCMPEQSD,
        translate_vcmpltsd,  //X86_INS_VCMPLTSD,
        translate_vcmplesd,  //X86_INS_VCMPLESD,
        translate_vcmpunordsd,//X86_INS_VCMPUNORDSD,
        translate_vcmpneqsd, //X86_INS_VCMPNEQSD,
        translate_vcmpnltsd, //X86_INS_VCMPNLTSD,
        translate_vcmpnlesd, //X86_INS_VCMPNLESD,
        translate_vcmpordsd, //X86_INS_VCMPORDSD,
        NULL,                //X86_INS_VCMPEQ_UQSD,
        NULL,                //X86_INS_VCMPNGESD,
        NULL,                //X86_INS_VCMPNGTSD,
        NULL,                //X86_INS_VCMPFALSESD,
        NULL,                //X86_INS_VCMPNEQ_OQSD,
        NULL,                //X86_INS_VCMPGESD,
        NULL,                //X86_INS_VCMPGTSD,
        NULL,                //X86_INS_VCMPTRUESD,
        NULL,                //X86_INS_VCMPEQ_OSSD,
        NULL,                //X86_INS_VCMPLT_OQSD,
        NULL,                //X86_INS_VCMPLE_OQSD,
        NULL,                //X86_INS_VCMPUNORD_SSD,
        NULL,                //X86_INS_VCMPNEQ_USSD,
        NULL,                //X86_INS_VCMPNLT_UQSD,
        NULL,                //X86_INS_VCMPNLE_UQSD,
        NULL,                //X86_INS_VCMPORD_SSD,
        NULL,                //X86_INS_VCMPEQ_USSD,
        NULL,                //X86_INS_VCMPNGE_UQSD,
        NULL,                //X86_INS_VCMPNGT_UQSD,
        NULL,                //X86_INS_VCMPFALSE_OSSD,
        NULL,                //X86_INS_VCMPNEQ_OSSD,
        NULL,                //X86_INS_VCMPGE_OQSD,
        NULL,                //X86_INS_VCMPGT_OQSD,
        NULL,                //X86_INS_VCMPTRUE_USSD,
        NULL,                //X86_INS_VCMPPS,
        translate_vcmpeqps,  //X86_INS_VCMPEQPS,
        translate_vcmpltps,  //X86_INS_VCMPLTPS,
        translate_vcmpleps,  //X86_INS_VCMPLEPS,
        translate_vcmpunordps,//X86_INS_VCMPUNORDPS,
        translate_vcmpneqps, //X86_INS_VCMPNEQPS,
        translate_vcmpnltps, //X86_INS_VCMPNLTPS,
        translate_vcmpnleps, //X86_INS_VCMPNLEPS,
        translate_vcmpordps, //X86_INS_VCMPORDPS,
        NULL,                //X86_INS_VCMPEQ_UQPS,
        NULL,                //X86_INS_VCMPNGEPS,
        NULL,                //X86_INS_VCMPNGTPS,
        NULL,                //X86_INS_VCMPFALSEPS,
        NULL,                //X86_INS_VCMPNEQ_OQPS,
        NULL,                //X86_INS_VCMPGEPS,
        NULL,                //X86_INS_VCMPGTPS,
        NULL,                //X86_INS_VCMPTRUEPS,
        NULL,                //X86_INS_VCMPEQ_OSPS,
        NULL,                //X86_INS_VCMPLT_OQPS,
        NULL,                //X86_INS_VCMPLE_OQPS,
        NULL,                //X86_INS_VCMPUNORD_SPS,
        NULL,                //X86_INS_VCMPNEQ_USPS,
        NULL,                //X86_INS_VCMPNLT_UQPS,
        NULL,                //X86_INS_VCMPNLE_UQPS,
        NULL,                //X86_INS_VCMPORD_SPS,
        NULL,                //X86_INS_VCMPEQ_USPS,
        NULL,                //X86_INS_VCMPNGE_UQPS,
        NULL,                //X86_INS_VCMPNGT_UQPS,
        NULL,                //X86_INS_VCMPFALSE_OSPS,
        NULL,                //X86_INS_VCMPNEQ_OSPS,
        NULL,                //X86_INS_VCMPGE_OQPS,
        NULL,                //X86_INS_VCMPGT_OQPS,
        NULL,                //X86_INS_VCMPTRUE_USPS,
        NULL,                //X86_INS_VCMPPD,
        translate_vcmpeqpd,  //X86_INS_VCMPEQPD,
        translate_vcmpltpd,  //X86_INS_VCMPLTPD,
        translate_vcmplepd,  //X86_INS_VCMPLEPD,
        translate_vcmpunordpd,//X86_INS_VCMPUNORDPD,
        translate_vcmpneqpd, //X86_INS_VCMPNEQPD,
        translate_vcmpnltpd, //X86_INS_VCMPNLTPD,
        translate_vcmpnlepd, //X86_INS_VCMPNLEPD,
        translate_vcmpordpd, //X86_INS_VCMPORDPD,
        NULL,                //X86_INS_VCMPEQ_UQPD,
        NULL,                //X86_INS_VCMPNGEPD,
        NULL,                //X86_INS_VCMPNGTPD,
        NULL,                //X86_INS_VCMPFALSEPD,
        NULL,                //X86_INS_VCMPNEQ_OQPD,
        NULL,                //X86_INS_VCMPGEPD,
        NULL,                //X86_INS_VCMPGTPD,
        NULL,                //X86_INS_VCMPTRUEPD,
        NULL,                //X86_INS_VCMPEQ_OSPD,
        NULL,                //X86_INS_VCMPLT_OQPD,
        NULL,                //X86_INS_VCMPLE_OQPD,
        NULL,                //X86_INS_VCMPUNORD_SPD,
        NULL,                //X86_INS_VCMPNEQ_USPD,
        NULL,                //X86_INS_VCMPNLT_UQPD,
        NULL,                //X86_INS_VCMPNLE_UQPD,
        NULL,                //X86_INS_VCMPORD_SPD,
        NULL,                //X86_INS_VCMPEQ_USPD,
        NULL,                //X86_INS_VCMPNGE_UQPD,
        NULL,                //X86_INS_VCMPNGT_UQPD,
        NULL,                //X86_INS_VCMPFALSE_OSPD,
        NULL,                //X86_INS_VCMPNEQ_OSPD,
        NULL,                //X86_INS_VCMPGE_OQPD,
        NULL,                //X86_INS_VCMPGT_OQPD,
        NULL,                //X86_INS_VCMPTRUE_USPD,
        translate_ud0,       //X86_INS_UD0,
        translate_endbr32,   //X86_INS_ENDBR32,
        NULL,                //X86_INS_ENDBR64,
        NULL,                //X86_INS_ENDING, // mark the end of the list of insn
    };
    /* 1. preprocess */
    /* 2. call translate_xx function */
    int tr_func_idx = ir1_opcode(ir1) - X86_INS_INVALID;
    bool translation_success = false;
#ifdef CONFIG_DEBUG_CHECK
    if (option_debugcall) {
        if (ir1_opcode(ir1) == X86_INS_CALL) {
            fprintf(stderr, "0x%08" PRIx64 ": ", (unsigned long)linear_address(ir1->info->address));
            fprintf(stderr, "%-8s %s\n", ir1->info->mnemonic, ir1->info->op_str);
#ifdef CONFIG_SOFTMMU
            if ((unsigned long)linear_address(ir1->info->address) == debug_pc_start) {
                debug_tb = 1;
            }
            if ((unsigned long)linear_address(ir1->info->address) == debug_pc_stop) {
                debug_tb = 0;
            }
#endif
        }
    }
#endif
    if (translate_functions[tr_func_idx] == NULL) {
        fprintf(stderr, "%s %s %d error : this ins not implemented\n",
                __FILE__, __func__, __LINE__);
        ir1_opcode_dump(ir1);
        exit(-1);
    }
    translation_success = translate_functions[tr_func_idx](ir1);

#if defined(CONFIG_XTM_PROFILE) && defined(CONFIG_SOFTMMU)
    xtm_pf_inc_ts_ins_translated();
#endif

    /* 3. postprocess */
    return translation_success;
}

void tr_gen_exit_tb_load_tb_addr(IR2_OPND *tbptr, ADDR tb_addr)
{
#ifndef CONFIG_SOFTMMU /* user-mode tblink */
    if (cpu_get_guest_base() != 0) {
        /* should used from_addr here. but we need to ori with tb address
           later, to avoid assertional in translate_or, we keep it zero
           extend.
         */
        load_imm64_to_ir2(tbptr, tb_addr);
    } else {
        /* when guest == 0, tb_addr with have high 32bit not zero, to minimize
           generated code here, we load only low 32bit for current tb, and 
           rely on the fact that all tb share the same high32 bit
           Be sure to extend this tb address into full address before use it
         */
        load_imm32_to_ir2(tbptr, (uint32_t)tb_addr, ZERO_EXTENSION);
    }
#else
    if (lsenv && lsenv->tr_data && lsenv->tr_data->curr_tb) {
        TranslationBlock *tb = lsenv->tr_data->curr_tb;
        ADDR code_ptr = (ADDR)tb->tc.ptr;
        ADDR code_off = (ADDR)(lsenv->tr_data->real_ir2_inst_num << 2);
        ADDR ins_pc = code_ptr + code_off;
        /* tbptr = ins_pc + offset => offset = tbptr - ins_pc */
        int offset = (tb_addr - ins_pc) >> 2;
        if (int32_in_int20(offset))
            append_ir2_opnd1i(LISA_PCADDI, tbptr, offset);
        else
            load_imm64_to_ir2(tbptr, tb_addr);
    } else {
        load_imm64_to_ir2(tbptr, tb_addr);
    }
#endif /* CONFIG_SOFTMMU */
}

void tr_gen_exit_tb_j_tb_link(TranslationBlock *tb, int succ_id)
{
    /* 1. set a label for native code linkage */
    IR2_OPND goto_label_opnd = ir2_opnd_new_label();

    /* remember where the j instruction position */
    /* reuse jmp_reset_offset[], they will be rewritten in label_dispose */
    /* at that time we can calculate exact offset for this position */
    append_ir2_opnd1(LISA_LABEL, &goto_label_opnd);
    tb->jmp_reset_offset[succ_id] = goto_label_opnd.val;

    /* point to current j insn addr plus 8 by default, will resolve in */
    /* label_dispose */
    append_ir2_opnda(LISA_B, 0);
    append_ir2_opnd0_(lisa_nop);
}

void tr_gen_exit_tb_load_next_eip(
        int reload_eip_from_env, 
        IR2_OPND *eip_opnd, ADDRX eip, int opnd_size)
{
    if (reload_eip_from_env) {
        /* Marked by special instruction that reaches special EOB
         *
         * This mark means that we should not update the eip in env.
         * But the context switch is fixed, it will always update eip.
         *
         * So we read env->eip into next_eip_opnd here. */
        append_ir2_opnd2i(LISA_LD_W, eip_opnd, &env_ir2_opnd,
                          lsenv_offset_of_eip(lsenv));
        return;
    }

//    lsassertm(0, "load tb addr to be implemented in LoongArch.\n");
    switch (opnd_size) {
    case 32:
        load_imm32_to_ir2(eip_opnd, eip, ZERO_EXTENSION);
        break;
    case 16:
        load_imm32_to_ir2(eip_opnd, eip & 0xffff, ZERO_EXTENSION);
        break;
    default: lsassert(0); break;
    }
}

void tr_gen_exit_tb_j_context_switch(
        IR2_OPND *tbptr, int can_link, int succ_id)
{
//    lsassertm(0, "j cs to be implemented in LoongArch.\n");
    if (can_link) {
        append_ir2_opnd2i(LISA_ORI, &ret0_ir2_opnd, tbptr, succ_id);
    } else {
        append_ir2_opnd2i(LISA_ORI, &ret0_ir2_opnd, &zero_ir2_opnd, succ_id);
    }
    /* jump to context_switch_native_to_bt */
    void *code_buf = lsenv->tr_data->curr_tb->tc.ptr;
    int offset = lsenv->tr_data->real_ir2_inst_num << 2;
    append_ir2_opnda(LISA_B, (context_switch_native_to_bt
                              - (ADDR)code_buf - offset) >> 2);
}

/* Should always use TB-Link. */
void tr_gen_eob_if_tb_too_large(IR1_INST *pir1)
{
    if (!pir1) return;

    if (ir1_is_tb_ending(pir1)) return;

//    tr_adjust_em();

    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;

#ifdef CONFIG_SOFTMMU
    lsassertm(!tb->extra_tb->sys_eob_pir1,
            "TB too large should not exist with system EOB.\n");
#endif

    int can_link = option_tb_link;
#ifdef CONFIG_SOFTMMU
    tr_gen_eob();
    CPUState *cpu = env_cpu(lsenv->cpu_state);
    if (td->sys.bp_hit || cpu->singlestep_enabled) {
        current_cpu->exception_index = EXCP_DEBUG;
        can_link = 0;
    }
#endif

    IR2_OPND tbptr = ra_alloc_dbt_arg1(); /* t8($24) */
    IR2_OPND next_eip_opnd = ra_alloc_dbt_arg2(); /* t9($25) */
    int succ_id = 1;

    /* t8: This TB's address */
    tr_gen_exit_tb_load_tb_addr(&tbptr, (ADDR)tb);

    if (can_link) {
        tr_gen_exit_tb_j_tb_link(tb, succ_id);
    }

    /* t9: next x86 instruction's address */
    ADDRX next_eip = ir1_addr_next(pir1);
#ifdef CONFIG_SOFTMMU
    tr_gen_exit_tb_load_next_eip(td->ignore_eip_update,
            &next_eip_opnd, next_eip, 32);
#else
    tr_gen_exit_tb_load_next_eip(0, &next_eip_opnd, next_eip, 32);
#endif

    /* jump to context switch */
    tr_gen_exit_tb_j_context_switch(&tbptr, can_link, succ_id);
}

/* Details of exit-tb
 *   > EOB: End-Of-TB worker  tr_gen_eob()
 *   > Load TB addr   $t8     tr_gen_exit_tb_load_tb_addr()
 *   > Load next EIP  $t9     tr_gen_exit_tb_load_next_eip()
 *   > TB-Link j 0            tr_gen_exit_tb_j_tb_link()
 *   > Context Switch         tr_gen_exit_tb_j_context_switch()
 *
 * Every exit-tb must contains:
 *   > EOB
 *   > Context Switch
 *
 * TB-Link j 0
 *   > if can_link
 *   > if not indirect jmp(jmp/call/ret)
 *
 * Load TB addr
 *   > always gen it for context switch
 *   > when no lsfpu, gen before TB-Link for FPU ratate
 *   > when FastCS, gen before TB-Link for context check
 *   > when cross-page-check, gen before TB-Link
 *     > for cross-page direct TB-Link
 *     > for indirect jmp(jmp/call/ret)
 *     > to do cross-page check
 */
void tr_generate_exit_tb(IR1_INST *branch, int succ_id)
{
    IR2_OPND next_eip_opnd = ra_alloc_dbt_arg2();
    int      next_eip_size = 32;
    ADDRX    next_eip;

    IR1_OPCODE opcode = ir1_opcode(branch);
    int can_link = option_tb_link;

    TranslationBlock *tb = lsenv->tr_data->curr_tb;
#ifdef CONFIG_SOFTMMU
    TRANSLATION_DATA *td = lsenv->tr_data;
    /* EOB will be generated before this TB's each exit (at most two exit) */
    if (!ir1_is_branch(branch)) {
        tr_gen_eob();
    }
    /* debug support : when hit break point, do NOT TB-link */
    CPUState *cpu = env_cpu(lsenv->cpu_state);
    if (td->sys.bp_hit || cpu->singlestep_enabled) {
        current_cpu->exception_index = EXCP_DEBUG;
        /* don't allow link to next tb, we already return to qemu to allow
           in time exception check
         */
        can_link = 0;
    }

    /* Cross Pgae Check */
    tb->extra_tb->tb_need_cpc[succ_id] =
        xtm_tb_need_cpc(tb, branch, succ_id);
#endif

    /* Load this TB's address into $t8
     * 
     *        |       | need TB addr   |
     * FastCS | LSFPU | for jmp glue ? | Reason
     * -----------------------------------------------------
     *    X   |   X   |     YES        | FPU rotate
     *    X   |   O   |     NO         |
     *    O   |   X   |     YES        | TB's fast cs mask
     *    O   |   O   |     YES        | TB's fast cs mask
     */

    IR2_OPND tbptr = ra_alloc_dbt_arg1(); /* t8($24) */
    ADDR tb_addr = (ADDR)tb;

#ifdef CONFIG_XTM_FAST_CS
    int need_tb_addr_for_jmp_glue = 1;
#else
    int need_tb_addr_for_jmp_glue = option_lsfpu ? 0 : 1;
#endif
#ifdef CONFIG_SOFTMMU
    if (xtm_cpc_enabled() && xtm_tb_need_cpc(tb, branch, succ_id)) {
        need_tb_addr_for_jmp_glue = 1;
    }
#endif

    if (!need_tb_addr_for_jmp_glue)
        goto IGNORE_LOAD_TB_ADDR_FOR_JMP_GLUE;

#ifndef CONFIG_SOFTMMU
    tr_gen_exit_tb_load_tb_addr(&tbptr , tb_addr);
#else
    if (!ir1_is_branch(branch))
        tr_gen_exit_tb_load_tb_addr(&tbptr, tb_addr);
    else {
        if (xtm_cpc_enabled() &&
            option_lsfpu &&
            need_tb_addr_for_jmp_glue)
        {
            tr_gen_exit_tb_load_tb_addr(&tbptr, tb_addr);
        }
    }
#endif

IGNORE_LOAD_TB_ADDR_FOR_JMP_GLUE:

    /* Generate 'j 0' if not indrect jmp for TB-Link */
    if (can_link &&
            !ir1_is_indirect_jmp(branch) &&
            !ir1_is_indirect_call(branch) &&
            opcode != X86_INS_RET) {
        tr_gen_exit_tb_j_tb_link(tb, succ_id);
    }

    if (!need_tb_addr_for_jmp_glue) {
        /* Always need to load TB addr for context switch : for TB-Lin */
        tr_gen_exit_tb_load_tb_addr(&tbptr, tb_addr);
    }

    /* Standard process:
     * 1. prepare t8 and t9
     *    > t8: Current TranslationBlock's address
     *    > t9: Next TranslationBlock's EIP 
     * 2. prepare the return value (v0) for TB-link
     * 3. jump to context switch native to bt */
    switch (opcode) {
    case X86_INS_CALL:
        if(ir1_is_indirect_call(branch))
            goto indirect_call;

        next_eip = ir1_target_addr(branch);
#ifdef CONFIG_SOFTMMU
        if (ir1_addr_size(branch) == 2 ||
            ir1_opnd_size(ir1_get_opnd(branch, 0)) == 16) {
            next_eip_size = 16;
        }
#endif
        tr_gen_exit_tb_load_next_eip(0, &next_eip_opnd, next_eip, next_eip_size);

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_PROFILE)
        if (option_monitor_jc) {
            /* monitor JC mark direct jmp lookup */
    lsassertm(0, "monitor jc to be implemented in LoongArch.\n");
            /*append_ir2_opnd2i(mips_ori, &arg0_ir2_opnd, &zero_ir2_opnd, 0x1);*/
            /*append_ir2_opnd2i(mips_sb, &arg0_ir2_opnd, &env_ir2_opnd,*/
                    /*lsenv_offset_of_pf_data_jc_is_jmpdr(lsenv));*/
        }
#endif

        tr_gen_exit_tb_j_context_switch(&tbptr, can_link, succ_id);
        break;

#ifdef CONFIG_SOFTMMU
    case X86_INS_LJMP:
        /* only ptr16:16/ptr16:32 is supported now */
        if (td->sys.pe && !td->sys.vm86) {
            lsassert(td->ignore_eip_update == 1);
            tr_gen_exit_tb_load_next_eip(1, &next_eip_opnd, 0, 0);
        }
        else if (!ir1_opnd_is_mem(ir1_get_opnd(branch, 0))) {
            next_eip = ir1_opnd_uimm(ir1_get_opnd(branch, 1));
            next_eip_size = ir1_opnd_size(ir1_get_opnd(branch, 1));
            tr_gen_exit_tb_load_next_eip(0, &next_eip_opnd, next_eip, next_eip_size);
        }

        /* Always no TB-link for jmp far */
        tr_gen_exit_tb_j_context_switch(NULL, 0, succ_id);
        break;
#endif

    case X86_INS_JMP:
        if(ir1_is_indirect_jmp(branch))
            goto indirect_jmp;

        next_eip = ir1_target_addr(branch);
#ifndef CONFIG_SOFTMMU /* user-mode */
        tr_gen_exit_tb_load_next_eip(0, &next_eip_opnd, next_eip, 32);
#else   /* sys-mode */
        next_eip_size = ir1_opnd_size(ir1_get_opnd(branch, 0));
        tr_gen_exit_tb_load_next_eip(td->ignore_eip_update,
                &next_eip_opnd, next_eip, next_eip_size);
#endif

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_PROFILE)
        if (option_monitor_jc) {
            /* monitor JC mark direct jmp lookup */
    lsassertm(0, "monitor jc to be implemented in LoongArch.\n");
            /*append_ir2_opnd2i(mips_ori, &arg0_ir2_opnd, &zero_ir2_opnd, 0x1);*/
            /*append_ir2_opnd2i(mips_sb, &arg0_ir2_opnd, &env_ir2_opnd,*/
                    /*lsenv_offset_of_pf_data_jc_is_jmpdr(lsenv));*/
        }
#endif
        tr_gen_exit_tb_j_context_switch(&tbptr, can_link, succ_id);
        break;

    case X86_INS_RET:
indirect_call :
indirect_jmp :

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_PROFILE)
        if (option_monitor_jc) {
            /* monitor JC mark jmpin lookup */
    lsassertm(0, "monitor jc to be implemented in LoongArch.\n");
            /*append_ir2_opnd2i(mips_ori, &arg0_ir2_opnd, &zero_ir2_opnd, 0x1);*/
            /*append_ir2_opnd2i(mips_sb, &arg0_ir2_opnd, &env_ir2_opnd,*/
                    /*lsenv_offset_of_pf_data_jc_is_jmpin(lsenv));*/
        }
#endif

        if (can_link) {
            /* store eip (in $11) into env */
            append_ir2_opnd2i(LISA_ST_W, &next_eip_opnd,
                              &env_ir2_opnd, lsenv_offset_of_eip(lsenv));
            /* tb->extra_tb.next_tb should be 0 so glue code know we are */
            /* indirect jmp append_ir2_opnd2i(mips_store_addr, &*/
            /* zero_ir2_opnd, env_ir2_opnd, */
            /*                   offsetof(CPUX86State, extra_tb) + */
            /*                   offsetof(struct ExtraBlock, next_tb)); */
            void *code_buf = lsenv->tr_data->curr_tb->tc.ptr;
            int offset = lsenv->tr_data->real_ir2_inst_num << 2;
            append_ir2_opnda(LISA_B, (native_jmp_glue_2
                                      - (ADDR)code_buf - offset) >> 2);

        } else {
            tr_gen_exit_tb_j_context_switch(NULL, 0, succ_id);
        }
        break;
    case X86_INS_JE:
    case X86_INS_JNE:
    case X86_INS_JS:
    case X86_INS_JNS:
    case X86_INS_JB:
    case X86_INS_JAE:
    case X86_INS_JO:
    case X86_INS_JNO:
    case X86_INS_JBE:
    case X86_INS_JA:
    case X86_INS_JP:
    case X86_INS_JNP:
    case X86_INS_JL:
    case X86_INS_JGE:
    case X86_INS_JLE:
    case X86_INS_JG:
    case X86_INS_JCXZ:
    case X86_INS_JECXZ:
    case X86_INS_JRCXZ:
    case X86_INS_LOOP:
    case X86_INS_LOOPE:
    case X86_INS_LOOPNE:
        next_eip =
            succ_id ? ir1_target_addr(branch) : ir1_addr_next(branch);
#ifdef CONFIG_SOFTMMU
        next_eip_size = ir1_opnd_size(ir1_get_opnd(branch, 0));
#endif
        tr_gen_exit_tb_load_next_eip(0, &next_eip_opnd, next_eip, next_eip_size);

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_PROFILE)
        if (option_monitor_jc) {
            /* monitor JC mark direct jmp lookup */
    lsassertm(0, "monitor jc to be implemented in LoongArch.\n");
//            append_ir2_opnd2i(mips_ori, &arg0_ir2_opnd, &zero_ir2_opnd, 0x1);
//            append_ir2_opnd2i(mips_sb, &arg0_ir2_opnd, &env_ir2_opnd,
//                    lsenv_offset_of_pf_data_jc_is_jmpdr(lsenv));
        }
#endif

        tr_gen_exit_tb_j_context_switch(&tbptr, can_link, succ_id);
        break;
    default:
        lsassertm(0, "not implement.\n");
    }
}

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_XTM_FAST_CS)

void tr_save_registers_to_env(
        uint8_t gpr_to_save,
        uint8_t fpr_to_save, int save_top,
        uint8_t xmm_lo_to_save,
        uint8_t xmm_hi_to_save,
        uint8_t vreg_to_save)
{
    lsassertm(0, "fastcs save registers to be implemented in LoongArch.\n");
    /* 1. GPR */
    tr_save_gprs_to_env(gpr_to_save);
    /* 2. virtual registers */
    tr_save_vreg_to_env(vreg_to_save);

    /* 3. load the fast context switch mask
     *    
     *    mask[0] : FPU need to save
     *    mask[1] : XMM need to save
     */
    IR2_OPND fast_cs_mask = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_lwu, &fast_cs_mask, &env_ir2_opnd,
//            lsenv_offset_of_fast_cs_mask(lsenv));

    /* 4. check for FPU save */
    IR2_OPND tmp = ra_alloc_itemp();
    IR2_OPND label_no_fpu = ir2_opnd_new_label();
//    append_ir2_opnd2i(mips_andi, &tmp, &fast_cs_mask, XTM_FAST_CS_MASK_FPU);
//    append_ir2_opnd3(mips_beq, &tmp, &zero_ir2_opnd, &label_no_fpu);
    ra_free_temp(&tmp);
    /* 4.1 not branch, save FPU */
    tr_save_fprs_to_env(fpr_to_save, save_top);

//    append_ir2_opnd1(mips_label, &label_no_fpu);

    /* 5. check for XMM save */
    tmp = ra_alloc_itemp();
    IR2_OPND label_no_xmm = ir2_opnd_new_label();
//    append_ir2_opnd2i(mips_andi, &tmp, &fast_cs_mask, XTM_FAST_CS_MASK_XMM);
//    append_ir2_opnd3(mips_beq, &tmp, &zero_ir2_opnd, &label_no_xmm);
    ra_free_temp(&tmp);
    ra_free_temp(&fast_cs_mask);
    /* 5.1 not branch, save XMM */
    tr_save_xmms_to_env(xmm_lo_to_save, xmm_hi_to_save);

//    append_ir2_opnd1(mips_label, &label_no_xmm);
}

void tr_load_registers_from_env(
        uint8_t gpr_to_load,
        uint8_t fpr_to_load, int load_top,
        uint8_t xmm_lo_to_load,
        uint8_t xmm_hi_to_load,
        uint8_t vreg_to_load)
{
    lsassertm(0, "fastcs load registers to be implemented in LoongArch.\n");
    /* 1. virtual registers */
    tr_load_vreg_from_env(vreg_to_load);

    /* 2. GPR */
    tr_load_gprs_from_env(gpr_to_load);

    /* 3. load the fast context switch mask
     *    
     *    mask[0] : FPU need to load 
     *    mask[1] : XMM need to load
     */
    IR2_OPND fast_cs_mask = ra_alloc_itemp();
//    append_ir2_opnd2i(mips_lwu, &fast_cs_mask, &env_ir2_opnd,
//            lsenv_offset_of_fast_cs_mask(lsenv));

    /* 4. check for FPU load */
    IR2_OPND tmp = ra_alloc_itemp();
    IR2_OPND label_no_fpu = ir2_opnd_new_label();
//    append_ir2_opnd2i(mips_andi, &tmp, &fast_cs_mask, XTM_FAST_CS_MASK_FPU);
//    append_ir2_opnd3(mips_beq, &tmp, &zero_ir2_opnd, &label_no_fpu);
    ra_free_temp(&tmp);
    /* 4.1 not branch, load FPU */
    tr_load_fprs_from_env(fpr_to_load, load_top);

//    append_ir2_opnd1(mips_label, &label_no_fpu);

    /* 5. check for XMM load */
    tmp = ra_alloc_itemp();
    IR2_OPND label_no_xmm = ir2_opnd_new_label();
//    append_ir2_opnd2i(mips_andi, &tmp, &fast_cs_mask, XTM_FAST_CS_MASK_XMM);
//    append_ir2_opnd3(mips_beq, &tmp, &zero_ir2_opnd, &label_no_xmm);
    ra_free_temp(&tmp);
    ra_free_temp(&fast_cs_mask);
    /* 5.1 not branch, load XMM */
    tr_load_xmms_from_env(xmm_lo_to_load, xmm_hi_to_load);

//    append_ir2_opnd1(mips_label, &label_no_xmm);
}

#else

void tr_save_registers_to_env(
        uint8_t gpr_to_save,
        uint8_t fpr_to_save, int save_top,
        uint8_t xmm_lo_to_save,
        uint8_t xmm_hi_to_save,
        uint8_t vreg_to_save)
{
    /* 1. GPR */
    tr_save_gprs_to_env(gpr_to_save);

    /* 2. FPR (MMX) */
    tr_save_fprs_to_env(fpr_to_save, save_top);

    /* 3. XMM */
    tr_save_xmms_to_env(xmm_lo_to_save, xmm_hi_to_save);

    /* 4. virtual registers */
    tr_save_vreg_to_env(vreg_to_save);
}

void tr_load_registers_from_env(
        uint8_t gpr_to_load,
        uint8_t fpr_to_load, int load_top,
        uint8_t xmm_lo_to_load,
        uint8_t xmm_hi_to_load,
        uint8_t vreg_to_load)
{
    /* 4. virtual registers */
    tr_load_vreg_from_env(vreg_to_load);

    /* 3. XMM */
    tr_load_xmms_from_env(xmm_lo_to_load, xmm_hi_to_load);

    /* 2. FPR (MMX) */
    tr_load_fprs_from_env(fpr_to_load, load_top);

    /* 1. GPR */
    tr_load_gprs_from_env(gpr_to_load);
}

#endif

void tr_save_gprs_to_env(uint8_t mask)
{
    int i = 0;
    for (i = 0; i < 8; ++i) {
        if (BITS_ARE_SET(mask, 1 << i)) {
            IR2_OPND gpr_opnd = ra_alloc_gpr(i);
            append_ir2_opnd2i(LISA_ST_W, &gpr_opnd, &env_ir2_opnd,
                              lsenv_offset_of_gpr(lsenv, i));
        }
    }
}

void tr_save_fprs_to_env(uint8_t mask, int save_top)
{
    int i = 0;

    /* 1. save FPU top: curr_top is rotated */
    if (save_top) {
        tr_gen_save_curr_top();
        tr_fpu_disable_top_mode();
    }
    /* 2. save FPRs: together with FPU top */
    for (i = 0; i < 8; i++) {
        if (BITS_ARE_SET(mask, 1 << i)) {
            IR2_OPND mmx_opnd = ra_alloc_mmx(i);
            append_ir2_opnd2i(LISA_FST_D, &mmx_opnd, &env_ir2_opnd,
                              lsenv_offset_of_mmx(lsenv, i));
        }
    }
}

void tr_save_xmms_to_env(uint8_t lo_mask, uint8_t hi_mask)
{
//    lsassertm(0, "save XMM to be implemented in LoongArch.\n");
    int i = 0;
    for (i = 0; i < 8; i++) {
        if (BITS_ARE_SET(lo_mask, 1 << i)){
            IR2_OPND xmm_opnd = ra_alloc_xmm(i);
            append_ir2_opnd2i(LISA_VST, &xmm_opnd, &env_ir2_opnd,
                              lsenv_offset_of_xmm(lsenv, i));
        }
    }
}

void tr_save_vreg_to_env(uint8_t mask)
{
    int i = 0;
    for (i = 0; i < 8; ++i) {
        if (BITS_ARE_SET(mask, 1 << i)) {
            IR2_OPND vreg_opnd = ra_alloc_vreg(i);
            append_ir2_opnd2i(LISA_ST_D, &vreg_opnd, &env_ir2_opnd,
                              lsenv_offset_of_vreg(lsenv, i));
        }
    }
}

void tr_load_gprs_from_env(uint8_t mask)
{
//    lsassertm(0, "load GPRs to be implemented in LoongArch.\n");
    int i = 0;
    for (i = 0; i < 8; ++i) {
        if (BITS_ARE_SET(mask, 1 << i)) {
            IR2_OPND gpr_opnd = ra_alloc_gpr(i);
            append_ir2_opnd2i(LISA_LD_W, &gpr_opnd, &env_ir2_opnd,
                lsenv_offset_of_gpr(lsenv, i));
        }
    }
}

void tr_load_fprs_from_env(uint8_t mask, int load_top)
{
    int i = 0;

    for (i = 0; i < 8; i++) {
        if (BITS_ARE_SET(mask, 1 << i)) {
            IR2_OPND mmx_opnd = ra_alloc_mmx(i);
            append_ir2_opnd2i(LISA_FLD_D, &mmx_opnd, &env_ir2_opnd,
                              lsenv_offset_of_mmx(lsenv, i));
        }
    }

    if (load_top && option_lsfpu) {
        IR2_OPND top = ra_alloc_itemp();
        tr_load_lstop_from_env(&top);
        ra_free_temp(&top);
        /* this can fetch only top of translation time, not runtime */
        tr_fpu_enable_top_mode();
    }
}

void tr_load_xmms_from_env(uint8_t lo_mask, uint8_t hi_mask)
{
//    lsassertm(0, "load XMMs to be implemented in LoongArch.\n");
    int i = 0;
    for (i = 0; i < 8; i++) {
        if (BITS_ARE_SET(lo_mask, 1 << i)){
            IR2_OPND xmm_opnd = ra_alloc_xmm(i);
            append_ir2_opnd2i(LISA_VLD, &xmm_opnd, &env_ir2_opnd,
                              lsenv_offset_of_xmm(lsenv, i));
        }
    }
}

void tr_load_vreg_from_env(uint8_t mask)
{
    int i = 0;
    for (i = 0; i < 8; ++i) {
        if (BITS_ARE_SET(mask, 1 << i)) {
            IR2_OPND vreg_opnd = ra_alloc_vreg(i);
            append_ir2_opnd2i(LISA_LD_D, &vreg_opnd, &env_ir2_opnd,
                              lsenv_offset_of_vreg(lsenv, i));
        }
    }
}

IR2_INST *tr_gen_call_to_helper(ADDR func_addr)
{
    IR2_OPND func = ra_alloc_itemp();
    load_addr_to_ir2(&func, (ADDR)func_addr);
    append_ir2_opnd1_(lisa_call, &func);
    ra_free_temp(&func);
    return NULL;
}

void tr_gen_call_to_helper_prologue(int use_fp)
{
    tr_save_registers_to_env(
            GPR_USEDEF_TO_SAVE,
            FPR_USEDEF_TO_SAVE, 1,
            XMM_LO_USEDEF_TO_SAVE, XMM_HI_USEDEF_TO_SAVE,
            0x1|options_to_save()); 

#ifdef CONFIG_SOFTMMU
    tr_save_eflags();
#endif
    if (use_fp) {
        IR2_OPND func = ra_alloc_itemp();
        load_addr_to_ir2(&func, (ADDR)convert_fpregs_64_to_x80);
        append_ir2_opnd1_(lisa_call, &func);

        load_addr_to_ir2(&func, (ADDR)update_fp_status);
        append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
        append_ir2_opnd1_(lisa_call, &func);
        ra_free_temp(&func);
    }
}

void tr_gen_call_to_helper_epilogue(int use_fp)
{
#ifdef CONFIG_SOFTMMU
    tr_load_eflags(0);
#endif
    if (use_fp) {
        IR2_OPND func = ra_alloc_itemp();
        load_addr_to_ir2(&func, (ADDR)convert_fpregs_x80_to_64);
        append_ir2_opnd1_(lisa_call, &func);
        ra_free_temp(&func);
    }

    tr_load_registers_from_env(
            GPR_USEDEF_TO_SAVE,
            FPR_USEDEF_TO_SAVE, 1,
            XMM_LO_USEDEF_TO_SAVE, XMM_HI_USEDEF_TO_SAVE,
            0x1|options_to_save());
}
/* helper with zero arg */
void tr_gen_call_to_helper0(ADDR func_addr)
{
    tr_gen_call_to_helper_prologue(0);
    IR2_OPND func_addr_opnd = ra_alloc_itemp();
    load_addr_to_ir2(&func_addr_opnd, (ADDR)func_addr);
    append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    ra_free_temp(&func_addr_opnd);
    tr_gen_call_to_helper_epilogue(0);
}
/* helper with 1 default arg(CPUArchState*) */ 
void tr_gen_call_to_helper1(ADDR func, int use_fp)
{
    tr_gen_call_to_helper_prologue(use_fp);
    IR2_OPND func_addr_opnd = ra_alloc_itemp();
    load_addr_to_ir2(&func_addr_opnd, (ADDR)func);
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    ra_free_temp(&func_addr_opnd);
    tr_gen_call_to_helper_epilogue(use_fp);
}
/* helper with 2 arg(CPUArchState*, int) */ 
void tr_gen_call_to_helper2(ADDR func, int arg2)
{
    tr_gen_call_to_helper_prologue(1);
    IR2_OPND func_addr_opnd = ra_alloc_itemp();
    load_addr_to_ir2(&func_addr_opnd, func);
    load_imm32_to_ir2(&arg1_ir2_opnd, arg2, SIGN_EXTENSION);
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    ra_free_temp(&func_addr_opnd);
    tr_gen_call_to_helper_epilogue(1);
}

/* Part of tr_sys_helper.c */
#ifdef CONFIG_SOFTMMU
/* Only used in tr_sys_gen_call_to_helper_prologue()
 * Called after tr_save_registers_to_env() */
void tr_cvt_fp64_to_80(void)
{
    /* 1. call convert_fpregs_x64_to_80 */
    IR2_OPND func_addr_opnd = ra_alloc_itemp();
    load_addr_to_ir2(&func_addr_opnd, (ADDR)convert_fpregs_64_to_x80);
    append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    /* 2. call update_fp_status */
    load_addr_to_ir2(&func_addr_opnd, (ADDR)update_fp_status);
    append_ir2_opnd2_(lisa_mov, &arg0_ir2_opnd, &env_ir2_opnd);
    append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    ra_free_temp(&func_addr_opnd);
}
/* Only used in tr_sys_gen_call_to_helper_epilogue()
 * Called before tr_load_register_from_env() */
void tr_cvt_fp80_to_64(void)
{
    /* 1. save return value */
    append_ir2_opnd2i(LISA_ST_D, &ret0_ir2_opnd, &env_ir2_opnd,
                      lsenv_offset_of_mips_iregs(lsenv, 0x2));
    /* 2. call convert_fpregs_x80_to_64 */
    IR2_OPND func_addr_opnd = ra_alloc_itemp();
    load_addr_to_ir2(&func_addr_opnd, (ADDR)convert_fpregs_x80_to_64);
    append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    ra_free_temp(&func_addr_opnd);
 
    /* 3. restore return value */
    append_ir2_opnd2i(LISA_LD_D, &ret0_ir2_opnd, &env_ir2_opnd,
                      lsenv_offset_of_mips_iregs(lsenv, 0x2));
}
#endif
