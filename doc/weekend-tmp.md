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

- [ ] acpi_walk_resources 是如何使用的 ?

1. 0x1fe00000 和 0x3ff00000 进行访问, 这是 legacy 的访问方式
2. 0x3ff00000 使用*路由配置寄存器*的

将 ht 总线上的 256 位中断直接转发到各个处理器核

应该来说，extioi_irq_dispatch 就是最上层的中断控制器了。

使用了扩展 io 中断之后，进行中断处理，HT 控制器对于软件是透明的
内核直接到 LOONGARCH_IOCSR_EXTIOI_ISR_BASE 中间读取。

14.3.1 HT 控制器会  将 PIC 中断写入到 256 个中断向量中的对应位置。

14.3.3 HT 控制器是通过外部中断转换配置寄存器来进行中断转发的。

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

## 分析 legacy 的 io controller 在被谁使用 ?
当中断到了，根据路由规则，进入到 ip2 / ip3，然后 cpu 执行，
然后在 liointc_chained_handle_irq 中检查，找到正确的 irq，然后继续向下。


- [ ] 从 ip2 dispatch 的方式是什么?

从这个函数是怎么调用到 legacy irq controller 的啊
```c
static void ip2_irqdispatch(void)
{
	do_IRQ(LOONGSON_LINTC_IRQ);
}
```
还是在这个初始化函数中间了:
```c
		irq_set_chained_handler_and_data(parent_irq[i],
				liointc_chained_handle_irq, &priv->handler[i]);
```

- setup_IRQ
  - liointc_init : LIOINTC_DEFAULT_PHYS_BASE

变量 parent_int_map 中来实现

legacy 中断控制器的入口是:
```c
#define LIOINTC_DEFAULT_PHYS_BASE	(LOONGSON_REG_BASE + 0x1400)
```

参考 11.1.1 按地址访问，表 11-2 

32 个中断源每一个都对应 8 位的路由规则。

- 表 11-4 描述每一个中断源的
  - [ ] 难道这是已经绑定好了的吗 ? 从 acpi 中间可以验证吗 ?
  - [ ] ht0-int1 是什么
  - [ ] pci 的信息在哪里

- [ ] 从 liointc_chained_handle_irq 的 irq 是怎么注册的

- [ ] 为什么有点控制器是从 legacy，有的是正常的 ?

- [ ] 从手册中分析路由规则，然后分析下面的代码，可以知道:
    - 中断是路由到 1 CPU 的 ip2 和 ip3
    - [ ] 无法理解的是，为什么会路由到 ip3 的，那个不是给 extioi 使用的
      - [ ] extioi 接下来就要读去他的寄存器了，那怎么办 ?
    - [ ] 而且使用 loongson_cpu_has_msi256 来判断
```c
	/* Generate parent INT part of map cache */
	for (i = 0; i < LIOINTC_NUM_PARENT; i++) {
		u32 pending = priv->handler[i].parent_int_map;

		while (pending) {
			int bit = __ffs(pending);

			priv->map_cache[bit] = BIT(i) << LIOINTC_SHIFT_INTx;
			pending &= ~BIT(bit);
		}
	}

	for (i = 0; i < LIOINTC_CHIP_IRQ; i++) {
		/* Generate core part of map cache */
		priv->map_cache[i] |= BIT(0);
		writeb(priv->map_cache[i], base + i);
	}
```

## 分析 extioi
- [ ] 扩展 io 中断使能是哪里打开的

- [ ] 支持 HT 控制器的中断跨片和动态分发功能，具体来说是怎么做的

- [ ] 成组路由的代码
  - ext_set_irq_affinity 中可以动态设置的

- [ ] 原理上说，extioi 和 liointc 是对称的，都存在中断路由，然后直接到达 cpu 上, 但是 extioi 下面还有 pch 啊

- [ ] pic 中断向量写入到 256 位，也就是 extioi 是需要 pic 来处理 ?

