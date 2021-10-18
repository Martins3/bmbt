## 主要需要分析的问题

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
