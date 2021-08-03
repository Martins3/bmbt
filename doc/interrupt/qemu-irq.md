## 问题
1. exception_index 和负数的关系是什么 ?
    - 从 cpu_loop_exit_noexc 的注释来说，似乎是离开的原因没有什么的时候的，就赋值为 -1

2. 如果 cpu_handle_exception 失败之后，这会导致 cpu_exec 函数退出，之后如何处理 ?
    - 退出的原因应该都是外部原因，`cpu->exception_index >= EXCP_INTERRUPT` 和  reply ，目前似乎不用过于考虑

3. 应该去跟踪一下从键盘注入中断的整个过程吧!
    - nvme 设备注入中断是怎么进行的 ?
      - 数据传输完成应该存在一个告知操作才可以

4. 始终让人恐惧的 irqchip 其实可以内核模拟或者 QEMU 模拟的分工处理
    - [x] 为什么说要采用内核中模拟, 其好处是什么?
    - 还有一个让人很烦的 gsi 的定义啊, 为什么一个中断号需要注入两次啊
    - [ ] 对于用户态提供的接口是什么?

5. 那么 tcg 处理中断的流程是什么 ?

## QEMU 是如何处理 interrupt 和 exception 的

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


## X86CPU::apic_state 是如何工作的

- [ ] 需要回答一个问题，为什么只是 apic 被单独处理了
  - [ ] 更加复杂的选择
  - [ ] 额外的 include/hw/i386/apic.h 支持

只是需要看一看，常规的 apic 是被注入中断就可以了

- [x] 检测一下，到 seabios 的时候，中断控制器需要被处理吗 ?
  - 需要的，在 smp_scan 中需要 apic 的
  - 在其他的位置一些位置(暂时没分析，需要 8259)

# TODO
- [ ] pc_machine_reset

## APIC 的抉择
对应的在三个文件中:
- intc/apic_common.c
- intc/apic.c
- i386/kvm/apic.c


- x86_cpu_apic_create
  - apic_get_class

在 kvm_set_kernel_irqchip 中，初始化 KVMState 的三个成员
```c
    bool kernel_irqchip_allowed;
    bool kernel_irqchip_required;
    OnOffAuto kernel_irqchip_split;
```


在 kvm_init 中调用 kvm_irqchip_create
```c
    if (s->kernel_irqchip_allowed) {
        kvm_irqchip_create(s);
    }
```

kvm_irqchip_create 中设置:
```c
kvm_kernel_irqchip = true;
```

```c
#define kvm_irqchip_in_kernel() (kvm_kernel_irqchip)
#define kvm_apic_in_kernel() (kvm_irqchip_in_kernel())
```

```c
APICCommonClass *apic_get_class(void)
{
    const char *apic_type = "apic";

    /* TODO: in-kernel irqchip for hvf */
    if (kvm_apic_in_kernel()) {
        apic_type = "kvm-apic";
    } else if (xen_enabled()) {
        apic_type = "xen-apic";
    } else if (whpx_apic_in_platform()) {
        apic_type = "whpx-apic";
    }

    return APIC_COMMON_CLASS(object_class_by_name(apic_type));
}
```

这两个 TypeInfo 创建出来，最后的效果都导致 apic_class_init 和 kvm_apic_class_init
注册的内容不同罢了。

```c
static const TypeInfo apic_info = {
    .name          = TYPE_APIC,
    .instance_size = sizeof(APICCommonState),
    .parent        = TYPE_APIC_COMMON,
    .class_init    = apic_class_init,
};

static const TypeInfo kvm_apic_info = {
    .name = "kvm-apic",
    .parent = TYPE_APIC_COMMON,
    .instance_size = sizeof(APICCommonState),
    .class_init = kvm_apic_class_init,
};
```

## IOAPIC 的抉择
- hw/intc/ioapic_common.c
- hw/intc/ioapic.c
- hw/i386/kvm/ioapic.c

在 pc_init1 中，
```c
    if (pcmc->pci_enabled) {
        ioapic_init_gsi(gsi_state, "i440fx");
    }
```
- [ ] 为什么说，当没有 enable pci 的时候，就不需要 iopaic 了

## PIC 的抉择
- hw/i386/kvm/i8259.c
- hw/intc/i8259.c
- hw/intc/i8259_common.c

在 pc_basic_device_init 中间抉择

## MSI : 以后再说

