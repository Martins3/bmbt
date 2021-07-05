# QEMU 启动代码

## 问题
- [ ] qdev_device_add 是做什么的
```
huxueshi:qdev_device_add isa-debugcon
huxueshi:qdev_device_add nvme
huxueshi:qdev_device_add virtio-9p-pci
```

- [ ] object_property_add_alias 是做啥的呀

## 整体
启动，总体划分三个部分：
1. x86_cpus_init ：进行 CPU 相关的初始化
2. pc_memory_init : 进行内存初始化
3. 设备的初始化
  - pci 相关的组织比较麻烦
4. machine 的初始化
   - 主要分布于 pc.c, 在 pc_piix.c 中只是定义了 pc_init1 而已

- CPUX86State : 这是 X86CPU 的成员，不是指针哦

文件内容的基本分析:
| file              | 行数 | 内容分析                                               |
|-------------------|------|--------------------------------------------------------|
| hw/i386/x86.c     | 1300 | cpu_hotplug / pic / x86_machine_class_init             |
| hw/i386/pc.c      | 1700 | 处理 Machine 相关的初始化，例如 hpet, vga 之类的       |
| target/i386/cpu.c | 7000 | X86CPU 相关，主要处理的都是 PC 的 feature 之类的       |
| hw/i386/pi_piix.c | 1000 | pc_init1 剩下的就是 DEFINE_I440FX_MACHINE 定义的东西了 |

定义的各种 type info
| variable          | location             | summary                                                              | instance_init                                                                     | class_init                                                                     |
|-------------------|----------------------|----------------------------------------------------------------------|-----------------------------------------------------------------------------------|--------------------------------------------------------------------------------|
| pc_machine_info   | hw/i386/pc.c         | pc_machine_initfn : 初始化一下 PCMachineState, pc_machine_class_init | 初始化了好多成员                                                                  |                                                                                |
| x86_machine_info  | hw/i386/x86.c        | 没啥东西                                                             |                                                                                   |                                                                                |
| machine_info      | hw/core/machine.c    |                                                                      | 没啥东西                                                                          | 注册 kernel initrd 之类的 property                                             |
| x86_cpu_type_info | target/i386/pc.c     |                                                                      | 调用一些 object_property_add_alias, x86_cpu_load_model 和 accel_cpu_instance_init | 注册了 x86_cpu_realizefn , 一些函数指针的初始化, vendor 之类的 property 初始化 |
| cpu_type_info     | hw/core/cpu-common.c | 并没有什么                                                           | 成员初始化，尤其是，list 之类的                                                   | 注册 cpu_common_parse_features 之类的                                          |
| device_type_info  | hw/core/qdev.c       | 处理 hotplugged 之类，这些抽象没有必要                               |                                                                                   |                                                                                |


到底初始化什么内容:
| item | necessary          | desc                                                                                     |
|------|--------------------|------------------------------------------------------------------------------------------|
| e820 | :heavy_check_mark: | - [ ] 为什么有了 acpi 还是需要 e820 啊，当使用增加了一个内存条，并没有说非要修改 acpi 啊 |
| apci | :x:                | - [ ] 在 QEMU 和 kernel 中间都存在 CONFIG_ACPI 的选项，也许暂时可以不去管                |
| pci  | :x:                | - [ ] `pcmc->pci_enabled`                                                                |
| cpu  | :x:                | - [ ] 到底初始化的是那几个结构体，和 tcg 耦合的结构体是谁                                                           |

非 PCI 设备枚举:
| Device       | parent              |
|--------------|---------------------|
| mc146818 rtc | TYPE_ISA_DEVICE     |
| i8254 pit    | TYPE_ISA_DEVICE     |
| i8257 dma    | TYPE_ISA_DEVICE     |
| hpet         | TYPE_SYS_BUS_DEVICE |
| i8259        | TYPE_ISA_DEVICE     |

从 type info 上可以轻易的看到一个设备是不是 TYPE_ISA_DEVICE 


