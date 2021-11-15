#ifndef ISA_H_LNNOWHFU
#define ISA_H_LNNOWHFU

#include "../irq.h"

#define ISA_NUM_IRQS 16
typedef struct {
#ifdef BMBT
  /*< private >*/
  BusState parent_obj;
  /*< public >*/

  MemoryRegion *address_space;
  MemoryRegion *address_space_io;
  qemu_irq *irqs;
  IsaDma *dma[2];
#endif
  qemu_irq *irqs;
} ISABus;

typedef struct {
} ISADevice;

void isa_bus_irqs(ISABus *bus, qemu_irq *irqs);
ISABus *get_isa_bus();

void QOM_init_debugcon();

#endif /* end of include guard: ISA_H_LNNOWHFU */
