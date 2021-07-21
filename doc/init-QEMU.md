# QEMU 启动代码

## 问题
- [ ] qdev_device_add 是做什么的
```
huxueshi:qdev_device_add isa-debugcon
huxueshi:qdev_device_add nvme
huxueshi:qdev_device_add virtio-9p-pci
```

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

## call graph
- qemu_init : 这里面存在很长的参数解析的内容
  - qemu_create_machine(select_machine()) : select_machine 中获取 MachineClass
    - cpu_exec_init_all :
      - io_mem_init : 初始化 io_mem_unassigned, 但是实际上，这个 mr 永远都不会被使用
      - memory_map_init : 初始化 system_memory, 和 io_memory 这两个都是 container 并不会真正的分配的映射空间
    - page_size_init : 初始化之后的 softmmu 需要的内容
  - configure_accelerators
    - qemu_opts_foreach
      - do_configure_accelerator
        - accel_init_machine : 在 tcg_accel_class_init 的位置初始化
          - tcg_init
            - tcg_exec_init
              - cpu_gen_init
                - tcg_context_init
              - page_init
              - tb_htable_init
              - alloc_code_gen_buffer
                - alloc_code_gen_buffer_anon
              - tcg_prologue_init
            - tcg_region_init
  - qmp_x_exit_preconfig
    - qemu_init_board
      - create_default_memdev : 比想象的复杂一点，是因为实际上，RAM 还可以是 filebased
      - machine_run_board_init
        - `machine_class->init` : DEFINE_I440FX_MACHINE 这个封装出来 pc_init_v6_1 来调用
          - pc_init1
            - x86_cpus_init : 在主线程中间多次调用 x86_cpu_new , 可能会创建新的 CPU 来
              - x86_cpu_new
                - qdev_realize : 经过 QOM 的 object_property 机制，最后调用到 device_set_realized :
                  - device_set_realized : 
                    - x86_cpu_realizefn : 需要重点分析一下
                      - cpu_list_add
                      - cpu_exec_realizefn
                        - accel_cpu_realizefn
                          - kvm_cpu_realizefn
                          - tcg_cpu_realizefn : 主要就是 address space 的初始化
                            - cpu_address_space_init 
                              - memory_listener_register
                                - tcg_commit
                        - tcg_exec_realizefn
                          - TCGCPUOps::initialize => tcg_x86_init: 这是 CPUClass 上注册的函数，进行一些 tcg 相关的的初始化, 例如 regs
                          - tlb_init
                            - tlb_mmu_init
                      - x86_cpu_expand_features
                      - x86_cpu_filter_features
                      - mce_init : machine check exception, 初始化之后，那些 helper 就可以正确工作了, mce 参考[^2]
                      - qemu_init_vcpu : 创建执行线程
                        - rr_cpu_thread_fn : 进行一些基本的注册工作，然后等待
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
    - qmp_cont : qmp_cont 可以作为一个通用的 qmp 函数来调用，让系统继续运行，当然也可以作为系统刚刚启动的效果
      - vm_start
        - vm_prepare_start
        - resume_all_vcpus
  - qemu_init_displays
  - accel_setup_post 
  - os_setup_post
  - resume_mux_open
- qemu_main_loop
  - qemu_debug_requested
  - qemu_suspend_requested
  - qemu_shutdown_requested
  - qemu_reset_requested
  - qemu_wakeup_requested

## e820
- 信息是如何构造出来的
  - 在  pc_memory_init 调用两次 e820_add_entry, 分别添加 below_4g_mem_size 和 above_4g_mem_size
- 如何通知 guest 内核的 ?
  - 在 fw_cfg_arch_create 中添加 `etc/e820` 实现的

- [ ] 类似 pci 映射的 MMIO 空间的分配是 e820 负责的操作的吗 ?

## pflash
在 hw/i386/pc_sysfw.c 似乎主要处理的就是 pflash

关于 pflash 和 bios 的关系可以首先看看[^1], 但是目前不需要知道 pflash 也可以
## choose Machine
挺无聊的一个， select_machine

从注册的里面进行选择一个 default, 也可以从参数中靠 machine_parse 解析出来
```c
// 部分省略了
/*
pc-q35-2.11
pc-i440fx-3.0
pc-q35-2.5
pc-i440fx-2.8
pc-i440fx-5.0
pc-i440fx-4.0
pc-i440fx-2.3
microvm
xenfv-4.2
isapc
x-remote
none
xenpv
```

