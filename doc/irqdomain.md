# irq domain
当完成 xv6 之后，也许对于中断的上下文保存，interrupt 和 exception 的区别等，你现在可能信心满满，感觉 Linux kernel 的中断设计差不多也是这么回事。
但是实际上，Linux 面对了现实世界的中的各种挑战，在中断处理中不得不考虑一些实际问题。

> abbr
>
> intc : interrupt controller，比如 ioapic lapic gic 等

- 因为各种原因，设备不能直接将中断发送到 CPU，比如一个时钟周期，多个设备同时发送中断，是需要仲裁的，比如 CPU 没有这么的引脚等等，所以设备的中断是首先发送给 intc，然后发送给 cpu
- 为了 intc 的可扩展性，不同的 intc 的功能各有侧重，或者 intc 的引脚数目限制，所有的设备的中断不能直接连接到一个 intc，而是 intc 连接到 intc 上
- 为了灵活的描述这种 chained intc 的情况，linux 使用了 irq domain 的操作

在内核和 irqdomain 设计主要的三个结构体:
- struct irq_chip : 用于操控 intc，通过 `ioapic_chip::irq_set_affinity` 可以实现修改 ioapic 将中断导入到哪一个中断中
- struct irq_desc : 一个 irq 对应一个 irq_desc，其中 irq_desc::action 就是各个驱动注册的处理函数
- struct irqdomain : 一个中断控制芯片对应一个 irqdomain

而 irq_data 聚合了一些常用信息，每一个 irq_desc 持有一个。


这里描述的很简单，现在来看看各个平台是如何把自己的架构和这个模型匹配起来的。

## Loongson
简单理解，设备中断发送到一个 extioi intc 上，然后 extioi intc 会根据实现的路由规则发送发送到 cpu 上，然后 cpu 的 estat 记录下，并且 cpu 跳转到中断处理函数中。

下面使用 serial 作为例子分析
- loongarch_cpu_intc_map 中调用 set_vi_handler 注册 plat_irq_dispatch
- plat_irq_dispatch 的参数为 **3**
  - 调用 irq_linear_revmap 将 3 装换为 **53** 去调用 do_IRQ
- do_IRQ 使用 53 获取其对应的 irq_desc
- 调用其 hook irq_desc::handle_irq
- 调用到 extioi_irq_dispatch
  - 通过读去 extioi 的状态，确定是 bit **2** 被拉高了
  - 调用 irq_linear_revmap 将 2 装换为 **19** 去调用 do_IRQ
- 调用 action handler 也就是 serial8250_interrupt

在这里我们看到了编号的变化过程为: 3 -> 53 -> 2 -> 19 下面来仔细说明:
- 3
  - do_vi 参数为发生中断时，estat 的那一个位被 set 的
  - do_vi 调用 set_vi_handler 注册的 handler 也就是 plat_irq_dispatch 的时候，继续传递这个参数，也就是 3
- 53
  - 在 `__loongarch_cpu_irq_init` 中，会创建出来 16 个 irq_desc，其 irq 为 (51, 52, ……) ，并且设置 cpu_inct 的映射为 (1, 2, 3 ... 16) -> (51, 52, 53, ... 66)
  - 所以在 plat_irq_dispatch 中，使用 3 可以获取到 53，并且获取到 irq=53 的 irq_desc
  - 接下来回去执行 irq=53 的 irq_desc::handle_irq
- 2
  - 在 extioi_vec_init 中通过调用 irq_set_chained_handler_and_data 会注册 irq=53 的 irq_desc::handlr_irq 为 extioi_irq_dispatch
  - 在 extioi_irq_dispatch 知道是其 2 bit 被 set 上
  - 通过 irq_linear_revmap 知道在 extioi irqdomain 中 2 被映射到 19 了
  - 2 是硬件配置的。在制作主板的时候，serial 的中断如何连接的就确定了，操作系统通过解析 firmware 信息知道这件事情
  - 19 是动态分配的 irq_desc 的 irq，只是一个索引作用
- 19
  - irq=19 的 irq_desc 的 irq_desc::irq_action 是调用 request_irq 的时候注册的

