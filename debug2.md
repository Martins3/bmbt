# debug 2

```plain
[huxueshi:apic_send_msi:631] 61
[huxueshi:apic_send_msi:631] 41
[   10.236717] IPv6: ADDRCONF(NETDEV_UP): eth0: link is not ready
[   10.250460] ip (819) used greatest stack depth: 6784 bytes left
udhcpc: started, v1.35.0
udhcpc: broadcasting discover
[huxueshi:apic_send_msi:631] 61
[   10.660939] e1000e: eth0 NIC Link is Up 1000 Mbps Full Duplex, Flow Control: Rx/Tx
[huxueshi:apic_send_msi:631] 41
[   10.673331] IPv6: ADDRCONF(NETDEV_CHANGE): eth0: link becomes ready
[huxueshi:apic_send_msi:631] 51
[huxueshi:apic_send_msi:631] 41
udhcpc: broadcasting discover
[huxueshi:apic_send_msi:631] 41
[   16.039304] ------------[ cut here ]------------
[   16.040841] WARNING: CPU: 0 PID: 0 at net/sched/sch_generic.c:305 dev_watchdog+0x1f7/0x200()
[   16.043054] NETDEV WATCHDOG: eth0 (e1000e): transmit queue 0 timed out
[   16.044397] Modules linked in:
[   16.046011] CPU: 0 PID: 0 Comm: swapper/0 Not tainted 4.4.142+ #30
[   16.046011] Hardware name: Bochs Bochs, BIOS Bochs 01/01/2011
[   16.046011]  00200286 00200286 c7821ed0 c12e696f c1a4f04d 00000131 c7821efc c104e882
[   16.046011]  c1a562c0 c7821f28 00000000 c1a4f04d 00000131 c16d7c87 c0044000 00000000
[   16.046011]  fffffefa c7821f14 c104e8d2 00000009 c7821f0c c1a562c0 c7821f28 c7821f48
[   16.046011] Call Trace:
[   16.046011]  [<c12e696f>] dump_stack+0x4b/0x6c
[   16.046011]  [<c104e882>] warn_slowpath_common+0x82/0xa0
[   16.046011]  [<c16d7c87>] ? dev_watchdog+0x1f7/0x200
[   16.046011]  [<c104e8d2>] warn_slowpath_fmt+0x32/0x40
[   16.046011]  [<c16d7c87>] dev_watchdog+0x1f7/0x200
[   16.046011]  [<c10a188e>] call_timer_fn+0x2e/0x120
[   16.046011]  [<c16d7a90>] ? dev_deactivate_queue.constprop.0+0x50/0x50
[   16.046011]  [<c10a1d7c>] run_timer_softirq+0x11c/0x230
[   16.046011]  [<c16d7a90>] ? dev_deactivate_queue.constprop.0+0x50/0x50
[   16.046011]  [<c1052874>] __do_softirq+0xb4/0x240
[   16.046011]  [<c10527c0>] ? cpu_callback+0x1d0/0x1d0
[   16.046011]  [<c1004d10>] call_on_stack+0x40/0x50
[   16.046011]  <IRQ>  [<c1052b65>] ? irq_exit+0x95/0xa0
[   16.046011]  [<c103590b>] ? smp_apic_timer_interrupt+0x3b/0x50
[   16.046011]  [<c187a08d>] ? apic_timer_interrupt+0x2d/0x34
[   16.046011]  [<c100ae90>] ? arch_remove_reservations+0xd0/0xd0
[   16.046011]  [<c100aea8>] ? default_idle+0x18/0xe0
[   16.046011]  [<c100b4e1>] ? arch_cpu_idle+0x11/0x20
[   16.046011]  [<c1086137>] ? default_idle_call+0x27/0x40
[   16.046011]  [<c1086415>] ? cpu_startup_entry+0x265/0x300
[   16.046011]  [<c187498d>] ? rest_init+0x62/0x64
[   16.046011]  [<c1b65ab6>] ? start_kernel+0x35f/0x364
[   16.046011]  [<c1b652c4>] ? i386_start_kernel+0x92/0x96
[   16.046494] ---[ end trace 6de316e51927e87d ]---
[   16.050129] e1000e 0000:00:02.0 eth0: Reset adapter unexpectedly
```


## 正确的操作是这样的

```txt
#0  e1000e_send_msi (msix=true, core=0xfff4336d00) at ../hw/net/e1000e_core.c:2171
#1  e1000e_update_interrupt_state (core=0xfff4336d00) at ../hw/net/e1000e_core.c:2171
#2  0x000000aaab02b1e8 in e1000e_set_interrupt_cause (val=1048576, core=<optimized out>) at ../hw/net/e1000e_core.c:2194
#3  e1000e_set_ics (core=0xfff4336d00, index=<optimized out>, val=<optimized out>) at ../hw/net/e1000e_core.c:2453
#4  0x000000aaab0304ec in e1000e_core_write (core=0xfff4336d00, addr=<optimized out>, val=1048576, size=<optimized out>) at ../hw/net/e1000e_core.c:3257
#5  0x000000aaab1cf3a0 in memory_region_write_accessor (mr=0xfff4336930, addr=200, value=<optimized out>, size=<optimized out>, shift=<optimized out>, mask=<optimized out>, attrs=...) at ../softmmu/memory.c:489
#6  0x000000aaab1cdad4 in access_with_adjusted_size (addr=addr@entry=200, value=value@entry=0xfff5472508, size=size@entry=4, access_size_min=<optimized out>, access_size_max=<optimized out>, access_fn=access_fn@entry=0xaaab1cf284 <memory_region_write_accessor>, mr=mr@entry=0xfff43360
#7  0x000000aaab1d1a14 in memory_region_dispatch_write (mr=mr@entry=0xfff4336930, addr=addr@entry=200, data=<optimized out>, data@entry=1048576, op=op@entry=MO_32, attrs=...) at ../softmmu/memory.c:1500
#8  0x000000aaab23d524 in io_writex (env=0xfff557c8d0, mmu_idx=<optimized out>, val=1048576, addr=<optimized out>, retaddr=1097011489644, op=<optimized out>, iotlbentry=<optimized out>, iotlbentry=<optimized out>) at ../accel/tcg/cputlb.c:1423
#9  0x000000ff6afaecc8 in code_gen_buffer ()
#10 0x0000000000000000 in ?? ()
Backtrace stopped: frame did not save the PC
```

## 重新分析一下

lat
```plain
[   10.703769] IPv6: ADDRCONF(NETDEV_CHANGE): eth0: link becomes ready
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[QEMU:e1000e_set_ics:2454]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 0
[huxueshi:apic_send_msi:736] 41
udhcpc: broadcasting discover
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 0
[huxueshi:apic_send_msi:736] 41
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
udhcpc: broadcasting select for 10.0.2.15, server 10.0.2.2
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 0
[huxueshi:apic_send_msi:736] 41
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
```

bmbt:
```txt
[   10.790523] IPv6: ADDRCONF(NETDEV_CHANGE): eth0: link becomes ready
[QEMU:e1000e_set_tdt:2447]
[QEMU:e1000e_send_msi:2132]
[QEMU:msix_notify:482] 1
[huxueshi:apic_send_msi:631] 51
[QEMU:e1000e_set_ics:2460]
[QEMU:e1000e_send_msi:2132]
[QEMU:msix_notify:482] 0
[huxueshi:apic_send_msi:631] 41
udhcpc: broadcasting discover
[QEMU:e1000e_set_ics:2460]
[QEMU:e1000e_send_msi:2132]
[QEMU:msix_notify:482] 0
[huxueshi:apic_send_msi:631] 41
```

