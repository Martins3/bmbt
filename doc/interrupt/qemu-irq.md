下面的代码暂时分析基于 v6.1.0-rc2
## 挑战
1. 当前的 QEMU 中间，时钟中断是采用现有设施的
    1. 因为 timer 线程是单独的线程，所以 CPU_INTERRUPT_POLL 来处理, 显然 BMBT 没有那么多的线程

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

6. MSI : 以后再说

7. 能不能只是模拟 pic 啊，又不是不能用 ?

8. 在进入 kernel 之前，似乎只是 QEMU 启动的时候，gsi_handler 似乎被调用数次，是因为初始化的原因，还是因为 seabios 调用的

9. 在 seabios 中，会处理中断吗 ?

10. level 和 edge 的处理有什么不同, 为什么需要 level 的触发方式

## 备忘
- tcg_handle_interrupt /  x86_cpu_exec_interrupt 的功能区别:
  - 前者: 让执行线程退出，去检查 interrupt
  - 后者: 线程相关的具体分析 interrupt 该如何处理

## TODO
- [ ] pc_machine_reset
- 补充材料 : ioapic 如何知道中断源头 : https://stackoverflow.com/questions/57704146/how-to-figure-out-the-interrupt-source-on-i-o-apic


## intel 手册知识准备
- volume 3 CHAPTER 6 (INTERRUPT AND EXCEPTION HANDLING) : 从 CPU 的角度描述了中断的处理过程
- volume 3 CHAPTER 10 (ADVANCED PROGRAMMABLE INTERRUPT CONTROLLER (APIC)): apic
  - 10.8.3.1 Task and Processor Priorities
  - 10.8.5 Signaling Interrupt Servicing Completion : 描述 eoi 的作用
  - 10.11.1 Message Address Register Format : 描述 MSI 地址的格式, 从中看到一个中断如何发送到特定的 vector 的

coreboot 三部曲 : https://habr.com/en/post/501912/ : 永远的神

## 一些关键流程
1. 如何从 ide 走到 apic 的
    - 可以看到，ide assert 是 ioapic 的 15 号引脚, 经过 io apic 转换之后，在 apic 哪里是 irr 等于 34
```c
/*
#0  huxueshi (i=34) at ../hw/intc/apic.c:402
#1  0x0000555555c6a21a in apic_set_irq (s=0x55555697f560, vector_num=34, trigger_mode=0) at ../hw/intc/apic.c:411
#2  0x0000555555c6a4a3 in apic_bus_deliver (deliver_bitmask=<optimized out>, delivery_mode=<optimized out>, vector_num=34 '"', trigger_mode=0 '\000') at ../hw/intc/apic.c:273
#3  0x0000555555c6a66f in apic_deliver_irq (dest=1 '\001', dest_mode=1 '\001', delivery_mode=0 '\000', vector_num=34 '"', trigger_mode=0 '\000') at ../hw/intc/apic.c:286
#4  0x0000555555c6aadb in apic_mem_write (opaque=<optimized out>, addr=4100, val=34, size=<optimized out>) at ../hw/intc/apic.c:766
#5  0x0000555555cd2611 in memory_region_write_accessor (mr=mr@entry=0x55555697f5f0, addr=4100, value=value@entry=0x7fffe888b7c8, size=size@entry=4, shift=<optimized out>, mask=mask@entry=4294967295, attrs=...) at ../softmmu/memory.c:492
#6  0x0000555555ccea9e in access_with_adjusted_size (addr=addr@entry=4100, value=value@entry=0x7fffe888b7c8, size=size@entry=4, access_size_min=<optimized out>, access_size_max=<optimized out>, access_fn=access_fn@entry=0x555555cd2580 <memory_region_write_accessor>, mr=0x55555697f5f0, attrs=...) at ../softmmu/memory.c:554
#7  0x0000555555cd1b47 in memory_region_dispatch_write (mr=mr@entry=0x55555697f5f0, addr=4100, data=<optimized out>, data@entry=34, op=op@entry=MO_32, attrs=attrs@entry=...) at ../softmmu/memory.c:1504
#8  0x0000555555ca11ed in address_space_stl_internal (endian=DEVICE_LITTLE_ENDIAN, result=0x0, attrs=..., val=34, addr=<optimized out>, as=0x0) at /home/maritns3/core/kvmqemu/include/exec/memory.h:2868
#9  address_space_stl_le (as=as@entry=0x555556606820 <address_space_memory>, addr=<optimized out>, val=34, attrs=attrs@entry=..., result=result@entry=0x0) at /home/maritns3/core/kvmqemu/memory_ldst.c.inc:357
#10 0x0000555555cee024 in stl_le_phys (val=<optimized out>, addr=<optimized out>, as=0x555556606820 <address_space_memory>) at /home/maritns3/core/kvmqemu/include/exec/memory_ldst_phys.h.inc:121
#11 ioapic_service (s=s@entry=0x555556a49e10) at ../hw/intc/ioapic.c:139
#12 0x0000555555cee2ff in ioapic_set_irq (opaque=0x555556a49e10, vector=<optimized out>, level=1) at ../hw/intc/ioapic.c:187
#13 0x0000555555b92644 in gsi_handler (opaque=0x555556a07620, n=15, level=1) at ../hw/i386/x86.c:600
#14 0x0000555555a9dc41 in qemu_irq_raise (irq=<optimized out>) at /home/maritns3/core/kvmqemu/include/hw/irq.h:12
#15 ide_set_irq (bus=<optimized out>, bus=<optimized out>) at /home/maritns3/core/kvmqemu/include/hw/ide/internal.h:576
#16 ide_set_irq (bus=<optimized out>, bus=<optimized out>) at /home/maritns3/core/kvmqemu/include/hw/ide/internal.h:573
#17 ide_atapi_cmd_error (s=s@entry=0x555557aa7df8, sense_key=sense_key@entry=2, asc=asc@entry=58) at ../hw/ide/atapi.c:193
#18 0x0000555555a9f622 in ide_atapi_cmd (s=0x555557aa7df8) at ../hw/ide/atapi.c:1356
#19 0x0000555555980bce in ide_data_writel (opaque=<optimized out>, addr=<optimized out>, val=0) at ../hw/ide/core.c:2398
#20 0x0000555555cd2611 in memory_region_write_accessor (mr=mr@entry=0x555557ba5640, addr=0, value=value@entry=0x7fffe888bbe8, size=size@entry=4, shift=<optimized out>,mask=mask@entry=4294967295, attrs=...) at ../softmmu/memory.c:492
#21 0x0000555555ccea9e in access_with_adjusted_size (addr=addr@entry=0, value=value@entry=0x7fffe888bbe8, size=size@entry=4, access_size_min=<optimized out>, access_size_max=<optimized out>, access_fn=access_fn@entry=0x555555cd2580 <memory_region_write_accessor>, mr=0x555557ba5640, attrs=...) at ../softmmu/memory.c:554
#22 0x0000555555cd1b47 in memory_region_dispatch_write (mr=mr@entry=0x555557ba5640, addr=0, data=<optimized out>, data@entry=0, op=op@entry=MO_32, attrs=attrs@entry=...) at ../softmmu/memory.c:1504
#23 0x0000555555ca102d in address_space_stl_internal (endian=DEVICE_NATIVE_ENDIAN, result=0x0, attrs=..., val=0, addr=<optimized out>, as=<optimized out>) at /home/maritns3/core/kvmqemu/include/exec/memory.h:2868
#24 address_space_stl (as=<optimized out>, addr=<optimized out>, val=0, attrs=..., result=0x0) at /home/maritns3/core/kvmqemu/memory_ldst.c.inc:350
#25 0x00007fff959ff27a in code_gen_buffer ()
#26 0x0000555555cd7c2d in cpu_tb_exec (tb_exit=<synthetic pointer>, itb=<optimized out>, cpu=0x555556aff890) at ../accel/tcg/cpu-exec.c:353
#27 cpu_loop_exec_tb (tb_exit=<synthetic pointer>, last_tb=<synthetic pointer>, tb=<optimized out>, cpu=0x555556aff890) at ../accel/tcg/cpu-exec.c:812
#28 cpu_exec (cpu=cpu@entry=0x555556af7000) at ../accel/tcg/cpu-exec.c:970
#29 0x0000555555c3ee57 in tcg_cpus_exec (cpu=cpu@entry=0x555556af7000) at ../accel/tcg/tcg-accel-ops.c:67
#30 0x0000555555cb86c3 in rr_cpu_thread_fn (arg=arg@entry=0x555556b02660) at ../accel/tcg/tcg-accel-ops-rr.c:216
#31 0x0000555555e55903 in qemu_thread_start (args=<optimized out>) at ../util/qemu-thread-posix.c:541
#32 0x00007ffff628d609 in start_thread (arg=<optimized out>) at pthread_create.c:477
#33 0x00007ffff61b4293 in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:95
```



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
- hw/intc/i8259_common.c
- hw/i386/kvm/i8259.c
- hw/intc/i8259.c

