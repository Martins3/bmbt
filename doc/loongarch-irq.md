# Loongarch irq

- [ ] virq 的分配规则
  - 全部都是记录在 allocated_irqs 中间了

- `__irq_set_handler` 就是设置的 virq 对应的 handler


## 连续分配

这两个循环: (hwirq) -> (virq)
- (0 - 13) -> (50 - 63) : `__loongarch_cpu_irq_init`
- (0 - 15) -> (0 - 15) : `pch_lpc_init`
- 16 : 是为了将 pch_lpc 控制芯片挂载到 pch_ipc 上

是的，`__loongarch_cpu_irq_init` 其中的 52 和 53 就是
后面超级重要的 ip2 ip3 入口了, 这种初始化的方法很僵硬，
这些入口都会被后面的代码覆盖。

参考: chen P89, 可以验证，lpc 占据中断号 (0 - 15), 
lpc 接入到 pch_pic 的 16 上，

两者对应的调用路径图:

一种情况是因为 `irq_domain_set_mapping` => irq_domain_associate_many
```c
/*
[    0.000000] Call Trace:
[    0.000000] [<900000000020864c>] show_stack+0x2c/0x100
[    0.000000] [<9000000000ec3968>] dump_stack+0x90/0xc0
[    0.000000] [<900000000029f8e0>] irq_domain_set_mapping+0x90/0xb8
[    0.000000] [<900000000029f9ac>] irq_domain_associate+0xa4/0x1d8
[    0.000000] [<90000000002a0eec>] irq_domain_associate_many+0x9c/0xc8
[    0.000000] [<900000000029fc34>] irq_domain_add_legacy+0x7c/0x80
[    0.000000] [<90000000012f3ce4>] __loongarch_cpu_irq_init+0x94/0xc8
[    0.000000] [<90000000012d8344>] setup_IRQ+0x14/0x110
[    0.000000] [<90000000012d845c>] arch_init_irq+0x1c/0x98
[    0.000000] [<90000000012d9cbc>] init_IRQ+0x4c/0xd4
[    0.000000] [<90000000012d4908>] start_kernel+0x274/0x454
```

另一种情况是因为
```c
/*
[    0.000000] [<900000000020864c>] show_stack+0x2c/0x100
[    0.000000] [<9000000000ec3968>] dump_stack+0x90/0xc0
[    0.000000] [<900000000029f8e0>] irq_domain_set_mapping+0x90/0xb8
[    0.000000] [<900000000029f9ac>] irq_domain_associate+0xa4/0x1d8
[    0.000000] [<90000000002a0eec>] irq_domain_associate_many+0x9c/0xc8
[    0.000000] [<900000000029fc34>] irq_domain_add_legacy+0x7c/0x80
[    0.000000] [<900000000081e770>] pch_lpc_init+0x88/0x178
[    0.000000] [<900000000081f1f4>] pch_pic_init+0x18c/0x358
[    0.000000] [<90000000012d8424>] setup_IRQ+0xf4/0x110
[    0.000000] [<90000000012d845c>] arch_init_irq+0x1c/0x98
[    0.000000] [<90000000012d9cbc>] init_IRQ+0x4c/0xd4
[    0.000000] [<90000000012d4908>] start_kernel+0x274/0x454
```
## pch_pic 和 extioi 一一对应的关系
直通 : 那就是在 extioi 哪里找到 hwirq 之后，
就可以确定是 virq, 然后就直接找到设备了

```c
[    1.860641] pch-pic: huxueshi:pch_pic_alloc 40 14
[    1.860642] extioi: huxueshi:extioi_domain_alloc 40 14
[    1.860643] __irq_set_handler 40
[    1.860645] __irq_set_handler 40
[    1.860646] irq: irq_domain_set_mapping 14 40
[    1.860647] irq: irq_domain_set_mapping 14 40
```
