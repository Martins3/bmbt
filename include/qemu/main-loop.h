#ifndef MAIN_LOOP_H_BMOIUZGQ
#define MAIN_LOOP_H_BMOIUZGQ

#include "../hw/core/cpu.h"
#include <stdbool.h>

extern int use_icount;

/**
 * qemu_mutex_lock_iothread: Lock the main loop mutex.
 *
 * This function locks the main loop mutex.  The mutex is taken by
 * main() in vl.c and always taken except while waiting on
 * external events (such as with select).  The mutex should be taken
 * by threads other than the main loop thread when calling
 * qemu_bh_new(), qemu_set_fd_handler() and basically all other
 * functions documented in this file.
 *
 * NOTE: tools currently are single-threaded and qemu_mutex_lock_iothread
 * is a no-op there.
 */
#define qemu_mutex_lock_iothread()                                             \
  qemu_mutex_lock_iothread_impl(__FILE__, __LINE__)

void qemu_mutex_lock_iothread_impl(const char *file, int line);

/**
 * qemu_mutex_iothread_locked: Return lock status of the main loop mutex.
 *
 * The main loop mutex is the coarsest lock in QEMU, and as such it
 * must always be taken outside other locks.  This function helps
 * functions take different paths depending on whether the current
 * thread is running within the main loop mutex.
 */
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
#endif /* end of include guard: MAIN_LOOP_H_BMOIUZGQ */
