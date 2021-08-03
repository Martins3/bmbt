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

- x86_cpu_apic_create
  - apic_get_class

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

