#include <setjmp.h>
#include <stdio.h>
#include <time.h>

_Noreturn void siglongjmp(sigjmp_buf buf, int val) {
  for (;;) {
  }
}

int sigsetjmp(sigjmp_buf buf, int val) { return 0; }

int timer_settime(timer_t a, int b, const struct itimerspec *__restrict c,
                  struct itimerspec *__restrict d) {
  return 9;
}

int timer_create(clockid_t a, struct sigevent *__restrict b,
                 timer_t *__restrict c) {
  return 0;
}
