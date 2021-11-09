#include <hw/isa/isa.h>
void isa_bus_irqs(ISABus *bus, qemu_irq *irqs) { bus->irqs = irqs; }
