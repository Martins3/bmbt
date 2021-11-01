#ifndef MC146818RTC_REGS_H_BAVDOJPR
#define MC146818RTC_REGS_H_BAVDOJPR

#include "../../../src/tcg/glib_stub.h"
#include <stdint.h>

#define RTC_SECONDS 0
#define RTC_SECONDS_ALARM 1
#define RTC_MINUTES 2
#define RTC_MINUTES_ALARM 3
#define RTC_HOURS 4
#define RTC_HOURS_ALARM 5
#define RTC_ALARM_DONT_CARE 0xC0

#define RTC_DAY_OF_WEEK 6
#define RTC_DAY_OF_MONTH 7
#define RTC_MONTH 8
#define RTC_YEAR 9

#define RTC_REG_A 10
#define RTC_REG_B 11
#define RTC_REG_C 12
#define RTC_REG_D 13

/* PC cmos mappings */
#define RTC_CENTURY 0x32
#define RTC_IBM_PS2_CENTURY_BYTE 0x37

#define REG_A_UIP 0x80

#define REG_B_SET 0x80
#define REG_B_PIE 0x40
#define REG_B_AIE 0x20
#define REG_B_UIE 0x10
#define REG_B_SQWE 0x08
#define REG_B_DM 0x04
#define REG_B_24H 0x02

#define REG_C_UF 0x10
#define REG_C_IRQF 0x80
#define REG_C_PF 0x40
#define REG_C_AF 0x20
#define REG_C_MASK 0x70

static inline uint32_t periodic_period_to_clock(int period_code) {
  if (!period_code) {
    return 0;
  }

  if (period_code <= 2) {
    period_code += 7;
  }
  /* period in 32 Khz cycles */
  return 1 << (period_code - 1);
}

#define RTC_CLOCK_RATE 32768

static inline int64_t periodic_clock_to_ns(int64_t clocks) {
  // in qemu, this function is never called if reach here, which means something
  // weird happened and more investigation is needed
  g_assert_not_reached();
  // return muldiv64(clocks, NANOSECONDS_PER_SECOND, RTC_CLOCK_RATE);
}

#endif /* end of include guard: MC146818RTC_REGS_H_BAVDOJPR */
