# 整体的计划分析

## 问题
- [ ] 所以还是使用 e820 来实现探测内存吗? 那么 acpi 是做什么用的?
- [ ] get_boot_devices_list 的返回结果是 /rom@genroms/linuxboot_dma.bin，通过这种方式直接启动 Linux，如果是从 disk 上启动 guest 的时候，那么其返回为 NULL
  - 所以，到底用什么方法来加载 guest 的啊

## 将用户态的依赖彻底移除掉
- setsigjmp 这种东西在内核态可以使用吗?
- 如何保证自己对于 glibc 没有依赖?

## 在 bare-mental 上测试 seabios 的执行
- 添加设备和中断支持
- 了解 UEFI SDK
- https://github.com/tianocore/tianocore.github.io
- apci probe
- cache init
- interrupt init
- TLB refill init
- memory probe
- probe pci devices

## 处理的问题

## how to port clock
- [ ] 为什么制作出来了 timer_list_group 的概念
- [ ] 全局变量的 rtc_clock 正确初始化，其中影响是什么?

## 需要处理的大块问题
```plain
src/hw/i386/fw_cfg.c:33:#if SMBIOS_TODO
```

```plain
src/hw/i386/pc_piix.c:133:#if NEED_LATER
src/hw/i386/pc_piix.c:156:#if NEED_LATER
src/hw/i386/pc_piix.c:219:#if NEED_LATER
src/hw/i386/fw_cfg.c:101:#ifdef NEED_LATER
src/hw/i386/fw_cfg.c:111:#ifdef NEED_LATER
src/i386/helper.c:397:#ifdef NEED_LATER
src/hw/intc/ioapic.c:398:#ifdef NEED_LATER
```
应该是这里有事情被简化掉了，所以现在 x86_cpu_realizefn 中没有初始化 features 字段
如果上面的 NEED_LATER 都修复了，但是问题还是没有解决，那么就出现了大问题了

## 需要
- [ ] x86_cpu_apic_realize : 向 memory 中添加 apic 的映射空间，这个是需要测试的
- [ ] 添加一个 assert 直接到最后比较 feature 是否相等的，但是这个需要 i386 的数据
- [ ] x86_cpu_properties 中的项目需要一个个的检查一遍
- [ ] 核查一下最后的 tlb flush 的事情是积累下来的
- [ ] disass 的调试感觉还是有必要的呀
- [ ] 比如 rtc_reset 中会调用一下 qemu_irq_lower，这种在 qemu_init 的时候导致的中断到时候还是需要分析一下的呀

## 还需要分析的细节问题
2. apic
    1. DeviceState 中定义为空
    2. /home/maritns3/core/ld/DuckBuBi/include/hw/i386/apic.h 都是空函数
    3. cpu_get_pic_interrupt 定义在 pc.c 中间了
    4. qemu_irq_raise : 在 fpu_raise_exception 中需要被调用
    5. hw/irq.h 中的东西似乎只会被 fpu_helper 使用，其他的位置在哪里呀
    6. tcg-all.c : 涉及到初始化 tcg engine, 很短的一个文件
        - 虽然主要是处理 interrupt 的，也是很麻烦的啊
    7. /home/maritns3/core/ld/DuckBuBi/src/i386/excp_helper.c 中为什么有那么多的处理 exception 的函数
    8. qemu_irq_raise
    9. 在 /home/maritns3/core/vn/docs/qemu/lock.md 的 interrupt_request 的那个 section 并没有理清楚到底如何注入 interrupt 和处理的
    10. ipi 是如何模拟的?
    11. 在 i386/cpu.h 中的 cpu_report_tpr_access 和 apic_handle_tpr_access_report 暂时无人使用的
4. icount 机制
    1. cpu_exec
    2. TranslationBlock::icount
    3. 在 CPUX86State::neg 似乎也有这个东西
    4. 在 /home/maritns3/core/ld/DuckBuBi/include/sysemu/replay.h 中间定义一些 reply 相关的函数，和 icount 到底是什么关系
    5. CF_USE_ICOUNT ?
    6. use_icount : 这个变量的解释似乎很清晰，不过可以和具体的效果对比一下
    7. can_do_io : lixinyu 说这个用于实现 icount 的，如果被 icount 那么就不可以直接进行 io 的之类的，阿巴阿巴
6. 处理一下满天飞羽的各种 type 定义
    1. ExtraBlock 是 LATX 中特殊需求的，其中 `_top_in` 的类型是 int8, 而 int8 又是定义在 src/i386/LATX/include/types.h 中间了，着很难受啊
    2. HOST_LONG_BITS 和 TARGET_LONG_BITS 的定义位置
    3. target_long 的定义
    4. hwaddr 的定义
8. 非常不统一的 assert
  - [ ] tcg_debug_assert
  - [ ] assert
  - [ ] 一些 unreachable 之类的
  - [ ] tcg_abort
  - [ ] 在 bitmap.c 中间是直接从
  - [ ] g_assert
  - [ ] error_report
9. pit 和 hpet 需要模拟?
