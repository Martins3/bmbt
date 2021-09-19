#include "../irq.h"
#include "../isa/i8259_internal.h"

extern PICCommonState *isa_pic;
// qemu_irq *i8259_init(ISABus *bus, qemu_irq parent_irq);
int pic_read_irq(PICCommonState *s);
int pic_get_output(PICCommonState *s);