- qemu_init : 这里面存在很长的参数解析的内容
  - qemu_create_machine(select_machine()) : select_machine 中获取 MachineClass
    - cpu_exec_init_all :
      - io_mem_init : 初始化 io_mem_unassigned, 但是实际上，这个 mr 永远都不会被使用
      - memory_map_init : 初始化 system_memory, 和 io_memory 这两个都是 container 并不会真正的分配的映射空间
    - page_size_init : 初始化之后的 softmmu 需要的内容
  - qmp_x_exit_preconfig
    - qemu_init_board
      - create_default_memdev : 比想象的复杂一点，是因为实际上，RAM 还可以是 filebased
      - machine_run_board_init
        - `machine_class->init` : DEFINE_I440FX_MACHINE 这个封装出来 pc_init_v6_1 来调用
          - pc_init1
            - x86_cpus_init
              - x86_cpu_new
                - qdev_realize : 经过 QOM 的 object_property 机制，最后调用到 device_set_realized :
                  - device_set_realized : 
                    - x86_cpu_realizefn : 需要重点分析一下
                      - cpu_list_add
                      - cpu_exec_realizefn
                        - accel_cpu_realizefn
                          - tcg_cpu_realizefn
                            - cpu_address_space_init 
                              - memory_listener_register
                                - tcg_commit
                        - tcg_exec_realizefn
                          - tcg_x86_init: 这是 CPUClass 上注册的函数，进行一些 tcg 相关的的初始化, 例如 regs
                      - x86_cpu_expand_features
                      - x86_cpu_filter_features
                      - mce_init : machine check exception, 初始化之后，那些 helper 就可以正确工作了, mce 参考[^2]
                      - qemu_init_vcpu : 创建执行线程
                      - x86_cpu_apic_realize 
                        - 通过 QOM 调用到 apic_common_realize
                           - 通过 QOM 调用 apic_realize
                        - 添加对应的 memory region
                      - X86CPUClass::parent_realize : 也就是 cpu_common_realizefn, 这里并没有做什么事情
          - pc_memory_init : 创建了两个mr alias，ram_below_4g 以及ram_above_4g，这两个mr分别指向ram的低 4g 以及高 4g 空间，这两个 alias 是挂在根 system_memory mr下面的
            - e820_add_entry
            - pc_system_firmware_init : pflash 参考 [pflash](#pflash)
              - x86_bios_rom_init : 不考虑 pflash, 这是唯一的调用者
                - memory_region_init_ram(bios, NULL, "pc.bios", bios_size, &error_fatal)
                - rom_add_file_fixed
                  - rom_add_file
                    - rom_insert
                    - add_boot_device_path
                - 还有两个 memory region 的操作, 将 bios 的后 128KB 映射到 ISA 空间，但是 bios 的大小是 256k 啊，其次，为什么映射到 pci 空间最上方啊
                  - [ ] map the last 128KB of the BIOS in ISA space
                  - [ ] map all the bios at the top of memory
            - memory_region_init_ram : 初始化 "pc.rom"
            - fw_cfg_arch_create : 创建 `FWCfgState *fw_cfg`, 并且初始化 e820 CPU 数量之类的参数, 具体参考 [fw_cfg](./fw_cfg.md)
            - rom_set_fw : 用从 fw_cfg_arch_create 返回的值初始化全局 fw_cfg
            - x86_load_linux : 如果指定了 kernel, 那么就从此处 load kernel
            - rom_add_option : 添加 rom 镜像，关于 rom 分析看 [loader](#loader)
          - pc_guest_info_init : 注册上 pc_machine_done 最后执行
          - smbios_set_defaults : 初始化一些 smbios 变量，为下一步制作 smbios table 打下基础
          - [ ] pc_gsi_create : 关于中断的事情可以重新看看狼书好好分析一下
          - i440fx_init : 只有 pcmc->pci_enabled 才会调用的
            - qdev_new("i440FX-pcihost") : 这当然会调用 i440fx_pcihost_initfn 和 i440fx_pcihost_class_init 之类的函数 
              - i440fx_pcihost_initfn : 初始化出来 0xcf8 0xcfb 这两个关键地址
            - pci_root_bus_new : 创建 PCIBus
              - [ ] PCIHostState 和分别是啥关系 ? host bridge 和 bus 的关系 ?
              - qbus_create("pci")
                - qbus_create("pci")
                  - pci_root_bus_init
                    - 一些常规的初始化
                    - pci_host_bus_register : 将 PCIHostState 挂载到一个全局的链表上
                - qbus_init
              - pci_root_bus_init
            - 处理 PCI 的地址空间的映射初始化
            - init_pam : https://wiki.qemu.org/Documentation/Platforms/PC
          - piix3_create
            - pci_create_simple_multifunction : 创建出来设备
            - 设置从 piix3 到 i440fx 的中断路由之类的事情
          - [ ] isa_bus_irqs
          - pc_i8259_create : 根据配置，存在多种选项
            - i8259_init
          - [ ] ioapic_init_gsi
          - [ ] pc_vga_init
            - pci_vga_init
            - isa_vga_init
          - pc_basic_device_init
            - ioport80_io 初始化
            - ioportF0_io 初始化
            - hpet 初始化 : hpet 不是
            - mc146818_rtc_init : 通过 QOM 调用 rtc_class_initfn 和 rtc_realizefn 之类的，进行 rtc 的初始化
            - i8254_pit_init
            - i8257_dma_init
            - pc_superio_init : https://en.wikipedia.org/wiki/Super_I/O
          - pc_nic_init : 网卡的初始化
          - pci_ide_create_devs
            - ide_drive_get
            - ide_create_drive
          - pc_cmos_init
            - 多次调用 rtc_set_memory 初始化 RTCState::cmos_data
          - piix4_pm_init : 当支持 acpi 的时候, 那么初始化电源管理
    - qemu_create_cli_devices
      - soundhw_init
      - parse_fw_cfg : 解析参数 -fw_cfg (Add named fw_cfg entry with contents from file file.)
      - usb_parse
      - device_init_func : 解析参数 -device 比如 nvme
    - qemu_machine_creation_done
      - qdev_machine_creation_done
        - notifier_list_notify : 通过 qemu_add_machine_init_done_notifier 的 references 可以很快的知道都注册了什么
          - pc_machine_done
            - [ ] x86_rtc_set_cpus_count : 神奇的机制，和 seabios 对称的看看
            - [ ] fw_cfg_add_extra_pci_roots 
            - [ ] acpi_setup
              - 依赖于 acpi 的 `x86ms->fw_cfg` 和 pcms->acpi_build_enabled, 否则都会失败
          - tcg_cpu_machine_done : 注册 smram 相关的工作
          - [ ] machine_init_notify

## CPUX86State 
使用 x86_cpu_reset 进行初始化

- [ ] 关于 reset 其实会进行两次, 真的有必要吗 ?

每一个 CPU 都会进行一次

- x86_cpu_realizefn
  - cpu_reset
    - device_cold_reset
      - resettable_reset
        - resettable_assert_reset
          - resettable_phase_hold
            - x86_cpu_reset

- qemu_init
  - qmp_x_exit_preconfig
    - qemu_machine_creation_done : 我们的老朋友啊
      - qdev_machine_creation_done
        - qemu_system_reset
          - pc_machine_reset
            - qemu_devices_reset
              - x86_cpu_machine_reset_cb
                - cpu_reset
                  - device_cold_reset
                    - resettable_assert_reset
                      - resettable_phase_hold
                        - device_transitional_reset
                          - x86_cpu_reset

## e820
- 信息是如何构造出来的
  - 在  pc_memory_init 调用两次 e820_add_entry, 分别添加 below_4g_mem_size 和 above_4g_mem_size
- 如何通知 guest 内核的 ?
  - 在 fw_cfg_arch_create 中添加 `etc/e820` 实现的

- [ ] 类似 pci 映射的 MMIO 空间的分配是 e820 负责的操作的吗 ?

## pflash
在 hw/i386/pc_sysfw.c 似乎主要处理的就是 pflash

关于 pflash 和 bios 的关系可以首先看看[^1], 但是目前不需要知道 pflash 也可以


## host cpu / qemu cpu
似乎 cpu 体系的最后，逐步到达 x86_cpu_type_info, 但是下面还是存在别的内容，其中的代码，直接

当使用上 tcg 的时候，是无法采用 host-x86_64-cpu 的, 当然 -cpu

- [ ] 如何确定是 32bit 还是 64bit cpu 初始化的区别

- 关于 cpu 中，其实还定义了 base 版本 和 max 版本

通过 x86_register_cpu_model_type 创建出来了这个一堆 TypeInfo, 其作用:
```c
static void x86_register_cpu_model_type(const char *name, X86CPUModel *model)
{
    g_autofree char *typename = x86_cpu_type_name(name);

    printf("huxueshi:%s %s\n", __FUNCTION__, typename);

    TypeInfo ti = {
        .name = typename,
        .parent = TYPE_X86_CPU,
        .class_init = x86_cpu_cpudef_class_init,
        .class_data = model,
    };

    type_register(&ti);
}
```


在 qemu_init 中进行 `current_machine->cpu_type` 的初始化, 
而 pc_machine_class_init 中进行选择 MachineClass::default_cpu_type

```c
static X86CPUDefinition builtin_x86_defs[] = {
    {
        .name = "qemu64",
        .level = 0xd,
        .vendor = CPUID_VENDOR_AMD,
        .family = 6,
        .model = 6,
        .stepping = 3,
        .features[FEAT_1_EDX] =
            PPRO_FEATURES |
            CPUID_MTRR | CPUID_CLFLUSH | CPUID_MCA |
            CPUID_PSE36,
        .features[FEAT_1_ECX] =
            CPUID_EXT_SSE3 | CPUID_EXT_CX16,
        .features[FEAT_8000_0001_EDX] =
            CPUID_EXT2_LM | CPUID_EXT2_SYSCALL | CPUID_EXT2_NX,
        .features[FEAT_8000_0001_ECX] =
            CPUID_EXT3_LAHF_LM | CPUID_EXT3_SVM,
        .xlevel = 0x8000000A,
        .model_id = "QEMU Virtual CPU version " QEMU_HW_VERSION,
    },
};
```
由此可见，重新第一出来的这些 type info 只是为了初始化主流体系而已

## 分析一下 TYPE_I440FX_PCI_HOST_BRIDGE
```c
static const TypeInfo i440fx_pcihost_info = {
    .name          = TYPE_I440FX_PCI_HOST_BRIDGE,
    .parent        = TYPE_PCI_HOST_BRIDGE,
    .instance_size = sizeof(I440FXState),
    .instance_init = i440fx_pcihost_initfn,
    .class_init    = i440fx_pcihost_class_init,
};
```

## BUS
- pci host bridge 和 pcibus 的关系?

```
#0  qbus_init (bus=0x55555608760c, parent=0x7fffffffd510, name=0x555555e65068 <object_initialize+99> "\220\311\303\363\017\036\372UH\211\345H\201\354\020\001") at ../hw
/core/bus.c:103
#1  0x0000555555e78fdb in qbus_create_inplace (bus=0x555556aacb60, size=120, typename=0x55555608760c "System", parent=0x0, name=0x5555560876dc "main-system-bus") at ../
hw/core/bus.c:158
#2  0x0000555555b03423 in main_system_bus_create () at ../hw/core/sysbus.c:346
#3  0x0000555555b03451 in sysbus_get_default () at ../hw/core/sysbus.c:354
#4  0x0000555555cd9c11 in qemu_create_machine (machine_class=0x5555569a3850) at ../softmmu/vl.c:2087
#5  0x0000555555cdd500 in qemu_init (argc=28, argv=0x7fffffffd7c8, envp=0x7fffffffd8b0) at ../softmmu/vl.c:3570
#6  0x000055555582e575 in main (argc=28, argv=0x7fffffffd7c8, envp=0x7fffffffd8b0) at ../softmmu/main.c:49
```

## init machine
- [ ] 注意, 这里拷贝的代码是 v6.0, 准备写代码的时候，将这个全部替换掉

#### MachineClass
```c
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
    ObjectClass parent_class;
    /*< public >*/

    const char *family; /* NULL iff @name identifies a standalone machtype */
    char *name;
    const char *alias;
    const char *desc;
    const char *deprecation_reason;

    void (*init)(MachineState *state);
    void (*reset)(MachineState *state);
    void (*wakeup)(MachineState *state);
    void (*hot_add_cpu)(MachineState *state, const int64_t id, Error **errp);
    int (*kvm_type)(MachineState *machine, const char *arg);
    void (*smp_parse)(MachineState *ms, QemuOpts *opts);

    BlockInterfaceType block_default_type;
    int units_per_default_bus;
    int max_cpus;
    int min_cpus;
    int default_cpus;
    unsigned int no_serial:1,
        no_parallel:1,
        no_floppy:1,
        no_cdrom:1,
        no_sdcard:1,
        pci_allow_0_address:1,
        legacy_fw_cfg_order:1;
    int is_default;
    const char *default_machine_opts;
    const char *default_boot_order;
    const char *default_display;
    GPtrArray *compat_props;
    const char *hw_version;
    ram_addr_t default_ram_size;
    const char *default_cpu_type;
    bool default_kernel_irqchip_split;
    bool option_rom_has_mr;
    bool rom_file_has_mr;
    int minimum_page_bits;
    bool has_hotpluggable_cpus;
    bool ignore_memory_transaction_failures;
    int numa_mem_align_shift;
    const char **valid_cpu_types;
    strList *allowed_dynamic_sysbus_devices;
    bool auto_enable_numa_with_memhp;
    void (*numa_auto_assign_ram)(MachineClass *mc, NodeInfo *nodes,
                                 int nb_nodes, ram_addr_t size);
    bool ignore_boot_device_suffixes;
    bool smbus_no_migration_support;
    bool nvdimm_supported;
    bool numa_mem_supported;
    bool auto_enable_numa;

    HotplugHandler *(*get_hotplug_handler)(MachineState *machine,
                                           DeviceState *dev);
    bool (*hotplug_allowed)(MachineState *state, DeviceState *dev,
                            Error **errp);
    CpuInstanceProperties (*cpu_index_to_instance_props)(MachineState *machine,
                                                         unsigned cpu_index);
    const CPUArchIdList *(*possible_cpu_arch_ids)(MachineState *machine);
    int64_t (*get_default_cpu_node_id)(const MachineState *ms, int idx);
};
```


#### X86MachineClass
```c
struct X86MachineClass {
    /*< private >*/
    MachineClass parent;

    /*< public >*/

    /* TSC rate migration: */
    bool save_tsc_khz;
    /* Enables contiguous-apic-ID mode */
    bool compat_apic_id_mode;
};
```

#### PCMachineClass
```c
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
struct PCMachineClass {
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
    int pci_root_uid;

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

    /* create kvmclock device even when KVM PV features are not exposed */
    bool kvmclock_create_always;
};
```
#### MachineState
```c
/**
 * MachineState:
 */
struct MachineState {
    /*< private >*/
    Object parent_obj;
    Notifier sysbus_notifier;

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
    uint64_t   ram_slots;
    const char *boot_order;
    char *kernel_filename;
    char *kernel_cmdline;
    char *initrd_filename;
    const char *cpu_type;
    AccelState *accelerator;
    CPUArchIdList *possible_cpus;
    CpuTopology smp;
    struct NVDIMMState *nvdimms_state;
    struct NumaState *numa_state;
};
```



#### X86MachineState
```c
typedef struct {
    /*< private >*/
    MachineState parent;

    /*< public >*/

    /* Pointers to devices and objects: */
    ISADevice *rtc;
    FWCfgState *fw_cfg;
    qemu_irq *gsi;
    GMappedFile *initrd_mapped_file;

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
```

#### PCMachineState
```c
/**
 * PCMachineState:
 * @acpi_dev: link to ACPI PM device that performs ACPI hotplug handling
 * @boot_cpus: number of present VCPUs
 * @smp_dies: number of dies per one package
 */
struct PCMachineState {
    /*< private >*/
    X86MachineState parent_obj;

    /* <public> */

    /* State for other subsystems/APIs: */
    Notifier machine_done;

    /* Pointers to devices and objects: */
    HotplugHandler *acpi_dev;
    PCIBus *bus;
    I2CBus *smbus;
    PFlashCFI01 *flash[2];

    /* Configuration options: */
    OnOffAuto vmport;
    OnOffAuto smm;

    bool acpi_build_enabled;
    bool smbus_enabled;
    bool sata_enabled;
    bool pit_enabled;

    /* NUMA information: */
    uint64_t numa_nodes;
    uint64_t *node_mem;

    /* ACPI Memory hotplug IO base address */
    hwaddr memhp_io_base;
};
```

## init cpu

#### DeviceState
```c
/**
 * DeviceState:
 * @realized: Indicates whether the device has been fully constructed.
 *
 * This structure should not be accessed directly.  We declare it here
 * so that it can be embedded in individual device state structures.
 */
struct DeviceState {
    /*< private >*/
    Object parent_obj;
    /*< public >*/

    const char *id;
    char *canonical_path;
    bool realized;
    bool pending_deleted_event;
    QemuOpts *opts;
    int hotplugged;
    bool allow_unplug_during_migration;
    BusState *parent_bus;
    QLIST_HEAD(, NamedGPIOList) gpios;
    QLIST_HEAD(, BusState) child_bus;
    int num_child_bus;
    int instance_id_alias;
    int alias_required_for_version;
};
```

#### CPUState
```c
/**
 * CPUState:
 * @cpu_index: CPU index (informative).
 * @cluster_index: Identifies which cluster this CPU is in.
 *   For boards which don't define clusters or for "loose" CPUs not assigned
 *   to a cluster this will be UNASSIGNED_CLUSTER_INDEX; otherwise it will
 *   be the same as the cluster-id property of the CPU object's TYPE_CPU_CLUSTER
 *   QOM parent.
 * @tcg_cflags: Pre-computed cflags for this cpu.
 * @nr_cores: Number of cores within this CPU package.
 * @nr_threads: Number of threads within this CPU.
 * @running: #true if CPU is currently running (lockless).
 * @has_waiter: #true if a CPU is currently waiting for the cpu_exec_end;
 * valid under cpu_list_lock.
 * @created: Indicates whether the CPU thread has been successfully created.
 * @interrupt_request: Indicates a pending interrupt request.
 * @halted: Nonzero if the CPU is in suspended state.
 * @stop: Indicates a pending stop request.
 * @stopped: Indicates the CPU has been artificially stopped.
 * @unplug: Indicates a pending CPU unplug request.
 * @crash_occurred: Indicates the OS reported a crash (panic) for this CPU
 * @singlestep_enabled: Flags for single-stepping.
 * @icount_extra: Instructions until next timer event.
 * @can_do_io: Nonzero if memory-mapped IO is safe. Deterministic execution
 * requires that IO only be performed on the last instruction of a TB
 * so that interrupts take effect immediately.
 * @cpu_ases: Pointer to array of CPUAddressSpaces (which define the
 *            AddressSpaces this CPU has)
 * @num_ases: number of CPUAddressSpaces in @cpu_ases
 * @as: Pointer to the first AddressSpace, for the convenience of targets which
 *      only have a single AddressSpace
 * @env_ptr: Pointer to subclass-specific CPUArchState field.
 * @icount_decr_ptr: Pointer to IcountDecr field within subclass.
 * @gdb_regs: Additional GDB registers.
 * @gdb_num_regs: Number of total registers accessible to GDB.
 * @gdb_num_g_regs: Number of registers in GDB 'g' packets.
 * @next_cpu: Next CPU sharing TB cache.
 * @opaque: User data.
 * @mem_io_pc: Host Program Counter at which the memory was accessed.
 * @kvm_fd: vCPU file descriptor for KVM.
 * @work_mutex: Lock to prevent multiple access to @work_list.
 * @work_list: List of pending asynchronous work.
 * @trace_dstate_delayed: Delayed changes to trace_dstate (includes all changes
 *                        to @trace_dstate).
 * @trace_dstate: Dynamic tracing state of events for this vCPU (bitmask).
 * @plugin_mask: Plugin event bitmap. Modified only via async work.
 * @ignore_memory_transaction_failures: Cached copy of the MachineState
 *    flag of the same name: allows the board to suppress calling of the
 *    CPU do_transaction_failed hook function.
 * @kvm_dirty_gfns: Points to the KVM dirty ring for this CPU when KVM dirty
 *    ring is enabled.
 * @kvm_fetch_index: Keeps the index that we last fetched from the per-vCPU
 *    dirty ring structure.
 *
 * State of one CPU core or thread.
 */
struct CPUState {
    /*< private >*/
    DeviceState parent_obj;
    /*< public >*/

    int nr_cores;
    int nr_threads;

    struct QemuThread *thread;
#ifdef _WIN32
    HANDLE hThread;
#endif
    int thread_id;
    bool running, has_waiter;
    struct QemuCond *halt_cond;
    bool thread_kicked;
    bool created;
    bool stop;
    bool stopped;

    /* Should CPU start in powered-off state? */
    bool start_powered_off;

    bool unplug;
    bool crash_occurred;
    bool exit_request;
    bool in_exclusive_context;
    uint32_t cflags_next_tb;
    /* updates protected by BQL */
    uint32_t interrupt_request;
    int singlestep_enabled;
    int64_t icount_budget;
    int64_t icount_extra;
    uint64_t random_seed;
    sigjmp_buf jmp_env;

    QemuMutex work_mutex;
    QSIMPLEQ_HEAD(, qemu_work_item) work_list;

    CPUAddressSpace *cpu_ases;
    int num_ases;
    AddressSpace *as;
    MemoryRegion *memory;

    void *env_ptr; /* CPUArchState */
    IcountDecr *icount_decr_ptr;

    /* Accessed in parallel; all accesses must be atomic */
    TranslationBlock *tb_jmp_cache[TB_JMP_CACHE_SIZE];

    struct GDBRegisterState *gdb_regs;
    int gdb_num_regs;
    int gdb_num_g_regs;
    QTAILQ_ENTRY(CPUState) node;

    /* ice debug support */
    QTAILQ_HEAD(, CPUBreakpoint) breakpoints;

    QTAILQ_HEAD(, CPUWatchpoint) watchpoints;
    CPUWatchpoint *watchpoint_hit;

    void *opaque;

    /* In order to avoid passing too many arguments to the MMIO helpers,
     * we store some rarely used information in the CPU context.
     */
    uintptr_t mem_io_pc;

    /* Only used in KVM */
    int kvm_fd;
    struct KVMState *kvm_state;
    struct kvm_run *kvm_run;
    struct kvm_dirty_gfn *kvm_dirty_gfns;
    uint32_t kvm_fetch_index;

    /* Used for events with 'vcpu' and *without* the 'disabled' properties */
    DECLARE_BITMAP(trace_dstate_delayed, CPU_TRACE_DSTATE_MAX_EVENTS);
    DECLARE_BITMAP(trace_dstate, CPU_TRACE_DSTATE_MAX_EVENTS);

    DECLARE_BITMAP(plugin_mask, QEMU_PLUGIN_EV_MAX);

#ifdef CONFIG_PLUGIN
    GArray *plugin_mem_cbs;
    /* saved iotlb data from io_writex */
    SavedIOTLB saved_iotlb;
#endif

    /* TODO Move common fields from CPUArchState here. */
    int cpu_index;
    int cluster_index;
    uint32_t tcg_cflags;
    uint32_t halted;
    uint32_t can_do_io;
    int32_t exception_index;

    /* shared by kvm, hax and hvf */
    bool vcpu_dirty;

    /* Used to keep track of an outstanding cpu throttle thread for migration
     * autoconverge
     */
    bool throttle_thread_scheduled;

    bool ignore_memory_transaction_failures;

    struct hax_vcpu_state *hax_vcpu;

    int hvf_fd;

    /* track IOMMUs whose translations we've cached in the TCG TLB */
    GArray *iommu_notifiers;
};
```


#### X86CPU
```c
/**
 * X86CPU:
 * @env: #CPUX86State
 * @migratable: If set, only migratable flags will be accepted when "enforce"
 * mode is used, and only migratable flags will be included in the "host"
 * CPU model.
 *
 * An x86 CPU.
 */
struct X86CPU {
    /*< private >*/
    CPUState parent_obj;
    /*< public >*/

    CPUNegativeOffsetState neg;
    CPUX86State env;

    uint32_t hyperv_spinlock_attempts;
    char *hyperv_vendor_id;
    bool hyperv_synic_kvm_only;
    uint64_t hyperv_features;
    bool hyperv_passthrough;
    OnOffAuto hyperv_no_nonarch_cs;

    bool check_cpuid;
    bool enforce_cpuid;
    /*
     * Force features to be enabled even if the host doesn't support them.
     * This is dangerous and should be done only for testing CPUID
     * compatibility.
     */
    bool force_features;
    bool expose_kvm;
    bool expose_tcg;
    bool migratable;
    bool migrate_smi_count;
    bool max_features; /* Enable all supported features automatically */
    uint32_t apic_id;

    /* Enables publishing of TSC increment and Local APIC bus frequencies to
     * the guest OS in CPUID page 0x40000010, the same way that VMWare does. */
    bool vmware_cpuid_freq;

    /* if true the CPUID code directly forward host cache leaves to the guest */
    bool cache_info_passthrough;

    /* if true the CPUID code directly forwards
     * host monitor/mwait leaves to the guest */
    struct {
        uint32_t eax;
        uint32_t ebx;
        uint32_t ecx;
        uint32_t edx;
    } mwait;

    /* Features that were filtered out because of missing host capabilities */
    FeatureWordArray filtered_features;

    /* Enable PMU CPUID bits. This can't be enabled by default yet because
     * it doesn't have ABI stability guarantees, as it passes all PMU CPUID
     * bits returned by GET_SUPPORTED_CPUID (that depend on host CPU and kernel
     * capabilities) directly to the guest.
     */
    bool enable_pmu;

    /* LMCE support can be enabled/disabled via cpu option 'lmce=on/off'. It is
     * disabled by default to avoid breaking migration between QEMU with
     * different LMCE configurations.
     */
    bool enable_lmce;

    /* Compatibility bits for old machine types.
     * If true present virtual l3 cache for VM, the vcpus in the same virtual
     * socket share an virtual l3 cache.
     */
    bool enable_l3_cache;

    /* Compatibility bits for old machine types.
     * If true present the old cache topology information
     */
    bool legacy_cache;

    /* Compatibility bits for old machine types: */
    bool enable_cpuid_0xb;

    /* Enable auto level-increase for all CPUID leaves */
    bool full_cpuid_auto_level;

    /* Enable auto level-increase for Intel Processor Trace leave */
    bool intel_pt_auto_level;

    /* if true fill the top bits of the MTRR_PHYSMASKn variable range */
    bool fill_mtrr_mask;

    /* if true override the phys_bits value with a value read from the host */
    bool host_phys_bits;

    /* if set, limit maximum value for phys_bits when host_phys_bits is true */
    uint8_t host_phys_bits_limit;

    /* Stop SMI delivery for migration compatibility with old machines */
    bool kvm_no_smi_migration;

    /* Number of physical address bits supported */
    uint32_t phys_bits;

    /* in order to simplify APIC support, we leave this pointer to the
       user */
    struct DeviceState *apic_state;
    struct MemoryRegion *cpu_as_root, *cpu_as_mem, *smram;
    Notifier machine_done;

    struct kvm_msrs *kvm_msr_buf;

    int32_t node_id; /* NUMA node this CPU belongs to */
    int32_t socket_id;
    int32_t die_id;
    int32_t core_id;
    int32_t thread_id;

    int32_t hv_max_vps;
};
```

#####  CPUX86State
```c
typedef struct CPUX86State {
#if defined(CONFIG_X86toMIPS) || defined(CONFIG_LATX)
    ZMMReg xmm_regs[CPU_NB_REGS == 8 ? 8 : 32];
    //ldq,only 10bits offset
#endif
#ifdef CONFIG_LATX
    /* vregs: Details in X86toMIPS/translator/reg_alloc.c */
    uint64_t vregs[6];
    /* mips_iregs: mips context backup when calling helper */
    uint64_t mips_iregs[32];
    uint32_t xtm_fpu;
#endif
    /* standard registers */
    target_ulong regs[CPU_NB_REGS];
    target_ulong eip;
    target_ulong eflags; /* eflags register. During CPU emulation, CC
                        flags and DF are set to zero because they are
                        stored elsewhere */

    /* emulator internal eflags handling */
    target_ulong cc_dst;
    target_ulong cc_src;
    target_ulong cc_src2;
    uint32_t cc_op;
    int32_t df; /* D flag : 1 if D = 0, -1 if D = 1 */
    uint32_t hflags; /* TB flags, see HF_xxx constants. These flags
                        are known at translation time. */
    uint32_t hflags2; /* various other flags, see HF2_xxx constants. */

#if defined (CONFIG_X86toMIPS) && defined(CONFIG_SOFTMMU)
    void *cpt_ptr; /* Point to Code Page Table */
#if defined(CONFIG_XTM_PROFILE)
    struct {
        struct {
            /* Flag for next Jmp Cachel Lookup */
            uint8_t is_jmpdr;
            uint8_t is_jmpin;
            uint8_t is_sys_eob;
            uint8_t is_excp;
        } jc;
        struct {
            uint8_t is_mov;
            uint8_t is_pop;
        } tbf;
    } xtm_pf_data;
#endif /* XTM PROFILE */
#endif

    /* segments */
    SegmentCache segs[6]; /* selector values */
    SegmentCache ldt;
    SegmentCache tr;
    SegmentCache gdt; /* only base and limit are used */
    SegmentCache idt; /* only base and limit are used */

    target_ulong cr[5]; /* NOTE: cr1 is unused */
    int32_t a20_mask;

    BNDReg bnd_regs[4];
    BNDCSReg bndcs_regs;
    uint64_t msr_bndcfgs;
    uint64_t efer;

    /* Beginning of state preserved by INIT (dummy marker).  */
    struct {} start_init_save;

    /* FPU state */
    unsigned int fpstt; /* top of stack index */
    uint16_t fpus;
    uint16_t fpuc;
    uint8_t fptags[8];   /* 0 = valid, 1 = empty */
    FPReg fpregs[8];
    /* KVM-only so far */
    uint16_t fpop;
    uint64_t fpip;
    uint64_t fpdp;

    /* emulator internal variables */
    float_status fp_status;
    floatx80 ft0;

    float_status mmx_status; /* for 3DNow! float ops */
    float_status sse_status;
    uint32_t mxcsr;
#ifndef CONFIG_X86toMIPS
    ZMMReg xmm_regs[CPU_NB_REGS == 8 ? 8 : 32];
#endif
    ZMMReg xmm_t0;
    MMXReg mmx_t0;

    XMMReg ymmh_regs[CPU_NB_REGS];

    uint64_t opmask_regs[NB_OPMASK_REGS];
    YMMReg zmmh_regs[CPU_NB_REGS];
    ZMMReg hi16_zmm_regs[CPU_NB_REGS];

    /* sysenter registers */
    uint32_t sysenter_cs;
    target_ulong sysenter_esp;
    target_ulong sysenter_eip;
    uint64_t star;

    uint64_t vm_hsave;

#ifdef TARGET_X86_64
    target_ulong lstar;
    target_ulong cstar;
    target_ulong fmask;
    target_ulong kernelgsbase;
#endif

    uint64_t tsc;
    uint64_t tsc_adjust;
    uint64_t tsc_deadline;
    uint64_t tsc_aux;

    uint64_t xcr0;

    uint64_t mcg_status;
    uint64_t msr_ia32_misc_enable;
    uint64_t msr_ia32_feature_control;

    uint64_t msr_fixed_ctr_ctrl;
    uint64_t msr_global_ctrl;
    uint64_t msr_global_status;
    uint64_t msr_global_ovf_ctrl;
    uint64_t msr_fixed_counters[MAX_FIXED_COUNTERS];
    uint64_t msr_gp_counters[MAX_GP_COUNTERS];
    uint64_t msr_gp_evtsel[MAX_GP_COUNTERS];

    uint64_t pat;
    uint32_t smbase;
    uint64_t msr_smi_count;

    uint32_t pkru;
    uint32_t tsx_ctrl;

    uint64_t spec_ctrl;
    uint64_t virt_ssbd;

    /* End of state preserved by INIT (dummy marker).  */
    struct {} end_init_save;

    uint64_t system_time_msr;
    uint64_t wall_clock_msr;
    uint64_t steal_time_msr;
    uint64_t async_pf_en_msr;
    uint64_t pv_eoi_en_msr;
    uint64_t poll_control_msr;

    /* Partition-wide HV MSRs, will be updated only on the first vcpu */
    uint64_t msr_hv_hypercall;
    uint64_t msr_hv_guest_os_id;
    uint64_t msr_hv_tsc;

    /* Per-VCPU HV MSRs */
    uint64_t msr_hv_vapic;
    uint64_t msr_hv_crash_params[HV_CRASH_PARAMS];
    uint64_t msr_hv_runtime;
    uint64_t msr_hv_synic_control;
    uint64_t msr_hv_synic_evt_page;
    uint64_t msr_hv_synic_msg_page;
    uint64_t msr_hv_synic_sint[HV_SINT_COUNT];
    uint64_t msr_hv_stimer_config[HV_STIMER_COUNT];
    uint64_t msr_hv_stimer_count[HV_STIMER_COUNT];
    uint64_t msr_hv_reenlightenment_control;
    uint64_t msr_hv_tsc_emulation_control;
    uint64_t msr_hv_tsc_emulation_status;

    uint64_t msr_rtit_ctrl;
    uint64_t msr_rtit_status;
    uint64_t msr_rtit_output_base;
    uint64_t msr_rtit_output_mask;
    uint64_t msr_rtit_cr3_match;
    uint64_t msr_rtit_addrs[MAX_RTIT_ADDRS];

    /* exception/interrupt handling */
    int error_code;
    int exception_is_int;
    target_ulong exception_next_eip;
    target_ulong dr[8]; /* debug registers; note dr4 and dr5 are unused */
    union {
        struct CPUBreakpoint *cpu_breakpoint[4];
        struct CPUWatchpoint *cpu_watchpoint[4];
    }; /* break/watchpoints for dr[0..3] */
    int old_exception;  /* exception in flight */

    uint64_t vm_vmcb;
    uint64_t tsc_offset;
    uint64_t intercept;
    uint16_t intercept_cr_read;
    uint16_t intercept_cr_write;
    uint16_t intercept_dr_read;
    uint16_t intercept_dr_write;
    uint32_t intercept_exceptions;
    uint64_t nested_cr3;
    uint32_t nested_pg_mode;
    uint8_t v_tpr;

    /* KVM states, automatically cleared on reset */
    uint8_t nmi_injected;
    uint8_t nmi_pending;

    uintptr_t retaddr;

    /* Fields up to this point are cleared by a CPU reset */
    struct {} end_reset_fields;

    /* Fields after this point are preserved across CPU reset. */

    /* processor features (e.g. for CPUID insn) */
    /* Minimum cpuid leaf 7 value */
    uint32_t cpuid_level_func7;
    /* Actual cpuid leaf 7 value */
    uint32_t cpuid_min_level_func7;
    /* Minimum level/xlevel/xlevel2, based on CPU model + features */
    uint32_t cpuid_min_level, cpuid_min_xlevel, cpuid_min_xlevel2;
    /* Maximum level/xlevel/xlevel2 value for auto-assignment: */
    uint32_t cpuid_max_level, cpuid_max_xlevel, cpuid_max_xlevel2;
    /* Actual level/xlevel/xlevel2 value: */
    uint32_t cpuid_level, cpuid_xlevel, cpuid_xlevel2;
    uint32_t cpuid_vendor1;
    uint32_t cpuid_vendor2;
    uint32_t cpuid_vendor3;
    uint32_t cpuid_version;
    FeatureWordArray features;
    /* Features that were explicitly enabled/disabled */
    FeatureWordArray user_features;
    uint32_t cpuid_model[12];
    /* Cache information for CPUID.  When legacy-cache=on, the cache data
     * on each CPUID leaf will be different, because we keep compatibility
     * with old QEMU versions.
     */
    CPUCaches cache_info_cpuid2, cache_info_cpuid4, cache_info_amd;

    /* MTRRs */
    uint64_t mtrr_fixed[11];
    uint64_t mtrr_deftype;
    MTRRVar mtrr_var[MSR_MTRRcap_VCNT];

    /* For KVM */
    uint32_t mp_state;
    int32_t exception_nr;
    int32_t interrupt_injected;
    uint8_t soft_interrupt;
    uint8_t exception_pending;
    uint8_t exception_injected;
    uint8_t has_error_code;
    uint8_t exception_has_payload;
    uint64_t exception_payload;
    uint32_t ins_len;
    uint32_t sipi_vector;
    bool tsc_valid;
    int64_t tsc_khz;
    int64_t user_tsc_khz; /* for sanity check only */
#if defined(CONFIG_KVM) || defined(CONFIG_HVF)
    void *xsave_buf;
#endif
#if defined(CONFIG_KVM)
    struct kvm_nested_state *nested_state;
#endif
#if defined(CONFIG_HVF)
    HVFX86EmulatorState *hvf_emul;
#endif

    uint64_t mcg_cap;
    uint64_t mcg_ctl;
    uint64_t mcg_ext_ctl;
    uint64_t mce_banks[MCE_BANKS_DEF*4];
    uint64_t xstate_bv;

    /* vmstate */
    uint16_t fpus_vmstate;
    uint16_t fptag_vmstate;
    uint16_t fpregs_format_vmstate;

    uint64_t xss;
    uint32_t umwait;

    TPRAccess tpr_access_type;

    unsigned nr_dies;
#ifdef CONFIG_X86toMIPS
#ifndef CONFIG_LATX
    /* vregs: Details in X86toMIPS/translator/reg_alloc.c */
    uint64_t vregs[6];
    /* mips_iregs: mips context backup when calling helper */
    uint64_t mips_iregs[32];
    /*
     * xtm_fpu: flag for FPU
     *
     * [0:2]: TOP in status word
     *        (fldenv will clear TOP in status and set the fpstt)
     *
     * [7]: = 0: status word is not loaded from memory
     *      = 1: status word loaded from memory
     *           (fldenv, frstor, fxrstor, xrstor)
     *
     * [6]: = 0: FPU state is not reset
     *      = 1: FPU state is reset (fninit, fnsave)
     *
     * [7] and [6] cannot all be 1 at same time.
     *
     * [08:10]: TOP out of last executed TB
     *
     * [11:13]: TOP in of last executed TB
     *
     * [14]: = 1: TOP out is valid
     *       = 0: TOP out is not valid and should not be used
     *
     * [15]: = 1: TOP in  is valid
     *       = 0: TOP in  is not valid and should not be used
     *
     * For now, this is only used in system-mode.
     * */
    uint32_t xtm_fpu;
#endif
    /*
     * X86toMIPS Flag is only useful when configure with
     *
     *      --enable-x86tomips-flag-int     # CONFIG_XTM_FLAG_INT
     *
     * If not, the value of flag will always be zero.
     *
     * is_in_int = 0: CPU is not handling interrupt
     *  > is_int_inst: meaningless
     *
     * is_in_int >= 1: CPU is handling interrupt
     *                 if nested interrupt, this will be greater than 1
     *  > is_int_inst = 0: this interrupt is NOT an int instruction
     *  > is_int_inst = 1: this interrupt is an int instruction
     *
     * is_top_int_inst: only useful in nested interrupt
     *  = 0: the first interrupt is not INT instruction
     *  = 1: the first interrupt is INT instruction
     */
    struct {
        int is_in_int;
        int is_int_inst;
        int is_top_int_inst; /* for nested interrupt */
    }xtm_flags;

#if defined(CONFIG_XTM_TEST)
    int exit_test;
#endif
#endif
#ifdef CONFIG_BTMMU
    struct TranslationBlock *current_tb;
#endif
} CPUX86State;
```

#### DeviceClass
```c
/**
 * DeviceClass:
 * @props: Properties accessing state fields.
 * @realize: Callback function invoked when the #DeviceState:realized
 * property is changed to %true.
 * @unrealize: Callback function invoked when the #DeviceState:realized
 * property is changed to %false.
 * @hotpluggable: indicates if #DeviceClass is hotpluggable, available
 * as readonly "hotpluggable" property of #DeviceState instance
 *
 * # Realization #
 * Devices are constructed in two stages,
 * 1) object instantiation via object_initialize() and
 * 2) device realization via #DeviceState:realized property.
 * The former may not fail (and must not abort or exit, since it is called
 * during device introspection already), and the latter may return error
 * information to the caller and must be re-entrant.
 * Trivial field initializations should go into #TypeInfo.instance_init.
 * Operations depending on @props static properties should go into @realize.
 * After successful realization, setting static properties will fail.
 *
 * As an interim step, the #DeviceState:realized property can also be
 * set with qdev_init_nofail().
 * In the future, devices will propagate this state change to their children
 * and along busses they expose.
 * The point in time will be deferred to machine creation, so that values
 * set in @realize will not be introspectable beforehand. Therefore devices
 * must not create children during @realize; they should initialize them via
 * object_initialize() in their own #TypeInfo.instance_init and forward the
 * realization events appropriately.
 *
 * Any type may override the @realize and/or @unrealize callbacks but needs
 * to call the parent type's implementation if keeping their functionality
 * is desired. Refer to QOM documentation for further discussion and examples.
 *
 * <note>
 *   <para>
 * Since TYPE_DEVICE doesn't implement @realize and @unrealize, types
 * derived directly from it need not call their parent's @realize and
 * @unrealize.
 * For other types consult the documentation and implementation of the
 * respective parent types.
 *   </para>
 * </note>
 *
 * # Hiding a device #
 * To hide a device, a DeviceListener function should_be_hidden() needs to
 * be registered.
 * It can be used to defer adding a device and therefore hide it from the
 * guest. The handler registering to this DeviceListener can save the QOpts
 * passed to it for re-using it later and must return that it wants the device
 * to be/remain hidden or not. When the handler function decides the device
 * shall not be hidden it will be added in qdev_device_add() and
 * realized as any other device. Otherwise qdev_device_add() will return early
 * without adding the device. The guest will not see a "hidden" device
 * until it was marked don't hide and qdev_device_add called again.
 *
 */
typedef struct DeviceClass {
    /*< private >*/
    ObjectClass parent_class;
    /*< public >*/

    DECLARE_BITMAP(categories, DEVICE_CATEGORY_MAX);
    const char *fw_name;
    const char *desc;
    Property *props;

    /*
     * Can this device be instantiated with -device / device_add?
     * All devices should support instantiation with device_add, and
     * this flag should not exist.  But we're not there, yet.  Some
     * devices fail to instantiate with cryptic error messages.
     * Others instantiate, but don't work.  Exposing users to such
     * behavior would be cruel; clearing this flag will protect them.
     * It should never be cleared without a comment explaining why it
     * is cleared.
     * TODO remove once we're there
     */
    bool user_creatable;
    bool hotpluggable;

    /* callbacks */
    DeviceReset reset;
    DeviceRealize realize;
    DeviceUnrealize unrealize;

    /* device state */
    const VMStateDescription *vmsd;

    /* Private to qdev / bus.  */
    const char *bus_type;
} DeviceClass;
```

#### CPUClass
```c
/**
 * CPUClass:
 * @class_by_name: Callback to map -cpu command line model name to an
 * instantiatable CPU type.
 * @parse_features: Callback to parse command line arguments.
 * @reset: Callback to reset the #CPUState to its initial state.
 * @reset_dump_flags: #CPUDumpFlags to use for reset logging.
 * @has_work: Callback for checking if there is work to do.
 * @do_interrupt: Callback for interrupt handling.
 * @do_unaligned_access: Callback for unaligned access handling, if
 * the target defines #TARGET_ALIGNED_ONLY.
 * @do_transaction_failed: Callback for handling failed memory transactions
 * (ie bus faults or external aborts; not MMU faults)
 * @virtio_is_big_endian: Callback to return %true if a CPU which supports
 * runtime configurable endianness is currently big-endian. Non-configurable
 * CPUs can use the default implementation of this method. This method should
 * not be used by any callers other than the pre-1.0 virtio devices.
 * @memory_rw_debug: Callback for GDB memory access.
 * @dump_state: Callback for dumping state.
 * @dump_statistics: Callback for dumping statistics.
 * @get_arch_id: Callback for getting architecture-dependent CPU ID.
 * @get_paging_enabled: Callback for inquiring whether paging is enabled.
 * @get_memory_mapping: Callback for obtaining the memory mappings.
 * @set_pc: Callback for setting the Program Counter register. This
 *       should have the semantics used by the target architecture when
 *       setting the PC from a source such as an ELF file entry point;
 *       for example on Arm it will also set the Thumb mode bit based
 *       on the least significant bit of the new PC value.
 *       If the target behaviour here is anything other than "set
 *       the PC register to the value passed in" then the target must
 *       also implement the synchronize_from_tb hook.
 * @synchronize_from_tb: Callback for synchronizing state from a TCG
 *       #TranslationBlock. This is called when we abandon execution
 *       of a TB before starting it, and must set all parts of the CPU
 *       state which the previous TB in the chain may not have updated.
 *       This always includes at least the program counter; some targets
 *       will need to do more. If this hook is not implemented then the
 *       default is to call @set_pc(tb->pc).
 * @tlb_fill: Callback for handling a softmmu tlb miss or user-only
 *       address fault.  For system mode, if the access is valid, call
 *       tlb_set_page and return true; if the access is invalid, and
 *       probe is true, return false; otherwise raise an exception and
 *       do not return.  For user-only mode, always raise an exception
 *       and do not return.
 * @get_phys_page_debug: Callback for obtaining a physical address.
 * @get_phys_page_attrs_debug: Callback for obtaining a physical address and the
 *       associated memory transaction attributes to use for the access.
 *       CPUs which use memory transaction attributes should implement this
 *       instead of get_phys_page_debug.
 * @asidx_from_attrs: Callback to return the CPU AddressSpace to use for
 *       a memory access with the specified memory transaction attributes.
 * @gdb_read_register: Callback for letting GDB read a register.
 * @gdb_write_register: Callback for letting GDB write a register.
 * @debug_check_watchpoint: Callback: return true if the architectural
 *       watchpoint whose address has matched should really fire.
 * @debug_excp_handler: Callback for handling debug exceptions.
 * @write_elf64_note: Callback for writing a CPU-specific ELF note to a
 * 64-bit VM coredump.
 * @write_elf32_qemunote: Callback for writing a CPU- and QEMU-specific ELF
 * note to a 32-bit VM coredump.
 * @write_elf32_note: Callback for writing a CPU-specific ELF note to a
 * 32-bit VM coredump.
 * @write_elf32_qemunote: Callback for writing a CPU- and QEMU-specific ELF
 * note to a 32-bit VM coredump.
 * @vmsd: State description for migration.
 * @gdb_num_core_regs: Number of core registers accessible to GDB.
 * @gdb_core_xml_file: File name for core registers GDB XML description.
 * @gdb_stop_before_watchpoint: Indicates whether GDB expects the CPU to stop
 *           before the insn which triggers a watchpoint rather than after it.
 * @gdb_arch_name: Optional callback that returns the architecture name known
 * to GDB. The caller must free the returned string with g_free.
 * @gdb_get_dynamic_xml: Callback to return dynamically generated XML for the
 *   gdb stub. Returns a pointer to the XML contents for the specified XML file
 *   or NULL if the CPU doesn't have a dynamically generated content for it.
 * @cpu_exec_enter: Callback for cpu_exec preparation.
 * @cpu_exec_exit: Callback for cpu_exec cleanup.
 * @cpu_exec_interrupt: Callback for processing interrupts in cpu_exec.
 * @disas_set_info: Setup architecture specific components of disassembly info
 * @adjust_watchpoint_address: Perform a target-specific adjustment to an
 * address before attempting to match it against watchpoints.
 *
 * Represents a CPU family or model.
 */
typedef struct CPUClass {
    /*< private >*/
    DeviceClass parent_class;
    /*< public >*/

    ObjectClass *(*class_by_name)(const char *cpu_model);
    void (*parse_features)(const char *typename, char *str, Error **errp);

    void (*reset)(CPUState *cpu);
    int reset_dump_flags;
    bool (*has_work)(CPUState *cpu);
    void (*do_interrupt)(CPUState *cpu);
    void (*do_unaligned_access)(CPUState *cpu, vaddr addr,
                                MMUAccessType access_type,
                                int mmu_idx, uintptr_t retaddr);
    void (*do_transaction_failed)(CPUState *cpu, hwaddr physaddr, vaddr addr,
                                  unsigned size, MMUAccessType access_type,
                                  int mmu_idx, MemTxAttrs attrs,
                                  MemTxResult response, uintptr_t retaddr);
    bool (*virtio_is_big_endian)(CPUState *cpu);
    int (*memory_rw_debug)(CPUState *cpu, vaddr addr,
                           uint8_t *buf, int len, bool is_write);
    void (*dump_state)(CPUState *cpu, FILE *, int flags);
    GuestPanicInformation* (*get_crash_info)(CPUState *cpu);
    void (*dump_statistics)(CPUState *cpu, int flags);
    int64_t (*get_arch_id)(CPUState *cpu);
    bool (*get_paging_enabled)(const CPUState *cpu);
    void (*get_memory_mapping)(CPUState *cpu, MemoryMappingList *list,
                               Error **errp);
    void (*set_pc)(CPUState *cpu, vaddr value);
    void (*synchronize_from_tb)(CPUState *cpu, struct TranslationBlock *tb);
    bool (*tlb_fill)(CPUState *cpu, vaddr address, int size,
                     MMUAccessType access_type, int mmu_idx,
                     bool probe, uintptr_t retaddr);
    hwaddr (*get_phys_page_debug)(CPUState *cpu, vaddr addr);
    hwaddr (*get_phys_page_attrs_debug)(CPUState *cpu, vaddr addr,
                                        MemTxAttrs *attrs);
    int (*asidx_from_attrs)(CPUState *cpu, MemTxAttrs attrs);
    int (*gdb_read_register)(CPUState *cpu, uint8_t *buf, int reg);
    int (*gdb_write_register)(CPUState *cpu, uint8_t *buf, int reg);
    bool (*debug_check_watchpoint)(CPUState *cpu, CPUWatchpoint *wp);
    void (*debug_excp_handler)(CPUState *cpu);

    int (*write_elf64_note)(WriteCoreDumpFunction f, CPUState *cpu,
                            int cpuid, void *opaque);
    int (*write_elf64_qemunote)(WriteCoreDumpFunction f, CPUState *cpu,
                                void *opaque);
    int (*write_elf32_note)(WriteCoreDumpFunction f, CPUState *cpu,
                            int cpuid, void *opaque);
    int (*write_elf32_qemunote)(WriteCoreDumpFunction f, CPUState *cpu,
                                void *opaque);

    const VMStateDescription *vmsd;
    const char *gdb_core_xml_file;
    gchar * (*gdb_arch_name)(CPUState *cpu);
    const char * (*gdb_get_dynamic_xml)(CPUState *cpu, const char *xmlname);
    void (*cpu_exec_enter)(CPUState *cpu);
    void (*cpu_exec_exit)(CPUState *cpu);
    bool (*cpu_exec_interrupt)(CPUState *cpu, int interrupt_request);

    void (*disas_set_info)(CPUState *cpu, disassemble_info *info);
    vaddr (*adjust_watchpoint_address)(CPUState *cpu, vaddr addr, int len);
    void (*tcg_initialize)(void);

    /* Keep non-pointer data at the end to minimize holes.  */
    int gdb_num_core_regs;
    bool gdb_stop_before_watchpoint;
} CPUClass;
```

#### X86CPUClass
```c
/**
 * X86CPUClass:
 * @cpu_def: CPU model definition
 * @host_cpuid_required: Whether CPU model requires cpuid from host.
 * @ordering: Ordering on the "-cpu help" CPU model list.
 * @migration_safe: See CpuDefinitionInfo::migration_safe
 * @static_model: See CpuDefinitionInfo::static
 * @parent_realize: The parent class' realize handler.
 * @parent_reset: The parent class' reset handler.
 *
 * An x86 CPU model or family.
 */
typedef struct X86CPUClass {
    /*< private >*/
    CPUClass parent_class;
    /*< public >*/

    /* CPU definition, automatically loaded by instance_init if not NULL.
     * Should be eventually replaced by subclass-specific property defaults.
     */
    X86CPUModel *model;

    bool host_cpuid_required;
    int ordering;
    bool migration_safe;
    bool static_model;

    /* Optional description of CPU model.
     * If unavailable, cpu_def->model_id is used */
    const char *model_description;

    DeviceRealize parent_realize;
    DeviceUnrealize parent_unrealize;
    void (*parent_reset)(CPUState *cpu);
} X86CPUClass;
```

[^1]: https://wiki.qemu.org/Features/PC_System_Flash
[^2]: https://en.wikipedia.org/wiki/Machine-check_exception
