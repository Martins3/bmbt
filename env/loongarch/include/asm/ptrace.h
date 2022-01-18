#ifndef PTRACE_H_RNPV8KSH
#define PTRACE_H_RNPV8KSH

/*
 * This struct defines the way the registers are stored on the stack during a
 * system call/exception.
 *
 * If you add a register here, also add it to regoffset_table[] in
 * arch/loongarch/kernel/ptrace.c.
 */
struct pt_regs {
  /* Saved main processor registers. */
  unsigned long regs[32];

  /* Saved special registers. */
  unsigned long csr_era;
  unsigned long csr_badv;
  unsigned long csr_crmd;
  unsigned long csr_prmd;
  unsigned long csr_euen;
  unsigned long csr_ecfg;
  unsigned long csr_estat;
  unsigned long orig_a0;
  unsigned long __last[0];
} __attribute__((aligned(8)));

#endif /* end of include guard: PTRACE_H_RNPV8KSH */
