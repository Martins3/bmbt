#ifndef TIMER_H_PBGYZKVR
#define TIMER_H_PBGYZKVR

#include "../../src/tcg/glib_stub.h"
#include "../types.h"

#define NANOSECONDS_PER_SECOND 1000000000LL

/* timers */

#define SCALE_MS 1000000
#define SCALE_US 1000
#define SCALE_NS 1

typedef enum {
  QEMU_CLOCK_REALTIME = 0,
  QEMU_CLOCK_VIRTUAL = 1,
  QEMU_CLOCK_HOST = 2,
  QEMU_CLOCK_VIRTUAL_RT = 3,
  QEMU_CLOCK_MAX
} QEMUClockType;

int64_t qemu_clock_get_ns(QEMUClockType type);

typedef void QEMUTimerCB(void *opaque);

typedef struct QEMUTimerList QEMUTimerList;

typedef struct QEMUTimer {
  int64_t expire_time; /* in nanoseconds */
  QEMUTimerList *timer_list;
  QEMUTimerCB *cb;
  void *opaque;
  struct QEMUTimer *next;
  int attributes;
  int scale;
} QEMUTimer;

typedef struct {
  QEMUTimerList *tl[QEMU_CLOCK_MAX];
} QEMUTimerListGroup;

void timer_mod(QEMUTimer *ts, int64_t expire_timer);

void timer_del(QEMUTimer *ts);

int64_t cpu_get_ticks(void);
/* Caller must hold BQL */
void cpu_enable_ticks(void);
/* Caller must hold BQL */
void cpu_disable_ticks(void);

/**
 * timer_expire_time_ns:
 * @ts: the timer
 *
 * Determine the expiry time of a timer
 *
 * Returns: the expiry time in nanoseconds
 */
uint64_t timer_expire_time_ns(QEMUTimer *ts);

/**
 * timer_pending:
 * @ts: the timer
 *
 * Determines whether a timer is pending (i.e. is on the
 * active list of timers, whether or not it has not yet expired).
 *
 * Returns: true if the timer is pending
 */
bool timer_pending(QEMUTimer *ts);

/**
 * timer_init_full:
 * @ts: the timer to be initialised
 * @timer_list_group: (optional) the timer list group to attach the timer to
 * @type: the clock type to use
 * @scale: the scale value for the timer
 * @attributes: 0, or one or more OR'ed QEMU_TIMER_ATTR_<id> values
 * @cb: the callback to be called when the timer expires
 * @opaque: the opaque pointer to be passed to the callback
 *
 * Initialise a timer with the given scale and attributes,
 * and associate it with timer list for given clock @type in @timer_list_group
 * (or default timer list group, if NULL).
 * The caller is responsible for allocating the memory.
 *
 * You need not call an explicit deinit call. Simply make
 * sure it is not on a list with timer_del.
 */
void timer_init_full(QEMUTimer *ts, QEMUTimerListGroup *timer_list_group,
                     QEMUClockType type, int scale, int attributes,
                     QEMUTimerCB *cb, void *opaque);

/**
 * timer_new_full:
 * @timer_list_group: (optional) the timer list group to attach the timer to
 * @type: the clock type to use
 * @scale: the scale value for the timer
 * @attributes: 0, or one or more OR'ed QEMU_TIMER_ATTR_<id> values
 * @cb: the callback to be called when the timer expires
 * @opaque: the opaque pointer to be passed to the callback
 *
 * Create a new timer with the given scale and attributes,
 * and associate it with timer list for given clock @type in @timer_list_group
 * (or default timer list group, if NULL).
 * The memory is allocated by the function.
 *
 * This is not the preferred interface unless you know you
 * are going to call timer_free. Use timer_init or timer_init_full instead.
 *
 * The default timer list has one special feature: in icount mode,
 * %QEMU_CLOCK_VIRTUAL timers are run in the vCPU thread.  This is
 * not true of other timer lists, which are typically associated
 * with an AioContext---each of them runs its timer callbacks in its own
 * AioContext thread.
 *
 * Returns: a pointer to the timer
 */
static inline QEMUTimer *timer_new_full(QEMUTimerListGroup *timer_list_group,
                                        QEMUClockType type, int scale,
                                        int attributes, QEMUTimerCB *cb,
                                        void *opaque) {
  QEMUTimer *ts = g_malloc0(sizeof(QEMUTimer));
  timer_init_full(ts, timer_list_group, type, scale, attributes, cb, opaque);
  return ts;
}

/**
 * timer_new:
 * @type: the clock type to use
 * @scale: the scale value for the timer
 * @cb: the callback to be called when the timer expires
 * @opaque: the opaque pointer to be passed to the callback
 *
 * Create a new timer with the given scale,
 * and associate it with the default timer list for the clock type @type.
 * See timer_new_full for details.
 *
 * Returns: a pointer to the timer
 */
static inline QEMUTimer *timer_new(QEMUClockType type, int scale,
                                   QEMUTimerCB *cb, void *opaque) {
  return timer_new_full(NULL, type, scale, 0, cb, opaque);
}

/**
 * timer_new_ns:
 * @type: the clock type to associate with the timer
 * @cb: the callback to call when the timer expires
 * @opaque: the opaque pointer to pass to the callback
 *
 * Create a new timer with nanosecond scale on the default timer list
 * associated with the clock.
 * See timer_new_full for details.
 *
 * Returns: a pointer to the newly created timer
 */
static inline QEMUTimer *timer_new_ns(QEMUClockType type, QEMUTimerCB *cb,
                                      void *opaque) {
  return timer_new(type, SCALE_NS, cb, opaque);
}

// signal-timer.c
typedef void(TimerHandler)(int sig, siginfo_t *si, void *uc);
timer_t setup_timer(TimerHandler handler);
void soonest_timer_ns(timer_t tid, long ns);
void soonest_timer_s(timer_t tid, long s);
void block_interrupt();
void unblock_interrupt();
bool is_interrupt_blocked();
void fire_timer();

#endif /* end of include guard: TIMER_H_PBGYZKVR */
