#include <hw/pci-host/pam.h>

void init_pam(PAMMemoryRegion *pam_mr, MemoryRegion *mr) {
  pam_mr->current = PAM_PCI;
  pam_mr->mr = mr;
}

/*
 * 0000000000000000-ffffffffffffffff (prio -1, i/o): pci
 *   a0000-bffff (prio 1, i/o): vga-lowmem
 *   c0000-dffff (prio 1, rom): pc.rom
 *   e0000-fffff (prio 1, i/o): alias isa-bios @pc.bios 20000-3ffff
 */
void set_pam_memory_region(PAMMemoryRegion *p, unsigned type) {
  switch (type) {
  case PAM_RAM:
    p->mr->readonly = false;
    break;
  case PAM_ROM:
    p->mr->readonly = true;
    break;
  case PAM_PCI:
    break;
  default:
    g_assert_not_reached();
  }

  p->current = type;
}

void pam_update(PAMMemoryRegion *pam, int idx, uint8_t val) {
  assert(0 <= idx && idx <= 12);
  unsigned prev = pam->current;
  pam->current = (val >> ((!(idx & 1)) * 4)) & PAM_ATTR_MASK;
  set_pam_memory_region(pam, pam->current);

  printf("huxueshi:%s pam=%d (%u -> %u)\n", __FUNCTION__, idx, prev,
         pam->current);
}
