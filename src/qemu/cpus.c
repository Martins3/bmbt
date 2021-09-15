#include "../../include/qemu/main-loop.h"

// [interface 11]
static bool iothread_locked = false;

bool qemu_mutex_iothread_locked(void) { return iothread_locked; }

/*
 * The BQL is taken from so many places that it is worth profiling the
 * callers directly, instead of funneling them all through a single function.
 */
void qemu_mutex_lock_iothread_impl(const char *file, int line) {
  printf("locked %s:%d\n", file, line);
  g_assert(!qemu_mutex_iothread_locked());
  iothread_locked = true;
}

void qemu_mutex_unlock_iothread(void) {
  g_assert(qemu_mutex_iothread_locked());
  iothread_locked = false;
}
