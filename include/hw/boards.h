#ifndef BOARDS_H_ANEGSNX6
#define BOARDS_H_ANEGSNX6
#include "../exec/cpu-common.h"
#include "../hw/core/cpu.h"
#include "../sysemu/numa.h"
#include <stdbool.h>

typedef struct MachineState MachineState;
typedef struct MachineClass MachineClass;

/**
 * CPUArchId:
 * @arch_id - architecture-dependent CPU ID of present or possible CPU
 * @cpu - pointer to corresponding CPU object if it's present on NULL otherwise
 * @type - QOM class name of possible @cpu object
 * @props - CPU object properties, initialized by board
 * #vcpus_count - number of threads provided by @cpu object
 */
typedef struct CPUArchId {
  uint64_t arch_id;
  int64_t vcpus_count;
  // FIXME
  // CpuInstanceProperties props;
  // [interface 20]
  CPUState *cpu;
  const char *type;
} CPUArchId;

/**
 * CPUArchIdList:
 * @len - number of @CPUArchId items in @cpus array
 * @cpus - array of present or possible CPUs for current machine configuration
 */
typedef struct {
  int len;
  CPUArchId cpus[0];
} CPUArchIdList;

/**
 * DeviceMemoryState:
 * @base: address in guest physical address space where the memory
 * address space for memory devices starts
 * @mr: address space container for memory devices
 */
typedef struct DeviceMemoryState {
  hwaddr base;
} DeviceMemoryState;

/**
 * MachineClass:
 * @deprecation_reason: If set, the machine is marked as deprecated. The
 *    string should provide some clear information about what to use instead.
 * @max_cpus: maximum number of CPUs supported. Default: 1
 * @min_cpus: minimum number of CPUs supported. Default: 1
 * @default_cpus: number of CPUs instantiated if none are specified. Default: 1
 * @get_hotplug_handler: this function is called during bus-less
 *    device hotplug. If defined it returns pointer to an instance
 *    of HotplugHandler object, which handles hotplug operation
 *    for a given @dev. It may return NULL if @dev doesn't require
 *    any actions to be performed by hotplug handler.
 * @cpu_index_to_instance_props:
 *    used to provide @cpu_index to socket/core/thread number mapping, allowing
 *    legacy code to perform maping from cpu_index to topology properties
 *    Returns: tuple of socket/core/thread ids given cpu_index belongs to.
 *    used to provide @cpu_index to socket number mapping, allowing
 *    a machine to group CPU threads belonging to the same socket/package
 *    Returns: socket number given cpu_index belongs to.
 * @hw_version:
 *    Value of QEMU_VERSION when the machine was added to QEMU.
 *    Set only by old machines because they need to keep
 *    compatibility on code that exposed QEMU_VERSION to guests in
 *    the past (and now use qemu_hw_version()).
 * @possible_cpu_arch_ids:
 *    Returns an array of @CPUArchId architecture-dependent CPU IDs
 *    which includes CPU IDs for present and possible to hotplug CPUs.
 *    Caller is responsible for freeing returned list.
 * @get_default_cpu_node_id:
 *    returns default board specific node_id value for CPU slot specified by
 *    index @idx in @ms->possible_cpus[]
 * @has_hotpluggable_cpus:
 *    If true, board supports CPUs creation with -device/device_add.
 * @default_cpu_type:
 *    specifies default CPU_TYPE, which will be used for parsing target
 *    specific features and for creating CPUs if CPU name wasn't provided
 *    explicitly at CLI
 * @minimum_page_bits:
 *    If non-zero, the board promises never to create a CPU with a page size
 *    smaller than this, so QEMU can use a more efficient larger page
 *    size than the target architecture's minimum. (Attempting to create
 *    such a CPU will fail.) Note that changing this is a migration
 *    compatibility break for the machine.
 * @ignore_memory_transaction_failures:
 *    If this is flag is true then the CPU will ignore memory transaction
 *    failures which should cause the CPU to take an exception due to an
 *    access to an unassigned physical address; the transaction will instead
 *    return zero (for a read) or be ignored (for a write). This should be
 *    set only by legacy board models which rely on the old RAZ/WI behaviour
 *    for handling devices that QEMU does not yet model. New board models
 *    should instead use "unimplemented-device" for all memory ranges where
 *    the guest will attempt to probe for a device that QEMU doesn't
 *    implement and a stub device is required.
 * @kvm_type:
 *    Return the type of KVM corresponding to the kvm-type string option or
 *    computed based on other criteria such as the host kernel capabilities.
 * @numa_mem_supported:
 *    true if '--numa node.mem' option is supported and false otherwise
 * @smp_parse:
 *    The function pointer to hook different machine specific functions for
 *    parsing "smp-opts" from QemuOpts to MachineState::CpuTopology and more
 *    machine specific topology fields, such as smp_dies for PCMachine.
 * @hotplug_allowed:
 *    If the hook is provided, then it'll be called for each device
 *    hotplug to check whether the device hotplug is allowed.  Return
 *    true to grant allowance or false to reject the hotplug.  When
 *    false is returned, an error must be set to show the reason of
 *    the rejection.  If the hook is not provided, all hotplug will be
 *    allowed.
 */