下面配合代码具体说明一下:
1. 在 acpi 的解析中，创建了从 2 到 19 的映射
```txt
#0  extioi_domain_alloc (domain=0x900000027c024800, virq=19, nr_irqs=1, arg=0x900000027cd338f0) at drivers/irqchip/irq-loongarch-extioi.c:303
#1  0x90000000008a3b20 in pch_pic_alloc (domain=0x900000027c0bfa00, virq=19, nr_irqs=<optimized out>, arg=<optimized out>) at drivers/irqchip/irq-loongson-pch-pic.c:287
#2  0x900000000028bf28 in irq_domain_alloc_irqs_hierarchy (arg=<optimized out>, nr_irqs=<optimized out>, irq_base=<optimized out>, domain=<optimized out>) at kernel/irq/irqdomain.c:1270
#3  __irq_domain_alloc_irqs (domain=0x900000027c0bfa00, irq_base=19, nr_irqs=1, node=<optimized out>, arg=<optimized out>, realloc=<optimized out>, affinity=<optimizedout>) at kernel/irq/irqdomain.c:1326
#4  0x900000000028c4cc in irq_domain_alloc_irqs (arg=<optimized out>, node=<optimized out>, nr_irqs=<optimized out>, domain=<optimized out>) at ./include/linux/irqdomain.h:466
#5  irq_create_fwspec_mapping (fwspec=0x900000027cd33a20) at kernel/irq/irqdomain.c:810
#6  0x900000000020bc40 in acpi_register_gsi (dev=<optimized out>, gsi=17, trigger=<optimized out>, polarity=<optimized out>) at arch/loongarch/kernel/acpi.c:90
#7  0x900000000020bcb4 in acpi_gsi_to_irq (gsi=<optimized out>, irqp=0x900000027cd33cdc) at arch/loongarch/kernel/acpi.c:49#8  0x90000000009132c4 in acpi_os_install_interrupt_handler (gsi=68, handler=0x9000000000931978 <acpi_ev_sci_xrupt_handler>, context=0x900000027d0f4180) at drivers/acpi/osl.c:573
#9  0x90000000009319f8 in acpi_ev_install_sci_handler () at drivers/acpi/acpica/evsci.c:156
#10 0x900000000092e840 in acpi_ev_install_xrupt_handlers () at drivers/acpi/acpica/evevent.c:94
#11 0x90000000014e92f0 in acpi_enable_subsystem (flags=<optimized out>) at drivers/acpi/acpica/utxfinit.c:184
#12 0x90000000014e7214 in acpi_bus_init () at drivers/acpi/bus.c:1157
#13 acpi_init () at drivers/acpi/bus.c:1253
#14 0x9000000000200b8c in do_one_initcall (fn=0x90000000014e7154 <acpi_init>) at init/main.c:884
#15 0x90000000014a4e8c in do_initcall_level (level=<optimized out>) at ./include/linux/init.h:131
#16 do_initcalls () at init/main.c:960
#17 do_basic_setup () at init/main.c:978
#18 kernel_init_freeable () at init/main.c:1145
#19 0x9000000000f790f8 in kernel_init (unused=<optimized out>) at init/main.c:1062
#20 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
Backtrace stopped: frame did not save the PC
```
在 acpi_register_gsi 组装了 fwspec 其中的 gsi 为 66，但是在 pch_pic_domain_translate 中，其被转换为 2 (66-64)了
```c
      struct irq_fwspec fwspec;

      fwspec.fwnode = handle;
      fwspec.param[0] = gsi;
      fwspec.param_count = 1;
```
并且其中选择 pch_pic 作为起点，逐步向上分析:
```c
    handle = pch_pic_get_fwnode(id);
```

在 `__irq_domain_alloc_irqs` 中调用  `__irq_alloc_descs` 分配 irq=19 的 irq_desc

然后 extioi_domain_alloc 调用 irq_domain_set_info 完成 2 到 19 的映射

