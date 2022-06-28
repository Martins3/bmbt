#ifndef INTERNAL_H_WCRWRL9K
#define INTERNAL_H_WCRWRL9K

#include <hw/pci/pci.h>
#include <linux/type.h>

enum pci_region_type {
  PCI_REGION_TYPE_IO,
  PCI_REGION_TYPE_MEM,
  PCI_REGION_TYPE_PREFMEM,
  PCI_REGION_TYPE_COUNT,
};

typedef struct {
  u32 offset;
  int entry_num;
} MSIxTable;

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

int add_pcie_devices(u16 bdf);
int msix_table_overlapped(hwaddr addr, unsigned size);
int get_msix_table_entry_offset(hwaddr addr, int idx);
u32 pcie_bar_translate(uint32_t addr, int l, u32 val, bool is_write,
                       bool *msix_table_overlapped);
u32 pcie_bridge_window_translate(int idx, uint32_t addr, int l, u32 val,
                                 bool is_write);
void msix_map_region(u16 bdf);

u64 msi_message_addr();
u32 pch_msi_allocate_hwirq(unsigned int irq);

BMBT_PCIExpressDevice *get_pcie_dev(int idx);
int bdf_to_idx(u16 bdf);
int get_pcie_dev_num();

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

static inline BMBT_PCIExpressDevice *bdf_to_dev(u16 bdf) {
  return get_pcie_dev(bdf_to_idx(bdf));
}

uint32_t msi_translate(uint32_t addr, uint32_t val, bool is_write);

bool is_pci_bridge(int idx);
int pci_find_capability(u16 bdf, int cap);
const char *show_bdf(u16 bdf);

bool msix_pass_write(hwaddr addr, uint64_t val, unsigned size);
bool msix_pass_read(hwaddr addr, unsigned size, u32 *result);

#endif /* end of include guard: INTERNAL_H_WCRWRL9K */
