#ifndef X86_H_L9VT3E8K
#define X86_H_L9VT3E8K

#include "../../src/hw/i386/fw_cfg.h"
#include "../boards.h"
#include "../nvram/fw_cfg.h"


typedef struct {
  /*< private >*/
  MachineClass parent;

  /*< public >*/

  /* TSC rate migration: */
  bool save_tsc_khz;
  /* Enables contiguous-apic-ID mode */
  bool compat_apic_id_mode;
} X86MachineClass;

typedef struct {
  /*< private >*/
  MachineState parent;

  /*< public >*/

#ifdef NEED_LATER
  /* Pointers to devices and objects: */
  ISADevice *rtc;
  qemu_irq *gsi;
  GMappedFile *initrd_mapped_file;
#endif
  FWCfgState *fw_cfg;

  /* Configuration options: */
  uint64_t max_ram_below_4g;

  /* RAM information (sizes, addresses, configuration): */
  ram_addr_t below_4g_mem_size, above_4g_mem_size;

  /* CPU and apic information: */
  bool apic_xrupt_override; // FIXME what does this mean ?
  unsigned apic_id_limit;
  uint16_t boot_cpus;
  unsigned smp_dies;

  /*
   * Address space used by IOAPIC device. All IOAPIC interrupts
   * will be translated to MSI messages in the address space.
   */
  AddressSpace *ioapic_as;
} X86MachineState;

void x86_bios_rom_init(MemoryRegion *rom_memory, bool isapc_ram_fw);

#endif /* end of include guard: X86_H_L9VT3E8K */
