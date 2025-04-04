#ifndef MC146818RTC_H_IJIALN5Y
#define MC146818RTC_H_IJIALN5Y
#include <exec/memory.h>
#include <hw/irq.h>
#include <qemu/notify.h>
#include <qemu/timer.h>

typedef enum LostTickPolicy {
  LOST_TICK_POLICY_DISCARD,
  LOST_TICK_POLICY_DELAY,
  LOST_TICK_POLICY_SLEW,
  LOST_TICK_POLICY__MAX,
} LostTickPolicy;

typedef struct RTCState {
  // ISADevice parent_obj;

  MemoryRegion io;
  MemoryRegion coalesced_io;
  uint8_t cmos_data[128];
  uint8_t cmos_index;
  int32_t base_year;
  uint64_t base_rtc;
  uint64_t last_update;
  int64_t offset;
  qemu_irq irq;
  int it_shift;
  /* periodic timer */
  QEMUTimer *periodic_timer;
  int64_t next_periodic_time;
  /* update-ended timer */
  QEMUTimer *update_timer;
  uint64_t next_alarm_time;
  uint16_t irq_reinject_on_ack_count;
  uint32_t irq_coalesced;
  uint32_t period;
  QEMUTimer *coalesced_timer;
  Notifier clock_reset_notifier;
  LostTickPolicy lost_tick_policy;
  Notifier suspend_notifier;
  QLIST_ENTRY(RTCState) link;

  GPIOList gpio;
} RTCState;

#define RTC_ISA_IRQ 8

RTCState *mc146818_rtc_init(int base_year, qemu_irq intercept_irq);
void rtc_set_memory(RTCState *dev, int addr, int val);
int rtc_get_memory(RTCState *dev, int addr);

#endif /* end of include guard: MC146818RTC_H_IJIALN5Y */
