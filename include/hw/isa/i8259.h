#ifndef HW_I8259_H
#define HW_I8259_H

// [interface 19]
#include "../irq.h"
#include "../isa/i8259_internal.h"

extern PICCommonState *isa_pic;
qemu_irq *i8259_init(qemu_irq parent_irq);
int pic_read_irq(PICCommonState *s);
int pic_get_output(PICCommonState *s);

#endif
