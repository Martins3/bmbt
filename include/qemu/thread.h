#ifndef THREAD_H_P9X05SBE
#define THREAD_H_P9X05SBE

#include <assert.h>
#include <stdbool.h>

// [interface 12]
typedef struct QemuSpin {
  bool lock;
} QemuSpin;

typedef struct QemuMutex {
  bool lock;
} QemuMutex;

static inline void qemu_spin_init(QemuSpin *spin) { spin->lock = false; }

static inline void qemu_spin_lock(QemuSpin *spin) {
  assert(spin->lock == false);
  spin->lock = true;
}

static inline void qemu_spin_unlock(QemuSpin *spin) {
  assert(spin->lock == true);
  spin->lock = false;
}

static inline bool qemu_spin_trylock(QemuSpin *spin) {
  qemu_spin_init(spin);
  return true;
}

static inline void qemu_mutex_init(QemuMutex *mutex) { mutex->lock = false; }

static inline void qemu_mutex_lock(QemuMutex *mutex) {
  assert(mutex->lock == false);
  mutex->lock = true;
}

static inline void qemu_mutex_unlock(QemuMutex *mutex) {
  assert(mutex->lock == true);
  mutex->lock = false;
}

#endif /* end of include guard: THREAD_H_P9X05SBE */