似乎，extioi 和 liointc 的实际作用是中断桥接的作用。

中断，从 32 / 256 输入，然后传递给 cpu, 其实是一个 lapic

**P88**

ls7a 芯片组使用 apic 中断控制器。
  - [ ] pch_pic 是 apic, 有证据吗?

ls7a 使用 legacy 中断的时候，通过 SYS INT0 路由，使用 MSI 时，通过 HT1 控制器路由。

ls7a 上的 ht1 中断的 IRQ0-15，IRQ64-127 来自下游的 apic 的固定输入，
固定输入之外，用于 MSI
IRQ56~63 一般保留给 CPU 内部使用

## HT
在 3a5000 中，HyperTransport 来实现外部设备链接。
**用于外设链接的时候，可由用户自由选择是否支持 IO Cache**一致性。

和 pci 耦合的部分，都是在 `kernel/arch/loongarch/pci` 下面了

- [ ] prom_init_env 中的宏看不懂啊

- [x] 什么叫做高 8 位 ht 中断控制器
  - 参考 chen book P88 的图, apic 的输出会接入到 HT1 中间

- [ ] 14.5.34 外部中断转换配置 : 这个用于实现 msi 的吗 ?

- [ ] 14.6 中描述中断控制器，当这个

## irq-loongson-pch-msi
- [ ] 注册 192 个 msi 位置从 64 开始，这是怎么规定的 ?

## pch pic
- [x] 配置空间是怎么规定的 ?

- ls7a 就是一些主板元素吧

```c
#define LS7A_PCH_REG_BASE		0x10000000UL
#define HT1LO_OFFSET		0xe0000000000UL

register_pch_pic(0, LS7A_PCH_REG_BASE, LOONGSON_PCH_IRQ_BASE);
```



```c
entries = (((unsigned long)ls7a_readq(pch_pic_addr(idx)) >> 48) & 0xff) + 1;
```

## pci 和 ht 的关系

```c
static int pci_read(struct pci_bus *bus,
		unsigned int devfn,
		int where, int size, u32 *value)
{
	return loongarch_pci_ops->read(bus,
				 devfn, where, size, value);
}

static int pci_write(struct pci_bus *bus,
		unsigned int devfn,
		int where, int size, u32 value)
{
	return loongarch_pci_ops->write(bus,
				  devfn, where, size, value);
}
```

## msi
- pch_msi_irq_chip 和 middle_irq_chip 的关系是什么?
  - 在 pch_msi_init_domains 中，msi_domain 将 middle_domain 作为 parent 传递进去

- [x] extioi 分配哪一个 irq 给 msi 的，是靠什么规则的啊
  - virq 就是顺序分配的，但是 hwirq 是靠顺序的

```c
/*
[    1.843069] [<900000000020864c>] show_stack+0x2c/0x100
[    1.843072] [<9000000000ec3948>] dump_stack+0x90/0xc0
[    1.843076] [<900000000081ce40>] pch_msi_middle_domain_alloc+0x48/0x220
[    1.843079] [<90000000002a24a8>] msi_domain_alloc+0x88/0x1a8
[    1.843082] [<90000000002a0640>] __irq_domain_alloc_irqs+0x1b0/0x2e0
[    1.843083] [<90000000002a2d48>] msi_domain_alloc_irqs+0x90/0x320
[    1.843085] [<900000000020306c>] arch_setup_msi_irqs+0x8c/0xc8
[    1.843088] [<900000000085cc2c>] __pci_enable_msi_range+0x324/0x5e8
[    1.843090] [<900000000085d040>] pci_alloc_irq_vectors_affinity+0x120/0x158
[    1.843094] [<9000000000a926d0>] ahci_init_one+0xb28/0x1020
[    1.843097] [<900000000083c8d8>] local_pci_probe+0x48/0xe0
[    1.843100] [<900000000024cff4>] work_for_cpu_fn+0x1c/0x30
[    1.843101] [<9000000000250778>] process_one_work+0x210/0x418
[    1.843103] [<9000000000250cb8>] worker_thread+0x338/0x5e0
[    1.843106] [<90000000002578fc>] kthread+0x124/0x128
[    1.843108] [<9000000000203cc8>] ret_from_kernel_thread+0xc/0x10
```

