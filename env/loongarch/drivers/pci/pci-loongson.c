/* copied from linux kernel drivers/pci/controller/pci-loongson.c */

#include <asm/io.h>
#include <linux/bits.h>
#include <linux/pci-ecam.h>
#include <linux/pci.h>
#include <standard-headers/linux/pci_regs.h> // for PCI_CFG_SPACE_EXP_SIZE

// put here temporarily, maybe a better way !
// /home/maritns3/core/linux-4.19-loongson/include/uapi/linux/pci.h
/*
 * The PCI interface treats multi-function devices as independent
 * devices.  The slot/function address of each device is encoded
 * in a single byte as follows:
 *
 *	7:3 = slot
 *	2:0 = function
 */
#define PCI_DEVFN(slot, func) ((((slot)&0x1f) << 3) | ((func)&0x07))
#define PCI_SLOT(devfn) (((devfn) >> 3) & 0x1f)
#define PCI_FUNC(devfn) ((devfn)&0x07)

#define FLAG_CFG0 BIT(0)
#define FLAG_CFG1 BIT(1)
#define FLAG_DEV_FIX BIT(2)

struct pcie_controller_data {
  u64 flags;
  u32 cfg0_type1_offset;
  u32 cfg1_type1_offset;
  struct pci_ops *ops;
};

struct loongson_pci {
  void __iomem *cfg0_base;
  void __iomem *cfg1_base;
  struct platform_device *pdev;
  struct pcie_controller_data *data;
};

static struct loongson_pci *pci_bus_to_loongson_pci(struct pci_bus *bus) {
  struct pci_config_window *cfg;

#ifdef BMBT
  if (acpi_disabled)
    return (struct loongson_pci *)(bus->sysdata);
#endif

  cfg = bus->sysdata;
  return (struct loongson_pci *)(cfg->priv);
}
static void __iomem *cfg1_map(struct loongson_pci *priv, int bus,
                              unsigned int devfn, int where,
                              int cfg_type1_offset) {
  unsigned long addroff = 0x0;

  if (bus != 0)
    addroff |= BIT(cfg_type1_offset); /* Type 1 Access */
  addroff |= (where & 0xff) | ((where & 0xf00) << 16);
  addroff |= (bus << 16) | (devfn << 8);
  return priv->cfg1_base + addroff;
}

static void __iomem *cfg0_map(struct loongson_pci *priv, int bus,
                              unsigned int devfn, int where,
                              int cfg_type1_offset) {
  unsigned long addroff = 0x0;

  if (bus != 0)
    addroff |= BIT(cfg_type1_offset); /* Type 1 Access */
  addroff |= (bus << 16) | (devfn << 8) | where;
  return priv->cfg0_base + addroff;
}

static void __iomem *pci_loongson_map_bus(struct pci_bus *bus,
                                          unsigned int devfn, int where) {
  unsigned char busnum = bus->number;
  struct loongson_pci *priv = pci_bus_to_loongson_pci(bus);
  int device = PCI_SLOT(devfn);
  int function = PCI_FUNC(devfn);

  /*
   * Do not read more than one device on the bus other than
   * the host bus. For our hardware the root bus is always bus 0.
   */
  if (priv->data->flags & FLAG_DEV_FIX && busnum != 0 && PCI_SLOT(devfn) > 0)
    return NULL;

  if ((busnum == 0) && (device >= 9 && device <= 20 && function == 1))
    return NULL;

  if (priv->cfg1_base && where < PCI_CFG_SPACE_EXP_SIZE) {
    return cfg1_map(priv, busnum, devfn, where, priv->data->cfg1_type1_offset);
  } else if (priv->cfg0_base && where < PCI_CFG_SPACE_SIZE) {
    abort();
    return cfg0_map(priv, busnum, devfn, where, priv->data->cfg0_type1_offset);
  }
  return NULL;
}

static int pci_loongson_config_read(struct pci_bus *bus, unsigned int devfn,
                                    int where, int size, u32 *val) {
  void __iomem *addr;

  addr = bus->ops->map_bus(bus, devfn, where);
  if (!addr) {
    *val = ~0;
    return PCIBIOS_DEVICE_NOT_FOUND;
  }

  if (size == 1)
    *val = readb(addr);
  else if (size == 2)
    *val = readw(addr);
  else
    *val = readl(addr);
  /*
   * fix some pcie card not scanning properly when bus number is
   * inconsistent during firmware and kernel scan phases.
   */
  if (*val == 0x0 && where == PCI_VENDOR_ID) {
    writel(*val, addr);
    *val = readl(addr);
  }

  return PCIBIOS_SUCCESSFUL;
}

static struct loongson_pci __loongson_pci;
static struct pcie_controller_data __pcie_controller_data;

static int loongson_pci_ecam_init(struct pci_config_window *cfg) {
  struct loongson_pci *priv = &__loongson_pci;
  struct pcie_controller_data *data = &__pcie_controller_data;

  data->flags = FLAG_CFG1 | FLAG_DEV_FIX;
  data->cfg1_type1_offset = 28;
  priv->data = data;
  priv->cfg1_base = cfg->win - (cfg->busr.start << 16);
  cfg->priv = priv;
  return 0;
}

struct pci_ecam_ops loongson_pci_ecam_ops = {
    .bus_shift = 16,
    .init = loongson_pci_ecam_init,
    .pci_ops = {
        .map_bus = pci_loongson_map_bus,
        .read = pci_loongson_config_read,
        .write = pci_generic_config_write,
    }};
