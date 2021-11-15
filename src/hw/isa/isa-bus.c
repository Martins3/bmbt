#include <hw/isa/isa.h>

static ISABus __isabus;
ISABus *get_isa_bus() { return &__isabus; }
void isa_bus_irqs(ISABus *bus, qemu_irq *irqs) { bus->irqs = irqs; }
