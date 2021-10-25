# Interface
1. address_space_stl_notdirty
    - 调用 address_space_stl_notdirty 的时候是在处理 page table, 因为 BMBT 不处理 migration， 只处理 SMC，所以哪里没用
3.  memory_access_is_direct
4.  memory_region_get_ram_ptr
    - 没有令人窒息的 alias 之类的，直接返回 qemu_map_ram_ptr 就可以了
7.  io_readx / io_writex
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
33. run_on_cpu
    - run_on_cpu 需要需要等待到 vCPU 将 hook 指向完成之后才会继续，其用户在 BMBT 模式下都消失了，这个函数直接被移除掉了
    - 但是 qemu_work_cond 暂时保留下来
34. iotlb
    - QEMU 本身存在 subpage 这让将 IO 访问也放到 TLB 中，但是现在 subpage 机制取消掉了，所以无法继续使用 TOTLB 机制了
    - 删除 memory_region_section_get_iotlb
    - 删除 iotlb_to_section
    - 向 iotlb 中本来装入的是 physical section number, 现在装入的是 magic
    - 原本 io 的 CPUTLBEntry::addend 装入的是 0，现在需要装入 paddr
    - io_readx 和 io_writex 从 CPUTLBEntry::addend 装入的 paddr 重新 address_space_translate
    - 在 address_space_translate_for_iotlb 中，如果是 ram，那么返回一个 iotlb_mr 就可以了

# BMBT 的说明
我发现，不要将原来的代码递归的拷贝过来，而是整个代码都拷贝过来，然后使用 `#if` 逐个 disable 掉。
应该在每一个 BMBT 跟上解释为什么不需要这个东西
