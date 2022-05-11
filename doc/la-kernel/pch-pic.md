## init
- pch_pic_domains_init
  - pch_pic_init
    - pch_pic_init
      - pch_pic_reset

reference 7a1000 table 5.1, it's clear that pcie interrupt line plug into 32 / 33

## When pch_pic_irq_chip's hook get called
```c
static struct irq_chip pch_pic_irq_chip = {
  .name     = "PCH-PIC-HT",
  .irq_mask   = pch_pic_mask_irq,
  .irq_unmask   = pch_pic_unmask_irq,
  .irq_ack    = pch_pic_ack_irq,
  .irq_set_affinity = irq_chip_set_affinity_parent,
  .irq_set_type   = pch_pic_set_type,
  .irq_startup    = pch_pic_startup,
  .irq_shutdown   = pch_pic_shutdown,
  .flags      = IRQCHIP_SKIP_SET_WAKE,
};
```


### pch_pic_startup
irq desc alread allocated

```txt
#0  pch_pic_startup (d=0x900000027d208c28) at drivers/irqchip/irq-loongson-pch-pic.c:172
#1  0x9000000000287038 in __irq_startup (desc=0x900000027d208c28) at kernel/irq/chip.c:245
#2  0x9000000000287160 in irq_startup (desc=0x900000027d208c00, resend=true, force=<optimized out>) at kernel/irq/chip.c:268
#3  0x9000000000284d14 in __setup_irq (irq=17, desc=0x900000027d208c00, new=0x900000027d442c80) at kernel/irq/manage.c:1510
#4  0x9000000000284f18 in request_threaded_irq (irq=2099285032, handler=0x1, thread_fn=0x0, irqflags=<optimized out>, devname=0x40 <error: Cannot access memory at addre
ss 0x40>, dev_id=0x9000000000913114 <acpi_irq>) at kernel/irq/manage.c:1931
#5  0x90000000009135c8 in request_irq (dev=<optimized out>, name=<optimized out>, flags=<optimized out>, handler=<optimized out>, irq=<optimized out>) at ./include/linu x/interrupt.h:147
#6  acpi_os_install_interrupt_handler (gsi=68, handler=0x9000000000931c44 <acpi_ev_sci_xrupt_handler>, context=0x900000027ceac180) at drivers/acpi/osl.c:581
#7  0x9000000000931cc4 in acpi_ev_install_sci_handler () at drivers/acpi/acpica/evsci.c:156
#8  0x900000000092eb0c in acpi_ev_install_xrupt_handlers () at drivers/acpi/acpica/evevent.c:94
#9  0x90000000014e92f0 in acpi_enable_subsystem (flags=<optimized out>) at drivers/acpi/acpica/utxfinit.c:184
#10 0x90000000014e7214 in acpi_bus_init () at drivers/acpi/bus.c:1157
#11 acpi_init () at drivers/acpi/bus.c:1253
#12 0x9000000000200b8c in do_one_initcall (fn=0x90000000014e7154 <acpi_init>) at init/main.c:884
#13 0x90000000014a4e8c in do_initcall_level (level=<optimized out>) at ./include/linux/init.h:131
#14 do_initcalls () at init/main.c:960
#15 do_basic_setup () at init/main.c:978
#16 kernel_init_freeable () at init/main.c:1145
#17 0x9000000000f78fa8 in kernel_init (unused=<optimized out>) at init/main.c:1062
#18 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
Backtrace stopped: frame did not save the PC
```
### pch_pic_unmask_irq

```txt
#0  pch_pic_unmask_irq (d=0x900000027d208c28) at ./include/linux/irq.h:785
#1  0x90000000008a404c in pch_pic_startup (d=<optimized out>) at drivers/irqchip/irq-loongson-pch-pic.c:172
#2  0x9000000000287038 in __irq_startup (desc=0x900000027d208c28) at kernel/irq/chip.c:245
#3  0x9000000000287160 in irq_startup (desc=0x900000027d208c00, resend=true, force=<optimized out>) at kernel/irq/chip.c:268
#4  0x9000000000284d14 in __setup_irq (irq=17, desc=0x900000027d208c00, new=0x900000027d442c80) at kernel/irq/manage.c:1510
#5  0x9000000000284f18 in request_threaded_irq (irq=2099285032, handler=0x1, thread_fn=0x0, irqflags=<optimized out>, devname=0x40 <error: Cannot access memory at addre
ss 0x40>, dev_id=0x9000000000913114 <acpi_irq>) at kernel/irq/manage.c:1931
#6  0x90000000009135c8 in request_irq (dev=<optimized out>, name=<optimized out>, flags=<optimized out>, handler=<optimized out>, irq=<optimized out>) at ./include/linu
x/interrupt.h:147
#7  acpi_os_install_interrupt_handler (gsi=68, handler=0x9000000000931c44 <acpi_ev_sci_xrupt_handler>, context=0x900000027ceac180) at drivers/acpi/osl.c:581
#8  0x9000000000931cc4 in acpi_ev_install_sci_handler () at drivers/acpi/acpica/evsci.c:156
#9  0x900000000092eb0c in acpi_ev_install_xrupt_handlers () at drivers/acpi/acpica/evevent.c:94
#10 0x90000000014e92f0 in acpi_enable_subsystem (flags=<optimized out>) at drivers/acpi/acpica/utxfinit.c:184
#11 0x90000000014e7214 in acpi_bus_init () at drivers/acpi/bus.c:1157
#12 acpi_init () at drivers/acpi/bus.c:1253
#13 0x9000000000200b8c in do_one_initcall (fn=0x90000000014e7154 <acpi_init>) at init/main.c:884
#14 0x90000000014a4e8c in do_initcall_level (level=<optimized out>) at ./include/linux/init.h:131
#15 do_initcalls () at init/main.c:960
#16 do_basic_setup () at init/main.c:978
#17 kernel_init_freeable () at init/main.c:1145
#18 0x9000000000f78fa8 in kernel_init (unused=<optimized out>) at init/main.c:1062
#19 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
```

