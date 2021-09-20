#ifndef TIMER_H_PBGYZKVR
#define TIMER_H_PBGYZKVR

#include "../types.h"
typedef enum {
  QEMU_CLOCK_REALTIME = 0,
  QEMU_CLOCK_VIRTUAL = 1,
  QEMU_CLOCK_HOST = 2,
  QEMU_CLOCK_VIRTUAL_RT = 3,
  QEMU_CLOCK_MAX
} QEMUClockType;

int64_t qemu_clock_get_ns(QEMUClockType type);

typedef struct {
} QEMUTimer;

void timer_mod(QEMUTimer *ts, int64_t expire_timer);

void timer_del(QEMUTimer *ts);

#endif /* end of include guard: TIMER_H_PBGYZKVR */
