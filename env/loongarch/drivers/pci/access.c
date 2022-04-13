#include <asm/cpu-features.h>
#include <assert.h>
#include <linux/pci-ecam.h>
#include <linux/pci.h>
#include <linux/type.h>
/*
 * Wrappers for all PCI configuration access functions.  They just check
 * alignment, do locking and call the low-level functions pointed to
 * by pci_dev->ops.
 */

#define PCI_byte_BAD 0
#define PCI_word_BAD (pos & 1)
#define PCI_dword_BAD (pos & 3)

static struct pci_bus __pci_bus;

#define CONFIG_PCI_LOCKLESS_CONFIG
#ifdef CONFIG_PCI_LOCKLESS_CONFIG
#define pci_lock_config(f)                                                     \
  do {                                                                         \
    (void)(f);                                                                 \
  } while (0)
#define pci_unlock_config(f)                                                   \
  do {                                                                         \
    (void)(f);                                                                 \
  } while (0)
#else
#define pci_lock_config(f) raw_spin_lock_irqsave(&pci_lock, f)
#define pci_unlock_config(f) raw_spin_unlock_irqrestore(&pci_lock, f)
#endif

// TMP_TODO 将 bdf & 0xff 的装换为 macro 吧
// TMP_TODO 2 似乎这个检查会触发错误，不知道为什么?
/* if (cpu_has_hypervisor)   */
/*   assert(bus->number == 0); */

#define PCI_OP_READ(size, type, len)                                           \
  int pci_bus_read_config_##size(u16 bdf, int pos, type *value) {              \
    int res;                                                                   \
    unsigned long flags;                                                       \
    struct pci_bus *bus = &__pci_bus;                                          \
    bus->number = bdf >> 8;                                                    \
    u32 data = 0;                                                              \
    if (PCI_##size##_BAD)                                                      \
      return PCIBIOS_BAD_REGISTER_NUMBER;                                      \
    pci_lock_config(flags);                                                    \
    res = bus->ops->read(bus, bdf & 0xff, pos, len, &data);                    \
    *value = (type)data;                                                       \
    pci_unlock_config(flags);                                                  \
    return res;                                                                \
  }

#define PCI_OP_WRITE(size, type, len)                                          \
  int pci_bus_write_config_##size(u16 bdf, int pos, type value) {              \
    int res;                                                                   \
    unsigned long flags;                                                       \
    struct pci_bus *bus = &__pci_bus;                                          \
    bus->number = bdf >> 8;                                                    \
    if (PCI_##size##_BAD)                                                      \
      return PCIBIOS_BAD_REGISTER_NUMBER;                                      \
    pci_lock_config(flags);                                                    \
    res = bus->ops->write(bus, bdf & 0xff, pos, len, value);                   \
    pci_unlock_config(flags);                                                  \
    return res;                                                                \
  }

PCI_OP_READ(byte, u8, 1)
PCI_OP_READ(word, u16, 2)
PCI_OP_READ(dword, u32, 4)
PCI_OP_WRITE(byte, u8, 1)
PCI_OP_WRITE(word, u16, 2)
PCI_OP_WRITE(dword, u32, 4)

// [interface 61]
void init_pci_bus(struct pci_config_window *cfg) {
  struct pci_bus *bus = &__pci_bus;
  bus->ops = &cfg->ops->pci_ops;
  bus->sysdata = cfg;
}
