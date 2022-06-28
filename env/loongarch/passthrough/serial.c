#include <asm/debug.h>
#include <asm/device.h>
#include <asm/io.h>
#include <asm/mach-la64/irq.h>
#include <exec/memory.h>
#include <hw/irq.h>
#include <hw/isa/isa.h>
#include <stdio.h>

static MemoryRegion mr;
static const hwaddr X86_ISA_SERIAL_IO_BASE = 0x3f8;
static const int ISA_SERIAL_IRQ = 4;

/*
 * if Divisor Latch Access Bit's DLAB bit is set, DLL and DLH can be used for
 * reading divisor. If the guest is using serial with pass through, in the
 * meanwhile, the host is using the serial with printf, the serial may set to
 * abnormal state unexpectly.
 */
static bool is_dlab_set = false;
bool can_write() { return !is_dlab_set; }

static uint64_t serial_ioport_pass_read(void *opaque, hwaddr addr,
                                        unsigned size) {
  assert(size == 1);
  /* debugcon_puts("pass read\n"); */
  return readb(LS_ISA_SERIAL_IO_BASE + addr);
}

// [interface 68]
static void serial_ioport_pass_write(void *opaque, hwaddr addr, uint64_t val,
                                     unsigned size) {
  assert(size == 1);
  /* debugcon_puts("pass write\n"); */
  writeb(val, LS_ISA_SERIAL_IO_BASE + addr);

  if (addr == LCR) {
    is_dlab_set = readb(LS_ISA_SERIAL_IO_BASE + LCR) & DLAB;
  }

  if (is_dlab_set) {
    switch (addr) {
    case DLL:
      writeb(0x36, LS_ISA_SERIAL_IO_BASE + addr);
      break;
    case DLH:
      writeb(0, LS_ISA_SERIAL_IO_BASE + addr);
      break;
    }
  }
}

static const MemoryRegionOps serial_pass_ops = {
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
  qemu_irq_lower(serial_irq);
  qemu_irq_raise(serial_irq);
}

void setup_serial_pass_through() {
  serial_mr_setup();
  qemu_irq_lower(serial_irq);
}
