## 主要需要分析的问题
5. 关键的接口和重构

| TODO                       | 描述                                                                                                                                                                                           |
|----------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| apic                       | include/hw/i386/apic.h 中定义了一些列的函数，具体的还没有分析，但是 原来的 QEMU 中间                                                                                                           |
| 多核                       | 现在为了代码的方便执行，也没有太搞清楚其中的作用，在 cputlb.c 的 async_run_on_cpu 以及各种 atomic 函数，qemu_spin_lock, 以及 RCU. 现在的操作是，先将接口保存下来，之后需要支持多核，有一个参考 |
| icount / record replay     | 没有 record replay 是不是很难调试，使用 record replay 会不会很难集成                                                                                                                           |
| trace                      |                                                                                                                                                                                                |
| 存在好几个数据结构需要重构 |qht.h 和 glib 的 qtree                                                                                                                                                                |

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
5. log debug 和 trace : 其实暂时可以补全的, 都是一些 printf 而已
    1. tlb_debug
    2. do_tb_flush 中间又是直接使用 printf 的
    7. qemu_log_mask_and_addr : 和 qemu_log_mask 的区别在于只有在地址范围内才会打印日志
6. 处理一下满天飞羽的各种 type 定义
    1. ExtraBlock 是 LATX 中特殊需求的，其中 `_top_in` 的类型是 int8, 而 int8 又是定义在 src/i386/LATX/include/types.h 中间了，着很难受啊
    2. HOST_LONG_BITS 和 TARGET_LONG_BITS 的定义位置
    3. target_long 的定义
    4. hwaddr 的定义
7. log printf 和 qemu printf
    1. include/qemu/log-for-trace.h 和 include/qemu/log-for-trace.h : 一起搞出来 mask 基于日志的角度
    2. qemu_printf 和 printf 的区别在于当前是否存在 monitor
8. 非常不统一的 assert
  - [ ] tcg_debug_assert
  - [ ] assert
  - [ ] 一些 unreachable 之类的
  - [ ] tcg_abort
  - [ ] 在 bitmap.c 中间是直接从
  - [ ] g_assert
  - error_report
9. pit 和 hpet 需要模拟?

