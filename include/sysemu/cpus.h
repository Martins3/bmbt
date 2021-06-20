#ifndef CPUS_H_XUGZP1FI
#define CPUS_H_XUGZP1FI

// FIXME cpus.h will be redesinged
extern int use_icount;

void qemu_mutex_lock_iothread();
void qemu_mutex_unlock_iothread();

/**
 * qemu_mutex_iothread_locked: Return lock status of the main loop mutex.
 *
 * The main loop mutex is the coarsest lock in QEMU, and as such it
 * must always be taken outside other locks.  This function helps
 * functions take different paths depending on whether the current
 * thread is running within the main loop mutex.
 */
bool qemu_mutex_iothread_locked(void);


#endif /* end of include guard: CPUS_H_XUGZP1FI */