struct MachineClass {
  /*< private >*/
  // ObjectClass parent_class;
  /*< public >*/

  const char *family; /* NULL iff @name identifies a standalone machtype */
  char *name;
  const char *alias;
  const char *desc;
  const char *deprecation_reason;

  void (*init)(MachineState *state);
  void (*reset)(MachineState *state);
  void (*wakeup)(MachineState *state);
  // void (*hot_add_cpu)(MachineState *state, const int64_t id, Error **errp);
  int (*kvm_type)(MachineState *machine, const char *arg);
  // void (*smp_parse)(MachineState *ms, QemuOpts *opts);

  // BlockInterfaceType block_default_type;
  int units_per_default_bus;
  int max_cpus;
  int min_cpus;
  int default_cpus;
  unsigned int no_serial : 1, no_parallel : 1, no_floppy : 1, no_cdrom : 1,
      no_sdcard : 1, pci_allow_0_address : 1, legacy_fw_cfg_order : 1;
  int is_default;
  const char *default_machine_opts;
  const char *default_boot_order;
  const char *default_display;
  // GPtrArray *compat_props;
  const char *hw_version;
  // ram_addr_t default_ram_size;
  const char *default_cpu_type;
  bool default_kernel_irqchip_split;
  bool option_rom_has_mr;
  bool rom_file_has_mr;
  int minimum_page_bits;
  bool has_hotpluggable_cpus;
  bool ignore_memory_transaction_failures;
  int numa_mem_align_shift;
  const char **valid_cpu_types;
  // strList *allowed_dynamic_sysbus_devices;
  bool auto_enable_numa_with_memhp;
  // void (*numa_auto_assign_ram)(MachineClass *mc, NodeInfo *nodes, int
  // nb_nodes, ram_addr_t size);
  bool ignore_boot_device_suffixes;
  bool smbus_no_migration_support;
  bool nvdimm_supported;
  bool numa_mem_supported;
  bool auto_enable_numa;

  // HotplugHandler *(*get_hotplug_handler)(MachineState *machine,
  // DeviceState *dev);
  // bool (*hotplug_allowed)(MachineState *state, DeviceState *dev, Error
  // **errp); CpuInstanceProperties (*cpu_index_to_instance_props)(MachineState
  // *machine, unsigned cpu_index);
  // const CPUArchIdList *(*possible_cpu_arch_ids)(MachineState *machine);
  // int64_t (*get_default_cpu_node_id)(const MachineState *ms, int idx);
};

/**
 * MachineState:
 */
struct MachineState {
  /*< private >*/
  // Object parent_obj;
  // Notifier sysbus_notifier;
  MachineClass *mc;

  /*< public >*/

  char *accel;
  bool kernel_irqchip_allowed;
  bool kernel_irqchip_required;
  bool kernel_irqchip_split;
  int kvm_shadow_mem;
  char *dtb;
  char *dumpdtb;
  int phandle_start;
  char *dt_compatible;
  bool dump_guest_core;
  bool mem_merge;
  bool usb;
  bool usb_disabled;
  bool igd_gfx_passthru;
  char *firmware;
  bool iommu;
  bool suppress_vmdesc;
  bool enforce_config_section;
  bool enable_graphics;
  char *memory_encryption;
  DeviceMemoryState *device_memory;

  ram_addr_t ram_size;
  ram_addr_t maxram_size;
  uint64_t ram_slots;
  const char *boot_order;
  char *kernel_filename;
  char *kernel_cmdline;
  char *initrd_filename;
  const char *cpu_type;
  // AccelState *accelerator;
  CPUArchIdList *possible_cpus;
  // CpuTopology smp;
  struct NVDIMMState *nvdimms_state;
  struct NumaState *numa_state;
};

static inline MachineState *qdev_get_machine() { return NULL; }

#define MACHINE_GET_CLASS(machine) machine->mc

#endif /* end of include guard: BOARDS_H_ANEGSNX6 */
