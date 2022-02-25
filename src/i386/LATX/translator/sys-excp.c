#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>

/*
 * Exception Check during translation.
 *
 * Generate native code that call the helper to
 * raise exception.
 *
 * target/i386/excp_helper.c
 * void helper_raise_exception(
 *      CPUX86State *env,
 *      int exception_index)
 *
 * Note1:   Exception must always be raised during execution.
 *
 * Note2:   Some exceptions that are related to memory access
 *          are handled inside the QEMU's softmmu helper.
 *          <#PF>   page fault
 *
 * Currently Ignored Exception:
 *          <#UD>   LOCK prefix
 *          <#GP>   memory access outside segment limit
 *          <#SS>   memory access outside SS segment limit
 */

/* Exception Number is defined in target/i386/cpu.h */
static void latxs_tr_gen_raise_exception(
        IR1_INST *ir1, int excp_index, int end)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    td->need_save_currtb_for_int = 0;

    /* 1. save the address of the instruction caused exception into eip */
    ADDRX ir1_eip = ir1_addr(ir1);
    IR2_OPND eip_reg = latxs_ra_alloc_itemp();
    latxs_load_addrx_to_ir2(&eip_reg, ir1_eip);
#ifdef TARGET_X86_64
    latxs_append_ir2_opnd2i(LISA_ST_D, &eip_reg,
            &latxs_env_ir2_opnd, lsenv_offset_of_eip(lsenv));
#else
    latxs_append_ir2_opnd2i(LISA_ST_W, &eip_reg,
            &latxs_env_ir2_opnd, lsenv_offset_of_eip(lsenv));
#endif
    latxs_ra_free_temp(&eip_reg);

    /*
     * 2. target/i386/excp_helper.c
     * void helper_raise_exception(
     *      CPUX86State *env,
     *      int exception_index)
     */
    helper_cfg_t cfg = default_helper_cfg;
    latxs_tr_gen_call_to_helper2_cfg(
            (ADDR)helper_raise_exception,
            excp_index, cfg);

    /* 3. Exception in native code should never return */
    latxs_tr_gen_infinite_loop();

    if (end) {
        lsenv->tr_data->end_with_exception = 1;
    }

    td->need_save_currtb_for_int = sigint_enabled();
}

static void latxs_tr_gen_raise_exception_addr(
        ADDRX addr, int excp_index, int end)
{
    /* 1. save exception instruction's EIP */
    IR2_OPND eip_reg = latxs_ra_alloc_itemp();
    latxs_load_addrx_to_ir2(&eip_reg, addr);
#ifdef TARGET_X86_64
    latxs_append_ir2_opnd2i(LISA_ST_D, &eip_reg,
            &latxs_env_ir2_opnd, lsenv_offset_of_eip(lsenv));
#else
    latxs_append_ir2_opnd2i(LISA_ST_W, &eip_reg,
            &latxs_env_ir2_opnd, lsenv_offset_of_eip(lsenv));
#endif

    /* 2. call helper raise exception */
    helper_cfg_t cfg = default_helper_cfg;
    latxs_tr_gen_call_to_helper2_cfg(
            (ADDR)helper_raise_exception,
            excp_index, cfg);

    /* 3. Exception in native code should never return */
    latxs_tr_gen_infinite_loop();

    if (end) {
        lsenv->tr_data->end_with_exception = 1;
    }
}

/*
 * Macro consists of three words
 *
 * 1> EC: stands for Exception Checking
 * 2> checking condition
 *    generate excption when condition is false
 *    # CPLEQ : current CPL equal to the given CPL
 *    # IOPLEQ : current IOPL equal to the given num
 *    # CPLLEIOPL : current CPL <= current IOPL
 *    # PE : CPU is running in protect mode
 * 3> generated exception
 *    # GP : general exception
 *    # ILLOP : illegal operation exception
 */
#define EC_CPLEQ_GP_AND_RETURN(num) do { \
    if (td->sys.cpl != num) { \
        latxs_tr_gen_excp_gp(pir1, 1); \
        return 1; \
    } \
} while (0)

