#include <hw/rtc/mc146818rtc.h>
#include <hw/rtc/mc146818rtc_regs.h>
#include <qemu/cutils.h>
#include <sysemu/reset.h>
#include <sysemu/sysemu.h>

/* #define DEBUG_CMOS */
#define DEBUG_COALESCED

#ifdef DEBUG_CMOS
#define CMOS_DPRINTF(format, ...) printf(format, ##__VA_ARGS__)
#else
#define CMOS_DPRINTF(format, ...)                                              \
  do {                                                                         \
  } while (0)
#endif

#ifdef DEBUG_COALESCED
#define DPRINTF_C(format, ...) printf(format, ##__VA_ARGS__)
#else
#define DPRINTF_C(format, ...)                                                 \
  do {                                                                         \
  } while (0)
#endif

#define SEC_PER_MIN 60
#define MIN_PER_HOUR 60
#define SEC_PER_HOUR 3600
#define HOUR_PER_DAY 24
#define SEC_PER_DAY 86400

#define RTC_REINJECT_ON_ACK_COUNT 20
#define RTC_CLOCK_RATE 32768
#define UIP_HOLD_LENGTH (8 * NANOSECONDS_PER_SECOND / 32768)

static void rtc_set_time(RTCState *s);
static void rtc_update_time(RTCState *s);
static void rtc_set_cmos(RTCState *s, const struct tm *tm);
static inline int rtc_from_bcd(RTCState *s, int a);
static uint64_t get_next_alarm(RTCState *s);

static inline bool rtc_running(RTCState *s) {
  return (!(s->cmos_data[RTC_REG_B] & REG_B_SET) &&
          (s->cmos_data[RTC_REG_A] & 0x70) <= 0x20);
}

static uint64_t get_guest_rtc_ns(RTCState *s) {
  uint64_t guest_clock = qemu_clock_get_ns(rtc_clock);

  return s->base_rtc * NANOSECONDS_PER_SECOND + guest_clock - s->last_update +
         s->offset;
}

#ifdef BMBT
static void rtc_coalesced_timer_update(RTCState *s) { g_assert_not_reached(); }
#endif

static QLIST_HEAD(, RTCState) rtc_devices = QLIST_HEAD_INITIALIZER(rtc_devices);

#ifdef TARGET_I386
void qmp_rtc_reset_reinjection() {
  RTCState *s;
  g_assert_not_reached();

  QLIST_FOREACH(s, &rtc_devices, link) { s->irq_coalesced = 0; }
}

#ifdef BMBT
static bool rtc_policy_slew_deliver_irq(RTCState *s) {
  apic_reset_irq_delivered();
  qemu_irq_raise(s->irq);
  return apic_get_irq_delivered();
}

static void rtc_coalesced_timer(void *opaque) { g_assert_not_reached(); }
#endif
#else
static bool rtc_policy_slew_deliver_irq(RTCState *s) {
  assert(0);
  return false;
}
#endif

static uint32_t rtc_periodic_clock_ticks(RTCState *s) {
  int period_code;

  if (!(s->cmos_data[RTC_REG_B] & REG_B_PIE)) {
    return 0;
  }

  period_code = s->cmos_data[RTC_REG_A] & 0x0f;

  return periodic_period_to_clock(period_code);
}

/*
 * handle periodic timer. @old_period indicates the periodic timer update
 * is just due to period adjustment.
 */
static void periodic_timer_update(RTCState *s, int64_t current_time,
                                  uint32_t old_period, bool period_change) {
  // in QEMU, never called
  g_assert_not_reached();
}

static void rtc_periodic_timer(void *opaque) {
  // in QEMU, never called
  g_assert_not_reached();
}

