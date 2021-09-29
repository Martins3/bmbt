#ifndef PC_H_0VFJYDT2
#define PC_H_0VFJYDT2

#include "../../exec/hwaddr.h"
#include "../../qemu/notify.h"
#include "../../sysemu/numa.h"
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

#define kvm_enabled() 0
#define kvm_irqchip_in_kernel() (false)
#define whpx_enabled() 0

static inline bool kvm_allows_irq0_override(void) { return 1; }
static inline bool xen_enabled(void) { return false; }

// copied from target/i386/sev-stub.c
bool sev_enabled(void) { return false; }
uint64_t sev_get_me_mask(void) { return ~0; }
uint32_t sev_get_cbit_position(void) { return 0; }
uint32_t sev_get_reduced_phys_bits(void) { return 0; }
char *sev_get_launch_measurement(void) { return NULL; }

// FIXME copied from include/hw/acpi/pc-hotplug.h
// maybe we will review the cpu hotplug and memory hotplug later
#define ACPI_MEMORY_HOTPLUG_BASE 0x0a00

/**
 * PCMachineState:
 * @acpi_dev: link to ACPI PM device that performs ACPI hotplug handling
 * @boot_cpus: number of present VCPUs
 * @smp_dies: number of dies per one package
 */
typedef struct PCMachineState {
  /*< private >*/
  X86MachineState parent_obj;
  struct PCMachineClass *pcmc;

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

#define PC_MACHINE_GET_CLASS(pcms) pcms->pcmc
#define PC_MACHINE(mc)                                                         \
  ({                                                                           \
    MachineState *tmp = mc;                                                    \
    (PCMachineState *)tmp;                                                     \
  })

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

#define compare_types(T1, T2) _Generic(((T1){0}), T2 : 1, default : 0)

static inline MachineState *X86_TO_MACHINE(X86MachineState *x) {
  return (MachineState *)x;
}

static inline X86MachineState *MS_TO_X86_MACHINE(const MachineState *x) {
  return (X86MachineState *)x;
}

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

#define MACHINE_CLASS(pcmc)                                                    \
  ({                                                                           \
    PCMachineClass *tmp = pcmc;                                                \
    (MachineClass *)tmp;                                                       \
  })

#define X86_MACHINE_CLASS(mc)                                                  \
  ({                                                                           \
    MachineClass *tmp = mc;                                                    \
    (X86MachineClass *)tmp;                                                    \
  })

#define PC_MACHINE_CLASS(m)                                                    \
  ({                                                                           \
    MachineClass *tmp = m;                                                     \
    (PCMachineClass *)tmp;                                                     \
  })
/* pc_sysfw.c */
static inline void pc_system_flash_create(PCMachineState *pcms) {}
void pc_system_firmware_init(PCMachineState *pcms, MemoryRegion *rom_memory);

void pc_guest_info_init(PCMachineState *pcms);

void pc_memory_init(PCMachineState *pcms, MemoryRegion *system_memory,
                    MemoryRegion *rom_memory, MemoryRegion **ram_memory);

GSIState *pc_gsi_create(qemu_irq **irqs, bool pci_enabled);

void pc_i8259_create(qemu_irq *i8259_irqs);
void ioapic_init_gsi(GSIState *gsi_state, const char *parent_name);

#endif /* end of include guard: PC_H_0VFJYDT2 */
