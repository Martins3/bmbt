#ifndef _LATXS_EXCP_H_
#define _LATXS_EXCP_H_

/* Different type of exception check */

#define CHECK_EXCP_CPLEQ_GP(pir1, num) do { \
    if (td->sys.cpl != num) { \
        latxs_tr_gen_excp_gp(pir1, 1); \
        return true; \
    } \
} while (0)

#define CHECK_EXCP_IOPLEQ_GP(pir1, num) do { \
    if (td->sys.iopl != num) { \
        latxs_tr_gen_excp_gp(pir1, 1); \
        return true; \
    } \
} while (0)

#define CHECK_EXCP_PE_IOPLEQ_GP(pir1, num) do { \
    if (td->sys.pe && td->sys.vm86) { \
        CHECK_EXCP_IOPLEQ_GP(pir1, num); \
    } \
} while (0)

#define CHECK_EXCP_CPLLEIOPL_GP(pir1) do { \
    if (!(td->sys.cpl <= td->sys.iopl)) { \
        latxs_tr_gen_excp_gp(pir1, 1); \
        return true; \
    } \
} while (0)

#define CHECK_EXCP_PE_ILLOP(pir1) do { \
    if (!td->sys.pe || td->sys.vm86) { \
        latxs_tr_gen_excp_illegal_op(pir1, 1); \
        return true; \
    } \
} while (0)

#define CHECK_EXCP_PE_GP(pir1) do { \
    if (!td->sys.pe) { \
        latxs_tr_gen_excp_gp(pir1, 1); \
        return true; \
    } \
} while (0)




/*
 * Exception check for each instruction
 * Should be used after
 * TRANSLATION_DATA *td = lsenv->tr_data
 */

/* Only allowed when CPL = 0 */
#define CHECK_EXCP_HLT(pir1)    CHECK_EXCP_CPLEQ_GP(pir1, 0)
#define CHECK_EXCP_LIDT(pir1)   CHECK_EXCP_CPLEQ_GP(pir1, 0)
#define CHECK_EXCP_LGDT(pir1)   CHECK_EXCP_CPLEQ_GP(pir1, 0)
#define CHECK_EXCP_INVD(pir1)   CHECK_EXCP_CPLEQ_GP(pir1, 0)
#define CHECK_EXCP_WBINVD(pir1) CHECK_EXCP_CPLEQ_GP(pir1, 0)
#define CHECK_EXCP_LMSW(pir1)   CHECK_EXCP_CPLEQ_GP(pir1, 0)
#define CHECK_EXCP_CLTS(pir1)   CHECK_EXCP_CPLEQ_GP(pir1, 0)
#define CHECK_EXCP_WRMSR(pir1)  CHECK_EXCP_CPLEQ_GP(pir1, 0)
#define CHECK_EXCP_RDMSR(pir1)  CHECK_EXCP_CPLEQ_GP(pir1, 0)

/*
 * Only not allowed in Real-Address mode
 * > [OK]  PE && !vm86
 * > [OK]  PE &&  vm86
 * > [NO] !PE (real mode)
 */
#define CHECK_EXCP_SYSENTER(pir1)   CHECK_EXCP_PE_GP(pir1)
#define CHECK_EXCP_SYSEXIT(pir1)    CHECK_EXCP_PE_GP(pir1)

/* Only allowed in PE (not vm86) */
#define CHECK_EXCP_VERR(pir1)   CHECK_EXCP_PE_ILLOP(pir1)
#define CHECK_EXCP_VERW(pir1)   CHECK_EXCP_PE_ILLOP(pir1)
#define CHECK_EXCP_SLDT(pir1)   CHECK_EXCP_PE_ILLOP(pir1)
#define CHECK_EXCP_STR(pir1)    CHECK_EXCP_PE_ILLOP(pir1)
#define CHECK_EXCP_ARPL(pir1)   CHECK_EXCP_PE_ILLOP(pir1)
#define CHECK_EXCP_LARLSL(pir1) CHECK_EXCP_PE_ILLOP(pir1)
#define CHECK_EXCP_VERR(pir1)   CHECK_EXCP_PE_ILLOP(pir1)
#define CHECK_EXCP_VERW(pir1)   CHECK_EXCP_PE_ILLOP(pir1)