pc_i8259_create 中

## select 总结
这几个东西的初始化都是 kvm 的事情, 但是 apic_get_class 中进行判断的，无论是 tcg 还是 kvm 的

- kvm_irqchip_create 使用 kvm 成员变量来初始化全局变量，也就是那些 macro

KVMState::kernel_irqchip_allowed : 似乎只是决定了 apic 啊
KVMState::kernel_irqchip_required 对于 x86 而言，没有任何意义的


- kvm_set_kernel_irqchip 真的理解了吗 ?
  - ON / OFF / SPLIT 就是用于描述，APIC kernel 里面模拟，其他的取决于 split 的情况

```c
// 表示 apic 在内核中间
#define kvm_irqchip_in_kernel() (kvm_kernel_irqchip)
// 表示 ioapic pic 在用户态
#define kvm_irqchip_is_split() (kvm_split_irqchip)
```
kvm_irqchip_create => kvm_arch_irqchip_create 中初始化了 kvm_split_irqchip 的内容

为此构建出来了几个 macro 来描述 split 的情况
```c
#define kvm_pit_in_kernel() \
    (kvm_irqchip_in_kernel() && !kvm_irqchip_is_split())
#define kvm_pic_in_kernel()  \
    (kvm_irqchip_in_kernel() && !kvm_irqchip_is_split())
#define kvm_ioapic_in_kernel() \
    (kvm_irqchip_in_kernel() && !kvm_irqchip_is_split())
```


- [x] 如何初始化出来 split 的效果来啊, split 的效果到底意味着什么 ?
  - ioapic_init_gsi / pc_i8259_create 都非常类似，如果是 split 的，那么就初始化出来和 tcg 版本相同的样子出来

所以，最后来分析一下为什么 hw/intc/ioapic.c 会出现 kvm_irqchip_is_split
- 其实主要就是 ioapic_service 了，因为是 split 的，所以需要将消息注入到 kvm 中间，似乎还是很有道理的

## interrupt 机制
- apic_local_deliver : 在 apic 中存在大量的模拟
  - cpu_interrupt
    - generic_handle_interrupt
      - `cpu->interrupt_request |= mask;` 

在 cpu_handle_interrupt 中，会处理几种特殊情况，默认是 `cc->cpu_exec_interrupt(cpu, interrupt_request)`
也就是 x86_cpu_exec_interrupt, 在这里根据 idt 之类的中断处理需要的地址, 然后跳转过去执行的

