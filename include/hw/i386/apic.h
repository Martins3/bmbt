#ifndef APIC_H_JOIQEZ9X
#define APIC_H_JOIQEZ9X
#include "../../exec/cpu-all.h"
#include "apic_internal.h"

// @todo why they are useless?
// void apic_deliver_irq(uint8_t dest, uint8_t dest_mode, uint8_t delivery_mode, uint8_t vector_num, uint8_t trigger_mode);
int apic_accept_pic_intr(APICCommonState *s);
void apic_deliver_pic_intr(APICCommonState *s, int level);
// void apic_deliver_nmi(DeviceState *d);
int apic_get_interrupt(APICCommonState *s);
// void apic_reset_irq_delivered(void);
// int apic_get_irq_delivered(void);
  void cpu_set_apic_base(APICCommonState *s, uint64_t val);
  uint64_t cpu_get_apic_base(APICCommonState *s);
  void cpu_set_apic_tpr(APICCommonState *s, uint8_t val);
  uint8_t cpu_get_apic_tpr(APICCommonState *s);
  void apic_init_reset(APICCommonState *s);
  void apic_sipi(APICCommonState *s);
  void apic_poll_irq(APICCommonState *d);
  void apic_designate_bsp(APICCommonState *d, bool bsp);
// int apic_get_highest_priority_irr(DeviceState *dev);

/* pc.c */
APICCommonState *cpu_get_current_apic(void);

#endif /* end of include guard: APIC_H_JOIQEZ9X */