好吧，让人奇怪
[    0.000000] pch-msi: huxueshi:pch_msi_init_domains middle_domain=9000000476018400 msi_domain=9000000476030000
[    1.846323] huxueshi:setup_msidomain_irq domain=9000000476030000

通过上面的 backtrace 才知道 msi_domain 实际上是公共的标准组件，
pci_msi_create_irq_domain => msi_create_irq_domain => irq_domain_create_hierarchy

好的，这两个 irq_domain 都是有 msi_compose 的，到时候是调用哪一个的啊

看 x86 的实现: arch/x86/kernel/apic/msi.c
```c
d = pci_msi_create_irq_domain(fn, &pci_msi_domain_info, x86_vector_domain);
```
msi_domain 是直接链接到 x86_vector_domain 上的。

- [ ] 如果不创建 middle_domain，那么会产生什么问题?
  - msi 是从 64 开始的
  - 如果不创建一个中间层，那么就会直接调用 extioi_domain_alloc, 那么偏移信息展示出来

pch_msi_compose_msi_msg 的调用路径，最后发送的 msi 应该是给 extioi 了
```c
/*
[    1.840737] [<900000000020866c>] show_stack+0x2c/0x100
[    1.840739] [<9000000000ec39a8>] dump_stack+0x90/0xc0
[    1.840740] [<900000000081ce24>] pch_msi_compose_msi_msg+0x34/0x58
[    1.840743] [<900000000029c3f0>] irq_chip_compose_msi_msg+0x48/0x60
[    1.840745] [<90000000002a2760>] msi_domain_activate+0x28/0x68
[    1.840747] [<900000000029e854>] __irq_domain_activate_irq+0x5c/0xa0
[    1.840749] [<90000000002a0da4>] irq_domain_activate_irq+0x34/0x60
[    1.840751] [<90000000002a2e48>] msi_domain_alloc_irqs+0x190/0x320
[    1.840753] [<900000000020308c>] arch_setup_msi_irqs+0xac/0xe8
[    1.840754] [<900000000085cc7c>] __pci_enable_msi_range+0x324/0x5e8
[    1.840756] [<900000000085d090>] pci_alloc_irq_vectors_affinity+0x120/0x158
[    1.840758] [<9000000000a92720>] ahci_init_one+0xb28/0x1020
[    1.840760] [<900000000083c928>] local_pci_probe+0x48/0xe0
[    1.840762] [<900000000024cff4>] work_for_cpu_fn+0x1c/0x30
[    1.840764] [<9000000000250778>] process_one_work+0x210/0x418
[    1.840765] [<9000000000250cb8>] worker_thread+0x338/0x5e0
[    1.840767] [<90000000002578fc>] kthread+0x124/0x128
[    1.840769] [<9000000000203ce8>] ret_from_kernel_thread+0xc/0x10
[    1.840774] pch-msi: huxueshi:pch_msi_compose_msi_msg 64
```
- [ ] 这个设计，msi 的中断并没有直接到达 cpu, 那么 x86 的 msi 是可以直接转发到 cpu

