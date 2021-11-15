#ifndef I440FX_H_DGSFF3CQ
#define I440FX_H_DGSFF3CQ

#include "../../exec/memory.h"
#include "../../hw/pci/pci_bus.h"

typedef struct PCII440FXState PCII440FXState;

#define TYPE_I440FX_PCI_HOST_BRIDGE "i440FX-pcihost"
#define TYPE_I440FX_PCI_DEVICE "i440FX"

#define TYPE_IGD_PASSTHROUGH_I440FX_PCI_DEVICE "igd-passthrough-i440FX"

PCIBus *i440fx_init(const char *host_type, const char *pci_type,
                    PCII440FXState **pi440fx_state,
                    MemoryRegion *address_space_mem,
                    MemoryRegion *address_space_io, ram_addr_t ram_size,
                    ram_addr_t below_4g_mem_size, ram_addr_t above_4g_mem_size,
                    MemoryRegion *pci_memory, MemoryRegion *ram_memory);

#endif /* end of include guard: I440FX_H_DGSFF3CQ */