### `pch_pic_set_type`
- [x] How can I know the type
- [x] Will CPU receive irq if the level irq transfer high into low

it seems not all irq need this, only irq=18 called it.
it change the call back from level to edge.
```txt
#0  pch_pic_set_type (d=0x900000027d209028, type=1) at ./include/linux/irq.h:785
#1  0x90000000002843d0 in __irq_set_trigger (desc=0x900000027d209028, flags=<optimized out>) at kernel/irq/manage.c:780
#2  0x90000000002848a4 in __setup_irq (irq=18, desc=0x900000027d209000, new=0x900000027ce01680) at kernel/irq/manage.c:1469
#3  0x9000000000284f18 in request_threaded_irq (irq=2099286056, handler=0x1, thread_fn=0x9000000001587528 <mask>, irqflags=<optimized out>, devname=0x40 <error: Cannot
access memory at address 0x40>, dev_id=0x900000027d643640) at kernel/irq/manage.c:1931
#4  0x90000000002881d0 in devm_request_threaded_irq (dev=0x900000027d08e010, irq=18, handler=0x9000000000cf042c <ls2x_rtc_alarmirq>, thread_fn=0x0, irqflags=1, devname=
0x90000000012aae38 "ls2x-rtc alarm", dev_id=0x900000027d643640) at kernel/irq/devres.c:67
#5  0x9000000000cf09d4 in devm_request_irq (dev_id=<optimized out>, devname=<optimized out>, irqflags=<optimized out>, handler=<optimized out>, irq=<optimized out>, dev
=<optimized out>) at ./include/linux/interrupt.h:182
#6  ls2x_rtc_probe (pdev=0x900000027d08e000) at drivers/rtc/rtc-ls2x.c:360
#7  0x9000000000a2b4a4 in platform_drv_probe (_dev=0x900000027d08e010) at drivers/base/platform.c:584
#8  0x9000000000a2936c in really_probe (dev=0x900000027d08e010, drv=0x900000000144f070 <ls2x_rtc_driver+40>) at drivers/base/dd.c:506
#9  0x9000000000a295a4 in driver_probe_device (drv=0x900000000144f070 <ls2x_rtc_driver+40>, dev=0x900000027d08e010) at drivers/base/dd.c:667
#10 0x9000000000a2974c in __driver_attach (data=<optimized out>, dev=<optimized out>) at drivers/base/dd.c:903
#11 __driver_attach (dev=0x900000027d08e010, data=0x900000000144f070 <ls2x_rtc_driver+40>) at drivers/base/dd.c:872
#12 0x9000000000a270a0 in bus_for_each_dev (bus=<optimized out>, start=<optimized out>, data=0x9000000001587528 <mask>, fn=0x900000027ce01680) at drivers/base/bus.c:279
#13 0x9000000000a28bb0 in driver_attach (drv=<optimized out>) at drivers/base/dd.c:922
#14 0x9000000000a2861c in bus_add_driver (drv=0x900000000144f070 <ls2x_rtc_driver+40>) at drivers/base/bus.c:672
#15 0x9000000000a2a390 in driver_register (drv=0x900000000144f070 <ls2x_rtc_driver+40>) at drivers/base/driver.c:170
#16 0x9000000000200b8c in do_one_initcall (fn=0x90000000014f563c <rtc_init>) at init/main.c:884
#17 0x90000000014a4e8c in do_initcall_level (level=<optimized out>) at ./include/linux/init.h:131
#18 do_initcalls () at init/main.c:960
#19 do_basic_setup () at init/main.c:978
#20 kernel_init_freeable () at init/main.c:1145
#21 0x9000000000f78fa8 in kernel_init (unused=<optimized out>) at init/main.c:1062
#22 0x900000000020316c in ret_from_kernel_thread () at arch/loongarch/kernel/entry.S:85
```

