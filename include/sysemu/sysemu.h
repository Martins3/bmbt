#ifndef SYSEMU_H_OILREKXF
#define SYSEMU_H_OILREKXF
#include "../qemu/notify.h"
#include "../types.h"

void qemu_add_machine_init_done_notifier(Notifier *notify);
void qemu_remove_machine_init_done_notifier(Notifier *notify);

// @todo boot devices
// void add_boot_device_path(int32_t bootindex, DeviceState *dev, const char
// *suffix);
char *get_boot_devices_list(size_t *size);

char *get_boot_devices_lchs_list(size_t *size);

#endif /* end of include guard: SYSEMU_H_OILREKXF */
