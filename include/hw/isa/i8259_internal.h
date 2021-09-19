#ifndef I8259_INTERNAL_H_XQYSJYON
#define I8259_INTERNAL_H_XQYSJYON

#include "../../types.h"
#include "../irq.h"
typedef struct {
  // ISADevice parent_obj;

  uint8_t last_irr;     /* edge detection */
  uint8_t irr;          /* interrupt request register */
  uint8_t imr;          /* interrupt mask register */
  uint8_t isr;          /* interrupt service register */
  uint8_t priority_add; /* highest irq priority */
  uint8_t irq_base;
  uint8_t read_reg_select;
  uint8_t poll;
  uint8_t special_mask;
  uint8_t init_state;
  uint8_t auto_eoi;
  uint8_t rotate_on_auto_eoi;
  uint8_t special_fully_nested_mode;
  uint8_t init4;       /* true if 4 byte init */
  uint8_t single_mode; /* true if slave pic is not initialized */
  uint8_t elcr;        /* PIIX edge/trigger selection*/
  uint8_t elcr_mask;
  qemu_irq int_out[1];
  uint32_t master; /* reflects /SP input pin */
  uint32_t iobase;
  uint32_t elcr_addr;
  // @todo fix it in memory model module
  // MemoryRegion base_io;
  // MemoryRegion elcr_io;
} PICCommonState;

void pic_reset_common(PICCommonState *s);
// [interface 17]
void i8259_init_chip(const char *name, bool master);

#endif /* end of include guard: I8259_INTERNAL_H_XQYSJYON */
