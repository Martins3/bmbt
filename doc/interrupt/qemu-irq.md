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

2. 从 ioapic 如何到 lapic 的
- [ ] 其实，irq routing 还是不明不白的
- [ ] 思考一下在 bmbt 中间的设计

6. MSI : 以后再说

7. 能不能只是模拟 pic 啊，又不是不能用 ?

## TODO
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
  - kvm 和 QEMU 需要同时支持各种类型的主板，类似一个 kbd 之类的设备其实也不知道具体是和 pic 还是 ioapic 上的, 目前这个处理方法是有效并且最简单的

- kvm_vm_ioctl
  - kvm_vm_ioctl_irq_line
    - kvm_set_irq : 循环调用注册到这个 gsi 上的函数
        - kvm_set_pic_irq / kvm_set_ioapic_irq / kvm_set_msi

https://cloud.tencent.com/developer/article/1087271

> 虚拟触发了 irq 1，那么需要经过 irq routing：
> irq 1 在 0-7 的范围内，所以会路由到 i8259 master，随后 i8259 master 会向 vCPU 注入中断。
> 同时，irq 1 也会路由到 io apic 一份，io apic 也会向 lapic 继续 delivery。lapic 继续向 vCPU 注入中断。
> linux 在启动阶段，检查到 io apic 后，会选择使用 io apic。尽管经过 irq routing 产生了 i8259 master 和 io apic 两个中断，但是 Linux 选择 io apic 上的中断。

回顾一下从 userspace 提交中断一路向下的过程:
- kvm_vm_ioctl
  - kvm_set_irq
    - kvm_set_ioapic_irq
      - kvm_ioapic_set_irq
        - ioapic_set_irq
          - ioapic_service
            - kvm_irq_delivery_to_apic
              - kvm_apic_set_irq
                - `__apic_accept_irq`
                  - kvm_make_request
                  - kvm_vcpu_kick : ipi 中断

进而分析 pic 的处理过程:
1. 想要发送中断总是 kvm_make_request + kvm_vcpu_kick 两件套实现的, arch/x86/kvm/i8259.c 中只有 pic_unlock 这个函数使用上
2. 调用之前, 会通过 kvm_apic_accept_pic_intr 检查一下, 当前配置( amd64 内核) 下，这个检查总是失败
3. 分析 kvm_apic_accept_pic_intr 的实现，其中只是相当于看 guest 的实现了


一些其他的发现:

在 gsi_handler 中，我们发现，设备想要发送中断给 gsi 的时候，实际上会分别调用
kvm_pic_set_irq / kvm_ioapic_set_irq，对于 kvm 的接口而言，都是 irq = 12 而已，
如果内核中间再去处理 irq routing 机制, 那么岂不是一共需要四次

使用 scrpit/bpftrace.bt 来进行检测，的确如此，一个 qemu_set_irq 经过 qemu 的 irq routing 会变为两个，
经过 kvm 的 irq routing 就成为了四个。

猜测，其实中断的次数并没有必要非常精确了，多发射几次也无所谓

## [ ] KVM_SET_IRQCHIP 的作用是什么?

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

## GSIState
其核心作用在于 : gsi_handler 中进一步的分发到具体的 chip 相关的 irq chip

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
kvm ioapic 初始化这些被拷贝的 qemu_irq 的位置在 : kvm_ioapic_realize , 其中
对应的 handler 为 kvm_ioapic_set_irq

```c
static void kvm_ioapic_set_irq(void *opaque, int irq, int level)
{
    KVMIOAPICState *s = opaque;
    IOAPICCommonState *common = IOAPIC_COMMON(s);
    int delivered;

    ioapic_stat_update_irq(common, irq, level);
    delivered = kvm_set_irq(kvm_state, s->kvm_gsi_base + irq, level);
    apic_report_irq_delivered(delivered);
}
```


## X86MachineState::gsi

1. 键盘的中断处理过程

