#include "internal.h"
#include <asm/device.h>
#include <exec/memory.h>
#include <hw/pci/pci.h>
#include <linux/pci.h>
#include <qemu/range.h>

static const char *show_bdf(u16 bdf) {
  static char tmp[100];
  int bus = bdf >> 8;
  int dev = (bdf >> 3) & 0x1f;
  int func = bdf & 0x7;
  sprintf(tmp, "%02x:%02x:%x", bus, dev, func);
  return tmp;
}

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
/* QEMU_BUILD_BUG_ON(LOONGSON_X86_PCI_MEM_OFFSET != 0x80000000); */
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
  u16 bdf;
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

// TMP_TODO 现在这里超过了 30 个，也许之前的算法有问题了
// [BMBT_OPTIMIZE 6]
#define MAX_PCI_DEV_NUM 30
static BMBT_PCIExpressDevice pcie_devices[MAX_PCI_DEV_NUM];
static int nr_dev;

static inline BMBT_PCIExpressDevice *get_pcie_dev(int idx) {
  assert(idx >= 0 && idx < nr_dev);
  return &(pcie_devices[idx]);
}

static inline int alloc_pcie_dev() {
  int res = nr_dev;
  nr_dev++;
  assert(nr_dev < MAX_PCI_DEV_NUM);
  return res;
}

static inline int try_bdf_to_idx(u16 bdf) {
  for (int i = 0; i < nr_dev; ++i) {
    if (get_pcie_dev(i)->bdf == bdf)
      return i;
  }
  return -1;
}

static inline int bdf_to_idx(u16 bdf) {
  int idx = try_bdf_to_idx(bdf);
  assert(idx != -1);
  return idx;
}

static inline BMBT_PCIExpressDevice *bdf_to_dev(u16 bdf) {
  return get_pcie_dev(bdf_to_idx(bdf));
}

static inline MSIxTable *msix_table(int idx) {
  return &(get_pcie_dev(idx)->table);
}

