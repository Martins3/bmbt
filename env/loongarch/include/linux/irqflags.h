#ifndef IRQFLAGS_H_TBGQZROC
#define IRQFLAGS_H_TBGQZROC

#include <asm/irqflags.h>
/*
 * Wrap the arch provided IRQ routines to provide appropriate checks.
 */
#define raw_local_irq_disable() arch_local_irq_disable()
#define raw_local_irq_enable() arch_local_irq_enable()

#define local_irq_enable()                                                     \
  do {                                                                         \
    raw_local_irq_enable();                                                    \
  } while (0)
#define local_irq_disable()                                                    \
  do {                                                                         \
    raw_local_irq_disable();                                                   \
  } while (0)

#endif /* end of include guard: IRQFLAGS_H_TBGQZROC */
