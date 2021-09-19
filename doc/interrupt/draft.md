# QEMU tcg 的中断模拟
下面的代码暂时分析基于 v6.1.0-rc2 的 x64

## 问题
- [ ] make a table explaining every field of APIC and IOAPIC struct in QEMU
- [ ] CPU_INTERRUPT_POLL 是做啥的? 为啥会出现在 cpu_handle_halt 中间。
  - 处理 halt 的
- [ ] 应该 review 一下 apic 内部的更新过程的

- [ ] 实际上，ISABus 中的只有成员 irq 有用，而且是通过 isa_bus_irqs 赋值的，实际上，没有啥作用的
  - 实际上，我猜测 ISABus 主要是为了 keyboard 之类的设备容易模拟吧

- [ ] 似乎没有整明白 : gsi_handler 和 qemu_set_irq 的关系
- [ ] qemu_irq 到底是什么?
  - [ ] qdev_connect_gpio_out 等价的效果到底是什么
  - [ ] 比如 pic_realize 中的注册

- i8259_init :其参数为 x86_allocate_cpu_irq

- [ ] 实际上，GSIState 中注册的 qemu_irq 是 pic 和 ioapic 的，apic 的定位是什么哇
  - [ ] 没有理清楚 msi / ioapic / pic 的中断到达 apic 的过程
  - [ ] 没有搞清楚 pic 直接发送中断到 cpu 的过程的

- [x] 中断发送给 lapic 是不是只是靠读写地址空间的方法？
  - pic 不是

- [x] 如果当前设备只是支持 pic 岂不是这个东西即使无需模拟了 ?
  - 不是的

分析一下 lapic 的特殊性:
-  需要回答一个问题，为什么只是 apic 被单独处理了, 只有这个一个在 X86CPU::apic_state 中的
  - [x] 这个东西在 X86CPU 中，因为这是 percpu 的
  - [ ] 额外的 include/hw/i386/apic.h 支持

- [ ] apic_check_pic : 这个东西是做啥的
  - 和 apic_update_irq 做出类似的判断
```c
  if (!apic_accept_pic_intr(dev) || !pic_get_output(isa_pic)) {
```

- apic timer : 总体来说，timer 是比较容易处理的
  - apic_timer 被周期性的触发
    - [ ] 思考一下如何获取 clock time, 实际上，guest 操作系统可以主动校准实践
    - QEMU_CLOCK_VIRTUAL : 当虚拟机停下来的时候，时钟中断需要停止下来
  - 考虑一个小问题，所有的 vCPU 都是需要接受 local timer 的时钟的，难道为此需要创建出来多个 timer 吗 ?
    - 是的, 而且 timer 这个线程是在 main_loop_wait => qemu_clock_run_all_timers 中使用一个新的线程来进行的

- apic_update_irq 的分析
  - apic_poll_irq : 如果中断是来自于其他的 thread，那么就采用这种方式，比如时钟中断
    - 因为时钟是在另一个线程处理的，所以需要实现
  - 如果不是来自于 pic 的中断，那就清理掉这个中断

## 备忘
- tcg_handle_interrupt / x86_cpu_exec_interrupt 的功能区别:
  - 前者: 让执行线程退出，去检查 interrupt
  - 后者: 线程相关的具体分析 interrupt 该如何处理

## how interrupt simulated by QEMU
- [ ] 使用 Niugene 的 blog 来分析基本的执行流程

- cpu_handle_exception
  - x86_cpu_do_interrupt

- cpu_handle_interrupt
  - 首先处理和架构无关的，类似 halt reset 之类的
  - 通过 CPUClass 进入 x86_cpu_exec_interrupt
      - x86_cpu_pending_interrupt : 需要分析出来真正的 interrupts, 因为可能屏蔽了
      - do_interrupt_x86_hardirq : 处理函数各种情况

- apic_local_deliver : 在 apic 中存在大量的模拟
  - cpu_interrupt
    - generic_handle_interrupt
      - `cpu->interrupt_request |= mask;`

在 cpu_handle_interrupt 中，会处理几种特殊情况，默认是 `cc->cpu_exec_interrupt(cpu, interrupt_request)`
也就是 x86_cpu_exec_interrupt, 在这里根据 idt 之类的中断处理需要的地址, 然后跳转过去执行的

