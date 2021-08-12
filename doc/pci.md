# pci


## 资料

- lspci
```
00:00.0 Host bridge: Intel Corporation 440FX - 82441FX PMC [Natoma] (rev 02)
00:01.0 ISA bridge: Intel Corporation 82371SB PIIX3 ISA [Natoma/Triton II]
00:01.1 IDE interface: Intel Corporation 82371SB PIIX3 IDE [Natoma/Triton II]
00:01.3 Bridge: Intel Corporation 82371AB/EB/MB PIIX4 ACPI (rev 03)
00:02.0 VGA compatible controller: Red Hat, Inc. Virtio GPU (rev 01)
00:03.0 Ethernet controller: Intel Corporation 82540EM Gigabit Ethernet Controller (rev 03)
00:04.0 Non-Volatile memory controller: Red Hat, Inc. Device 0010 (rev 02)
00:05.0 Non-Volatile memory controller: Red Hat, Inc. Device 0010 (rev 02)
00:06.0 Unclassified device [0002]: Red Hat, Inc. Virtio filesystem
```


## 分析
- [ ] 能不能不要让其探测出来 piix3 ，如此一了百了
  - 使用其他的方法来模拟 piix3 的功能


在我们的设想当中，pci 设备都是可以进行穿透的，还是存在很多需要考虑的问题, 比如
- [ ] 如果提供 piix3 / piix4 等虚拟设备，如何处理
- [ ] 因为需要模拟 piix3 / piix4 所以，pci_default_write_config 这个函数实现，这似乎是一个无底洞啊


- [x] 检查一下，除了 piix3 / piix4 之外，还有那些 x86 独占的设备
  - 从 lspci 看，应该只有这些了
  - [ ] 可以检查一下 seabios 的 pci_device_tbl

- [ ] kvmtool 中 pci_config_mmio_access 让我意识到，pci 的配置空间也是可以进行 MMIO 的, 实际上，LoongArch 还可以配置这个功能

- [ ] 真正的恐惧 : /home/maritns3/core/kvmqemu/hw/isa/piix3.c 中需要模拟，但是到底为什么需要模拟，到底会牵涉多少东西
  - [ ] piix4 也需要处理吗

- [ ] 只是因为为了处理 isa 才需要搞 piix4 的吗 ?
  - piix4 和 acpi 中还有一些酷炫的关系


11. 分析这个路径产生的原因是什么:
```c
/*
#0  huxueshi (irq=14) at ../hw/i386/kvm/ioapic.c:112
#1  insert_counter (irq=14) at ../hw/i386/kvm/ioapic.c:125
#2  kvm_ioapic_set_irq (opaque=0x555556bec800, irq=14, level=0) at ../hw/i386/kvm/ioapic.c:135
#3  0x0000555555b92644 in gsi_handler (opaque=0x555556a0e200, n=14, level=0) at ../hw/i386/x86.c:600
#4  0x0000555555a23e2e in piix3_set_irq_pic (piix3=0x555556a60310, piix3=0x555556a60310, pic_irq=<optimized out>) at ../hw/isa/piix3.c:123
#5  piix3_write_config (address=<optimized out>, val=<optimized out>, len=<optimized out>, dev=<optimized out>) at ../hw/isa/piix3.c:123
#6  piix3_write_config (dev=<optimized out>, address=<optimized out>, val=<optimized out>, len=<optimized out>) at ../hw/isa/piix3.c:112
#7  0x0000555555a34bbb in pci_host_config_write_common (pci_dev=0x555556a60310, addr=96, limit=<optimized out>, val=10, len=1) at ../hw/pci/pci_host.c:83
#8  0x0000555555cd2661 in memory_region_write_accessor (mr=mr@entry=0x555556a22de0, addr=0, value=value@entry=0x7fffe890d0a8, size=size@entry=1, shift=<optimized out>,
mask=mask@entry=255, attrs=...) at ../softmmu/memory.c:492
#9  0x0000555555cceaee in access_with_adjusted_size (addr=addr@entry=0, value=value@entry=0x7fffe890d0a8, size=size@entry=1, access_size_min=<optimized out>, access_siz
e_max=<optimized out>, access_fn=access_fn@entry=0x555555cd25d0 <memory_region_write_accessor>, mr=0x555556a22de0, attrs=...) at ../softmmu/memory.c:554
#10 0x0000555555cd1b97 in memory_region_dispatch_write (mr=mr@entry=0x555556a22de0, addr=0, data=<optimized out>, op=<optimized out>, attrs=attrs@entry=...) at ../softm
mu/memory.c:1504
#11 0x0000555555c9c060 in flatview_write_continue (fv=fv@entry=0x7ffe4c043840, addr=addr@entry=3324, attrs=..., ptr=ptr@entry=0x7fffeb180000, len=len@entry=1, addr1=<op
timized out>, l=<optimized out>, mr=0x555556a22de0) at /home/maritns3/core/kvmqemu/include/qemu/host-utils.h:165
#12 0x0000555555c9c276 in flatview_write (fv=0x7ffe4c043840, addr=addr@entry=3324, attrs=attrs@entry=..., buf=buf@entry=0x7fffeb180000, len=len@entry=1) at ../softmmu/p
hysmem.c:2818
#13 0x0000555555c9ef46 in address_space_write (as=0x555556606a40 <address_space_io>, addr=addr@entry=3324, attrs=..., buf=0x7fffeb180000, len=len@entry=1) at ../softmmu
/physmem.c:2910
#14 0x0000555555c9efde in address_space_rw (as=<optimized out>, addr=addr@entry=3324, attrs=..., attrs@entry=..., buf=<optimized out>, len=len@entry=1, is_write=is_writ
e@entry=true) at ../softmmu/physmem.c:2920
#15 0x0000555555c8ecb9 in kvm_handle_io (count=1, size=1, direction=<optimized out>, data=<optimized out>, attrs=..., port=3324) at ../accel/kvm/kvm-all.c:2632
#16 kvm_cpu_exec (cpu=cpu@entry=0x555556b030d0) at ../accel/kvm/kvm-all.c:2883
#17 0x0000555555cf17f5 in kvm_vcpu_thread_fn (arg=arg@entry=0x555556b030d0) at ../accel/kvm/kvm-accel-ops.c:49
#18 0x0000555555e55953 in qemu_thread_start (args=<optimized out>) at ../util/qemu-thread-posix.c:541
#19 0x00007ffff628d609 in start_thread (arg=<optimized out>) at pthread_create.c:477
#20 0x00007ffff61b4293 in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:95
```