## interrupt 机制
- apic_local_deliver : 在 apic 中存在大量的模拟
  - cpu_interrupt
    - generic_handle_interrupt
      - `cpu->interrupt_request |= mask;` 

在 cpu_handle_interrupt 中，会处理几种特殊情况，默认是 `cc->cpu_exec_interrupt(cpu, interrupt_request)`
也就是 x86_cpu_exec_interrupt, 在这里根据 idt 之类的中断处理需要的地址, 然后跳转过去执行的

## [ ] kvm 中为什么的 irq routing 是做什么的
- 为什么需要 irq routing
  - 一个正常的 x86 主板是不会同时存在 pic 和 apic 的
  - 同时 kvm_vm_ioctl_irq_line 其实是一个标准的接口, arm 对于函数实现就是不需要 irq routing 的操作
      - 所以，当调用 KVM_IRQ_LINE 的时候, 只是需要提供一个中断号
- kvm_vm_ioctl
  - kvm_vm_ioctl_irq_line
    - kvm_set_irq : 循环调用注册到这个 gsi 上的函数
        - kvm_set_pic_irq / kvm_set_ioapic_irq / kvm_set_msi

- KVM_SET_IRQCHIP : 找到 QEMU 调用这个函数的位置

- [ ] kvm_arch_vm_ioctl : 
    - `struct kvm_irqchip *chip = memdup_user(argp, sizeof(*chip));` : KVM_SET_IRQCHIP 的
		- kvm_vm_ioctl_set_irqchip(kvm, chip);

**首先等一下**

## [ ] 到底选择哪一个中断控制器，受什么控制的

- [ ] 所以，kvm 是如何处理 apic 的，

kvm_ioapic_put 和 kvm_pic_put 是两个调用 KVM_SET_IRQCHIP 的位置

```c
/*
#0  kvm_ioapic_put (s=0x555556a38100) at ../hw/i386/kvm/ioapic.c:80
#1  0x0000555555de4729 in resettable_phase_hold (obj=0x555556a38100, opaque=<optimized out>, type=<optimized out>) at ../hw/core/resettable.c:182
#2  0x0000555555de0044 in bus_reset_child_foreach (obj=<optimized out>, cb=0x555555de4640 <resettable_phase_hold>, opaque=0x0, type=RESET_TYPE_COLD) at ../hw/core/bus.c
:97
#3  0x0000555555de46e4 in resettable_child_foreach (rc=0x555556879da0, type=RESET_TYPE_COLD, opaque=0x0, cb=0x555555de4640 <resettable_phase_hold>, obj=0x5555569265d0)
at ../hw/core/resettable.c:96
#4  resettable_phase_hold (obj=obj@entry=0x5555569265d0, opaque=opaque@entry=0x0, type=type@entry=RESET_TYPE_COLD) at ../hw/core/resettable.c:173
#5  0x0000555555de4ec9 in resettable_assert_reset (obj=0x5555569265d0, type=<optimized out>) at ../hw/core/resettable.c:60
#6  0x0000555555de525d in resettable_reset (obj=0x5555569265d0, type=RESET_TYPE_COLD) at ../hw/core/resettable.c:45
#7  0x0000555555de4225 in qemu_devices_reset () at ../hw/core/reset.c:69
#8  0x0000555555b8a07f in pc_machine_reset (machine=<optimized out>) at ../hw/i386/pc.c:1654
#9  0x0000555555c57ac0 in qemu_system_reset (reason=reason@entry=SHUTDOWN_CAUSE_NONE) at ../softmmu/runstate.c:443
#10 0x0000555555a9c0ca in qdev_machine_creation_done () at ../hw/core/machine.c:1332
#11 0x0000555555d09fe0 in qemu_machine_creation_done () at ../softmmu/vl.c:2671
#12 qmp_x_exit_preconfig (errp=<optimized out>) at ../softmmu/vl.c:2694
#13 0x0000555555d0d6b0 in qemu_init (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/vl.c:3713
#14 0x0000555555940c8d in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49
```