/* handle update-ended timer */
static void check_update_timer(RTCState *s) {
  uint64_t next_update_time;
  uint64_t guest_nsec;
  int next_alarm_sec;

  /* From the data sheet: "Holding the dividers in reset prevents
   * interrupts from operating, while setting the SET bit allows"
   * them to occur.
   */
  if ((s->cmos_data[RTC_REG_A] & 0x60) == 0x60) {
    assert((s->cmos_data[RTC_REG_A] & REG_A_UIP) == 0);
    timer_del(s->update_timer);
    return;
  }

  guest_nsec = get_guest_rtc_ns(s) % NANOSECONDS_PER_SECOND;
  next_update_time =
      qemu_clock_get_ns(rtc_clock) + NANOSECONDS_PER_SECOND - guest_nsec;

  /* Compute time of next alarm.  One second is already accounted
   * for in next_update_time.
   */
  next_alarm_sec = get_next_alarm(s);
  s->next_alarm_time =
      next_update_time + (next_alarm_sec - 1) * NANOSECONDS_PER_SECOND;

  /* If update_in_progress latched the UIP bit, we must keep the timer
   * programmed to the next second, so that UIP is cleared.  Otherwise,
   * if UF is already set, we might be able to optimize.
   */
  if (!(s->cmos_data[RTC_REG_A] & REG_A_UIP) &&
      (s->cmos_data[RTC_REG_C] & REG_C_UF)) {
    /* If AF cannot change (i.e. either it is set already, or
     * SET=1 and then the time is not updated), nothing to do.
     */
    if ((s->cmos_data[RTC_REG_B] & REG_B_SET) ||
        (s->cmos_data[RTC_REG_C] & REG_C_AF)) {
      timer_del(s->update_timer);
      return;
    }

    /* UF is set, but AF is clear.  Program the timer to target
     * the alarm time.  */
    next_update_time = s->next_alarm_time;
  }
  if (next_update_time != timer_expire_time_ns(s->update_timer)) {
    timer_mod(s->update_timer, next_update_time);
  }
}

static inline uint8_t convert_hour(RTCState *s, uint8_t hour) {
  if (!(s->cmos_data[RTC_REG_B] & REG_B_24H)) {
    hour %= 12;
    if (s->cmos_data[RTC_HOURS] & 0x80) {
      hour += 12;
    }
  }
  return hour;
}

static uint64_t get_next_alarm(RTCState *s) {
  int32_t alarm_sec, alarm_min, alarm_hour, cur_hour, cur_min, cur_sec;
  int32_t hour, min, sec;

  rtc_update_time(s);

  alarm_sec = rtc_from_bcd(s, s->cmos_data[RTC_SECONDS_ALARM]);
  alarm_min = rtc_from_bcd(s, s->cmos_data[RTC_MINUTES_ALARM]);
  alarm_hour = rtc_from_bcd(s, s->cmos_data[RTC_HOURS_ALARM]);
  alarm_hour = alarm_hour == -1 ? -1 : convert_hour(s, alarm_hour);

  cur_sec = rtc_from_bcd(s, s->cmos_data[RTC_SECONDS]);
  cur_min = rtc_from_bcd(s, s->cmos_data[RTC_MINUTES]);
  cur_hour = rtc_from_bcd(s, s->cmos_data[RTC_HOURS]);
  cur_hour = convert_hour(s, cur_hour);

  if (alarm_hour == -1) {
    alarm_hour = cur_hour;
    if (alarm_min == -1) {
      alarm_min = cur_min;
      if (alarm_sec == -1) {
        alarm_sec = cur_sec + 1;
      } else if (cur_sec > alarm_sec) {
        alarm_min++;
      }
    } else if (cur_min == alarm_min) {
      if (alarm_sec == -1) {
        alarm_sec = cur_sec + 1;
      } else {
        if (cur_sec > alarm_sec) {
          alarm_hour++;
        }
      }
      if (alarm_sec == SEC_PER_MIN) {
        /* wrap to next hour, minutes is not in don't care mode */
        alarm_sec = 0;
        alarm_hour++;
      }
    } else if (cur_min > alarm_min) {
      alarm_hour++;
    }
  } else if (cur_hour == alarm_hour) {
    if (alarm_min == -1) {
      alarm_min = cur_min;
      if (alarm_sec == -1) {
        alarm_sec = cur_sec + 1;
      } else if (cur_sec > alarm_sec) {
        alarm_min++;
      }

      if (alarm_sec == SEC_PER_MIN) {
        alarm_sec = 0;
        alarm_min++;
      }
      /* wrap to next day, hour is not in don't care mode */
      alarm_min %= MIN_PER_HOUR;
    } else if (cur_min == alarm_min) {
      if (alarm_sec == -1) {
        alarm_sec = cur_sec + 1;
      }
      /* wrap to next day, hours+minutes not in don't care mode */
      alarm_sec %= SEC_PER_MIN;
    }
  }

  /* values that are still don't care fire at the next min/sec */
  if (alarm_min == -1) {
    alarm_min = 0;
  }
  if (alarm_sec == -1) {
    alarm_sec = 0;
  }

  /* keep values in range */
  if (alarm_sec == SEC_PER_MIN) {
    alarm_sec = 0;
    alarm_min++;
  }
  if (alarm_min == MIN_PER_HOUR) {
    alarm_min = 0;
    alarm_hour++;
  }
  alarm_hour %= HOUR_PER_DAY;

  hour = alarm_hour - cur_hour;
  min = hour * MIN_PER_HOUR + alarm_min - cur_min;
  sec = min * SEC_PER_MIN + alarm_sec - cur_sec;
  return sec <= 0 ? sec + SEC_PER_DAY : sec;
}

