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


# TODO
- [ ] pc_machine_reset
- 补充材料 : ioapic 如何知道中断源头 : https://stackoverflow.com/questions/57704146/how-to-figure-out-the-interrupt-source-on-i-o-apic

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

- [ ] 需要回答一个问题，为什么只是 apic 被单独处理了, 只有这个一个在 X86CPU::apic_state 中的
  - [ ] 这个东西在 X86CPU 中，因为这是 percpu 的
  - [ ] 更加复杂的选择
  - [ ] 额外的 include/hw/i386/apic.h 支持

只是需要看一看，常规的 apic 是被注入中断就可以了

- [x] 检测一下，到 seabios 的时候，中断控制器需要被处理吗 ?
  - 需要的，在 smp_scan 中需要 apic 的
  - 在其他的位置一些位置(暂时没分析，需要 8259)

## select APIC
对应的在三个文件中:
- intc/apic_common.c
- intc/apic.c
- i386/kvm/apic.c

- do_configure_accelerator
  - accel_init_machine : 调用 AccelClass::init_machine
    - kvm_init
      - kvm_irqchip_create 当 KVMState::kernel_irqchip_allowed 的时候，才会调用这个函数
          - kvm_vm_ioctl(s, KVM_CREATE_IRQCHIP)
          - kvm_kernel_irqchip = true; --------------------------------------------------------------------------------------------------|
                                                                                                                                         |
- x86_cpu_realizefn                                                                                                                      |
  - x86_cpu_apic_create                                                                                                                  |
    - x86_cpu_apic_create : 调用这个函数实际上会做出判断, 只有当前 CPU 支持这个特性 或者 cpu 的数量超过两个的时候才可以调用下面的函数    |
        - apic_get_class                                                                                                                 |
          - kvm_apic_in_kernel <---------------------------------------------------------------------------------------------------------|
        - `cpu->apic_state = DEVICE(object_new_with_class(apic_class));`

在 kvm_set_kernel_irqchip 中， 可以通过命令行来初始化 KVMState 的三个成员
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

两个 TypeInfo 对应的 instance 都是 APICCommonState, 其差别在于 apic_class_init 和 kvm_apic_class_init
注册的函数指针完全不同。

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

## select IOAPIC 
- hw/intc/ioapic_common.c
- hw/intc/ioapic.c
- hw/i386/kvm/ioapic.c

在 pc_init1 中，
```c
    if (pcmc->pci_enabled) {
        ioapic_init_gsi(gsi_state, "i440fx");
    }
```
只有支持 pci 的时候才会初始化 ioapic ，这似乎说的过去:

在 ioapic_init_gsi 中, 一些常规的易于理解的操作:
```c
    if (kvm_ioapic_in_kernel()) {
        dev = qdev_new(TYPE_KVM_IOAPIC);
    } else {
        dev = qdev_new(TYPE_IOAPIC);
    }
```

## select PIC
- hw/i386/kvm/i8259.c
- hw/intc/i8259.c
- hw/intc/i8259_common.c

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
  - [ ] 一个正常的 x86 主板是不会同时存在 pic 和 apic 的
      - [ ] 真的如此吗? 实际上，QEMU 的地址空间上是同时存在 kvm-ioapic /kvmvapic / kvm-pic 的
          - [ ] 并不能完全这么判断，从地址空间上看，tcg 运行的时候，其地址空间还是包含了一堆 kvm-pic 了
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

https://cloud.tencent.com/developer/article/1087271 : 似乎说的很有道理

> 虚拟触发了irq 1，那么需要经过irq routing：
> irq 1在0-7的范围内，所以会路由到i8259 master，随后i8259 master会向vCPU注入中断。
> 同时，irq 1也会路由到io apic一份，io apic也会向lapic继续delivery。lapic继续向vCPU注入中断。
> linux在启动阶段，检查到io apic后，会选择使用io apic。尽管经过irq routing产生了i8259 master和io apic两个中断，但是Linux选择io apic上的中断。