```c
/*
#0  kvm_pic_put (s=0x55555698d520) at ../hw/i386/kvm/i8259.c:71
#1  0x0000555555de4729 in resettable_phase_hold (obj=0x55555698d520, opaque=<optimized out>, type=<optimized out>) at ../hw/core/resettable.c:182
#2  0x0000555555de0044 in bus_reset_child_foreach (obj=<optimized out>, cb=0x555555de4640 <resettable_phase_hold>, opaque=0x0, type=RESET_TYPE_COLD) at ../hw/core/bus.c:97
#3  0x0000555555de46e4 in resettable_child_foreach (rc=0x5555567da870, type=RESET_TYPE_COLD, opaque=0x0, cb=0x555555de4640 <resettable_phase_hold>, obj=0x555556acb9c0) at ../hw/core/resettable.c:96
#4  resettable_phase_hold (obj=0x555556acb9c0, opaque=<optimized out>, type=RESET_TYPE_COLD) at ../hw/core/resettable.c:173 #5  0x0000555555de1e1b in device_reset_child_foreach (obj=<optimized out>, cb=0x555555de4640 <resettable_phase_hold>, opaque=0x0, type=RESET_TYPE_COLD) at ../hw/core/qdev.c:366
#6  0x0000555555de46e4 in resettable_child_foreach (rc=0x5555567df1d0, type=RESET_TYPE_COLD, opaque=0x0, cb=0x555555de4640 <resettable_phase_hold>, obj=0x555556aee300) at ../hw/core/resettable.c:96
#7  resettable_phase_hold (obj=0x555556aee300, opaque=<optimized out>, type=RESET_TYPE_COLD) at ../hw/core/resettable.c:173
#8  0x0000555555de0044 in bus_reset_child_foreach (obj=<optimized out>, cb=0x555555de4640 <resettable_phase_hold>, opaque=0x0, type=RESET_TYPE_COLD) at ../hw/core/bus.c:97
#9  0x0000555555de46e4 in resettable_child_foreach (rc=0x5555567c66e0, type=RESET_TYPE_COLD, opaque=0x0, cb=0x555555de4640 <resettable_phase_hold>, obj=0x555556f49e00) at ../hw/core/resettable.c:96
#10 resettable_phase_hold (obj=0x555556f49e00, opaque=<optimized out>, type=RESET_TYPE_COLD) at ../hw/core/resettable.c:173
#11 0x0000555555de1e1b in device_reset_child_foreach (obj=<optimized out>, cb=0x555555de4640 <resettable_phase_hold>, opaque=0x0, type=RESET_TYPE_COLD) at ../hw/core/qdev.c:366
#12 0x0000555555de46e4 in resettable_child_foreach (rc=0x555556786f30, type=RESET_TYPE_COLD, opaque=0x0, cb=0x555555de4640 <resettable_phase_hold>, obj=0x555556a477c0) at ../hw/core/resettable.c:96
#13 resettable_phase_hold (obj=0x555556a477c0, opaque=<optimized out>, type=RESET_TYPE_COLD) at ../hw/core/resettable.c:173 #14 0x0000555555de0044 in bus_reset_child_foreach (obj=<optimized out>, cb=0x555555de4640 <resettable_phase_hold>, opaque=0x0, type=RESET_TYPE_COLD) at ../hw/core/bus.c:97
#15 0x0000555555de46e4 in resettable_child_foreach (rc=0x555556879da0, type=RESET_TYPE_COLD, opaque=0x0, cb=0x555555de4640 <resettable_phase_hold>, obj=0x5555569265d0) at ../hw/core/resettable.c:96
#16 resettable_phase_hold (obj=obj@entry=0x5555569265d0, opaque=opaque@entry=0x0, type=type@entry=RESET_TYPE_COLD) at ../hw/core/resettable.c:173
#17 0x0000555555de4ec9 in resettable_assert_reset (obj=0x5555569265d0, type=<optimized out>) at ../hw/core/resettable.c:60
#18 0x0000555555de525d in resettable_reset (obj=0x5555569265d0, type=RESET_TYPE_COLD) at ../hw/core/resettable.c:45
#19 0x0000555555de4225 in qemu_devices_reset () at ../hw/core/reset.c:69
#20 0x0000555555b8a07f in pc_machine_reset (machine=<optimized out>) at ../hw/i386/pc.c:1654
#21 0x0000555555c57ac0 in qemu_system_reset (reason=reason@entry=SHUTDOWN_CAUSE_NONE) at ../softmmu/runstate.c:443
#22 0x0000555555a9c0ca in qdev_machine_creation_done () at ../hw/core/machine.c:1332
#23 0x0000555555d09fe0 in qemu_machine_creation_done () at ../softmmu/vl.c:2671
#24 qmp_x_exit_preconfig (errp=<optimized out>) at ../softmmu/vl.c:2694
#25 0x0000555555d0d6b0 in qemu_init (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/vl.c:3713
#26 0x0000555555940c8d in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49
```

