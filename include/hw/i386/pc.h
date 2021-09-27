#ifndef PC_H_0VFJYDT2
#define PC_H_0VFJYDT2

#include "../../exec/hwaddr.h"
#include "../../qemu/notify.h"
#include "../../types.h"
#include "../irq.h"
#include "../isa/isa.h"
#include "ioapic.h"
#include "x86.h"

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

static inline bool kvm_allows_irq0_override(void) { return 1; }

/**
 * PCMachineState:
 * @acpi_dev: link to ACPI PM device that performs ACPI hotplug handling
 * @boot_cpus: number of present VCPUs
 * @smp_dies: number of dies per one package
 */
typedef struct PCMachineState {
  /*< private >*/
  X86MachineState parent_obj;

  /* <public> */

  /* State for other subsystems/APIs: */
  Notifier machine_done;

#ifdef NEED_LATER
  /* Pointers to devices and objects: */
  HotplugHandler *acpi_dev;
  PCIBus *bus;
  I2CBus *smbus;
  PFlashCFI01 *flash[2];

  /* Configuration options: */
  OnOffAuto vmport;
  OnOffAuto smm;

#endif

  bool acpi_build_enabled;
  bool smbus_enabled;
  bool sata_enabled;
  bool pit_enabled;

  /* NUMA information: */
  uint64_t numa_nodes;
  uint64_t *node_mem;

  /* ACPI Memory hotplug IO base address */
  hwaddr memhp_io_base;
} PCMachineState;

/**
 * PCMachineClass:
 *
 * Compat fields:
 *
 * @enforce_aligned_dimm: check that DIMM's address/size is aligned by
 *                        backend's alignment value if provided
 * @acpi_data_size: Size of the chunk of memory at the top of RAM
 *                  for the BIOS ACPI tables and other BIOS
 *                  datastructures.
 * @gigabyte_align: Make sure that guest addresses aligned at
 *                  1Gbyte boundaries get mapped to host
 *                  addresses aligned at 1Gbyte boundaries. This
 *                  way we can use 1GByte pages in the host.
 *
 */
typedef struct PCMachineClass {
  /*< private >*/
  X86MachineClass parent_class;

  /*< public >*/

  /* Device configuration: */
  bool pci_enabled;
  bool kvmclock_enabled;
  const char *default_nic_model;

  /* Compat options: */

  /* Default CPU model version.  See x86_cpu_set_default_version(). */
  int default_cpu_version;

  /* ACPI compat: */
  bool has_acpi_build;
  bool rsdp_in_ram;
  int legacy_acpi_table_size;
  unsigned acpi_data_size;
  bool do_not_add_smb_acpi;

  /* SMBIOS compat: */
  bool smbios_defaults;
  bool smbios_legacy_mode;
  bool smbios_uuid_encoded;

  /* RAM / address space compat: */
  bool gigabyte_align;
  bool has_reserved_memory;
  bool enforce_aligned_dimm;
  bool broken_reserved_end;

  /* generate legacy CPU hotplug AML */
  bool legacy_cpu_hotplug;

  /* use DMA capable linuxboot option rom */
  bool linuxboot_dma_enabled;

  /* use PVH to load kernels that support this feature */
  bool pvh_enabled;
} PCMachineClass;

// FIXME what if pcms is not PCMachineState ?
#define MACHINE(pcms)                                                          \
  ({                                                                           \
    PCMachineState *tmp = pcms;                                                \
    (MachineState *)pcms;                                                      \
  })

#define X86_MACHINE(pcms)                                                      \
  ({                                                                           \
    PCMachineState *tmp = pcms;                                                \
    (X86MachineState *)tmp;                                                    \
  })

#endif /* end of include guard: PC_H_0VFJYDT2 */
