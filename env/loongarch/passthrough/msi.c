#ifndef MSI_C_O9QNSIMV
#define MSI_C_O9QNSIMV

#include "internal.h"
#include <asm/device.h>
#include <linux/pci.h>

static inline MSIxTable *msix_table(int idx) {
  return &(get_pcie_dev(idx)->table);
}

int msix_table_overlapped(hwaddr addr, unsigned size) {
  for (int i = 0; i < get_pcie_dev_num(); ++i) {
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

  if (!msix_cap) {
    return;
  }

  pci_bus_read_config_dword(bdf, msix_cap + PCI_MSIX_TABLE, &table_offset);
  pci_bus_read_config_word(bdf, msix_cap + PCI_MSIX_FLAGS, &entry_num);

  bir = (u8)(table_offset & PCI_MSIX_TABLE_BIR);
  table_offset &= PCI_MSIX_TABLE_OFFSET;
  pci_bus_read_config_dword(bdf, PCI_BASE_ADDRESS_0 + bir * 4,
                            &msix_base_offset);

  BAR *bar = &(bdf_to_dev(bdf)->bar[bir]);
  assert(bar->size != 0);

  printf("[huxueshi:%s:%d] bdf=%x msix_base_offset=%x table_offset=%x "
         "entry_num=%x barnum=%d size=%lx\n",
         __FUNCTION__, __LINE__, bdf, msix_base_offset, table_offset, entry_num,
         bir, bar->size);

  /**
   * please check QEMU's function : usb_xhci_pci_realize, and you'll find
   * there's no guarantee for:
   * 1. assert(bar->is_64bit_bottom_half == false);
   * 2. assert((msix_base_offset & ~PCI_BASE_ADDRESS_MEM_MASK) == 0);
   *
   * and it enables both msix and msi. What's more, it only has one valid bar
   * and the bar is used for MSIX
   */

  assert(bar->type == PCI_REGION_TYPE_MEM);
  msix_base_offset &= PCI_BASE_ADDRESS_MEM_MASK;

  add_msix_table(bdf, msix_base_offset + table_offset, entry_num);
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

bool msix_pass_read(hwaddr addr, unsigned size, u32 *result) {
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
      val = translate_msi_entry_data(false, val);
      break;
    case PCI_MSIX_ENTRY_VECTOR_CTRL:
      break;
    default:
      g_assert_not_reached();
    }
    *result = val;
    return true;
  }
  return false;
}

bool msix_pass_write(hwaddr addr, uint64_t val, unsigned size) {
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
      val = translate_msi_entry_data(true, val);
      break;
    case PCI_MSIX_ENTRY_VECTOR_CTRL:
      break;
    default:
      g_assert_not_reached();
    }
    writel(val, (void *)TO_UNCAC(addr));
    return true;
  }
  return false;
}

/* bmbt_linux/drivers/pci/msi.c:__pci_write_msi_msg */
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

#endif /* end of include guard: MSI_C_O9QNSIMV */