2. virq=19 作为 irq resource 添加进去
```txt
#0  pnp_add_resource (dev=0x900000027ce0f000, res=0x900000027cd33ad8) at drivers/pnp/resource.c:519
#1  0x900000000095df2c in pnpacpi_add_irqresource (r=<optimized out>, dev=<optimized out>) at drivers/pnp/pnpacpi/rsparser.c:115
#2  pnpacpi_allocated_resource (res=0x900000027d4526c8, data=0x900000027ce0f000) at drivers/pnp/pnpacpi/rsparser.c:192
#3  0x9000000000946f8c in acpi_walk_resource_buffer (buffer=<optimized out>, user_function=0x900000027cd33ad8, context=0x0) at drivers/acpi/acpica/rsxface.c:547
#4  0x9000000000947458 in acpi_walk_resources (context=<optimized out>, user_function=<optimized out>, name=<optimized out>, device_handle=<optimized out>) at drivers/acpi/acpica/rsxface.c:623
#5  acpi_walk_resources (device_handle=<optimized out>, name=<optimized out>, user_function=0x900000000095ddfc <pnpacpi_allocated_resource>, context=0x900000027ce0f000)at drivers/acpi/acpica/rsxface.c:594
#6  0x900000000095e18c in pnpacpi_parse_allocated_resource (dev=0x900000027ce0f000) at drivers/pnp/pnpacpi/rsparser.c:289
#7  0x90000000014ea154 in pnpacpi_add_device (device=<optimized out>) at drivers/pnp/pnpacpi/core.c:271
#8  pnpacpi_add_device_handler (handle=<optimized out>, lvl=<optimized out>, context=<optimized out>, rv=<optimized out>) at drivers/pnp/pnpacpi/core.c:308
#9  0x9000000000941f40 in acpi_ns_get_device_callback (return_value=<optimized out>, context=<optimized out>, nesting_level=<optimized out>, obj_handle=<optimized out>)at drivers/acpi/acpica/nsxfeval.c:740
#10 acpi_ns_get_device_callback (obj_handle=0x900000027c0d0398, nesting_level=2, context=0x900000027cd33d58, return_value=0x0) at drivers/acpi/acpica/nsxfeval.c:635
#11 0x9000000000941798 in acpi_ns_walk_namespace (type=<optimized out>, start_node=0x900000027c0d0050, max_depth=<optimized out>, flags=<optimized out>, descending_call
back=<optimized out>, ascending_callback=0x0, context=0x900000027cd33d58, return_value=0x0) at drivers/acpi/acpica/nswalk.c:229
#12 0x90000000009418ac in acpi_get_devices (HID=<optimized out>, user_function=<optimized out>, context=<optimized out>, return_value=0x0) at drivers/acpi/acpica/nsxfeval.c:805
#13 0x90000000014e9f78 in pnpacpi_init () at drivers/pnp/pnpacpi/core.c:321
#14 0x9000000000200b8c in do_one_initcall (fn=0x90000000014e9f10 <pnpacpi_init>) at init/main.c:884
#15 0x90000000014a4e8c in do_initcall_level (level=<optimized out>) at ./include/linux/init.h:131
#16 do_initcalls () at init/main.c:960
#17 do_basic_setup () at init/main.c:978
#18 kernel_init_freeable () at init/main.c:1145
#19 0x9000000000f790f8 in kernel_init (unused=<optimized out>) at init/main.c:1062
#20 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
Backtrace stopped: frame did not save the PC
```

```txt
[    0.776121] pnp 00:00: [irq 19]
```

3. 在 serial 驱动启动的时候，会访问 irq resource
```c
#0  serial_pnp_probe (dev=0x900000027d32f000, dev_id=0x9000000001068570 <pnp_dev_table+1248>) at drivers/tty/serial/8250/8250_pnp.c:440
#1  0x9000000000959b54 in pnp_device_probe (dev=0x900000027d32f000) at drivers/pnp/driver.c:109
#2  0x9000000000a294cc in really_probe (dev=0x900000027d32f000, drv=0x9000000001428160 <serial_pnp_driver+64>) at drivers/base/dd.c:506
#3  0x9000000000a29704 in driver_probe_device册 (drv=0x9000000001428160 <serial_pnp_driver+64>, dev=0x900000027d32f000) at drivers/base/dd.c:667
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
4. 然后 serial 驱动注册 irq=19 的 irq_desc 的 irq_action
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


其他问题的讨论:
1. 虽然创建了很多 irq_desc，但是不是所有的都会在 /proc/interrupts 中显示出来，具体代码看 show_interrupts
```c
  if ((!desc->action || irq_desc_is_chained(desc)) && !any_count)
    goto outsparse;
