#ifndef PCI_H_65PSRQAZ
#define PCI_H_65PSRQAZ

#include <asm/io.h>
struct pci_bus {
  struct pci_ops *ops;  /* Configuration access functions */
  void *sysdata;        /* Hook for sys-specific extension */
  unsigned char number; /* Bus number */
};

/* Error values that may be returned by PCI functions */
#define PCIBIOS_SUCCESSFUL 0x00
#define PCIBIOS_FUNC_NOT_SUPPORTED 0x81
#define PCIBIOS_BAD_VENDOR_ID 0x83
#define PCIBIOS_DEVICE_NOT_FOUND 0x86
#define PCIBIOS_BAD_REGISTER_NUMBER 0x87
#define PCIBIOS_SET_FAILED 0x88
#define PCIBIOS_BUFFER_TOO_SMALL 0x89

/* Low-level architecture-dependent routines */

struct pci_ops {
  int (*add_bus)(struct pci_bus *bus);
  void (*remove_bus)(struct pci_bus *bus);
  void __iomem *(*map_bus)(struct pci_bus *bus, unsigned int devfn, int where);
  int (*read)(struct pci_bus *bus, unsigned int devfn, int where, int size,
              u32 *val);
  int (*write)(struct pci_bus *bus, unsigned int devfn, int where, int size,
               u32 val);
};

int pci_bus_read_config_byte(unsigned int devfn, int where, u8 *val);
int pci_bus_read_config_word(unsigned int devfn, int where, u16 *val);
int pci_bus_read_config_dword(unsigned int devfn, int where, u32 *val);
int pci_bus_write_config_byte(unsigned int devfn, int where, u8 val);
int pci_bus_write_config_word(unsigned int devfn, int where, u16 val);
int pci_bus_write_config_dword(unsigned int devfn, int where, u32 val);

// bmbt: pci pass through
void pci_pass_through_write(uint32_t addr, uint32_t val, int l);
uint32_t pci_pass_through_read(uint32_t addr, int l);
void pci_pass_through_init();
void loongarch_pci_init();
#endif /* end of include guard: PCI_H_65PSRQAZ */