```c
/* XXX: not generating the irqs if KBD_MODE_DISABLE_KBD is set may be
   incorrect, but it avoids having to simulate exact delays */
static void kbd_update_irq_lines(KBDState *s)
{
    int irq_kbd_level, irq_mouse_level;

    irq_kbd_level = 0;
    irq_mouse_level = 0;

    if (s->status & KBD_STAT_OBF) {
        if (s->status & KBD_STAT_MOUSE_OBF) {
            if (s->mode & KBD_MODE_MOUSE_INT) {
                irq_mouse_level = 1;
            }
        } else {
            if ((s->mode & KBD_MODE_KBD_INT) &&
                !(s->mode & KBD_MODE_DISABLE_KBD)) {
                irq_kbd_level = 1;
            }
        }
    }
    qemu_set_irq(s->irq_kbd, irq_kbd_level);
    qemu_set_irq(s->irq_mouse, irq_mouse_level);
}
```


每一次键盘对应的调用两次函数:
```c
/*
#0  kbd_update_irq_lines (s=0x555556844d98) at ../hw/input/pckbd.c:144
#1  0x00005555559f5b9b in ps2_put_keycode (opaque=0x555556d28f20, keycode=<optimized out>) at ../hw/input/ps2.c:277
#2  0x00005555559f5e05 in ps2_keyboard_event (dev=0x555556d28f20, src=<optimized out>, evt=<optimized out>) at ../hw/input/ps2.c:478
#3  0x0000555555a35f88 in qemu_input_event_send_impl (src=0x55555668ffb0, evt=0x555556e5ad40) at ../ui/input.c:349
#4  0x0000555555a368eb in qemu_input_event_send_key (src=0x55555668ffb0, key=0x555556d3fcc0, down=<optimized out>) at ../ui/input.c:422
#5  0x0000555555a36946 in qemu_input_event_send_key_qcode (src=<optimized out>, q=q@entry=Q_KEY_CODE_R, down=down@entry=true) at ../ui/input.c:444
#6  0x000055555595afea in qkbd_state_key_event (down=<optimized out>, qcode=Q_KEY_CODE_R, kbd=0x555556a42c10) at ../ui/kbd-state.c:102
#7  qkbd_state_key_event (kbd=0x555556a42c10, qcode=qcode@entry=Q_KEY_CODE_R, down=<optimized out>) at ../ui/kbd-state.c:40
#8  0x0000555555b4cb23 in gd_key_event (widget=<optimized out>, key=0x555556971e40, opaque=0x555556b24d70) at ../ui/gtk.c:1112
#9  0x00007ffff78dd4fb in  () at /lib/x86_64-linux-gnu/libgtk-3.so.0
#10 0x00007ffff7077802 in g_closure_invoke () at /lib/x86_64-linux-gnu/libgobject-2.0.so.0
#11 0x00007ffff708b814 in  () at /lib/x86_64-linux-gnu/libgobject-2.0.so.0
#12 0x00007ffff709647d in g_signal_emit_valist () at /lib/x86_64-linux-gnu/libgobject-2.0.so.0
#13 0x00007ffff70970f3 in g_signal_emit () at /lib/x86_64-linux-gnu/libgobject-2.0.so.0
#14 0x00007ffff7887c23 in  () at /lib/x86_64-linux-gnu/libgtk-3.so.0
#15 0x00007ffff78a95db in gtk_window_propagate_key_event () at /lib/x86_64-linux-gnu/libgtk-3.so.0
#16 0x00007ffff78ad873 in  () at /lib/x86_64-linux-gnu/libgtk-3.so.0
#17 0x00007ffff78dd5ef in  () at /lib/x86_64-linux-gnu/libgtk-3.so.0
#18 0x00007ffff7077a56 in  () at /lib/x86_64-linux-gnu/libgobject-2.0.so.0
#19 0x00007ffff7095df1 in g_signal_emit_valist () at /lib/x86_64-linux-gnu/libgobject-2.0.so.0
#20 0x00007ffff70970f3 in g_signal_emit () at /lib/x86_64-linux-gnu/libgobject-2.0.so.0
#21 0x00007ffff7887c23 in  () at /lib/x86_64-linux-gnu/libgtk-3.so.0
#22 0x00007ffff77431df in  () at /lib/x86_64-linux-gnu/libgtk-3.so.0
#23 0x00007ffff77453db in gtk_main_do_event () at /lib/x86_64-linux-gnu/libgtk-3.so.0
#24 0x00007ffff742df79 in  () at /lib/x86_64-linux-gnu/libgdk-3.so.0
#25 0x00007ffff7461106 in  () at /lib/x86_64-linux-gnu/libgdk-3.so.0
#26 0x00007ffff6f8c17d in g_main_context_dispatch () at /lib/x86_64-linux-gnu/libglib-2.0.so.0
#27 0x0000555555e4ce88 in glib_pollfds_poll () at ../util/main-loop.c:232
#28 os_host_main_loop_wait (timeout=<optimized out>) at ../util/main-loop.c:255
#29 main_loop_wait (nonblocking=nonblocking@entry=0) at ../util/main-loop.c:531
#30 0x0000555555c58261 in qemu_main_loop () at ../softmmu/runstate.c:726
#31 0x0000555555940c92 in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:50
*/
```

