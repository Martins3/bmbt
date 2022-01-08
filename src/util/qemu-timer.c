#include "../../include/qemu/timer.h"
#include <qemu/error-report.h>
#include <qemu/log.h>
#include <qemu/main-loop.h>

/* A QEMUTimerList is a list of timers attached to a clock. More
 * than one QEMUTimerList can be attached to each clock, for instance
 * used by different AioContexts / threads. Each clock also has
 * a list of the QEMUTimerLists associated with it, in order that
 * reenabling the clock can call all the notifiers.
 */

typedef struct QEMUClock {
  /* We rely on BQL to protect the timerlists */
  QLIST_HEAD(, QEMUTimerList) timerlists;

  QEMUClockType type;
  bool enabled;
} QEMUClock;

struct QEMUTimerList {
  QEMUClock *clock;
  QemuMutex active_timers_lock;
  QEMUTimer *active_timers;
  QLIST_ENTRY(QEMUTimerList) list;
  // [interface 37]
  // QEMUTimerListNotifyCB *notify_cb;
  void *notify_opaque;

  // [interface 35]
#ifdef BMBT
  /* lightweight method to mark the end of timerlist's running */
  QemuEvent timers_done_ev;
#endif
};

QEMUTimerListGroup main_loop_tlg;

int64_t qemu_clock_get_ns(QEMUClockType type) {
  switch (type) {
  case QEMU_CLOCK_REALTIME:
    return get_clock();
  default:
  case QEMU_CLOCK_VIRTUAL:
    if (use_icount) {
      return cpu_get_icount();
    } else {
      return cpu_get_clock();
    }
  case QEMU_CLOCK_HOST:
    // return REPLAY_CLOCK(REPLAY_CLOCK_HOST, get_clock_realtime());
    g_assert_not_reached();
    return 0;
  case QEMU_CLOCK_VIRTUAL_RT:
    // return REPLAY_CLOCK(REPLAY_CLOCK_VIRTUAL_RT, cpu_get_clock());
    g_assert_not_reached();
    return 0;
  }
  return 0;
}

static void timer_del_locked(QEMUTimerList *timer_list, QEMUTimer *ts) {
  QEMUTimer **pt, *t;

  ts->expire_time = -1;
  pt = &timer_list->active_timers;
  for (;;) {
    t = *pt;
    if (!t)
      break;
    if (t == ts) {
      atomic_set(pt, t->next);
      break;
    }
    pt = &t->next;
  }
}

static bool timer_expired_ns(QEMUTimer *timer_head, int64_t current_time) {
  return timer_head && (timer_head->expire_time <= current_time);
}

static bool timer_mod_ns_locked(QEMUTimerList *timer_list, QEMUTimer *ts,
                                int64_t expire_time) {
  QEMUTimer **pt, *t;

  /* add the timer in the sorted list */
  pt = &timer_list->active_timers;
  for (;;) {
    t = *pt;
    if (!timer_expired_ns(t, expire_time)) {
      break;
    }
    pt = &t->next;
  }
  ts->expire_time = MAX(expire_time, 0);
  ts->next = *pt;
  atomic_set(pt, ts);

  return pt == &timer_list->active_timers;
}

// [interface 37]
void timerlist_notify(QEMUTimerList *timer_list) {
  duck_check(qemu_mutex_iothread_locked());
#ifdef BMBT
  if (timer_list->notify_cb) {
    timer_list->notify_cb(timer_list->notify_opaque, timer_list->clock->type);
  } else {
    qemu_notify_event();
  }
#endif
  fire_timer();
}

static void timerlist_rearm(QEMUTimerList *timer_list) {
  /* Interrupt execution to force deadline recalculation.  */
  if (timer_list->clock->type == QEMU_CLOCK_VIRTUAL) {
    qemu_start_warp_timer();
  }
  timerlist_notify(timer_list);
}

/* modify the current timer so that it will be fired when current_time
   >= expire_time. The corresponding callback will be called. */
void timer_mod_ns(QEMUTimer *ts, int64_t expire_time) {
  QEMUTimerList *timer_list = ts->timer_list;
  bool rearm;

  qemu_mutex_lock(&timer_list->active_timers_lock);
  timer_del_locked(timer_list, ts);
  rearm = timer_mod_ns_locked(timer_list, ts, expire_time);
  qemu_mutex_unlock(&timer_list->active_timers_lock);

  if (rearm) {
    timerlist_rearm(timer_list);
  }
}

void timer_mod(QEMUTimer *ts, int64_t expire_time) {
  timer_mod_ns(ts, expire_time * ts->scale);
}

void timer_del(QEMUTimer *ts) {
  QEMUTimerList *timer_list = ts->timer_list;

  if (timer_list) {
    qemu_mutex_lock(&timer_list->active_timers_lock);
    timer_del_locked(timer_list, ts);
    qemu_mutex_unlock(&timer_list->active_timers_lock);
  }
}

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