## kvm irq routing
- 为什么需要 irq routing
  - 同时 kvm_vm_ioctl_irq_line 其实是一个标准的接口, arm 对于函数实现就是不需要 irq routing 的操作
      - 所以，当调用 KVM_IRQ_LINE 的时候, 只是需要提供一个中断号
  - kvm 和 QEMU 需要同时支持各种类型的主板，类似一个 kbd 之类的设备其实也不知道具体是和 pic 还是 ioapic 上的, 目前这个处理方法是有效并且最简单的
  - kvm 不知道 os 使用的是哪一个中断控制器

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

## GSIState
每次调用中断，都是从 X86MachineState::gsi 开始，其在 pc_gsi_create 中初始化, handler 为 gsi_handler, gsi_handler 再去调用 GSIState 中三个 irqchip 对应的 handler

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
    - qemu_allocate_irqs(kvm_pic_set_irq, NULL, ISA_NUM_IRQS); // 创建出来的 qemu_irq 赋值给
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

- [ ] APICCommonState::vapic_paddr 似乎和 kvmvapic 有关的
## kvm ioapic
```c
struct KVMIOAPICState {
    IOAPICCommonState ioapic;
    uint32_t kvm_gsi_base;
};
```
除了和初始化相关的，几乎没有什么需要做的事情，而且 kvm_gsi_base 的取值总是 0 的

总体来说，就是一些简单的保存 恢复工作 : kvm_ioapic_get / kvm_ioapic_put, 通过 KVM_GET_IRQCHIP 来实现

顺便回答一下 KVM_SET_IRQCHIP 的作用是什么, 因为 kvm 在内核中维护 irqchip 的状态，
但是实际上，在用户态也是可以设置 irqchip 的，从而实现 machine migration

## kvm pic
和 kvm ioapic 类似, 不过有趣的是 lapic 使用的是 KVM_SET_LAPIC ，而 ioapic 和 pic 都是 KVM_GET_IRQCHIP

## tcg pic
- pc_i8259_create
  - `i8259_init(isa_bus, x86_allocate_cpu_irq())`

可以分析一下 qemu_irq_lower 关于 gpio 的内容，pic 内部是存在两级级联的中断的，
而中断最后导入到 x86_allocate_cpu_irq() 创建的 qemu_irq 中处理
```c
/*
#0  pic_irq_request (opaque=0x0, irq=0, level=0) at ../hw/i386/x86.c:530
#1  0x00005555559641de in qemu_irq_lower (irq=<optimized out>) at /home/maritns3/core/kvmqemu/include/hw/irq.h:17
#2  pic_update_irq (s=0x555556999920) at ../hw/intc/i8259.c:116
#3  pic_update_irq (s=0x555556999920) at ../hw/intc/i8259.c:107
#4  0x000055555596497e in pic_set_irq (opaque=0x555556999920, irq=<optimized out>, level=0) at ../hw/intc/i8259.c:156
#5  0x0000555555b92674 in gsi_handler (opaque=0x555556b0b1f0, n=4, level=0) at ../hw/i386/x86.c:596
```

- 分析 pic_irq_request
    - [x] 为什么还存在 kvm_irqchip_in_kernel 的判断啊? 因为 irq chip 的 split 原因
    - 一般来说，pic 是和 lapic 联系在一起的，但是实际上可以越过去

- [ ] apic_check_pic : 这个东西是做啥的
  - 和 apic_update_irq 做出类似的判断
```c
  if (!apic_accept_pic_intr(dev) || !pic_get_output(isa_pic)) {
```


## tcg 模式下的 irq routing
- 还是会调用 gsi_handler，而 gsi_handler 对于中断号小于 16 的会连续调用两次, 分别是
  - pic_irq_request
  - ioapic_set_irq

和 kvm 非常类似，在系统启动之后，抛弃使用 pic
具体表现为 apic_accept_pic_intr 的这个判断失败

- [x] 找到对应的源代码位置, 将 apic_accept_pic_intr disabled 的
  - 就是在 apic_mem_write 的位置
  - [ ] 但是进行 mem write 的操作具体发生在 guest 的哪一个源码，暂时是不知道的

```c
/*
#0  apic_set_irq (s=0x55555698a050, vector_num=0, trigger_mode=0) at ../hw/intc/apic.c:401
#1  0x0000555555c6a2d6 in apic_local_deliver (s=0x55555698a050, vector=3) at ../hw/intc/apic.c:166
#2  0x0000555555b90d74 in pic_irq_request (opaque=<optimized out>, irq=<optimized out>, level=1) at ../hw/i386/x86.c:540
#3  pic_irq_request (opaque=<optimized out>, irq=<optimized out>, level=1) at ../hw/i386/x86.c:529
#4  0x0000555555964178 in qemu_irq_raise (irq=<optimized out>) at /home/maritns3/core/kvmqemu/include/hw/irq.h:12
#5  pic_update_irq (s=0x555556999520) at ../hw/intc/i8259.c:114
#6  0x000055555596493e in pic_set_irq (opaque=0x555556999520, irq=<optimized out>, level=1) at ../hw/intc/i8259.c:156
#7  0x0000555555b92634 in gsi_handler (opaque=0x555556adfc30, n=0, level=1) at ../hw/i386/x86.c:596
```

> The local APIC is enabled at boot-time and can be disabled by clearing bit 11 of the IA32_APIC_BASE Model Specific Register (MSR). [^3]

> In general, one pin is used for chaining the legacy PIC and the other for NMIs (Or occasionally SMIs). No devices are actually connected to the LINT pins for a couple of reasons, one of which is that id just doesn't work on multiprocessor systems (They inevitably get delivered to one core).
> In practice, all devices are connected to the I/O APIC and/or PIC.[^4]