#define EC_IOPLEQ_GP_AND_RETURN(num) do { \
    if (td->sys.iopl != num) { \
        latxs_tr_gen_excp_gp(pir1, 1); \
        return 1; \
    } \
} while (0)

#define EC_CPLLEIOPL_GP_AND_RETURN() do { \
    if (!(td->sys.cpl <= td->sys.iopl)) { \
        latxs_tr_gen_excp_gp(pir1, 1); \
        return 1; \
    } \
} while (0)

#define EC_PE_ILLOP_AND_RETURN() do { \
    if (!(td->sys.pe && !td->sys.vm86)) { \
        latxs_tr_gen_excp_illegal_op(pir1, 1); \
        return 1; \
    } \
} while (0)

#define EC_PE_GP_AND_RETURN() do { \
    if (!td->sys.pe) { \
        latxs_tr_gen_excp_gp(pir1, 1); \
        return 1; \
    } \
} while (0)


#define GEN_EXCP_ILLOP_AND_RETURN() do { \
    latxs_tr_gen_excp_illegal_op(pir1, 1); \
    return 1; \
} while (0)

#define GEN_EXCP_GP_AND_RETURN() do { \
    latxs_tr_gen_excp_gp(pir1, 1); \
    return 1; \
} while (0)

#define GEN_EXCP_PREX_AND_RETURN() do { \
    latxs_tr_gen_excp_prex(pir1, 1); \
    return 1; \
} while (0)

int latxs_tr_gen_fp_common_excp_check(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (td->sys.flags & (HF_TS_MASK | HF_EM_MASK)) {
        GEN_EXCP_PREX_AND_RETURN();
    }

    return 0;
}

int latxs_tr_gen_sse_common_excp_check(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (td->sys.flags & HF_TS_MASK) {
        GEN_EXCP_PREX_AND_RETURN();
    }

    if (td->sys.flags & HF_EM_MASK) {
        GEN_EXCP_ILLOP_AND_RETURN();
    }

    return 0;
}

