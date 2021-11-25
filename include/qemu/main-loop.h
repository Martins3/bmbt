#ifndef MAIN_LOOP_H_BMOIUZGQ
#define MAIN_LOOP_H_BMOIUZGQ

#include "../hw/core/cpu.h"
#include <stdbool.h>

extern int use_icount;

// `IOReadHandler`, `IOCanReadHandler` is used by serial
/* async I/O support */

typedef void IOReadHandler(void *opaque, const uint8_t *buf, int size);

/**
 * IOCanReadHandler: Return the number of bytes that #IOReadHandler can accept
 *
 * This function reports how many bytes #IOReadHandler is prepared to accept.
 * #IOReadHandler may be invoked with up to this number of bytes.  If this
 * function returns 0 then #IOReadHandler is not invoked.
 *
 * This function is typically called from an event loop.  If the number of
 * bytes changes outside the event loop (e.g. because a vcpu thread drained the
 * buffer), then it is necessary to kick the event loop so that this function
 * is called again.  aio_notify() or qemu_notify_event() can be used to kick
 * the event loop.
 */
typedef int IOCanReadHandler(void *opaque);

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

// used to initialize xtm optiosn
void init_xtm_options();

void setup_timer_interrupt();
#endif /* end of include guard: MAIN_LOOP_H_BMOIUZGQ */
