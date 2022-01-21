- [ ] 如何知道启动的时候，自己是在哪一个 core 上的
  - cpu_probe 似乎对于 smp 可以调用一次，启动的时候调用一次
- [ ] 实现一个操作，当触发各种 exception 的时候，跳转到 exit 上
  - 实现 exception 的时候不要切换 stack，从而可以快速的调试出来 backtrace 的
- [ ] 无法理解 cpu_probe 中为什么需要读取 csr_readq(LOONGARCH_CSR_ASID)
- [ ] set_merr_handler 中的作用是什么?
- [ ] 清理掉 #253
- [ ] 给 kernel_stack 增加一个 canary 防止溢出

## 基本的执行流程
整个初始化的路径奇怪:

- set_tlb_handler : 初始化部分
- trap_init : 初始化所有的


- [x] 6727ad9e206cc08b80d8000a4d67f8417e53539d : 好强的技巧
  - 将 cpu_idle 相关的函数放到一起的

```c
/*
#0  mask_loongarch_irq (d=0x900000027c0bf028) at drivers/irqchip/irq-loongarch-cpu.c:32
#1  0x90000000002a2c94 in handle_percpu_irq (desc=0x900000027c0bf000) at kernel/irq/chip.c:871
#2  0x900000000029c344 in generic_handle_irq_desc (desc=<optimized out>) at ./include/linux/irqdesc.h:155
#3  generic_handle_irq (irq=<optimized out>) at kernel/irq/irqdesc.c:639
#4  0x9000000000fdcec0 in do_IRQ (irq=<optimized out>) at arch/loongarch/kernel/irq.c:103
#5  0x9000000000203414 in except_vec_vi_handler () at arch/loongarch/kernel/genex.S:92
```
## 需要调整的东西
- [ ] 内核什么时候初始化的 ECFG

## 深入理解一下 interrupt 的过程
- SAME_SOME : 如果当前是在用户态，那么需要首先切换到内核 stack 上，否则不用切换

### tiemr 实现需要注意的点

```c
/*
#0  hrtimer_interrupt (dev=0x9000000006000080) at kernel/time/hrtimer.c:1511
#1  0x90000000002084a8 in constant_timer_interrupt (irq=<optimized out>, data=<optimized out>) at arch/loongarch/kernel/time.c:49
#2  0x900000000029d4ac in __handle_irq_event_percpu (desc=0x9000000006000080, flags=0x0) at kernel/irq/handle.c:149
#3  0x900000000029d708 in handle_irq_event_percpu (desc=0x900000027c0bf000) at kernel/irq/handle.c:189
#4  0x90000000002a2c9c in handle_percpu_irq (desc=0x900000027c0bf000) at kernel/irq/chip.c:873
#5  0x900000000029c344 in generic_handle_irq_desc (desc=<optimized out>) at ./include/linux/irqdesc.h:155
#6  generic_handle_irq (irq=<optimized out>) at kernel/irq/irqdesc.c:639
#7  0x9000000000fdcec0 in do_IRQ (irq=<optimized out>) at arch/loongarch/kernel/irq.c:103
#8  0x9000000000203414 in except_vec_vi_handler () at arch/loongarch/kernel/genex.S:92
Backtrace stopped: frame did not save the PC
```
- 在 constant_timer_interrupt 中

```c
	write_csr_tintclear(CSR_TINTCLR_TI);
```

## interrupt
之前一致感觉 interrupt handler 的通用路径上会打开中断，但是实际上没有找到对应的代码。
也就是找到这种位置的 enable 的:

do_IRQ => irq_exit:
```c
/*
#0  tasklet_action (a=0xffffffff824060f0 <softirq_vec+48>) at kernel/softirq.c:805
#1  0xffffffff81e000f4 in __do_softirq () at kernel/softirq.c:558
#2  0xffffffff8106d668 in invoke_softirq () at kernel/softirq.c:432
#3  __irq_exit_rcu () at kernel/softirq.c:637
#4  irq_exit_rcu () at kernel/softirq.c:649
#5  0xffffffff81bcec34 in common_interrupt (regs=0xffffffff82403df8, error_code=<optimized out>) at arch/x86/kernel/irq.c:240
Backtrace stopped: Cannot access memory at address 0xffffc90000004018
```

从 handle_percpu_irq 中，显然存在对于单个 interrupt 的处理过程才对的。j

对于 x86 代码，i8042_interrupt 中，中断还是屏蔽的，所以，实际上，取消屏蔽的情况比我们想的少很多。
```c
/*
#0  i8042_interrupt (irq=12, dev_id=0xffff888003ede400) at ./include/linux/spinlock.h:324
#1  0xffffffff810c2465 in __handle_irq_event_percpu (desc=desc@entry=0xffff8880030f6000, flags=flags@entry=0xffffc90000003f74) at kernel/irq/handle.c:158
#2  0xffffffff810c25dc in handle_irq_event_percpu (desc=desc@entry=0xffff8880030f6000) at kernel/irq/handle.c:198
#3  0xffffffff810c264f in handle_irq_event (desc=desc@entry=0xffff8880030f6000) at kernel/irq/handle.c:215
#4  0xffffffff810c66b6 in handle_edge_irq (desc=0xffff8880030f6000) at kernel/irq/chip.c:822
#5  0xffffffff81022f24 in generic_handle_irq_desc (desc=0xffff8880030f6000) at ./include/linux/irqdesc.h:158
#6  handle_irq (regs=<optimized out>, desc=0xffff8880030f6000) at arch/x86/kernel/irq.c:231
#7  __common_interrupt (regs=<optimized out>, vector=36) at arch/x86/kernel/irq.c:250
#8  0xffffffff81bcec2f in common_interrupt (regs=0xffffc900001a7ac8, error_code=<optimized out>) at arch/x86/kernel/irq.c:240
Backtrace stopped: Cannot access memory at address 0xffffc90000004018
```

