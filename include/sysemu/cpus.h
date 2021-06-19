#ifndef CPUS_H_XUGZP1FI
#define CPUS_H_XUGZP1FI

extern int use_icount;

// FIXME iothread lock need further investigation
void qemu_mutex_lock_iothread();
void qemu_mutex_unlock_iothread();

#endif /* end of include guard: CPUS_H_XUGZP1FI */