## [ ] tcg ioapic
ioapic_realize 中的初始化，总是 ioapic_set_irq 来作为入口的
- ioapic_service
  - [ ] stl_le_phys(ioapic_as, info.addr, info.data);
      - 实际上，这就是通往 apic 的过程，上面的注释也分析过，采用类似 msi 的过程
      - stl_le_phys 最终会调用起来 apic 的 handler 的

- [ ] 跟踪一下 ioapic 的 ioredtbl 是如何处理的

- [ ] cat /proc/interrupts 中的编号到底是啥啊

- [ ] Understanding Linux Kernel 中的 : Table 4-2. Interrupt vectors in Linux
    - 这个东西实际上是 idt 的编号啊

| Vector range        | Use                                                                                                         |
|---------------------|-------------------------------------------------------------------------------------------------------------|
| 0–19 (0x0-0x13)     | Nonmaskable interrupts and exceptions                                                                       |
| 20–31 (0x14-0x1f)   | Intel-reserved                                                                                              |
| 32–127 (0x20-0x7f)  | External interrupts (IRQs)                                                                                  |
| 128 (0x80)          | Programmed exception for system calls (see Chapter 10)                                                      |
| 129–238 (0x81-0xee) | External interrupts (IRQs)                                                                                  |
| 239 (0xef)          | Local APIC timer interrupt (see Chapter 6)                                                                  |
| 240 (0xf0)          | Local APIC thermal interrupt (introduced in the Pentium 4 models)                                           |
| 241–250 (0xf1-0xfa) | Reserved by Linux for future use                                                                            |
| 251–253 (0xfb-0xfd) | Interprocessor interrupts (see the section "Interprocessor Interrupt Handling" later in this chapter)       |
| 254 (0xfe)          | Local APIC error interrupt (generated when the local APIC detects an erroneous condition)                   |
| 255 (0xff)          | Local APIC spurious interrupt (generated if the CPU masks an interrupt while the hardware device raises it) |

- ioapic_service : 注意 ioapic 也是有 irr 的，当一个引脚上有信号，那么就 irr 数组对应的就会 assert 一下, io apic 的 irr 是在 ioapic_set_irq 里面设置的
  - ioapic_entry_parse : 填充 ioapic_entry_info，实际上就是从 ioredtbl 中解析填充数组，最后构建 MSI 的信息, 其中比较关键是知道了 vector 是什么

通过 ioapic_mem_write 来让软件来编码这些优先级

- [x] 所以 ioredtbl 中间编码了优先级没有啊
  - 应该是没有的

- [x] 在 ioapic_service 直接调用 stl_le_phys ，最后是如何选定上一个 CPU 的
  - 而且 cpu 中的所有的 apic 的地址空间都是相同的，现在模拟设备，从 ioapic 的写地址空间，最后是如何选择出来到底是谁?
  - cpu_get_current_apic : 当前线程对应的 vcpu 持有的 apic
  - 对此 ioapic_entry_parse 中的注释的解释是，发送给哪一个 CPU 的事情并不想去考虑

## Linux 内核如何配置 ioapic 的
在 x86 这一边采用了 gsi 的概念
gsi，等价于 ioapic 的引脚编号，pic 的引脚编号，linux irq 的
而 vector 是从 0x20 开始分配的，vector 是 idt 的编号，ioapic 映射从 gsi 到 vector

在 Linux 内核这一侧, 配置 ioapic 也是采用 msi 的方式:

- mp_irqdomain_activate
  - ioapic_configure_entry : 看 `__add_pin_to_irq_node` 的注释: The common case is 1:1 `IRQ<->pin` mappings.
      - ioapic_setup_msg_from_msi : 构造 msi
          - irq_chip_compose_msi_msg :  构建 struct msi_msg
              - irq_chip::irq_compose_msi_msg
                - x86_vector_msi_compose_msg  : 这就是注册的 hook 函数
          - 使用 struct msi_msg 来填充 struct IO_APIC_route_entry
      - `__ioapic_write_entry` : 将 msi 信息写入到 ioapic 的重定向表格

下面是一个典型的 mp_irqdomain_activate 的调用路径:
```c
/*
>>> bt
#0  mp_irqdomain_activate (domain=0xffff888100101a00, irq_data=0xffff888100100e28, reserve=false) at arch/x86/kernel/apic/io_apic.c:3076
#1  0xffffffff81136d4e in __irq_domain_activate_irq (irqd=irqd@entry=0xffff888100100e28, reserve=reserve@entry=false) at kernel/irq/irqdomain.c:1761
#2  0xffffffff811387f0 in irq_domain_activate_irq (irq_data=irq_data@entry=0xffff888100100e28, reserve=reserve@entry=false) at kernel/irq/irqdomain.c:1784
#3  0xffffffff81135bfb in irq_activate (desc=desc@entry=0xffff888100100e00) at kernel/irq/chip.c:291
#4  0xffffffff81133515 in __setup_irq (irq=irq@entry=11, desc=desc@entry=0xffff888100100e00, new=new@entry=0xffff888100282580) at kernel/irq/manage.c:1709
#5  0xffffffff81133a57 in request_threaded_irq (irq=11, handler=handler@entry=0xffffffff81772720 <nvme_irq>, thread_fn=thread_fn@entry=0x0 <fixed_percpu_data>, irqflags=irqflags@entry=128, devname=devname@entry=0xffff8881003d31a0 "nvme1q0", dev_id=dev_id@entry=0xffff888101380400) at kernel/irq/manage.c:2173
#6  0xffffffff814bd11e in pci_request_irq (dev=0xffff888100843000, nr=0, handler=handler@entry=0xffffffff81772720 <nvme_irq>, thread_fn=thread_fn@entry=0x0 <fixed_percpu_data>, dev_id=0xffff888101380400, fmt=fmt@entry=0xffffffff824da235 "d") at drivers/pci/irq.c:48
#7  0xffffffff81771246 in queue_request_irq (nvmeq=nvmeq@entry=0xffff888101380400) at drivers/nvme/host/pci.c:1537
#8  0xffffffff81773727 in nvme_pci_configure_admin_queue (dev=0xffff8881009b5000) at drivers/nvme/host/pci.c:1737
#9  nvme_reset_work (work=0xffff8881009b56d0) at drivers/nvme/host/pci.c:2602
#10 0xffffffff810f567f in process_one_work (worker=0xffff888100817e40, work=0xffff8881009b56d0) at kernel/workqueue.c:2276
#11 0xffffffff810f5865 in worker_thread (__worker=0xffff888100817e40) at kernel/workqueue.c:2422
#12 0xffffffff810fcc02 in kthread (_create=0xffff8881008f1e80) at kernel/kthread.c:319
#13 0xffffffff81001992 in ret_from_fork () at arch/x86/entry/entry_64.S:295
#14 0x0000000000000000 in ?? ()
```