- 分析一下 piix3_write_config 的源代码位置
  - 因为 piix3 是 isa 的 bridge, 所以应该可以实现一些中断路由的操作?
  - [ ] 无法理解为什么写的时候就要触发一下中断啊
  - [ ] 在 seabios 中如果没有注册 irq handler 的话，这些是如何处理的

```c
static void pci_bios_init_devices(void)
{
    struct pci_device *pci;
    foreachpci(pci) {
        pci_bios_init_device(pci);
    }
}

/* PIIX3/PIIX4 PCI to ISA bridge */
static void piix_isa_bridge_setup(struct pci_device *pci, void *arg)
{
    int i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;
    for (i = 0; i < 4; i++) {
        irq = pci_irqs[i];
        /* set to trigger level */
        elcr[irq >> 3] |= (1 << (irq & 7));
        /* activate irq remapping in PIIX */
        pci_config_writeb(pci->bdf, 0x60 + i, irq);
    }
    outb(elcr[0], PIIX_PORT_ELCR1);
    outb(elcr[1], PIIX_PORT_ELCR2);
    dprintf(1, "PIIX3/PIIX4 init: elcr=%02x %02x\n", elcr[0], elcr[1]);
}
```
- [ ] 为什么需要将 bridge 接入的四个中断全部设置为 level 的

- [ ] wiki[^2] 描述了 elcr 说处理 isa 总线之类的
  - [ ] 找到 QEMU 对应的 handler 的处理效果

- [ ] 一个好端端的 PCI bridge 为什么需要和 pic 控制器产生联系
    - [ ] 而且 pic 不是很快就 disable 掉了吗?

- [ ] 内核中也会调用 piix3_write_config 的, 找到对应的位置

- [ ] piix3_read_config 并没有对应的版本的


## [ ] elcr
- i8259_init_chip : 注册地址
- pic_common_realize : 将 PICCommonState::elcr_io 这个地址空间注册到 isa 上去
- pic_realize : 注册 handler, 似乎超级简单, 作用就是修改 PICCommonState::elcr 的数值，最后的作用体现在 pic_set_irq 上的

- [ ] kvm 为何无需考虑 eclr 这个东西啊

## [ ] 中断在 piix3 中的工作方式
- nvme_irq_check
  - nvme_irq_check
    - pci_irq_assert
      - pci_set_irq
        - pci_irq_handler


分析一下 piix3_set_irq 的使用位置: 
1. nvme_init_cq : 注册是 timer 上的，会周期性的触发 nvme_post_cqes
  - nvme_post_cqes
    - nvme_irq_assert
      - nvme_irq_check
        - pci_irq_assert
          - int intx = pci_intx(pci_dev); // 用于获取到底是 pci 中断线
          - pci_irq_handler
            - pci_change_irq_level
              - pci_bus_change_irq_level
                - PCIBus::set_irq : 根据当前的机器配置，注册的是 piix3_set_irq

              
