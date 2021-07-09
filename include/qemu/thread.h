#ifndef THREAD_H_P9X05SBE
#define THREAD_H_P9X05SBE

#include "thread-posix.h"

typedef struct QemuSpin {
  int value;
} QemuSpin;

static inline void qemu_spin_init(QemuSpin *spin) {}

static inline void qemu_spin_lock(QemuSpin *spin) {}

static inline void qemu_spin_unlock(QemuSpin *spin) {}

static inline bool qemu_spin_trylock(QemuSpin *spin) {return true;}

void qemu_mutex_init(QemuMutex *mutex){}

#endif /* end of include guard: THREAD_H_P9X05SBE */