而至于 vector 的具体数值的，就要看 apic_update_irq_cfg 了:

```c
/*
>>> bt
#0  apic_update_irq_cfg (irqd=irqd@entry=0xffff88810004e440, vector=vector@entry=239, cpu=0) at arch/x86/kernel/apic/vector.c:120
#1  0xffffffff810be093 in vector_assign_managed_shutdown (irqd=0xffff88810004e440) at arch/x86/kernel/apic/vector.c:181
#2  reserve_irq_vector_locked (irqd=irqd@entry=0xffff88810004e440) at arch/x86/kernel/apic/vector.c:208
#3  0xffffffff810be419 in reserve_irq_vector (irqd=0xffff88810004e440) at arch/x86/kernel/apic/vector.c:216
#4  assign_irq_vector_policy (info=0xffffffff82603da0, irqd=0xffff88810004e440) at arch/x86/kernel/apic/vector.c:309
#5  x86_vector_alloc_irqs (arg=<optimized out>, nr_irqs=<optimized out>, virq=<optimized out>, domain=<optimized out>) at arch/x86/kernel/apic/vector.c:592
#6  x86_vector_alloc_irqs (domain=0xffff88810004d180, virq=1, nr_irqs=1, arg=0xffffffff82603da0) at arch/x86/kernel/apic/vector.c:531
#7  0xffffffff810c035f in mp_irqdomain_alloc (domain=0xffff888100101a00, virq=1, nr_irqs=1, arg=0xffffffff82603da0) at arch/x86/kernel/apic/io_apic.c:3023
#8  0xffffffff81138135 in irq_domain_alloc_irqs_hierarchy (arg=0xffffffff82603da0, nr_irqs=1, irq_base=1, domain=0xffff888100101a00) at kernel/irq/irqdomain.c:1421
#9  __irq_domain_alloc_irqs (domain=domain@entry=0xffff888100101a00, irq_base=irq_base@entry=1, nr_irqs=nr_irqs@entry=1, node=node@entry=-1, arg=arg@entry=0xffffffff826
03da0, realloc=realloc@entry=true, affinity=0x0 <fixed_percpu_data>) at kernel/irq/irqdomain.c:1477
#10 0xffffffff810bf23a in alloc_isa_irq_from_domain (domain=domain@entry=0xffff888100101a00, irq=irq@entry=1, ioapic=ioapic@entry=0, info=info@entry=0xffffffff82603da0,
 pin=1) at arch/x86/kernel/apic/io_apic.c:1008
#11 0xffffffff810bff43 in mp_map_pin_to_irq (gsi=gsi@entry=1, idx=idx@entry=5, ioapic=ioapic@entry=0, pin=pin@entry=1, flags=1, info=info@entry=0x0 <fixed_percpu_data>)
 at arch/x86/kernel/apic/io_apic.c:1057
#12 0xffffffff810c00ed in pin_2_irq (idx=5, ioapic=ioapic@entry=0, pin=pin@entry=1, flags=flags@entry=1) at arch/x86/kernel/apic/io_apic.c:1103
#13 0xffffffff82dbe3a8 in setup_IO_APIC_irqs () at arch/x86/kernel/apic/io_apic.c:1222
#14 setup_IO_APIC () at arch/x86/kernel/apic/io_apic.c:2419
#15 0xffffffff82dbd0cb in apic_bsp_setup (upmode=<optimized out>) at arch/x86/kernel/apic/apic.c:2606
#16 apic_intr_mode_init () at arch/x86/kernel/apic/apic.c:1444
#17 0xffffffff82db1cd8 in x86_late_time_init () at arch/x86/kernel/time.c:100
#18 0xffffffff82daa109 in start_kernel () at init/main.c:1080
#19 0xffffffff81000107 in secondary_startup_64 () at arch/x86/kernel/head_64.S:283
#20 0x0000000000000000 in ?? ()

>>> bt
#0  apic_update_irq_cfg (irqd=irqd@entry=0xffff88810004e4c0, vector=48, cpu=0) at arch/x86/kernel/apic/vector.c:120
#1  0xffffffff810be258 in vector_configure_legacy (apicd=0xffff88810004e500, irqd=0xffff88810004e4c0, virq=0) at arch/x86/kernel/apic/vector.c:519
#2  x86_vector_alloc_irqs (arg=<optimized out>, nr_irqs=<optimized out>, virq=<optimized out>, domain=<optimized out>) at arch/x86/kernel/apic/vector.c:588
#3  x86_vector_alloc_irqs (domain=0xffff88810004d180, virq=0, nr_irqs=1, arg=0xffffffff82603da0) at arch/x86/kernel/apic/vector.c:531
#4  0xffffffff810c035f in mp_irqdomain_alloc (domain=0xffff888100101a00, virq=0, nr_irqs=1, arg=0xffffffff82603da0) at arch/x86/kernel/apic/io_apic.c:3023
#5  0xffffffff81138135 in irq_domain_alloc_irqs_hierarchy (arg=0xffffffff82603da0, nr_irqs=1, irq_base=0, domain=0xffff888100101a00) at kernel/irq/irqdomain.c:1421
#6  __irq_domain_alloc_irqs (domain=domain@entry=0xffff888100101a00, irq_base=irq_base@entry=0, nr_irqs=nr_irqs@entry=1, node=node@entry=-1, arg=arg@entry=0xffffffff82603da0, realloc=realloc@entry=true, affinity=0x0 <fixed_percpu_data>) at kernel/irq/irqdomain.c:1477
#7  0xffffffff810bf23a in alloc_isa_irq_from_domain (domain=domain@entry=0xffff888100101a00, irq=irq@entry=0, ioapic=ioapic@entry=0, info=info@entry=0xffffffff82603da0,pin=2) at arch/x86/kernel/apic/io_apic.c:1008
#8  0xffffffff810bff43 in mp_map_pin_to_irq (gsi=gsi@entry=2, idx=idx@entry=0, ioapic=ioapic@entry=0, pin=pin@entry=2, flags=1, info=info@entry=0x0 <fixed_percpu_data>) at arch/x86/kernel/apic/io_apic.c:1057
#9  0xffffffff810c00ed in pin_2_irq (idx=0, ioapic=ioapic@entry=0, pin=pin@entry=2, flags=flags@entry=1) at arch/x86/kernel/apic/io_apic.c:1103
#10 0xffffffff82dbe3a8 in setup_IO_APIC_irqs () at arch/x86/kernel/apic/io_apic.c:1222
#11 setup_IO_APIC () at arch/x86/kernel/apic/io_apic.c:2419
#12 0xffffffff82dbd0cb in apic_bsp_setup (upmode=<optimized out>) at arch/x86/kernel/apic/apic.c:2606
#13 apic_intr_mode_init () at arch/x86/kernel/apic/apic.c:1444
#14 0xffffffff82db1cd8 in x86_late_time_init () at arch/x86/kernel/time.c:100
#15 0xffffffff82daa109 in start_kernel () at init/main.c:1080
#16 0xffffffff81000107 in secondary_startup_64 () at arch/x86/kernel/head_64.S:283
#17 0x0000000000000000 in ?? ()

#0  apic_update_irq_cfg (irqd=irqd@entry=0xffff88810004e840, vector=33, cpu=1) at arch/x86/kernel/apic/vector.c:120
#1  0xffffffff810bd9a8 in assign_vector_locked (irqd=irqd@entry=0xffff88810004e840, dest=dest@entry=0xffffffff82efdb60 <vector_searchmask>) at arch/x86/kernel/apic/vector.c:253
#2  0xffffffff810bdb9b in assign_irq_vector_any_locked (irqd=0xffff88810004e840) at arch/x86/kernel/apic/vector.c:279
#3  0xffffffff810bdfa4 in activate_reserved (irqd=0xffff88810004e840) at arch/x86/kernel/apic/vector.c:393
#4  x86_vector_activate (dom=<optimized out>, irqd=0xffff88810004e840, reserve=<optimized out>) at arch/x86/kernel/apic/vector.c:462
#5  0xffffffff81136d4e in __irq_domain_activate_irq (irqd=0xffff88810004e840, reserve=reserve@entry=false) at kernel/irq/irqdomain.c:1761
#6  0xffffffff81136d2d in __irq_domain_activate_irq (irqd=irqd@entry=0xffff888100100a28, reserve=reserve@entry=false) at kernel/irq/irqdomain.c:1758
#7  0xffffffff811387f0 in irq_domain_activate_irq (irq_data=irq_data@entry=0xffff888100100a28, reserve=reserve@entry=false) at kernel/irq/irqdomain.c:1784
#8  0xffffffff81135bfb in irq_activate (desc=desc@entry=0xffff888100100a00) at kernel/irq/chip.c:291
#9  0xffffffff81133515 in __setup_irq (irq=irq@entry=9, desc=desc@entry=0xffff888100100a00, new=new@entry=0xffff888100325a00) at kernel/irq/manage.c:1709
#10 0xffffffff81133a57 in request_threaded_irq (irq=9, handler=handler@entry=0xffffffff814e36f0 <acpi_irq>, thread_fn=thread_fn@entry=0x0 <fixed_percpu_data>, irqflags=irqflags@entry=128, devname=devname@entry=0xffffffff8243c085 "acpi", dev_id=dev_id@entry=0xffffffff814e36f0 <acpi_irq>) at kernel/irq/manage.c:2173
#11 0xffffffff814e3af7 in request_irq (dev=0xffffffff814e36f0 <acpi_irq>, name=0xffffffff8243c085 "acpi", flags=128, handler=0xffffffff814e36f0 <acpi_irq>, irq=<optimized out>) at ./include/linux/interrupt.h:167
#12 acpi_os_install_interrupt_handler (gsi=9, handler=handler@entry=0xffffffff814ffd00 <acpi_ev_sci_xrupt_handler>, context=0xffff888100309ae0) at drivers/acpi/osl.c:586
#13 0xffffffff814ffd47 in acpi_ev_install_sci_handler () at drivers/acpi/acpica/evsci.c:156
#14 0xffffffff814fd4c3 in acpi_ev_install_xrupt_handlers () at drivers/acpi/acpica/evevent.c:94
#15 0xffffffff82ddf987 in acpi_enable_subsystem (flags=flags@entry=2) at drivers/acpi/acpica/utxfinit.c:184
#16 0xffffffff82dddca0 in acpi_bus_init () at drivers/acpi/bus.c:1230
#17 acpi_init () at drivers/acpi/bus.c:1323
#18 0xffffffff81000def in do_one_initcall (fn=0xffffffff82dddc1c <acpi_init>) at init/main.c:1278
#19 0xffffffff82daa3d3 in do_initcall_level (command_line=0xffff8881001277c0 "root", level=4) at ./include/linux/compiler.h:250
#20 do_initcalls () at init/main.c:1367
#21 do_basic_setup () at init/main.c:1387
#22 kernel_init_freeable () at init/main.c:1589
#23 0xffffffff81c38121 in kernel_init (unused=<optimized out>) at init/main.c:1481
#24 0xffffffff81001992 in ret_from_fork () at arch/x86/entry/entry_64.S:295
#25 0x0000000000000000 in ?? ()
```

