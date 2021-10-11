#include "../../../include/hw/irq.h"
#include "../../tcg/glib_stub.h"

struct IRQState {
  // Object parent_obj;

  qemu_irq_handler handler;
  void *opaque;
  int n;
};

void qemu_set_irq(qemu_irq irq, int level) {
  if (!irq)
    return;

  irq->handler(irq->opaque, irq->n, level);
}

qemu_irq *qemu_extend_irqs(qemu_irq *old, int n_old, qemu_irq_handler handler,
                           void *opaque, int n) {
  qemu_irq *s;
  int i;

  if (!old) {
    n_old = 0;
  }
  s = old ? g_renew(qemu_irq, old, n + n_old) : g_new(qemu_irq, n);
  for (i = n_old; i < n + n_old; i++) {
    s[i] = qemu_allocate_irq(handler, opaque, i);
  }
  return s;
}

qemu_irq *qemu_allocate_irqs(qemu_irq_handler handler, void *opaque, int n) {
  return qemu_extend_irqs(NULL, 0, handler, opaque, n);
}
