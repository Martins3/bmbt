#include "atomic.h"
#include "pthread_impl.h"
#include "syscall.h"
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>

struct ksigevent {
  union sigval sigev_value;
  int sigev_signo;
  int sigev_notify;
  int sigev_tid;
};

int timer_create(clockid_t clk, struct sigevent *restrict evp,
                 timer_t *restrict res) {
  struct ksigevent ksev, *ksevp = 0;
  int timerid;

  switch (evp ? evp->sigev_notify : SIGEV_SIGNAL) {
  case SIGEV_NONE:
  case SIGEV_SIGNAL:
  case SIGEV_THREAD_ID:
    if (evp) {
      ksev.sigev_value = evp->sigev_value;
      ksev.sigev_signo = evp->sigev_signo;
      ksev.sigev_notify = evp->sigev_notify;
      if (evp->sigev_notify == SIGEV_THREAD_ID)
        ksev.sigev_tid = evp->sigev_notify_thread_id;
      else
        ksev.sigev_tid = 0;
      ksevp = &ksev;
    }
    if (syscall(SYS_timer_create, clk, ksevp, &timerid) < 0)
      return -1;
    *res = (void *)(intptr_t)timerid;
    break;
  case SIGEV_THREAD:
    // bmbt : unused
    assert(false);
    break;
  default:
    errno = EINVAL;
    return -1;
  }

  return 0;
}