- acpi 的这个很有意思，最开始是 irq = 9 最后就变为 vector=33 了, 其他的几个标准注册是类似的
  - vector 数值的分配和 irq 没有关系，是从 irq_matrix_alloc  中分配的
  - irq_matrix::alloc_start 和 irq_matrix::alloc_end 决定了分配的起始位置
  - 从 acpi_ev_install_sci_handler 中间看，acpi 的 gsi = 9 是从 acpi 的 FADT 中间读取的


- 分析调用过程 : 在 ./x86-code-flow.md tirgger 的哪一个小节发现，从 common_interrupt 到 nvme_irq 之间，实际上存在多次中间调用啊
  - 才知道一个 irq_desc 同时存在 handle_irq 和 action，前者来注册 handle_edge_irq ，后者注册 nvme_irq
  - 整个的处理过程中，始终都是使用同一个 irq_desc
      - 所以，在 common_interrupt 的时候获取就是 linux irq 才对
  - [x] 分析一下，nvme_irq 的 backtrace 开始的时候 vector 是 37, 但是之后，但是到了 nvme_irq 的位置，irq = 24
    - 可以理解的
  - [x] 上面的 acpi 的例子 request_threaded_irq 的时候，其使用 irq = 9 来作为索引来获取 desc 的
    - 分析 apic_update_vector 可以看到, 通过 irq = 9 用于索引出来一个 desc，之后，该 desc 最后赋值给 vector

