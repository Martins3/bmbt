# QEMU 启动代码

## 整体的问题
怎么说，是启动到 bios 部分还是启动到 Linux Kernel 部分
  - [ ] seabios 的启动过程是什么 ?

- 到底什么必须在物理机器上测试 ?
  - [ ] PCIe 设备穿透，这个能否可以直接拒绝提供，让 seabios 无法探测出来任何 PCIe 设备
  - [ ] legacy 设备模拟，不清楚是否能够进行一些穿透
  - [ ] 能不能只是提供一个串口出去 ?

- [ ] seabios 其实也是输出信息出去的 ?
  - 是串口实现的吗 ?

- [ ] 如果只是提供一个串口设备，系统真的可以启动吗 ?
  - [ ] 应该思考一下, la-qemu 的代码初始化机器也是很容易的啊，观测一下他们怎么操作的

- [ ] KVM 和 tcg 的初始化的差别是在调用那些函数的时候体现的

- [ ] 我们需要 qemu 的 acpi 机制来实现 acpi table 的组装吗 ?
  - 还是感觉不需要, QEMU 的功能过于强大
  - [ ] 当通过 qemu_create_cli_devices 来实现添加 cli 提供的设备
    - [ ] 似乎很多设备，e1000 没有提供，最后也是默认添加了, 怎么添加上的
    - [ ] 似乎，不添加磁盘，一般是等待最后 mount 系统的时候才会出现问题啊
    - [ ] 各种 isa 设备能够不要添加
```
huxueshi:qdev_device_add isa-debugcon
huxueshi:qdev_device_add nvme
huxueshi:qdev_device_add virtio-9p-pci
```

- [ ] 为了让 bios 可以正确运行，一定需要 fw_cfg 吗 ?
  - 似乎是的，至少 e820 的信息是靠 fw_cfg 传递过去的

- [ ] /home/maritns3/core/kvmqemu/hw/core/loader.c 在干嘛
  - rom_add_file : 加载 bios 的
  - [ ] 除了 bios 之外，还搞了一堆 rom 例如 linuxboot_dma.bin 之类的，都是什么作用，如何确保自己找到的就是全部的


## 细节的 mark
- [ ] apic_id_limit : 初始化中老是处理的 apci id 是啥 ?
  - [ ] 还有，内核其实也可以不提供 acpi (应该使用 kvmqemu trace 一下)


文件内容的基本分析:
| file              | 行数 | 内容分析                                               |
|-------------------|------|--------------------------------------------------------|
| hw/i386/x86.c     | 1300 | cpu_hotplug / pic / x86_machine_class_init             |
| hw/i386/pc.c      | 1700 | 处理 Machine 相关的初始化，例如 hpet, vga 之类的       |
| target/i386/cpu.c | 7000 | X86CPU 相关，主要处理的都是 PC 的 feature 之类的       |
| hw/i386/pi_piix.c | 1000 | pc_init1 剩下的就是 DEFINE_I440FX_MACHINE 定义的东西了 |


定义的各种 type info
| variable          | location         | desc                                                                                                 |
|-------------------|------------------|------------------------------------------------------------------------------------------------------|
| x86_cpu_type_info | target/i386/pc.c | x86_cpu_initfn : 调用一些 object_property_add 和 accel_cpu_instance_init(这是 tcg 和 kvm 分叉点之一) |
| pc_machine_info   | hw/i386/pc.c     | pc_machine_initfn : 初始化一下 PCMachineState, pc_machine_class_init : 实际上，                                 |

到底初始化什么内容:
| item | necessary          | desc                                                                                     |
|------|--------------------|------------------------------------------------------------------------------------------|
| e820 | :heavy_check_mark: | - [ ] 为什么有了 acpi 还是需要 e820 啊，当使用增加了一个内存条，并没有说非要修改 acpi 啊 |
| apci | :x:                | - [ ] 在 QEMU 和 kernel 中间都存在 CONFIG_ACPI 的选项，也许暂时可以不去管                |
| pci  | :x:                | - [ ] `pcmc->pci_enabled`                                                                |

