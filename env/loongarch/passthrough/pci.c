#include <asm/device.h>
#include <exec/memory.h>
#include <hw/pci/pci.h>
#include <linux/pci.h>
#include <qemu/range.h>

static u8 get_devfn(uint32_t addr) { return addr >> 8; }

static u8 get_offset(uint32_t addr) { return addr & 0xff; }

static int __pci_find_next_cap_ttl(unsigned int devfn, u8 pos, int cap,
                                   int *ttl) {
  u8 id;
  u16 ent;

  pci_bus_read_config_byte(devfn, pos, &pos);

  while ((*ttl)--) {
    if (pos < 0x40)
      break;
    pos &= ~3;
    pci_bus_read_config_word(devfn, pos, &ent);

    id = ent & 0xff;
    if (id == 0xff)
      break;
    if (id == cap)
      return pos;
    pos = (ent >> 8);
  }
  return 0;
}

static int __pci_find_next_cap(unsigned int devfn, u8 pos, int cap) {
  int ttl = PCI_FIND_CAP_TTL;

  return __pci_find_next_cap_ttl(devfn, pos, cap, &ttl);
}

static int __pci_bus_find_cap_start(unsigned int devfn, u8 hdr_type) {
  u16 status;

  pci_bus_read_config_word(devfn, PCI_STATUS, &status);
  if (!(status & PCI_STATUS_CAP_LIST))
    return 0;

  switch (hdr_type) {
  case PCI_HEADER_TYPE_NORMAL:
  case PCI_HEADER_TYPE_BRIDGE:
    return PCI_CAPABILITY_LIST;
  case PCI_HEADER_TYPE_CARDBUS:
    return PCI_CB_CAPABILITY_LIST;
  }

  return 0;
}

int pci_find_capability(unsigned int devfn, int cap) {
  int pos;
  u8 hdr_type;

  pci_bus_read_config_byte(devfn, PCI_HEADER_TYPE, &hdr_type);
  pos = __pci_bus_find_cap_start(devfn, hdr_type);
  if (pos)
    pos = __pci_find_next_cap(devfn, pos, cap);

  return pos;
}

typedef struct {
  u32 offset;
  int entry_num;
  u8 devfn;
} msix_table;

// [BMBT_OPTIMIZE 6]
#define MAX_MSI_TBALE 10
static msix_table tables[MAX_MSI_TBALE];
static int table_num;

void insert_msix_table(u8 devfn, u32 offset, int entry_num) {
  for (int i = 0; i < table_num; ++i) {
    if (tables[i].devfn == devfn) {
      assert(tables[i].entry_num == entry_num);
      tables[i].offset = offset;
      return;
    }
  }

  tables[table_num].offset = offset;
  tables[table_num].devfn = devfn;
  tables[table_num].entry_num = entry_num;
  table_num++;
  assert(table_num < MAX_MSI_TBALE);
}

int msix_table_overlapped(hwaddr addr, unsigned size) {
  for (int i = 0; i < table_num; ++i) {
    if (ranges_overlap(tables[i].offset,
                       tables[i].entry_num * PCI_MSIX_ENTRY_SIZE, addr, size)) {
      return i;
    }
  }
  return -1;
}

static void msix_map_region(unsigned int devfn) {
  u32 table_offset;
  u8 bir;
  u32 msix_bir_offset;
  u16 entry_num;
  int msix_cap = pci_find_capability(devfn, PCI_CAP_ID_MSIX);
  if (!msix_cap) {
    return;
  }

  pci_bus_read_config_dword(devfn, msix_cap + PCI_MSIX_TABLE, &table_offset);
  bir = (u8)(table_offset & PCI_MSIX_TABLE_BIR);
  pci_bus_read_config_dword(devfn, PCI_BASE_ADDRESS_0 + bir * 4,
                            &msix_bir_offset);
  table_offset &= PCI_MSIX_TABLE_OFFSET;

  pci_bus_read_config_word(devfn, msix_cap + PCI_MSIX_FLAGS, &entry_num);
  insert_msix_table(devfn, msix_bir_offset + table_offset, entry_num);
}

static void unimplemented(const char *str) {
  printf("unimplemented : %s\n", str);
  g_assert_not_reached();
}

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
  if (ranges_overlap(config_addr, l, PCI_BASE_ADDRESS_0, 24)) {
    assert(l == 4);

    uint32_t orig_val = pci_config_read(addr, l);
    if ((val & 0xff000000) != 0xff000000 && val != 0) {
      if (orig_val & PCI_BASE_ADDRESS_MEM_TYPE_64)
        unimplemented("64bit mmio");

      if (orig_val & PCI_BASE_ADDRESS_SPACE_IO) {
        assert(val >= X86_PCI_IO_START && val <= X86_PCI_IO_END);
        val -= LOONGSON_X86_PCI_IO_OFFSET;
      } else {
        assert(val >= BUILD_PCIMEM_START && val <= BUILD_PCIMEM_END);
        val -= LOONGSON_X86_PCI_MEM_OFFSET;
        msix_table_updated = true;
      }
    }
  }

  if (ranges_overlap(config_addr, l, PCI_ROM_ADDRESS, 4) ||
      ranges_overlap(config_addr, l, PCI_ROM_ADDRESS1, 4)) {
    return;
  }

  pci_config_write(addr, val, l);

  if (msix_table_updated)
    msix_map_region(get_devfn(addr));
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

u64 msi_message_addr();
u32 pch_msi_allocate_hwirq(unsigned int irq);

static uint64_t pass_mmio_pass_read(void *opaque, hwaddr addr, unsigned size) {
  addr += BUILD_PCIMEM_START;
  addr -= LOONGSON_X86_PCI_MEM_OFFSET;

  int idx = msix_table_overlapped(addr, size);
  if (idx != -1) {
    assert(size == 4); // QEMU tell me guest access size is 4
    int entry = (addr - tables[idx].offset) / PCI_MSIX_ENTRY_SIZE;
    int entry_offset = (addr - tables[idx].offset) % PCI_MSIX_ENTRY_SIZE;
    assert(entry >= 0 && entry < tables[idx].entry_num);
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
    int entry = (addr - tables[idx].offset) / PCI_MSIX_ENTRY_SIZE;
    int entry_offset = (addr - tables[idx].offset) % PCI_MSIX_ENTRY_SIZE;
    assert(entry >= 0 && entry < tables[idx].entry_num);

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