## diff 两边的数值

lat
```txt
[huxueshi:e1000e_core_write:3253] 3818 1
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[huxueshi:e1000e_core_write:3253] 3818 2
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[huxueshi:e1000e_core_write:3253] 3818 3
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[huxueshi:e1000e_core_write:3253] 3818 4
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[huxueshi:e1000e_core_write:3253] 3818 5
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
```

bmbt
```txt
[huxueshi:e1000e_core_write:3252] 3818 1
[QEMU:e1000e_set_tdt:2447]
[QEMU:e1000e_send_msi:2132]
[QEMU:msix_notify:482] 1
[huxueshi:apic_send_msi:631] 51
```

现在看到的现象就是，接受到数据之后，然后 guest 就开始调用 ics 了

## 为什么 read 两次啊
还是那个问题，为什么不去直接分析两个的操作难度。

所有的基本问题是什么?

```diff
2448c2448
< [huxueshi:e1000e_core_write:3255] 3800 7be3000
---
> [huxueshi:e1000e_core_write:3255] 3800 7b66000
2466,2475c2466,2475
< [huxueshi:e1000e_core_write:3255] 5c80 8efe47a7
< [huxueshi:e1000e_core_write:3255] 5c84 accbc283
< [huxueshi:e1000e_core_write:3255] 5c88 ce941ded
< [huxueshi:e1000e_core_write:3255] 5c8c 7b904924
< [huxueshi:e1000e_core_write:3255] 5c90 c0d1280c
< [huxueshi:e1000e_core_write:3255] 5c94 20c03681
< [huxueshi:e1000e_core_write:3255] 5c98 1ce0678e
< [huxueshi:e1000e_core_write:3255] 5c9c c5bfcb53
< [huxueshi:e1000e_core_write:3255] 5ca0 c438c145
< [huxueshi:e1000e_core_write:3255] 5ca4 3eeacd30
---
> [huxueshi:e1000e_core_write:3255] 5c80 45fb679b
> [huxueshi:e1000e_core_write:3255] 5c84 a24ad8a5
> [huxueshi:e1000e_core_write:3255] 5c88 2d9c952e
> [huxueshi:e1000e_core_write:3255] 5c8c def8ab04
> [huxueshi:e1000e_core_write:3255] 5c90 a43886d0
> [huxueshi:e1000e_core_write:3255] 5c94 9655f7e2
> [huxueshi:e1000e_core_write:3255] 5c98 8211c450
> [huxueshi:e1000e_core_write:3255] 5c9c 35e39e16
> [huxueshi:e1000e_core_write:3255] 5ca0 8a5e5cd
> [huxueshi:e1000e_core_write:3255] 5ca4 d722ed6e
2528c2528
< [huxueshi:e1000e_core_write:3255] 2800 7bd6000
---
> [huxueshi:e1000e_core_write:3255] 2800 7b74000
3305,3309d3304
< [huxueshi:e1000e_core_write:3255] 3818 2
< [huxueshi:e1000e_core_write:3255] 3818 3
< [huxueshi:e1000e_core_write:3255] 3818 4
< [huxueshi:e1000e_core_write:3255] 3818 5
< [huxueshi:e1000e_core_write:3255] 3818 6
3379,3380c3374,3375
< [huxueshi:e1000e_core_read:3281] 4080 6
< [huxueshi:e1000e_core_read:3281] 4090 1fc
---
> [huxueshi:e1000e_core_read:3281] 4080 0
> [huxueshi:e1000e_core_read:3281] 4090 0
3384c3379
< [huxueshi:e1000e_core_read:3281] 40f0 6
---
> [huxueshi:e1000e_core_read:3281] 40f0 0
3386c3381
< [huxueshi:e1000e_core_read:3281] 40d4 6
---
> [huxueshi:e1000e_core_read:3281] 40d4 0
```

## 测试一下时钟的频率是不是正确的
应该不是时钟的问题

## 实在是没有办法了，调试 guest 内核吧