PCMachineState <- X86MachineState <- MachineState

- [ ] io_mem_unassigned 的引用位置扑朔迷离

- [ ] 这些路径中间，为什么没有看到 pcie 相关的初始化

- qemu_init : 这里面存在很长的参数解析的内容
  - qemu_create_machine(select_machine()) : select_machine 中获取 MachineClass
    - cpu_exec_init_all :
      - [ ] io_mem_init : 初始化 io_mem_unassigned
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
                        - tcg_exec_realizefn
                          - tcg_x86_init: 这是 CPUClass 上注册的函数，进行一些 tcg 相关的的初始化, 例如 regs
                      - x86_cpu_expand_features
                      - x86_cpu_filter_features
                      - [ ] mce_init : 根本不知道干啥的
                      - qemu_init_vcpu : 创建执行线程
                      - x86_cpu_apic_realize 
                      - X86CPUClass::parent_realize : 也就是 cpu_common_realizefn, 这里并没有做什么事情
          - pc_memory_init : 创建了两个mr alias，ram_below_4g 以及ram_above_4g，这两个mr分别指向ram的低 4g 以及高 4g 空间，这两个 alias 是挂在根 system_memory mr下面的
            - e820_add_entry
            - pc_system_firmware_init : pflash 参考 [pflash](#pflash)
              - x86_bios_rom_init : 不考虑 pflash, 这是唯一的调用者
                - memory_region_init_ram(bios, NULL, "pc.bios", bios_size, &error_fatal)
                - rom_add_file_fixed
                - 还有两个 memory region 的操作, 将 bios 的后 128KB 映射到 ISA 空间，但是 bios 的大小是 256k 啊，其次，为什么映射到 pci 空间最上方啊
                  - [ ] map the last 128KB of the BIOS in ISA space
                  - [ ] map all the bios at the top of memory
            - memory_region_init_ram : 初始化 "pc.rom"
            - fw_cfg_arch_create : 创建 `FWCfgState *fw_cfg`, 并且初始化 e820 CPU 数量之类的参数, 具体参考 [fw_cfg](./fw_cfg.md)
            - rom_set_fw : 用从 fw_cfg_arch_create 返回的值初始化全局 fw_cfg
            - x86_load_linux : 如果指定了 kernel, 那么就从此处 load kernel
            - rom_add_option : 添加 rom 镜像，关于 rom 分析看 [loaer](#loader)
          - pc_guest_info_init
              - qemu_add_machine_init_done_notifier
          - smbios_set_defaults : 参考 [smbios](#smbios)
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
            - [ ] init_pam : https://wiki.qemu.org/Documentation/Platforms/PC
          - piix3_create
            - pci_create_simple_multifunction : 创建出来设备
            - 设置从 piix3 到 i440fx 的中断路由之类的事情
          - [ ] isa_bus_irqs
          - [ ] pc_i8259_create
          - [ ] ioapic_init_gsi
          - [ ] pc_vga_init
            - pci_vga_init
            - isa_vga_init
          - [ ] pc_basic_device_init
            - ioport80_io 初始化
            - ioportF0_io 初始化
            - hpet 初始化
            - mc146818_rtc_init
            - i8254_pit_init
            - i8257_dma_init
            - pc_superio_init
          - pci_ide_create_devs
            - ide_drive_get
            - ide_create_drive
          - [ ] pc_cmos_init
            - 多次调用 rtc_set_memory
          - piix4_pm_init : 当支持 acpi 的时候, 那么初始化电源管理
    - qemu_create_cli_devices
      - soundhw_init
      - parse_fw_cfg : 解析参数 -fw_cfg (Add named fw_cfg entry with contents from file file.)
      - usb_parse
      - device_init_func : 解析参数 -device 比如 nvme
    - qemu_machine_creation_done : 在 vn/hack/qemu/internals/seabios.md 中展示了从这里一直到 i8042_build_aml 的过程
      - qdev_machine_creation_done
        - notifier_list_notify : 这会调用 pc_machine_done, 实际上，注册到 machine_init_done_notifiers 上的 notifier 只有这一个而已
          - pc_machine_done
            - [ ] x86_rtc_set_cpus_count 
            - [ ] fw_cfg_add_extra_pci_roots 
            - acpi_setup
            - [ ] 似乎是可以 disable fw_cfg 的

```c
/*
#0  cpu_common_initfn (obj=0x555555e64ab3 <object_init_with_type+96>) at ../hw/core/cpu-common.c:237
#1  0x0000555555e64ab3 in object_init_with_type (obj=0x555556c89f00, ti=0x555556852de0) at ../qom/object.c:375
#2  0x0000555555e64a95 in object_init_with_type (obj=0x555556c89f00, ti=0x55555686beb0) at ../qom/object.c:371
#3  0x0000555555e64a95 in object_init_with_type (obj=0x555556c89f00, ti=0x55555687da00) at ../qom/object.c:371
#4  0x0000555555e64a95 in object_init_with_type (obj=0x555556c89f00, ti=0x55555687df20) at ../qom/object.c:371
#5  0x0000555555e6500e in object_initialize_with_type (obj=0x555556c89f00, size=42944, type=0x55555687df20) at ../qom/object.c:517
#6  0x0000555555e65743 in object_new_with_type (type=0x55555687df20) at ../qom/object.c:732
#7  0x0000555555e657a2 in object_new (typename=0x55555687e0a0 "host-x86_64-cpu") at ../qom/object.c:747
#8  0x0000555555b67369 in x86_cpu_new (x86ms=0x555556a94800, apic_id=0, errp=0x5555567a94b0 <error_fatal>) at ../hw/i386/x86.c:106
#9  0x0000555555b67485 in x86_cpus_init (x86ms=0x555556a94800, default_cpu_version=1) at ../hw/i386/x86.c:138
#10 0x0000555555b7b69b in pc_init1 (machine=0x555556a94800, host_type=0x55555609e70a "i440FX-pcihost", pci_type=0x55555609e703 "i440FX") at ../hw/i386/pc_piix.c:157
#11 0x0000555555b7c24e in pc_init_v6_1 (machine=0x555556a94800) at ../hw/i386/pc_piix.c:425
#12 0x0000555555aec313 in machine_run_board_init (machine=0x555556a94800) at ../hw/core/machine.c:1239
#13 0x0000555555cdada9 in qemu_init_board () at ../softmmu/vl.c:2526
#14 0x0000555555cdaf88 in qmp_x_exit_preconfig (errp=0x5555567a94b0 <error_fatal>) at ../softmmu/vl.c:2600
#15 0x0000555555cdd660 in qemu_init (argc=28, argv=0x7fffffffd7c8, envp=0x7fffffffd8b0) at ../softmmu/vl.c:3635
#16 0x000055555582e575 in main (argc=28, argv=0x7fffffffd7c8, envp=0x7fffffffd8b0) at ../softmmu/main.c:49
```

#### e820
- 信息是如何构造出来的
  - 在  pc_memory_init 调用两次 e820_add_entry, 分别添加 below_4g_mem_size 和 above_4g_mem_size
- 如何通知 guest 内核的 ?
  - 在 fw_cfg_arch_create 中添加 `etc/e820` 实现的


- [ ] 类似 pci 映射的 MMIO 空间的分配是 e820 之类的操作的吗 ?


#### pflash
在 hw/i386/pc_sysfw.c 似乎主要处理的就是 pflash

关于 pflash 和 bios 的关系可以首先看看[^1], 但是目前不需要知道 pflash 也可以

## loader
- [ ] 等待分析 loader.c

## smbios
- [ ] 等待分析 smbios.c

smbios_set_defaults

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


[^1]: https://wiki.qemu.org/Features/PC_System_Flash
