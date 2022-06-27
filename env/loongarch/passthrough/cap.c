/* copied from linux/drivers/pci/pci.c */
#include "internal.h"
#include <linux/pci.h>

static int __pci_find_next_cap_ttl(u16 bdf, u8 pos, int cap, int *ttl) {
  u8 id;
  u16 ent;

  pci_bus_read_config_byte(bdf, pos, &pos);

  while ((*ttl)--) {
    if (pos < 0x40)
      break;
    pos &= ~3;
    pci_bus_read_config_word(bdf, pos, &ent);

    id = ent & 0xff;
    if (id == 0xff)
      break;
    if (id == cap)
      return pos;
    pos = (ent >> 8);
  }
  return 0;
}

static int __pci_find_next_cap(u16 bdf, u8 pos, int cap) {
  int ttl = PCI_FIND_CAP_TTL;

  return __pci_find_next_cap_ttl(bdf, pos, cap, &ttl);
}

static int __pci_bus_find_cap_start(u16 bdf, u8 hdr_type) {
  u16 status;

  pci_bus_read_config_word(bdf, PCI_STATUS, &status);
  if (!(status & PCI_STATUS_CAP_LIST))
    return 0;

  switch (hdr_type) {
  case PCI_HEADER_TYPE_NORMAL:
  case PCI_HEADER_TYPE_BRIDGE:
    return PCI_CAPABILITY_LIST;
  case PCI_HEADER_TYPE_CARDBUS:
    return PCI_CB_CAPABILITY_LIST;
  }

  return 0;
}

int pci_find_capability(u16 bdf, int cap) {
  int pos;
  u8 hdr_type;

  pci_bus_read_config_byte(bdf, PCI_HEADER_TYPE, &hdr_type);
  pos = __pci_bus_find_cap_start(bdf, hdr_type);
  if (pos)
    pos = __pci_find_next_cap(bdf, pos, cap);

  return pos;
}