```plain
[huxueshi:e1000e_core_write:3257] 5408 0 ip=c156022d
[huxueshi:e1000e_core_read:3283] 8 283
[huxueshi:e1000e_core_write:3257] 100 4008002 ip=c156022d
[huxueshi:e1000e_core_read:3283] 0 58140245
[huxueshi:e1000e_core_write:3257] 0 58140245 ip=c156022d
[huxueshi:e1000e_core_write:3257] 3818 1 ip=c155debc
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[huxueshi:e1000e_core_write:3257] 3818 2 ip=c155de8c
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[huxueshi:e1000e_core_write:3257] 3818 3 ip=c155de8c
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[huxueshi:e1000e_core_write:3257] 3818 4 ip=c155de8c
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[huxueshi:e1000e_core_write:3257] 3818 5 ip=c155de8c
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[huxueshi:e1000e_core_write:3257] 3818 6 ip=c155de8c
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[huxueshi:e1000e_core_read:3283] f00 8
[huxueshi:e1000e_core_write:3257] f00 28 ip=c1549774
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] 20 8210000 ip=c15551be
[huxueshi:e1000e_core_read:3283] 20 1821796d
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] f00 8 ip=c15487c0
[huxueshi:e1000e_core_read:3283] f00 8
[huxueshi:e1000e_core_write:3257] f00 28 ip=c1549774
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] 20 8210000 ip=c15551be
[huxueshi:e1000e_core_read:3283] 20 1821796d
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] f00 8 ip=c15487c0
[huxueshi:e1000e_core_read:3283] f00 8
[huxueshi:e1000e_core_write:3257] f00 28 ip=c1549774
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] 20 4360000 ip=c15551be
[huxueshi:e1000e_core_read:3283] 20 14360000
[huxueshi:e1000e_core_write:3257] 20 8300000 ip=c155521f
[huxueshi:e1000e_core_read:3283] 20 18303b60
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] f00 8 ip=c15487c0
[huxueshi:e1000e_core_read:3283] f00 8
[huxueshi:e1000e_core_write:3257] f00 28 ip=c1549774
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] 20 4360000 ip=c15551be
[huxueshi:e1000e_core_read:3283] 20 14360000
[huxueshi:e1000e_core_write:3257] 20 8310000 ip=c155521f
[huxueshi:e1000e_core_read:3283] 20 1831ac08
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] f00 8 ip=c15487c0
[huxueshi:e1000e_core_read:3283] f00 8
[huxueshi:e1000e_core_write:3257] f00 28 ip=c1549774
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] 20 4360000 ip=c15551be
[huxueshi:e1000e_core_read:3283] 20 14360000
[huxueshi:e1000e_core_write:3257] 20 8310000 ip=c155521f
[huxueshi:e1000e_core_read:3283] 20 1831ac08
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] f00 8 ip=c15487c0
[huxueshi:e1000e_core_read:3283] f00 8
[huxueshi:e1000e_core_write:3257] f00 28 ip=c1549774
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] 20 4360000 ip=c15551be
[huxueshi:e1000e_core_read:3283] 20 14360000
[huxueshi:e1000e_core_write:3257] 20 8310000 ip=c155521f
[huxueshi:e1000e_core_read:3283] 20 1831ac08
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] f00 8 ip=c15487c0
[huxueshi:e1000e_core_read:3283] f00 8
[huxueshi:e1000e_core_write:3257] f00 28 ip=c1549774
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] 20 82a0000 ip=c15551be
[huxueshi:e1000e_core_read:3283] 20 182a3c00
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] f00 8 ip=c15487c0
[huxueshi:e1000e_core_read:3283] 4000 0
[huxueshi:e1000e_core_read:3283] 4074 0
[huxueshi:e1000e_core_read:3283] 4088 0
[huxueshi:e1000e_core_read:3283] 408c 0
[huxueshi:e1000e_core_read:3283] 4078 0
[huxueshi:e1000e_core_read:3283] 407c 0
[huxueshi:e1000e_core_read:3283] 40ac 0
[huxueshi:e1000e_core_read:3283] 4010 0
[huxueshi:e1000e_core_read:3283] 4048 0
[huxueshi:e1000e_core_read:3283] 404c 0
[huxueshi:e1000e_core_read:3283] 4050 0
[huxueshi:e1000e_core_read:3283] 4054 0
[huxueshi:e1000e_core_read:3283] 4080 6
[huxueshi:e1000e_core_read:3283] 4090 1fc
[huxueshi:e1000e_core_read:3283] 4094 0
[huxueshi:e1000e_core_read:3283] 40a0 0
[huxueshi:e1000e_core_read:3283] 40a4 0
[huxueshi:e1000e_core_read:3283] 40f0 6
[huxueshi:e1000e_core_read:3283] 40f4 0
[huxueshi:e1000e_core_read:3283] 40d4 6
[huxueshi:e1000e_core_read:3283] 4004 0
[huxueshi:e1000e_core_read:3283] 400c 0
[huxueshi:e1000e_core_read:3283] 403c 0
[huxueshi:e1000e_core_read:3283] 40f8 0
[huxueshi:e1000e_core_read:3283] 40fc 0
[huxueshi:e1000e_core_read:3283] 40bc 0
[huxueshi:e1000e_core_read:3283] 40b4 0
[huxueshi:e1000e_core_read:3283] 40b8 0
[huxueshi:e1000e_core_write:3257] c8 100000 ip=c15620a6
[QEMU:e1000e_set_ics:2454]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 0
[huxueshi:apic_send_msi:736] 41
[huxueshi:e1000e_core_write:3257] e8 1b2 ip=c155b7d0
[huxueshi:e1000e_core_write:3257] d0 100000 ip=c15624b8
[huxueshi:e1000e_core_write:3257] 3820 80000008 ip=c155f0a0
[huxueshi:e1000e_core_write:3257] 2820 80000020 ip=c155f0a0
[huxueshi:e1000e_core_read:3283] 8 283
[huxueshi:e1000e_core_write:3257] 3820 80000008 ip=c155f135
[huxueshi:e1000e_core_write:3257] 2820 80000020 ip=c155f135
[huxueshi:e1000e_core_read:3283] 8 283
[huxueshi:e1000e_core_read:3283] f00 8
[huxueshi:e1000e_core_write:3257] f00 28 ip=c1549774
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] 20 4360000 ip=c15551be
[huxueshi:e1000e_core_read:3283] 20 14360000
[huxueshi:e1000e_core_write:3257] 20 8350000 ip=c155521f
[huxueshi:e1000e_core_read:3283] 20 18350000
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] f00 8 ip=c15487c0
udhcpc: broadcasting discover
[huxueshi:e1000e_core_write:3257] 3818 7 ip=c155de8c
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 0
[huxueshi:apic_send_msi:736] 41
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[huxueshi:e1000e_core_write:3257] e8 117 ip=c155b7d0
[huxueshi:e1000e_core_write:3257] d0 100000 ip=c15624b8
udhcpc: broadcasting select for 10.0.2.15, server 10.0.2.2
[huxueshi:e1000e_core_write:3257] 3818 8 ip=c155de8c
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 0
[huxueshi:apic_send_msi:736] 41
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[huxueshi:e1000e_core_write:3257] e8 cd ip=c155b7d0
[huxueshi:e1000e_core_write:3257] 2818 0 ip=c155e655
[huxueshi:e1000e_core_write:3257] d0 100000 ip=c15624b8
udhcpc: lease of 10.0.2.15 obtained from 10.0.2.2, lease time 86400
[   13.354459] udhcpc (941) used greatest stack depth: 6528 bytes left
run-parts: /etc/network/if-up.d: No such file or directory
ifup: can't open '/var/run/ifstate.new': No such file or directory
/bin/sh: can't access tty; job control turned off
/ # [huxueshi:e1000e_core_read:3283] 4000 0
[huxueshi:e1000e_core_read:3283] 4074 2
[huxueshi:e1000e_core_read:3283] 4088 4a4
[huxueshi:e1000e_core_read:3283] 408c 0
[huxueshi:e1000e_core_read:3283] 4078 2
[huxueshi:e1000e_core_read:3283] 407c 0
[huxueshi:e1000e_core_read:3283] 40ac 0
[huxueshi:e1000e_core_read:3283] 4010 0
[huxueshi:e1000e_core_read:3283] 4048 0
[huxueshi:e1000e_core_read:3283] 404c 0
[huxueshi:e1000e_core_read:3283] 4050 0
[huxueshi:e1000e_core_read:3283] 4054 0
[huxueshi:e1000e_core_read:3283] 4080 2
[huxueshi:e1000e_core_read:3283] 4090 4a8
[huxueshi:e1000e_core_read:3283] 4094 0
[huxueshi:e1000e_core_read:3283] 40a0 0
[huxueshi:e1000e_core_read:3283] 40a4 0
[huxueshi:e1000e_core_read:3283] 40f0 0
[huxueshi:e1000e_core_read:3283] 40f4 2
[huxueshi:e1000e_core_read:3283] 40d4 2
[huxueshi:e1000e_core_read:3283] 4004 0
[huxueshi:e1000e_core_read:3283] 400c 0
[huxueshi:e1000e_core_read:3283] 403c 0
[huxueshi:e1000e_core_read:3283] 40f8 0
[huxueshi:e1000e_core_read:3283] 40fc 0
[huxueshi:e1000e_core_read:3283] 40bc 0
[huxueshi:e1000e_core_read:3283] 40b4 0
[huxueshi:e1000e_core_read:3283] 40b8 0
[huxueshi:e1000e_core_write:3257] c8 100000 ip=c15620a6
[QEMU:e1000e_set_ics:2454]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 0
[huxueshi:apic_send_msi:736] 41
[huxueshi:e1000e_core_write:3257] e8 c3 ip=c155b7d0
[huxueshi:e1000e_core_write:3257] d0 100000 ip=c15624b8
[huxueshi:e1000e_core_write:3257] 3820 80000008 ip=c155f0a0
[huxueshi:e1000e_core_write:3257] 2820 80000020 ip=c155f0a0
[huxueshi:e1000e_core_read:3283] 8 283
[huxueshi:e1000e_core_write:3257] 3820 80000008 ip=c155f0a0
[huxueshi:e1000e_core_write:3257] 2820 80000020 ip=c155f0a0
[huxueshi:e1000e_core_read:3283] 8 283
[huxueshi:e1000e_core_read:3283] f00 8
[huxueshi:e1000e_core_write:3257] f00 28 ip=c1549774
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] 20 4360000 ip=c15551be
[huxueshi:e1000e_core_read:3283] 20 14360000
[huxueshi:e1000e_core_write:3257] 20 8350000 ip=c155521f
[huxueshi:e1000e_core_read:3283] 20 18350000
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] f00 8 ip=c15487c0
[huxueshi:e1000e_core_write:3257] 3818 9 ip=c155de8c
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[huxueshi:e1000e_core_read:3283] 4000 0
[huxueshi:e1000e_core_read:3283] 4074 0
[huxueshi:e1000e_core_read:3283] 4088 0
[huxueshi:e1000e_core_read:3283] 408c 0
[huxueshi:e1000e_core_read:3283] 4078 0
[huxueshi:e1000e_core_read:3283] 407c 0
[huxueshi:e1000e_core_read:3283] 40ac 0
[huxueshi:e1000e_core_read:3283] 4010 0
[huxueshi:e1000e_core_read:3283] 4048 0
[huxueshi:e1000e_core_read:3283] 404c 0
[huxueshi:e1000e_core_read:3283] 4050 0
[huxueshi:e1000e_core_read:3283] 4054 0
[huxueshi:e1000e_core_read:3283] 4080 1
[huxueshi:e1000e_core_read:3283] 4090 4ee
[huxueshi:e1000e_core_read:3283] 4094 0
[huxueshi:e1000e_core_read:3283] 40a0 0
[huxueshi:e1000e_core_read:3283] 40a4 0
[huxueshi:e1000e_core_read:3283] 40f0 1
[huxueshi:e1000e_core_read:3283] 40f4 0
[huxueshi:e1000e_core_read:3283] 40d4 1
[huxueshi:e1000e_core_read:3283] 4004 0
[huxueshi:e1000e_core_read:3283] 400c 0
[huxueshi:e1000e_core_read:3283] 403c 0
[huxueshi:e1000e_core_read:3283] 40f8 0
[huxueshi:e1000e_core_read:3283] 40fc 0
[huxueshi:e1000e_core_read:3283] 40bc 0
[huxueshi:e1000e_core_read:3283] 40b4 0
[huxueshi:e1000e_core_read:3283] 40b8 0
[huxueshi:e1000e_core_write:3257] c8 100000 ip=c15620a6
[QEMU:e1000e_set_ics:2454]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 0
[huxueshi:apic_send_msi:736] 41
[huxueshi:e1000e_core_write:3257] d0 100000 ip=c15624b8
[huxueshi:e1000e_core_write:3257] 3820 80000008 ip=c155f0a0
[huxueshi:e1000e_core_write:3257] 2820 80000020 ip=c155f0a0
[huxueshi:e1000e_core_read:3283] 8 283
[huxueshi:e1000e_core_write:3257] 3820 80000008 ip=c155f0a0
[huxueshi:e1000e_core_write:3257] 2820 80000020 ip=c155f0a0
[huxueshi:e1000e_core_read:3283] 8 283
[huxueshi:e1000e_core_read:3283] f00 8
[huxueshi:e1000e_core_write:3257] f00 28 ip=c1549774
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] 20 4360000 ip=c15551be
[huxueshi:e1000e_core_read:3283] 20 14360000
[huxueshi:e1000e_core_write:3257] 20 8350000 ip=c155521f
[huxueshi:e1000e_core_read:3283] 20 18350000
[huxueshi:e1000e_core_read:3283] f00 28
[huxueshi:e1000e_core_write:3257] f00 8 ip=c15487c0
QEMU: Terminated
```

