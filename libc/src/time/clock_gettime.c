#include "atomic.h"
#include "syscall.h"
#include <errno.h>
#include <stdint.h>
#include <time.h>

int __clock_gettime(clockid_t clk, struct timespec *ts) {
  int r;

#ifdef VDSO_CGT_SYM
  int (*f)(clockid_t, struct timespec *) =
      (int (*)(clockid_t, struct timespec *))vdso_func;
  if (f) {
    r = f(clk, ts);
    if (!r)
      return r;
    if (r == -EINVAL)
      return libc_syscall_ret(r);
    /* Fall through on errors other than EINVAL. Some buggy
     * vdso implementations return ENOSYS for clocks they
     * can't handle, rather than making the syscall. This
     * also handles the case where cgt_init fails to find
     * a vdso function to use. */
  }
#endif

#ifdef SYS_clock_gettime64
  r = -ENOSYS;
  if (sizeof(time_t) > 4)
    r = libc_syscall(SYS_clock_gettime64, clk, ts);
  if (SYS_clock_gettime == SYS_clock_gettime64 || r != -ENOSYS)
    return libc_syscall_ret(r);
  long ts32[2];
  r = libc_syscall(SYS_clock_gettime, clk, ts32);
  if (r == -ENOSYS && clk == CLOCK_REALTIME) {
    r = libc_syscall(SYS_gettimeofday, ts32, 0);
    ts32[1] *= 1000;
  }
  if (!r) {
    ts->tv_sec = ts32[0];
    ts->tv_nsec = ts32[1];
    return r;
  }
  return libc_syscall_ret(r);
#else
  r = libc_syscall(SYS_clock_gettime, clk, ts);
  if (r == -ENOSYS) {
    if (clk == CLOCK_REALTIME) {
      libc_syscall(SYS_gettimeofday, ts, 0);
      ts->tv_nsec = (int)ts->tv_nsec * 1000;
      return 0;
    }
    r = -EINVAL;
  }
  return libc_syscall_ret(r);
#endif
}

weak_alias(__clock_gettime, clock_gettime);
