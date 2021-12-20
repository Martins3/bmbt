#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <host-timer.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unitest/greatest.h>

timer_id setup_timer(TimerHandler handler) {
  if (signal(SIGALRM, handler) == SIG_ERR) {
    perror("Unable to catch SIGALRM");
    exit(1);
  }

  return NULL;
}

#define NANOSECONDS_PER_SECOND 1000000000LL
#define MICROSECONDS_PER_SECOND 1000000LL

void soonest_interrupt_ns(long ns) {
  struct itimerval it_val; /* for setting itimer */

  it_val.it_value.tv_sec = ns / NANOSECONDS_PER_SECOND;
  // https://stackoverflow.com/questions/2745074/fast-ceiling-of-an-integer-division-in-c-c
  it_val.it_value.tv_usec =
      (ns % NANOSECONDS_PER_SECOND + MICROSECONDS_PER_SECOND - 1) %
      MICROSECONDS_PER_SECOND;
  it_val.it_interval.tv_sec = 0;
  it_val.it_interval.tv_usec = 0;

  EFI_TPL Tpl;
  Tpl = gBS->RaiseTPL(TPL_CALLBACK);
  if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
    perror("error calling setitimer()");
    exit(1);
  }
  gBS->RestoreTPL(Tpl);
}
