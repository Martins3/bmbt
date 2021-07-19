#ifndef IRQ_H_TZGVAY9K
#define IRQ_H_TZGVAY9K

typedef void (*qemu_irq_handler)(void *opaque, int n, int level);

struct IRQState {
  qemu_irq_handler handler;
  void *opaque;
  int n;
};

/*
 * Pointer types
 * Such typedefs should be limited to cases where the typedef's users
 * are oblivious of its "pointer-ness".
 * Please keep this list in case-insensitive alphabetical order.
 */
typedef struct IRQState *qemu_irq;

void qemu_set_irq(qemu_irq irq, int level) {
  if (!irq)
    return;

  irq->handler(irq->opaque, irq->n, level);
}

static inline void qemu_irq_raise(qemu_irq irq) { qemu_set_irq(irq, 1); }

#endif /* end of include guard: IRQ_H_TZGVAY9K */