```c
/*
#0  kbd_update_irq_lines (s=0x555556844d98) at ../hw/input/pckbd.c:144
#1  0x00005555559ab077 in kbd_deassert_irq (s=0x555556844d98) at ../hw/input/pckbd.c:165
#2  kbd_read_data (opaque=0x555556844d98, addr=<optimized out>, size=<optimized out>) at ../hw/input/pckbd.c:387
#3  0x0000555555cd2092 in memory_region_read_accessor (mr=mr@entry=0x555556844df0, addr=0, value=value@entry=0x7fffd9ff9130, size=size@entry=1, shift=0, mask=mask@entry=255, attrs=...) at ../softmmu/memory.c:440
#4  0x0000555555cceb1e in access_with_adjusted_size (addr=addr@entry=0, value=value@entry=0x7fffd9ff9130, size=size@entry=1, access_size_min=<optimized out>, access_size_max=<optimized out>, access_fn=0x555555cd2050 <memory_region_read_accessor>, mr=0x555556844df0, attrs=...) at ../softmmu/memory.c:554
#5  0x0000555555cd1ac1 in memory_region_dispatch_read1 (attrs=..., size=<optimized out>, pval=0x7fffd9ff9130, addr=0, mr=0x555556844df0) at ../softmmu/memory.c:1424
#6  memory_region_dispatch_read (mr=mr@entry=0x555556844df0, addr=0, pval=pval@entry=0x7fffd9ff9130, op=MO_8, attrs=attrs@entry=...) at ../softmmu/memory.c:1452
#7  0x0000555555c9eb89 in flatview_read_continue (fv=fv@entry=0x7ffe4402d230, addr=addr@entry=96, attrs=..., ptr=ptr@entry=0x7fffeb17d000, len=len@entry=1, addr1=<optimized out>, l=<optimized out>, mr=0x555556844df0) at /home/maritns3/core/kvmqemu/include/qemu/host-utils.h:165
#8  0x0000555555c9ed43 in flatview_read (fv=0x7ffe4402d230, addr=addr@entry=96, attrs=attrs@entry=..., buf=buf@entry=0x7fffeb17d000, len=len@entry=1) at ../softmmu/physmem.c:2881
#9  0x0000555555c9ee96 in address_space_read_full (as=0x555556606880 <address_space_io>, addr=96, attrs=..., buf=0x7fffeb17d000, len=1) at ../softmmu/physmem.c:2894
#10 0x0000555555c9f015 in address_space_rw (as=<optimized out>, addr=addr@entry=96, attrs=..., attrs@entry=..., buf=<optimized out>, len=len@entry=1, is_write=is_write@entry=false) at ../softmmu/physmem.c:2922
#11 0x0000555555c8ece9 in kvm_handle_io (count=1, size=1, direction=<optimized out>, data=<optimized out>, attrs=..., port=96) at ../accel/kvm/kvm-all.c:2635
#12 kvm_cpu_exec (cpu=cpu@entry=0x555556af4410) at ../accel/kvm/kvm-all.c:2886
#13 0x0000555555cf1825 in kvm_vcpu_thread_fn (arg=arg@entry=0x555556af4410) at ../accel/kvm/kvm-accel-ops.c:49
#14 0x0000555555e55983 in qemu_thread_start (args=<optimized out>) at ../util/qemu-thread-posix.c:541
#15 0x00007ffff628d609 in start_thread (arg=<optimized out>) at pthread_create.c:477
#16 0x00007ffff61b4293 in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:95
*/
```
- [ ] **根本无法理解为什么 kvm 那一边还是会到来一个中断啊**