static QEMUClock qemu_clocks[QEMU_CLOCK_MAX];
/**
 * qemu_clock_ptr:
 * @type: type of clock
 *
 * Translate a clock type into a pointer to QEMUClock object.
 *
 * Returns: a pointer to the QEMUClock object
 */
static inline QEMUClock *qemu_clock_ptr(QEMUClockType type) {
  return &qemu_clocks[type];
}

QEMUTimerList *timerlist_new(QEMUClockType type, void *opaque) {
  QEMUTimerList *timer_list;
  QEMUClock *clock = qemu_clock_ptr(type);

  timer_list = g_malloc0(sizeof(QEMUTimerList));
  // qemu_event_init(&timer_list->timers_done_ev, true);
  timer_list->clock = clock;
  // timer_list->notify_cb = cb;
  timer_list->notify_opaque = opaque;
  qemu_mutex_init(&timer_list->active_timers_lock);
  QLIST_INSERT_HEAD(&clock->timerlists, timer_list, list);
  return timer_list;
}

static void qemu_clock_init(QEMUClockType type) {
  QEMUClock *clock = qemu_clock_ptr(type);

  /* Assert that the clock of type TYPE has not been initialized yet. */
  assert(main_loop_tlg.tl[type] == NULL);

  clock->type = type;
  clock->enabled = (type == QEMU_CLOCK_VIRTUAL ? false : true);
  QLIST_INIT(&clock->timerlists);
  main_loop_tlg.tl[type] = timerlist_new(type, NULL);
}

void init_clocks() {
  QEMUClockType type;
  for (type = 0; type < QEMU_CLOCK_MAX; type++) {
    qemu_clock_init(type);
  }

#ifdef BMBT
#ifdef CONFIG_PRCTL_PR_SET_TIMERSLACK
  prctl(PR_SET_TIMERSLACK, 1, 0, 0, 0);
#endif
#endif
}

bool timer_pending(QEMUTimer *ts) { return ts->expire_time >= 0; }

bool timerlist_run_timers(QEMUTimerList *timer_list) {
  QEMUTimer *ts;
  int64_t current_time;
  bool progress = false;
  QEMUTimerCB *cb;
  void *opaque;
  // bool need_replay_checkpoint = false;

  if (!atomic_read(&timer_list->active_timers)) {
    return false;
  }

  // [interface 35]
#ifdef BMBT
  qemu_event_reset(&timer_list->timers_done_ev);
#endif
  if (!timer_list->clock->enabled) {
    // I don't know why some timer already began before vm_start
    error_report("trace it, understand it and fix it");
    goto out;
  }

#ifdef BMBT
  switch (timer_list->clock->type) {
  case QEMU_CLOCK_REALTIME:
    break;
  default:
  case QEMU_CLOCK_VIRTUAL:
    if (replay_mode != REPLAY_MODE_NONE) {
      /* Checkpoint for virtual clock is redundant in cases where
       * it's being triggered with only non-EXTERNAL timers, because
       * these timers don't change guest state directly.
       * Since it has conditional dependence on specific timers, it is
       * subject to race conditions and requires special handling.
       * See below.
       */
      need_replay_checkpoint = true;
    }
    break;
  case QEMU_CLOCK_HOST:
    if (!replay_checkpoint(CHECKPOINT_CLOCK_HOST)) {
      goto out;
    }
    break;
  case QEMU_CLOCK_VIRTUAL_RT:
    if (!replay_checkpoint(CHECKPOINT_CLOCK_VIRTUAL_RT)) {
      goto out;
    }
    break;
  }
#endif

  /*
   * Extract expired timers from active timers list and and process them.
   *
   * In rr mode we need "filtered" checkpointing for virtual clock.  The
   * checkpoint must be recorded/replayed before processing any non-EXTERNAL
   * timer, and that must only be done once since the clock value stays the
   * same. Because non-EXTERNAL timers may appear in the timers list while it
   * being processed, the checkpoint can be issued at a time until no timers are
   * left and we are done".
   */
  current_time = qemu_clock_get_ns(timer_list->clock->type);
  qemu_mutex_lock(&timer_list->active_timers_lock);
  while ((ts = timer_list->active_timers)) {
    if (!timer_expired_ns(ts, current_time)) {
      /* No expired timers left.  The checkpoint can be skipped
       * if no timers fired or they were all external.
       */
      break;
    }
#ifdef BMBT
    if (need_replay_checkpoint &&
        !(ts->attributes & QEMU_TIMER_ATTR_EXTERNAL)) {
      /* once we got here, checkpoint clock only once */
      need_replay_checkpoint = false;
      qemu_mutex_unlock(&timer_list->active_timers_lock);
      if (!replay_checkpoint(CHECKPOINT_CLOCK_VIRTUAL)) {
        goto out;
      }
      qemu_mutex_lock(&timer_list->active_timers_lock);
      /* The lock was released; start over again in case the list was
       * modified.
       */
      continue;
    }
#endif

    /* remove timer from the list before calling the callback */
    timer_list->active_timers = ts->next;
    ts->next = NULL;
    ts->expire_time = -1;
    cb = ts->cb;
    opaque = ts->opaque;

    /* run the callback (the timer list can be modified) */
    qemu_mutex_unlock(&timer_list->active_timers_lock);
    cb(opaque);
    qemu_mutex_lock(&timer_list->active_timers_lock);

    progress = true;
  }
  qemu_mutex_unlock(&timer_list->active_timers_lock);

out:
  // [interface 35]
#ifdef BMBT
  qemu_event_set(&timer_list->timers_done_ev);
#endif
  return progress;
}

