#include "internal.h"
#include <asm/device.h>
#include <linux/pci.h>

static void pci_config_write(uint32_t addr, uint32_t val, int l) {
  u16 bdf = get_bdf(addr);
  int where = get_where(addr);
  switch (l) {
  case 1:
    pci_bus_write_config_byte(bdf, where, val);
    break;
  case 2:
    pci_bus_write_config_word(bdf, where, val);
    break;
  case 4:
    pci_bus_write_config_dword(bdf, where, val);
    break;
  default:
    g_assert_not_reached();
  }
}

// [BMBT_OPTIMIZE 7]
static uint32_t pci_config_read(uint32_t addr, int l) {
  uint32_t val32;
  uint16_t val16;
  uint8_t val8;
  u16 bdf = get_bdf(addr);
  int where = get_where(addr);
  switch (l) {
  case 1:
    pci_bus_read_config_byte(bdf, where, &val8);
    return val8;
  case 2:
    pci_bus_read_config_word(bdf, where, &val16);
    return val16;
  case 4:
    pci_bus_read_config_dword(bdf, where, &val32);
    return val32;
  default:
    g_assert_not_reached();
  }
}

void pci_config_pass_write(uint32_t addr, uint32_t val, int l) {
  uint32_t config_addr = get_config_addr(addr);
  bool msix_table_updated = false;
  /**
   * 1. add_PCIe_devices will create one BMBT_PCIExpressDevice.
   * 2. BIOS or OS probe PCI devices by writting PCI config space and read it
   * back.
   *
   * There's no need to create BMBT_PCIExpressDevice for nonexistent devices.
   */
  if (!ranges_overlap(config_addr, l, 0, PCI_BASE_ADDRESS_0)) {
    int idx = add_pcie_devices(get_bdf(addr));

    val = msi_translate(addr, val, true);

    val = pcie_bridge_window_translate(idx, addr, l, val, true);
    if (ranges_overlap(config_addr, l, PCI_BASE_ADDRESS_0, 24) ||
        ranges_overlap(config_addr, l, PCI_ROM_ADDRESS, 4) ||
        ranges_overlap(config_addr, l, PCI_ROM_ADDRESS1, 4)) {
      val = pcie_bar_translate(addr, l, val, true, &msix_table_updated);
    }
  }

  pci_config_write(addr, val, l);

  if (msix_table_updated)
    msix_map_region(get_bdf(addr));
}

uint32_t pci_config_pass_read(uint32_t addr, int l) {
  uint32_t config_addr = get_config_addr(addr);

  if (!ranges_overlap(config_addr, l, 0, PCI_BASE_ADDRESS_0)) {
    bool unused;
    int idx = add_pcie_devices(get_bdf(addr));
    u32 val = pci_config_read(addr, l);

    val = pcie_bridge_window_translate(idx, addr, l, val, false);
    if (ranges_overlap(config_addr, l, PCI_BASE_ADDRESS_0, 24) ||
        ranges_overlap(config_addr, l, PCI_ROM_ADDRESS, 4) ||
        ranges_overlap(config_addr, l, PCI_ROM_ADDRESS1, 4)) {

      return pcie_bar_translate(addr, l, val, false, &unused);
    }
  }

  return pci_config_read(addr, l);
}

static uint64_t pass_mmio_pass_read(void *opaque, hwaddr addr, unsigned size) {
  addr += BUILD_PCIMEM_START;
  addr -= LOONGSON_X86_PCI_MEM_OFFSET;

  u32 val;
  if (msix_pass_read(addr, size, &val)) {
    return val;
  }

  switch (size) {
  case 1:
    return readb((void *)TO_UNCAC(addr));
  case 2:
    return readw((void *)TO_UNCAC(addr));
  case 4:
    return readl((void *)TO_UNCAC(addr));
  case 8:
    return readq((void *)TO_UNCAC(addr));
  default:
    g_assert_not_reached();
  }
}

static void pci_mmio_pass_write(void *opaque, hwaddr addr, uint64_t val,
                                unsigned size) {
  addr += BUILD_PCIMEM_START;
  addr -= LOONGSON_X86_PCI_MEM_OFFSET;

  if (msix_pass_write(addr, val, size))
    return;

  switch (size) {
  case 1:
    writeb(val, (void *)TO_UNCAC(addr));
    break;
  case 2:
    writew(val, (void *)TO_UNCAC(addr));
    break;
  case 4:
    writel(val, (void *)TO_UNCAC(addr));
    break;
  case 8:
    writeq(val, (void *)TO_UNCAC(addr));
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