2. 键盘中断如何初始化的 KBDState::irq_kbd 以及 KBDState::irq_mouse

- i8042_realizefn
  - `isa_init_irq(isadev, &s->irq_kbd, 1);`
  - `isa_init_irq(isadev, &s->irq_mouse, 12);`
    - isa_get_irq 

```c
qemu_irq isa_get_irq(ISADevice *dev, unsigned isairq)
{
    assert(!dev || ISA_BUS(qdev_get_parent_bus(DEVICE(dev))) == isabus);
    assert(isairq < ISA_NUM_IRQS);
    return isabus->irqs[isairq];
}
```

注意，其中 `isabus->irqs` 被初始化为 X86MachineState::gsi 了


3. qemu_set_irq 如何运行的?
```c
void qemu_set_irq(qemu_irq irq, int level)
{
    if (!irq)
        return;

    irq->handler(irq->opaque, irq->n, level);
}
```

## X86CPU::apic_state
- [ ] 如果当前设备只是支持 pic 岂不是这个东西即使无需模拟了 ?

-  需要回答一个问题，为什么只是 apic 被单独处理了, 只有这个一个在 X86CPU::apic_state 中的
  - [x] 这个东西在 X86CPU 中，因为这是 percpu 的
  - [ ] 额外的 include/hw/i386/apic.h 支持

只是需要看一看，常规的 apic 是被注入中断就可以了

- [x] 检测一下，到 seabios 的时候，中断控制器需要被处理吗 ?
  - 需要的，在 smp_scan 中需要 apic 的
  - 在其他的位置一些位置(暂时没分析，需要 8259)
  - 暂时实际上，apic 和系统配置存在一些莫名奇妙的关系，因为多核才需要 ioapic 的呀, 所以需要的

分析一下 APICCommonClass 中间注册的 hook 函数, 这些 hook 都是在 hw/intc/apic_common.c 中调用的
其中的一个比如:

```c
void apic_enable_vapic(DeviceState *dev, hwaddr paddr)
{
    APICCommonState *s = APIC_COMMON(dev);
    APICCommonClass *info = APIC_COMMON_GET_CLASS(s);

    s->vapic_paddr = paddr;
    info->vapic_base_update(s);
}
```

实际上，持有的是这个玩意儿:
```c
struct APICCommonState {
    /*< private >*/
    DeviceState parent_obj;
    /*< public >*/

    MemoryRegion io_memory;
    X86CPU *cpu;
    uint32_t apicbase;
    uint8_t id; /* legacy APIC ID */
    uint32_t initial_apic_id;
    uint8_t version;
    uint8_t arb_id;
    uint8_t tpr;
    uint32_t spurious_vec;
    uint8_t log_dest;
    uint8_t dest_mode;
    uint32_t isr[8];  /* in service register */
    uint32_t tmr[8];  /* trigger mode register */
    uint32_t irr[8]; /* interrupt request register */
    uint32_t lvt[APIC_LVT_NB];
    uint32_t esr; /* error register */
    uint32_t icr[2];

    uint32_t divide_conf;
    int count_shift;
    uint32_t initial_count;
    int64_t initial_count_load_time;
    int64_t next_time;
    QEMUTimer *timer;
    int64_t timer_expiry;
    int sipi_vector;
    int wait_for_sipi;

    uint32_t vapic_control;
    DeviceState *vapic;
    hwaddr vapic_paddr; /* note: persistence via kvmvapic */
    bool legacy_instance_id;
};
```
从 kvm 的角度，这个东西似乎什么意义，用于临时保存一下  kvm_get_apic_state / kvm_put_apic_state 实现迁移

