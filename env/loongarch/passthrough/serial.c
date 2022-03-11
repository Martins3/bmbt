#include <asm/device.h>
#include <asm/io.h>
#include <asm/mach-la64/irq.h>
#include <exec/memory.h>
#include <hw/irq.h>
#include <hw/isa/isa.h>
#include <stdio.h>
#include <asm/debugcon.h>

static MemoryRegion mr;
static const hwaddr X86_ISA_SERIAL_IO_BASE = 0x3f8;
static const int ISA_SERIAL_IRQ = 4;
#define IRQ_IOAPIC_SERIAL 2

static uint64_t serial_ioport_pass_read(void *opaque, hwaddr addr,
                                        unsigned size) {
  assert(size == 1);
  /* debugcon_puts("pass read\n"); */
  return readb(LS_ISA_SERIAL_IO_BASE + addr);
}

static void serial_ioport_pass_write(void *opaque, hwaddr addr, uint64_t val,
                                     unsigned size) {
  assert(size == 1);
  /* debugcon_puts("pass write\n"); */
  writeb(val, LS_ISA_SERIAL_IO_BASE + addr);
}

const MemoryRegionOps serial_pass_ops = {
    .read = serial_ioport_pass_read,
    .write = serial_ioport_pass_write,
    .impl =
        {
            .min_access_size = 1,
            .max_access_size = 1,
        },
    .endianness = DEVICE_LITTLE_ENDIAN,
};

static qemu_irq serial_irq;
static void serial_mr_setup() {
  memory_region_init_io(&mr, &serial_pass_ops, NULL, "serial", 8);
  io_add_memory_region(X86_ISA_SERIAL_IO_BASE, &mr);
  isa_init_irq(NULL, &serial_irq, ISA_SERIAL_IRQ);
}

void serial_handler(int hwirq) {
  /* debugcon_puts("i "); */
  qemu_irq_lower(serial_irq);
  qemu_irq_raise(serial_irq);
}

void setup_serial_pass_through() {
  serial_mr_setup();
  set_pch_pci_action_handler(IRQ_IOAPIC_SERIAL, serial_handler);
  qemu_irq_lower(serial_irq);
}
