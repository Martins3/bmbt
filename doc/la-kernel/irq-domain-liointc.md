# 中断
`liointc_resume` 不会被调用


## `irq_gc_mask_disable_reg` 的 backtrace

1. 从关闭的角度
```txt
[  352.886915] [<9000000000209844>] show_stack+0x34/0x140
[  352.886917] [<9000000000c4d8f8>] dump_stack+0x90/0xc0
[  352.886918] [<9000000000c468dc>] irq_gc_mask_disable_reg+0x44/0xd0
[  352.886920] [<90000000002a2d48>] mask_irq.part.2+0x28/0x58
[  352.886922] [<90000000002a3974>] irq_shutdown+0xa4/0xb0
[  352.886923] [<900000000029f9bc>] __free_irq+0x344/0x3c8
[  352.886925] [<900000000029faf4>] free_irq+0x44/0xb8
[  352.886927] [<900000000086afd4>] univ8250_release_irq+0xdc/0xe8
[  352.886929] [<9000000000866664>] uart_port_shutdown+0x3c/0x68
[  352.886930] [<900000000086670c>] uart_tty_port_shutdown+0x7c/0x108
[  352.886932] [<900000000084cac0>] tty_port_shutdown+0x90/0xb0
[  352.886935] [<900000000084d7e8>] tty_port_close+0x58/0xa0
[  352.886936] [<90000000008434f0>] tty_release+0x140/0x6b0
[  352.886938] [<900000000045544c>] __fput+0xc4/0x248
[  352.886939] [<900000000024b5b0>] task_work_run+0xf8/0x148
[  352.886941] [<9000000000208fb8>] do_notify_resume+0xb8/0xc0
[  352.886943] [<9000000000203920>] work_notifysig+0x8/0x10
```

2. 从使用的角度来
```txt
[  352.877722] [<9000000000209844>] show_stack+0x34/0x140
[  352.877724] [<9000000000c4d8f8>] dump_stack+0x90/0xc0
[  352.877725] [<9000000000c468dc>] irq_gc_mask_disable_reg+0x44/0xd0
[  352.877727] [<90000000002a2ed0>] handle_level_irq+0x58/0x198
[  352.877728] [<900000000029d2c0>] generic_handle_irq+0x28/0x48
[  352.877730] [<900000000075f3b8>] liointc_chained_handle_irq+0x98/0x140
[  352.877731] [<900000000029d2c0>] generic_handle_irq+0x28/0x48
[  352.877733] [<9000000000c5b3bc>] do_IRQ+0x1c/0x30
[  352.877734] [<9000000000203a90>] except_vec_vi_handler+0xac/0xdc
[  352.877736] [<90000000002039e0>] __cpu_wait+0x20/0x24
[  352.877737] [<90000000002614ac>] do_idle+0x274/0x340
[  352.877739] [<90000000002617b8>] cpu_startup_entry+0x20/0x28
[  352.877740] [<90000000014b8c68>] start_kernel+0x44c/0x474
[  352.877742] [<9000000000c4dba0>] dtb_found+0x94/0x98
[  352.886892] [huxueshi:irq_gc_mask_disable_reg:41]
```