## choose cpu
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

当然还可以选择其他的 cpu，其解析工作在 parse_cpu_option

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

## cpu feature
总体来说，cpu feature 都是通过 CPUX86State::features 进行的，QOM property 闲的很傻

feature_word_info : 是定义了所有的存在的 feature

X86CPUDefinition::features 在 x86_cpu_load_model 中，将这个拷贝到 CPUX86State::features 中

- x86_cpu_common_class_init
  - x86_cpu_register_feature_bit_props : 给 xcc 添加上一堆 property , 这些 property 的访问方式是 x86_cpu_set_bit_prop

kvm
```c
/*
#0  x86_cpu_set_bit_prop (obj=0x555555e64ac8 <object_property_find_err+43>, v=0x7fffffffd0a0, name=0x55555689ee30 "\220\356\211VUU", opaque=0x555556963e80, errp=0x555556c32070) at ../target/i386/cpu.c:4001
#1  0x0000555555e64f5a in object_property_set (obj=0x555556c32070, name=0x55555608fef1 "kvmclock", v=0x555556b55070, errp=0x5555567a1ee8 <error_abort>) at ../qom/object.c:1402
#2  0x0000555555e65b0f in object_property_parse (obj=0x555556c32070, name=0x55555608fef1 "kvmclock", string=0x55555608fefa "on", errp=0x5555567a1ee8 <error_abort>) at ../qom/object.c:1642
#3  0x0000555555ba00f3 in x86_cpu_apply_props (cpu=0x555556c32070, props=0x5555566c7e60 <kvm_default_props>) at ../target/i386/cpu.c:2638
#4  0x0000555555b3df02 in kvm_cpu_instance_init (cs=0x555556c32070) at ../target/i386/kvm/kvm-cpu.c:126
#5  0x0000555555c82967 in accel_cpu_instance_init (cpu=0x555556c32070) at ../accel/accel-common.c:110
#6  0x0000555555ba3ffa in x86_cpu_initfn (obj=0x555556c32070) at ../target/i386/cpu.c:4131
```

tcg
```c
/*
#0  x86_cpu_set_bit_prop (obj=0x555555e64ac8 <object_property_find_err+43>, v=0x7fffffffd090, name=0x5555568963e0 "@d\211VUU", opaque=0x555556974ba0, errp=0x555556c28050) at ../target/i386/cpu.c:4001
#1  0x0000555555e64f5a in object_property_set (obj=0x555556c28050, name=0x5555560a7af1 "vme", v=0x555556b56000, errp=0x5555567a1ee8 <error_abort>) at ../qom/object.c:14
#2  0x0000555555e65b0f in object_property_parse (obj=0x555556c28050, name=0x5555560a7af1 "vme", string=0x5555560a7af5 "off", errp=0x5555567a1ee8 <error_abort>) at ../qom/object.c:1642
#3  0x0000555555ba00f3 in x86_cpu_apply_props (cpu=0x555556c28050, props=0x5555566d9c00 <tcg_default_props>) at ../target/i386/cpu.c:2638
#4  0x0000555555bd68f2 in tcg_cpu_instance_init (cs=0x555556c28050) at ../target/i386/tcg/tcg-cpu.c:95
#5  0x0000555555c82967 in accel_cpu_instance_init (cpu=0x555556c28050) at ../accel/accel-common.c:110
#6  0x0000555555ba3ffa in x86_cpu_initfn (obj=0x555556c28050) at ../target/i386/cpu.c:4131
```
一共就是只是调用两次 x86_cpu_set_bit_prop

x86_cpu_get_bit_prop 从来都不会被调用啊, 那么存在什么意义啊

- x86_cpu_set_bit_prop 只是在 tcg 和 kvm 的这样使用，那么那些 CPU feature 几乎就是全都没有使用了
  - 实际上，这个有点多余， x86_cpu_set_bit_prop 也只是设置了一下 CPUX86State::features

- 在 86_cpu_initfn 中，多次的调用了 object_property_add_alias 这都是做啥的 (只是为了修改一下名称吧)
  - 这个想法在 x86_cpu_register_feature_bit_props  x86_cpu_register_feature_bit_props 中得到验证

## init machine
我们发现，整个 machine 的体系几乎完全没有被拷贝进去，对此，
原因是
1. machine 的初始化只是为了完成 cpu 的初始化，所以，当 cpu 没有初始化
，machine 的内容完全看不到了。
2. machine 的工作在于 acpi smbios pci 之类的, 暂时没有处理到

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