- [ ] 什么叫做 linux 选择了 io apic 上的中断啊
    - [ ] 是不是这两个中断都是可以注入到 guest 中间的，然后让 guest 来决定，还是说 guest 启动之后，最后会 disable 掉一个 i8259, 最后导致只有一个注入

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

## [ ] GSIState
```c
/*
 * Pointer types
 * Such typedefs should be limited to cases where the typedef's users
 * are oblivious of its "pointer-ness".
 * Please keep this list in case-insensitive alphabetical order.
 */
typedef struct IRQState *qemu_irq;

struct IRQState {
    Object parent_obj;

    qemu_irq_handler handler;
    void *opaque;
    int n;
};
```

发生在 pc_init1
1. pc_gsi_create : 创建了 qemu_irq，分配了 GSIState , 但是 GSIState 没有被初始化

- pc_gsi_create
  - kvm_pc_setup_irq_routing : 
    - kvm_irqchip_add_irq_route(s, i, KVM_IRQCHIP_PIC_MASTER, i); i = [0,8)
    - kvm_irqchip_add_irq_route(s, i, KVM_IRQCHIP_PIC_SLAVE, i - 8); i = [8, 16)
    - kvm_irqchip_add_irq_route(s, i, KVM_IRQCHIP_IOAPIC, i); i = [0, 24) i == 2 被特殊处理了 
    - kvm_irqchip_commit_routes
        - `kvm_vm_ioctl(s, KVM_SET_GSI_ROUTING, s->irq_routes);`
  - qemu_allocate_irqs : 创建一组 IRQState 其 handler 是 gsi_handler, 其 opaque 是 GSIState, 这些 gsi 通过 X86MachineState::gsi 来索引的

总体来说，和 kvm 内调用 kvm_setup_default_irq_routing 的差别在于处理了 irq = 2 的关系

```c
void gsi_handler(void *opaque, int n, int level)
{
    GSIState *s = opaque;

    trace_x86_gsi_interrupt(n, level);
    switch (n) {
    case 0 ... ISA_NUM_IRQS - 1:
        if (s->i8259_irq[n]) {
            /* Under KVM, Kernel will forward to both PIC and IOAPIC */
            qemu_set_irq(s->i8259_irq[n], level);
        }
        /* fall through */
    case ISA_NUM_IRQS ... IOAPIC_NUM_PINS - 1:
        qemu_set_irq(s->ioapic_irq[n], level);
        break;
    case IO_APIC_SECONDARY_IRQBASE
        ... IO_APIC_SECONDARY_IRQBASE + IOAPIC_NUM_PINS - 1:
        qemu_set_irq(s->ioapic2_irq[n - IO_APIC_SECONDARY_IRQBASE], level);
        break;
    }
}
```
从这里看，gsi_handler 总是入口，然后


2. `pc_i8259_create(isa_bus, gsi_state->i8259_irq);`

- pc_i8259_create
  - kvm_i8259_init : i8259 也就是 pic 本身是一个设备，所以需要调用 i8259_init_chip 初始化一下，这很好
    - i8259_init_chip(TYPE_KVM_I8259, bus, true); // master
    - i8259_init_chip(TYPE_KVM_I8259, bus, false); // slave
    - [ ] qemu_allocate_irqs(kvm_pic_set_irq, NULL, ISA_NUM_IRQS); 很好，又初始化了一堆 qemu_irq 出来了，之后就可以在 gsi_handler 中调用了
  - xen_interrupt_controller_init
  - i8259_init

```c
static void kvm_pic_set_irq(void *opaque, int irq, int level)
{
    int delivered;

    pic_stat_update_irq(irq, level);
    delivered = kvm_set_irq(kvm_state, irq, level);
    apic_report_irq_delivered(delivered);
}
```

3. ioapic_init_gsi

其实和 pc_i8259_create 类似，初始化设备，然后拷贝 qemu_irq

kvm ioapic 初始化这些内容的位置在 : kvm_ioapic_realize 的位置

到这里，其实关于 pic 和 io apic 的部分搞定了, 接下来:
1. 如何到达 gsi 的
2. 从 ioapic 如何到 lapic 的
- [ ] 其实，irq routing 还是不明不白的
- [ ] 思考一下在 bmbt 中间的设计