/* Only allowed in PE (not vm86) and CPL = 0 */
#define CHECK_EXCP_LLDT(pir1) do { \
    CHECK_EXCP_PE_ILLOP(pir1); \
    CHECK_EXCP_CPLEQ_GP(pir1, 0); \
} while (0)

#define CHECK_EXCP_LTR(pir1) do { \
    CHECK_EXCP_PE_ILLOP(pir1); \
    CHECK_EXCP_CPLEQ_GP(pir1, 0); \
} while (0)

/*
 * Only allowed in :
 * 1. PE not vm86
 * 2. PE and vm86 and IOPL is 3
 */
#define CHECK_EXCP_POPF(pir1)   CHECK_EXCP_PE_IOPLEQ_GP(pir1, 3)
#define CHECK_EXCP_POPFD(pir1)  CHECK_EXCP_PE_IOPLEQ_GP(pir1, 3)
#define CHECK_EXCP_PUSHF(pir1)  CHECK_EXCP_PE_IOPLEQ_GP(pir1, 3)
#define CHECK_EXCP_PUSHFD(pir1) CHECK_EXCP_PE_IOPLEQ_GP(pir1, 3)
#define CHECK_EXCP_INT(pir1)    CHECK_EXCP_PE_IOPLEQ_GP(pir1, 3)
#define CHECK_EXCP_IRET(pir1)   CHECK_EXCP_PE_IOPLEQ_GP(pir1, 3)
#define CHECK_EXCP_IRETD(pir1)  CHECK_EXCP_PE_IOPLEQ_GP(pir1, 3)

/*
 * Only allowed in :
 * 1. PE (not vm86) or Real && CPL <= IOPL
 * 2. vm86 mode and IOPL is 3
 */
#define CHECK_EXCP_CLI(pir1) do { \
    if (!td->sys.vm86) { \
        CHECK_EXCP_CPLLEIOPL_GP(pir1); \
    } else { \
        CHECK_EXCP_IOPLEQ_GP(pir1, 3); \
    } \
} while (0)

#define CHECK_EXCP_STI(pir1) do { \
    if (!td->sys.vm86) { \
        CHECK_EXCP_CPLLEIOPL_GP(pir1); \
    } else { \
        CHECK_EXCP_IOPLEQ_GP(pir1, 3); \
    } \
} while (0)

/*
 * Mov instruction :
 * 1. mov to/from CR/DR : CPL is 0
 * 3. mov to      seg   : ES,   SS,DS,FS,GS
 * 4. mov    from seg   : ES,CS,SS,DS,FS,GS
 */
#define CHECK_EXCP_MOV_CR(pir1) do { \
    if (td->sys.cpl != 0) { \
        latxs_tr_gen_excp_gp(pir1, 1); \
        return ; \
    } \
} while (0)

#define CHECK_EXCP_MOV_DR(pir1) do { \
    if (td->sys.cpl != 0) { \
        latxs_tr_gen_excp_gp(pir1, 1); \
        return ; \
    } \
} while (0)

#define CHECK_EXCP_MOV_TO_SEG(pir1, seg) do { \
    if ((seg) >= 6 || (seg) == cs_index) { \
        latxs_tr_gen_excp_illegal_op(pir1, 1); \
        return; \
    } \
} while (0)

#define CHECK_EXCP_MOV_FROM_SEG(pir1, seg) do { \
    if ((seg) >= 6) { \
        latxs_tr_gen_excp_illegal_op(pir1, 1); \
        return; \
    } \
} while (0)

/* Need to check CPU features. */
#define CHECK_EXCP_RSM(pir1) do { \
    if (!(td->sys.flags & HF_SMM_MASK)) { \
        latxs_tr_gen_excp_illegal_op(pir1, 1); \
        return true; \
    } \
} while (0)

#define CHECK_EXCP_CLAC(pir1) do { \
    if (!(td->sys.cpuid_7_0_ebx_features & CPUID_7_0_EBX_SMAP) \
        || td->sys.cpl != 0) { \
        latxs_tr_gen_excp_illegal_op(pir1, 1); \
        return true; \
    } \
} while (0)

#define CHECK_EXCP_STAC(pir1) do { \
    if (!(td->sys.cpuid_7_0_ebx_features & CPUID_7_0_EBX_SMAP) \
        || td->sys.cpl != 0) { \
        latxs_tr_gen_excp_illegal_op(pir1, 1); \
        return true; \
    } \
} while (0)

#endif