static void rtc_update_timer(void *opaque) {
  RTCState *s = opaque;
  int32_t irqs = REG_C_UF;
  int32_t new_irqs;

  assert((s->cmos_data[RTC_REG_A] & 0x60) != 0x60);

  /* UIP might have been latched, update time and clear it.  */
  rtc_update_time(s);
  s->cmos_data[RTC_REG_A] &= ~REG_A_UIP;

  if (qemu_clock_get_ns(rtc_clock) >= s->next_alarm_time) {
    irqs |= REG_C_AF;
    if (s->cmos_data[RTC_REG_B] & REG_B_AIE) {
      g_assert_not_reached();
      // qemu_system_wakeup_request(QEMU_WAKEUP_REASON_RTC, NULL);
    }
  }

  new_irqs = irqs & ~s->cmos_data[RTC_REG_C];
  s->cmos_data[RTC_REG_C] |= irqs;
  if ((new_irqs & s->cmos_data[RTC_REG_B]) != 0) {
    s->cmos_data[RTC_REG_C] |= REG_C_IRQF;
    qemu_irq_raise(s->irq);
  }
  check_update_timer(s);
}

static void cmos_ioport_write(void *opaque, hwaddr addr, uint64_t data,
                              unsigned size) {
  RTCState *s = opaque;
  uint32_t old_period;
  bool update_periodic_timer;

  if ((addr & 1) == 0) {
    s->cmos_index = data & 0x7f;
  } else {
    CMOS_DPRINTF("cmos: write index=0x%02x val=0x%02" PRIx64 "\n",
                 s->cmos_index, data);
    switch (s->cmos_index) {
    case RTC_SECONDS_ALARM:
    case RTC_MINUTES_ALARM:
    case RTC_HOURS_ALARM:
      s->cmos_data[s->cmos_index] = data;
      check_update_timer(s);
      break;
    case RTC_IBM_PS2_CENTURY_BYTE:
      s->cmos_index = RTC_CENTURY;
      /* fall through */
    case RTC_CENTURY:
    case RTC_SECONDS:
    case RTC_MINUTES:
    case RTC_HOURS:
    case RTC_DAY_OF_WEEK:
    case RTC_DAY_OF_MONTH:
    case RTC_MONTH:
    case RTC_YEAR:
      s->cmos_data[s->cmos_index] = data;
      /* if in set mode, do not update the time */
      if (rtc_running(s)) {
        rtc_set_time(s);
        check_update_timer(s);
      }
      break;
    case RTC_REG_A:
      update_periodic_timer = (s->cmos_data[RTC_REG_A] ^ data) & 0x0f;
      old_period = rtc_periodic_clock_ticks(s);

      if ((data & 0x60) == 0x60) {
        if (rtc_running(s)) {
          rtc_update_time(s);
        }
        /* What happens to UIP when divider reset is enabled is
         * unclear from the datasheet.  Shouldn't matter much
         * though.
         */
        s->cmos_data[RTC_REG_A] &= ~REG_A_UIP;
      } else if (((s->cmos_data[RTC_REG_A] & 0x60) == 0x60) &&
                 (data & 0x70) <= 0x20) {
        /* when the divider reset is removed, the first update cycle
         * begins one-half second later*/
        if (!(s->cmos_data[RTC_REG_B] & REG_B_SET)) {
          s->offset = 500000000;
          rtc_set_time(s);
        }
        s->cmos_data[RTC_REG_A] &= ~REG_A_UIP;
      }
      /* UIP bit is read only */
      s->cmos_data[RTC_REG_A] =
          (data & ~REG_A_UIP) | (s->cmos_data[RTC_REG_A] & REG_A_UIP);

      if (update_periodic_timer) {
        periodic_timer_update(s, qemu_clock_get_ns(rtc_clock), old_period,
                              true);
      }

      check_update_timer(s);
      break;
    case RTC_REG_B:
      update_periodic_timer = (s->cmos_data[RTC_REG_B] ^ data) & REG_B_PIE;
      old_period = rtc_periodic_clock_ticks(s);

      if (data & REG_B_SET) {
        /* update cmos to when the rtc was stopping */
        if (rtc_running(s)) {
          rtc_update_time(s);
        }
        /* set mode: reset UIP mode */
        s->cmos_data[RTC_REG_A] &= ~REG_A_UIP;
        data &= ~REG_B_UIE;
      } else {
        /* if disabling set mode, update the time */
        if ((s->cmos_data[RTC_REG_B] & REG_B_SET) &&
            (s->cmos_data[RTC_REG_A] & 0x70) <= 0x20) {
          s->offset = get_guest_rtc_ns(s) % NANOSECONDS_PER_SECOND;
          rtc_set_time(s);
        }
      }
      /* if an interrupt flag is already set when the interrupt
       * becomes enabled, raise an interrupt immediately.  */
      if (data & s->cmos_data[RTC_REG_C] & REG_C_MASK) {
        s->cmos_data[RTC_REG_C] |= REG_C_IRQF;
        qemu_irq_raise(s->irq);
      } else {
        s->cmos_data[RTC_REG_C] &= ~REG_C_IRQF;
        qemu_irq_lower(s->irq);
      }
      s->cmos_data[RTC_REG_B] = data;

      if (update_periodic_timer) {
        periodic_timer_update(s, qemu_clock_get_ns(rtc_clock), old_period,
                              true);
      }

      check_update_timer(s);
      break;
    case RTC_REG_C:
    case RTC_REG_D:
      /* cannot write to them */
      break;
    default:
      s->cmos_data[s->cmos_index] = data;
      break;
    }
  }
}

