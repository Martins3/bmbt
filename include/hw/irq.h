#ifndef IRQ_H_TZGVAY9K
#define IRQ_H_TZGVAY9K

typedef void (*qemu_irq_handler)(void *opaque, int n, int level);

/*
 * Pointer types
 * Such typedefs should be limited to cases where the typedef's users
 * are oblivious of its "pointer-ness".
 * Please keep this list in case-insensitive alphabetical order.
 */
typedef struct IRQState *qemu_irq;

void qemu_set_irq(qemu_irq irq, int level);

static inline void qemu_irq_raise(qemu_irq irq) { qemu_set_irq(irq, 1); }

static inline void qemu_irq_lower(qemu_irq irq) { qemu_set_irq(irq, 0); }

#endif /* end of include guard: IRQ_H_TZGVAY9K */
