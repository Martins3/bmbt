#ifndef THREAD_H_P9X05SBE
#define THREAD_H_P9X05SBE

struct QemuSpin {
  int value;
};

static inline void qemu_spin_init(QemuSpin *spin) {}

static inline void qemu_spin_lock(QemuSpin *spin){}

static inline void qemu_spin_unlock(QemuSpin *spin){}

#endif /* end of include guard: THREAD_H_P9X05SBE */
