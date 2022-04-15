#ifndef INTERNAL_H_WCRWRL9K
#define INTERNAL_H_WCRWRL9K

#include <hw/pci/pci.h>

int add_PCIe_devices(u16 bdf);
int msix_table_overlapped(hwaddr addr, unsigned size);
int get_msix_table_entry_offset(hwaddr addr, int idx);
u32 pcie_bar_translate(uint32_t addr, int l, u32 val, bool is_write,
                       bool *msix_table_overlapped);
u32 pcie_bridge_window_translate(int idx, uint32_t addr, int l, u32 val,
                                 bool is_write);
void msix_map_region(u16 bdf);

u64 msi_message_addr();
u32 pch_msi_allocate_hwirq(unsigned int irq);

static inline uint32_t get_config_addr(uint32_t addr) {
  return addr & (PCI_CONFIG_SPACE_SIZE - 1);
}

static inline u16 get_bdf(uint32_t addr) { return addr >> 8; }

/*
 * 0x80000000 | (offset & 0xf00) << 16 |
 *    bus << 16 | device << 11 | function << 8 | (offset & 0xff)
 * */
static inline int get_where(uint32_t addr) {
  return (addr & 0xff) | ((addr >> 16) & 0xf00);
}

bool is_pci_bridge(int idx);
#endif /* end of include guard: INTERNAL_H_WCRWRL9K */
