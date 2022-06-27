#include "internal.h"
#include <asm/device.h>
#include <exec/memory.h>
#include <hw/pci/pci.h>
#include <linux/pci.h>
#include <qemu/range.h>

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

// TMP_TODO 将这个名称和 MMIO 的修改对称一点
void pci_pass_through_write(uint32_t addr, uint32_t val, int l) {
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
    int idx = add_PCIe_devices(get_bdf(addr));

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

uint32_t pci_pass_through_read(uint32_t addr, int l) {
  uint32_t config_addr = get_config_addr(addr);

  if (!ranges_overlap(config_addr, l, 0, PCI_BASE_ADDRESS_0)) {
    bool unused;
    int idx = add_PCIe_devices(get_bdf(addr));
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

#include "cpu.h"

#define ENABLE_MSIX 1

static uint64_t pass_mmio_pass_read(void *opaque, hwaddr addr, unsigned size) {
  addr += BUILD_PCIMEM_START;
  addr -= LOONGSON_X86_PCI_MEM_OFFSET;

#ifdef ENABLE_MSIX
  int idx = msix_table_overlapped(addr, size);
  if (idx != -1) {
    assert(size == 4); // QEMU say guest access size is always 4
    int entry_offset = get_msix_table_entry_offset(addr, idx);
    u32 val = readl((void *)TO_UNCAC(addr));
    switch (entry_offset) {
    case PCI_MSIX_ENTRY_LOWER_ADDR:
      assert(val == msi_message_addr());
      val = X86_MSI_ADDR_BASE_LO;
      break;
    case PCI_MSIX_ENTRY_UPPER_ADDR:
      assert(val == 0);
      break;
    case PCI_MSIX_ENTRY_DATA:
      assert(val > 0 && val < 256);
      val |= X86_MSI_ENTRY_DATA_FLAG;
      break;
    case PCI_MSIX_ENTRY_VECTOR_CTRL:
      break;
    default:
      g_assert_not_reached();
    }
    return val;
  }
#endif

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

#ifdef ENABLE_MSIX
  int idx = msix_table_overlapped(addr, size);
  // TMP_TODO 将这个放到 PCI-device.c 中吧
  if (idx != -1) {
    assert(size == 4); // QEMU tell me guest access size is 4
    assert((val >> 32) == 0);
    int entry_offset = get_msix_table_entry_offset(addr, idx);
    switch (entry_offset) {
    case PCI_MSIX_ENTRY_LOWER_ADDR:
      assert(val == X86_MSI_ADDR_BASE_LO);
      val = msi_message_addr();
      break;
    case PCI_MSIX_ENTRY_UPPER_ADDR:
      assert(val == 0);
      break;
    case PCI_MSIX_ENTRY_DATA:
      // TMP_TODO 将这个合并一下
      // see arch/x86/kernel/apic/msi.c:irq_msi_compose_msg
      // maybe fail on other operating system
      assert((val & 0xff00) == X86_MSI_ENTRY_DATA_FLAG);
      val &= 0xff;
      assert(val > 0 && val < 256);
      val = pch_msi_allocate_hwirq(val);
      break;
    case PCI_MSIX_ENTRY_VECTOR_CTRL:
      break;
    default:
      g_assert_not_reached();
    }
    writel(val, (void *)TO_UNCAC(addr));
    return;
  }
#endif

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