static inline int rtc_to_bcd(RTCState *s, int a) {
  if (s->cmos_data[RTC_REG_B] & REG_B_DM) {
    return a;
  } else {
    return ((a / 10) << 4) | (a % 10);
  }
}

static inline int rtc_from_bcd(RTCState *s, int a) {
  if ((a & 0xc0) == 0xc0) {
    return -1;
  }
  if (s->cmos_data[RTC_REG_B] & REG_B_DM) {
    return a;
  } else {
    return ((a >> 4) * 10) + (a & 0x0f);
  }
}

static void rtc_get_time(RTCState *s, struct tm *tm) {
  tm->tm_sec = rtc_from_bcd(s, s->cmos_data[RTC_SECONDS]);
  tm->tm_min = rtc_from_bcd(s, s->cmos_data[RTC_MINUTES]);
  tm->tm_hour = rtc_from_bcd(s, s->cmos_data[RTC_HOURS] & 0x7f);
  if (!(s->cmos_data[RTC_REG_B] & REG_B_24H)) {
    tm->tm_hour %= 12;
    if (s->cmos_data[RTC_HOURS] & 0x80) {
      tm->tm_hour += 12;
    }
  }
  tm->tm_wday = rtc_from_bcd(s, s->cmos_data[RTC_DAY_OF_WEEK]) - 1;
  tm->tm_mday = rtc_from_bcd(s, s->cmos_data[RTC_DAY_OF_MONTH]);
  tm->tm_mon = rtc_from_bcd(s, s->cmos_data[RTC_MONTH]) - 1;
  tm->tm_year = rtc_from_bcd(s, s->cmos_data[RTC_YEAR]) + s->base_year +
                rtc_from_bcd(s, s->cmos_data[RTC_CENTURY]) * 100 - 1900;
}