## 为什么有的会注册到 legacy 上
```c
// parent_irq[i] = 52
irq_set_chained_handler_and_data(parent_irq[i], liointc_chained_handle_irq, &priv->handler[i]);
```
这个和 extioi 其实相同的, ip3_irqdispatch 触发 52 
也就是 `do_IRQ(LOONGSON_BRIDGE_IRQ)`
```c
/*
[    2.327702] huxueshi:liointc_chained_handle_irq 0
[    2.327703] huxueshi:liointc_chained_handle_irq 0
[    2.327704] huxueshi:liointc_chained_handle_irq 0
[    2.327705] huxueshi:liointc_chained_handle_irq 0
[    2.327706] huxueshi:liointc_chained_handle_irq 0
[    2.327707] huxueshi:liointc_chained_handle_irq 0
[    2.327708] huxueshi:liointc_chained_handle_irq 0
[    2.327708] huxueshi:liointc_chained_handle_irq 0
[    2.327709] huxueshi:liointc_chained_handle_irq 0
[    2.327710] huxueshi:liointc_chained_handle_irq 0

[    2.327711] huxueshi:liointc_chained_handle_irq 27
[    2.327712] huxueshi:liointc_chained_handle_irq 0
[    2.327712] huxueshi:liointc_chained_handle_irq 0
[    2.327713] huxueshi:liointc_chained_handle_irq 0
[    2.327714] huxueshi:liointc_chained_handle_irq 0
[    2.327714] huxueshi:liointc_chained_handle_irq 0
[    2.327715] huxueshi:liointc_chained_handle_irq 0
[    2.327716] huxueshi:liointc_chained_handle_irq 17
[    2.327717] huxueshi:liointc_chained_handle_irq 0
[    2.327717] huxueshi:liointc_chained_handle_irq 0

[    2.327718] huxueshi:liointc_chained_handle_irq 20
[    2.327719] huxueshi:liointc_chained_handle_irq 0
[    2.327720] huxueshi:liointc_chained_handle_irq 0
[    2.327720] huxueshi:liointc_chained_handle_irq 0
[    2.327721] huxueshi:liointc_chained_handle_irq 0
[    2.327722] huxueshi:liointc_chained_handle_irq 0
[    2.327723] huxueshi:liointc_chained_handle_irq 0
[    2.327723] huxueshi:liointc_chained_handle_irq 0
[    2.327724] huxueshi:liointc_chained_handle_irq 0
[    2.327725] huxueshi:liointc_chained_handle_irq 0

[    2.327726] huxueshi:liointc_chained_handle_irq 0
[    2.327726] huxueshi:liointc_chained_handle_irq 0
```

- [x] 找到这个东西是在什么时候初始化的 ?
  - [x] 注册时候，普普通通的样子啊

然后可以找到函数 loongson_crypto_init 和 acpi_register_gsi

