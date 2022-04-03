#ifndef TIMER_H_PBGYZKVR
#define TIMER_H_PBGYZKVR

#include <env/timer.h>
#include <signal.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <uglib.h>

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

int64_t cpu_get_icount(void);
int64_t cpu_get_clock(void);

/* get host real time in nanosecond */
static inline int64_t get_clock_realtime(void) {
  struct timeval tv;

  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000000LL + (tv.tv_usec * 1000);
}

/**
 * qemu_start_warp_timer:
 *
 * Starts a timer for virtual clock update
 */
void qemu_start_warp_timer(void);

/**
 * qemu_clock_enable:
 * @type: the clock type
 * @enabled: true to enable, false to disable
 *
 * Enable or disable a clock
 * Disabling the clock will wait for related timerlists to stop
 * executing qemu_run_timers.  Thus, this functions should not
 * be used from the callback of a timer that is based on @clock.
 * Doing so would cause a deadlock.
 *
 * Caller should hold BQL.
 */
void qemu_clock_enable(QEMUClockType type, bool enabled);

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

/**
 * timer_mod_anticipate:
 * @ts: the timer
 * @expire_time: the expiry time in nanoseconds
 *
 * Modify a timer to expire at @expire_time or the current time, whichever
 * comes earlier, taking into account the scale associated with the timer.
 *
 * This function is thread-safe but the timer and its timer list must not be
 * freed while this function is running.
 */
void timer_mod_anticipate(QEMUTimer *ts, int64_t expire_time);

/**
 * qemu_soonest_timeout:
 * @timeout1: first timeout in nanoseconds (or -1 for infinite)
 * @timeout2: second timeout in nanoseconds (or -1 for infinite)
 *
 * Calculates the soonest of two timeout values. -1 means infinite, which
 * is later than any other value.
 *
 * Returns: soonest timeout value in nanoseconds (or -1 for infinite)
 */
static inline int64_t qemu_soonest_timeout(int64_t timeout1, int64_t timeout2) {
  /* we can abuse the fact that -1 (which means infinite) is a maximal
   * value when cast to unsigned. As this is disgusting, it's kept in
   * one inline function.
   */
  return ((uint64_t)timeout1 < (uint64_t)timeout2) ? timeout1 : timeout2;
}

/**
 * qemu_clock_get_ms;
 * @type: the clock type
 *
 * Get the millisecond value of a clock with
 * type @type
 *
 * Returns: the clock value in milliseconds
 */
static inline int64_t qemu_clock_get_ms(QEMUClockType type) {
  return qemu_clock_get_ns(type) / SCALE_MS;
}

static inline int64_t get_clock(void) {
  {
    /* XXX: using gettimeofday leads to problems if the date
       changes, so it should be avoided. */
    return get_clock_realtime();
  }
}

/* The host CPU doesn't have an easily accessible cycle counter.
   Just return a monotonically increasing value.  This will be
   totally wrong, but hopefully better than nothing.  */
static inline int64_t cpu_get_host_ticks(void) { return get_clock(); }

// vl.c
void qemu_get_timedate(struct tm *tm, int offset);
#endif /* end of include guard: TIMER_H_PBGYZKVR */
