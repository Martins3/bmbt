## 一些根本的疑惑
- [x] 为什么那些长长的 acpi 调用，最后会选择 pch_pic 的
- [ ] 从哪里知道 cpu extioi 和 pch_pic 的链接到一起的
- [ ] 应该 msi 是放到 extioi 下，其地位和 pch 差不多的吧
- [ ] 中断负载均衡在哪里做的 ?

## 一些 irq_domain 中值得关注的函数
- irq_set_chained_handler_and_data
- irq_set_chip_and_handler


## irq domain
- [x] [What are linux irq domains, why are they needed?](https://stackoverflow.com/questions/34371352/what-are-linux-irq-domains-why-are-they-needed)
  - 基本的思路是，信号是逐级的传递到 CPU 中间的
  - 通过逐级的 irq domain, 将最开始的 hardware irq 映射为 linux irq, 而 linux irq 就是 device 注册的

从汇编代码 到 generic_handle_irq_desc 很容易理解，中间经过一次 gic 的 irq_domain，从 hwirq 找到 irq_desc，最后调用注册在上面的 irq handler
从代码中间找到如下内容:

```c
  ret = devm_request_threaded_irq(&pdev->dev, gpio_irq, NULL,
          max77620_gpio_irqhandler, IRQF_ONESHOT,
          "max77620-gpio", mgpio);
```

```c
static irqreturn_t max77620_gpio_irqhandler(int irq, void *data)
{
  struct max77620_gpio *gpio = data;
  unsigned int value, offset;
  unsigned long pending;
  int err;

  err = regmap_read(gpio->rmap, MAX77620_REG_IRQ_LVL2_GPIO, &value);
  if (err < 0) {
    dev_err(gpio->dev, "REG_IRQ_LVL2_GPIO read failed: %d\n", err);
    return IRQ_NONE;
  }

  pending = value;

  for_each_set_bit(offset, &pending, MAX77620_GPIO_NR) {
    unsigned int virq;

    virq = irq_find_mapping(gpio->gpio_chip.irq.domain, offset);
    handle_nested_irq(virq);
  }

  return IRQ_HANDLED;
}
```
也就是说，当 irq 到达之后，下级的中断控制器和普通的设备其实没有区别，
调用其 handler, 然后中断控制器决定如何处理自己得到的中断。

对于 gpio 也是建立对应的 irq domain 实现从 gpio 引脚编号 到 linux irq 之间的映射。

在 handle_nested_irq 中间:
1. 获取了 irq 就是获取了 irq_desc
2. 从而得到 handler 了

- [x] 好吧，流程的确是这个流程，但是既然存在 device tree, 完全可以在开机的时候，让所有的设备驱动都知道自己对应的
linux irq 是什么，根本不需要，首先在 gpio 的 domain 中间

如果 gic 接入了 gpio 中断控制器，叫做 gpio_a
从 linux 的角度看，他只是接收到 hwirq a, 但是到底是 gpio_a 引脚上的哪一个设备，
这是不清楚的，只能执行 gpio_a 的 handler, 在其中找到是哪一个引脚，然后就知道其 linux irq 是什么，
然后就可以执行对应的代码。

所以，依靠 dtb 即使可以知道每一个设备分配的 irq, 由于没有办法区分 gic 下的次级中断信息，还是需要 irq domain 计算
次级中断控制器的引脚到 linux irq 的映射。

- [ ] 但是我们还是无法理解:
  - [ ] MSI
  - [ ] PCIe 设备为什么可以 随插随用, 插入的时候自动注册 irq 来

> 相比这些东西，还是去理解一下到底 softirq, threaded irq，之类的东西是怎么链接起来的吧 !

[kernel doc](https://www.kernel.org/doc/html/latest/core-api/irq/irq-domain.html)

we need a mechanism to separate controller-local interrupt numbers, called hardware irq's, from Linux IRQ numbers.

[兰新宇](https://zhuanlan.zhihu.com/p/85353687)
> 虽然 radix tree、线性映射和硬件映射不会同时存在，但它们在"irq_domain"中是定义在一起的，"revmap_tree"只对 radix tree 有意义，revmap_size 和 linear_revmap[]只对线性映射有意义，revmap_direct_max_irq 则只对硬件映射有意义。


// --------------------- trace the functions -------------------------------------------

1. alloc_desc <=== alloc_descs <=== `__irq_alloc_descs` <=== irq_domain_alloc_descs <=== `__irq_domain_alloc_irqs` <==== alloc_irq_from_domain <==== mp_map_pin_to_irq <==== pin_2_irq , mp_map_gsi_to_irq <== pin_2_irq <== setup_IO_APIC_irqs <== setup_IO_APIC <== apic_bsp_setup <== apic_intr_mode_init
  - maybe this how initial irq set up

2. default_get_smp_config ==> check_physptr ==> smp_read_mpc ==> mp_register_ioapic ==> mp_irqdomain_create


- [x] apic_intr_mode_select : choose intr mode which can be determined by krenel parameter
- [ ] apic_intr_mode_init --> *default_setup_apic_routing* , apic_bsp_setup
- [x] setup_local_APIC : as name suggests, doing something like this:
- [ ] enable_IO_APIC : why we need i8259, maybe just legacy. it seems io apic has a different mmio space
- [ ] mp_pin_to_gsi
  - [ ] mpprase
  - [ ] what's gsi
  - [ ] why we have set up relation with ioapic and pin, and what's pin, it's pin of cpu ?


- [ ] mp_map_pin_to_irq
  - if irq domain already set up, return `irq_find_mapping`
  - otherwise, `alloc_irq_from_domain` firstly

// --------------------- trace the functions -------------------------------------------

## irq domain hierarchy
[kernel doc](https://www.kernel.org/doc/html/latest/core-api/irq/irq-domain.html)

There are four major interfaces to use hierarchy `irq_domain`:
- `irq_domain_alloc_irqs()`: allocate IRQ descriptors and interrupt controller related resources to deliver these interrupts.
- `irq_domain_free_irqs()`: free IRQ descriptors and interrupt controller related resources associated with these interrupts.
- `irq_domain_activate_irq()`: activate interrupt controller hardware to deliver the interrupt.
- `irq_domain_deactivate_irq()`: deactivate interrupt controller hardware to stop delivering the interrupt.


```c
static const struct irq_domain_ops x86_vector_domain_ops = {
  .alloc    = x86_vector_alloc_irqs,
  .free   = x86_vector_free_irqs,
  .activate = x86_vector_activate,
  .deactivate = x86_vector_deactivate,
#ifdef CONFIG_GENERIC_IRQ_DEBUGFS
  .debug_show = x86_vector_debug_show,
#endif
};

const struct irq_domain_ops mp_ioapic_irqdomain_ops = {
  .alloc    = mp_irqdomain_alloc,
  .free   = mp_irqdomain_free,
  .activate = mp_irqdomain_activate,
  .deactivate = mp_irqdomain_deactivate,
};

static const struct irq_domain_ops msi_domain_ops = {
  .alloc    = msi_domain_alloc,
  .free   = msi_domain_free,
  .activate = msi_domain_activate,
  .deactivate = msi_domain_deactivate,
};
```

:cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat:


To support such a hardware topology and make software architecture match hardware architecture,
an `irq_domain` data structure is built for each interrupt controller and those `irq_domains` are organized into hierarchy.
When building `irq_domain` hierarchy, the `irq_domain` near to the device is **child** and the `irq_domain` near to CPU is parent.
So a hierarchy structure as below will be built for the example above:
```plain
CPU Vector irq_domain (root irq_domain to manage CPU vectors)
        ^
        |
Interrupt Remapping irq_domain (manage irq_remapping entries)
        ^
        |
IOAPIC irq_domain (manage IOAPIC delivery entries/pins)
```
:cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat::cat:


- [ ] mp_irqdomain_alloc : setup mapping from child chip irq to parent irq which is IR or vector
- [ ] x86_vector_alloc_irqs : it will update vector at last

## 使用一个小例子分析一下

### cpu
1. 定义一个 irq_chip ，其中注册的 hook 用于操作芯片，比如 lapic 或者 apic，在龙芯中就是 csr ecfg 中一些位
2. irq_set_chip_and_handler :
  - [x] 应该测试一下 loongarch_cpu_intc_map 中的 irq 是谁
    - 50 ~ 64 之间的
  - 在这里注意 handle 和 action 是一个不同的东西的
```c
  // set chip
  desc->irq_data.chip = chip;
  // set handler
  desc->handle_irq = handle;
```
3. irq_domain_ops
  - irq_domain_ops::maps 描述 irq 和 hwirq 之间的映射关系


- 在代码编写的时候，不知道一个中断的 linux irq 是什么，那个东西是动态分配的
- 在系统初始化的时候，通过 acpi 之类的，操作系统会知道中断是如何逐个传递到 cpu 哪里的
- [ ] 应该找到从 extioi 到 cpu 的 linux irq 的分配过程


## extioi

```c
  irq_set_chained_handler_and_data(cascade,
          extioi_irq_dispatch,
          extioi_priv[nr_extioi]);
```
- [ ] 没有搞清楚 extioi_irq_dispatch 是被谁调用的
  - [ ] loongarch_cpu_intc_map 中注册了 handle_percpu_irq

```c
static const struct irq_domain_ops extioi_domain_ops = {
  .translate  = extioi_domain_translate,
  .alloc    = extioi_domain_alloc,
  .free   = extioi_domain_free,
};
```
- 在 cpu inc 中，linux irq 是首先创建好的，然后使用 irq_domain_associate_many 关联的
但是在此处，是在 extioi_domain_alloc 的时候自动创建的

- [ ] 在 extioi_domain_alloc 中调用 extioi_domain_translate，其参数为 extioi_domain_translate，其中的参数为 irq_fwspec，有点好奇这个东西是从哪里来的

```c
#0  loongarch_cpu_intc_map (d=0x900000027c03ec00, irq=50, hw=0) at drivers/irqchip/irq-loongarch-cpu.c:61
#1  0x900000000028ae18 in irq_domain_associate (domain=0x900000027c03ec00, virq=50, hwirq=0) at kernel/irq/irqdomain.c:536
#2  0x900000000028afd8 in irq_domain_associate_many (domain=0x900000027c03ec00, irq_base=<optimized out>, hwirq_base=<optimized out>, count=<optimized out>) at kernel/irq/irqdomain.c:579
#3  0x90000000014e2d54 in __loongarch_cpu_irq_init (of_node=<optimized out>) at drivers/irqchip/irq-loongarch-cpu.c:97
#4  0x90000000014e2db0 in loongarch_cpu_irq_init () at drivers/irqchip/irq-loongarch-cpu.c:102
#5  0x9000000000f6e3b4 in irqchip_init_default () at arch/loongarch/la64/irq.c:279
#6  0x90000000014a8e78 in setup_IRQ () at arch/loongarch/la64/irq.c:311
#7  0x90000000014a8ea4 in arch_init_irq () at arch/loongarch/la64/irq.c:360
#8  0x90000000014aa708 in init_IRQ () at arch/loongarch/kernel/irq.c:59
#9  0x90000000014a4a40 in start_kernel () at init/main.c:636
#10 0x9000000000f79004 in kernel_entry () at arch/loongarch/kernel/head.S:129
```

- [ ] 一个 irq_desc 只是持有一个 irq_data，但是 irq_data 只是关联一个 irq_domain 的，为此，我感觉实际上
每一个中断芯片的每一个引脚都会制造出来一个 irq_desc 出来。
  - 但是 irq_desc 对应的 linux irq 在整个系统中都是 unique 的

## 分析 extioi_domain_alloc 的实现
```c
#0  extioi_domain_alloc (domain=0x900000027c024800, virq=16, nr_irqs=1, arg=0x900000000134bc00) at drivers/irqchip/irq-loongarch-extioi.c:303
#1  0x90000000008a3b20 in pch_pic_alloc (domain=0x900000027c0bfa00, virq=16, nr_irqs=<optimized out>, arg=<optimized out>) at drivers/irqchip/irq-loongson-pch-pic.c:287
#2  0x900000000028bf28 in irq_domain_alloc_irqs_hierarchy (arg=<optimized out>, nr_irqs=<optimized out>, irq_base=<optimized out>, domain=<optimized out>) at kernel/irq/irqdomain.c:1270
#3  __irq_domain_alloc_irqs (domain=0x900000027c0bfa00, irq_base=16, nr_irqs=1, node=<optimized out>, arg=<optimized out>, realloc=<optimized out>, affinity=<optimized out>) at kernel/irq/irqdomain.c:1326
#4  0x900000000028c4cc in irq_domain_alloc_irqs (arg=<optimized out>, node=<optimized out>, nr_irqs=<optimized out>, domain=<optimized out>) at ./include/linux/irqdomain.h:466
#5  irq_create_fwspec_mapping (fwspec=0x900000000134bd50) at kernel/irq/irqdomain.c:810
#6  0x9000000000f6e698 in pch_lpc_domain_init () at arch/loongarch/la64/irq.c:203
#7  irqchip_init_default () at arch/loongarch/la64/irq.c:287
#8  0x90000000014a8e78 in setup_IRQ () at arch/loongarch/la64/irq.c:311
#9  0x90000000014a8ea4 in arch_init_irq () at arch/loongarch/la64/irq.c:360
#10 0x90000000014aa708 in init_IRQ () at arch/loongarch/kernel/irq.c:59
#11 0x90000000014a4a40 in start_kernel () at init/main.c:636
#12 0x9000000000f79014 in kernel_entry () at arch/loongarch/kernel/head.S:129
```
- [ ] 这个 virq 在 /proc/interrupts 中间没有显示，而且
- [ ] 当然 /proc/interrupts 中间缺失了 20 和 21

## [ ] 我们发现了 pch_pic_alloc

- [ ] 初始化的过程
```c
#0  pch_pic_init (irq_handle=0x900000027c011540, addr=268435456, size=1024, model=2, gsi_base=64) at ./include/linux/slab.h:709
#1  0x9000000000f6e67c in pch_pic_domains_init () at arch/loongarch/la64/irq.c:153
#2  irqchip_init_default () at arch/loongarch/la64/irq.c:286
#3  0x90000000014a8e78 in setup_IRQ () at arch/loongarch/la64/irq.c:311
#4  0x90000000014a8ea4 in arch_init_irq () at arch/loongarch/la64/irq.c:360
#5  0x90000000014aa708 in init_IRQ () at arch/loongarch/kernel/irq.c:59
#6  0x90000000014a4a40 in start_kernel () at init/main.c:636
#7  0x9000000000f79014 in kernel_entry () at arch/loongarch/kernel/head.S:129
Backtrace stopped: frame did not save the PC
```

- [ ] 是用来干什么的
- [ ] 是中断级联的一部分吗?
- [x] 确认一下 irq route model 是 PCH_IRQ_ROUTE_EXT_SOC 的

实际上，如果进行一些修改，那么 /proc/interrupts 中间的部分内容修改了。
```diff
diff --git a/drivers/irqchip/irq-loongson-pch-pic.c b/drivers/irqchip/irq-loongson-pch-pic.c
index bf8b7f1e812a..e0d640b3404d 100644
--- a/drivers/irqchip/irq-loongson-pch-pic.c
+++ b/drivers/irqchip/irq-loongson-pch-pic.c
@@ -387,7 +387,7 @@ int pch_pic_init(struct fwnode_handle *irq_handle,
    struct irq_domain *parent_domain;
    if (priv->model == PCH_IRQ_ROUTE_EXT) {
      parent = eiointc_get_fwnode(nr_pch_pic);
-     pch_pic_irq_chip.name = "PCH-PIC-EXT";
+     pch_pic_irq_chip.name = "ri ni ma tui qian";
    } else
      parent = htvec_get_fwnode();
    parent_domain = irq_find_matching_fwnode(
```

```c
loongson@loongson-pc:~$ cat /proc/interrupts
            CPU0
  17:          0  ri ni ma tui qian    4  acpi
  18:          0  ri ni ma tui qian    3  ls2x-rtc alarm
  19:       1224  ri ni ma tui qian    2  ttyS0
  22:          0  PCH-MSI-EXT   32  virtio1-config
  23:       1332  PCH-MSI-EXT   33  virtio1-req.0
  61:       1187  COREINTC   11  timer
IPI0:          0       Rescheduling interrupts
IPI1:          0       Call Function interrupts
 ERR:          0
```
- [ ] 应该在主板手册这个控制器的说明
- [x] 为什么感觉 pch_handle_irq 完全没有被调用啊
  - 因为只有在 PCH_IRQ_ROUTE_LINE 中的模式下才会被使用

但是这个名称是怎么回事啊?
```c
      pch_pic_irq_chip.name = "ri ni ma tui qian";
```
- 是因为 pch_pic_alloc

- [x] pch_pic_alloc 总是会调用 extioi_domain_alloc 吗?
  - 是的

在 register_pch_pic 中:
```c
  pr_info("PCH_PIC[%d]: pch_pic_id %d, version %d, address 0x%lx, IRQ %d-%d\n",
    idx, pch_pic_id(idx),
    pch_pic_ver(idx), pch_pic_addr(idx),
    irq_cfg->irq_base, irq_cfg->irq_end);
```
对应的输出为:
```c
[    0.000000] PCH_PIC[0]: pch_pic_id 0, version 0, address 0x10000000, IRQ 64-95
```

这是 pch 的装换:
```c
static int pch_pic_domain_translate(struct irq_domain *d,
          struct irq_fwspec *fwspec,
          unsigned long *hwirq,
          unsigned int *type)
{
  struct pch_pic *priv = d->host_data;
  if (fwspec->param_count < 1)
    return -EINVAL;
  *hwirq = fwspec->param[0] - priv->gsi_base; // priv->gsi_base 是 64 的
  *type = IRQ_TYPE_NONE;
  return 0;
}
```
看上去，pch 只是负责将 acpi gsi 的中断号翻译一下。

## 分析所有的 irq 的分配的过程
```c
#0  __irq_alloc_descs (irq=-1, from=50, cnt=14, node=0, owner=0x0, affinity=0x0) at kernel/irq/irqdesc.c:727
#1  0x90000000014e2cd0 in __loongarch_cpu_irq_init (of_node=<optimized out>) at drivers/irqchip/irq-loongarch-cpu.c:89
#2  0x90000000014e2db0 in loongarch_cpu_irq_init () at drivers/irqchip/irq-loongarch-cpu.c:102
#3  0x9000000000f6e3b4 in irqchip_init_default () at arch/loongarch/la64/irq.c:279
#4  0x90000000014a8e78 in setup_IRQ () at arch/loongarch/la64/irq.c:311
#5  0x90000000014a8ea4 in arch_init_irq () at arch/loongarch/la64/irq.c:360
#6  0x90000000014aa708 in init_IRQ () at arch/loongarch/kernel/irq.c:59
#7  0x90000000014a4a40 in start_kernel () at init/main.c:636
#8  0x9000000000f79004 in kernel_entry () at arch/loongarch/kernel/head.S:129
Backtrace stopped: frame did not save the PC
```

```c
#0  __irq_alloc_descs (irq=-1, from=1, cnt=1, node=-1, owner=0x0, affinity=0x0) at kernel/irq/irqdesc.c:727
#1  0x900000000028bea0 in __irq_domain_alloc_irqs (domain=0x900000027c0bfa00, irq_base=-1, nr_irqs=1, node=<optimized out>, arg=<optimized out>, realloc=<optimized out>, affinity=<optimized out>) at kernel/irq/irqdomain.c:1310
#2  0x900000000028c4c0 in irq_domain_alloc_irqs (arg=<optimized out>, node=<optimized out>, nr_irqs=<optimized out>, domain=<optimized out>) at ./include/linux/irqdomain.h:466
#3  irq_create_fwspec_mapping (fwspec=0x900000000134bd50) at kernel/irq/irqdomain.c:810
#4  0x9000000000f6e698 in pch_lpc_domain_init () at arch/loongarch/la64/irq.c:203
#5  irqchip_init_default () at arch/loongarch/la64/irq.c:287
#6  0x90000000014a8e78 in setup_IRQ () at arch/loongarch/la64/irq.c:311
#7  0x90000000014a8ea4 in arch_init_irq () at arch/loongarch/la64/irq.c:360
#8  0x90000000014aa708 in init_IRQ () at arch/loongarch/kernel/irq.c:59
#9  0x90000000014a4a40 in start_kernel () at init/main.c:636
#10 0x9000000000f79004 in kernel_entry () at arch/loongarch/kernel/head.S:129
Backtrace stopped: frame did not save the PC
```

```c
#0  __irq_alloc_descs (irq=-1, from=1, cnt=1, node=-1, owner=0x0, affinity=0x0) at kernel/irq/irqdesc.c:727
#1  0x900000000028bea0 in __irq_domain_alloc_irqs (domain=0x900000027c0bfa00, irq_base=-1, nr_irqs=1, node=<optimized out>, arg=<optimized out>, realloc=<optimized out>, affinity=<optimized out>) at kernel/irq/irqdomain.c:1310
#2  0x900000000028c4c0 in irq_domain_alloc_irqs (arg=<optimized out>, node=<optimized out>, nr_irqs=<optimized out>, domain=<optimized out>) at ./include/linux/irqdomain.h:466
#3  irq_create_fwspec_mapping (fwspec=0x900000027cd33c50) at kernel/irq/irqdomain.c:810
#4  0x900000000020bc40 in acpi_register_gsi (dev=<optimized out>, gsi=1, trigger=<optimized out>, polarity=<optimized out>) at arch/loongarch/kernel/acpi.c:90
#5  0x900000000020bcb4 in acpi_gsi_to_irq (gsi=<optimized out>, irqp=0x900000027cd33cdc) at arch/loongarch/kernel/acpi.c:49
#6  0x90000000009135b0 in acpi_os_install_interrupt_handler (gsi=68, handler=0x9000000000931c64 <acpi_ev_sci_xrupt_handler>, context=0x900000027d11c180) at drivers/acpi/osl.c:573
#7  0x9000000000931ce4 in acpi_ev_install_sci_handler () at drivers/acpi/acpica/evsci.c:156
#8  0x900000000092eb2c in acpi_ev_install_xrupt_handlers () at drivers/acpi/acpica/evevent.c:94
#9  0x90000000014e92f0 in acpi_enable_subsystem (flags=<optimized out>) at drivers/acpi/acpica/utxfinit.c:184
#10 0x90000000014e7214 in acpi_bus_init () at drivers/acpi/bus.c:1157
#11 acpi_init () at drivers/acpi/bus.c:1253
#12 0x9000000000200b8c in do_one_initcall (fn=0x90000000014e7154 <acpi_init>) at init/main.c:884
#13 0x90000000014a4e8c in do_initcall_level (level=<optimized out>) at ./include/linux/init.h:131
#14 do_initcalls () at init/main.c:960
#15 do_basic_setup () at init/main.c:978
#16 kernel_init_freeable () at init/main.c:1145
#17 0x9000000000f79108 in kernel_init (unused=<optimized out>) at init/main.c:1062
#18 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
Backtrace stopped: frame did not save the PC
```

```c
#0  __irq_alloc_descs (irq=-1, from=1, cnt=1, node=-1, owner=0x0, affinity=0x0) at kernel/irq/irqdesc.c:727
#1  0x900000000028bea0 in __irq_domain_alloc_irqs (domain=0x900000027c0bfa00, irq_base=-1, nr_irqs=1, node=<optimized out>, arg=<optimized out>, realloc=<optimized out>, affinity=<optimized out>) at kernel/irq/irqdomain.c:1310
#2  0x900000000028c4c0 in irq_domain_alloc_irqs (arg=<optimized out>, node=<optimized out>, nr_irqs=<optimized out>, domain=<optimized out>) at ./include/linux/irqdomain.h:466
#3  irq_create_fwspec_mapping (fwspec=0x900000027cd339c0) at kernel/irq/irqdomain.c:810
#4  0x900000000020bc40 in acpi_register_gsi (dev=<optimized out>, gsi=1, trigger=<optimized out>, polarity=<optimized out>) at arch/loongarch/kernel/acpi.c:90
#5  0x900000000091e0d8 in acpi_dev_get_irqresource (res=0xffffffffffffffff, gsi=1, triggering=<optimized out>, polarity=<optimized out>, shareable=<optimized out>, legacy=<optimized out>) at drivers/acpi/resource.c:432
#6  0x900000000091e1b8 in acpi_dev_resource_interrupt (ares=<optimized out>, index=<optimized out>, res=<optimized out>) at drivers/acpi/resource.c:488
#7  0x900000000091e4d8 in acpi_dev_process_resource (context=<optimized out>, ares=<optimized out>) at drivers/acpi/resource.c:570
#8  acpi_dev_process_resource (ares=0x900000027d441348, context=0x900000027cd33b40) at drivers/acpi/resource.c:538
#9  0x9000000000947278 in acpi_walk_resource_buffer (buffer=<optimized out>, user_function=0x1, context=0x1) at drivers/acpi/acpica/rsxface.c:547
#10 0x9000000000947744 in acpi_walk_resources (context=<optimized out>, user_function=<optimized out>, name=<optimized out>, device_handle=<optimized out>) at drivers/acpi/acpica/rsxface.c:623
#11 acpi_walk_resources (device_handle=<optimized out>, name=<optimized out>, user_function=0x900000000091e400 <acpi_dev_process_resource>, context=0x900000027cd33b40) at drivers/acpi/acpica/rsxface.c:594
#12 0x900000000091e298 in __acpi_dev_get_resources (method=<optimized out>, preproc_data=<optimized out>, preproc=<optimized out>, list=<optimized out>, adev=<optimized out>) at drivers/acpi/resource.c:596
#13 __acpi_dev_get_resources (adev=0x900000027d297800, list=0x900000027cd33b90, preproc=0x0, preproc_data=0x0, method=0x90000000012551d0 "_CRS") at drivers/acpi/resource.c:577
#14 0x90000000009249e8 in acpi_create_platform_device (properties=<optimized out>, adev=<optimized out>) at drivers/acpi/acpi_platform.c:82
#15 acpi_create_platform_device (adev=0x900000027d297800, properties=0x0) at drivers/acpi/acpi_platform.c:64
#16 0x900000000091b348 in acpi_default_enumeration (device=<optimized out>) at drivers/acpi/scan.c:1942
#17 acpi_default_enumeration (device=0x900000027d297800) at drivers/acpi/scan.c:1935
#18 0x900000000091b560 in acpi_bus_attach (device=0x900000027d297800) at drivers/acpi/scan.c:2048
#19 0x900000000091b3fc in acpi_bus_attach (device=0x900000027d295000) at drivers/acpi/scan.c:2054
#20 0x900000000091b3fc in acpi_bus_attach (device=0x900000027d294800) at drivers/acpi/scan.c:2054
#21 0x900000000091d414 in acpi_bus_scan (handle=0xffffffffffffffff) at drivers/acpi/scan.c:2106
#22 0x90000000014e77b8 in acpi_scan_init () at drivers/acpi/scan.c:2259
#23 0x90000000014e7438 in acpi_init () at drivers/acpi/bus.c:1261
#24 0x9000000000200b8c in do_one_initcall (fn=0x90000000014e7154 <acpi_init>) at init/main.c:884
#25 0x90000000014a4e8c in do_initcall_level (level=<optimized out>) at ./include/linux/init.h:131
#26 do_initcalls () at init/main.c:960
#27 do_basic_setup () at init/main.c:978
#28 kernel_init_freeable () at init/main.c:1145
#29 0x9000000000f79108 in kernel_init (unused=<optimized out>) at init/main.c:1062
#30 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
Backtrace stopped: frame did not save the PC
```

```c
#0  __irq_alloc_descs (irq=-1, from=1, cnt=1, node=-1, owner=0x0, affinity=0x0) at kernel/irq/irqdesc.c:727
#1  0x900000000028bea0 in __irq_domain_alloc_irqs (domain=0x900000027c0bfa00, irq_base=-1, nr_irqs=1, node=<optimized out>, arg=<optimized out>, realloc=<optimized out>, affinity=<optimized out>) at kernel/irq/irqdomain.c:1310
#2  0x900000000028c4c0 in irq_domain_alloc_irqs (arg=<optimized out>, node=<optimized out>, nr_irqs=<optimized out>, domain=<optimized out>) at ./include/linux/irqdomain.h:466
#3  irq_create_fwspec_mapping (fwspec=0x900000027cd33a20) at kernel/irq/irqdomain.c:810
#4  0x900000000020bc40 in acpi_register_gsi (dev=<optimized out>, gsi=1, trigger=<optimized out>, polarity=<optimized out>) at arch/loongarch/kernel/acpi.c:90
#5  0x900000000091e0d8 in acpi_dev_get_irqresource (res=0xffffffffffffffff, gsi=1, triggering=<optimized out>, polarity=<optimized out>, shareable=<optimized out>, legacy=<optimized out>) at drivers/acpi/resource.c:432
#6  0x900000000091e1b8 in acpi_dev_resource_interrupt (ares=<optimized out>, index=<optimized out>, res=<optimized out>) at drivers/acpi/resource.c:488
#7  0x900000000095e144 in pnpacpi_allocated_resource (res=0x900000027d42c248, data=0x900000027cfc7000) at drivers/pnp/pnpacpi/rsparser.c:191
#8  0x9000000000947278 in acpi_walk_resource_buffer (buffer=<optimized out>, user_function=0x1, context=0x1) at drivers/acpi/acpica/rsxface.c:547
#9  0x9000000000947744 in acpi_walk_resources (context=<optimized out>, user_function=<optimized out>, name=<optimized out>, device_handle=<optimized out>) at drivers/acpi/acpica/rsxface.c:623
#10 acpi_walk_resources (device_handle=<optimized out>, name=<optimized out>, user_function=0x900000000095e0a4 <pnpacpi_allocated_resource>, context=0x900000027cfc7000) at drivers/acpi/acpica/rsxface.c:594
#11 0x900000000095e434 in pnpacpi_parse_allocated_resource (dev=0xffffffffffffffff) at drivers/pnp/pnpacpi/rsparser.c:289
#12 0x90000000014ea154 in pnpacpi_add_device (device=<optimized out>) at drivers/pnp/pnpacpi/core.c:271
#13 pnpacpi_add_device_handler (handle=<optimized out>, lvl=<optimized out>, context=<optimized out>, rv=<optimized out>) at drivers/pnp/pnpacpi/core.c:308
#14 0x900000000094222c in acpi_ns_get_device_callback (return_value=<optimized out>, context=<optimized out>, nesting_level=<optimized out>, obj_handle=<optimized out>) at drivers/acpi/acpica/nsxfeval.c:740
#15 acpi_ns_get_device_callback (obj_handle=0x900000027c0d0398, nesting_level=2, context=0x900000027cd33d58, return_value=0x0) at drivers/acpi/acpica/nsxfeval.c:635
#16 0x9000000000941a84 in acpi_ns_walk_namespace (type=<optimized out>, start_node=0x900000027c0d0050, max_depth=<optimized out>, flags=<optimized out>, descending_callback=<optimized out>, ascending_callback=0x0, context=0x900000027cd33d58, return_value=0x0) at drivers/acpi/acpica/nswalk.c:229
#17 0x9000000000941b98 in acpi_get_devices (HID=<optimized out>, user_function=<optimized out>, context=<optimized out>, return_value=0x0) at drivers/acpi/acpica/nsxfeval.c:805
#18 0x90000000014e9f78 in pnpacpi_init () at drivers/pnp/pnpacpi/core.c:321
#19 0x9000000000200b8c in do_one_initcall (fn=0x90000000014e9f10 <pnpacpi_init>) at init/main.c:884
#20 0x90000000014a4e8c in do_initcall_level (level=<optimized out>) at ./include/linux/init.h:131
#21 do_initcalls () at init/main.c:960
#22 do_basic_setup () at init/main.c:978
#23 kernel_init_freeable () at init/main.c:1145
#24 0x9000000000f79108 in kernel_init (unused=<optimized out>) at init/main.c:1062
#25 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
Backtrace stopped: frame did not save the PC
```

```c
#0  __irq_alloc_descs (irq=-1, from=1, cnt=1, node=-1, owner=0x0, affinity=0x0) at kernel/irq/irqdesc.c:727
#1  0x900000000028bea0 in __irq_domain_alloc_irqs (domain=0x900000027c0bfa00, irq_base=-1, nr_irqs=1, node=<optimized out>, arg=<optimized out>, realloc=<optimized out>, affinity=<optimized out>) at kernel/irq/irqdomain.c:1310
#2  0x900000000028c4c0 in irq_domain_alloc_irqs (arg=<optimized out>, node=<optimized out>, nr_irqs=<optimized out>, domain=<optimized out>) at ./include/linux/irqdomain.h:466
#3  irq_create_fwspec_mapping (fwspec=0x900000027d9dbc40) at kernel/irq/irqdomain.c:810
#4  0x900000000020bc40 in acpi_register_gsi (dev=<optimized out>, gsi=1, trigger=<optimized out>, polarity=<optimized out>) at arch/loongarch/kernel/acpi.c:90
#5  0x900000000092464c in acpi_pci_irq_enable (dev=0x900000027d523000) at drivers/acpi/pci_irq.c:478
#6  0x90000000008bd710 in do_pci_enable_device (dev=0x900000027d523000, bars=19) at drivers/pci/pci.c:1532
#7  0x90000000008bfa1c in pci_enable_device_flags (dev=0xffffffffffffffff, flags=768) at drivers/pci/pci.c:1621
#8  0x90000000008bfa90 in pci_enable_device (dev=<optimized out>) at drivers/pci/pci.c:1668
#9  0x90000000009630fc in virtio_pci_probe (pci_dev=0x900000027d523000, id=<optimized out>) at drivers/virtio/virtio_pci_common.c:534
#10 0x90000000008c28c0 in local_pci_probe (_ddi=0x900000027cd33c58) at drivers/pci/pci-driver.c:306
#11 0x9000000000235030 in work_for_cpu_fn (work=0x900000027cd33c08) at kernel/workqueue.c:4908
#12 0x9000000000238ce0 in process_one_work (worker=0x900000027d438480, work=0x900000027cd33c08) at kernel/workqueue.c:2152
#13 0x9000000000239114 in worker_thread (__worker=0x900000027d438480) at kernel/workqueue.c:2295
#14 0x900000000023fc20 in kthread (_create=0x900000027d435200) at kernel/kthread.c:259
#15 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
Backtrace stopped: frame did not save the PC
```

```c
#0  __irq_alloc_descs (irq=-1, from=1, cnt=1, node=0, owner=0x0, affinity=0x900000027d06c8c0) at kernel/irq/irqdesc.c:727
#1  0x900000000028bea0 in __irq_domain_alloc_irqs (domain=0x900000027c08c600, irq_base=-1, nr_irqs=1, node=<optimized out>, arg=<optimized out>, realloc=<optimized out>, affinity=<optimized out>) at kernel/irq/irqdomain.c:1310
#2  0x900000000028e3f0 in msi_domain_alloc_irqs (domain=0x900000027c08c600, dev=0x900000027d5260a8, nvec=<optimized out>) at ./include/linux/device.h:1075
#3  0x90000000008e1840 in msix_capability_init (affd=<optimized out>, nvec=<optimized out>, entries=<optimized out>, dev=<optimized out>) at drivers/pci/msi.c:759
#4  __pci_enable_msix (affd=<optimized out>, nvec=<optimized out>, entries=<optimized out>, dev=<optimized out>) at drivers/pci/msi.c:967
#5  __pci_enable_msix_range (affd=<optimized out>, maxvec=<optimized out>, minvec=<optimized out>, entries=<optimized out>, dev=<optimized out>) at drivers/pci/msi.c:1100
#6  __pci_enable_msix_range (dev=0x900000027d526000, entries=0x0, minvec=2, maxvec=2, affd=0x900000027cd33b98) at drivers/pci/msi.c:1081
#7  0x90000000008e2298 in pci_alloc_irq_vectors_affinity (dev=0x900000027d526000, min_vecs=2, max_vecs=2, flags=12, affd=0x900000027cd33b98) at drivers/pci/msi.c:1170
#8  0x900000000096363c in vp_request_msix_vectors (desc=<optimized out>, per_vq_vectors=<optimized out>, nvectors=<optimized out>, vdev=<optimized out>) at drivers/virtio/virtio_pci_common.c:136
#9  vp_find_vqs_msix (vdev=0x900000027cfe2800, nvqs=1, vqs=<optimized out>, callbacks=0x900000027d06c780, names=0x900000027d06c740, per_vq_vectors=true, ctx=0x0, desc=0x900000027cd33b98) at drivers/virtio/virtio_pci_common.c:307
#10 0x9000000000963a00 in vp_find_vqs (vdev=0x900000027cfe2800, nvqs=1, vqs=0x900000027d06c7c0, callbacks=0x900000027d06c780, names=0x900000027d06c740, ctx=0x0, desc=0x900000027cd33b98) at drivers/virtio/virtio_pci_common.c:403
#11 0x90000000009624d4 in vp_modern_find_vqs (vdev=0x900000027cfe2800, nvqs=<optimized out>, vqs=<optimized out>, callbacks=<optimized out>, names=<optimized out>, ctx=<optimized out>, desc=<optimized out>) at drivers/virtio/virtio_pci_modern.c:413
#12 0x9000000000a51d80 in virtio_find_vqs (desc=<optimized out>, names=<optimized out>, callbacks=<optimized out>, vqs=<optimized out>, nvqs=<optimized out>, vdev=<optimized out>) at ./include/linux/virtio_config.h:192
#13 init_vq (vblk=0x900000027cfe9800) at drivers/block/virtio_blk.c:542
#14 0x9000000000a52e34 in virtblk_probe (vdev=0xffffffffffffffff) at drivers/block/virtio_blk.c:774
#15 0x900000000095f778 in virtio_dev_probe (_d=0x900000027cfe2810) at drivers/virtio/virtio.c:245
#16 0x9000000000a294c0 in really_probe (dev=0x900000027cfe2810, drv=0x90000000014359c0 <virtio_blk>) at drivers/base/dd.c:506
#17 0x9000000000a296f8 in driver_probe_device (drv=0x90000000014359c0 <virtio_blk>, dev=0x900000027cfe2810) at drivers/base/dd.c:667
#18 0x9000000000a298a0 in __driver_attach (data=<optimized out>, dev=<optimized out>) at drivers/base/dd.c:903
#19 __driver_attach (dev=0x900000027cfe2810, data=0x90000000014359c0 <virtio_blk>) at drivers/base/dd.c:872
#20 0x9000000000a271f4 in bus_for_each_dev (bus=<optimized out>, start=<optimized out>, data=0x1, fn=0x0) at drivers/base/bus.c:279
#21 0x9000000000a28d04 in driver_attach (drv=<optimized out>) at drivers/base/dd.c:922
#22 0x9000000000a28770 in bus_add_driver (drv=0x90000000014359c0 <virtio_blk>) at drivers/base/bus.c:672
#23 0x9000000000a2a4e4 in driver_register (drv=0x90000000014359c0 <virtio_blk>) at drivers/base/driver.c:170
#24 0x900000000095f154 in register_virtio_driver (driver=<optimized out>) at drivers/virtio/virtio.c:296
#25 0x90000000014ee768 in init () at drivers/block/virtio_blk.c:1019
#26 0x9000000000200b8c in do_one_initcall (fn=0x90000000014ee6f0 <init>) at init/main.c:884
#27 0x90000000014a4e8c in do_initcall_level (level=<optimized out>) at ./include/linux/init.h:131
#28 do_initcalls () at init/main.c:960
#29 do_basic_setup () at init/main.c:978
#30 kernel_init_freeable () at init/main.c:1145
#31 0x9000000000f79108 in kernel_init (unused=<optimized out>) at init/main.c:1062
#32 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
Backtrace stopped: frame did not save the PC
```

## irq_domain_set_mapping 的设计

irq domain 一个引脚关联的 linux irq

```c
static void irq_domain_set_mapping(struct irq_domain *domain,
           irq_hw_number_t hwirq,
           struct irq_data *irq_data)
{
  if (hwirq < domain->revmap_size) {
    domain->linear_revmap[hwirq] = irq_data->irq;
  } else {
    mutex_lock(&domain->revmap_tree_mutex);
    radix_tree_insert(&domain->revmap_tree, hwirq, irq_data);
    mutex_unlock(&domain->revmap_tree_mutex);
  }
}
```

```c
#0  huxueshi () at kernel/irq/irqdesc.c:104
#1  0x9000000000281664 in desc_set_defaults (owner=<optimized out>, affinity=<optimized out>, node=<optimized out>, desc=<optimized out>, irq=<optimized out>) at kernel/irq/irqdesc.c:112
#2  alloc_desc (irq=<optimized out>, node=-1, flags=0, affinity=0x0, owner=0x0) at kernel/irq/irqdesc.c:415
#3  0x9000000000f79748 in alloc_descs (owner=<optimized out>, affinity=<optimized out>, node=-1, cnt=<optimized out>, start=<optimized out>) at kernel/irq/irqdesc.c:497
#4  __irq_alloc_descs (irq=<optimized out>, from=<optimized out>, cnt=1, node=-1, owner=0x0, affinity=0x0) at kernel/irq/irqdesc.c:763
#5  0x900000000028beac in __irq_domain_alloc_irqs (domain=0x900000027c0bfa00, irq_base=-1, nr_irqs=1, node=<optimized out>, arg=<optimized out>, realloc=<optimized out>, affinity=<optimized out>) at kernel/irq/irqdomain.c:1310
#6  0x900000000028c4cc in irq_domain_alloc_irqs (arg=<optimized out>, node=<optimized out>, nr_irqs=<optimized out>, domain=<optimized out>) at ./include/linux/irqdomain.h:466
#7  irq_create_fwspec_mapping (fwspec=0x900000027cd33a20) at kernel/irq/irqdomain.c:810
#8  0x900000000020bc40 in acpi_register_gsi (dev=<optimized out>, gsi=18787048, trigger=<optimized out>, polarity=<optimized out>) at arch/loongarch/kernel/acpi.c:90
#9  0x900000000091e0e4 in acpi_dev_get_irqresource (res=0x900000027d8cdb58, gsi=18787048, triggering=<optimized out>, polarity=<optimized out>, shareable=<optimized out>, legacy=<optimized out>) at drivers/acpi/resource.c:432
#10 0x900000000091e1c4 in acpi_dev_resource_interrupt (ares=<optimized out>, index=<optimized out>, res=<optimized out>) at drivers/acpi/resource.c:488
#11 0x900000000095e150 in pnpacpi_allocated_resource (res=0x900000027d45f9c8, data=0x900000027d537000) at drivers/pnp/pnpacpi/rsparser.c:191
#12 0x9000000000947284 in acpi_walk_resource_buffer (buffer=<optimized out>, user_function=0x90000000011eaae8, context=0x9000000001586f08 <irq_kobj_base>) at drivers/acpi/acpica/rsxface.c:547
#13 0x9000000000947750 in acpi_walk_resources (context=<optimized out>, user_function=<optimized out>, name=<optimized out>, device_handle=<optimized out>) at drivers/acpi/acpica/rsxface.c:623
#14 acpi_walk_resources (device_handle=<optimized out>, name=<optimized out>, user_function=0x900000000095e0b0 <pnpacpi_allocated_resource>, context=0x900000027d537000) at drivers/acpi/acpica/rsxface.c:594
#15 0x900000000095e440 in pnpacpi_parse_allocated_resource (dev=0x900000027d8cdb58) at drivers/pnp/pnpacpi/rsparser.c:289
#16 0x90000000014ea154 in pnpacpi_add_device (device=<optimized out>) at drivers/pnp/pnpacpi/core.c:271
#17 pnpacpi_add_device_handler (handle=<optimized out>, lvl=<optimized out>, context=<optimized out>, rv=<optimized out>) at drivers/pnp/pnpacpi/core.c:308
#18 0x9000000000942238 in acpi_ns_get_device_callback (return_value=<optimized out>, context=<optimized out>, nesting_level=<optimized out>, obj_handle=<optimized out>) at drivers/acpi/acpica/nsxfeval.c:740
#19 acpi_ns_get_device_callback (obj_handle=0x900000027c0d0398, nesting_level=2, context=0x900000027cd33d58, return_value=0x0) at drivers/acpi/acpica/nsxfeval.c:635
#20 0x9000000000941a90 in acpi_ns_walk_namespace (type=<optimized out>, start_node=0x900000027c0d0050, max_depth=<optimized out>, flags=<optimized out>, descending_callback=<optimized out>, ascending_callback=0x0, context=0x900000027cd33d58, return_value=0x0) at drivers/acpi/acpica/nswalk.c:229
#21 0x9000000000941ba4 in acpi_get_devices (HID=<optimized out>, user_function=<optimized out>, context=<optimized out>, return_value=0x0) at drivers/acpi/acpica/nsxfeval.c:805
#22 0x90000000014e9f78 in pnpacpi_init () at drivers/pnp/pnpacpi/core.c:321
#23 0x9000000000200b8c in do_one_initcall (fn=0x90000000014e9f10 <pnpacpi_init>) at init/main.c:884
#24 0x90000000014a4e8c in do_initcall_level (level=<optimized out>) at ./include/linux/init.h:131
#25 do_initcalls () at init/main.c:960
#26 do_basic_setup () at init/main.c:978
#27 kernel_init_freeable () at init/main.c:1145
#28 0x9000000000f79118 in kernel_init (unused=<optimized out>) at init/main.c:1062
#29 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
Backtrace stopped: frame did not save the PC
```

## 分析一下 linux irq 的分配的过程
- pch_pic_alloc
  - 构建 fwspec 主要是用于承载 acpi gsi
  - [ ] irq_domain_alloc_irqs_parent
  - [ ] irq_domain_set_info : 好多类似的函数啊

```c
static int pch_pic_alloc(struct irq_domain *domain, unsigned int virq,
        unsigned int nr_irqs, void *arg)
{
  int err;
  unsigned int type;
  struct irq_fwspec fwspec;
  struct pch_pic *priv = domain->host_data;
  unsigned long hwirq = 0;

  pch_pic_domain_translate(domain, arg, &hwirq, &type);

  fwspec.fwnode = domain->parent->fwnode;
  fwspec.param_count = 1;
  fwspec.param[0] = hwirq;
  err = irq_domain_alloc_irqs_parent(domain, virq, 1, &fwspec);
  if (err)
    return err;
  irq_domain_set_info(domain, virq, hwirq,
        &pch_pic_irq_chip, priv,
        handle_level_irq, NULL, NULL);
  irq_set_noprobe(virq);

  return 0;
}
```

一个经典的 pch_pic_alloc 的调用路径:
```c
#0  pch_pic_alloc (domain=0x900000027c0bfa00, virq=17, nr_irqs=1, arg=0x900000027cd33c50) at drivers/irqchip/irq-loongson-pch-pic.c:279
#1  0x900000000028bf28 in irq_domain_alloc_irqs_hierarchy (arg=<optimized out>, nr_irqs=<optimized out>, irq_base=<optimized out>, domain=<optimized out>) at kernel/irq/irqdomain.c:1270
#2  __irq_domain_alloc_irqs (domain=0x900000027c0bfa00, irq_base=17, nr_irqs=1, node=<optimized out>, arg=<optimized out>, realloc=<optimized out>, affinity=<optimizedout>) at kernel/irq/irqdomain.c:1326
#3  0x900000000028c4cc in irq_domain_alloc_irqs (arg=<optimized out>, node=<optimized out>, nr_irqs=<optimized out>, domain=<optimized out>) at ./include/linux/irqdomain.h:466
#4  irq_create_fwspec_mapping (fwspec=0x900000027cd33c50) at kernel/irq/irqdomain.c:810
#5  0x900000000020bc40 in acpi_register_gsi (dev=<optimized out>, gsi=17, trigger=<optimized out>, polarity=<optimized out>) at arch/loongarch/kernel/acpi.c:90
#6  0x900000000020bcb4 in acpi_gsi_to_irq (gsi=<optimized out>, irqp=0x900000027cd33cdc) at arch/loongarch/kernel/acpi.c:49
#7  0x90000000009135bc in acpi_os_install_interrupt_handler (gsi=68, handler=0x9000000000931c70 <acpi_ev_sci_xrupt_handler>, context=0x900000027d11c180) at drivers/acpi/osl.c:573
#8  0x9000000000931cf0 in acpi_ev_install_sci_handler () at drivers/acpi/acpica/evsci.c:156
#9  0x900000000092eb38 in acpi_ev_install_xrupt_handlers () at drivers/acpi/acpica/evevent.c:94
#10 0x90000000014e92f0 in acpi_enable_subsystem (flags=<optimized out>) at drivers/acpi/acpica/utxfinit.c:184
#11 0x90000000014e7214 in acpi_bus_init () at drivers/acpi/bus.c:1157
#12 acpi_init () at drivers/acpi/bus.c:1253
#13 0x9000000000200b8c in do_one_initcall (fn=0x90000000014e7154 <acpi_init>) at init/main.c:884
#14 0x90000000014a4e8c in do_initcall_level (level=<optimized out>) at ./include/linux/init.h:131
#15 do_initcalls () at init/main.c:960
#16 do_basic_setup () at init/main.c:978
#17 kernel_init_freeable () at init/main.c:1145
#18 0x9000000000f79118 in kernel_init (unused=<optimized out>) at init/main.c:1062
#19 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
Backtrace stopped: frame did not save the PC
```

从而实现的操作:
```c
int acpi_register_gsi(struct device *dev, u32 gsi, int trigger, int polarity)

      struct irq_fwspec fwspec;

      fwspec.fwnode = handle;
      fwspec.param[0] = gsi; // 比如 gsi = 68
      fwspec.param_count = 1;
      return irq_create_fwspec_mapping(&fwspec);
```

使用 pch_pic 的原因是因为 hwhand

pch_pic 的初始化的位置在 pch_pic_domains_init
```c
    irq_handle = irq_domain_alloc_fwnode((void *)address);
```
而在 acpi_register_gsi 中存在的这个东西
```c
    handle = pch_pic_get_fwnode(id);
```

## 既然存在那么多的 irq desc，为什么只有这个 irq desc 被打印出来了
- [ ] 我猜测是因为不是所有的 desc 都注册了 action 的

```c
  if ((!desc->action || irq_desc_is_chained(desc)) && !any_count)
    goto outsparse;
```
比如在 loongarch_cpu_intc_map 中调用 irq_set_chip_and_handler 中就是仅仅注册了 handlr_irq，那个只是一个 generic 的部分。

但是，request_irq 之类的操作，为什么最后 action 最后选择上那几个 irq 的。

## 忽然意识到，其实 request irq 操作和 irq_set_chip_and_handler 的操作分别两个操作
- [x] 那么 request_irq 的时候，怎么和对应的 irq desc 匹配起来
  - [x] 找到 ttyS0 的注册位置

```c
#0  request_threaded_irq (irq=61, handler=0x9000000000208430 <constant_timer_interrupt>, thread_fn=0x0, irqflags=83456, devname=0x90000000011ee470 "timer", dev_id=0x0)
at kernel/irq/manage.c:1884
#1  0x9000000000208764 in request_irq (dev=<optimized out>, name=<optimized out>, flags=<optimized out>, handler=<optimized out>, irq=<optimized out>) at ./include/linu
x/interrupt.h:147
#2  constant_clockevent_init () at arch/loongarch/kernel/time.c:175
#3  constant_clockevent_init () at arch/loongarch/kernel/time.c:140
#4  0x90000000014aba54 in time_init () at arch/loongarch/kernel/time.c:233
#5  0x90000000014a4a58 in start_kernel () at init/main.c:643
#6  0x9000000000f79014 in kernel_entry () at arch/loongarch/kernel/head.S:129
Backtrace stopped: frame did not save the PC
```
- [x] 比如这个地方注册，他是怎么知道自己的 irq = 61 的啊?
  - 好吧，我 tm 的人傻了，这是写死的, 这只是一个例外而已，问题不大



```c
#0  request_threaded_irq (irq=19, handler=0x900000000098dd20 <serial8250_interrupt>, thread_fn=0x0, irqflags=0, devname=0x900000027cff6780 "ttyS0", dev_id=0x900000027cf
1de80) at kernel/irq/manage.c:1884
#1  0x900000000098ecf4 in request_irq (dev=<optimized out>, name=<optimized out>, flags=<optimized out>, handler=<optimized out>, irq=<optimized out>) at ./include/linu
x/interrupt.h:147
#2  serial_link_irq_chain (up=<optimized out>) at drivers/tty/serial/8250/8250_core.c:215
#3  univ8250_setup_irq (up=0x90000000015f68c8 <serial8250_ports>) at drivers/tty/serial/8250/8250_core.c:340
#4  0x90000000009925f8 in serial8250_do_startup (port=0x90000000015f68c8 <serial8250_ports>) at drivers/tty/serial/8250/8250_port.c:2347
#5  0x900000000098bd98 in uart_port_startup (tty=0x900000027ce14400, state=0x900000027ce5a000, init_hw=0) at drivers/tty/serial/serial_core.c:218
#6  0x900000000098cac8 in uart_startup (init_hw=<optimized out>, state=<optimized out>, tty=<optimized out>) at drivers/tty/serial/serial_core.c:1743
#7  uart_startup (init_hw=<optimized out>, state=<optimized out>, tty=<optimized out>) at drivers/tty/serial/serial_core.c:248
#8  uart_port_activate (tty=<optimized out>, port=<optimized out>) at drivers/tty/serial/serial_core.c:1758
#9  uart_port_activate (port=0x900000027ce5a000, tty=0x900000027ce14400) at drivers/tty/serial/serial_core.c:1743
#10 0x9000000000971898 in tty_port_open (port=0x900000027ce5a000, tty=0x900000027ce14400, filp=0x0) at drivers/tty/tty_port.c:696
#11 0x900000000098a12c in uart_open (tty=0x900000027ce14400, filp=<optimized out>) at drivers/tty/serial/serial_core.c:1734
#12 0x9000000000969660 in tty_open (inode=0x900000027a934720, filp=0x900000027da6e400) at drivers/tty/tty_io.c:2046
#13 0x90000000003fd310 in chrdev_open (inode=0x13, filp=0x900000027da6e400) at fs/char_dev.c:423
#14 0x90000000003f3fd8 in do_dentry_open (f=0x900000027da6e400, inode=0x900000027a934720, open=0x90000000003fd25c <chrdev_open>) at fs/open.c:796
#15 0x90000000003f56b4 in vfs_open (path=<optimized out>, file=<optimized out>) at ./include/linux/dcache.h:545
#16 0x9000000000408d94 in do_last (op=<optimized out>, file=<optimized out>, nd=<optimized out>) at fs/namei.c:3421
#17 path_openat (nd=0x900000027cd33cb8, op=0x900000027cd33db8, flags=<optimized out>) at fs/namei.c:3537
#18 0x900000000040b360 in do_filp_open (dfd=<optimized out>, pathname=<optimized out>, op=0x900000027cd33db8) at fs/namei.c:3567
#19 0x90000000003f5a3c in do_sys_open (dfd=-100, filename=<optimized out>, flags=<optimized out>, mode=<optimized out>) at fs/open.c:1085
#20 0x90000000014a4ec4 in ksys_open (mode=<optimized out>, flags=<optimized out>, filename=<optimized out>) at ./include/linux/syscalls.h:1280
#21 kernel_init_freeable () at init/main.c:1148
#22 0x9000000000f79118 in kernel_init (unused=<optimized out>) at init/main.c:1062
#23 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
Backtrace stopped: frame did not save the PC
```
很容易跟踪到其 irq 为 uart.port.irq

而在 serial_pnp_probe 中正好是初始化 irq 为 19 的:
```c
#0  serial_pnp_probe (dev=0x900000027d32f000, dev_id=0x9000000001068570 <pnp_dev_table+1248>) at drivers/tty/serial/8250/8250_pnp.c:440
#1  0x9000000000959b54 in pnp_device_probe (dev=0x900000027d32f000) at drivers/pnp/driver.c:109
#2  0x9000000000a294cc in really_probe (dev=0x900000027d32f000, drv=0x9000000001428160 <serial_pnp_driver+64>) at drivers/base/dd.c:506
#3  0x9000000000a29704 in driver_probe_device (drv=0x9000000001428160 <serial_pnp_driver+64>, dev=0x900000027d32f000) at drivers/base/dd.c:667
#4  0x9000000000a298ac in __driver_attach (data=<optimized out>, dev=<optimized out>) at drivers/base/dd.c:903
#5  __driver_attach (dev=0x900000027d32f000, data=0x9000000001428160 <serial_pnp_driver+64>) at drivers/base/dd.c:872
#6  0x9000000000a27200 in bus_for_each_dev (bus=<optimized out>, start=<optimized out>, data=0x900000027d8a85c3, fn=0xffc0000000000000) at drivers/base/bus.c:279
#7  0x9000000000a28d10 in driver_attach (drv=<optimized out>) at drivers/base/dd.c:922
#8  0x9000000000a2877c in bus_add_driver (drv=0x9000000001428160 <serial_pnp_driver+64>) at drivers/base/bus.c:672
#9  0x9000000000a2a4f0 in driver_register (drv=0x9000000001428160 <serial_pnp_driver+64>) at drivers/base/driver.c:170
#10 0x9000000000959934 in pnp_register_driver (drv=<optimized out>) at drivers/pnp/driver.c:272
#11 0x900000000098f14c in serial8250_pnp_init () at drivers/tty/serial/8250/8250_pnp.c:539
#12 0x90000000014ebf0c in serial8250_init () at drivers/tty/serial/8250/8250_core.c:1158
#13 0x9000000000200b8c in do_one_initcall (fn=0x90000000014ebe70 <serial8250_init>) at init/main.c:884
#14 0x90000000014a4e8c in do_initcall_level (level=<optimized out>) at ./include/linux/init.h:131
#15 do_initcalls () at init/main.c:960
#16 do_basic_setup () at init/main.c:978
#17 kernel_init_freeable () at init/main.c:1145
#18 0x9000000000f79118 in kernel_init (unused=<optimized out>) at init/main.c:1062
#19 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
Backtrace stopped: frame did not save the PC
```

## 找到 ttyS0 分配的所有的 irq ，以及他们之间实现映射的方法
- irq=3
- irq=53
- irq=2
- irq=19

### 在 cpu intc 中
`__loongarch_cpu_irq_init` 你可以分配，然后调用 irq_domain_associate_many, 最后会调用到 irq_domain_set_mapping

```c
    domain->linear_revmap[hwirq] = irq_data->irq;
```
记录的是，hwirq 到 linux irq 的映射，当拿到了 linux irq 之后，就可以找到对应 irq desc 了，然后找到这个函数。

在这里，实现了从 (hwirq, irq) => (1, 51) 的映射

cpu intc 将这个注册为 extioi_irq_dispatch
```c
static inline void generic_handle_irq_desc(struct irq_desc *desc)
{
  desc->handle_irq(desc);
}
```
居然是在 eiointc_domain_init => extioi_vec_init 中初始化的
- irq_set_chained_handler_and_data : 因为知道 cpu intc 的 desc 和 irq 的对应
  - `__irq_do_set_handler`
    - 在这里注册其 desc::handle_irq 也就是 extioi_irq_dispatch

## 在 extioi 中的映射是如何构建的