#### CPUState
| fields         | 初始化的位置                                                                                 |
|----------------|----------------------------------------------------------------------------------------------|
| nr_cores       | cpu_common_initfn 中初始化为 1, 而在 qemu_init_vcpu 中初始化为 ms->smp.cores, 默认初始化为 1 |
| nr_threads     |                                                                                              |
| jmp_env        | 无需初始化, 使用的位置就是哪里                                                               |

```c
    /* TODO Move common fields from CPUArchState here. */
    int cpu_index;
    int cluster_index;
    uint32_t halted;
    uint32_t can_do_io;
    int32_t exception_index;
```
上面的几个变量有点类似，
在 cpu_common_reset 中进行一些简单的初始化，但是实际上这并不是正确的位置。
其实这是一个废话，都是 CPUState 在这里初始化的
这些变量之所以再次会在其他的位置初始化，是因为 X86CPU 的进一步初始化


- cpu_index : 赋值位置
  - cpu_list_add / cpu_list_remove 
  - cpu_common_initfn
  - x86_cpu_pre_plug : 
    - 这是实际上初始化的位置，实际上，这个 idx 获取似乎有点麻烦，但是实际上，并没有必要
- cluster_index
- halted
- can_do_io
- exception_index


#### X86CPU

| fields                        | 初始化的位置                                                   |
|-------------------------------|----------------------------------------------------------------|
| neg                           | tlb_init                                                       |
| env                           | x86_cpu_reset                                                  |
| apic_state                    | x86_cpu_realizefn => x86_cpu_apic_create                       |
| apic_id                       | x86_cpu_new 使用 CPUArchId::arch_id 初始化                     |
| expose_tcg                    | 应该是可以直接去掉, 默认是打开的，*但是不知道打开的效果是什么* |
| phys_bits                     | x86_cpu_realizefn                                              |
| enable_l3_cache / enable_lmce | 和 expose_tcg 的操作方式类似, 理解了 GlobalProperty 在说吧     |
| singlestep_enabled            | 暂时保证永远不会被启动吧                                       |
| cpu_index / cluster_index     |                                                                |