## https://niugenen.github.io/qemu-timer-interrupt
- apic_timer()
  - apic_local_deliver()
   - apic_set_irq()
      - apic_update_irq()
         - cpu = CPU(s->cpu)   // s is APICCommonState and lapic will attach to one CPU
         - if (!qemu_cpu_is_self(cpu)) : 如果是自己，那么就直接触发了
            - cpu_interrupt(cpu, CPU_INTERRUPT_POLL);

#### EOI
- [ ] apic_eoi : 和 10.8.5 中描述的一致，当 apic 接受到一个 EOIUpon receiving an EOI, the APIC clears the highest priority bit in the ISR and dispatches the next highest priority
interrupt to the processor.
  - [ ] 10.8.5 : 手册中间分析的 ioapic 的 broadcast 是什么意思
  - [x] apic_sync_vapic : 这个是处理 kvm 的，暂时不分析

*If the terminated interrupt was a level-triggered interrupt, the local APIC Also sends an
end-of-interrupt message to all I/O APICs.* (**无法理解为什么 level-triggered 的就需要向 io apic 发送**)

System software may prefer to direct EOIs to specific I/O APICs rather than having the local APIC send end-of-interrupt messages to all I/O APICs.

Software can inhibit the broadcast of EOI message by setting bit 12 of the *Spurious Interrupt Vector Register* (see
Section 10.9). If this bit is set, a broadcast EOI is not generated on an EOI cycle even if the associated *TMR* bit indicates that the current interrupt was level-triggered.
The default value for the bit is 0, indicating that EOI broadcasts are performed.

Bit 12 of the Spurious Interrupt Vector Register is reserved to 0 if the processor does not support suppression of
EOI broadcasts. Support for EOI-broadcast suppression is reported in bit 24 in the Local APIC Version Register (see
Section 10.4.8); the feature is supported if that bit is set to 1. When supported, the feature is available in both
xAPIC mode and x2APIC mode.

System software desiring to perform directed EOIs for level-triggered interrupts should set bit 12 of the *Spurious Interrupt Vector Register* and follow each the EOI to the local xAPIC for a level triggered interrupt with a directed
EOI to the I/O APIC generating the interrupt (this is done by writing to the I/O APIC’s EOI register).
System software performing directed EOIs must retain a mapping associating level-triggered interrupts with the I/O APICs in the system. (**并没有看懂这个英语，是如何实现 dedicated 的 EOI 的**)

- [ ] 实际上，ioapic 也是存在 eoi 的, 而且还在两个调用位置, 放到 tcg ioapic 中间分析吧

#### lvt
lvt  中的取值总是在发生改变的, 但是
apic_timer => apic_local_deliver => apic_set_irq 的过程中，本来 apic 的中断是 APIC_LVT_TIMER 的，但是最后装换为 236 了

```txt
[0=236] [1=65536] [2=65536] [3=67328] [4=1024] [5=254]
```

- [ ] 解释一下 nvme 中地址空间的内容
```txt
      00000000febf0000-00000000febf3fff (prio 1, i/o): nvme-bar0
        00000000febf0000-00000000febf1fff (prio 0, i/o): nvme
        00000000febf2000-00000000febf240f (prio 0, i/o): msix-table
        00000000febf3000-00000000febf300f (prio 0, i/o): msix-pba
```

[^1]: https://events.static.linuxfound.org/sites/events/files/slides/VT-d%20Posted%20Interrupts-final%20.pdf
[^2]: https://luohao-brian.gitbooks.io/interrupt-virtualization/content/qemu-kvm-zhong-duan-xu-ni-hua-kuang-jia-fen-679028-4e0a29.html

> In general, one pin is used for chaining the legacy PIC and the other for NMIs (Or occasionally SMIs).
> No devices are actually connected to the LINT pins for a couple of reasons, one of which is that id just doesn't work on multiprocessor systems (They inevitably get delivered to one core).
> In practice, all devices are connected to the I/O APIC and/or PIC.[^4]

> The local APIC is enabled at boot-time and can be disabled by clearing bit 11 of the IA32_APIC_BASE Model Specific Register (MSR). [^3]

[^3]: https://wiki.osdev.org/APIC
[^4]: https://forum.osdev.org/viewtopic.php?f=1&t=22024
