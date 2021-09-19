#include "../../include/exec/hwaddr.h"
#include "../../include/hw/irq.h"
#include "../../include/hw/isa/i8259_internal.h"
#include "../../include/qemu/log.h"

void pic_reset_common(PICCommonState *s) {
  s->last_irr = 0;
  s->irr &= s->elcr;
  s->imr = 0;
  s->isr = 0;
  s->priority_add = 0;
  s->irq_base = 0;
  s->read_reg_select = 0;
  s->poll = 0;
  s->special_mask = 0;
  s->init_state = 0;
  s->auto_eoi = 0;
  s->rotate_on_auto_eoi = 0;
  s->special_fully_nested_mode = 0;
  s->init4 = 0;
  s->single_mode = 0;
  /* Note: ELCR is not reset */
}

void i8259_init_chip(const char *name, bool master) {
  // @todo @init memory model related
}
