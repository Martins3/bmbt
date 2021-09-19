#include "../../include/hw/irq.h"

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
