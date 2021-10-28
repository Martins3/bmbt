#ifndef SYSEMU_H_OILREKXF
#define SYSEMU_H_OILREKXF
#include "../hw/core/cpu.h"
#include "../qemu/compiler.h"
#include "../qemu/notify.h"
#include "../qemu/uuid.h"
#include "../types.h"

#include "cpus.h"

void qemu_add_machine_init_done_notifier(Notifier *notify);
void qemu_remove_machine_init_done_notifier(Notifier *notify);

static inline char *get_boot_devices_list(size_t *size) {
  // FIXME :linker: need more thinking about how to load guest image
  return NULL;
}

static inline char *get_boot_devices_lchs_list(size_t *size) { return NULL; }

extern int boot_menu;

extern QemuUUID qemu_uuid;

void qemu_init();

static inline void qemu_boot() {
  duck_check(first_cpu != NULL);
  qemu_tcg_rr_cpu_thread_fn(first_cpu);
}

#endif /* end of include guard: SYSEMU_H_OILREKXF */
