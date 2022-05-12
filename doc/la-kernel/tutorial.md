## Loongarch qemu 注入中断的过程

1. 在 qemu 中的部分
```txt
#1  0x00000001201af1d4 in ioapic_handler (opaque=0x120ce1ae0, irq=2, level=0) at /home/loongson/core/centos-qemu/hw/loongarch/larch_3a.c:1066
#2  0x00000001202c66b0 in qemu_set_irq (irq=0x120e78ed0, level=0) at /home/loongson/core/centos-qemu/hw/core/irq.c:44
#3  0x00000001201aedac in legacy_set_irq (opaque=0x120c25010, irq=0, level=0) at /home/loongson/core/centos-qemu/hw/loongarch/larch_3a.c:917
#4  0x00000001202c66b0 in qemu_set_irq (irq=0x12128b830, level=0) at /home/loongson/core/centos-qemu/hw/core/irq.c:44
#5  0x00000001202baa28 in qemu_irq_lower (irq=0x12128b830) at /home/loongson/core/centos-qemu/include/hw/irq.h:17
#6  0x00000001202baf58 in serial_update_irq (s=0x12128b9a0) at /home/loongson/core/centos-qemu/hw/char/serial.c:152
#7  0x00000001202bb58c in serial_xmit (s=0x12128b9a0) at /home/loongson/core/centos-qemu/hw/char/serial.c:261
#8  0x00000001202bba80 in serial_ioport_write (opaque=0x12128b9a0, addr=0, val=74, size=1) at /home/loongson/core/centos-qemu/hw/char/serial.c:373
#9  0x00000001202bcf6c in serial_mm_write (opaque=0x12128b9a0, addr=0, value=74, size=1) at /home/loongson/core/centos-qemu/hw/char/serial.c:1032
#10 0x00000001200d8114 in memory_region_write_accessor (mr=0x12128ba80, addr=0, value=0xffe7ffdfc8, size=1, shift=0, mask=255, attrs=...) at /home/loongson/core/centos-qemu/memory.c:483
#11 0x00000001200d83f0 in access_with_adjusted_size (addr=0, value=0xffe7ffdfc8, size=1, access_size_min=1, access_size_max=8, access_fn=0x1200d8024 <memory_region_write_accessor>, mr=0x12128ba80, attrs=...) at /home/loongson/core/centos-qemu/memory.c:544
#12 0x00000001200db830 in memory_region_dispatch_write (mr=0x12128ba80, addr=0, data=74, op=MO_8, attrs=...) at /home/loongson/core/centos-qemu/memory.c:1475
#13 0x0000000120065a68 in flatview_write_continue (fv=0xffe0a46e70, addr=534774240, attrs=..., buf=0xfff7fcc028 "J", len=1, addr1=0, l=1, mr=0x12128ba80) at /home/loongson/core/centos-qemu/exec.c:3129
#14 0x0000000120065c00 in flatview_write (fv=0xffe0a46e70, addr=534774240, attrs=..., buf=0xfff7fcc028 "J", len=1) at /home/loongson/core/centos-qemu/exec.c:3169
#15 0x0000000120066020 in address_space_write (as=0x120b30e88 <address_space_memory>, addr=534774240, attrs=..., buf=0xfff7fcc028 "J", len=1) at /home/loongson/core/centos-qemu/exec.c:3259
#16 0x00000001200660b4 in address_space_rw (as=0x120b30e88 <address_space_memory>, addr=534774240, attrs=..., buf=0xfff7fcc028 "J", len=1, is_write=true) at /home/loongson/core/centos-qemu/exec.c:3269
#17 0x00000001200f9540 in kvm_cpu_exec (cpu=0x120c91360) at /home/loongson/core/centos-qemu/accel/kvm/kvm-all.c:2386
#18 0x00000001200c5d20 in qemu_kvm_cpu_thread_fn (arg=0x120c91360) at /home/loongson/core/centos-qemu/cpus.c:1318
#19 0x00000001206e2538 in qemu_thread_start (args=0x120cc0790) at /home/loongson/core/centos-qemu/util/qemu-thread-posix.c:519
#20 0x000000fff754489c in start_thread () at /lib/loongarch64-linux-gnu/libpthread.so.0
```

2. 在内核中的部分
```txt
[41565.981837] kvm_vm_ioctl
[41565.981837] kvm_vm_ioctl_irq_line
[41565.981837] kvm_ls7a_ioapic_set_irq level=1 state->intedge=8 mask=4 last_intirr=0 intirr=0
[41565.981838] kvm_ls7a_ioapic_raise
[41565.981838] kvm [7368]: msi_irq_handler,2,up
[41565.981839] kvm [7368]: ext_irq_handler:irq = 2,level = 1
[41565.981841] huxueshi:msi_irq_handler raise
[41565.981842] kvm [7368]: ext_irq_update_core:86
[41565.981844] huxueshi:kvm_vcpu_ioctl_interrupt intr=3
[41565.981845] huxueshi:lvz_queue_irq 3
[41565.981848] huxueshi:lvz_irq_deliver 2 3
```

