# QEMU 是如何处理 interrupt 和 exception 的

- 在 cpu_exec 中，为什么实现 while 检查 exception, 然后 while interrupt
  - 中断的检测总是发生 tb 最后的位置，如果检查发现了中断，那么从 tb 山下文离开就可以了，然后就处理中断
  - 在中断处理中，存在一堆返回 true 的情况，这让 while interrupt 失败，这就是为什么 while exception 在外层

- cpu_handle_exception
  - x86_cpu_do_interrupt

- cpu_handle_interrupt
  - 首先处理和架构无关的，类似 halt reset 之类的
  - 通过 CPUClass 进入 x86_cpu_exec_interrupt
      - x86_cpu_pending_interrupt : 需要分析出来真正的 interrupts, 因为可能屏蔽了
      - do_interrupt_x86_hardirq : 处理函数各种情况

- 在 tcg_qemu_tb_exec 的位置发生跳转, 利用变量 context_switch_bt_to_native 来进行

## 问题
1. exception_index 和负数的关系是什么 ?
    - 从 cpu_loop_exit_noexc 的注释来说，似乎是离开的原因没有什么的时候的，就赋值为 -1

2. 如果 cpu_handle_exception 失败之后，这会导致 cpu_exec 函数退出，之后如何处理 ?
    - 退出的原因应该都是外部原因，`cpu->exception_index >= EXCP_INTERRUPT` 和  reply ，目前似乎不用过于考虑