static void rtc_set_time(RTCState *s) {
  struct tm tm;

  rtc_get_time(s, &tm);
  s->base_rtc = mktimegm(&tm);
  s->last_update = qemu_clock_get_ns(rtc_clock);

#ifdef BMBT
  qapi_event_send_rtc_change(qemu_timedate_diff(&tm));
#endif
}

static void rtc_set_cmos(RTCState *s, const struct tm *tm) {
  int year;

  s->cmos_data[RTC_SECONDS] = rtc_to_bcd(s, tm->tm_sec);
  s->cmos_data[RTC_MINUTES] = rtc_to_bcd(s, tm->tm_min);
  if (s->cmos_data[RTC_REG_B] & REG_B_24H) {
    /* 24 hour format */
    s->cmos_data[RTC_HOURS] = rtc_to_bcd(s, tm->tm_hour);
  } else {
    /* 12 hour format */
    int h = (tm->tm_hour % 12) ? tm->tm_hour % 12 : 12;
    s->cmos_data[RTC_HOURS] = rtc_to_bcd(s, h);
    if (tm->tm_hour >= 12)
      s->cmos_data[RTC_HOURS] |= 0x80;
  }
  s->cmos_data[RTC_DAY_OF_WEEK] = rtc_to_bcd(s, tm->tm_wday + 1);
  s->cmos_data[RTC_DAY_OF_MONTH] = rtc_to_bcd(s, tm->tm_mday);
  s->cmos_data[RTC_MONTH] = rtc_to_bcd(s, tm->tm_mon + 1);
  year = tm->tm_year + 1900 - s->base_year;
  // temporal fix the rtc time,see #321
  s->cmos_data[RTC_YEAR] = rtc_to_bcd(s, 0);
  s->cmos_data[RTC_CENTURY] = rtc_to_bcd(s, year / 100);
}

static void rtc_update_time(RTCState *s) {
  struct tm ret;
  time_t guest_sec;
  int64_t guest_nsec;

  guest_nsec = get_guest_rtc_ns(s);
  guest_sec = guest_nsec / NANOSECONDS_PER_SECOND;
  gmtime_r(&guest_sec, &ret);

  /* Is SET flag of Register B disabled? */
  if ((s->cmos_data[RTC_REG_B] & REG_B_SET) == 0) {
    rtc_set_cmos(s, &ret);
  }
}

static int update_in_progress(RTCState *s) {
  int64_t guest_nsec;

  if (!rtc_running(s)) {
    return 0;
  }
  if (timer_pending(s->update_timer)) {
    int64_t next_update_time = timer_expire_time_ns(s->update_timer);
    /* Latch UIP until the timer expires.  */

    if(next_update_time < qemu_clock_get_ns(rtc_clock)){
      assert(false);
    }

    if (qemu_clock_get_ns(rtc_clock) >= (next_update_time - UIP_HOLD_LENGTH)) {
      s->cmos_data[RTC_REG_A] |= REG_A_UIP;
      return 1;
    }
  }

  guest_nsec = get_guest_rtc_ns(s);
  /* UIP bit will be set at last 244us of every second. */
  if ((guest_nsec % NANOSECONDS_PER_SECOND) >=
      (NANOSECONDS_PER_SECOND - UIP_HOLD_LENGTH)) {
    return 1;
  }
  return 0;
}

