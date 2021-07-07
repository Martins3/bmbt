#ifndef APIC_H_JOIQEZ9X
#define APIC_H_JOIQEZ9X
#include "../../exec/cpu-all.h"

// FIXME I don't know how to handle apic yet
/* apic.c */
void cpu_report_tpr_access(CPUX86State *env, TPRAccess access);
void apic_handle_tpr_access_report(DeviceState *d, target_ulong ip,
                                   TPRAccess access);
void apic_poll_irq(DeviceState *d);
void apic_init_reset(DeviceState *s);
void apic_sipi(DeviceState *s);
void cpu_set_apic_base(DeviceState *s, uint64_t val);
uint64_t cpu_get_apic_base(DeviceState *s);
void cpu_set_apic_tpr(DeviceState *s, uint8_t val);
uint8_t cpu_get_apic_tpr(DeviceState *s);
void apic_designate_bsp(DeviceState *d, bool bsp);

#endif /* end of include guard: APIC_H_JOIQEZ9X */
