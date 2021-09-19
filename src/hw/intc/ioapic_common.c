#include "../../include/hw/i386/ioapic_internal.h"

void ioapic_reset_common(IOAPICCommonState *s) {
  int i;

  s->id = 0;
  s->ioregsel = 0;
  s->irr = 0;
  for (i = 0; i < IOAPIC_NUM_PINS; i++) {
    s->ioredtbl[i] = 1 << IOAPIC_LVT_MASKED_SHIFT;
  }
}
