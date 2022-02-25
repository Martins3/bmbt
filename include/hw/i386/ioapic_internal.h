#ifndef IOAPIC_INTERNAL_H_NKHBSHQ4
#define IOAPIC_INTERNAL_H_NKHBSHQ4

#include "../../exec/memory.h"
#include "../../qemu/error-report.h"
#include "../../qemu/notify.h"
#include "../../types.h"
#include "../irq.h"
#include "ioapic.h"
#include <hw/sysbus.h>
#include <qemu/timer.h>

#define MAX_IOAPICS 1

#define IOAPIC_LVT_DEST_SHIFT 56
#define IOAPIC_LVT_DEST_IDX_SHIFT 48
#define IOAPIC_LVT_MASKED_SHIFT 16
#define IOAPIC_LVT_TRIGGER_MODE_SHIFT 15
#define IOAPIC_LVT_REMOTE_IRR_SHIFT 14
#define IOAPIC_LVT_POLARITY_SHIFT 13
#define IOAPIC_LVT_DELIV_STATUS_SHIFT 12
#define IOAPIC_LVT_DEST_MODE_SHIFT 11
#define IOAPIC_LVT_DELIV_MODE_SHIFT 8

#define IOAPIC_LVT_MASKED (1 << IOAPIC_LVT_MASKED_SHIFT)
#define IOAPIC_LVT_TRIGGER_MODE (1 << IOAPIC_LVT_TRIGGER_MODE_SHIFT)
#define IOAPIC_LVT_REMOTE_IRR (1 << IOAPIC_LVT_REMOTE_IRR_SHIFT)
#define IOAPIC_LVT_POLARITY (1 << IOAPIC_LVT_POLARITY_SHIFT)
#define IOAPIC_LVT_DELIV_STATUS (1 << IOAPIC_LVT_DELIV_STATUS_SHIFT)
#define IOAPIC_LVT_DEST_MODE (1 << IOAPIC_LVT_DEST_MODE_SHIFT)
#define IOAPIC_LVT_DELIV_MODE (7 << IOAPIC_LVT_DELIV_MODE_SHIFT)

/* Bits that are read-only for IOAPIC entry */
#define IOAPIC_RO_BITS (IOAPIC_LVT_REMOTE_IRR | IOAPIC_LVT_DELIV_STATUS)
#define IOAPIC_RW_BITS (~(uint64_t)IOAPIC_RO_BITS)

#define IOAPIC_TRIGGER_EDGE 0
#define IOAPIC_TRIGGER_LEVEL 1

/*io{apic,sapic} delivery mode*/
#define IOAPIC_DM_FIXED 0x0
#define IOAPIC_DM_LOWEST_PRIORITY 0x1
#define IOAPIC_DM_PMI 0x2
#define IOAPIC_DM_NMI 0x4
#define IOAPIC_DM_INIT 0x5
#define IOAPIC_DM_SIPI 0x6
#define IOAPIC_DM_EXTINT 0x7
#define IOAPIC_DM_MASK 0x7

#define IOAPIC_VECTOR_MASK 0xff

#define IOAPIC_IOREGSEL 0x00
#define IOAPIC_IOWIN 0x10
#define IOAPIC_EOI 0x40

#define IOAPIC_REG_ID 0x00
#define IOAPIC_REG_VER 0x01
#define IOAPIC_REG_ARB 0x02
#define IOAPIC_REG_REDTBL_BASE 0x10
#define IOAPIC_ID 0x00

#define IOAPIC_ID_SHIFT 24
#define IOAPIC_ID_MASK 0xf

#define IOAPIC_VER_ENTRIES_SHIFT 16

typedef struct IOAPICCommonState IOAPICCommonState;

typedef struct IOAPICCommonClass {
  // SysBusDeviceClass parent_class;

  void (*realize)(IOAPICCommonState *s);
  // DeviceUnrealize unrealize;
  void (*pre_save)(IOAPICCommonState *s);
  void (*post_load)(IOAPICCommonState *s);
} IOAPICCommonClass;

struct IOAPICCommonState {
  SysBusDevice busdev;
  GPIOList gpio;

  IOAPICCommonClass *icc;
  MemoryRegion io_memory;
  uint8_t id;
  uint8_t ioregsel;
  uint32_t irr;
  uint64_t ioredtbl[IOAPIC_NUM_PINS];
  Notifier machine_done;
  uint8_t version;
  uint64_t irq_count[IOAPIC_NUM_PINS];
  int irq_level[IOAPIC_NUM_PINS];
  int irq_eoi[IOAPIC_NUM_PINS];
  QEMUTimer *delayed_ioapic_service_timer;
};

static inline SysBusDevice *IOAPIC_SYS_BUS_DEVICE(IOAPICCommonState *s) {
  return (SysBusDevice *)s;
}

static inline IOAPICCommonClass *
IOAPIC_COMMON_GET_CLASS(IOAPICCommonState *is) {
  bmbt_check(is->icc != NULL);
  return is->icc;
}

static inline void IOAPIC_COMMON_SET_CLASS(IOAPICCommonState *is,
                                           IOAPICCommonClass *icc) {
  bmbt_check(icc != NULL);
  is->icc = icc;
}

// QOM
IOAPICCommonState *QOM_ioapic_init();
void ioapic_common_class_init(IOAPICCommonClass *c);
void ioapic_reset_common(IOAPICCommonState *s);
void ioapic_common_realize(IOAPICCommonState *s);

#endif /* end of include guard: IOAPIC_INTERNAL_H_NKHBSHQ4 */