static uint64_t cmos_ioport_read(void *opaque, hwaddr addr, unsigned size) {
  RTCState *s = opaque;
  int ret;
  if ((addr & 1) == 0) {
    return 0xff;
  } else {
    switch (s->cmos_index) {
    case RTC_IBM_PS2_CENTURY_BYTE:
      s->cmos_index = RTC_CENTURY;
      /* fall through */
    case RTC_CENTURY:
    case RTC_SECONDS:
    case RTC_MINUTES:
    case RTC_HOURS:
    case RTC_DAY_OF_WEEK:
    case RTC_DAY_OF_MONTH:
    case RTC_MONTH:
    case RTC_YEAR:
      /* if not in set mode, calibrate cmos before
       * reading*/
      if (rtc_running(s)) {
        rtc_update_time(s);
      }
      ret = s->cmos_data[s->cmos_index];
      break;
    case RTC_REG_A:
      ret = s->cmos_data[s->cmos_index];
      if (update_in_progress(s)) {
        ret |= REG_A_UIP;
      }
      break;
    case RTC_REG_C:
      ret = s->cmos_data[s->cmos_index];
      qemu_irq_lower(s->irq);
      s->cmos_data[RTC_REG_C] = 0x00;
      if (ret & (REG_C_UF | REG_C_AF)) {
        check_update_timer(s);
      }

      if (s->irq_coalesced && (s->cmos_data[RTC_REG_B] & REG_B_PIE) &&
          s->irq_reinject_on_ack_count < RTC_REINJECT_ON_ACK_COUNT) {
        g_assert_not_reached();
#ifdef BMBT
        s->irq_reinject_on_ack_count++;
        s->cmos_data[RTC_REG_C] |= REG_C_IRQF | REG_C_PF;
        DPRINTF_C("cmos: injecting on ack\n");
        if (rtc_policy_slew_deliver_irq(s)) {
          s->irq_coalesced--;
          DPRINTF_C("cmos: coalesced irqs decreased to %d\n", s->irq_coalesced);
        }
#endif
      }
      break;
    default:
      ret = s->cmos_data[s->cmos_index];
      break;
    }
    CMOS_DPRINTF("cmos: read index=0x%02x val=0x%02x\n", s->cmos_index, ret);
    return ret;
  }
}

void rtc_set_memory(RTCState *s, int addr, int val) {
  if (addr >= 0 && addr <= 127)
    s->cmos_data[addr] = val;
  else
    g_assert_not_reached();
}

int rtc_get_memory(RTCState *s, int addr) {
  assert(addr >= 0 && addr <= 127);
  return s->cmos_data[addr];
}

static void rtc_set_date_from_host(RTCState *s) {
  struct tm tm;

  qemu_get_timedate(&tm, 0);

  s->base_rtc = mktimegm(&tm);
  s->last_update = qemu_clock_get_ns(rtc_clock);
  s->offset = 0;

  /* set the CMOS date */
  rtc_set_cmos(s, &tm);
}

#ifdef BMBT
static int rtc_pre_save(void *opaque) {
  RTCState *s = opaque;

  rtc_update_time(s);

  return 0;
}

static int rtc_post_load(void *opaque, int version_id) {
  RTCState *s = opaque;

  if (version_id <= 2 || rtc_clock == QEMU_CLOCK_REALTIME) {
    rtc_set_time(s);
    s->offset = 0;
    check_update_timer(s);
  }
  s->period = rtc_periodic_clock_ticks(s);

  /* The periodic timer is deterministic in record/replay mode,
   * so there is no need to update it after loading the vmstate.
   * Reading RTC here would misalign record and replay.
   */
  if (replay_mode == REPLAY_MODE_NONE) {
    uint64_t now = qemu_clock_get_ns(rtc_clock);
    if (now < s->next_periodic_time ||
        now > (s->next_periodic_time + get_max_clock_jump())) {
      periodic_timer_update(s, qemu_clock_get_ns(rtc_clock), s->period, false);
    }
  }

  if (version_id >= 2) {
    if (s->lost_tick_policy == LOST_TICK_POLICY_SLEW) {
      rtc_coalesced_timer_update(s);
    }
  }
  return 0;
}

static bool rtc_irq_reinject_on_ack_count_needed(void *opaque) {
  RTCState *s = (RTCState *)opaque;
  return s->irq_reinject_on_ack_count != 0;
}

static const VMStateDescription vmstate_rtc_irq_reinject_on_ack_count = {
    .name = "mc146818rtc/irq_reinject_on_ack_count",
    .version_id = 1,
    .minimum_version_id = 1,
    .needed = rtc_irq_reinject_on_ack_count_needed,
    .fields =
        (VMStateField[]){VMSTATE_UINT16(irq_reinject_on_ack_count, RTCState),
                         VMSTATE_END_OF_LIST()}};

