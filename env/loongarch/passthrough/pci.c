#include <asm/device.h>
#include <exec/memory.h>
#include <hw/pci/pci.h>
#include <linux/pci.h>
#include <qemu/range.h>

static void unimplemented(const char *str) {
  printf("unimplemented pci function : %s\n", str);
  g_assert_not_reached();
}

static void pci_config_write(uint32_t addr, uint32_t val, int l) {
  uint8_t devfn = addr >> 8;
  uint8_t where = addr & 0xfc;
  switch (l) {
  case 1:
    pci_bus_write_config_byte(devfn, where, val);
    break;
  case 2:
    pci_bus_write_config_word(devfn, where, val);
    break;
  case 4:
    pci_bus_write_config_dword(devfn, where, val);
    break;
  default:
    g_assert_not_reached();
  }
}

static uint32_t pci_config_read(uint32_t addr, int l) {
  uint32_t val;
  uint8_t devfn = addr >> 8;
  uint8_t where = addr & 0xfc;
  // read more is better than read less, no need to take care of length
  pci_bus_read_config_dword(devfn, where, &val);
  return val;
}

void pci_pass_through_write(uint32_t addr, uint32_t val, int l) {
  uint32_t config_addr = addr & (PCI_CONFIG_SPACE_SIZE - 1);
  if (ranges_overlap(config_addr, l, PCI_BASE_ADDRESS_0, 24)) {
    assert(l == 4);

    uint32_t orig_val = pci_config_read(addr, l);
    if ((val & 0xff000000) != 0xff000000 && val != 0) {
      if (orig_val & PCI_BASE_ADDRESS_MEM_TYPE_64)
        unimplemented("doesn't support 64bit mmio");

      if (orig_val & PCI_BASE_ADDRESS_SPACE_IO) {
        assert(val >= X86_PCI_IO_START && val <= X86_PCI_IO_END);
        val -= LOONGSON_X86_PCI_IO_OFFSET;
      } else {
        assert(val >= BUILD_PCIMEM_START && val <= BUILD_PCIMEM_END);
        val -= LOONGSON_X86_PCI_MEM_OFFSET;
      }
    }
  }

  if (ranges_overlap(config_addr, l, PCI_ROM_ADDRESS, 4) ||
      ranges_overlap(config_addr, l, PCI_ROM_ADDRESS1, 4)) {
    return;
  }

  pci_config_write(addr, val, l);
}

uint32_t pci_pass_through_read(uint32_t addr, int l) {
  uint32_t val = pci_config_read(addr, l);
  uint32_t config_addr = addr & (PCI_CONFIG_SPACE_SIZE - 1);

  if (ranges_overlap(config_addr, l, PCI_BASE_ADDRESS_0, 24)) {
    assert(l == 4);

    if (val & PCI_BASE_ADDRESS_MEM_TYPE_64)
      unimplemented("doesn't support 64bit mmio");

    if ((val & 0xff000000) != 0xff000000 && val != 0) {
      if (val & PCI_BASE_ADDRESS_SPACE_IO) {
        val += LOONGSON_X86_PCI_IO_OFFSET;
        assert(val >= X86_PCI_IO_START && val <= X86_PCI_IO_END);
      } else {
        val += LOONGSON_X86_PCI_MEM_OFFSET;
        assert(val >= BUILD_PCIMEM_START && val <= BUILD_PCIMEM_END);
      }
    }
  }

  if (ranges_overlap(config_addr, l, PCI_ROM_ADDRESS, 4) ||
      ranges_overlap(config_addr, l, PCI_ROM_ADDRESS1, 4)) {
    return 0;
  }
  return val;
}

static uint64_t pass_mmio_pass_read(void *opaque, hwaddr addr, unsigned size) {
  addr += BUILD_PCIMEM_START;
  return readq((void *)TO_UNCAC((addr - LOONGSON_X86_PCI_MEM_OFFSET)));
}

static void pci_mmio_pass_write(void *opaque, hwaddr addr, uint64_t val,
                                unsigned size) {
  addr += BUILD_PCIMEM_START;

  printf("[huxueshi:%s:%d] %lx %lx\n", __FUNCTION__, __LINE__, addr,
         addr - LOONGSON_X86_PCI_MEM_OFFSET);
  switch (size) {
  case 1:
    writeb(val, (void *)TO_UNCAC((addr - LOONGSON_X86_PCI_MEM_OFFSET)));
    break;
  case 2:
    writew(val, (void *)TO_UNCAC((addr - LOONGSON_X86_PCI_MEM_OFFSET)));
    break;
  case 4:
    writel(val, (void *)TO_UNCAC((addr - LOONGSON_X86_PCI_MEM_OFFSET)));
    break;
  case 8:
    writeq(val, (void *)TO_UNCAC((addr - LOONGSON_X86_PCI_MEM_OFFSET)));
    break;
  default:
    g_assert_not_reached();
  }
}

static const MemoryRegionOps pci_mmio_pass_ops = {
    .read = pass_mmio_pass_read,
    .write = pci_mmio_pass_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
};

static MemoryRegion pci_mmio_mr;
void pci_pass_through_init() {
  memory_region_init_io(&pci_mmio_mr, &pci_mmio_pass_ops, NULL, "pci-mmio",
                        BUILD_PCIMEM_END - BUILD_PCIMEM_START);
  mmio_add_memory_region(BUILD_PCIMEM_START, &pci_mmio_mr);
}