### 检查一下代码的 backtrace 看看
```txt
[   10.746670] e1000e: [KERNEL:e1000_xmit_frame:5816] 1
[huxueshi:e1000e_core_write:3257] 3818 1 ip=c164fd33
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
nn[   11.520657] e1000e: [KERNEL:e1000_xmit_frame:5816] 1
[huxueshi:e1000e_core_write:3257] 3818 2 ip=c164fd0f
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[   11.632579] e1000e: [KERNEL:e1000_xmit_frame:5816] 1
[huxueshi:e1000e_core_write:3257] 3818 3 ip=c164fd0f
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[   12.522918] e1000e: [KERNEL:e1000_xmit_frame:5816] 1
[huxueshi:e1000e_core_write:3257] 3818 4 ip=c164fd0f
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[   12.524488] e1000e: [KERNEL:e1000_xmit_frame:5816] 1
[huxueshi:e1000e_core_write:3257] 3818 5 ip=c164fd0f
```
应该连续接受到数次的传输才对的啊

## 是不是因为 option rom 的模拟过于随意了

正确的:
```txt
[huxueshi:e1000e_core_write:3257] 3818 1 ip=c164fe71
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[   10.711040] e1000e: [huxueshi:e1000_xmit_frame:5848] ------------------------
[   10.711040] CPU: 0 PID: 0 Comm: swapper/0 Not tainted 4.4.142+ #34
[   10.711040] Hardware name: QEMU Standard PC (i440FX + PIIX, 1996), BIOS rel-1.14.0-14-g748d619-dirty-20220316_232631-maritns3-pc 04/01/2014
[   10.711040]  00200286 00200286 c7821c4c c12e696f c7b74000 c7b74000 c7821cc8 c164fe9a
[   10.711040]  c1a121c8 c18f0eec 000016d8 00000001 00000000 00000000 00000000 00000000
[   10.711040]  00000000 c794d064 c1008ec0 c794d000 00006000 0000005a c7059cc0 00001ccc
[   10.711040] Call Trace:
[   10.711040]  [<c12e696f>] dump_stack+0x4b/0x6c
[   10.711040]  [<c164fe9a>] e1000_xmit_frame.cold+0x370/0x391
[   10.711040]  [<c1008ec0>] ? nommu_map_sg+0xa0/0xa0
[   10.711040]  [<c16b9566>] dev_hard_start_xmit+0x226/0x3b0
[   10.711040]  [<c16b9070>] ? validate_xmit_skb.isra.0.part.0+0x10/0x290
[   10.711040]  [<c16d7db6>] sch_direct_xmit+0xb6/0x190
[   10.711040]  [<c16b9ada>] __dev_queue_xmit+0x30a/0x580
[   10.711040]  [<c18791b7>] ? _raw_write_unlock_bh+0x17/0x20
[   10.711040]  [<c16b9d5e>] dev_queue_xmit+0xe/0x10
[   10.711040]  [<c16c1d98>] neigh_resolve_output+0xf8/0x1b0
[   10.711040]  [<c16d6bb0>] ? eth_mac_addr+0x50/0x50
[   10.711040]  [<c176fe9a>] ip6_finish_output2+0x17a/0x600
[   10.711040]  [<c17a668f>] ? ipv6_confirm+0x4f/0xf0
[   10.711040]  [<c17a6640>] ? ipv6_net_init+0x70/0x70
[   10.711040]  [<c16e59c9>] ? nf_iterate+0x69/0x80
[   10.711040]  [<c177293b>] ip6_finish_output+0x7b/0xf0
[   10.711040]  [<c16e5a42>] ? nf_hook_slow+0x62/0xb0
[   10.711040]  [<c17729e4>] ip6_output+0x34/0x150
[   10.711040]  [<c17728c0>] ? ip6_fragment+0xbd0/0xbd0
[   10.711040]  [<c17729b0>] ? ip6_finish_output+0xf0/0xf0
[   10.711040]  [<c179212e>] NF_HOOK_THRESH.constprop.0+0x2e/0x90
[   10.711040]  [<c17729b0>] ? ip6_finish_output+0xf0/0xf0
[   10.711040]  [<c1790aa0>] ? ipv6_icmp_sysctl_init+0x30/0x30
[   10.711040]  [<c179237f>] mld_sendpack+0x1ef/0x300
[   10.711040]  [<c1793663>] mld_ifc_timer_expire+0x193/0x2b0
[   10.711040]  [<c10a188e>] call_timer_fn+0x2e/0x120
[   10.711040]  [<c17934d0>] ? igmp6_timer_handler+0x60/0x60
[   10.711040]  [<c10a1d7c>] run_timer_softirq+0x11c/0x230
[   10.711040]  [<c17934d0>] ? igmp6_timer_handler+0x60/0x60
[   10.711040]  [<c1052874>] __do_softirq+0xb4/0x240
[   10.711040]  [<c10527c0>] ? cpu_callback+0x1d0/0x1d0
[   10.711040]  [<c1004d10>] call_on_stack+0x40/0x50
[   10.711040]  <IRQ>  [<c1052b65>] ? irq_exit+0x95/0xa0
[   10.711040]  [<c103590b>] ? smp_apic_timer_interrupt+0x3b/0x50
[   10.711040]  [<c187a10d>] ? apic_timer_interrupt+0x2d/0x34
[   10.711040]  [<c100ae90>] ? arch_remove_reservations+0xd0/0xd0
[   10.711040]  [<c100aea8>] ? default_idle+0x18/0xe0
[   10.711040]  [<c100b4e1>] ? arch_cpu_idle+0x11/0x20
[   10.711040]  [<c1086137>] ? default_idle_call+0x27/0x40
[   10.711040]  [<c1086415>] ? cpu_startup_entry+0x265/0x300
[   10.711040]  [<c18749fd>] ? rest_init+0x62/0x64
[   10.711040]  [<c1b65ab6>] ? start_kernel+0x35f/0x364
[   10.711040]  [<c1b652c4>] ? i386_start_kernel+0x92/0x96
[   10.711040] e1000e: [huxueshi:e1000_xmit_frame:5850] ------------------------
[   11.062653] e1000e: [KERNEL:e1000_xmit_frame:5816] 1
[huxueshi:e1000e_core_write:3257] 3818 2 ip=c164fd3f
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[   11.063234] e1000e: [huxueshi:e1000_xmit_frame:5848] ------------------------
[   11.063234] CPU: 0 PID: 0 Comm: swapper/0 Not tainted 4.4.142+ #34
[   11.063234] Hardware name: QEMU Standard PC (i440FX + PIIX, 1996), BIOS rel-1.14.0-14-g748d619-dirty-20220316_232631-maritns3-pc 04/01/2014
[   11.063234]  00200286 00200286 c7821c7c c12e696f c7b74000 c7b74000 c7821cf8 c164fe9a
[   11.063234]  c1a121c8 c18f0eec 000016d8 00000001 c7821cc1 c7071838 c7071f40 c7059cc0
[   11.063234]  c7821d2c c794d064 c1008ec0 c794d000 00006000 0000005a c7059cc0 00011cfc
[   11.063234] Call Trace:
[   11.063234]  [<c12e696f>] dump_stack+0x4b/0x6c
[   11.063234]  [<c164fe9a>] e1000_xmit_frame.cold+0x370/0x391
[   11.063234]  [<c1008ec0>] ? nommu_map_sg+0xa0/0xa0
[   11.063234]  [<c16b9566>] dev_hard_start_xmit+0x226/0x3b0
[   11.063234]  [<c16b9070>] ? validate_xmit_skb.isra.0.part.0+0x10/0x290
[   11.063234]  [<c16d7db6>] sch_direct_xmit+0xb6/0x190
[   11.063234]  [<c16b9ada>] __dev_queue_xmit+0x30a/0x580
[   11.063234]  [<c1295d2e>] ? selinux_ip_postroute+0x15e/0x390
[   11.063234]  [<c187916f>] ? _raw_read_unlock_bh+0x1f/0x30
[   11.063234]  [<c1794c4a>] ? ipv6_chk_mcast_addr+0x14a/0x160
[   11.063234]  [<c16b9d5e>] dev_queue_xmit+0xe/0x10
[   11.063234]  [<c177007f>] ip6_finish_output2+0x35f/0x600
[   11.063234]  [<c17a668f>] ? ipv6_confirm+0x4f/0xf0
[   11.063234]  [<c17a6640>] ? ipv6_net_init+0x70/0x70
[   11.063234]  [<c16e59c9>] ? nf_iterate+0x69/0x80
[   11.063234]  [<c177293b>] ip6_finish_output+0x7b/0xf0
[   11.063234]  [<c16e5a42>] ? nf_hook_slow+0x62/0xb0
[   11.063234]  [<c17729e4>] ip6_output+0x34/0x150
[   11.063234]  [<c17728c0>] ? ip6_fragment+0xbd0/0xbd0
[   11.063234]  [<c17729b0>] ? ip6_finish_output+0xf0/0xf0
[   11.063234]  [<c179212e>] NF_HOOK_THRESH.constprop.0+0x2e/0x90
[   11.063234]  [<c17729b0>] ? ip6_finish_output+0xf0/0xf0
[   11.063234]  [<c1790aa0>] ? ipv6_icmp_sysctl_init+0x30/0x30
[   11.063234]  [<c179237f>] mld_sendpack+0x1ef/0x300
[   11.063234]  [<c1793663>] mld_ifc_timer_expire+0x193/0x2b0
[   11.063234]  [<c10a188e>] call_timer_fn+0x2e/0x120
[   11.063234]  [<c17934d0>] ? igmp6_timer_handler+0x60/0x60
[   11.063234]  [<c10a1d7c>] run_timer_softirq+0x11c/0x230
[   11.063234]  [<c17934d0>] ? igmp6_timer_handler+0x60/0x60
[   11.063234]  [<c1052874>] __do_softirq+0xb4/0x240
[   11.063234]  [<c10527c0>] ? cpu_callback+0x1d0/0x1d0
[   11.063234]  [<c1004d10>] call_on_stack+0x40/0x50
[   11.063234]  <IRQ>  [<c1052b65>] ? irq_exit+0x95/0xa0
[   11.063234]  [<c103590b>] ? smp_apic_timer_interrupt+0x3b/0x50
[   11.063234]  [<c187a10d>] ? apic_timer_interrupt+0x2d/0x34
[   11.063234]  [<c100ae90>] ? arch_remove_reservations+0xd0/0xd0
[   11.063234]  [<c100aea8>] ? default_idle+0x18/0xe0
[   11.063234]  [<c100b4e1>] ? arch_cpu_idle+0x11/0x20
[   11.063234]  [<c1086137>] ? default_idle_call+0x27/0x40
[   11.063234]  [<c1086415>] ? cpu_startup_entry+0x265/0x300
```