##### apic_id
```c
/*
#0  x86_find_cpu_slot (ms=0x555556095510 <__func__.35759>, id=32767, idx=0x555555d79f55 <trace_object_dynamic_cast_assert+57>) at ../hw/i386/x86.c:172
#1  0x0000555555b5e714 in x86_cpu_pre_plug (hotplug_dev=0x5555569069e0, dev=0x555556d00c00, errp=0x7fffffffd090) at ../hw/i386/x86.c:357
#2  0x0000555555b96ced in pc_machine_device_pre_plug_cb (hotplug_dev=0x5555569069e0, dev=0x555556d00c00, errp=0x7fffffffd090) at ../hw/i386/pc.c:1380
#3  0x0000555555d70dd8 in hotplug_handler_pre_plug (plug_handler=0x5555569069e0, plugged_dev=0x555556d00c00, errp=0x7fffffffd090) at ../hw/core/hotplug.c:23
#4  0x0000555555d757f6 in device_set_realized (obj=0x555556d00c00, value=true, errp=0x7fffffffd198) at ../hw/core/qdev.c:754
#5  0x0000555555d7f28d in property_set_bool (obj=0x555556d00c00, v=0x555556c07c60, name=0x5555560f1e79 "realized", opaque=0x555556884f70, errp=0x7fffffffd198) at ../qom
/object.c:2257
#6  0x0000555555d7d2ae in object_property_set (obj=0x555556d00c00, name=0x5555560f1e79 "realized", v=0x555556c07c60, errp=0x5555567a1f68 <error_fatal>) at ../qom/object
.c:1402
#7  0x0000555555d79b63 in object_property_set_qobject (obj=0x555556d00c00, name=0x5555560f1e79 "realized", value=0x555556bc6e50, errp=0x5555567a1f68 <error_fatal>) at .
./qom/qom-qobject.c:28
#8  0x0000555555d7d626 in object_property_set_bool (obj=0x555556d00c00, name=0x5555560f1e79 "realized", value=true, errp=0x5555567a1f68 <error_fatal>) at ../qom/object.
c:1472
#9  0x0000555555d7484e in qdev_realize (dev=0x555556d00c00, bus=0x0, errp=0x5555567a1f68 <error_fatal>) at ../hw/core/qdev.c:389
#10 0x0000555555b5db48 in x86_cpu_new (x86ms=0x5555569069e0, apic_id=0, errp=0x5555567a1f68 <error_fatal>) at ../hw/i386/x86.c:113
#11 0x0000555555b5dc1b in x86_cpus_init (x86ms=0x5555569069e0, default_cpu_version=1) at ../hw/i386/x86.c:140
#12 0x0000555555b627cb in pc_init1 (machine=0x5555569069e0, host_type=0x555556095eaa "i440FX-pcihost", pci_type=0x555556095ea3 "i440FX") at ../hw/i386/pc_piix.c:157
#13 0x0000555555b6337e in pc_init_v6_1 (machine=0x5555569069e0) at ../hw/i386/pc_piix.c:425
#14 0x0000555555963cc6 in machine_run_board_init (machine=0x5555569069e0) at ../hw/core/machine.c:1239
#15 0x0000555555c67acd in qemu_init_board () at ../softmmu/vl.c:2526
#16 0x0000555555c67cac in qmp_x_exit_preconfig (errp=0x5555567a1f68 <error_fatal>) at ../softmmu/vl.c:2600
#17 0x0000555555c6a384 in qemu_init (argc=29, argv=0x7fffffffd748, envp=0x7fffffffd838) at ../softmmu/vl.c:3635
#18 0x000055555582c575 in main (argc=29, argv=0x7fffffffd748, envp=0x7fffffffd838) at ../softmmu/main.c:49
```
和
```c
/*
#0  x86_possible_cpu_arch_ids (ms=0x555555b5da43 <x86_cpu_apic_id_from_index+86>) at ../hw/i386/x86.c:451
#1  0x0000555555b5dbd3 in x86_cpus_init (x86ms=0x5555569069e0, default_cpu_version=1) at ../hw/i386/x86.c:138
#2  0x0000555555b627cb in pc_init1 (machine=0x5555569069e0, host_type=0x555556095eaa "i440FX-pcihost", pci_type=0x555556095ea3 "i440FX") at ../hw/i386/pc_piix.c:157
#3  0x0000555555b6337e in pc_init_v6_1 (machine=0x5555569069e0) at ../hw/i386/pc_piix.c:425
#4  0x0000555555963cc6 in machine_run_board_init (machine=0x5555569069e0) at ../hw/core/machine.c:1239
#5  0x0000555555c67acd in qemu_init_board () at ../softmmu/vl.c:2526
#6  0x0000555555c67cac in qmp_x_exit_preconfig (errp=0x5555567a1f68 <error_fatal>) at ../softmmu/vl.c:2600
#7  0x0000555555c6a384 in qemu_init (argc=29, argv=0x7fffffffd748, envp=0x7fffffffd838) at ../softmmu/vl.c:3635
#8  0x000055555582c575 in main (argc=29, argv=0x7fffffffd748, envp=0x7fffffffd838) at ../softmmu/main.c:49
```

##### cpu_index / cluster_index

- [ ] cluster_index : 应该是没有被重新初始化过，具体需要使用 xqm 分析一下


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

#### CPUClass
几乎所有的成员都是和 vmstate 相关的，所以实际上，

x86_cpu_common_class_init 注册了大量函数
在 cpu_class_init 中也是注册了一些，处理方法很简单，首先找到
x86_cpu_common_class_init 中注册是什么，然后找 cpu_class_init 注册内容:

确定一个决定，**不要删除 CPUClass 这个东西, 将这些函数全部放到一起，这样出入更小，更容易理解**
#### X86CPUClass
- [x] model 如何处理的 : 用于 list 所有可选 cpu 的, 参考 x86_cpu_list
- [x] parent_realize 处理的

在 x86_cpu_common_class_init 中，将通过 
```
    device_class_set_parent_realize(dc, x86_cpu_realizefn,
                                    &xcc->parent_realize);
```

```c
void device_class_set_parent_realize(DeviceClass *dc,
                                     DeviceRealize dev_realize,
                                     DeviceRealize *parent_realize)
{
    *parent_realize = dc->realize;
    dc->realize = dev_realize;
}
```
同时初始化一下 parent_realize 和 realize


所以，最后 x86_cpu_realizefn 会调用 cpu_common_realizefn


[^1]: https://wiki.qemu.org/Features/PC_System_Flash
[^2]: https://en.wikipedia.org/wiki/Machine-check_exception
