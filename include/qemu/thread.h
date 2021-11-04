#ifndef THREAD_H_P9X05SBE
#define THREAD_H_P9X05SBE

#include "../../include/qemu/compiler.h"
#include <assert.h>
#include <stdbool.h>

// [interface 12]
typedef struct QemuSpin {
  bool lock;
} QemuSpin;

typedef struct QemuMutex {
  bool lock;
  bool initialized;
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

static inline void qemu_mutex_init(QemuMutex *mutex) {
  mutex->initialized = true;
  mutex->lock = false;
}

static inline void qemu_mutex_lock(QemuMutex *mutex) {
  assert(mutex->initialized);
  assert(mutex->lock == false);
  mutex->lock = true;
}

static inline void qemu_mutex_lock__raw(QemuMutex *mutex) {
  qemu_mutex_lock(mutex);
}

static inline bool qemu_mutex_trylock(QemuMutex *mutex) {
  qemu_mutex_lock(mutex);
  return false;
}

static inline bool qemu_mutex_trylock__raw(QemuMutex *mutex) {
  qemu_mutex_trylock(mutex);
  return true;
}

static inline void qemu_mutex_unlock(QemuMutex *mutex) {
  assert(mutex->lock == true);
  mutex->lock = false;
}

static inline bool qemu_mutex_locked(QemuMutex *mutex) { return mutex->lock; }

typedef struct QemuThread {
  int thread;
} QemuThread;

static inline void qemu_thread_get_self(QemuThread *thread) {
  thread->thread = 0x1234;
}

// CPUState::thread_id is used by machine-qmp-cmds.c
#ifdef BMBT
static inline int qemu_get_thread_id(void) { return syscall(SYS_gettid); }
#endif

// In BMBT mode,  signal or broadcast firstly, then wait on it
typedef struct QemuCond {
  bool cond_ready;
  bool initiated;
} QemuCond;

static inline void qemu_cond_init(QemuCond *cond) {
  duck_check(!cond->initiated);
  cond->cond_ready = false;
  cond->initiated = true;
}
/*
 * IMPORTANT: The implementation does not guarantee that pthread_cond_signal
 * and pthread_cond_broadcast can be called except while the same mutex is
 * held as in the corresponding pthread_cond_wait calls!
 */
static inline void qemu_cond_signal(QemuCond *cond) {
  duck_check(!cond->cond_ready);
  cond->cond_ready = true;
}

static inline void qemu_cond_broadcast(QemuCond *cond) {
  duck_check(!cond->cond_ready);
  cond->cond_ready = true;
}

static inline void qemu_cond_wait(QemuCond *cond, QemuMutex *mutex) {
  duck_check(cond->cond_ready);
  duck_check(!qemu_mutex_locked(mutex));
  cond->cond_ready = false;
}

#define QEMU_LOCK_GUARD(x)                                                     \
  {                                                                            \
    qemu_mutex_lock(x);                                                        \
    qemu_mutex_unlock(x);                                                      \
  }

#endif /* end of include guard: THREAD_H_P9X05SBE */
