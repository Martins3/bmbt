# QEMU 启动代码

- [ ] 都做了什么事情，初始化了什么环境啊

- [ ] KVM 和 tcg 的初始化的差别是在调用那些函数的时候体现的

- [ ] 我们需要 qemu 的 acpi 机制来实现 acpi table 的组装啊 ?
- [ ] 为了让 bios 可以正确运行，一定需要 fw_cfg 吗 ?

- [ ] 定义在 target 下的 pc.c 和 hw 下的 pc.c 存在什么关联


| file             | 行数 | 内容分析                                   |
| hw/i386/x86.c    | 1300 | cpu_hotplug / pic / x86_machine_class_init |
| hw/i386/pc.c     | 1700 | pc_machine_info                            |
| target/i386/pc.c | 7000 |                                            |

定义的各种 type info

| variable          | location         | desc                                                                                                 |
|-------------------|------------------|------------------------------------------------------------------------------------------------------|
| x86_cpu_type_info | target/i386/pc.c | x86_cpu_initfn : 调用一些 object_property_add 和 accel_cpu_instance_init(这是 tcg 和 kvm 分叉点之一) |
| pc_machine_info   | hw/i286/pc.c     | pc_machine_initfn : 初始化一下 PCMachineState                                                        |



这些玩意儿都是什么时候初始化的 ?
- [ ] X86MachineState
- [ ] MachineState
- [ ] MachineClass
  - [ ] select_machine 的
- [ ] PCMachineState
- [ ] PCMachineClass

- [ ] io_mem_unassigned 的引用位置扑朔迷离


pc_memory_init

一路向下的分析一下:

- qemu_init : 这里面存在很长的参数解析的内容
  - qemu_create_machine(select_machine()) : select_machine 中获取 MachineClass
    - cpu_exec_init_all :
      - [ ] io_mem_init : 初始化 io_mem_unassigned
      - memory_map_init : 初始化 system_memory, 和 io_memory 这两个都是 container 并不会真正的分配的映射空间
    - page_size_init : 初始化之后的 softmmu 需要的内容
  - qmp_x_exit_preconfig
    - qemu_init_board
      - machine_run_board_init
        - `machine_class->init` : DEFINE_I440FX_MACHINE 这个封装出来 pc_init_v6_1 来调用
          - pc_init1
            - x86_cpus_init
              - x86_cpu_new
                - qdev_realize : 经过 QOM 的 object_property 机制，最后调用到 device_set_realized :
                  - device_set_realized : 
                    - x86_cpu_realizefn
                      - cpu_exec_realizefn
                        - accel_cpu_realizefn
                          - tcg_cpu_realizefn
                            - cpu_address_space_init
                              - memory_listener_register
            - pc_memory_init : 创建了两个mr alias，ram_below_4g 以及ram_above_4g，这两个mr分别指向ram的低4g以及高4g空间，这两个alias是挂在根system_memory mr下面的
    - [ ] qemu_create_cli_devices
    - qemu_machine_creation_done : 在 vn/hack/qemu/internals/seabios.md 中展示了从这里一直到 i8042_build_aml 的过程


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

```c
    MachineClass *machine_class = MACHINE_GET_CLASS(machine);
```

