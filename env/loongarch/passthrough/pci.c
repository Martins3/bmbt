#include <asm/device.h>
#include <exec/memory.h>
#include <hw/pci/pci.h>
#include <linux/pci.h>
#include <qemu/range.h>

// functions defined in ./pci-device.c
int add_PCIe_devices(u8 devfn);
int msix_table_overlapped(hwaddr addr, unsigned size);
int get_msix_table_entry_offset(hwaddr addr, int idx);
u32 pcie_mmio_space_translate(int idx, u32 addr, u32 val, bool is_write);
void add_msix_table(u8 devfn, u32 offset, int entry_num);
void msix_map_region(u8 devfn);

u64 msi_message_addr();
u32 pch_msi_allocate_hwirq(unsigned int irq);

static u8 get_devfn(uint32_t addr) { return addr >> 8; }

static u8 get_offset(uint32_t addr) { return addr & 0xff; }

static void pci_config_write(uint32_t addr, uint32_t val, int l) {
  uint8_t devfn = get_devfn(addr);
  uint8_t where = get_offset(addr);
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

// [BMBT_OPTIMIZE 7]
static uint32_t pci_config_read(uint32_t addr, int l) {
  uint32_t val32;
  uint16_t val16;
  uint8_t val8;
  uint8_t devfn = get_devfn(addr);
  uint8_t where = get_offset(addr);
  switch (l) {
  case 1:
    pci_bus_read_config_byte(devfn, where, &val8);
    return val8;
  case 2:
    pci_bus_read_config_word(devfn, where, &val16);
    return val16;
  case 4:
    pci_bus_read_config_dword(devfn, where, &val32);
    return val32;
  default:
    g_assert_not_reached();
  }
}

void pci_pass_through_write(uint32_t addr, uint32_t val, int l) {
  uint32_t config_addr = addr & (PCI_CONFIG_SPACE_SIZE - 1);
  bool msix_table_updated = false;
  if (ranges_overlap(config_addr, l, PCI_BASE_ADDRESS_0, 24) ||
      ranges_overlap(config_addr, l, PCI_ROM_ADDRESS, 4) ||
      ranges_overlap(config_addr, l, PCI_ROM_ADDRESS1, 4)) {
    assert(l == 4);
    val = pcie_mmio_space_translate(add_PCIe_devices(get_devfn(addr)),
                                    config_addr, val, true);
    msix_table_updated = true;
  }

  pci_config_write(addr, val, l);

  if (msix_table_updated)
    msix_map_region(get_devfn(addr));
}

uint32_t pci_pass_through_read(uint32_t addr, int l) {
  uint32_t config_addr = addr & (PCI_CONFIG_SPACE_SIZE - 1);
  if (ranges_overlap(config_addr, l, PCI_BASE_ADDRESS_0, 24) ||
      ranges_overlap(config_addr, l, PCI_ROM_ADDRESS, 4) ||
      ranges_overlap(config_addr, l, PCI_ROM_ADDRESS1, 4)) {
    assert(l == 4);
    u32 val = pci_config_read(addr, l);
    return pcie_mmio_space_translate(add_PCIe_devices(get_devfn(addr)),
                                     config_addr, val, false);
  }
  return pci_config_read(addr, l);
}

static uint64_t pass_mmio_pass_read(void *opaque, hwaddr addr, unsigned size) {
  addr += BUILD_PCIMEM_START;
  addr -= LOONGSON_X86_PCI_MEM_OFFSET;

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
      val |= 0x4100;
      break;
    case PCI_MSIX_ENTRY_VECTOR_CTRL:
      break;
    default:
      g_assert_not_reached();
    }
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

  int idx = msix_table_overlapped(addr, size);
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
      // see arch/x86/kernel/apic/msi.c:irq_msi_compose_msg
      // maybe fail on other operating system
      assert((val & 0xff00) == 0x4100);
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