错误的

```txt
[   11.031685] e1000e: [KERNEL:e1000_xmit_frame:5816] 1
[huxueshi:e1000e_core_write:3254] 3818 1
[QEMU:e1000e_set_tdt:2447]
[QEMU:e1000e_send_msi:2132]
[QEMU:msix_notify:482] 1
[huxueshi:apic_send_msi:631] 51
[   11.035630] e1000e: [huxueshi:e1000_xmit_frame:5848] ------------------------
[   11.037582] CPU: 0 PID: 4 Comm: kworker/0:0 Not tainted 4.4.142+ #34
[   11.037582] Hardware name: Bochs Bochs, BIOS Bochs 01/01/2011
[   11.037582] Workqueue: ipv6_addrconf addrconf_dad_work
[   11.037582]  00000286 00000286 c7821c4c c12e696f c0044000 c0044000 c7821cc8 c164fe9a
[   11.037582]  c1a121c8 c18f0eec 000016d8 00000001 c0044000 c7b86300 00000046 c0044000
[   11.037582]  c7b86300 c0093064 c1008ec0 c0093000 00006000 0000005a c7b86300 0000a10d
[   11.037582] Call Trace:
[   11.037582]  [<c12e696f>] dump_stack+0x4b/0x6c
[   11.037582]  [<c164fe9a>] e1000_xmit_frame.cold+0x370/0x391
[   11.037582]  [<c1008ec0>] ? nommu_map_sg+0xa0/0xa0
[   11.037582]  [<c16b9566>] dev_hard_start_xmit+0x226/0x3b0
[   11.037582]  [<c16d7db6>] sch_direct_xmit+0xb6/0x190
[   11.037582]  [<c16b9ada>] __dev_queue_xmit+0x30a/0x580
[   11.037582]  [<c103590b>] ? smp_apic_timer_interrupt+0x3b/0x50
[   11.037582]  [<c16d6ac0>] ? eth_header_parse+0x30/0x30
[   11.037582]  [<c16b9d5e>] dev_queue_xmit+0xe/0x10
[   11.037582]  [<c16c1d98>] neigh_resolve_output+0xf8/0x1b0
[   11.037582]  [<c16d6bb0>] ? eth_mac_addr+0x50/0x50
[   11.037582]  [<c176fe9a>] ip6_finish_output2+0x17a/0x600
[   11.037582]  [<c177293b>] ip6_finish_output+0x7b/0xf0
[   11.037582]  [<c16e5a42>] ? nf_hook_slow+0x62/0xb0
[   11.037582]  [<c17729e4>] ip6_output+0x34/0x150
[   11.037582]  [<c17728c0>] ? ip6_fragment+0xbd0/0xbd0
[   11.037582]  [<c17729b0>] ? ip6_finish_output+0xf0/0xf0
[   11.037582]  [<c179212e>] NF_HOOK_THRESH.constprop.0+0x2e/0x90
[   11.037582]  [<c17729b0>] ? ip6_finish_output+0xf0/0xf0
[   11.037582]  [<c1790aa0>] ? ipv6_icmp_sysctl_init+0x30/0x30
[   11.037582]  [<c179237f>] mld_sendpack+0x1ef/0x300
[   11.037582]  [<c1793663>] mld_ifc_timer_expire+0x193/0x2b0
[   11.037582]  [<c10a188e>] call_timer_fn+0x2e/0x120
[   11.037582]  [<c10702b2>] ? ttwu_do_wakeup+0x12/0x110
[   11.037582]  [<c17934d0>] ? igmp6_timer_handler+0x60/0x60
[   11.037582]  [<c10a1d7c>] run_timer_softirq+0x11c/0x230
[   11.037582]  [<c17934d0>] ? igmp6_timer_handler+0x60/0x60
[   11.037582]  [<c1052874>] __do_softirq+0xb4/0x240
[   11.037582]  [<c10527c0>] ? cpu_callback+0x1d0/0x1d0
[   11.037582]  [<c1004d10>] call_on_stack+0x40/0x50
[   11.037582]  <IRQ>  [<c1052b65>] ? irq_exit+0x95/0xa0
[   11.037582]  [<c103590b>] ? smp_apic_timer_interrupt+0x3b/0x50
[   11.037582]  [<c187a10d>] ? apic_timer_interrupt+0x2d/0x34
[   11.037582]  [<c12f5801>] ? prandom_seed+0x61/0x70
[   11.037582]  [<c1779ecd>] ? addrconf_dad_work+0x5d/0x300
[   11.037582]  [<c1062ea2>] ? process_one_work+0x192/0x3a0
[   11.037582]  [<c10631d6>] ? worker_thread+0x126/0x470
[   11.037582]  [<c1067b5a>] ? kthread+0xaa/0xc0
[   11.037582]  [<c10630b0>] ? process_one_work+0x3a0/0x3a0
[   11.037582]  [<c1875e01>] ? __schedule+0x211/0x750
[   11.037582]  [<c18796a0>] ? ret_from_kernel_thread+0x38/0x4c
[   11.037582]  [<c1067ab0>] ? kthread_worker_fn+0x140/0x140
[   11.037843] e1000e: [huxueshi:e1000_xmit_frame:5850] ------------------------
```

