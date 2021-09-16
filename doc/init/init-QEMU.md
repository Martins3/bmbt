# QEMU 启动代码

## 初始化 QEMU 大约需要处理的事情
- [ ] tcg_register_thread : 唯一 reference 了 MachineState
- [ ] cpu_exec_realizefn 中调用 cpu_list_add 添加 CPUState 的
- [ ] CPUState 中的 cpu_index, cluster_index 等
- [ ] 问题是 PCI 设备实际上是需要的，需要初始化一下 PCI 的设备空间吗?
  - [ ] 问题是，这个初始化过程中，需要保持那些设备的之间的联系的吗?
- [ ] fw_cfg 的初始化过程
- [ ] cpu 初始化

## 几个关键的结构体功能和移植差异说明

| struct      | explaination                                                                            |
|-------------|-----------------------------------------------------------------------------------------|
| CPUClass    | 在函数 x86_cpu_common_class_init 中已经知道注册的函数, 可以将其直接定义为一个静态函数集 |
| CPUState    |                                                                                         |
| CPUX86State | 和 CPUState 没有父子关系，而是靠 CPUState::env_ptr 决定的                               |
| X86CPU      |                                                                                         |
| TBContext   | 一个统计，一个 qht, 似乎只是定义了一个全局变量                                          |
| TCGContext  | TODO 每一个 thread 定义了一个，同时存在一个全局的 tcg_init_ctx                          |

## 问题
- [ ] qdev_device_add 是做什么的
```plain
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
| cpu  | :x:                | - [ ] 到底初始化的是那几个结构体，和 tcg 耦合的结构体是谁                                |

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
                - tcg_context_init : 在 xqm 下这个没有意义
              - page_init
              - tb_htable_init
              - code_gen_alloc
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
                        - rr_cpu_thread_fn : 进行一些基本的注册工作，然后等待, 注意，此时在另一个线程中间了
                      - x86_cpu_apic_realize
                        - 通过 QOM 调用到 apic_common_realize
                           - 通过 QOM 调用 apic_realize
                        - 添加对应的 memory region
                      - X86CPUClass::parent_realize : 也就是 cpu_common_realizefn, 这里并没有做什么事情
          - pc_memory_init : 创建了两个 mr alias，ram_below_4g 以及 ram_above_4g，这两个 mr 分别指向 ram 的低 4g 以及高 4g 空间，这两个 alias 是挂在根 system_memory mr 下面的
            - e820_add_entry
            - pc_system_firmware_init : 处理 `-drive if=pflash` 的选项
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
            - init_pam
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

```plain
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
```plain
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

#### CPUX86State
- [ ] smbase : 这个地址似乎用于 smm 保存上下文的地方, 这个东西就是 SMRAM 的基地址

[^2]: https://en.wikipedia.org/wiki/Machine-check_exception