分析了一下引用位置, 其他的位置都是 tcg 带来的, 甚至包括 hw/i386/pc.c
分析一下 target/i386/kvm/kvm.c 中，实际上也并不访问其中内容。

## kvmvapic 的作用
首先可以复习一下 posted interrupt 和 vt-d[^1]

- [ ] pc-bios/kvmvapic.bin : 这个到底是如何制作出来的

```diff
History:        #0
Commit:         e5ad936b0fd7dfd7fd7908be6f9f1ca88f63b96b
Author:         Jan Kiszka <jan.kiszka@siemens.com>
Committer:      Avi Kivity <avi@redhat.com>
Author Date:    Sat 18 Feb 2012 01:31:19 AM CST
Committer Date: Sat 18 Feb 2012 06:15:59 PM CST

kvmvapic: Introduce TPR access optimization for Windows guests

This enables acceleration for MMIO-based TPR registers accesses of
32-bit Windows guest systems. It is mostly useful with KVM enabled,
either on older Intel CPUs (without flexpriority feature, can also be
manually disabled for testing) or any current AMD processor.

The approach introduced here is derived from the original version of
qemu-kvm. It was refactored, documented, and extended by support for
user space APIC emulation, both with and without KVM acceleration. The
VMState format was kept compatible, so was the ABI to the option ROM
that implements the guest-side para-virtualized driver service. This
enables seamless migration from qemu-kvm to upstream or, one day,
between KVM and TCG mode.

The basic concept goes like this:
 - VAPIC PV interface consisting of I/O port 0x7e and (for KVM in-kernel
   irqchip) a vmcall hypercall is registered
 - VAPIC option ROM is loaded into guest
 - option ROM activates TPR MMIO access reporting via port 0x7e
 - TPR accesses are trapped and patched in the guest to call into option
   ROM instead, VAPIC support is enabled
 - option ROM TPR helpers track state in memory and invoke hypercall to
   poll for pending IRQs if required

Signed-off-by: Jan Kiszka <jan.kiszka@siemens.com>
Signed-off-by: Avi Kivity <avi@redhat.com>
```

vapic_write : 只是调用过一次, 在 seabios 中的 callrom 中进行的

- apic_enable_vapic : 从来没有被调用过
  - kvm_apic_vapic_base_update
    - `kvm_vcpu_ioctl(CPU(s->cpu), KVM_SET_VAPIC_ADDR, &vapid_addr);`

在 vapic_realize 中间，注释掉代码，不添加 kvmvapic.bin 这个 rom 实际上也没有问题

所以，总结就是这个东西实际上暂时是没有必要的

- [ ] 存在一个关联的 exit reason 是 KVM_EXIT_TPR_ACCESS
- [ ] 一个关联的 kvm ioctl 命令 : KVM_SET_VAPIC_ADDR

## kvm ioapic
```c
struct KVMIOAPICState {
    IOAPICCommonState ioapic;
    uint32_t kvm_gsi_base;
};
```
除了和初始化相关的，几乎没有什么需要做的事情，而且 kvm_gsi_base 的取值总是 0 的

总体来说，就是一些简单的保存 恢复工作 : kvm_ioapic_get / kvm_ioapic_put, 通过 KVM_GET_IRQCHIP 来实现

## kvm i8259
和 kvm ioapic 类似, 不过有趣的是 lapic 使用的是 KVM_SET_LAPIC ，而 ioapic 和 8259 都是 KVM_GET_IRQCHIP

[^1]: https://events.static.linuxfound.org/sites/events/files/slides/VT-d%20Posted%20Interrupts-final%20.pdf