```c
/*
[    0.894227] pnp: PnP ACPI init
[    0.894316] huxueshi:irq_map_generic_chip hw_irq=10, virq=27
[    0.894317] __irq_set_handler 27
[    0.894319] CPU: 1 PID: 1 Comm: swapper/0 Not tainted 4.19.167 #47
[    0.894320] Hardware name: Loongson Loongson-LS3A5000-7A1000-1w-V0.1-CRB/Loongson-LS3A5000-7A1000-1w-EVB-V1.21, BIOS Loongson-UDK2018-V2.0.04073-beta2 03/
[    0.894321] Stack : 000000000000089f 9000000000ec39a8 9000000477918000 900000047791b800
[    0.894323]         0000000000000000 900000047791b800 0000000000000000 9000000001369b48
[    0.894325]         9000000001369b40 0000000000000000 9000000001359302 000000000000008e
[    0.894327]         0000000000000000 9000000001261298 0000000000000001 000000000000089f
[    0.894328]         900000047aef8000 000000009bbb5611 0000000000000000 0000000000000001
[    0.894330]         000000000000089f 00000004774d0000 0000000000004006 0000000000000000
[    0.894332]         00000000000000b4 0000000000000000 0000000000000000 000000000000001b
[    0.894333]         0000000000000007 900000047268c0a0 0000000000000000 0000000000004000
[    0.894335]         0000000000000000 0000000000000000 0000000000000000 0000000000000000
[    0.894336]         0000000000000000 900000000020866c 0000000000000000 00000000000000b4
[    0.894338]         ...
[    0.894339] Call Trace:
[    0.894342] [<900000000020866c>] show_stack+0x2c/0x100
[    0.894345] [<9000000000ec39a8>] dump_stack+0x90/0xc0
[    0.894347] [<90000000002a0da8>] irq_domain_set_mapping+0x34/0xcc
[    0.894350] [<900000000029f488>] irq_domain_associate+0x178/0x1b0
[    0.894352] [<90000000002a0290>] irq_create_mapping+0xd0/0x148
[    0.894354] [<90000000002a09a8>] irq_create_fwspec_mapping+0x138/0x378
[    0.894356] [<900000000020f454>] acpi_register_gsi+0xd4/0x100
[    0.894359] [<900000000089b574>] acpi_dev_get_irqresource.isra.2+0x54/0xc0
[    0.894361] [<900000000089b6c0>] acpi_dev_resource_interrupt+0xe0/0x120
[    0.894364] [<90000000008dc844>] pnpacpi_allocated_resource+0xa4/0x388
[    0.894366] [<90000000008c5e68>] acpi_walk_resource_buffer+0xb8/0x180
[    0.894368] [<90000000008c66ac>] acpi_walk_resources+0x84/0xc4
[    0.894370] [<90000000008dcb78>] pnpacpi_parse_allocated_resource+0x50/0xc0
[    0.894373] [<90000000012fa470>] pnpacpi_add_device_handler+0x1a4/0x25c
[    0.894376] [<90000000008c0504>] acpi_ns_get_device_callback+0x178/0x18c
[    0.894378] [<90000000008bfd54>] acpi_ns_walk_namespace+0x158/0x210
[    0.894380] [<90000000008bfe68>] acpi_get_devices+0x5c/0x8c
[    0.894382] [<90000000012fa294>] pnpacpi_init+0x68/0xa0
[    0.894384] [<90000000002004fc>] do_one_initcall+0x6c/0x170
[    0.894386] [<90000000012d4ce0>] kernel_init_freeable+0x1f8/0x2b8
[    0.894389] [<9000000000eda7b4>] kernel_init+0x10/0xf4
[    0.894391] [<9000000000203ce8>] ret_from_kernel_thread+0xc/0x10
[    0.894392] irq: irq_domain_set_mapping  hwirq=10 virq=27
[    0.894414] pnp 00:00: Plug and Play ACPI device, IDs PNP0501 (active)
```

```c
/*
[    0.288643] huxueshi:irq_map_generic_chip hw_irq=20, virq=20
[    0.288647] __irq_set_handler 20
[    0.288652] CPU: 0 PID: 1 Comm: swapper/0 Not tainted 4.19.167 #47
[    0.288653] Hardware name: Loongson Loongson-LS3A5000-7A1000-1w-V0.1-CRB/Loongson-LS3A5000-7A1000-1w-EVB-V1.21, BIOS Loongson-UDK2018-V2.0.04073-beta2 03/
[    0.288655] Stack : 0000000000000433 9000000000ec39a8 9000000477918000 900000047791bb20
[    0.288657]         0000000000000000 900000047791bb20 0000000000000000 9000000001369b48
[    0.288659]         9000000001369b40 0000000000000000 9000000001359302 000000000000008e
[    0.288660]         0000000000000000 9000000001261298 0000000000000001 0000000000000433
[    0.288662]         900000047aef8000 000000009bbb5611 0000000000000000 0000000000000001
[    0.288664]         0000000000000433 0000000476cd0000 900000000132b7f8 0000000000000000
[    0.288665]         00000000000000b4 0000000000000000 0000000000000000 0000000000000014
[    0.288667]         0000000000000008 90000000012d4188 900000000134c008 9000000001312a58
[    0.288669]         0000000000000000 0000000000000000 0000000000000000 0000000000000000
[    0.288670]         0000000000000000 900000000020866c 0000000000000000 00000000000000b4
[    0.288672]         ...
[    0.288673] Call Trace:
[    0.288678] [<900000000020866c>] show_stack+0x2c/0x100
[    0.288684] [<9000000000ec39a8>] dump_stack+0x90/0xc0
[    0.288688] [<90000000002a0da8>] irq_domain_set_mapping+0x34/0xcc
[    0.288690] [<900000000029f488>] irq_domain_associate+0x178/0x1b0
[    0.288693] [<90000000002a0290>] irq_create_mapping+0xd0/0x148
[    0.288695] [<90000000002a09a8>] irq_create_fwspec_mapping+0x138/0x378
[    0.288698] [<90000000012d8a3c>] loongson_crypto_init+0x48/0xdc
[    0.288700] [<90000000002004fc>] do_one_initcall+0x6c/0x170
[    0.288704] [<90000000012d4ce0>] kernel_init_freeable+0x1f8/0x2b8
[    0.288707] [<9000000000eda7b4>] kernel_init+0x10/0xf4
[    0.288710] [<9000000000203ce8>] ret_from_kernel_thread+0xc/0x10
[    0.288712] irq: irq_domain_set_mapping  hwirq=20 virq=20
```

