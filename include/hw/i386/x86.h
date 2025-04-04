#ifndef X86_H_L9VT3E8K
#define X86_H_L9VT3E8K

#include "../../../src/hw/i386/fw_cfg.h"
#include "../boards.h"
#include "../irq.h"
#include "../nvram/fw_cfg.h"
#include <hw/rtc/mc146818rtc.h>

typedef struct {
  /*< private >*/
  MachineClass parent_class;

  /*< public >*/

  /* TSC rate migration: */
  bool save_tsc_khz;
  /* Enables contiguous-apic-ID mode */
  bool compat_apic_id_mode;
} X86MachineClass;

typedef struct {
  /*< private >*/
  MachineState parent_obj;
  X86MachineClass *x86mc;

  /*< public >*/
  /* Pointers to devices and objects: */
  RTCState *rtc;
#ifdef BMBT
  GMappedFile *initrd_mapped_file;
#endif
  qemu_irq *gsi;
  FWCfgState *fw_cfg;

  /* Configuration options: */
  uint64_t max_ram_below_4g;

  /* RAM information (sizes, addresses, configuration): */
  ram_addr_t below_4g_mem_size, above_4g_mem_size;

  /* CPU and apic information: */
  bool apic_xrupt_override;
  unsigned apic_id_limit;
  uint16_t boot_cpus;
  unsigned smp_dies;

  /*
   * Address space used by IOAPIC device. All IOAPIC interrupts
   * will be translated to MSI messages in the address space.
   */
  AddressSpace *ioapic_as;
} X86MachineState;

static inline X86MachineClass *X86_MACHINE_GET_CLASS(X86MachineState *x86ms) {
  assert(x86ms->x86mc != NULL);
  return x86ms->x86mc;
}

static inline void X86_MACHINE_SET_CLASS(X86MachineState *x86ms,
                                         X86MachineClass *x86mc) {
  assert(x86mc != NULL);
  x86ms->x86mc = x86mc;
}

void x86_cpus_init(X86MachineState *pcms, int default_cpu_version);

void x86_machine_class_init(X86MachineClass *x86mc);
void x86_machine_initfn(X86MachineState *x86ms);

void x86_load_linux(X86MachineState *x86ms, FWCfgState *fw_cfg,
                    int acpi_data_size, bool pvh_enabled,
                    bool linuxboot_dma_enabled);

#endif /* end of include guard: X86_H_L9VT3E8K */
