#ifndef IRQFLAGS_H_TBGQZROC
#define IRQFLAGS_H_TBGQZROC

#include <asm/irqflags.h>
#include <linux/typecheck.h>
/*
 * Wrap the arch provided IRQ routines to provide appropriate checks.
 */
#define raw_local_irq_disable() arch_local_irq_disable()
#define raw_local_irq_enable() arch_local_irq_enable()
#define raw_local_irq_save(flags)                                              \
  do {                                                                         \
    typecheck(unsigned long, flags);                                           \
    flags = arch_local_irq_save();                                             \
  } while (0)
#define raw_local_irq_restore(flags)                                           \
  do {                                                                         \
    typecheck(unsigned long, flags);                                           \
    arch_local_irq_restore(flags);                                             \
  } while (0)

#define local_irq_enable()                                                     \
  do {                                                                         \
    raw_local_irq_enable();                                                    \
  } while (0)

#define local_irq_disable()                                                    \
  do {                                                                         \
    raw_local_irq_disable();                                                   \
  } while (0)

#define local_irq_save(flags)                                                  \
  do {                                                                         \
    raw_local_irq_save(flags);                                                 \
  } while (0)
#define local_irq_restore(flags)                                               \
  do {                                                                         \
    raw_local_irq_restore(flags);                                              \
  } while (0)

#endif /* end of include guard: IRQFLAGS_H_TBGQZROC */
