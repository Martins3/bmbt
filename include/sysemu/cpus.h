#ifndef CPUS_H_XUGZP1FI
#define CPUS_H_XUGZP1FI

#include "../hw/core/cpu.h"
#include <stdbool.h>

// FIXME cpus.h will be redesinged
extern int use_icount;

static inline void qemu_mutex_lock_iothread() {
  // FIXME
}
static inline void qemu_mutex_unlock_iothread() {
  // FIXME
}

/**
 * qemu_mutex_iothread_locked: Return lock status of the main loop mutex.
 *
 * The main loop mutex is the coarsest lock in QEMU, and as such it
 * must always be taken outside other locks.  This function helps
 * functions take different paths depending on whether the current
 * thread is running within the main loop mutex.
 */
bool qemu_mutex_iothread_locked(void) {
  // FIXME
  return false;
}
bool qemu_cpu_is_self(CPUState *cpu) {
  // FIXME
  return false;
}

#endif /* end of include guard: CPUS_H_XUGZP1FI */
