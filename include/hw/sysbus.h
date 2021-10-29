#ifndef SYSBUS_H_PYCYMQIH
#define SYSBUS_H_PYCYMQIH

#include <exec/memory.h>

#define QDEV_MAX_MMIO 32
#define QDEV_MAX_PIO 32

typedef struct SysBusDevice {
#ifdef BMBT
  /*< private >*/
  DeviceState parent_obj;
#endif
  /*< public >*/

  int num_mmio;
  struct {
    hwaddr addr;
    MemoryRegion *memory;
  } mmio[QDEV_MAX_MMIO];
  int num_pio;
  uint32_t pio[QDEV_MAX_PIO];
} SysBusDevice;

void sysbus_mmio_map(SysBusDevice *dev, int n, hwaddr addr);
void sysbus_init_mmio(SysBusDevice *dev, MemoryRegion *memory);

#endif /* end of include guard: SYSBUS_H_PYCYMQIH */
