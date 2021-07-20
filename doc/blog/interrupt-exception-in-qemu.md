# QEMU 是如何处理 interrupt 和 exception 的

- [ ] 如果不方便，可以使用 x86 来测试一下


检测这两个赋值位置
CPUState::exception_index
CPUState::interrupt_request

- [ ] 在 cpu_exec 中，为什么实现 while 检查 exception, 然后 while interrupt
- [ ] 猜测，exception_index 之类的显然都是发生在
- [ ] 我的印象当中，exception 似乎是和 setjmp 相关的，找到
  - [ ] 上下文的保存
  - [ ] 恢复
- [ ] 为什么需要采用 setjmp 的操作 ？

- cpu_handle_interrupt
  - 首先处理和架构无关的，类似 halt reset 之类的
  - 通过 CPUClass 进入 x86_cpu_exec_interrupt
      - x86_cpu_pending_interrupt : 需要分析出来真正的 interrupts, 因为可能屏蔽了
      - do_interrupt_x86_hardirq : 一些处理函数

## CPUState::exception_index
exception_index 和负数的关系是什么 ?

总体来说，

从 cpu_loop_exit_noexc 的注释来说，似乎是离开的原因没有什么的时候的，
就采用这种。