### `pch_pic_mask_irq` / `pch_pic_ack_irq` / `pch_pic_unmask_irq`
- [x] `handle_level_irq`
```txt
#0  pch_pic_mask_irq (d=0x900000027d429a28) at drivers/irqchip/irq-loongson-pch-pic.c:95
#1  0x90000000002863a0 in mask_irq (desc=0x900000027d429a00) at kernel/irq/chip.c:424
#2  0x90000000002865e8 in mask_irq (desc=<optimized out>) at ./include/linux/irq.h:325
#3  mask_ack_irq (desc=<optimized out>) at kernel/irq/chip.c:412
#4  handle_level_irq (desc=0x900000027d429a00) at kernel/irq/chip.c:633
#5  0x9000000000280f00 in generic_handle_irq_desc (desc=<optimized out>) at ./include/linux/irqdesc.h:155
#6  generic_handle_irq (irq=<optimized out>) at kernel/irq/irqdesc.c:639
#7  0x90000000008a2b0c in extioi_irq_dispatch (desc=<optimized out>) at drivers/irqchip/irq-loongarch-extioi.c:254
#8  0x9000000000280f00 in generic_handle_irq_desc (desc=<optimized out>) at ./include/linux/irqdesc.h:155
#9  generic_handle_irq (irq=<optimized out>) at kernel/irq/irqdesc.c:639
#10 0x9000000000f85778 in do_IRQ (irq=<optimized out>) at arch/loongarch/kernel/irq.c:103
#11 0x90000000002033d4 in except_vec_vi_handler () at arch/loongarch/kernel/genex.S:92
```

```txt
#0  pch_pic_ack_irq (d=0x900000027d429a28) at drivers/irqchip/irq-loongson-pch-pic.c:185
#1  0x90000000002865fc in mask_ack_irq (desc=<optimized out>) at kernel/irq/chip.c:414
#2  handle_level_irq (desc=0x900000027d429a00) at kernel/irq/chip.c:633
#3  0x9000000000280f00 in generic_handle_irq_desc (desc=<optimized out>) at ./include/linux/irqdesc.h:155
#4  generic_handle_irq (irq=<optimized out>) at kernel/irq/irqdesc.c:639
#5  0x90000000008a2b0c in extioi_irq_dispatch (desc=<optimized out>) at drivers/irqchip/irq-loongarch-extioi.c:254
#6  0x9000000000280f00 in generic_handle_irq_desc (desc=<optimized out>) at ./include/linux/irqdesc.h:155
#7  generic_handle_irq (irq=<optimized out>) at kernel/irq/irqdesc.c:639
#8  0x9000000000f85778 in do_IRQ (irq=<optimized out>) at arch/loongarch/kernel/irq.c:103
#9  0x90000000002033d4 in except_vec_vi_handler () at arch/loongarch/kernel/genex.S:92
```

```txt
#0  pch_pic_unmask_irq (d=0x900000027d429a28) at ./include/linux/irq.h:785
#1  0x9000000000286484 in unmask_irq (desc=0x900000027d429a00) at kernel/irq/chip.c:435
#2  0x9000000000286634 in unmask_irq (desc=<optimized out>) at ./include/linux/irq.h:325
#3  cond_unmask_irq (desc=<optimized out>) at kernel/irq/chip.c:618
#4  handle_level_irq (desc=0x900000027d429a00) at kernel/irq/chip.c:652
#5  0x9000000000280f00 in generic_handle_irq_desc (desc=<optimized out>) at ./include/linux/irqdesc.h:155
#6  generic_handle_irq (irq=<optimized out>) at kernel/irq/irqdesc.c:639
#7  0x90000000008a2b0c in extioi_irq_dispatch (desc=<optimized out>) at drivers/irqchip/irq-loongarch-extioi.c:254
#8  0x9000000000280f00 in generic_handle_irq_desc (desc=<optimized out>) at ./include/linux/irqdesc.h:155
#9  generic_handle_irq (irq=<optimized out>) at kernel/irq/irqdesc.c:639
#10 0x9000000000f85778 in do_IRQ (irq=<optimized out>) at arch/loongarch/kernel/irq.c:103
#11 0x90000000002033d4 in except_vec_vi_handler () at arch/loongarch/kernel/genex.S:92
```
- what's `d->hwirq` in `pch_pic_ack_irq`
- if it's 2, that's unbelievable, 2 belongs to extioi

## how to program the `pch_pic` so that ttyS0 will route to 2 in extioi
