#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <host-timer.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unitest/greatest.h>

#define INTERVAL 1000 /* number of milliseconds to go off */

/*
 * DoStuff
 */
void DoStuff(int x) { printf("huxueshi:%s \n", __FUNCTION__); }

int main(int argc, char *argv[]) {
  struct itimerval it_val; /* for setting itimer */

  /* Upon SIGALRM, call DoStuff().
   * Set interval timer.  We want frequency in ms,
   * but the setitimer call needs seconds and useconds. */
  if (signal(SIGALRM, (void (*)(int))DoStuff) == SIG_ERR) {
    perror("Unable to catch SIGALRM");
    exit(1);
  }

  it_val.it_value.tv_sec = INTERVAL / 1000;
  it_val.it_value.tv_usec = (INTERVAL * 1000) % 1000000;
  it_val.it_interval.tv_sec = 0;
  it_val.it_interval.tv_usec = 0;

  printf("huxueshi:%s %d\n", __FUNCTION__, EfiGetCurrentTpl());

  EFI_TPL Tpl;
  Tpl = gBS->RaiseTPL(TPL_CALLBACK);
  if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
    perror("error calling setitimer()");
    exit(1);
  }
  gBS->RestoreTPL(Tpl);
  printf("huxueshi:%s \n", __FUNCTION__);

  while (1) {
    asm("hlt");
  }
  return 0;
}
