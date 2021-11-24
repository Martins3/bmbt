#ifndef MAIN_LOOP_H_BMOIUZGQ
#define MAIN_LOOP_H_BMOIUZGQ

#include "../hw/core/cpu.h"
#include <stdbool.h>

extern int use_icount;

// `IOReadHandler`, `IOCanReadHandler` is used by serial
#ifdef _WIN32
/* return TRUE if no sleep should be done afterwards */
typedef int PollingFunc(void *opaque);

/**
 * qemu_add_polling_cb: Register a Windows-specific polling callback
 *
 * Currently, under Windows some events are polled rather than waited for.
 * Polling callbacks do not ensure that @func is called timely, because
 * the main loop might wait for an arbitrarily long time.  If possible,
 * you should instead create a separate thread that does a blocking poll
 * and set a Win32 event object.  The event can then be passed to
 * qemu_add_wait_object.
 *
 * Polling callbacks really have nothing Windows specific in them, but
 * as they are a hack and are currently not necessary under POSIX systems,
 * they are only available when QEMU is running under Windows.
 *
 * @func: The function that does the polling, and returns 1 to force
 * immediate completion of main_loop_wait.
 * @opaque: A pointer-size value that is passed to @func.
 */
int qemu_add_polling_cb(PollingFunc *func, void *opaque);

/**
 * qemu_del_polling_cb: Unregister a Windows-specific polling callback
 *
 * This function removes a callback that was registered with
 * qemu_add_polling_cb.
 *
 * @func: The function that was passed to qemu_add_polling_cb.
 * @opaque: A pointer-size value that was passed to qemu_add_polling_cb.
 */
void qemu_del_polling_cb(PollingFunc *func, void *opaque);

/* Wait objects handling */
typedef void WaitObjectFunc(void *opaque);

/**
 * qemu_add_wait_object: Register a callback for a Windows handle
 *
 * Under Windows, the iohandler mechanism can only be used with sockets.
 * QEMU must use the WaitForMultipleObjects API to wait on other handles.
 * This function registers a #HANDLE with QEMU, so that it will be included
 * in the main loop's calls to WaitForMultipleObjects.  When the handle
 * is in a signaled state, QEMU will call @func.
 *
 * @handle: The Windows handle to be observed.
 * @func: A function to be called when @handle is in a signaled state.
 * @opaque: A pointer-size value that is passed to @func.
 */
int qemu_add_wait_object(HANDLE handle, WaitObjectFunc *func, void *opaque);

/**
 * qemu_del_wait_object: Unregister a callback for a Windows handle
 *
 * This function removes a callback that was registered with
 * qemu_add_wait_object.
 *
 * @func: The function that was passed to qemu_add_wait_object.
 * @opaque: A pointer-size value that was passed to qemu_add_wait_object.
 */
void qemu_del_wait_object(HANDLE handle, WaitObjectFunc *func, void *opaque);
#endif

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