int latxs_tr_gen_excp_check(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    IR1_OPND *opnd0;
    IR1_OPND *opnd1;

    IR1_OPCODE opc = ir1_opcode(pir1);
    switch (opc) {
    /* Only allowed when CPL = 0 */
    case X86_INS_HLT:
    case X86_INS_LIDT:
    case X86_INS_LGDT:
    case X86_INS_INVD:
    case X86_INS_WBINVD:
    case X86_INS_LMSW:
    case X86_INS_CLTS:
    case X86_INS_WRMSR:
    case X86_INS_RDMSR:
        EC_CPLEQ_GP_AND_RETURN(0);
        break;
    /*
     * Only not allowed in Real-Address mode
     * > [OK]  PE && !vm86
     * > [OK]  PE &&  vm86
     * > [NO] !PE (real mode)
     */
    case X86_INS_SYSENTER:
    case X86_INS_SYSEXIT:
        EC_PE_GP_AND_RETURN();
        break;
    /* Only allowed in PE (not vm86) */
    case X86_INS_VERR:
    case X86_INS_VERW:
    case X86_INS_SLDT:
    case X86_INS_STR:
    case X86_INS_ARPL:
    case X86_INS_LAR:
    case X86_INS_LSL:
        EC_PE_ILLOP_AND_RETURN();
        break;
    /* Only allowed in PE (not vm86) and CPL = 0 */
    case X86_INS_LLDT:
    case X86_INS_LTR:
        if (!td->sys.pe || td->sys.vm86) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        EC_CPLEQ_GP_AND_RETURN(0);
        break;
    /*
     * Only allowed in :
     * 1. PE not vm86
     * 2. PE and vm86 and IOPL is 3
     */
    case X86_INS_POPF:
    case X86_INS_POPFD:
    case X86_INS_PUSHF:
    case X86_INS_PUSHFD:
    case X86_INS_INT:
    case X86_INS_IRET:
    case X86_INS_IRETD:
        if (td->sys.pe && td->sys.vm86) {
            EC_IOPLEQ_GP_AND_RETURN(3);
        }
        break;
    /*
     * Only allowed in :
     * 1. PE (not vm86) or Real && CPL <= IOPL
     * 2. vm86 mode and IOPL is 3
     */
    case X86_INS_CLI:
    case X86_INS_STI:
        if (!td->sys.vm86) {
            EC_CPLLEIOPL_GP_AND_RETURN();
        } else {
            EC_IOPLEQ_GP_AND_RETURN(3);
        }
        break;
    /*
     * Mov instruction :
     * 1. mov to/from CR/DR : CPL is 0
     * 3. mov to      seg   : ES,   SS,DS,FS,GS
     * 4. mov    from seg   : ES,CS,SS,DS,FS,GS
     */
    case X86_INS_MOV:
        opnd0 = ir1_get_opnd(pir1, 0);
        opnd1 = ir1_get_opnd(pir1, 1);
        /* mov to/from control register */
        if (latxs_ir1_opnd_is_cr(opnd0) || latxs_ir1_opnd_is_cr(opnd1)) {
            EC_CPLEQ_GP_AND_RETURN(0);
        }
        /* mov to/from debug register */
        if (latxs_ir1_opnd_is_dr(opnd0) || latxs_ir1_opnd_is_dr(opnd1)) {
            EC_CPLEQ_GP_AND_RETURN(0);
        }
        /* mov to segment register */
        if (ir1_opnd_is_seg(opnd0)) {
            int seg_reg = ir1_opnd_base_reg_num(opnd0);
            if (seg_reg >= 6 || seg_reg == cs_index) {
                GEN_EXCP_ILLOP_AND_RETURN();
            }
        }
        /* mov from segment register */
        if (ir1_opnd_is_seg(opnd1)) {
            int seg_reg = ir1_opnd_base_reg_num(opnd1);
            if (seg_reg >= 6) {
                GEN_EXCP_ILLOP_AND_RETURN();
            }
        }
        break;
    /* Need to check CPU features. */
    case X86_INS_FCMOVB:
    case X86_INS_FCMOVE:
    case X86_INS_FCMOVBE:
    case X86_INS_FCMOVU:
    case X86_INS_FCMOVNB:
    case X86_INS_FCMOVNE:
    case X86_INS_FCMOVNBE:
    case X86_INS_FCMOVNU:
    case X86_INS_FCOMI:
    case X86_INS_FUCOMI:
    case X86_INS_FCOMIP:
    case X86_INS_FUCOMIP:
    case X86_INS_FICOM:
    case X86_INS_FICOMP:
        if (!(td->sys.cpuid_features & CPUID_CMOV)) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        break;
    case X86_INS_LDMXCSR:
    case X86_INS_STMXCSR:
        if ((td->sys.flags & HF_EM_MASK) ||
            !(td->sys.flags & HF_OSFXSR_MASK)) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        break;
    case X86_INS_SFENCE:
    case X86_INS_LFENCE:
        if (!(td->sys.cpuid_features & CPUID_SSE) ||
                latxs_ir1_has_prefix_lock(pir1)) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        break;
    case X86_INS_MFENCE:
        if (!(td->sys.cpuid_features & CPUID_SSE2) ||
                latxs_ir1_has_prefix_lock(pir1)) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        break;
    case X86_INS_XSAVEOPT:
        if (!(td->sys.cpuid_xsave_features & CPUID_XSAVE_XSAVEOPT)) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        /* fall through */
    case X86_INS_XSAVE:
    case X86_INS_XRSTOR:
    case X86_INS_XGETBV:
    case X86_INS_XSETBV:
        if (!(td->sys.cpuid_ext_features & CPUID_EXT_XSAVE) ||
            latxs_ir1_has_prefix_lock(pir1)      ||
            latxs_ir1_has_prefix_opsize(pir1)    ||
            latxs_ir1_has_prefix_repe(pir1)      ||
            latxs_ir1_has_prefix_repne(pir1)) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        break;
    case X86_INS_RDTSCP:
        if (!(td->sys.cpuid_ext2_features & CPUID_EXT2_RDTSCP)) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        break;
    case X86_INS_CLAC:
    case X86_INS_STAC:
        if (!(td->sys.cpuid_7_0_ebx_features & CPUID_7_0_EBX_SMAP)
            || td->sys.cpl != 0) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        break;
    case X86_INS_RSM:
        if (!(td->sys.flags & HF_SMM_MASK)) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        break;
    /* //////////AMD SVM(Secure Virtual Machine) Extension///////////// */
    /* ==The following svm inst not need privilege. SKINIT, VMMCALL== */
    case X86_INS_SKINIT:
        if (!(td->sys.cpuid_ext3_features & CPUID_EXT3_SKINIT)) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        /* fall through */
    case X86_INS_VMMCALL:
        if (!(td->sys.flags & HF_SVME_MASK) || !td->sys.pe) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        break;
    /* ==The above svm inst not need privilege. SKINIT, VMMCALL== */
    case X86_INS_STGI:
        if (!(td->sys.cpuid_ext3_features & CPUID_EXT3_SKINIT)) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        /* fall through */
    case X86_INS_CLGI:
    case X86_INS_VMLOAD:
    case X86_INS_VMSAVE:
    case X86_INS_VMRUN:
    case X86_INS_INVLPGA:
        if (!(td->sys.flags & HF_SVME_MASK) || !td->sys.pe) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        /* fall through */
    case X86_INS_INVLPG:
        EC_CPLEQ_GP_AND_RETURN(0);
        break;
    /* ///////End of AMD SVM(Secure Virtual Machine) Extension////////// */
#ifdef TARGET_X86_64
    case X86_INS_FXSAVE64:
    case X86_INS_FXRSTOR64:
#endif
    case X86_INS_FXSAVE:
    case X86_INS_FXRSTOR:
        if (!(td->sys.cpuid_features & CPUID_FXSR)) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        if ((td->sys.flags & HF_EM_MASK) ||
            (td->sys.flags & HF_TS_MASK)) {
            GEN_EXCP_PREX_AND_RETURN();
        }
        break;
    case X86_INS_WAIT:
        if ((td->sys.flags & (HF_MP_MASK | HF_TS_MASK)) ==
            (HF_MP_MASK | HF_TS_MASK)) {
            GEN_EXCP_PREX_AND_RETURN();
        }
        break;
    case X86_INS_CMPXCHG8B:
        if (!(td->sys.cpuid_features & CPUID_CX8)) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        break;
    /* ///////AMD 3DNOW! Extension///////////////// */
    case X86_INS_PI2FW:
    case X86_INS_PI2FD:
    case X86_INS_PF2IW:
    case X86_INS_PF2ID:
    case X86_INS_PFNACC:
    case X86_INS_PFPNACC:
    case X86_INS_PFCMPEQ:
    case X86_INS_PFCMPGE:
    case X86_INS_PFCMPGT:
    case X86_INS_PFMIN:
    case X86_INS_PFRCP:
    case X86_INS_PFRSQRT:
    case X86_INS_PFSUB:
    case X86_INS_PFADD:
    case X86_INS_PFMAX:
    case X86_INS_PFRCPIT1:
    case X86_INS_PFRSQIT1:
    case X86_INS_PFSUBR:
    case X86_INS_PFACC:
    case X86_INS_PFMUL:
    case X86_INS_PFRCPIT2:
    case X86_INS_PMULHRW:
    case X86_INS_PSWAPD:
    case X86_INS_PAVGUSB:
        if (!(td->sys.cpuid_ext2_features & CPUID_EXT2_3DNOW)) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        break;
    /* ///////End of AMD 3DNOW! Extension////////// */
    /* SSE instructions with CPUID check */
    case X86_INS_FEMMS:
        if (!(td->sys.cpuid_ext2_features & CPUID_EXT2_3DNOW)) {
            GEN_EXCP_PREX_AND_RETURN();
        }
        break;
    case X86_INS_MONITOR:
    case X86_INS_MWAIT:
        EC_CPLEQ_GP_AND_RETURN(0);
        if (!(td->sys.cpuid_ext_features & CPUID_EXT_MONITOR)) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        break;
    case X86_INS_PALIGNR:
    case X86_INS_PSHUFB:
    case X86_INS_PHADDW:
    case X86_INS_PHADDD:
    case X86_INS_PHADDSW:
    case X86_INS_PMADDUBSW:
    case X86_INS_PHSUBW:
    case X86_INS_PHSUBD:
    case X86_INS_PHSUBSW:
    case X86_INS_PSIGNB:
    case X86_INS_PSIGNW:
    case X86_INS_PSIGND:
    case X86_INS_PMULHRSW:
    case X86_INS_PABSB:
    case X86_INS_PABSW:
    case X86_INS_PABSD:
        if (!(td->sys.cpuid_ext_features & CPUID_EXT_SSSE3)) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        break;
    case X86_INS_ROUNDPS:
    case X86_INS_ROUNDPD:
    case X86_INS_ROUNDSS:
    case X86_INS_ROUNDSD:
    case X86_INS_BLENDPS:
    case X86_INS_BLENDPD:
    case X86_INS_PBLENDW:
    case X86_INS_DPPS:
    case X86_INS_DPPD:
    case X86_INS_MPSADBW:
    case X86_INS_PBLENDVB:
    case X86_INS_BLENDVPS:
    case X86_INS_BLENDVPD:
    case X86_INS_PTEST:
    case X86_INS_PMOVSXBW:
    case X86_INS_PMOVSXBD:
    case X86_INS_PMOVSXBQ:
    case X86_INS_PMOVSXWD:
    case X86_INS_PMOVSXWQ:
    case X86_INS_PMOVSXDQ:
    case X86_INS_PMULDQ:
    case X86_INS_PCMPEQQ:
    case X86_INS_PACKUSDW:
    case X86_INS_PMOVZXBW:
    case X86_INS_PMOVZXBD:
    case X86_INS_PMOVZXBQ:
    case X86_INS_PMOVZXWD:
    case X86_INS_PMOVZXWQ:
    case X86_INS_PMOVZXDQ:
    case X86_INS_PMINSB:
    case X86_INS_PMINSD:
    case X86_INS_PMINUW:
    case X86_INS_PMINUD:
    case X86_INS_PMAXSB:
    case X86_INS_PMAXSD:
    case X86_INS_PMAXUW:
    case X86_INS_PMAXUD:
    case X86_INS_PMULLD:
    case X86_INS_PHMINPOSUW:
        if (!(td->sys.cpuid_ext_features & CPUID_EXT_SSE41)) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        break;
    case X86_INS_PCMPESTRM:
    case X86_INS_PCMPESTRI:
    case X86_INS_PCMPISTRM:
    case X86_INS_PCMPISTRI:
    case X86_INS_CRC32:
    case X86_INS_PCMPGTQ:
        if (!(td->sys.cpuid_ext_features & CPUID_EXT_SSE42)) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        break;
    case X86_INS_AESKEYGENASSIST:
    case X86_INS_AESIMC:
    case X86_INS_AESENC:
    case X86_INS_AESENCLAST:
    case X86_INS_AESDEC:
    case X86_INS_AESDECLAST:
        if (!(td->sys.cpuid_ext_features & CPUID_EXT_AES)) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        break;
    case X86_INS_PCLMULQDQ:
        if (!(td->sys.cpuid_ext_features & CPUID_EXT_PCLMULQDQ)) {
            GEN_EXCP_ILLOP_AND_RETURN();
        }
        break;
    /* not implemented instruction's exception check */
    default:
        lsassertm(0, "Exception check not implement for %s.\n",
                ir1_name(ir1_opcode(pir1)));
        break;
    }

    return 0;
}

