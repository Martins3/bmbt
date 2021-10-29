#ifndef IOAPIC_H_TN4O6XW8
#define IOAPIC_H_TN4O6XW8

#define IOAPIC_NUM_PINS 24
#define IO_APIC_DEFAULT_ADDRESS 0xfec00000

#define TYPE_IOAPIC "ioapic"
#include "ioapic_internal.h"

void ioapic_eoi_broadcast(int vector);


#endif /* end of include guard: IOAPIC_H_TN4O6XW8 */
