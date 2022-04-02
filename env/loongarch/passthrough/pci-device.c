#include <asm/device.h>
#include <exec/memory.h>
#include <hw/pci/pci.h>
#include <linux/pci.h>
#include <qemu/range.h>

typedef struct {
  u32 offset;
  int entry_num;
} MSIxTable;

/**
 * PCI_BASE_ADDRESS_MEM_TYPE_64 bar need two 32bit to store the mmio address,
 * loongarch and x86's pcie mmio lies in the range of 0 ~ 4G, so top half of
 * PCI_BASE_ADDRESS_MEM_TYPE_64 bar are always 0 or 0xffffffff, if the guest
 * operating system or firmware are rational.
 */
QEMU_BUILD_BUG_ON(LOONGSON_X86_PCI_MEM_OFFSET != 0x80000000);
QEMU_BUILD_BUG_ON(BUILD_PCIMEM_END >= 0xffffffff);
QEMU_BUILD_BUG_ON(LOONGSON_PCI_MEM_END >= 0xffffffff);

enum pci_region_type {
  PCI_REGION_TYPE_IO,
  PCI_REGION_TYPE_MEM,
  PCI_REGION_TYPE_PREFMEM,
  PCI_REGION_TYPE_COUNT,
};

typedef struct {
  bool is_64bit_top_half;
  bool is_64bit_bottom_half;
  int type;
  u64 size;
} BAR;

typedef struct {
  MSIxTable table;
  u8 devfn;
  u16 class;
  u8 header_type;
  BAR bar[PCI_NUM_REGIONS];
} BMBT_PCIExpressDevice;

static inline void init_bar(BAR *bar, bool bottom, bool top, int type,
                            u64 size) {
  bar->is_64bit_bottom_half = bottom;
  bar->is_64bit_top_half = top;
  bar->type = type;
  bar->size = size;
}

// [BMBT_OPTIMIZE 6]
#define MAX_PCI_DEV_NUM 10
static BMBT_PCIExpressDevice pcie_devices[MAX_PCI_DEV_NUM];
static int nr_dev;

static inline int alloc_pci_dev() {
  int res = nr_dev;
  nr_dev++;
  assert(nr_dev < MAX_PCI_DEV_NUM);
  return res;
}

static inline int try_devfn_to_idx(u8 devfn) {
  for (int i = 0; i < nr_dev; ++i) {
    if (pcie_devices[i].devfn == devfn)
      return i;
  }
  return -1;
}

static inline int devfn_to_idx(u8 devfn) {
  int idx = try_devfn_to_idx(devfn);
  assert(idx != -1);
  return idx;
}

static inline BMBT_PCIExpressDevice *devfn_to_dev(u8 devfn) {
  return &pcie_devices[devfn_to_idx(devfn)];
}

static inline MSIxTable *msix_table(int idx) {
  assert(idx < nr_dev);
  return &(pcie_devices[idx].table);
}

int msix_table_overlapped(hwaddr addr, unsigned size) {
  for (int i = 0; i < nr_dev; ++i) {
    MSIxTable *table = msix_table(i);
    if (ranges_overlap(table->offset, table->entry_num * PCI_MSIX_ENTRY_SIZE,
                       addr, size)) {
      return i;
    }
  }
  return -1;
}

static u32 bmbt_pci_bar(BMBT_PCIExpressDevice *pci, int region_num) {
  if (region_num != PCI_ROM_SLOT) {
    return PCI_BASE_ADDRESS_0 + region_num * 4;
  }

  u8 type = pci->header_type & ~PCI_HEADER_TYPE_MULTI_FUNCTION;
  return type == PCI_HEADER_TYPE_BRIDGE ? PCI_ROM_ADDRESS1 : PCI_ROM_ADDRESS;
}

static void pci_bios_get_bar(BMBT_PCIExpressDevice *pci, int bar, int *ptype,
                             u64 *psize, int *pis64) {
  int is64 = 0, type = PCI_REGION_TYPE_MEM;
  u64 mask;

  u32 ofs = bmbt_pci_bar(pci, bar);
  u16 bdf = pci->devfn;
  u32 old;
  pci_bus_read_config_dword(bdf, ofs, &old);

  if (bar == PCI_ROM_SLOT) {
    mask = PCI_ROM_ADDRESS_MASK;
    pci_bus_write_config_dword(bdf, ofs, mask);
  } else {
    if (old & PCI_BASE_ADDRESS_SPACE_IO) {
      mask = PCI_BASE_ADDRESS_IO_MASK;
      type = PCI_REGION_TYPE_IO;
    } else {
      mask = PCI_BASE_ADDRESS_MEM_MASK;
      if (old & PCI_BASE_ADDRESS_MEM_PREFETCH)
        type = PCI_REGION_TYPE_PREFMEM;
      is64 = ((old & PCI_BASE_ADDRESS_MEM_TYPE_MASK) ==
              PCI_BASE_ADDRESS_MEM_TYPE_64);
    }
    pci_bus_write_config_dword(bdf, ofs, ~0);
  }
  u64 val = 0;
  pci_bus_read_config_dword(bdf, ofs, (u32 *)&val);
  pci_bus_write_config_dword(bdf, ofs, old);
  if (is64) {
    u32 hold, high;
    pci_bus_read_config_dword(bdf, ofs + 4, &hold);
    pci_bus_write_config_dword(bdf, ofs + 4, ~0);
    pci_bus_read_config_dword(bdf, ofs + 4, &high);
    pci_bus_write_config_dword(bdf, ofs + 4, hold);
    val |= ((u64)high << 32);
    mask |= ((u64)0xffffffff << 32);
    *psize = (~(val & mask)) + 1;
  } else {
    *psize = ((~(val & mask)) + 1) & 0xffffffff;
  }
  *ptype = type;
  *pis64 = is64;
}