void latxs_tr_gen_excp_illegal_op_addr(ADDRX addr, int end)
{
    latxs_tr_gen_raise_exception_addr(addr, EXCP06_ILLOP, end);
}
void latxs_tr_gen_excp_illegal_op(IR1_INST *ir1, int end)
{
    /* #define EXCP06_ILLOP 6 */
    latxs_tr_gen_raise_exception(ir1, EXCP06_ILLOP, end);
}

void latxs_tr_gen_excp_gp(IR1_INST *ir1, int end)
{
    /* #define EXCP0D_GPF 13 */
    latxs_tr_gen_raise_exception(ir1, EXCP0D_GPF, end);
}

void latxs_tr_gen_excp_prex(IR1_INST *ir1, int end)
{
    /* #define EXCP07_PREX 7 */
    latxs_tr_gen_raise_exception(ir1, EXCP07_PREX, end);
}

void latxs_tr_gen_excp_divz(IR1_INST *ir1, int end)
{
    /* #define EXCP00_DIVZ 0 */
    latxs_tr_gen_raise_exception(ir1, EXCP00_DIVZ, end);
}

static uint8_t *encode_sleb128(uint8_t *p, ulongx val)
{
    int more, byte;

    do {
        byte = val & 0x7f;
        val >>= 7;
        more = !((val == 0 && (byte & 0x40) == 0)
                 || (val == -1 && (byte & 0x40) != 0));
        if (more) {
            byte |= 0x80;
        }
        *p++ = byte;
    } while (more);

    return p;
}

