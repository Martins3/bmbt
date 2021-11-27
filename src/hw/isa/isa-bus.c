#include <hw/isa/isa.h>
#include <qemu/error-report.h>

static ISABus __isabus;
ISABus *get_isa_bus() { return &__isabus; }
void isa_bus_irqs(ISABus *bus, qemu_irq *irqs) {
  assert(bus == get_isa_bus());
  bus->irqs = irqs;
}

/*
 * isa_get_irq() returns the corresponding qemu_irq entry for the i8259.
 *
 * This function is only for special cases such as the 'ferr', and
 * temporary use for normal devices until they are converted to qdev.
 */
qemu_irq isa_get_irq(ISADevice *dev, int isairq) {
  // assert(!dev || ISA_BUS(qdev_get_parent_bus(DEVICE(dev))) == isabus);
  if (isairq < 0 || isairq > 15) {
    error_report("isa irq %d invalid", isairq);
  }
  return get_isa_bus()->irqs[isairq];
}

void isa_init_irq(ISADevice *dev, qemu_irq *p, int isairq) {
#ifdef BMBT
  assert(dev->nirqs < ARRAY_SIZE(dev->isairq));
  dev->isairq[dev->nirqs] = isairq;
  *p = isa_get_irq(dev, isairq);
  dev->nirqs++;
#endif
  *p = isa_get_irq(dev, isairq);
}
