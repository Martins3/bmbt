#ifndef SYSEMU_H_OILREKXF
#define SYSEMU_H_OILREKXF
#include "../types.h"

// @todo boot devices
// void add_boot_device_path(int32_t bootindex, DeviceState *dev, const char
// *suffix);
char *get_boot_devices_list(size_t *size);

char *get_boot_devices_lchs_list(size_t *size);

#endif /* end of include guard: SYSEMU_H_OILREKXF */