/*
 * Encode Search for Precise Exception
 *
 * Serival numbers are saved at the end of the native code
 * of one TB to tell some information about the translated code.
 *
 * Each x86 instruction is associated with 3 numbers:
 *  1> distance in byte between this inst's address with:
 *     > with TB's PC if this inst is the first inst of TB.
 *       Clearly the result should be zero.
 *     > with prev inst's address
 *  2> always zero. This is designed to be the condition code,
 *     but currently it is not used in TCG.
 *  3> the number of mips instruction generated from translating
 *     this x86 instruction.
 *
 * Each number is encoded in sleb128 format. The abrove function
 * "encode_sleb128" is exactly the same from that one in TCG.
 *
 * This function dependes on the counting result during assembling.
 * So this function should be called after assembling.
 *
 * When handling exception, the EPC points to one of the
 * native code in code cache. According to the encoded search
 * information, we could know which x86 instruction causes the
 * exception. After handleing exception, the CPU should rerun
 * that x86 instruction.
 */

int latxs_tb_encode_search(TranslationBlock *tb, uint8_t *block)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    uint8_t *p = block;
    int i, j, n;

    if (option_dump) {
        fprintf(stderr, "encode search result:\n");
    }

    for (i = 0, n = tb->icount; i < n; ++i) {
        ulongx prev_data;
        ulongx prev_addr;
        ulongx this_data;
        ulongx this_addr;

        for (j = 0; j < TARGET_INSN_START_WORDS; ++j) {

            if (i == 0) {
                prev_data = (j == 0 ? tb->pc : 0);
            } else {
                prev_addr = ir1_addr(td->ir1_inst_array + i - 1)
                            + td->sys.cs_base;
                prev_data = (j == 0 ? prev_addr : 0);
            }

            this_addr = ir1_addr(td->ir1_inst_array + i)
                        + td->sys.cs_base;
            this_data = (j == 0 ? this_addr : 0);

            p = encode_sleb128(p, this_data - prev_data);
            if (option_dump) {
                fprintf(stderr, "[%3d][%d] = 0x%x\n",
                        i, j, (int)(this_data - prev_data));
            }
        }

        prev_data = (i == 0 ? 0 : td->x86_ins_lisa_nr[i - 1] * 4);
        this_data = td->x86_ins_lisa_nr[i] * 4;
        p = encode_sleb128(p, this_data - prev_data);
        if (option_dump) {
            fprintf(stderr, "[%3d]OFF = 0x%x\n", i, (int)this_data);
        }
    }

    return p - block;
}