// TMP_TODO 实际上，将两个代码直接
int msix_table_overlapped(hwaddr addr, unsigned size) {
  for (int i = 0; i < nr_dev; ++i) {
    MSIxTable *table = msix_table(i);
    if (table->offset != 0) {
      if (ranges_overlap(table->offset, table->entry_num * PCI_MSIX_ENTRY_SIZE,
                         addr, size)) {
        return i;
      }
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
  u16 bdf = pci->bdf;
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
  printf("[huxueshi:%s:%d] bdf=%x barnum=%d low=%x\n", __FUNCTION__, __LINE__,
         pci->bdf, ofs, old);
  if (is64) {
    u32 hold, high;
    pci_bus_read_config_dword(bdf, ofs + 4, &hold);
    pci_bus_write_config_dword(bdf, ofs + 4, ~0);
    pci_bus_read_config_dword(bdf, ofs + 4, &high);
    pci_bus_write_config_dword(bdf, ofs + 4, hold);
    val |= ((u64)high << 32);
    printf("[huxueshi:%s:%d] bdf=%x barnum=%d high=%x\n", __FUNCTION__,
           __LINE__, pci->bdf, ofs, hold);
    mask |= ((u64)0xffffffff << 32);
    *psize = (~(val & mask)) + 1;
  } else {
    *psize = ((~(val & mask)) + 1) & 0xffffffff;
  }
  printf("[huxueshi:%s:%d] bdf=%x barnum=%d size=%lx\n", __FUNCTION__, __LINE__,
         pci->bdf, ofs, *psize);
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
    if (size != 0) {
      printf("[huxueshi:%s:%d] bdf=%x bar=%d size=%lx\n", __FUNCTION__,
             __LINE__, pci->bdf, i, size);
      // TMP_TODO 和做主板的核实一下吧，很难啊!
      if (pci->bdf == 0xb8) {
        size = 0;
      } else {
        assert(((size - 1) & LOONGSON_X86_PCI_MEM_OFFSET) == 0);
      }
    }

    init_bar(&pci->bar[i], is64, false, type, size);
    if (is64) {
      i++;
      init_bar(&pci->bar[i], false, true, type, size);
    }
  }
}

/* reference seabios/src/hw/pcidevice.c:pci_probe_devices */
int add_PCIe_devices(u16 bdf) {
  u32 classrev;
  BMBT_PCIExpressDevice *pci;
  int idx = try_bdf_to_idx(bdf);
  if (idx != -1)
    return idx;

  idx = alloc_pcie_dev();
  pci = get_pcie_dev(idx);
  pci->bdf = bdf;

  pci_bus_read_config_dword(bdf, PCI_CLASS_REVISION, &classrev);
  pci->class = classrev >> 16;
  pci_bus_read_config_byte(bdf, PCI_HEADER_TYPE, &(pci->header_type));
  printf("[huxueshi:%s:%d] bdf=%x type=%d\n", __FUNCTION__, __LINE__, pci->bdf,
         is_pci_bridge(idx));

  // TMP_TODO 应该对于 header type 做点假设才可以的啊

  pci_bios_check_devices(pci);

  return idx;
}

int get_msix_table_entry_offset(hwaddr addr, int idx) {
  MSIxTable *table = msix_table(idx);
  int entry = (addr - table->offset) / PCI_MSIX_ENTRY_SIZE;
  int entry_offset = (addr - table->offset) % PCI_MSIX_ENTRY_SIZE;
  if (!(entry >= 0 && entry < table->entry_num)) {
    BMBT_PCIExpressDevice *pci = get_pcie_dev(idx);
    printf("[huxueshi:%s:%d] bdf=%x table->offset=%d table->entry_num=%d\n",
           __FUNCTION__, __LINE__, pci->bdf, table->offset, table->entry_num);
    assert(false);
  }
  return entry_offset;
}

static bool is_bar_access(BMBT_PCIExpressDevice *pci, int l,
                          uint32_t config_addr) {
  if (l != 4) {
    assert(pci->class == PCI_CLASS_BRIDGE_PCI);
    assert(!ranges_overlap(config_addr, l, PCI_BASE_ADDRESS_0, 8));
    return false;
  }

  // TMP_TODO 使用 is_pci_bridge 来替代一下
  if (pci->class == PCI_CLASS_BRIDGE_PCI) {
    return ranges_overlap(config_addr, l, PCI_BASE_ADDRESS_0, 8) ||
           ranges_overlap(config_addr, l, PCI_ROM_ADDRESS1, 4);
  }

  return true;
}

/**
 * This is the base of following algorithm, Guest access size is bigger than
 * pcie bridge's window config field.
 *
 * The algorithm is simple
 * 1. shift
 * 2. minus the offset
 * 3. shift back
 */
static inline void assert_access_cover_config(u32 config_addr, int l,
                                              int offset, int size) {
  bool x = (config_addr <= offset && config_addr + l >= offset + size);
  // see seabios/src/fw/pciinit.c:pci_bridge_has_region
  bool y = (config_addr == offset);
  assert(x || y);
}

static inline u32 clear_lower_bits(u32 val, int s) {
  assert(s >= 0 && s <= 3);
  s *= 8;
  return (val >> s) << s;
}

static inline u32 clear_upper_bits(u32 val, int s) {
  assert(s >= 0 && s <= 3);
  s *= 8;
  return (val << s) >> s;
}

u32 get_value_in_range(u32 val, int offset, int size) {
  u32 tmp1 = clear_lower_bits(val, offset);
  u32 tmp2 = clear_upper_bits(val, 4 - offset - size);
  return tmp1 & tmp2;
}

u32 get_value_out_range(u32 val, int offset, int size) {
  return val ^ get_value_in_range(val, offset, size);
}

u32 win_trans(u32 config_addr, int l, int offset, int size, bool is_write,
              u32 val, u32 shift) {
  if (ranges_overlap(config_addr, l, offset, size)) {
    assert_access_cover_config(config_addr, l, offset, size);
    u32 val_in = get_value_in_range(val, offset - config_addr, size);
    u32 val_out = get_value_out_range(val, offset - config_addr, size);
    val_in = val_in >> (offset - config_addr) * 8;
    if (is_write) {
      val_in -= shift;
    } else {
      val_in += shift;
    }
    val_in = val_in << (offset - config_addr) * 8;
    val = val_in | val_out;
  }
  return val;
}

#define PCI_IO_SHIFT 8
#define PCI_MEMORY_SHIFT 16
#define PCI_PREF_MEMORY_SHIFT 16

#define IO_SHIFT (LOONGSON_X86_PCI_IO_OFFSET >> PCI_IO_SHIFT)
#define MEM_SHIFT (LOONGSON_X86_PCI_MEM_OFFSET >> PCI_MEMORY_SHIFT)
#define PREF_MEM_SHIFT (LOONGSON_X86_PCI_MEM_OFFSET >> PCI_PREF_MEMORY_SHIFT)

/**
 * To understand the how pci bridge works:
 * 1. read the source code : qemu/hw/pci/pci_bridge.c
 * 2. https://www.cnblogs.com/LoyenWang/p/14165852.html
 */
u32 pcie_bridge_window_translate(int idx, uint32_t addr, int l, u32 val,
                                 bool is_write) {
  u32 config_addr = get_config_addr(addr);
  if (is_pci_bridge(idx)) {
    val = win_trans(config_addr, l, PCI_IO_BASE, 1, is_write, val, IO_SHIFT);
    val = win_trans(config_addr, l, PCI_IO_LIMIT, 1, is_write, val, IO_SHIFT);
    val =
        win_trans(config_addr, l, PCI_MEMORY_BASE, 2, is_write, val, MEM_SHIFT);
    val = win_trans(config_addr, l, PCI_MEMORY_LIMIT, 2, is_write, val,
                    MEM_SHIFT);
    val = win_trans(config_addr, l, PCI_PREF_MEMORY_BASE, 2, is_write, val,
                    PREF_MEM_SHIFT);
    val = win_trans(config_addr, l, PCI_PREF_MEMORY_LIMIT, 2, is_write, val,
                    PREF_MEM_SHIFT);
  }

  return val;
}

u32 pcie_bar_translate(uint32_t addr, int l, u32 val, bool is_write,
                       bool *msix_table_updated) {
  int idx = add_PCIe_devices(get_bdf(addr));
  u32 config_addr = get_config_addr(addr);
  BMBT_PCIExpressDevice *pci = get_pcie_dev(idx);
  *msix_table_updated = false;
  if (!is_bar_access(pci, l, config_addr)) {
    return val;
  }

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
    *msix_table_updated = true;
  }

  return val;
}

