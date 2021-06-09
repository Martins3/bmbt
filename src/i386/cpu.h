#ifndef CPU_H_CJEDABLV
#define CPU_H_CJEDABLV
// FIXME
// included by "../../include/exec/exec-all.h"
// Don't include anything here
// what does you include the data?

#include "../../include/exec/cpu-defs.h"
#include "../../include/fpu/softfloat-types.h"
#include "../../include/types.h"

#define CPU_NB_REGS64 16
#define CPU_NB_REGS32 8

#ifdef TARGET_X86_64
#define CPU_NB_REGS CPU_NB_REGS64
#else
#define CPU_NB_REGS CPU_NB_REGS32
#endif

// FIXME
struct TranslationBlock;
struct CPUState;

typedef union {
  floatx80 d __attribute__((aligned(16)));
  // FIXME comment it temporarily
  // MMXReg mmx;
} FPReg;

// FIXME target_ulong isn't highlighted by ccls
typedef struct CPUX86State {
  CPUState *cpu;

  target_ulong regs[CPU_NB_REGS];
  target_ulong eip;
  target_ulong eflags; /* eflags register. During CPU emulation, CC
                      flags and DF are set to zero because they are
                      stored elsewhere */

  /* emulator internal eflags handling */
  target_ulong cc_dst;
  target_ulong cc_src;
  target_ulong cc_src2;
  uint32_t cc_op;
  int32_t df;       /* D flag : 1 if D = 0, -1 if D = 1 */
  uint32_t hflags;  /* TB flags, see HF_xxx constants. These flags
                       are known at translation time. */
  uint32_t hflags2; /* various other flags, see HF2_xxx constants. */

  /* FPU state */
  unsigned int fpstt; /* top of stack index */
  uint16_t fpus;
  uint16_t fpuc;
  uint8_t fptags[8]; /* 0 = valid, 1 = empty */
  FPReg fpregs[8];

  /* exception/interrupt handling */
  int error_code;
  int exception_is_int;
  target_ulong exception_next_eip;
  target_ulong dr[8]; /* debug registers; note dr4 and dr5 are unused */

  void *cpt_ptr; /* Point to Code Page Table */

} CPUX86State;

/* Instead of computing the condition codes after each x86 instruction,
 * QEMU just stores one operand (called CC_SRC), the result
 * (called CC_DST) and the type of operation (called CC_OP). When the
 * condition codes are needed, the condition codes can be calculated
 * using this information. Condition codes are not generated if they
 * are only needed for conditional branches.
 */
typedef enum {
  CC_OP_DYNAMIC, /* must use dynamic code to get cc_op */
  CC_OP_EFLAGS,  /* all cc are explicitly computed, CC_SRC = flags */

  CC_OP_MULB, /* modify all flags, C, O = (CC_SRC != 0) */
  CC_OP_MULW,
  CC_OP_MULL,
  CC_OP_MULQ,

  CC_OP_ADDB, /* modify all flags, CC_DST = res, CC_SRC = src1 */
  CC_OP_ADDW,
  CC_OP_ADDL,
  CC_OP_ADDQ,

  CC_OP_ADCB, /* modify all flags, CC_DST = res, CC_SRC = src1 */
  CC_OP_ADCW,
  CC_OP_ADCL,
  CC_OP_ADCQ,

  CC_OP_SUBB, /* modify all flags, CC_DST = res, CC_SRC = src1 */
  CC_OP_SUBW,
  CC_OP_SUBL,
  CC_OP_SUBQ,

  CC_OP_SBBB, /* modify all flags, CC_DST = res, CC_SRC = src1 */
  CC_OP_SBBW,
  CC_OP_SBBL,
  CC_OP_SBBQ,

  CC_OP_LOGICB, /* modify all flags, CC_DST = res */
  CC_OP_LOGICW,
  CC_OP_LOGICL,
  CC_OP_LOGICQ,

  CC_OP_INCB, /* modify all flags except, CC_DST = res, CC_SRC = C */
  CC_OP_INCW,
  CC_OP_INCL,
  CC_OP_INCQ,

  CC_OP_DECB, /* modify all flags except, CC_DST = res, CC_SRC = C  */
  CC_OP_DECW,
  CC_OP_DECL,
  CC_OP_DECQ,

  CC_OP_SHLB, /* modify all flags, CC_DST = res, CC_SRC.msb = C */
  CC_OP_SHLW,
  CC_OP_SHLL,
  CC_OP_SHLQ,

  CC_OP_SARB, /* modify all flags, CC_DST = res, CC_SRC.lsb = C */
  CC_OP_SARW,
  CC_OP_SARL,
  CC_OP_SARQ,

  CC_OP_BMILGB, /* Z,S via CC_DST, C = SRC==0; O=0; P,A undefined */
  CC_OP_BMILGW,
  CC_OP_BMILGL,
  CC_OP_BMILGQ,

  CC_OP_ADCX,  /* CC_DST = C, CC_SRC = rest.  */
  CC_OP_ADOX,  /* CC_DST = O, CC_SRC = rest.  */
  CC_OP_ADCOX, /* CC_DST = C, CC_SRC2 = O, CC_SRC = rest.  */

  CC_OP_CLR,    /* Z set, all other flags clear.  */
  CC_OP_POPCNT, /* Z via CC_SRC, all other flags clear.  */

  CC_OP_NB,
} CCOp;

typedef CPUX86State CPUArchState;

#endif /* end of include guard: CPU_H_CJEDABLV */
