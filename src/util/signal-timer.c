#include <assert.h>
#include <errno.h>
#include <hw/core/cpu.h>
#include <qemu/atomic.h>
#include <qemu/error-report.h>
#include <qemu/timer.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define TIMER_SIG SIGRTMAX /* Our timer notification signal */

timer_t setup_timer(TimerHandler handler) {
  struct sigaction sa;
  struct sigevent sev;
  static timer_t tid;

  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = handler;
  sigemptyset(&sa.sa_mask);
  if (sigaction(TIMER_SIG, &sa, NULL) == -1) {
    error_report("set sigaction failed\n");
  }
  sev.sigev_value.sival_ptr = &tid;
  sev.sigev_notify = SIGEV_SIGNAL; /* Notify via signal */
  sev.sigev_signo = TIMER_SIG;     /* Notify using this signal */

  if (timer_create(CLOCK_REALTIME, &sev, &tid) == -1) {
    error_report("timer_create failed\n");
  }
  return tid;
}

static void soonest_timer(timer_t tid, long s, long ns) {
  struct itimerspec ts;
  ts.it_value.tv_sec = s;
  ts.it_value.tv_nsec = ns;
  ts.it_interval.tv_sec = 0;
  ts.it_interval.tv_nsec = 0;

  // return EINVAL if new_value.it_value is negative; or
  // new_value.it_value.tv_nsec is negative or greater than 999,999,999.
  //   -- quoted from timer_settime(2)
  assert(s >= 0);
  assert(ns >= 0);
  assert(ns < 1000 * 1000 * 1000);

  // To disarm a timer, we make a call to timer_settime() specifying both fields
  // of value.it_value as 0.
  //  -- quoted from tlpi 23.6.2
  assert(s + ns >= 0);

  if (timer_settime(tid, 0, &ts, NULL) == -1) {
    error_report("timer_settime failed\n");
  }
}

void soonest_timer_s(timer_t tid, long s) { soonest_timer(tid, s, 0); }

void soonest_timer_ns(timer_t tid, long ns) { soonest_timer(tid, 0, ns); }

static bool __blocked = false;

bool is_interrupt_blocked(void) { return __blocked; }

void block_interrupt() {
  assert(__blocked == false);
  assert(qemu_cpu_is_self(NULL));
  __blocked = true;
  sigset_t blocked;
  sigemptyset(&blocked);
  sigaddset(&blocked, TIMER_SIG);
  if (sigprocmask(SIG_SETMASK, &blocked, NULL) == -1)
    error_report("block interrupt failed\n");
}

void unblock_interrupt() {
  assert(__blocked == true);
  assert(qemu_cpu_is_self(NULL));
  __blocked = false;
  sigset_t blocked;
  sigemptyset(&blocked);
  if (sigprocmask(SIG_SETMASK, &blocked, NULL) == -1)
    error_report("unblock interrupt failed\n");
}

static bool __interrupt_context = false;
bool qemu_cpu_is_self(CPUState *cpu) { return !__interrupt_context; }
void enter_interrpt_context() {
  assert(__interrupt_context == false);
  __interrupt_context = true;
}

void leave_interrpt_context() {
  assert(__interrupt_context == true);
  __interrupt_context = false;
}

void fire_timer() { kill(getpid(), TIMER_SIG); }