## thread info
在 interrupt 的处理路径中，except_vec_vi_handler 中会更新 thread_info::regs

- [ ] 在内核中，tp 存放的是 thread_info 结构体的
```c
/*
 * macros/functions for gaining access to the thread information structure
 */
#define INIT_THREAD_INFO(tsk)			\
{						\
	.task		= &tsk,			\
	.flags		= _TIF_FIXADE,		\
	.cpu		= 0,			\
	.preempt_count	= INIT_PREEMPT_COUNT,	\
	.addr_limit	= KERNEL_DS,		\
}

/* How to get the thread information struct from C. */
register struct thread_info *__current_thread_info __asm__("$r2");

static inline struct thread_info *current_thread_info(void)
{
	return __current_thread_info;
}

/*
 * Initial thread structure. Alignment of this is handled by a special
 * linker map entry.
 */
#ifndef CONFIG_THREAD_INFO_IN_TASK
struct thread_info init_thread_info __init_thread_info = INIT_THREAD_INFO(init_task);
#endif
```

```c
#ifdef CONFIG_THREAD_INFO_IN_TASK
static inline struct thread_info *task_thread_info(struct task_struct *task)
{
	return &task->thread_info;
}
#elif !defined(__HAVE_THREAD_FUNCTIONS)
// 这是满足的配置，这说明，thread_info 在 process 的 stack 上
# define task_thread_info(task)	((struct thread_info *)(task)->stack)
#endif
```
- get_irq_regs 被多个位置调用，但是 set_irq_regs 没有

- [x] 每一个 process 的 thread_info 是如何初始化的?
  - 因为 thread_info 就是在 stack 的头上的，
  - 在 copy_thread_tls 中通过 stack 就可以直接访问内核数据 copy_thread_tls
  - 在 ret_from_irq 做了一次恢复
- [ ] 完全无法理解 `__current_thread_info` 声明的语法啊

- [x] 找到上下文切换的时候，tp 寄存器的切换
  - 在 loongarch/kernel/switch.S 中的确处理了

## 记录
正常状态的 crmd : b0 1011 0000
进入到 tlb 的 crmd : a8 1010 1000

正好是 PA 和 GA 之间的切换
## 总结
- tlb 拷贝长度不够
- 对于内核的代码删除不够
- macro 实际上修改了某些代码
- 在 TLB refill 的时候不能 0x9000000006000080 这种虚拟地址
  - 调用函数不能直接使用 b，而是 tlbera
  - SAVE_ALL 之前需要修改 sp 的数值为物理数值
- 使用 ertn 而不是 jmp 从而回复正常状态
- 在 TLB refill 中的 era 是无意义的数值

## one shot timer 被多次注入了
- kvm_acquire_timer 总是会 enable hardware timer
- 但是同时 lvz_irq_deliver 也是在注入中断的，我怀疑两个是互相冲突了发

## 总是在注入中断
```c
/*
kernel_syscall at /home/loongson/core/bmbt/env/loongarch/syscall/syscall.c:115
libc_syscall1 at /home/loongson/core/bmbt/libc/src/internal/../bits/loongarch/syscall_arch.h:40
_Exit at /home/loongson/core/bmbt/libc/src/exit/_Exit.c:5
__assert_fail at ??:?
a_ctz_32 at realloc.c:?
time_init at /home/loongson/core/bmbt/env/loongarch/kernel/time.c:28
timerlist_notify at /home/loongson/core/bmbt/src/util/qemu-timer.c:113
timerlist_rearm at /home/loongson/core/bmbt/src/util/qemu-timer.c:121
timer_mod_ns at /home/loongson/core/bmbt/src/util/qemu-timer.c:137
timer_mod at /home/loongson/core/bmbt/src/util/qemu-timer.c:141
check_update_timer at /home/loongson/core/bmbt/src/hw/rtc/mc146818rtc.c:159

rtc_realizefn at /home/loongson/core/bmbt/src/hw/rtc/mc146818rtc.c:766 (discriminator 1)
mc146818_rtc_init at /home/loongson/core/bmbt/src/hw/rtc/mc146818rtc.c:820 (discriminator 1)
pc_basic_device_init at /home/loongson/core/bmbt/src/hw/i386/pc.c:1466
pc_init1 at /home/loongson/core/bmbt/src/hw/i386/pc_piix.c:179 (discriminator 1)
pc_init_v4_2 at /home/loongson/core/bmbt/src/hw/i386/pc_piix.c:274
machine_run_board_init at /home/loongson/core/bmbt/src/hw/core/machine.c:222
qemu_init at /home/loongson/core/bmbt/src/qemu/vl.c:317 (discriminator 1)
test_qemu_init at /home/loongson/core/bmbt/src/main.c:148
wip at /home/loongson/core/bmbt/src/main.c:161 (discriminator 3)
greatest_run_suite at /home/loongson/core/bmbt/src/main.c:164 (discriminator 1)
main at /home/loongson/core/bmbt/src/main.c:177
start_kernel at /home/loongson/core/bmbt/env/loongarch/init/main.c:31
*/
```
- [ ] 实际上，backtrace 有问题啊
