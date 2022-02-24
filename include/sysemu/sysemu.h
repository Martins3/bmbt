#ifndef SYSEMU_H_OILREKXF
#define SYSEMU_H_OILREKXF
#include "../hw/core/cpu.h"
#include "../qemu/compiler.h"
#include "../qemu/notify.h"
#include "../qemu/uuid.h"
#include <chardev/char.h>
#include <hw/qdev-core.h>
#include <qemu/timer.h>

#include "cpus.h"
#include <string.h>

void qemu_add_machine_init_done_notifier(Notifier *notify);
void qemu_remove_machine_init_done_notifier(Notifier *notify);

static inline char *get_boot_devices_lchs_list(size_t *size) {
  *size = 0;
  return NULL;
}

extern int boot_menu;

extern QemuUUID qemu_uuid;
extern QEMUClockType rtc_clock;

#define MAX_OPTION_ROMS 16
typedef struct QEMUOptionRom {
  const char *name;
  int32_t bootindex;
} QEMUOptionRom;
extern QEMUOptionRom option_rom[MAX_OPTION_ROMS];
extern int nb_option_roms;

void add_boot_device_path(int32_t bootindex, DeviceState *dev,
                          const char *suffix);
char *get_boot_devices_list(size_t *size);
extern bool boot_strict;

void qemu_init();

static inline void qemu_boot() {
  bmbt_check(first_cpu != NULL);
  qemu_tcg_rr_cpu_thread_fn(first_cpu);
}

/* Return the Chardev for serial port i, or NULL if none */
Chardev *serial_hd(int i);

#endif /* end of include guard: SYSEMU_H_OILREKXF */
