#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <assert.h>
#include <env-timer.h>
#include <hw/core/cpu.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unitest/greatest.h>

static TimerHandler uefi_timer_handler;
static void timer_handler(int _unused) {
  enter_interrpt_context();
  uefi_timer_handler();
  leave_interrpt_context();
}

STATIC EFI_EVENT SignalTimer = NULL;

STATIC
VOID EFIAPI signalTimer(IN EFI_EVENT Event, IN VOID *Context) {
  raise(SIGALRM);
}

void setup_timer(TimerHandler handler) {
  uefi_timer_handler = handler;
  if (signal(SIGALRM, timer_handler) == SIG_ERR) {
    perror("Unable to catch SIGALRM");
    exit(1);
  }
  assert(EfiGetCurrentTpl() == TPL_APPLICATION);

  EFI_STATUS Status;
  EFI_EVENT *EventPointer = &SignalTimer;
  Status = gBS->CreateEvent(EVT_NOTIFY_SIGNAL, TPL_CALLBACK, signalTimer,
                            NULL, // no context
                            EventPointer);

  if (EFI_ERROR(Status)) {
    perror("Unable to create signal event for fire_timer");
    exit(1);
  }
}

#define NANO_PER_SECOND 1000000000LL
#define NANO_PER_MICRO 1000LL

void soonest_interrupt_ns(long ns) {
  struct itimerval it_val; /* for setting itimer */

  it_val.it_value.tv_sec = ns / NANO_PER_SECOND;
  // https://stackoverflow.com/questions/2745074/fast-ceiling-of-an-integer-division-in-c-c
  it_val.it_value.tv_usec =
      (ns % NANO_PER_SECOND + NANO_PER_MICRO - 1) / NANO_PER_MICRO;
  it_val.it_interval.tv_sec = 0;
  it_val.it_interval.tv_usec = 0;

  EFI_TPL Tpl = EfiGetCurrentTpl();
  if (Tpl == TPL_CALLBACK) {
    assert(is_interrupt_blocked() || !qemu_cpu_is_self(NULL));
  } else {
    Tpl = gBS->RaiseTPL(TPL_CALLBACK);
    assert(Tpl == TPL_APPLICATION);
    if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
      perror("error calling setitimer()");
      exit(1);
    }
    gBS->RestoreTPL(Tpl);
  }
}

static inline void cancel_timer() { soonest_interrupt_ns(0); }

static bool __blocked = false;

bool is_interrupt_blocked(void) { return __blocked; }

void block_interrupt() {
  assert(__blocked == false);
  assert(qemu_cpu_is_self(NULL));

  EFI_TPL Tpl = gBS->RaiseTPL(TPL_CALLBACK);
  assert(Tpl == TPL_APPLICATION);
  __blocked = true;
}

void unblock_interrupt() {
  assert(__blocked == true);
  assert(qemu_cpu_is_self(NULL));
  __blocked = false;

  EFI_TPL Tpl = EfiGetCurrentTpl();
  assert(Tpl == TPL_CALLBACK);
  gBS->RestoreTPL(TPL_APPLICATION);
}

void fire_timer() {
  cancel_timer();
  gBS->SignalEvent(SignalTimer);
}
