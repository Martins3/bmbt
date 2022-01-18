#ifndef IRQFLAGS_H_QAJUFPSI
#define IRQFLAGS_H_QAJUFPSI
#include <asm/loongarchregs.h>

static inline void arch_local_irq_disable(void) {
  csr_xchgl(0, CSR_CRMD_IE, LOONGARCH_CSR_CRMD);
}

static inline void arch_local_irq_enable(void) {
  csr_xchgl(CSR_CRMD_IE, CSR_CRMD_IE, LOONGARCH_CSR_CRMD);
}

#endif /* end of include guard: IRQFLAGS_H_QAJUFPSI */
