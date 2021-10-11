# Interface

1. address_space_stl_notdirty
    - 调用 address_space_stl_notdirty 的时候是在处理 page table, 因为 BMBT 不处理 migration， 只处理 SMC，所以哪里没用
2.  address_space_translate_for_iotlb
    - 之后直接返回 MemoryRegionSection 的
3.  memory_access_is_direct
4.  memory_region_get_ram_ptr
    - 没有令人窒息的 alias 之类的，直接返回 qemu_map_ram_ptr 就可以了
5.  qemu_map_ram_ptr
    - 只是一个非常阴间的函数，addr 可以是 RAM 内偏移，也同时可以是 ram_addr, 但是 BMBT 中其调用者可以保证一定是 ram 内偏移
6.  iotlb_to_section
7.  CPUAddressSpace
    - 被简化，CPUState 还是持有一个 AddressSpace 的，但是只是靠两个 CPUAddressSpace。
8.  AddressSpace 直接持有的 MemoryRegion
9.  memory_region_clear_dirty_bitmap
    - 这是给 migration 使用的，没有必要
10. RAMList::mru_block
    - 暂时保留 RAMList 的概念，但是因为 RAMList 现在只有一个 RAM, 所以 mru_block 就是那个一，不存在对应的 list 了
11. BQl
    - 因为暂时只有一个 thread，所以 BQL 无需上锁了，现在只有逻辑功能的模拟
12. QemuSpin 和 QemuMutex
    - 采取和 BQL 类似的做法
13. start_tcg_kick_timer
    - 暂时只是支持一个 guest vCPU 的，所以无需 timer 来 kick 实现 vCPU 的轮转
14. cpu_can_run, CPUState::stop, CPUState::stopped
    - stop 适用于 migration 的时候将 CPU stop 的
15. cpu_exec_start / end_exclusive
    - 因为现在只是支持单核，所以暂时不支持这些个操作
16. cpus.c 中的几个 icount 函数
17. i8259_init_chip
    - 没有必要再去构建 ISABus ISADevice 之类的抽象概念了，没有总线的概念了，所有的设备也就是这几个
18. pic_common_realize
    - 统计之类的事情以后再说吧
19. i8259.h
    - 将 pc.h 中和 i8259 相关的代码移动到 i8259.h 中
20. CPUArchId::cpu
    - 为了保持 X86_CPU 的语义一致性，当然也没有实现 Object 类型，将类型修改为 CPUState 了
21. pc_memory_init
    - 在这里设置了众多的 memory region 的映射问题
      - 在 pc_init1 中处理了 PCI 相关的地址空间
      - 在 x86_bios_rom_init 处理 bios 相关的空间
    - [ ] 整个 pc_memory_init 实际上被过多的删除了，需要重新 review 一下，而且 pc.ram 之类的东西还是重新在这里添加的
22. pc_system_firmware_init
    - 因为不支持 pflash 所以这个函数被简化为直接调用 x86_bios_rom_init 了
23. smp_parse
    - 虽然不支持 smp_parse，但是保留空的接口
24. machine_class_init 中的定义的 MachineClass property
    - 最后这个玩意儿只是在 machine_class_init 中通过 machine_set_property 设置，但是这些变量都是写死的
25. hotplug.h
    - hotplug 的使用位置只有两个地方，pc.c 和 acpi.h，需要 plug 的总是 CPU
    - hotplug.c 在 qdev_realize 的位置手动显式的调用
    - 为了让 HotplugHandler * 可以装换为 DeviceState 或者是 PCMachineState 之类的，需要让 PCMachineState 内部持有一个 HotplugHandler
    - 然后 HotplugHandler 再持有 HotplugDeviceClass * 并且手动检查
26. ram_size
    - 因为 `current_machine->ram_size` == ram_size 的，所以在 fw_cfg_arch_create 直接替换掉
    - [ ] 虽然暂时没有完全移除掉，但是我希望之后彻底移除掉这个 ram_size，通过 `qdev_get_machine()->ram_size` 代替
27. page_size_init
    - 初始化的 qemu_host_page_size / qemu_host_page_mask 的两个变量实际上没有作用
    - [ ] 是不是因为没有移植 cross page 相关的代码，所以暂时无需 host page size 是多少?
28. check_exception
    - 如果检测出来 triple fault，那么会导致整个机器重置，这种复杂的情况暂时不是我们能够处理的
29. log.c / log.h
    - 在原本的 QEMU 中，划分出来了 qemu-printf.c qemu/log.h exec/log.h
    - 现在将所有的 .c 都合并到 log.c 中
    - qemu/log.h exec/log.h 两个合并到 qemu/log.h 中
30. target/i386/X86toMIPS/tests/run_one_tb.c
    - 虽然暂时打开了 CONFIG_XTM_TEST，但是实际上 run_one_tb.c 等用户没有移植进来
31. include/qemu/atomic.h
    - 当前只是支持单核，所有的 atomic 操作都是简化的
32. cpu_interrupt
    - 原来通过 cpu_interrupt_handler 全局变量赋值的设计过于鬼畜和充满误导性，让其直接调用 tcg_handle_interrupt

# 几个 macro 的说明
我发现，不要将原来的代码递归的拷贝过来，而是整个代码都拷贝过来，然后使用 `#if` 逐个 disable 掉。

- BMBT : 一定不需要，应该在每一个 BMBT 跟上解释为什么不需要这个东西
- NEED_LATER : 将来应该需要
- MEM_TODO : 暂时没有移植的，和 MEM 相关的
- RTC_TODO : 关于 rtc 的代码
- SMBIOS_TODO :


#!/bin/sh
. "$(dirname "$0")/_/husky.sh"

npx --no-install commitlint --edit ""
