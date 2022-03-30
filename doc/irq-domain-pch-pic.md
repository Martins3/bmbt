# pch_pic 的定位和 x86 ioapic 的定位相同

本来以为 pch_pic 是一个根本不需要的中断控制器，
作为中断路由的，如果不去 mask 掉，那么他就会不停的向 CPU 汇报。

而 extioi 的定位则是 apic 的。

```txt
#0  pch_pic_ack_irq (d=0x900000027d8cda28) at drivers/irqchip/irq-loongson-pch-pic.c:185
#1  0x90000000002865fc in mask_ack_irq (desc=<optimized out>) at kernel/irq/chip.c:414
#2  handle_level_irq (desc=0x900000027d8cda00) at kernel/irq/chip.c:633
#3  0x9000000000280f00 in generic_handle_irq_desc (desc=<optimized out>) at ./include/linux/irqdesc.h:155
#4  generic_handle_irq (irq=<optimized out>) at kernel/irq/irqdesc.c:639
#5  0x90000000008a2b58 in extioi_irq_dispatch (desc=<optimized out>) at drivers/irqchip/irq-loongarch-extioi.c:256
#6  0x9000000000280f00 in generic_handle_irq_desc (desc=<optimized out>) at ./include/linux/irqdesc.h:155
#7  generic_handle_irq (irq=<optimized out>) at kernel/irq/irqdesc.c:639
#8  0x9000000000f857f8 in do_IRQ (irq=<optimized out>) at arch/loongarch/kernel/irq.c:103
#9  0x90000000002033d4 in except_vec_vi_handler () at arch/loongarch/kernel/genex.S:92
Backtrace stopped: frame did not save the PC
```
- 实际上，pch_pic 这个中断控制器还是存在的，但是为什么没有 dispatch 的啊

为什么他的 base 是这个东西啊:
```txt
[    0.000000] pch-pic: huxueshi:pch_pic_init ffff800002010000
```

- [x] 可以找到这 pch_pic 的文档吗?
  - 7a1000 manual chapter 5

-> hardcode is enough
```c
    register_pch_pic(0, LS7A_PCH_REG_BASE, LOONGSON_PCH_IRQ_BASE);
    irqchip_init_default();
```

- register_pch_pic
```txt
#0  register_pch_pic (id=0, address=268435456, irq_base=64) at arch/loongarch/la64/irq.c:96
#1  0x90000000014ac048 in acpi_parse_pch_pic (header=0x0, end=<optimized out>) at arch/loongarch/kernel/acpi.c:118
#2  0x90000000014e5b30 in acpi_parse_entries_array (max_entries=<optimized out>, proc_num=<optimized out>, proc=<optimized out>, table_header=<optimized out>, table_siz
e=<optimized out>, id=<optimized out>) at drivers/acpi/tables.c:298
#3  acpi_table_parse_entries_array (id=0x0, table_size=<optimized out>, proc=0x900000000134bd58, proc_num=1, max_entries=16) at drivers/acpi/tables.c:353
#4  0x90000000014e5c18 in acpi_table_parse_entries (id=<optimized out>, table_size=<optimized out>, entry_id=<optimized out>, handler=<optimized out>, max_entries=<opti
mized out>) at drivers/acpi/tables.c:372
#5  0x90000000014e5c40 in acpi_table_parse_madt (id=<optimized out>, handler=<optimized out>, max_entries=<optimized out>) at drivers/acpi/tables.c:380
#6  0x90000000014ac348 in acpi_parse_madt_pch_pic_entries () at arch/loongarch/kernel/acpi.c:141
#7  acpi_process_madt () at arch/loongarch/kernel/acpi.c:308
#8  acpi_boot_init () at arch/loongarch/kernel/acpi.c:343
#9  0x90000000014a8620 in platform_init () at arch/loongarch/la64/init.c:474
#10 0x90000000014ab260 in setup_arch (cmdline_p=0x900000000134be58) at arch/loongarch/kernel/setup.c:796
#11 0x90000000014a47c4 in start_kernel () at init/main.c:551
#12 0x9000000000f78ea4 in kernel_entry () at arch/loongarch/kernel/head.S:129
```

## 奇怪的地方在于 pch_pic_mask_irq 的调用位置
因为 irq_desc 关联的 irq_data 是 pch_pic 的，所以最后会这这个位置调用 mask 的

```txt
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

```txt
#0  pch_pic_mask_irq (d=0x900000027d87da28) at drivers/irqchip/irq-loongson-pch-pic.c:95
#1  0x90000000002863a0 in mask_irq (desc=0x900000027d87da00) at kernel/irq/chip.c:424
#2  0x90000000002865e8 in mask_irq (desc=<optimized out>) at ./include/linux/irq.h:325
#3  mask_ack_irq (desc=<optimized out>) at kernel/irq/chip.c:412
#4  handle_level_irq (desc=0x900000027d87da00) at kernel/irq/chip.c:633
#5  0x9000000000280f00 in generic_handle_irq_desc (desc=<optimized out>) at ./include/linux/irqdesc.h:155
#6  generic_handle_irq (irq=<optimized out>) at kernel/irq/irqdesc.c:639
#7  0x90000000008a2b0c in extioi_irq_dispatch (desc=<optimized out>) at drivers/irqchip/irq-loongarch-extioi.c:254
#8  0x9000000000280f00 in generic_handle_irq_desc (desc=<optimized out>) at ./include/linux/irqdesc.h:155
#9  generic_handle_irq (irq=<optimized out>) at kernel/irq/irqdesc.c:639
#10 0x9000000000f85778 in do_IRQ (irq=<optimized out>) at arch/loongarch/kernel/irq.c:103
#11 0x90000000002033d4 in except_vec_vi_handler () at arch/loongarch/kernel/genex.S:92
Backtrace stopped: frame did not save the PC
```