## extioi 初始化
```txt
#0  extioi_init () at drivers/irqchip/irq-loongarch-extioi.c:165
#1  0x90000000008a3428 in extioi_vec_init (fwnode=0x900000027c011300, cascade=<optimized out>, vec_count=<optimized out>, misc_func=<optimized out>, eio_en_off=<optimiz ed out>, node_map=1, node=0) at drivers/irqchip/irq-loongarch-extioi.c:376
#2  0x9000000000f6e5e8 in eiointc_domain_init () at arch/loongarch/la64/irq.c:251
#3  irqchip_init_default () at arch/loongarch/la64/irq.c:283
#4  0x90000000014ace78 in setup_IRQ () at arch/loongarch/la64/irq.c:311
#5  0x90000000014acea4 in arch_init_irq () at arch/loongarch/la64/irq.c:360
#6  0x90000000014ae708 in init_IRQ () at arch/loongarch/kernel/irq.c:59
#7  0x90000000014a8a40 in start_kernel () at init/main.c:636
#8  0x9000000000f79084 in kernel_entry () at arch/loongarch/kernel/head.S:129
Backtrace stopped: frame did not save the PC
```

## loongarch address flatview
```txt
FlatView #0
 AS "ls7a1000_pcie", root: bus master container
 AS "virtio-net-pci", root: bus master container
 Root memory region: (none)
  No rendered FlatView

FlatView #1
 AS "I/O", root: io
 Root memory region: io
  0000000000000000-00000000000003af (prio 0, i/o): iohuxueshi:kvm_cpu_exec 1fe001e6
FlatView #0
 AS "ls7a1000_pcie", root: bus master container
 AS "virtio-net-pci", root: bus master container
 Root memory region: (none)
  No rendered FlatView

FlatView #1
 AS "I/O", root: io
 Root memory region: io
  0000000000000000-00000000000003af (prio 0, i/o): io
  00000000000003b0-00000000000003df (prio 0, i/o): cirrus-io
  00000000000003e0-000000000000ffff (prio 0, i/o): io @00000000000003e0

FlatView #2
 AS "memory", root: system
 AS "cpu-memory-0", root: system
 AS "cirrus-vga", root: bus master container
 Root memory region: system
  0000000000000000-000000000009ffff (prio 0, ram): loongarch_ls3a.ram kvm
  00000000000a0000-00000000000affff (prio 1, ram): vga.vram kvm
  00000000000b0000-00000000000bffff (prio 0, i/o): cirrus-low-memory @0000000000010000
  00000000000c0000-000000000fffffff (prio 0, ram): loongarch_ls3a.ram @00000000000c0000 kvm
  0000000010000000-0000000010000fff (prio 0, i/o): ioapic
  0000000010002000-0000000010002013 (prio 1, i/o): 0x10002000
  000000001001041c-000000001001041f (prio 1, i/o): 0x1001041c
  0000000010013ffc-0000000010013fff (prio 1, i/o): 0x10013ffc
  0000000010080000-00000000100800ff (prio 0, i/o): ls3a_pm
  00000000100d000c-00000000100d0013 (prio 0, i/o): acpi-evt
  00000000100d0014-00000000100d0017 (prio 0, i/o): acpi-cnt
  00000000100d0018-00000000100d001b (prio 0, i/o): acpi-tmr
  00000000100d0028-00000000100d002f (prio 0, i/o): acpi-gpe0
  00000000100d0030-00000000100d0033 (prio 0, i/o): acpi-reset
  00000000100d0100-00000000100d01ff (prio 0, i/o): ls7a_rtc
  0000000018000000-00000000180003af (prio 0, i/o): io
  00000000180003b0-00000000180003df (prio 0, i/o): cirrus-io
  00000000180003e0-000000001800ffff (prio 0, i/o): io @00000000000003e0
  000000001a000000-000000001bffffff (prio 0, i/o): ls7a_pci_conf
  000000001c000000-000000001c3fffff (prio 0, rom): loongarch.bios kvm
  000000001e000000-000000001e00000b (prio 0, i/o): acpi-mem-hotplug
  000000001e020000-000000001e020001 (prio 0, i/o): fwcfg.ctl
  000000001e020008-000000001e02000f (prio 0, i/o): fwcfg.data
  000000001f000000-000000001f0000ff (prio 0, i/o): gipi0
  000000001f010000-000000001f02ffff (prio 0, i/o): apic0
  000000001fe00008-000000001fe0000f (prio 1, i/o): ((hwaddr)0x1fe00008 | off)
  000000001fe00010-000000001fe00017 (prio 1, i/o): ((hwaddr)0x1fe00010 | off)
  000000001fe00020-000000001fe00027 (prio 1, i/o): ((hwaddr)0x1fe00020 | off)
  000000001fe00180-000000001fe00187 (prio 1, i/o): ((hwaddr)0x1fe00180 | off)
  000000001fe0019c-000000001fe001a3 (prio 1, i/o): ((hwaddr)0x1fe0019c | off)
  000000001fe001d0-000000001fe001d7 (prio 1, i/o): ((hwaddr)0x1fe001d0 | off)
  000000001fe001e0-000000001fe001e7 (prio 0, i/o): serial
  000000001fe002e0-000000001fe002e0 (prio 0, i/o): debugcon
  000000001fe00420-000000001fe00427 (prio 1, i/o): ((hwaddr)0x1fe00420 | off)
  0000000020000000-0000000027ffffff (prio 0, i/o): pcie-mmcfg-mmio
  000000002ff00000-000000002ff00007 (prio 0, i/o): ls3a_msi
  0000000040000000-00000000403fffff (prio 1, ram): vga.vram kvm
  0000000041000000-00000000413fffff (prio 0, i/o): cirrus-bitblt-mmio
  0000000042004000-0000000042004fff (prio 1, i/o): cirrus-mmio
  0000000090000000-000000017fffffff (prio 0, ram): loongarch_ls3a.ram @0000000010000000 kvm
```
