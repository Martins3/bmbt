#ifndef PC_H_0VFJYDT2
#define PC_H_0VFJYDT2

#include "../irq.h"
#include "../isa/isa.h"
#include "ioapic.h"

/* Global System Interrupts */

#define GSI_NUM_PINS IOAPIC_NUM_PINS

typedef struct GSIState {
  qemu_irq i8259_irq[ISA_NUM_IRQS];
  qemu_irq ioapic_irq[IOAPIC_NUM_PINS];
} GSIState;

#define kvm_pit_in_kernel() 0
#define kvm_pic_in_kernel() 0
#define kvm_ioapic_in_kernel() 0

#define kvm_irqchip_in_kernel() (false)

#endif /* end of include guard: PC_H_0VFJYDT2 */
