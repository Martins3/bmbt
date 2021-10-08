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

typedef struct {
  int thread;
} QemuThread;

static inline void qemu_thread_get_self(QemuThread *thread) {
  thread->thread = 0x1234;
}

static inline int qemu_get_thread_id(void) {
  // return syscall(SYS_gettid);
  return 0x4567;
}

typedef struct {
} QemuCond;

/*
 * IMPORTANT: The implementation does not guarantee that pthread_cond_signal
 * and pthread_cond_broadcast can be called except while the same mutex is
 * held as in the corresponding pthread_cond_wait calls!
 */
static inline void qemu_cond_signal(QemuCond *cond) {}
static inline void qemu_cond_broadcast(QemuCond *cond) {}

#define QEMU_LOCK_GUARD(x)                                                     \
  {}

#endif /* end of include guard: THREAD_H_P9X05SBE */