```c
/*
[    0.288721] huxueshi:irq_map_generic_chip hw_irq=17, virq=17
[    0.288721] __irq_set_handler 17
[    0.288723] CPU: 0 PID: 1 Comm: swapper/0 Not tainted 4.19.167 #47
[    0.288724] Hardware name: Loongson Loongson-LS3A5000-7A1000-1w-V0.1-CRB/Loongson-LS3A5000-7A1000-1w-EVB-V1.21, BIOS Loongson-UDK2018-V2.0.04073-beta2 03/
[    0.288725] Stack : 0000000000000450 9000000000ec39a8 9000000477918000 900000047791bb20
[    0.288726]         0000000000000000 900000047791bb20 0000000000000000 9000000001369b48
[    0.288728]         9000000001369b40 0000000000000000 9000000001359302 000000000000008e
[    0.288730]         0000000000000000 9000000001261298 0000000000000001 0000000000000450
[    0.288731]         900000047aef8000 000000009bbb5611 0000000000000000 0000000000000001
[    0.288733]         0000000000000450 0000000476cd0000 900000000132b7f8 0000000000000000
[    0.288734]         00000000000000b4 0000000000000000 0000000000000000 0000000000000011
[    0.288736]         0000000000000008 90000000012d4188 900000000134c008 9000000001312a58
[    0.288738]         0000000000000000 0000000000000000 0000000000000000 0000000000000000
[    0.288739]         0000000000000000 900000000020866c 0000000000000000 00000000000000b4
[    0.288741]         ...
[    0.288742] Call Trace:
[    0.288744] [<900000000020866c>] show_stack+0x2c/0x100
[    0.288746] [<9000000000ec39a8>] dump_stack+0x90/0xc0
[    0.288748] [<90000000002a0da8>] irq_domain_set_mapping+0x34/0xcc
[    0.288750] [<900000000029f488>] irq_domain_associate+0x178/0x1b0
[    0.288752] [<90000000002a0290>] irq_create_mapping+0xd0/0x148
[    0.288754] [<90000000002a09a8>] irq_create_fwspec_mapping+0x138/0x378
[    0.288756] [<90000000012d8a70>] loongson_crypto_init+0x7c/0xdc
[    0.288757] [<90000000002004fc>] do_one_initcall+0x6c/0x170
[    0.288759] [<90000000012d4ce0>] kernel_init_freeable+0x1f8/0x2b8
[    0.288761] [<9000000000eda7b4>] kernel_init+0x10/0xf4
[    0.288763] [<9000000000203ce8>] ret_from_kernel_thread+0xc/0x10
[    0.288765] irq: irq_domain_set_mapping  hwirq=17 virq=17
```


## pch_pic 真的是 apic 吗 ?