```
2. 在 request_threaded_irq 中会最后调用到 ext_set_irq_affinity，从而决定中断路由到哪里去。

### loongson doc
```txt
loongson@loongson-pc:~$ cat /proc/interrupts
            CPU0
  17:          0  PCH-PIC-EXT    4  acpi
  18:          0  PCH-PIC-EXT    3  ls2x-rtc alarm
  19:       1216  PCH-PIC-EXT    2  ttyS0
  22:          0  PCH-MSI-EXT   32  virtio1-config
  23:       1282  PCH-MSI-EXT   33  virtio1-req.0
  61:       1201  COREINTC   11  timer
IPI0:          0       Rescheduling interrupts
IPI1:          0       Call Function interrupts
 ERR:          0
```

```c
#0  serial8250_interrupt (irq=19, dev_id=0x900000027da3ca00) at ./include/linux/spinlock_api_smp.h:141
#1  0x9000000000282070 in __handle_irq_event_percpu (desc=0x900000027d7f5a00, flags=0x900000027c0cbeec) at kernel/irq/handle.c:152
#2  0x90000000002822dc in handle_irq_event_percpu (desc=0x900000027d7f5a00) at kernel/irq/handle.c:192
#3  0x9000000000282380 in handle_irq_event (desc=0x900000027d7f5a00) at kernel/irq/handle.c:209
#4  0x90000000002865a8 in handle_level_irq (desc=0x900000027d7f5a00) at kernel/irq/chip.c:650
#5  0x9000000000280f00 in generic_handle_irq_desc (desc=<optimized out>) at ./include/linux/irqdesc.h:155
#6  generic_handle_irq (irq=<optimized out>) at kernel/irq/irqdesc.c:639
#7  0x90000000008a2ab8 in extioi_irq_dispatch (desc=<optimized out>) at drivers/irqchip/irq-loongarch-extioi.c:254
#8  0x9000000000280f00 in generic_handle_irq_desc (desc=<optimized out>) at ./include/linux/irqdesc.h:155
#9  generic_handle_irq (irq=<optimized out>) at kernel/irq/irqdesc.c:639
#10 0x9000000000f85908 in do_IRQ (irq=<optimized out>) at arch/loongarch/kernel/irq.c:103
#11 0x90000000002033d4 in except_vec_vi_handler () at arch/loongarch/kernel/genex.S:92
```

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

```c
/*
#0  hrtimer_interrupt (dev=0x9000000006000080) at kernel/time/hrtimer.c:1511
#1  0x90000000002084a8 in constant_timer_interrupt (irq=<optimized out>, data=<optimized out>) at arch/loongarch/kernel/time.c:49
#2  0x900000000029d4ac in __handle_irq_event_percpu (desc=0x9000000006000080, flags=0x0) at kernel/irq/handle.c:149
#3  0x900000000029d708 in handle_irq_event_percpu (desc=0x900000027c0bf000) at kernel/irq/handle.c:189
#4  0x90000000002a2c9c in handle_percpu_irq (desc=0x900000027c0bf000) at kernel/irq/chip.c:873
#5  0x900000000029c344 in generic_handle_irq_desc (desc=<optimized out>) at ./include/linux/irqdesc.h:155
#6  generic_handle_irq (irq=<optimized out>) at kernel/irq/irqdesc.c:639
#7  0x9000000000fdcec0 in do_IRQ (irq=<optimized out>) at arch/loongarch/kernel/irq.c:103
#8  0x9000000000203414 in except_vec_vi_handler () at arch/loongarch/kernel/genex.S:92
Backtrace stopped: frame did not save the PC
```

## x86
x86 中 irq_domain_ops:
- arch/x86/kernel/apic/vector.c : x86_vector_domain_ops
- arch/x86/kernel/apic/io_apic.c : mp_ioapic_irqdomain_ops

在 Loongson extioi 的功能和 ioapic 很类似，但是不存在对应的 lapic

从这里看，其只是需要一个 domain 的装换，因为在 common_interrupt 中，通过 `vector_irq` 就直接完成 x86_vector_domain_ops 的装换了。


在 early_irq_init 中一次性分配完成所有的 irq desc 的
```c
#0  x86_vector_alloc_irqs (domain=0xffff88800304d180, virq=1, nr_irqs=1, arg=0xffffffff82403dd8) at arch/x86/kernel/apic/vector.c:533
#1  0xffffffff8104d77f in mp_irqdomain_alloc (domain=0xffff8880030f6a00, virq=1, nr_irqs=1, arg=0xffffffff82403dd8) at arch/x86/kernel/apic/io_apic.c:3022
#2  0xffffffff810c8d35 in irq_domain_alloc_irqs_hierarchy (arg=0xffffffff82403dd8, nr_irqs=1, irq_base=1, domain=0xffff8880030f6a00) at kernel/irq/irqdomain.c:1423
#3  __irq_domain_alloc_irqs (domain=domain@entry=0xffff8880030f6a00, irq_base=irq_base@entry=1, nr_irqs=nr_irqs@entry=1, node=node@entry=-1, arg=arg@entry=0xffffffff82403dd8, realloc=realloc@entry=true, affinity=0x0 <fixed_percpu_data>) at kernel/irq/irqdomain.c:1479
#4  0xffffffff8104c67a in alloc_isa_irq_from_domain (domain=domain@entry=0xffff8880030f6a00, irq=irq@entry=1, ioapic=ioapic@entry=0, info=info@entry=0xffffffff82403dd8,pin=1) at arch/x86/kernel/apic/io_apic.c:1008
#5  0xffffffff8104d383 in mp_map_pin_to_irq (gsi=1, idx=<optimized out>, ioapic=<optimized out>, pin=pin@entry=1, flags=1, info=info@entry=0x0 <fixed_percpu_data>) at arch/x86/kernel/apic/io_apic.c:1057
#6  0xffffffff8104d539 in pin_2_irq (idx=<optimized out>, ioapic=ioapic@entry=0, pin=pin@entry=1, flags=flags@entry=1) at arch/x86/kernel/apic/io_apic.c:1103
#7  0xffffffff82b759eb in setup_IO_APIC_irqs () at arch/x86/kernel/apic/io_apic.c:1219
#8  setup_IO_APIC () at arch/x86/kernel/apic/io_apic.c:2418
#9  0xffffffff82b746f1 in apic_bsp_setup (upmode=<optimized out>) at arch/x86/kernel/apic/apic.c:2603
#10 apic_intr_mode_init () at arch/x86/kernel/apic/apic.c:1441
#11 0xffffffff82b69051 in x86_late_time_init () at arch/x86/kernel/time.c:100
#12 0xffffffff82b620f2 in start_kernel () at init/main.c:1099
#13 0xffffffff81000107 in secondary_startup_64 () at arch/x86/kernel/head_64.S:283
#14 0x0000000000000000 in ?? ()
```
此时的 irq desc 只是只是创建了，但是其 irq_desc::action_handler 没有创建。

```c
#0  serial8250_interrupt (irq=4, dev_id=0xffff88800317b8c0) at ./include/linux/spinlock.h:349
#1  0xffffffff810c2465 in __handle_irq_event_percpu (desc=desc@entry=0xffff8880030f5000, flags=flags@entry=0xffffc90000003f74) at kernel/irq/handle.c:158
#2  0xffffffff810c25dc in handle_irq_event_percpu (desc=desc@entry=0xffff8880030f5000) at kernel/irq/handle.c:198
#3  0xffffffff810c264f in handle_irq_event (desc=desc@entry=0xffff8880030f5000) at kernel/irq/handle.c:215
#4  0xffffffff810c66b6 in handle_edge_irq (desc=0xffff8880030f5000) at kernel/irq/chip.c:822
#5  0xffffffff81022f24 in generic_handle_irq_desc (desc=0xffff8880030f5000) at ./include/linux/irqdesc.h:158
#6  handle_irq (regs=<optimized out>, desc=0xffff8880030f5000) at arch/x86/kernel/irq.c:231
#7  __common_interrupt (regs=<optimized out>, vector=39) at arch/x86/kernel/irq.c:250
#8  0xffffffff81bcec2f in common_interrupt (regs=0xffffc9000022fbb8, error_code=<optimized out>) at arch/x86/kernel/irq.c:240
Backtrace stopped: Cannot access memory at address 0xffffc90000004018
```
这里 `__common_interrupt` 的参数 vector=39 是 idt 号码
而 irq=4 的原有则是 x86 类似的

1. pnp_add_resource : 在 acpi 初始化中添加 serial dev 的 irq=4 的 resource
2. serial_pnp_probe : 在 serial 驱动初始化过程中访问资源
```c
  if (pnp_irq_valid(dev, 0))
    uart.port.irq = pnp_irq(dev, 0);
```
3. 驱动注册
```c
#0  request_threaded_irq (irq=4, handler=handler@entry=0xffffffff814f3b30 <serial8250_interrupt>, thread_fn=thread_fn@entry=0x0 <fixed_percpu_data>, irqflags=0, devname
=0xffff88800339a550 "ttyS0", dev_id=dev_id@entry=0xffff88800305d8c0) at kernel/irq/manage.c:2120
```

注意，这里的 irq=4 就是 acpi 中获取的 gsi，而在 Loongson 的 irq=19 是系统分配的，其从 acpi 获取的是 gsi=66

### x86 doc
```c
           CPU0
  0:         38   IO-APIC   2-edge      timer
  1:        198   IO-APIC   1-edge      i8042
  4:        320   IO-APIC   4-edge      ttyS0
  8:          1   IO-APIC   8-edge      rtc0
  9:          0   IO-APIC   9-fasteoi   acpi
 11:         43   IO-APIC  11-fasteoi   eth0
 12:        125   IO-APIC  12-edge      i8042
 14:        591   IO-APIC  14-edge      ata_piix
 15:         82   IO-APIC  15-edge      ata_piix
```

```c
#0  hrtimer_interrupt (dev=0xffff888007a1af40) at kernel/time/hrtimer.c:1783
#1  0xffffffff81048d5b in local_apic_timer_interrupt () at arch/x86/kernel/apic/apic.c:1086
#2  __sysvec_apic_timer_interrupt (regs=0xffffc9000022fbb8) at arch/x86/kernel/apic/apic.c:1103
#3  0xffffffff81bd09f9 in sysvec_apic_timer_interrupt (regs=0xffffc9000022fbb8) at arch/x86/kernel/apic/apic.c:1097
```

```c
#0  e1000_intr (irq=11, data=0xffff8880031aa000) at drivers/net/ethernet/intel/e1000/e1000_main.c:3749
#1  0xffffffff810c2465 in __handle_irq_event_percpu (desc=desc@entry=0xffff8880030f5e00, flags=flags@entry=0xffffc90000003f6c) at kernel/irq/handle.c:158
#2  0xffffffff810c25dc in handle_irq_event_percpu (desc=desc@entry=0xffff8880030f5e00) at kernel/irq/handle.c:198
#3  0xffffffff810c264f in handle_irq_event (desc=desc@entry=0xffff8880030f5e00) at kernel/irq/handle.c:215
#4  0xffffffff810c634e in handle_fasteoi_irq (desc=0xffff8880030f5e00) at kernel/irq/chip.c:717
#5  0xffffffff81022f24 in generic_handle_irq_desc (desc=0xffff8880030f5e00) at ./include/linux/irqdesc.h:158
#6  handle_irq (regs=<optimized out>, desc=0xffff8880030f5e00) at arch/x86/kernel/irq.c:231
#7  __common_interrupt (regs=<optimized out>, vector=40) at arch/x86/kernel/irq.c:250
#8  0xffffffff81bcec2f in common_interrupt (regs=0xffffc9000044fb88, error_code=<optimized out>) at arch/x86/kernel/irq.c:240
```
现在来看，x86 架构来分析 irq domain 非常的痛苦，因为可以编码保证，ioapic 路由到 cpu 的时候，中断号不会重叠的。

## ref
- [LoyenWang](https://www.cnblogs.com/LoyenWang/p/13052677.html)
- [What are linux irq domains, why are they needed?](https://stackoverflow.com/questions/34371352/what-are-linux-irq-domains-why-are-they-needed)
- [kernel doc](https://www.kernel.org/doc/html/latest/core-api/irq/irq-domain.html)
- [兰新宇](https://zhuanlan.zhihu.com/p/85353687)