## guest 真的接受到了 51 号中断吗
似乎是没有的，那么为什么啊?

正确的操作:
```txt
[huxueshi:e1000e_core_write:3257] 100 4008002 ip=c155fff8
[huxueshi:e1000e_core_read:3283] 0 58140245
[huxueshi:e1000e_core_write:3257] 0 58140245 ip=c155fff8
[   10.741682] e1000e: [KERNEL:e1000_xmit_frame:5819] 1
[huxueshi:e1000e_core_write:3257] 3818 1 ip=c164fe20
[QEMU:e1000e_set_tdt:2441]
[QEMU:e1000e_send_msi:2125]
[QEMU:msix_notify:491] 1
[huxueshi:apic_send_msi:736] 51
[   10.742741] e1000e: [huxueshi:e1000_intr_msix_tx:1948]
[   10.744833] e1000e: [huxueshi:e1000_xmit_frame:5851] ------------------------
[   10.745977] CPU: 0 PID: 0 Comm: swapper/0 Not tainted 4.4.142+ #35
[   10.746154] Hardware name: QEMU Standard PC (i440FX + PIIX, 1996), BIOS rel-1.14.0-14-g748d619-dirty-20220316_232631-maritns3-pc 04/01/2014
[   10.746154]  00200286 00200286 c7821c4c c12e696f c0078000 c0078000 c7821cc8 c164fe49
[   10.746154]  c1a12208 c18f0eec 000016db 00000001 00000000 00000002 c7dae600 c7dae678
[   10.746154]  c7dae698 c7957064 c1008ec0 c7957000 00006000 0000005a c7bfbcc0 00000002
[   10.746154] Call Trace:
[   10.746154]  [<c12e696f>] dump_stack+0x4b/0x6c
[   10.746154]  [<c164fe49>] e1000_xmit_frame.cold+0x370/0x391
[   10.746154]  [<c1008ec0>] ? nommu_map_sg+0xa0/0xa0
[   10.746154]  [<c16b9596>] dev_hard_start_xmit+0x226/0x3b0
```

