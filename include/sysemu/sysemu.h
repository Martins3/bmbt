#ifndef SYSEMU_H_OILREKXF
#define SYSEMU_H_OILREKXF
#include "../qemu/notify.h"
#include "../qemu/uuid.h"
#include "../types.h"

void qemu_add_machine_init_done_notifier(Notifier *notify);
void qemu_remove_machine_init_done_notifier(Notifier *notify);

static inline char *get_boot_devices_list(size_t *size) {
  // FIXME :linker: need more thinking about how to load guest image
  return NULL;
}

static inline char *get_boot_devices_lchs_list(size_t *size) { return NULL; }

extern int boot_menu;

extern QemuUUID qemu_uuid;

#endif /* end of include guard: SYSEMU_H_OILREKXF */