static const VMStateDescription vmstate_rtc = {
    .name = "mc146818rtc",
    .version_id = 3,
    .minimum_version_id = 1,
    .pre_save = rtc_pre_save,
    .post_load = rtc_post_load,
    .fields = (VMStateField[]){VMSTATE_BUFFER(cmos_data, RTCState),
                               VMSTATE_UINT8(cmos_index, RTCState),
                               VMSTATE_UNUSED(7 * 4),
                               VMSTATE_TIMER_PTR(periodic_timer, RTCState),
                               VMSTATE_INT64(next_periodic_time, RTCState),
                               VMSTATE_UNUSED(3 * 8),
                               VMSTATE_UINT32_V(irq_coalesced, RTCState, 2),
                               VMSTATE_UINT32_V(period, RTCState, 2),
                               VMSTATE_UINT64_V(base_rtc, RTCState, 3),
                               VMSTATE_UINT64_V(last_update, RTCState, 3),
                               VMSTATE_INT64_V(offset, RTCState, 3),
                               VMSTATE_TIMER_PTR_V(update_timer, RTCState, 3),
                               VMSTATE_UINT64_V(next_alarm_time, RTCState, 3),
                               VMSTATE_END_OF_LIST()},
    .subsections = (const VMStateDescription *[]){
        &vmstate_rtc_irq_reinject_on_ack_count, NULL}};

/* set CMOS shutdown status register (index 0xF) as S3_resume(0xFE)
   BIOS will read it and start S3 resume at POST Entry */
static void rtc_notify_suspend(Notifier *notifier, void *data) {
  RTCState *s = container_of(notifier, RTCState, suspend_notifier);
  rtc_set_memory(s, 0xF, 0xFE);
}
#endif

static void rtc_reset(void *opaque) {
  RTCState *s = opaque;

  s->cmos_data[RTC_REG_B] &= ~(REG_B_PIE | REG_B_AIE | REG_B_SQWE);
  s->cmos_data[RTC_REG_C] &= ~(REG_C_UF | REG_C_IRQF | REG_C_PF | REG_C_AF);
  check_update_timer(s);

  qemu_irq_lower(s->irq);

  if (s->lost_tick_policy == LOST_TICK_POLICY_SLEW) {
    s->irq_coalesced = 0;
    s->irq_reinject_on_ack_count = 0;
  }
}

static const MemoryRegionOps cmos_ops = {
    .read = cmos_ioport_read,
    .write = cmos_ioport_write,
    .impl =
        {
            .min_access_size = 1,
            .max_access_size = 1,
        },
    .endianness = DEVICE_LITTLE_ENDIAN,
};

#ifdef BMBT
static void rtc_get_date(RTCState *s, struct tm *current_tm) {
  RTCState *s = MC146818_RTC(obj);

  rtc_update_time(s);
  rtc_get_time(s, current_tm);
}
#endif