bmbt 的操作
```txt
[31872.288276] e1000e: [KERNEL:e1000_xmit_frame:5816] 1
[huxueshi:e1000e_core_write:3254] 3818 1
[QEMU:e1000e_set_tdt:2447]
[QEMU:e1000e_send_msi:2132]
[QEMU:msix_notify:482] 1
[huxueshi:apic_send_msi:631] 51
[31872.289311] e1000e: [huxueshi:e1000_xmit_frame:5848] ------------------------
[31872.289999] CPU: 0 PID: 0 Comm: swapper/0 Tainted: G        W       4.4.142+ #34
[31872.290936] Hardware name: Bochs Bochs, BIOS Bochs 01/01/2011
[31872.290936]  00200286 00200286 c7821c7c c12e696f c0044000 c0044000 c7821cf8 c164fe9a
[31872.290936]  c1a121c8 c18f0eec 000016d8 00000001 c7821cc1 c7bcb838 00000000 c7b73240
[31872.290936]  c7821d2c c0093064 c1008ec0 c0093000 00006000 0000005a c7b73240 00001cfc
[31872.290936] Call Trace:
[31872.290936]  [<c12e696f>] dump_stack+0x4b/0x6c
[31872.290936]  [<c164fe9a>] e1000_xmit_frame.cold+0x370/0x391
[31872.290936]  [<c1008ec0>] ? nommu_map_sg+0xa0/0xa0
[31872.290936]  [<c16b9566>] dev_hard_start_xmit+0x226/0x3b0
[31872.290936]  [<c16b9070>] ? validate_xmit_skb.isra.0.part.0+0x10/0x290
[31872.290936]  [<c16d7db6>] sch_direct_xmit+0xb6/0x190
[31872.290936]  [<c16b9ada>] __dev_queue_xmit+0x30a/0x580
[31872.290936]  [<c1295d2e>] ? selinux_ip_postroute+0x15e/0x390
[31872.290936]  [<c187916f>] ? _raw_read_unlock_bh+0x1f/0x30
[31872.290936]  [<c1794c4a>] ? ipv6_chk_mcast_addr+0x14a/0x160
[31872.290936]  [<c16b9d5e>] dev_queue_xmit+0xe/0x10
[31872.290936]  [<c177007f>] ip6_finish_output2+0x35f/0x600
[31872.290936]  [<c17a668f>] ? ipv6_confirm+0x4f/0xf0
[31872.290936]  [<c17a6640>] ? ipv6_net_init+0x70/0x70
[31872.290936]  [<c16e59c9>] ? nf_iterate+0x69/0x80
```

实际上，是注入过的:
```txt
[    8.357236] e1000e: [KERNEL:e1000_xmit_frame:5819] 1
[huxueshi:e1000e_core_write:3254] 3818 1
[QEMU:e1000e_set_tdt:2447]
[QEMU:e1000e_send_msi:2132]
[QEMU:msix_notify:482] 1
[huxueshi:apic_send_msi:636] 51
[huxueshi:apic_set_irq:311] 51
[    8.361235] e1000e: [huxueshi:e1000_intr_msix_tx:1948]
[    8.362235] e1000e: [huxueshi:e1000_xmit_frame:5851] ------------------------
[    8.363235] CPU: 0 PID: 4 Comm: kworker/0:0 Not tainted 4.4.142+ #35
[    8.363235] Hardware name: Bochs Bochs, BIOS Bochs 01/01/2011
[    8.363235] Workqueue: ipv6_addrconf addrconf_dad_work
[    8.363235]  00000286 00000286 c7821c4c c12e696f c003c000 c003c000 c7821cc8 c164fe49
```
现在，是不是就是网络无法发布的原因啊

## e1000 接受的东西总是相同的，那么中断也是可以正确反馈的

### watch dog 的报错是什么含义

- [ ] 发送了，但是无法接受数据?
  - [ ] 但是，我们总是发送数据了之后，总是可以立刻接受啊

那么这个 watch dog 的报错是什么意思啊?
```txt
[   10.820043] [huxueshi:dev_hard_start_xmit:2776]
[   10.821175] [huxueshi:xmit_one:2753]
[   10.822217] [huxueshi:netdev_start_xmit:3752]
[   10.827033] e1000e: [KERNEL:e1000_xmit_frame:5821] 1
```

```txt
[   10.978847]  [<c12e696f>] dump_stack+0x4b/0x6c
[   10.978847]  [<c186da2e>] dev_hard_start_xmit.cold+0x3f/0x39f
[   10.978847]  [<c16d79e6>] sch_direct_xmit+0xb6/0x190
[   10.978847]  [<c16b975a>] __dev_queue_xmit+0x30a/0x580
[   10.978847]  [<c103590b>] ? smp_apic_timer_interrupt+0x3b/0x50
[   10.978847]  [<c16d67e0>] ? eth_mac_addr+0x50/0x50
[   10.978847]  [<c16b99de>] dev_queue_xmit+0xe/0x10
[   10.978847]  [<c16c1a18>] neigh_resolve_output+0xf8/0x1b0
[   10.978847]  [<c16d67e0>] ? eth_mac_addr+0x50/0x50
[   10.978847]  [<c176faca>] ip6_finish_output2+0x17a/0x600
[   10.978847]  [<c177256b>] ip6_finish_output+0x7b/0xf0
[   10.978847]  [<c16e5672>] ? nf_hook_slow+0x62/0xb0
[   10.978847]  [<c1772614>] ip6_output+0x34/0x150
[   10.978847]  [<c17724f0>] ? ip6_fragment+0xbd0/0xbd0
[   10.978847]  [<c17725e0>] ? ip6_finish_output+0xf0/0xf0
[   10.978847]  [<c1791d5e>] NF_HOOK_THRESH.constprop.0+0x2e/0x90
[   10.978847]  [<c17725e0>] ? ip6_finish_output+0xf0/0xf0
[   10.978847]  [<c17906d0>] ? ipv6_icmp_sysctl_init+0x30/0x30
[   10.978847]  [<c1791faf>] mld_sendpack+0x1ef/0x300
[   10.978847]  [<c1793293>] mld_ifc_timer_expire+0x193/0x2b0
[   10.978847]  [<c10a188e>] call_timer_fn+0x2e/0x120
[   10.978847]  [<c10702b2>] ? ttwu_do_wakeup+0x12/0x110
[   10.978847]  [<c1793100>] ? igmp6_timer_handler+0x60/0x60
[   10.978847]  [<c10a1d7c>] run_timer_softirq+0x11c/0x230
[   10.978847]  [<c1793100>] ? igmp6_timer_handler+0x60/0x60
[   10.978847]  [<c1052874>] __do_softirq+0xb4/0x240
[   10.978847]  [<c10527c0>] ? cpu_callback+0x1d0/0x1d0
[   10.978847]  [<c1004d10>] call_on_stack+0x40/0x50
[   10.978847]  <IRQ>  [<c1051e35>] ? do_softirq.part.0+0x25/0x30
[   10.978847]  [<c1051ebf>] ? __local_bh_enable_ip+0x7f/0x90
[   10.978847]  [<c187919f>] ? _raw_read_unlock_bh+0x1f/0x30
[   10.978847]  [<c1779b7b>] ? addrconf_dad_work+0xdb/0x300
[   10.978847]  [<c1062ea2>] ? process_one_work+0x192/0x3a0
[   10.978847]  [<c10631d6>] ? worker_thread+0x126/0x470
[   10.978847]  [<c1067b5a>] ? kthread+0xaa/0xc0
[   10.978847]  [<c10630b0>] ? process_one_work+0x3a0/0x3a0
[   10.978847]  [<c1875e31>] ? __schedule+0x211/0x750
[   10.978847]  [<c18796a0>] ? ret_from_kernel_thread+0x38/0x4c
[   10.978847]  [<c1067ab0>] ? kthread_worker_fn+0x140/0x140
```

