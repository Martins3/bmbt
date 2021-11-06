既然需要 QemuEvent QEMUTimerList::timers_done_ev 来同步，那么说明两者实际上不是 BQL 互斥的 ?
- qemu_clock_enable : 需要持有 BQL 的呀
- 但是 qemu_clock_run_all_timers 也是持有 BQL 的
  - 实际上，iothread 不会的

- [ ] 将所有的 timer 检查一下，看看其 callback 都是啥

- [ ] 本身 QEMU 是如何实现向 timer 中添加项目的，靠什么保护的
  - 需要保护的吧
    - 其实，一般来说，哪里是 callback 函数，update timer 的位置
      - vCPU thread 会调用 callback 函数吗? 应该不会吧！ 都是 new_ns 之类的，那是靠什么触发第一次的呀！
    - 是通过 QEMUTimerList::active_timers_lock 进行保护的

当前的设计模型的:
- handler :
  - [ ] 屏蔽信号的方式不好吗? 这样和屏蔽的信号的方法相同
  - [ ] active_timers_lock 也去采用信号屏蔽的方法可以吗 ?
    - [ ] 可以保证持有 active_timers_lock 的时候，实际上，总是持有 BQL 的吗?
      - 从原则上，修改 timer 一般都是 MemoryRegionOps 中
      - [ ] 在 QEMU 只要持有 BQL main loop 才会执行 qemu_clock_run_all_timers 而 MemoryRegionOps 也是持有 BQL 的，看来还是存在其他的位置进行修改这些东西

- [ ] 除了 active_timers_lock 多出来的 lock 还有别的吗?
  - QemuSeqLock vm_clock_seqlock;
  - QemuSpin vm_clock_lock;
  - 分别用于 cpu_get_clock 和 cpu_get_ticks 的访问上，我认为和 active_timers_lock 类似的作用


- 在 timer_del_locked 和 timer_mod_ns_locked 中添加上，qemu_mutex_iothread_locked 检测，"当前配置" 下，执行到这些函数，BQL 总是有的
- 这是 QEMU 的失误吗?
    - 不是的，因为 iothread 中也是可以访问 timerlist 的
    - 从 978f2205c791de0e02c8802a645bea657408abfd 中看，应该也是差不多的

- 如何处理那种同时被两个地方调用的代码:
  - periodic_timer_update 这种东西既会被 main loop 调用，
  - 有的修改变得好奇怪呀

- 注入 timer 中断的方法: 从 apic_timer => apic_local_deliver => cpu_interrupt => tcg_handle_interrupt
- kick_cpu 和 cpu_exit 的区别: kick_cpu 回去调用 cpu_exit，并且同时设置 CPUState::exit_request
- main loop 的执行会等待 vCPU 释放 BQL 的，而 vCPU 为了保证中断的及时响应，需要在每一个 tb 结束的位置检查，从而退出 tb 的执行，来处理中断
- 在用户态中，为了去掉在每一个 tb 后面的中断检查，采用信号的方法，但是信号的 handler 无法知道自己自己是从哪一个 tb 上离开的

## 还是使用 qemu_clock_run_all_timers 机制吗
1. 使用 timer_create 对于每一个 timer 创建一个，分别注册自己的函数 ?
2. 使用 qemu_clock_run_all_timers 可以只是创建出来一个 timer

使用方案二，比较节省物理 timer, 扩展性比较好，而且代码都写好了，不用白不用。

## 使用 signal 而不是 thread 的方式

## 确定只是处理 timer 吗
如果 main loop 需要接受 vCPU thread 的任务，那整个设计就太复杂了吧。
现在处理机制也是只能给 tiemr 服务的呀!

## 条件变量整个机制是错误的
qemu_cpu_kick 的设计: 因为 halt 指令 vCPU thread 可以卡到 qemu_tcg_rr_wait_io_event 上了

- qemu_cond_broadcast 和 qemu_cond_wait 的设计有问题的
  - 因为 qemu_cpu_kick 可以反复的 qemu_cond_broadcast 的
  - 正确的操作是，直接取消掉，halt_cond 的机制

## qemu_cpu_is_self
- apic_update_irq : 主要是因为 apic_sync_vapic 需要参数 APICCommonState 那是一个 cpu local 的属性
- tlb flush : 因为访问了 CPU 私有的数据

显然，在进行这些私有数据访问的时候，显然是不可以到来 signal 的

如果允许 qemu_cpu_is_self 总是返回 true, 那么就在各种地方都是需要进行屏蔽了中断了。

所以 qemu_cpu_is_self  在 signal 中应该返回 false

## 单核和 signal 的编程差异
- BQL 可以保证"同时"只有一个 CPU 在执行
- 持有 BQL 表示屏蔽中断 / 或者信号屏蔽

## 细节
需要补全的实现:
- [ ] resume_all_vcpus
- [ ] qemu_clock_enable
- [ ] qemu_clock_deadline_ns_all
- [ ] QEMUClock / QEMUTimerListGroup 的构建