/*  src/fw/pciinit.c:pci_probe_devices */
static void pci_bios_check_devices(BMBT_PCIExpressDevice *pci) {
  for (int i = 0; i < PCI_NUM_REGIONS; i++) {
    if ((pci->class == PCI_CLASS_BRIDGE_PCI) &&
        (i >= PCI_BRIDGE_NUM_REGIONS && i < PCI_ROM_SLOT))
      continue;
    int type, is64;
    u64 size;
    pci_bios_get_bar(pci, i, &type, &size, &is64);

    // wmask equals to (size - 1), everytime x86 guest write PCIe config bar
    // with (X), (X - LOONGSON_X86_PCI_MEM_OFFSET) will be written to loongarch
    // PCIe config. But if wmask is bigger than the LOONGSON_X86_PCI_MEM_OFFSET,
    // x86 guest can't read X again.
    if (size != 0)
      assert(((size - 1) & LOONGSON_X86_PCI_MEM_OFFSET) == 0);

    init_bar(&pci->bar[i], is64, false, type, size);
    if (is64) {
      i++;
      init_bar(&pci->bar[i], false, true, type, size);
    }
  }
}

/* reference seabios/src/hw/pcidevice.c:pci_probe_devices */
int add_PCIe_devices(u8 devfn) {
  u32 classrev;
  BMBT_PCIExpressDevice *pci;
  int idx = try_devfn_to_idx(devfn);
  if (idx != -1)
    return idx;

  idx = alloc_pci_dev();
  pci = &pcie_devices[idx];
  pci->devfn = devfn;

  pci_bus_read_config_dword(devfn, PCI_CLASS_REVISION, &classrev);
  pci->class = classrev >> 16;
  pci_bus_read_config_byte(devfn, PCI_HEADER_TYPE, &(pci->header_type));

  pci_bios_check_devices(pci);

  return idx;
}

int get_msix_table_entry_offset(hwaddr addr, int idx) {
  MSIxTable *table = msix_table(idx);
  int entry = (addr - table->offset) / PCI_MSIX_ENTRY_SIZE;
  int entry_offset = (addr - table->offset) % PCI_MSIX_ENTRY_SIZE;
  assert(entry >= 0 && entry < table->entry_num);
  return entry_offset;
}

u32 pcie_mmio_space_translate(int idx, u32 config_addr, u32 val,
                              bool is_write) {
  BMBT_PCIExpressDevice *pci = &pcie_devices[idx];
  int bar_idx = (config_addr - PCI_BASE_ADDRESS_0) / 4;
  if (bar_idx > 6) {
    assert(config_addr == PCI_ROM_ADDRESS || config_addr == PCI_ROM_ADDRESS1);
    bar_idx = 6;
  }

  BAR *bar = &pci->bar[bar_idx];
  if (bar->is_64bit_top_half) {
    return val;
  }

  if (bar->size == 0)
    return 0;

  if (bar->type == PCI_REGION_TYPE_IO) {
    if (is_write) {
      val -= LOONGSON_X86_PCI_IO_OFFSET;
    } else {
      val += LOONGSON_X86_PCI_IO_OFFSET;
    }
  } else {
    if (is_write) {
      val -= LOONGSON_X86_PCI_MEM_OFFSET;
    } else {
      val += LOONGSON_X86_PCI_MEM_OFFSET;
    }
  }

  return val;
}

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

static inline void add_msix_table(u8 devfn, u32 offset, int entry_num) {
  MSIxTable *table = msix_table(devfn_to_idx(devfn));
  table->offset = offset;
  table->entry_num = entry_num;
}

void msix_map_region(u8 devfn) {
  u32 table_offset;
  u8 bir;
  u32 msix_base_offset;
  u16 entry_num;
  int msix_cap = pci_find_capability(devfn, PCI_CAP_ID_MSIX);
  if (!msix_cap) {
    return;
  }

  pci_bus_read_config_dword(devfn, msix_cap + PCI_MSIX_TABLE, &table_offset);
  pci_bus_read_config_word(devfn, msix_cap + PCI_MSIX_FLAGS, &entry_num);

  bir = (u8)(table_offset & PCI_MSIX_TABLE_BIR);
  pci_bus_read_config_dword(devfn, PCI_BASE_ADDRESS_0 + bir * 4,
                            &msix_base_offset);

  BAR *bar = &(devfn_to_dev(devfn)->bar[bir]);
  assert(bar->is_64bit_bottom_half == false);
  assert(bar->type == PCI_REGION_TYPE_MEM);
  assert((msix_base_offset & ~PCI_BASE_ADDRESS_MEM_MASK) == 0);

  table_offset &= PCI_MSIX_TABLE_OFFSET;
  add_msix_table(devfn, msix_base_offset + table_offset, entry_num);
}