2. 看来 pci 设备发送都是需要经过的 piix3 的
```c
/*
#0  piix3_set_irq (opaque=0x555556a1ee20, pirq=2, level=0) at ../hw/isa/piix3.c:79
#1  0x0000555555b5e754 in mac_icr_read (s=<optimized out>, index=<optimized out>) at ../hw/net/e1000.c:1083
#2  0x0000555555b5d32e in e1000_mmio_read (opaque=<optimized out>, addr=<optimized out>, size=<optimized out>) at ../hw/net/e1000.c:1351
#3  0x0000555555cd20c2 in memory_region_read_accessor (mr=mr@entry=0x555557a510d0, addr=192, value=value@entry=0x7fffd9ff9130, size=size@entry=4, shift=0, mask=mask@entry=4294967295, attrs=...) at ../softmmu/memory.c:440
#4  0x0000555555cceb4e in access_with_adjusted_size (addr=addr@entry=192, value=value@entry=0x7fffd9ff9130, size=size@entry=4, access_size_min=<optimized out>, access_size_max=<optimized out>, access_fn=0x555555cd2080 <memory_region_read_accessor>, mr=0x555557a510d0, attrs=...) at ../softmmu/memory.c:554
#5  0x0000555555cd1af1 in memory_region_dispatch_read1 (attrs=..., size=<optimized out>, pval=0x7fffd9ff9130, addr=192, mr=0x555557a510d0) at ../softmmu/memory.c:1424
#6  memory_region_dispatch_read (mr=mr@entry=0x555557a510d0, addr=192, pval=pval@entry=0x7fffd9ff9130, op=MO_32, attrs=attrs@entry=...) at ../softmmu/memory.c:1452
#7  0x0000555555c9ebb9 in flatview_read_continue (fv=fv@entry=0x7ffe4c0fefb0, addr=addr@entry=4273733824, attrs=..., ptr=ptr@entry=0x7fffeb17c028, len=len@entry=4, addr 1=<optimized out>, l=<optimized out>, mr=0x555557a510d0) at /home/maritns3/core/kvmqemu/include/qemu/host-utils.h:165
#8  0x0000555555c9ed73 in flatview_read (fv=0x7ffe4c0fefb0, addr=addr@entry=4273733824, attrs=attrs@entry=..., buf=buf@entry=0x7fffeb17c028, len=len@entry=4) at ../softmmu/physmem.c:2881
#9  0x0000555555c9eec6 in address_space_read_full (as=0x5555566079e0 <address_space_memory>, addr=4273733824, attrs=..., buf=0x7fffeb17c028, len=4) at ../softmmu/physmem.c:2894
#10 0x0000555555c9f045 in address_space_rw (as=<optimized out>, addr=<optimized out>, attrs=..., attrs@entry=..., buf=buf@entry=0x7fffeb17c028, len=<optimized out>, is_write=<optimized out>) at ../softmmu/physmem.c:2922
#11 0x0000555555c8ec96 in kvm_cpu_exec (cpu=cpu@entry=0x555556bd6c00) at ../accel/kvm/kvm-all.c:2893
#12 0x0000555555cf1855 in kvm_vcpu_thread_fn (arg=arg@entry=0x555556bd6c00) at ../accel/kvm/kvm-accel-ops.c:49
#13 0x0000555555e559b3 in qemu_thread_start (args=<optimized out>) at ../util/qemu-thread-posix.c:541
#14 0x00007ffff628d609 in start_thread (arg=<optimized out>) at pthread_create.c:477
#15 0x00007ffff61b4293 in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:95
```