看一下 interrupt 的展开的全过程
```c
/**
 * idtentry_body - Macro to emit code calling the C function
 * @cfunc:		C function to be called
 * @has_error_code:	Hardware pushed error code on stack
 */
.macro idtentry_body cfunc has_error_code:req

	call	error_entry // 其中的 error_entry 是用于保存 pt_regs 的上下文的
	UNWIND_HINT_REGS

	movq	%rsp, %rdi			/* pt_regs pointer into 1st argument*/

	.if \has_error_code == 1
		movq	ORIG_RAX(%rsp), %rsi	/* get error code into 2nd argument*/
		movq	$-1, ORIG_RAX(%rsp)	/* no syscall to restart */
	.endif

	call	\cfunc

	jmp	error_return
.endm

/**
 * idtentry - Macro to generate entry stubs for simple IDT entries
 * @vector:		Vector number
 * @asmsym:		ASM symbol for the entry point
 * @cfunc:		C function to be called
 * @has_error_code:	Hardware pushed error code on stack
 *
 * The macro emits code to set up the kernel context for straight forward
 * and simple IDT entries. No IST stack, no paranoid entry checks.
 */
.macro idtentry vector asmsym cfunc has_error_code:req
SYM_CODE_START(\asmsym)
	UNWIND_HINT_IRET_REGS offset=\has_error_code*8
	ASM_CLAC

	.if \has_error_code == 0
		pushq	$-1			/* ORIG_RAX: no syscall to restart */
	.endif

	.if \vector == X86_TRAP_BP
		/*
		 * If coming from kernel space, create a 6-word gap to allow the
		 * int3 handler to emulate a call instruction.
		 */
		testb	$3, CS-ORIG_RAX(%rsp)
		jnz	.Lfrom_usermode_no_gap_\@
		.rept	6
		pushq	5*8(%rsp)
		.endr
		UNWIND_HINT_IRET_REGS offset=8
.Lfrom_usermode_no_gap_\@:
	.endif

	idtentry_body \cfunc \has_error_code

_ASM_NOKPROBE(\asmsym)
SYM_CODE_END(\asmsym)
.endm

/*
 * Interrupt entry/exit.
 *
 + The interrupt stubs push (vector) onto the stack, which is the error_code
 * position of idtentry exceptions, and jump to one of the two idtentry points
 * (common/spurious).
 *
 * common_interrupt is a hotpath, align it to a cache line
 */
.macro idtentry_irq vector cfunc
	.p2align CONFIG_X86_L1_CACHE_SHIFT
	idtentry \vector asm_\cfunc \cfunc has_error_code=1
.endm

/* Entries for common/spurious (device) interrupts */
#define DECLARE_IDTENTRY_IRQ(vector, func)				\
	idtentry_irq vector func

/* Device interrupts common/spurious */
DECLARE_IDTENTRY_IRQ(X86_TRAP_OTHER,	common_interrupt);
#ifdef CONFIG_X86_LOCAL_APIC
DECLARE_IDTENTRY_IRQ(X86_TRAP_OTHER,	spurious_interrupt);
#endif

/*
 * On syscall entry, this is syscall#. On CPU exception, this is error code.
 * On hw interrupt, it's IRQ number:
 */
#define ORIG_RAX 120
```
回忆一下 ucore 的设计，实际上，hw interrupt 的 error_code 是自己 push 进去的

