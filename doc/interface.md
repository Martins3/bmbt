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
    - 使用 CONFIG_XTM_TEST 会包含进来 run_one_tb.c
    - 启动 x86_to_mips_before_exec_tb 和 x86_to_mips_after_exec_tb 中的调试
    - 暂时不打开
32. cpu_interrupt
    - 原来通过 cpu_interrupt_handler 全局变量赋值的设计过于鬼畜和充满误导性，让其直接调用 tcg_handle_interrupt
33. run_on_cpu
    - run_on_cpu 需要需要等待到 vCPU 将 hook 指向完成之后才会继续，其用户在 BMBT 模式下都消失了，这个函数直接被移除掉了
    - 条件变量的逻辑不能简单的修改为: 前面 boardcast 后面接受的模式, 所以讲牵连的 qemu_work_cond 也需要被一并被删除掉的
34. iotlb
    - QEMU 本身存在 subpage 这让将 IO 访问也放到 TLB 中，但是现在 subpage 机制取消掉了，所以无法继续使用 TOTLB 机制了
    - 删除 memory_region_section_get_iotlb
    - 删除 iotlb_to_section
    - 向 iotlb 中本来装入的是 physical section number, 现在装入的是 magic
    - 原本 io 的 CPUTLBEntry::addend 装入的是 0，现在需要装入 paddr
    - io_readx 和 io_writex 从 CPUTLBEntry::addend 装入的 paddr 重新 address_space_translate
    - 在 address_space_translate_for_iotlb 中，如果是 ram，那么返回一个 iotlb_mr 就可以了
35. QemuEvent QEMUTimerList::timers_done_ev
    - qemu_clock_enable 中需要 timerlist_run_timers 执行完成才可以可以返回，表示 timerlist 确实结束了呀
36. qemu_cpu_is_self
    - 在 QEMU 中，为了其他线程访问直接访问 vCPU 的私有数据，在 tlb flush 和 tcg_handle_interrupt 的地方需要使用上这个操作
    - 现在，这个函数用于区分当前是否运行在 signal handler 中的
    - **感觉** qemu_cpu_is_self 可以被优化为永远返回 true 的
      - tlb flush 发生在 vCPU thread 的，比如 helper, 所以无需考虑
      - interrupt 因为 vapic 的移除，可以简化掉
37. timerlist_notify
    - 如果添加的 timer 当前的 soonest 的 timer 还要早，那么需要尽快执行一下 qemu_clock_run_timers
    - [ ] 在 bare metal 状态下实现，这个问题需要重新思考
39. verify_BQL_held
    - 我们认为 vm_clock_seqlock vm_clock_lock 这两个 lock 在 bmbt 中总是会 vCPU thread 已经持有了 BQL 或者在 signal handler 中调用
    - 所以没有必要添加更多的锁
40. qemu_mutex_lock_iothread_impl
    - 只有在 vCPU thread 中才对于 BQL 上锁，而 signal handler 不能试图去上 BQL 的锁
      - 因为在 QEMU 中，原来的 timer 的 callback 都是在 main loop 中持有 BQL 之后才会去调用的，如果再次上锁，那么是死锁了
    - 所以 tcg_handle_interrupt 也需要进行对应的修改。
41. do_pci_register_device
    - 不支持 hotplug 设备，所以 DeviceState::hotplugged 总是不能为 true
42. i440fx_init
    - i440fx_init 初始化了三个东西
      - i440fx pci host bridge
      - pci bus
      - i440fx pci device
    - 因为 qdev 的设计，在进行 host bridege 的初始化的时候，会自动 realize 关联到上的 pci bus 的
    - pci bus 的 realize 都是一些设置 memory region 的事情，暂时没有什么用途，就不添加了
43. pcie
    - 暂时不支持 pcie 设备的，这导致下面的函数被简化:
      - pci_is_express_downstream_port 直接返回 false
44. pci_bus_num
    - 暂时不支持多个 pci bus ，总是返回 0
45. pci_get_bus
    - 之前是通过 DeviceState::parent_bus 实现的，现在直接在 PCIDevice::bus 中保存
46. pci_update_mappings
    - [ ] 理论上，所有的 PCI 设备都是直通的，所以 PCIIORegion 是不需要我们来管理的
47. PITCommonClass
    - 在 QOM 中，如果 parent 注册了 realize 的 hook，child 重新注册，默认会覆盖 parent 的 hook
    - 有时候，为了让 realize hook 可以向面向对象一样，首先执行 parent 的，然后执行 child 的，可以调用 device_class_set_parent_realize
    - 然后在 child 的 DeviceClass 中添加一个 DeviceRealize parent_realize，感觉有点丑陋，直接调用，结束了。
48. seqlock.h
    - QemuLockable 类型在 seqlock.h 中为了简化实现直接直接修改为 spinlock 类型了
49. gmessages.h
    - 在 glib 原来的实现中，这里只是警告，在移植的实现中，我们认为只要出现了警告，那么就应该 crash 掉
50. slice.c
    - 在 glib/gslice.c 中，为了高性能，实现的库非常的复杂，在这里直接简化为 g_malloc 和 g_free 了
<!-- musl  begin -->
51. `__sigsetjmp`
    - 在 BMBT 中的无需处理信号的问题，所以 `__sigsetjmp` 无需处理信号 mask 的保存问题
52. errno
    - 将 errno 实现为单线程
53. musl malloc glue.h
    - export symbols like malloc free to outside directly
    - so remove useless function name like `__libc_malloc`
54. bits/syscall.h
    - in musl, bits/syscall.h is generated by configure and is located at obj/
    - bmbt put it into include and it's copied from /usr/include/asm-generic/unistd.h
55. realloc
    - realloc 真正需要利用上操作 remalloc 的情况很少，直接拷贝算了
<!-- musl  over -->
56. icache flush
    - 内核中实现是 gcc asm inline 的方法，而 QEMU 的实现方法是 `__builtin___clear_cache`
    - 实际上都会忽略参数 start 和 end，变为 `ibar 0`
    - ibar 的实现参考手册卷一 2.2.8.2
57. 只要保持 ns 和 clock 在任何位置是对应的，暂时可以勉强使用
    - kernel_clock_gettime 中直接返回 drdtime 的数值
    - soonest_interrupt_ns 从 ns 参数直接装换为 cycle 传递给 timer 硬件
58. atomic_common.c.inc
    - 只是 trace 和 plugin 函数，所以清其中的函数内容空就可以了。
    - 并且向其中添加一个 trace_mem_build_info 的空定义
59. 暂时不支持 acpi
    - 龙芯的代码中似乎总是存在不支持 acpi 的那一条道路，为了简化代码，总是直接选择不支持 acpi 的那个道路
60. irqdomain
    - Loongonix 构建了三个 irq domain，以及对应的几个文件 irq-loongarch-extioi.c irq-loongson-pch-pic.c irq-loongarch-cpu.c irq-loongson-pch-msi.c
    - 这实在是太复杂了，我们直接硬编码了