src/hw/core/machine.c:143:  // FIXME this is hard coded, I don't know why MachineClass name this one
src/hw/core/machine.c:145:  // FIXME compat_props
src/hw/core/machine.c:159:  // FIXME check it, we don't support nvdimm
src/hw/core/machine.c:211:    // FIXME I don't know why possible_cpus's length is empty
src/hw/core/machine.c:230:    // FIXME in x86 types
src/hw/core/cpu.c:22:  // FIXME call the realize function here
src/hw/core/cpu.c:92:// FIXME does we need cpu_reset
src/hw/intc/ioapic.c:226:          // FIXME :linker: timer ???
src/hw/intc/ioapic.c:402:  // FIXME :linker: timer
src/hw/i386/fw_cfg.c:29:// FIXME port bios
src/hw/i386/x86.c:156:// FIXME As for how to boot kernel, it's not clear
src/hw/i386/x86.c:689:  // FIXME port it
src/i386/cpu.h:1474:  // FIXME put here randomly, fix it later
src/i386/cpu.h:1483:  // FIXME put here randomly
src/i386/cpu.h:1490:  // FIXME feild are put here randomly, we will fix them
src/i386/cpu.h:1644:  // FIXME it will be removed, no qdev anymore
src/i386/cpu.h:1685:  // FIXME we will rethink realize at:
src/i386/cpu.h:2150:// FIXME should be empty, verify it
src/i386/cpu.h:2166:// FIXME the only user is test-def.h
src/hw/nvram/fw_cfg.c:950:  // FIXME init the memory region
src/hw/nvram/fw_cfg.c:955:    // FIXME init the memory region
src/i386/LATX/x86tomips-options.c:656:        // FIXME
src/i386/LATX/optimization/flag_reduction.c:7:// FIXME maybe this is bug of original QEMU
src/i386/LATX/x86tomips-config.c:25:// #include "trace.h" FIXME comment it temporary
src/i386/LATX/x86tomips-config.c:95:        // FIXME how etb works ?
src/i386/LATX/x86tomips-config.c:169:// FIXME this is called after tb_find
src/i386/LATX/x86tomips-config.c:818:// FIXME comment this, vregs defined in CPUX86State ?
src/i386/LATX/x86tomips-config.c:897:// FIXME comment the code related with signal handler
src/i386/LATX/x86tomips-config.c:928:// FIXME this is a temporary fix
src/i386/LATX/x86tomips-config.c:931:// FIXME no more signal anymore
src/i386/cpu.c:1365:  // FIXME
src/i386/cpu.c:1370:  // FIXME
src/i386/cpu.c:1381:  // FIXME
src/i386/cpu.c:1436:  // FIXME
src/i386/cpu.c:1574:  // FIXME anchor : maybe we will remove the version system
src/i386/cpu.c:1578:  // FIXME anchor : maybe we will remove the version system
src/i386/cpu.c:1584:  // FIXME anchor : maybe we will remove the version system
src/i386/cpu.c:2047:  // FIXME how to port property?
src/i386/cpu.c:2055:  // FIXME how to port property?
src/i386/cpu.c:2059:  // FIXME how to port property?
src/i386/cpu.c:3038:// FIXME
src/i386/cpu.c:3586:  // FIXME when will QEMU call unrealizefn?
src/i386/cpu.c:3756:  // FIXME last three property are never used
src/i386/cpu.c:3979:// FIXME check this one by one
src/i386/cpu.c:4153:  // FIXME why we need user_creatable?
src/i386/cpu.c:4162:  // FIXME when did the newly allocated CPU passed to exec thread ?
src/tcg/cputlb.c:47:// FIXME just remove the content to avoid error
src/tcg/cputlb.c:80:// FIXME i don't know why we need time
src/tcg/cputlb.c:843:  // FIXME remove btmmu related code
src/tcg/cpu-exec-common.c:38:    // FIXME rethink the code later
src/tcg/cpu-exec.c:83:  // FIXME why xqm need extra hacking for this?
src/tcg/cpu-exec.c:196:  // FIXME
src/tcg/cpu-exec.c:277:  // FIXME deeper understanding how tb expands to second pages
src/tcg/cpu-exec.c:478:  // FIXME how SyncClocks works
src/tcg/cpu-exec.c:500:    // FIXME some checks for sigsetjmp bugs, review it later
src/tcg/cpu-exec.c:538:      // FIXME icount related
src/tcg/translate-all.c:29:// FIXME copied from /usr/include/x86_64-linux-gnu/bits/mman-linux.h
src/tcg/translate-all.c:46:// FIXME copied from  util/cacheinfo.c
src/tcg/translate-all.c:55:// FIXME why xqm take special attention cross page
src/tcg/translate-all.c:1047:  // FIXME is mips restriction still valid ?
src/tcg/translate-all.c:1081:  // FIXME
src/tcg/translate-all.c:2122:  // FIXME we should define this
src/hw/intc/apic.c:755:// FIXME put msi_nonbroken here temporarily, maybe we never need it
src/hw/intc/ioapic_common.c:162:  // FIXME call realize functions
include/qemu/bitmap.h:24:  // FIXME interface
include/qemu/osdep.h:26:// FIXME defined in util/cacheinfo.c
include/qemu/osdep.h:41:// FIXME get the page size, that's really easy
include/qemu/osdep.h:66:// FIXME
include/qemu/log-for-trace.h:6:// FIXME a fix, review this file later
include/qemu/config-host.h:161:// FIXME
include/qemu/qemu-printf.h:5:  // FIXME actually
include/qemu/atomic.h:4:// FIXME If we want to support multicore
include/hw/i386/pc.h:41:// FIXME copied from include/hw/acpi/pc-hotplug.h
include/hw/i386/pc.h:56:  // FIXME init it
include/hw/i386/pc.h:63:  // FIXME actually, QOM can convert acpi_dev to DeviceState
include/hw/core/cpu.h:76:// FIXME maybe we have copy too many comments here, remove them later
include/hw/core/cpu.h:183:  // FIXME currently, this field is only referenced by tb_lookup__cpu_state
include/hw/core/cpu.h:194:// FIXME clear the comments
include/hw/core/cpu.h:387:// FIXME initialize cpu_interrupt_handler
include/hw/core/cpu.h:428:  // FIXME interface
include/hw/core/cpu.h:458:// FIXME
include/hw/core/cpu.h:534:// FIXME I don't know why x86 doesn't register the handler
include/hw/i386/x86.h:43:  bool apic_xrupt_override; // FIXME what does this mean ?
include/hw/isa/i8259_internal.h:10:  // FIXME can I remove it safely
include/exec/cpu-all.h:17:// FIXME
include/exec/cpu-all.h:130:// FIXME prototypes above are autogenerated, why this one copied  manually
include/exec/cputlb.h:8:void tlb_protect_code(ram_addr_t ram_addr); // FIXME protect code ? understand it
include/exec/cpu-defs.h:101:// FIXME, if QEMU_BUILD_BUG_ON only used one time, create a simpler one
include/exec/exec-all.h:163:// FIXME todo we doesn't supported mttcg, so init it as zero
include/exec/exec-all.h:458:  // FIXME :linker: Please return MemoryRegion instead of MemoryRegionSection
include/exec/memory.h:40:  // FIXME
include/exec/memory.h:44:  // FIXME
include/exec/memory.h:53:  // FIXME port later
include/exec/memory.h:71:  // FIXME port later
include/exec/memory.h:89:  // FIXME port later
include/exec/ram_addr.h:26:  // FIXME add the only RAMBlock to RAMList, check the addr in range and return
include/sysemu/sysemu.h:11:  // FIXME :linker: need more thinking about how to load guest image
include/sysemu/replay.h:7:  // FIXME
include/sysemu/replay.h:11:// FIXME /home/maritns3/core/ld/x86-qemu-mips/build/qapi/qapi-types-misc.h
include/sysemu/replay.h:24:  // FIXME
include/sysemu/replay.h:29:  // FIXME
include/sysemu/replay.h:36:  // FIXME