- [x] asm_common_interrupt : 利用 macro idtentry 实现定义的
- [x] idt_setup_apic_and_irq_gates : 设置 idt 的跳转位置为 asm_common_interrupt

好的，总体来说, CPU 这边直接获取的就是 linux irq

再次回忆 Understanding, 忽然可以理解为什么 irq_matrix_alloc 的开始位置是 0x20 了

-  所以 ioapic 的编码是如何进行的
  - 当在 qemu 中间调用 gsi_handler 的 gsi 数值就是 kernel 中的 linux irq 的数值, 也就是 /proc/interrupts
  - ioapic 中编码从 gsi 到 vector 的操作就是发生在 : ioapic_configure_entry : 给定一定 gsi ，知道其 vector 最后编码上去
  - ioapic 之类的和 Loongarch 的中断控制器还是非常不一样的啊

## [ ] tcg apic

- apic timer : 总体来说，timer 是比较容易处理的
  - apic_timer 被周期性的触发
    - [ ] 思考一下如何获取 clock time, 实际上，guest 操作系统可以主动校准实践
    - QEMU_CLOCK_VIRTUAL : 当虚拟机停下来的时候，时钟中断需要停止下来
  - 考虑一个小问题，所有的 vCPU 都是需要接受 local timer 的时钟的，难道为此需要创建出来多个 timer 吗 ?
    - 是的, 而且 timer 这个线程是在 main_loop_wait => qemu_clock_run_all_timers 中使用一个新的线程来进行的

- [ ] apic_eoi : 和 10.8.5 中描述的一致，但是
  - [ ] 10.8.5 : 手册中间分析的 ioapic 的 broadcast 是什么意思
  - [x] apic_sync_vapic : 这个是处理 kvm 的，暂时不分析
- [ ] isr

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

- apic_update_irq 的分析
  - apic_poll_irq : 如果中断是来自于其他的 thread，那么就采用这种方式，比如时钟中断
    - 因为时钟是在另一个线程处理的，所以需要实现
  - 如果不是来自于 pic 的中断，那就清理掉这个中断

- [ ] cr8 和 tpr 的关系是什么？
    - apic_set_tpr 的唯一调用者是 helper_write_crN
    - 从 apic_set_tpr 和 apic_get_tpr 的效果看，SDM 10-18 的 sub-class 的实际上没有用的
    - [^5] 中间描述，如果一个中断的优先级不够, 那么是无法通知 CPU 的，那么如何知道一个中断的优先级
    - [ ] 似乎，实际上，tpr 的意义在于计算出来 ppr, 因为 ppr 是 tpr + isrv 中的较大值，只有一个中断同时超过两者才可以

- [ ] irr / isr 的操作是什么?
    - 10.8.4 Interrupt Acceptance for Fixed Interrupts : 虽然在描述了，但是
        - 为什么存在 256 bit，为什么设置出来了两个 register 啊

- apic_get_interrupt : 从 irr 中接受中断之后，然后立刻装换到系统中间

The local APIC queues the fixed interrupts that it accepts in one of two interrupt pending registers: the interrupt
request register (IRR) or in-service register (ISR).

- [ ] 为什么 apic_get_interrupt 返回的数值都是 236 之类的, 操作系统是如何解析的
    - [ ] apic_local_deliver : 似乎和 lvt 有关的
    - [ ] 众所周知，idt 中间一共是存在 256 项目的，其中的一些还是和 exception 相关的，是如何实现让的 apic 的引脚 和 idt 产生联系的
        - 忽然间意识到一个问题，之前说的 gsi 什么的，其实那都是 apic 和 pic 的引脚啊，但是实际上，CPU 看到的就是 icr 之类的东西了
        - [ ] 是所有的中断的入口都是相同的，还是对于 irr 256 每一个都是可以建立对应的 idt 的


lvt  中的取值总是在发生改变的, 但是
apic_timer => apic_local_deliver => apic_set_irq 的过程中，本来 apic 的中断是 APIC_LVT_TIMER 的，但是最后装换为 236 了

```txt
[0=236] [1=65536] [2=65536] [3=67328] [4=1024] [5=254]
```

从 apic_mem_write 的操作看，似乎, msi 根本不用遵守 io apic pin 的要求哇

但是，需要注意一个问题，之前描述的是 ioapic 的引脚，但是现在是 apic

- [x] 到底存在那些 idt 来响应中断
    - 在 idt.c  和 irq.c 中定义相关的项目
    - common_interrupt 是一般的入口, 但是其他的中断，例如 ipi 有其他的特殊入口，这个和 Loongarch 很类似
- [x] 当中断的处理函数中间，是否存在检测分析 isr 从而知道是那个中断的函数
  - DEFINE_IDTENTRY_IRQ 的注释说，The vector number is pushed by the low level entry stub

破案了，原来是 ioapic_entry_parse 将 ioapic 的 pin 最后装换为 apic 上的 irr 之类的

## 键盘的中断是如何注入的
参考 vn/hack/qemu/internals/i8042.md

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

[^1]: https://events.static.linuxfound.org/sites/events/files/slides/VT-d%20Posted%20Interrupts-final%20.pdf
[^2]: https://luohao-brian.gitbooks.io/interrupt-virtualization/content/qemu-kvm-zhong-duan-xu-ni-hua-kuang-jia-fen-679028-4e0a29.html
[^3]: https://wiki.osdev.org/APIC
[^4]: https://forum.osdev.org/viewtopic.php?f=1&t=22024
[^5]: https://stackoverflow.com/questions/51490552/how-is-cr8-register-used-to-prioritize-interrupts-in-an-x86-64-cpu
