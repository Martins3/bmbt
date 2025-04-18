#include <asm/cpu-features.h>
#include <asm/device.h>
#include <asm/io.h>
#include <autoconf.h>
#include <linux/pci-ecam.h>
#include <linux/pci.h>
#include <linux/type.h>
#include <stdbool.h>
#include <uglib.h>

/*
 * On 64-bit systems, we do a single ioremap for the whole config space
 * since we have enough virtual address range available.  On 32-bit, we
 * ioremap the config space for each bus individually.
 */
static const bool per_bus_mapping = !CONFIG_64BIT;

int pci_generic_config_write(struct pci_bus *bus, unsigned int devfn, int where,
                             int size, u32 val) {
  void __iomem *addr;

  addr = bus->ops->map_bus(bus, devfn, where);
  if (!addr)
    return PCIBIOS_DEVICE_NOT_FOUND;

  switch (size) {
  case 1:
    writeb(val, addr);
    break;
  case 2:
    writew(val, addr);
    break;
  case 4:
    writel(val, addr);
    break;
  default:
    g_assert_not_reached();
  }

  return PCIBIOS_SUCCESSFUL;
}

int pci_generic_config_read(struct pci_bus *bus, unsigned int devfn, int where,
                            int size, u32 *val) {
  void __iomem *addr;

  addr = bus->ops->map_bus(bus, devfn, where);
  if (!addr) {
    *val = ~0;
    return PCIBIOS_DEVICE_NOT_FOUND;
  }

  switch (size) {
  case 1:
    *val = readb(addr);
    break;
  case 2:
    *val = readw(addr);
    break;
  case 4:
    *val = readl(addr);
    break;
  default:
    g_assert_not_reached();
  }
  return PCIBIOS_SUCCESSFUL;
}

/*
 * Function to implement the pci_ops ->map_bus method
 */
void __iomem *pci_ecam_map_bus(struct pci_bus *bus, unsigned int devfn,
                               int where) {
  struct pci_config_window *cfg = bus->sysdata;
  unsigned int devfn_shift = cfg->ops->bus_shift - 8;
  unsigned int busn = bus->number;
  void __iomem *base;

  if (busn < cfg->busr.start || busn > cfg->busr.end)
    return NULL;

  busn -= cfg->busr.start;
  if (per_bus_mapping)
    base = cfg->winp[busn];
  else
    base = cfg->win + (busn << cfg->ops->bus_shift);
  return base + (devfn << devfn_shift) + where;
}

/* bmbt: loongson_pci_ecam_ops isn't used by the kernel, see
 * pci_acpi_setup_ecam_mapping */
struct pci_ecam_ops pci_generic_ecam_ops = {
    .bus_shift = 20,
    .pci_ops = {
        .map_bus = pci_ecam_map_bus,
        .read = pci_generic_config_read,
        .write = pci_generic_config_write,
    }};

static struct pci_config_window __pci_config_window;

struct pci_config_window *pci_ecam_create(struct pci_ecam_ops *ops) {
  struct pci_config_window *cfg = &__pci_config_window;
  cfg->ops = ops;
  cfg->busr.start = 0;
  cfg->busr.end = 127;
  cfg->win = QEMU_PCI_CONFIG_IO_BASE;
  return cfg;
}

struct pci_config_window *arch_pci_ecam_create(struct pci_ecam_ops *ops) {
  struct pci_config_window *cfg = &__pci_config_window;
  cfg->ops = ops;
  cfg->busr.start = 0;
  cfg->busr.end = 127;
  cfg->win = MCFG_EXT_PCICFG_BASE;
  return cfg;
}

extern struct pci_ecam_ops loongson_pci_ecam_ops;

void loongarch_pci_init() {
  struct pci_config_window *cfg;
  if (cpu_has_hypervisor) {
    cfg = pci_ecam_create(&pci_generic_ecam_ops);
  } else {
    cfg = arch_pci_ecam_create(&loongson_pci_ecam_ops);
  }

  if (cfg->ops->init) {
    cfg->ops->init(cfg);
  }

  init_pci_bus(cfg);
}
