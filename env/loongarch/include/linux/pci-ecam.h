#ifndef PCI_ECAM_H_QNWUDIJ9
#define PCI_ECAM_H_QNWUDIJ9

#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/pci.h>

/*
 * struct to hold the mappings of a config space window. This
 * is expected to be used as sysdata for PCI controllers that
 * use ECAM.
 */
struct pci_config_window {
  struct resource busr;
  struct pci_ecam_ops *ops;
  void *priv;
  union {
    void __iomem *win;   /* 64-bit single mapping */
    void __iomem **winp; /* 32-bit per-bus mapping */
  };
};

struct pci_ecam_ops {
  unsigned int bus_shift;
  struct pci_ops pci_ops;
  int (*init)(struct pci_config_window *);
};

void init_pci_bus(struct pci_config_window *cfg);

#endif /* end of include guard: PCI_ECAM_H_QNWUDIJ9                            \
        */
