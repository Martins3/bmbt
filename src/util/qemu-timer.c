#include "../../include/qemu/timer.h"

int64_t qemu_clock_get_ns(QEMUClockType type) {
  // @todo
  return 0;
}

void timer_mod(QEMUTimer *ts, int64_t expire_timer) {
  // @todo
}

void timer_del(QEMUTimer *ts) {
  // @todo
}

int64_t cpu_get_ticks(void) {
  // @todo
  return 0;
}

QEMUTimerListGroup main_loop_tlg;

void timer_init_full(QEMUTimer *ts, QEMUTimerListGroup *timer_list_group,
                     QEMUClockType type, int scale, int attributes,
                     QEMUTimerCB *cb, void *opaque) {
  if (!timer_list_group) {
    timer_list_group = &main_loop_tlg;
  }
  ts->timer_list = timer_list_group->tl[type];
  ts->cb = cb;
  ts->opaque = opaque;
  ts->scale = scale;
  ts->attributes = attributes;
  ts->expire_time = -1;
}

uint64_t timer_expire_time_ns(QEMUTimer *ts) {
  return timer_pending(ts) ? ts->expire_time : -1;
}

bool timer_pending(QEMUTimer *ts) { return ts->expire_time >= 0; }
