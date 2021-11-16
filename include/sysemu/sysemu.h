#ifndef SYSEMU_H_OILREKXF
#define SYSEMU_H_OILREKXF
#include "../hw/core/cpu.h"
#include "../qemu/compiler.h"
#include "../qemu/notify.h"
#include "../qemu/uuid.h"
#include "../types.h"
#include <qemu/timer.h>

#include "cpus.h"
#include <string.h>

void qemu_add_machine_init_done_notifier(Notifier *notify);
void qemu_remove_machine_init_done_notifier(Notifier *notify);

static inline char *get_boot_devices_list(size_t *size) {
  const char *linux_dma = "/rom@genroms/linuxboot_dma.bin";
  *size = strlen(linux_dma) + 1;
  char *boot_list = g_malloc0(*size);
  strcpy(boot_list, linux_dma);
  duck_check(*size == 0x1f);
  return boot_list;
}

static inline char *get_boot_devices_lchs_list(size_t *size) {
  *size = 0;
  return NULL;
}

extern int boot_menu;

extern QemuUUID qemu_uuid;
extern QEMUClockType rtc_clock;

void qemu_init();

static inline void qemu_boot() {
  duck_check(first_cpu != NULL);
  qemu_tcg_rr_cpu_thread_fn(first_cpu);
}

#endif /* end of include guard: SYSEMU_H_OILREKXF */
