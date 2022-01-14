#include "internal.h"
#include <stdbool.h>
#include <stdlib.h> // NULL
#include <time.h>

long kernel_clock_gettime(long arg0, long arg1, long arg2, long arg3, long arg4,
                          long arg5, long arg6) {
  static int counter = 0;
  clockid_t clk_id = arg0;
  duck_assert(clk_id == CLOCK_REALTIME);
  struct timespec *res = (struct timespec *)arg1;
  res->tv_nsec = 0;
  res->tv_sec = counter ++;
  return 0;
}
