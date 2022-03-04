#include "internal.h"
#include <asm/loongarchregs.h>
#include <qemu/timer.h>
#include <stdbool.h>
#include <stdlib.h> // NULL
#include <time.h>

extern const int NS_PER_CYCLE;
long kernel_clock_gettime(long arg0, long arg1, long arg2, long arg3, long arg4,
                          long arg5, long arg6) {
  clockid_t clk_id = arg0;
  kern_assert(clk_id == CLOCK_REALTIME);
  struct timespec *res = (struct timespec *)arg1;
  unsigned long t = drdtime() * NS_PER_CYCLE;
  // [interface 57]
  res->tv_nsec = t % NANOSECONDS_PER_SECOND;
  res->tv_sec = t / NANOSECONDS_PER_SECOND;
  return 0;
}