## 操作方法

bmbt:
```txt
[   10.801774] [huxueshi:mld_ifc_start_timer:1014] 0
[   10.805497] [huxueshi:mld_ifc_timer_expire:2432]
[   10.813703] [huxueshi:mld_sendpack:1616]
[   10.851795] [huxueshi:ip6_finish_output2:113]
[   10.860264] [huxueshi:dev_hard_start_xmit:2776]
[   10.861403] [huxueshi:xmit_one:2753]
[   10.862530] [huxueshi:netdev_start_xmit:3752]
[   10.867258] e1000e: [KERNEL:e1000_xmit_frame:5821] 1
[huxueshi:apic_send_msi:636] 51
[huxueshi:apic_set_irq:311] 51
[   10.871212] e1000e: [huxueshi:e1000_intr_msix_tx:1948]
[   10.874562] [huxueshi:mld_ifc_start_timer:1014] 17b
[   11.257195] [huxueshi:mld_ifc_timer_expire:2432]
[   11.258146] [huxueshi:mld_sendpack:1616]
[   11.259511] [huxueshi:ip6_finish_output2:113] <------------------------
[   11.307438] [huxueshi:ip6_finish_output2:113]
[   12.308894] [huxueshi:mld_sendpack:1616]
[   12.309215] [huxueshi:ip6_finish_output2:113]
[   12.314718] [huxueshi:ip6_finish_output2:113]
```
为什么会 send 不出去啊?

lat
```txt
[   11.102712] [huxueshi:mld_ifc_start_timer:1014] 0
[   11.104173] [huxueshi:mld_ifc_timer_expire:2432]
[   11.106805] [huxueshi:mld_sendpack:1616]
[   11.119300] [huxueshi:ip6_finish_output2:113]
[   11.122026] [huxueshi:dev_hard_start_xmit:2776]
[   11.122338] [huxueshi:xmit_one:2753]
[   11.122617] [huxueshi:netdev_start_xmit:3752]
[   11.124302] e1000e: [KERNEL:e1000_xmit_frame:5821] 1
[   11.125321] e1000e: [huxueshi:e1000_intr_msix_tx:1948]
[   11.128360] [huxueshi:mld_ifc_start_timer:1014] 203
[   11.646710] [huxueshi:mld_ifc_timer_expire:2432]
[   11.647070] [huxueshi:mld_sendpack:1616]
[   11.647578] [huxueshi:ip6_finish_output2:113] <--------------------------
[   11.647578] [huxueshi:dev_hard_start_xmit:2776]
[   11.647578] [huxueshi:xmit_one:2753]
[   11.647578] [huxueshi:netdev_start_xmit:3752]
[   11.647578] e1000e: [KERNEL:e1000_xmit_frame:5821] 1
[   11.647578] e1000e: [huxueshi:e1000_intr_msix_tx:1948]
[   12.100188] [huxueshi:ip6_finish_output2:113]
[   12.100497] [huxueshi:dev_hard_start_xmit:2776]
[   12.100652] [huxueshi:xmit_one:2753]
[   12.100799] [huxueshi:netdev_start_xmit:3752]
[   12.100994] e1000e: [KERNEL:e1000_xmit_frame:5821] 1
[   12.101386] e1000e: [huxueshi:e1000_intr_msix_tx:1948]
```

## 增加更多的调整

```txt
[   10.861454] IPv6: ADDRCONF(NETDEV_CHANGE): eth0: link becomes ready
[   10.875869] [huxueshi:mld_ifc_start_timer:1014] 0
[   10.878166] [huxueshi:mld_ifc_timer_expire:2432]
[   10.880789] [huxueshi:mld_sendpack:1616]
[   10.888538] [huxueshi:ip6_output:146]
[   10.890430] [huxueshi:ip6_finish_output:127]
[   10.893792] [huxueshi:ip6_finish_output2:113]
[   10.896600] [huxueshi:dev_hard_start_xmit:2776]
[   10.896857] [huxueshi:xmit_one:2753]
[   10.897205] [huxueshi:netdev_start_xmit:3752]
[   10.898849] e1000e: [KERNEL:e1000_xmit_frame:5821] 1
[   10.899915] e1000e: [huxueshi:e1000_intr_msix_tx:1948]
[   10.902925] [huxueshi:mld_ifc_start_timer:1014] 262
[   11.516812] [huxueshi:mld_ifc_timer_expire:2432]
[   11.517161] [huxueshi:mld_sendpack:1616]
[   11.517549] [huxueshi:ip6_output:146]
[   11.517687] [huxueshi:ip6_finish_output:127]
[   11.517687] [huxueshi:ip6_finish_output2:113]
[   11.517687] [huxueshi:dev_hard_start_xmit:2776]
[   11.517687] [huxueshi:xmit_one:2753]
[   11.517687] [huxueshi:netdev_start_xmit:3752]
```

错误的:
```txt
[   10.972803] [huxueshi:mld_ifc_start_timer:1014] 0
[   10.976574] [huxueshi:mld_ifc_timer_expire:2432]
[   10.984729] [huxueshi:mld_sendpack:1616]
[   11.008318] [huxueshi:ip6_output:146]
[   11.014183] [huxueshi:ip6_finish_output:127]
[   11.024528] [huxueshi:ip6_finish_output2:113]
[   11.033054] [huxueshi:dev_hard_start_xmit:2776]
[   11.033872] [huxueshi:xmit_one:2753]
[   11.035075] [huxueshi:netdev_start_xmit:3752]
[   11.040262] e1000e: [KERNEL:e1000_xmit_frame:5821] 1
[huxueshi:apic_send_msi:636] 51
[huxueshi:apic_set_irq:311] 51
[   11.044039] e1000e: [huxueshi:e1000_intr_msix_tx:1948]
[   11.048185] [huxueshi:mld_ifc_start_timer:1014] 292
[   11.212910] [huxueshi:ip6_output:146]
[   11.213716] [huxueshi:ip6_finish_output:127]
[   11.214508] [huxueshi:ip6_finish_output2:113]
[   11.710609] [huxueshi:mld_ifc_timer_expire:2432]
[   11.711532] [huxueshi:mld_sendpack:1616]
[   11.711662] [huxueshi:ip6_output:146]
[   11.712614] [huxueshi:ip6_finish_output:127]
[   11.714057] [huxueshi:ip6_finish_output2:113]
[   12.214227] [huxueshi:mld_sendpack:1616]
[   12.214606] [huxueshi:ip6_output:146]
[   12.215562] [huxueshi:ip6_finish_output:127]
[   12.216552] [huxueshi:ip6_finish_output2:113]
[   12.220811] [huxueshi:ip6_output:146]
[   12.221558] [huxueshi:ip6_finish_output:127]
[   12.222338] [huxueshi:ip6_finish_output2:113]
[   12.442707] [huxueshi:mld_sendpack:1616]
[   12.443531] [huxueshi:ip6_output:146]
[   12.444207] [huxueshi:ip6_finish_output:127]
[   12.444499] [huxueshi:ip6_finish_output2:113]
```

## 分析从 output2 到 start_xmit 的过程是什么
```txt
[   10.963940] [huxueshi:ip6_finish_output2:115]
[   10.964249] [huxueshi:dev_hard_start_xmit:2776]
```