/* modify the current timer so that it will be fired when current_time
   >= expire_time or the current deadline, whichever comes earlier.
   The corresponding callback will be called. */
void timer_mod_anticipate_ns(QEMUTimer *ts, int64_t expire_time) {
  QEMUTimerList *timer_list = ts->timer_list;
  bool rearm;

  qemu_mutex_lock(&timer_list->active_timers_lock);
  if (ts->expire_time == -1 || ts->expire_time > expire_time) {
    if (ts->expire_time != -1) {
      timer_del_locked(timer_list, ts);
    }
    rearm = timer_mod_ns_locked(timer_list, ts, expire_time);
  } else {
    rearm = false;
  }
  qemu_mutex_unlock(&timer_list->active_timers_lock);

  if (rearm) {
    timerlist_rearm(timer_list);
  }
}

void timer_mod_anticipate(QEMUTimer *ts, int64_t expire_time) {
  timer_mod_anticipate_ns(ts, expire_time * ts->scale);
}

bool qemu_clock_use_for_deadline(QEMUClockType type) {
  return !(use_icount && (type == QEMU_CLOCK_VIRTUAL));
}

bool qemu_clock_run_timers(QEMUClockType type) {
  return timerlist_run_timers(main_loop_tlg.tl[type]);
}

bool qemu_clock_run_all_timers(void) {
  bool progress = false;
  QEMUClockType type;

  for (type = 0; type < QEMU_CLOCK_MAX; type++) {
    if (qemu_clock_use_for_deadline(type)) {
      progress |= qemu_clock_run_timers(type);
    }
  }

  return progress;
}

/*
 * As above, but return -1 for no deadline, and do not cap to 2^32
 * as we know the result is always positive.
 */

int64_t timerlist_deadline_ns(QEMUTimerList *timer_list) {
  int64_t delta;
  int64_t expire_time;

  if (!atomic_read(&timer_list->active_timers)) {
    return -1;
  }

  if (!timer_list->clock->enabled) {
    return -1;
  }

  /* The active timers list may be modified before the caller uses our return
   * value but ->notify_cb() is called when the deadline changes.  Therefore
   * the caller should notice the change and there is no race condition.
   */
  qemu_mutex_lock(&timer_list->active_timers_lock);
  if (!timer_list->active_timers) {
    qemu_mutex_unlock(&timer_list->active_timers_lock);
    return -1;
  }
  expire_time = timer_list->active_timers->expire_time;
  qemu_mutex_unlock(&timer_list->active_timers_lock);

  delta = expire_time - qemu_clock_get_ns(timer_list->clock->type);

  if (delta <= 0) {
    return 0;
  }

  return delta;
}

int64_t timerlistgroup_deadline_ns(QEMUTimerListGroup *tlg) {
  int64_t deadline = -1;
  QEMUClockType type;
  for (type = 0; type < QEMU_CLOCK_MAX; type++) {
    if (qemu_clock_use_for_deadline(type)) {
      deadline =
          qemu_soonest_timeout(deadline, timerlist_deadline_ns(tlg->tl[type]));
    }
  }
  return deadline;
}

static void timer_interrupt_handler() {
  qemu_log("timer interrupt comming");
  int64_t timeout_ns = -1;

  qemu_clock_run_all_timers();

  timeout_ns = qemu_soonest_timeout(timeout_ns,
                                    timerlistgroup_deadline_ns(&main_loop_tlg));
  if (timeout_ns == -1) {
    warn_report("no timer to fire");
  }
  soonest_interrupt_ns(timeout_ns);
}

void setup_timer_interrupt() {
  init_clocks();
  setup_timer(timer_interrupt_handler);
}

static void qemu_clock_notify(QEMUClockType type) {
  QEMUTimerList *timer_list;
  QEMUClock *clock = qemu_clock_ptr(type);
  QLIST_FOREACH(timer_list, &clock->timerlists, list) {
    timerlist_notify(timer_list);
  }
}

/* Disabling the clock will wait for related timerlists to stop
 * executing qemu_run_timers.  Thus, this functions should not
 * be used from the callback of a timer that is based on @clock.
 * Doing so would cause a deadlock.
 *
 * Caller should hold BQL.
 */
void qemu_clock_enable(QEMUClockType type, bool enabled) {
  QEMUClock *clock = qemu_clock_ptr(type);
  // QEMUTimerList *tl;
  bool old = clock->enabled;
  clock->enabled = enabled;
  if (enabled && !old) {
    qemu_clock_notify(type);
  } else if (!enabled && old) {
    // bmbt will never disable the clock
    g_assert_not_reached();
  }
}
