# 11

- [ ] 扩展 IO 中断到底是放到什么位置的，和 ht 的关系是什么 ?
将 HT 上的中断直接分发给各个处理器核。 HT 总线?
- [ ] MSI 中断如何处理的?
- [ ] 岂不是说，legacy 和 extioi 的区别在于 ht 的工作模式上 ?

- [ ] 是不是说，pic 定位就是 ioapic，而 ht 定位是 lapic?
  - [ ] 所以，我们可以看到 msi 和 pch_pic 是一个级别的?

- [ ] 所以，申请总是 pic 开始的，当这个东西，而一个设备的中断具体接入到什么位置
  - [ ] 是需要靠 acpi 知道的

- [ ] legacy 设备到底是怎么回事?
- [ ] 验证一下内核中的 hwirq 分配和 asl 的描述是否一致

- [ ] 彻底搞清楚中断级联的层次

- [ ] acpi_walk_resources 是如何使用的 ?


1. 0x1fe00000 和 0x3ff00000 进行访问, 这是 legacy 的访问方式
2. 0x3ff00000 使用*路由配置寄存器*的

将 ht 总线上的 256 位中断直接转发到各个处理器核

应该来说，extioi_irq_dispatch 就是最上层的中断控制器了。

使用了扩展 io 中断之后，进行中断处理，HT 控制器对于软件是透明的
内核直接到 LOONGARCH_IOCSR_EXTIOI_ISR_BASE 中间读取。

# loongson.h
在 /home/maritns3/core/ld/kernel/arch/loongarch/include/asm/mach-la64/loongson.h 的宏的定义啊

1. `#define LOONGSON_LIO_BASE	0x18000000` ? 手册描述 ?
2. `#define LOONGSON_BOOT_BASE	0x1c000000` ?
3. `#define LOONGSON_REG_BASE	0x1fe00000`

从 `extioi_irq_dispatch` 中的扫描地址空间，表 11-10

11.2.2 中间可以解释, 在 `extioi_init` 中为什么是这种的

## Wait the machine
- [ ] pch_pic 是不是其实根本没有任何作用 ?

其实并不是，应该是 pch_handle_irq 从来没有被调用过，但是
pch_pic_alloc 的总是会调用 extioi_domain_init 中。

register_pch_pic 中初始化了 pch 的中断控制器。

14.5.7 中断控制器

14.3.1 中可以可以 pch_pic_ack_irq 的行为
