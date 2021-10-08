#ifndef I8259_INTERNAL_H_XQYSJYON
#define I8259_INTERNAL_H_XQYSJYON

#include "../../exec/memory.h"
#include "../../types.h"
#include "../irq.h"

typedef struct PICCommonState PICCommonState;
typedef struct PICCommonClass {
  // FIXME can I remove it safely
#ifdef BMBT
  ISADeviceClass parent_class;

  void (*pre_save)(PICCommonState *s);
  void (*post_load)(PICCommonState *s);
#endif
} PICCommonClass;

/**
 * PICClass:
 * @parent_realize: The parent's realizefn.
 */
typedef struct PICClass {
  PICCommonClass parent_class;

  void (*parent_realize)(PICCommonState *s);
} PICClass;

#define PIC_COMMON_CLASS(pc)                                                   \
  ({                                                                           \
    PICClass *tmp = pc;                                                        \
    (PICCommonClass *)pc;                                                      \
  })

struct PICCommonState {
  // ISADevice parent_obj;
  PICClass *pc;

  GPIOList gpio;

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
  MemoryRegion base_io;
  MemoryRegion elcr_io;
};

void pic_reset_common(PICCommonState *s);
// [interface 17]
PICCommonState *i8259_init_chip(const char *name, bool master);

#define PIC_GET_CLASS(i) i->pc

void pic_realize(PICCommonState *s);
void pic_common_realize(PICCommonState *s);
void i8259_class_init(PICClass *k);
PICCommonState *QOM_init_PIC(bool master);

void pic_init_reset(PICCommonState *s);

#endif /* end of include guard: I8259_INTERNAL_H_XQYSJYON */
