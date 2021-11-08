#ifndef IRQ_H_TZGVAY9K
#define IRQ_H_TZGVAY9K
#include "../../src/tcg/glib_stub.h"
#include <assert.h>

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

/* Returns an array of N IRQs. Each IRQ is assigned the argument handler and
 * opaque data.
 */
qemu_irq *qemu_allocate_irqs(qemu_irq_handler handler, void *opaque, int n);

/*
 * Allocates a single IRQ. The irq is assigned with a handler, an opaque
 * data and the interrupt number.
 */
qemu_irq qemu_allocate_irq(qemu_irq_handler handler, void *opaque, int n);

/* Extends an Array of IRQs. Old IRQs have their handlers and opaque data
 * preserved. New IRQs are assigned the argument handler and opaque data.
 */
qemu_irq *qemu_extend_irqs(qemu_irq *old, int n_old, qemu_irq_handler handler,
                           void *opaque, int n);

typedef struct {
  qemu_irq *in;
  qemu_irq *out[1];
} GPIOList;

static inline void qdev_init_gpio_in(GPIOList *dev_gpio,
                                     qemu_irq_handler handler, void *opaque,
                                     int n) {
  dev_gpio->in = g_new(qemu_irq, n);
  for (int i = 0; i < n; i++) {
    dev_gpio->in[i] = qemu_allocate_irq(handler, opaque, i);
  }
}

static inline void qdev_init_gpio_out(GPIOList *dev_gpio, qemu_irq *pins,
                                      int n) {
  assert(n == 1);
  dev_gpio->out[0] = &pins[0];
}

static inline void qdev_connect_gpio_out(GPIOList *dev_gpio, int n,
                                         qemu_irq pin) {
  assert(n == 0);
  *(dev_gpio->out[n]) = pin;
}

static inline qemu_irq qdev_get_gpio_in(GPIOList *dev_gpio, int n) {
  return dev_gpio->in[n];
}

#endif /* end of include guard: IRQ_H_TZGVAY9K */