static void rtc_realizefn(RTCState *s) {
  int base = 0x70;

  s->cmos_data[RTC_REG_A] = 0x26;
  s->cmos_data[RTC_REG_B] = 0x02;
  s->cmos_data[RTC_REG_C] = 0x00;
  s->cmos_data[RTC_REG_D] = 0x80;

  /* This is for historical reasons.  The default base year qdev property
   * was set to 2000 for most machine types before the century byte was
   * implemented.
   *
   * This if statement means that the century byte will be always 0
   * (at least until 2079...) for base_year = 1980, but will be set
   * correctly for base_year = 2000.
   */
  if (s->base_year == 2000) {
    s->base_year = 0;
  }

  rtc_set_date_from_host(s);

  assert(s->lost_tick_policy == LOST_TICK_POLICY_DISCARD);
#ifdef BMBT
  switch (s->lost_tick_policy) {
#ifdef TARGET_I386
  case LOST_TICK_POLICY_SLEW:
    s->coalesced_timer = timer_new_ns(rtc_clock, rtc_coalesced_timer, s);
    break;
#endif
  case LOST_TICK_POLICY_DISCARD:
    break;
  default:
    error_setg(errp, "Invalid lost tick policy.");
    return;
  }
#endif

  s->periodic_timer = timer_new_ns(rtc_clock, rtc_periodic_timer, s);
  s->update_timer = timer_new_ns(rtc_clock, rtc_update_timer, s);
  check_update_timer(s);

#ifdef BMBT
  s->suspend_notifier.notify = rtc_notify_suspend;
  qemu_register_suspend_notifier(&s->suspend_notifier);
#endif

  memory_region_init_io(&s->io, &cmos_ops, s, "rtc", 2);
#ifdef BMBT
  isa_register_ioport(isadev, &s->io, base);
#else
  io_add_memory_region(base, &s->io);
#endif

  // coalesced io is used by kvm
#ifdef BMBT
  memory_region_set_flush_coalesced(&s->io);
  memory_region_init_io(&s->coalesced_io, &cmos_ops, s, "rtc-index", 1);
  memory_region_add_subregion(&s->io, 0, &s->coalesced_io);
  memory_region_add_coalescing(&s->coalesced_io, 0, 1);
#endif

  // qdev_set_legacy_instance_id(dev, base, 3);
  qemu_register_reset(rtc_reset, s);

  // object_property_add_tm(OBJECT(s), "date", rtc_get_date, NULL);

  qdev_init_gpio_out(&s->gpio, &s->irq, 1);
  QLIST_INSERT_HEAD(&rtc_devices, s, link);
}

static RTCState __mc146818_rtc;
RTCState *mc146818_rtc_init(int base_year, qemu_irq intercept_irq) {
  RTCState *mc146818_rtc = &__mc146818_rtc;
  assert(intercept_irq != NULL);
#ifdef BMBT
  DeviceState *dev;
  ISADevice *isadev;

  isadev = isa_create(bus, TYPE_MC146818_RTC);
  dev = DEVICE(isadev);
  qdev_prop_set_int32(dev, "base_year", base_year);
  qdev_init_nofail(dev);
  if (intercept_irq) {
    qdev_connect_gpio_out(dev, 0, intercept_irq);
  } else {
    isa_connect_gpio_out(isadev, 0, RTC_ISA_IRQ);
  }

  object_property_add_alias(qdev_get_machine(), "rtc-time", OBJECT(isadev),
                            "date", NULL);
#endif
  mc146818_rtc->base_year = base_year;
  mc146818_rtc->lost_tick_policy = LOST_TICK_POLICY_DISCARD;
  rtc_realizefn(mc146818_rtc);
  qdev_connect_gpio_out(&mc146818_rtc->gpio, 0, intercept_irq);
  assert(mc146818_rtc->irq == intercept_irq);

  return mc146818_rtc;
}

#ifdef BMBT
static Property mc146818rtc_properties[] = {
    DEFINE_PROP_INT32("base_year", RTCState, base_year, 1980),
    DEFINE_PROP_LOSTTICKPOLICY("lost_tick_policy", RTCState, lost_tick_policy,
                               LOST_TICK_POLICY_DISCARD),
    DEFINE_PROP_END_OF_LIST(),
};
#endif

void rtc_resetdev() {
  // RTCState *s = MC146818_RTC(d);
  RTCState *s = &__mc146818_rtc;

  /* Reason: VM do suspend self will set 0xfe
   * Reset any values other than 0xfe(Guest suspend case) */
  if (s->cmos_data[0x0f] != 0xfe) {
    s->cmos_data[0x0f] = 0x00;
  }
}

#ifdef BMBT
static void rtc_class_initfn(ObjectClass *klass, void *data) {
  DeviceClass *dc = DEVICE_CLASS(klass);

  dc->realize = rtc_realizefn;
  dc->reset = rtc_resetdev;
  dc->vmsd = &vmstate_rtc;
  dc->props = mc146818rtc_properties;
}

static const TypeInfo mc146818rtc_info = {
    .name = TYPE_MC146818_RTC,
    .parent = TYPE_ISA_DEVICE,
    .instance_size = sizeof(RTCState),
    .class_init = rtc_class_initfn,
};

static void mc146818rtc_register_types(void) {
  type_register_static(&mc146818rtc_info);
}

type_init(mc146818rtc_register_types)
#endif
