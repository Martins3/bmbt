#include "syscall.h"
#include <time.h>

int nanosleep(const struct timespec *req, struct timespec *rem) {
  return libc_syscall_ret(-__clock_nanosleep(CLOCK_REALTIME, 0, req, rem));
}