static int __pci_find_next_cap_ttl(u16 bdf, u8 pos, int cap, int *ttl) {
  u8 id;
  u16 ent;

  pci_bus_read_config_byte(bdf, pos, &pos);

  while ((*ttl)--) {
    if (pos < 0x40)
      break;
    pos &= ~3;
    pci_bus_read_config_word(bdf, pos, &ent);

    id = ent & 0xff;
    if (id == 0xff)
      break;
    if (id == cap)
      return pos;
    pos = (ent >> 8);
  }
  return 0;
}

static int __pci_find_next_cap(u16 bdf, u8 pos, int cap) {
  int ttl = PCI_FIND_CAP_TTL;

  return __pci_find_next_cap_ttl(bdf, pos, cap, &ttl);
}

static int __pci_bus_find_cap_start(u16 bdf, u8 hdr_type) {
  u16 status;

  pci_bus_read_config_word(bdf, PCI_STATUS, &status);
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

int pci_find_capability(u16 bdf, int cap) {
  int pos;
  u8 hdr_type;

  pci_bus_read_config_byte(bdf, PCI_HEADER_TYPE, &hdr_type);
  pos = __pci_bus_find_cap_start(bdf, hdr_type);
  if (pos)
    pos = __pci_find_next_cap(bdf, pos, cap);

  return pos;
}

static inline void add_msix_table(u16 bdf, u32 offset, int entry_num) {
  MSIxTable *table = msix_table(bdf_to_idx(bdf));
  table->offset = offset;
  table->entry_num = entry_num;
}

void msix_map_region(u16 bdf) {
  u32 table_offset;
  u8 bir;
  u32 msix_base_offset;
  u16 entry_num;
  int msix_cap = pci_find_capability(bdf, PCI_CAP_ID_MSIX);

  int msi_cap = pci_find_capability(bdf, PCI_CAP_ID_MSI);
  if (msi_cap) {
    printf("[huxueshi:%s:%d] %s\n", __FUNCTION__, __LINE__, show_bdf(bdf));
  }

  if (!msix_cap) {
    return;
  }

  // TMP_TODO 需要优化一下如何，根本没必要每次 bir 之类的
  pci_bus_read_config_dword(bdf, msix_cap + PCI_MSIX_TABLE, &table_offset);
  pci_bus_read_config_word(bdf, msix_cap + PCI_MSIX_FLAGS, &entry_num);

  bir = (u8)(table_offset & PCI_MSIX_TABLE_BIR);
  pci_bus_read_config_dword(bdf, PCI_BASE_ADDRESS_0 + bir * 4,
                            &msix_base_offset);

  BAR *bar = &(bdf_to_dev(bdf)->bar[bir]);
  if (bar->size == 0) {
    printf("[huxueshi:%s:%d] oooooooo ???\n", __FUNCTION__, __LINE__);
  }

  printf("[huxueshi:%s:%d] bdf=%x msix_base_offset=%x table_offset=%x "
         "entry_num=%x barnum=%d size=%lx\n",
         __FUNCTION__, __LINE__, bdf, msix_base_offset, table_offset, entry_num,
         bir, bar->size);
  // 很有可能根本没人初始化这个东西啊
  /* TMP_TODO // 如果这个 assert 取消，那么很多的地方都需要修改关于 msix table
   * 的数值长度
   */
  /* assert(bar->is_64bit_bottom_half == false); */
  /* assert((msix_base_offset & ~PCI_BASE_ADDRESS_MEM_MASK) == 0); */

  assert(bar->type == PCI_REGION_TYPE_MEM);
  msix_base_offset &= PCI_BASE_ADDRESS_MEM_MASK;

  table_offset &= PCI_MSIX_TABLE_OFFSET;
  add_msix_table(bdf, msix_base_offset + table_offset, entry_num);
}

bool is_pci_bridge(int idx) {
  BMBT_PCIExpressDevice *pci = get_pcie_dev(idx);
  return pci->class == PCI_CLASS_BRIDGE_PCI;
}

uint32_t translate_msi_entry_data(bool is_write, uint32_t val) {
  // see arch/x86/kernel/apic/msi.c:irq_msi_compose_msg
  // maybe fail on other operating system
  if (is_write) {
    assert((val & 0xff00) == X86_MSI_ENTRY_DATA_FLAG);
    val &= 0xff;
    assert(val > 0 && val < 256);
    val = pch_msi_allocate_hwirq(val);
  } else {
    assert(val > 0 && val < 256);
    val |= X86_MSI_ENTRY_DATA_FLAG;
  }
  return val;
}

/* bmbt_linux/drivers/pci/msi.c:__pci_write_msi_msg */
// TMP_TODO 应该给 pci_pass_through_read 也使用上吧
uint32_t msi_translate(uint32_t addr, uint32_t val, bool is_write) {
  u16 bdf = get_bdf(addr);
  uint32_t config_addr = get_config_addr(addr);
  int msi_cap = pci_find_capability(bdf, PCI_CAP_ID_MSI);
  int cap_offset = config_addr - msi_cap;
  if (!msi_cap || cap_offset < 0) {
    return val;
  }

  printf("[huxueshi:%s:%d] bdf=%s\n", __FUNCTION__, __LINE__, show_bdf(bdf));

  u16 msgctl;
  pci_bus_read_config_word(bdf, msi_cap + PCI_MSI_FLAGS, &msgctl);
  bool is_64 = !!(msgctl & PCI_MSI_FLAGS_64BIT);
  if (cap_offset == PCI_MSI_ADDRESS_LO) {
    printf("[huxueshi:%s:%d] addr lo val=%x\n", __FUNCTION__, __LINE__, val);
    assert(val == X86_MSI_ADDR_BASE_LO);
    return msi_message_addr();
  }

  if (is_64 && cap_offset == PCI_MSI_ADDRESS_HI) {
    assert(val == 0);
    return val;
  }

  if (is_64 && cap_offset == PCI_MSI_DATA_64) {
    printf("[huxueshi:%s:%d] data 64 val=%x\n", __FUNCTION__, __LINE__, val);
    return translate_msi_entry_data(is_write, val);
  }

  if (!is_64 && cap_offset == PCI_MSI_DATA_32) {
    printf("[huxueshi:%s:%d] data 32 val=%x\n", __FUNCTION__, __LINE__, val);
    return translate_msi_entry_data(is_write, val);
  }

  return val;
}