3. 
```c
/*
#0  piix3_set_irq (opaque=0x555556a1ee20, pirq=2, level=1) at ../hw/isa/piix3.c:79
#1  0x0000555555b5f610 in set_ics (val=<optimized out>, index=0, s=0x555557a4e750) at ../hw/net/e1000.c:1031
#2  e1000_receive_iov (nc=<optimized out>, iov=0x7ffffffed1a0, iovcnt=<optimized out>) at ../hw/net/e1000.c:1031
#3  0x000055555595c623 in qemu_deliver_packet_iov (sender=<optimized out>, opaque=0x555556d3d120, iovcnt=1, iov=0x7ffffffed190, flags=0) at ../net/net.c:762
#4  qemu_deliver_packet_iov (sender=<optimized out>, flags=0, iov=0x7ffffffed190, iovcnt=1, opaque=0x555556d3d120) at ../net/net.c:743
#5  0x0000555555b6442c in qemu_net_queue_deliver_iov (iovcnt=1, iov=0x7ffffffed190, flags=0, sender=0x555556a27460, queue=0x555556d3d2a0) at ../net/queue.c:179
#6  qemu_net_queue_send_iov (queue=0x555556d3d2a0, sender=0x555556a27460, flags=0, iov=0x7ffffffed190, iovcnt=1, sent_cb=0x0) at ../net/queue.c:246
#7  0x0000555555adc9bb in net_hub_receive_iov (hub=<optimized out>, iovcnt=1, iov=0x7ffffffed190, source_port=0x555556a27c50) at ../net/hub.c:74
#8  net_hub_port_receive_iov (nc=0x555556a27c50, iov=0x7ffffffed190, iovcnt=1) at ../net/hub.c:125
#9  0x000055555595c623 in qemu_deliver_packet_iov (sender=<optimized out>, opaque=0x555556a27c50, iovcnt=1, iov=0x7ffffffed190, flags=0) at ../net/net.c:762
#10 qemu_deliver_packet_iov (sender=<optimized out>, flags=0, iov=0x7ffffffed190, iovcnt=1, opaque=0x555556a27c50) at ../net/net.c:743
#11 0x0000555555b6436b in qemu_net_queue_deliver (size=110, data=0x7ffffffed310 "33", flags=0, sender=0x5555569fe310, queue=0x555556706890) at ../net/queue.c:164
#12 qemu_net_queue_send (queue=0x555556706890, sender=sender@entry=0x5555569fe310, flags=flags@entry=0, data=data@entry=0x7ffffffed310 "33", size=size@entry=110, sent_cb=sent_cb@entry=0x0) at ../net/queue.c:221
#13 0x000055555595c84c in qemu_send_packet_async_with_flags (sender=0x5555569fe310, flags=0, buf=0x7ffffffed310 "33", size=<optimized out>, sent_cb=0x0) at ../net/net.c:671
#14 0x0000555555aca88f in net_slirp_send_packet (pkt=0x7ffffffed310, pkt_len=110, opaque=0x5555569fe310) at ../net/slirp.c:129
#15 0x0000555555e84661 in slirp_send_packet_all (slirp=<optimized out>, buf=<optimized out>, len=110) at ../slirp/src/slirp.c:1181
#16 0x0000555555e84be5 in if_encap (slirp=slirp@entry=0x555556d57770, ifm=ifm@entry=0x555557c3b300) at ../slirp/src/slirp.c:985
#17 0x0000555555e8fb8f in if_start (slirp=0x555556d57770) at ../slirp/src/if.c:183
#18 0x0000555555e8fd70 in if_output (so=so@entry=0x0, ifm=ifm@entry=0x555557c3b300) at ../slirp/src/if.c:128
#19 0x0000555555e9122b in ip6_output (so=so@entry=0x0, m=m@entry=0x555557c3b300, fast=fast@entry=0) at ../slirp/src/ip6_output.c:35
#20 0x0000555555e90360 in ndp_send_ra (slirp=0x555556d57770) at ../slirp/src/ip6_icmp.c:213
#21 0x0000555555e6fe38 in timerlist_run_timers (timer_list=0x555556708070) at ../util/qemu-timer.c:573
#22 timerlist_run_timers (timer_list=0x555556708070) at ../util/qemu-timer.c:498
#23 0x0000555555e70047 in qemu_clock_run_all_timers () at ../util/qemu-timer.c:669
#24 0x0000555555e4ce89 in main_loop_wait (nonblocking=nonblocking@entry=0) at ../util/main-loop.c:542
#25 0x0000555555c582f1 in qemu_main_loop () at ../softmmu/runstate.c:726
#26 0x0000555555940c92 in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:50
```

## piix4
piix4 只是 piix3 的升级版本而已，但是不采用这个东西

- [x] 似乎所有的中断都是经过 piix3 的，piix4 的作用是什么?
  - hw/isa/meson.build : 指出只有 CONFIG_PIIX4 的时候，才会编译 piix4.c
  - 仅仅是在 configs/devices/mips-softmmu/common.mak 中存在的 CONFIG_PIIX4
  - 但是 build/x86_64-softmmu-config-devices.h 中，并没有配置

## i440fx
- [ ] 为什么 nvme 的中断最后不是经过 i440fx 的，这才是 pci hub 的啊


[^1]: https://stackoverflow.com/questions/52136259/how-to-access-pci-express-configuration-space-via-mmio
[^2]: https://en.wikipedia.org/wiki/Intel_8259
