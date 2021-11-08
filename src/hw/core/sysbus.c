#include <hw/sysbus.h>
#include "../../glib/gtestutils.h"
static void sysbus_mmio_map_common(SysBusDevice *dev, int n, hwaddr addr,
                                   bool may_overlap, int priority) {
  assert(n >= 0 && n < dev->num_mmio);

  if (dev->mmio[n].addr == addr) {
    /* ??? region already mapped here.  */
    g_assert_not_reached();
  }
  if (dev->mmio[n].addr != (hwaddr)-1) {
    /* Unregister previous mapping.  */
    g_assert_not_reached();
  }
  dev->mmio[n].addr = addr;
  if (may_overlap) {
    g_assert_not_reached();
  } else {
#ifdef BMBT
    memory_region_add_subregion(get_system_memory(), addr, dev->mmio[n].memory);
#else
    mmio_add_memory_region(addr, dev->mmio[n].memory);
#endif
  }
}

void sysbus_mmio_map(SysBusDevice *dev, int n, hwaddr addr) {
  sysbus_mmio_map_common(dev, n, addr, false, 0);
}

void sysbus_init_mmio(SysBusDevice *dev, MemoryRegion *memory) {
  int n;

  assert(dev->num_mmio < QDEV_MAX_MMIO);
  n = dev->num_mmio++;
  dev->mmio[n].addr = -1;
  dev->mmio[n].memory = memory;
}
