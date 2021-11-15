#ifndef PIC_HOST_H_VY7F9MVR
#define PIC_HOST_H_VY7F9MVR

#include "../../exec/memory.h"
#include "../../qemu/queue.h"
#include "pci_bus.h"

typedef struct PCIHostState {
  // SysBusDevice busdev;

  MemoryRegion conf_mem;
  MemoryRegion data_mem;
  MemoryRegion mmcfg;
  uint32_t config_reg;
  PCIBus *bus;

  QLIST_ENTRY(PCIHostState) next;
} PCIHostState;

typedef struct PCIHostBridgeClass {
  // SysBusDeviceClass parent_class;

  const char *(*root_bus_path)(PCIHostState *, PCIBus *);
} PCIHostBridgeClass;

/* common internal helpers for PCI/PCIe hosts, cut off overflows */
void pci_host_config_write_common(PCIDevice *pci_dev, uint32_t addr,
                                  uint32_t limit, uint32_t val, uint32_t len);
uint32_t pci_host_config_read_common(PCIDevice *pci_dev, uint32_t addr,
                                     uint32_t limit, uint32_t len);

void pci_data_write(PCIBus *s, uint32_t addr, uint32_t val, int len);
uint32_t pci_data_read(PCIBus *s, uint32_t addr, int len);

extern const MemoryRegionOps pci_host_conf_le_ops;
extern const MemoryRegionOps pci_host_conf_be_ops;
extern const MemoryRegionOps pci_host_data_le_ops;
extern const MemoryRegionOps pci_host_data_be_ops;

#endif /* end of include guard: PIC_HOST_H_VY7F9MVR */
