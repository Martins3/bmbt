## 访存
```c
#0  tlb_set_page_with_attrs (cpu=0x7ffff7ffd9e8 <_rtld_global+2440>, vaddr=32767, paddr=140737354006919, attrs=..., prot=0, mmu_idx=-1, size=4096) at src/tcg/cputlb.c:6
82
#1  0x0000555555629469 in handle_mmu_fault (cs=0x55555596ea80 <__x86_cpu>, addr=4294967280, size=0, is_write1=2, mmu_idx=2) at src/i386/excp_helper.c:637
#2  0x0000555555629640 in x86_cpu_tlb_fill (cs=0x55555596ea80 <__x86_cpu>, addr=4294967280, size=0, access_type=MMU_INST_FETCH, mmu_idx=2, probe=false, retaddr=0) at sr
c/i386/excp_helper.c:685
#3  0x00005555555d94d8 in tlb_fill (cpu=0x55555596ea80 <__x86_cpu>, addr=4294967280, size=0, access_type=MMU_INST_FETCH, mmu_idx=2, retaddr=0) at src/tcg/cputlb.c:895
#4  0x00005555555d9d88 in get_page_addr_code_hostp (env=0x5555559771f0 <__x86_cpu+34672>, addr=4294967280, hostp=0x0) at src/tcg/cputlb.c:1075
#5  0x00005555555d9f0e in get_page_addr_code (env=0x5555559771f0 <__x86_cpu+34672>, addr=4294967280) at src/tcg/cputlb.c:1106
#6  0x00005555555cd59e in tb_htable_lookup (cpu=0x55555596ea80 <__x86_cpu>, pc=4294967280, cs_base=4294901760, flags=64, cflags=4278190080) at src/tcg/cpu-exec.c:675
#7  0x00005555555cbaab in tb_lookup__cpu_state (cpu=0x55555596ea80 <__x86_cpu>, pc=0x7fffffffd4b8, cs_base=0x7fffffffd4b4, flags=0x7fffffffd4bc, cflags=4278190080) at
src/tcg/../../include/exec/tb-lookup.h:44
#8  0x00005555555cc5a1 in tb_find (cpu=0x55555596ea80 <__x86_cpu>, last_tb=0x0, tb_exit=0, cflags=0) at src/tcg/cpu-exec.c:285
#9  0x00005555555cd0a6 in cpu_exec (cpu=0x55555596ea80 <__x86_cpu>) at src/tcg/cpu-exec.c:559
#10 0x000055555561afb8 in tcg_cpu_exec (cpu=0x55555596ea80 <__x86_cpu>) at src/qemu/cpus.c:122
#11 0x000055555561b22c in qemu_tcg_rr_cpu_thread_fn (arg=0x55555596ea80 <__x86_cpu>) at src/qemu/cpus.c:235
#12 0x00005555555c7e29 in qemu_boot () at /home/maritns3/core/ld/DuckBuBi/include/sysemu/sysemu.h:34
#13 0x00005555555c8fb4 in test_qemu_init () at src/main.c:159
#14 0x00005555555c926d in wip () at src/main.c:173
#15 0x00005555555ca124 in greatest_run_suite (suite_cb=0x5555555c9215 <wip>, suite_name=0x5555556da6d7 "wip") at src/main.c:176
#16 0x00005555555cb1ee in main (argc=1, argv=0x7fffffffd728) at src/main.c:185
```

## smc
```c
#0  0x0000555555619baf in cpu_physical_memory_test_and_clear_dirty (start=1044480, length=4096, client=1) at src/qemu/exec.c:334
#1  0x00005555555d863a in tlb_protect_code (ram_addr=1044480) at src/tcg/cputlb.c:515
#2  0x00005555555d3f78 in tb_page_add (p=0x5555559e03c8, tb=0x70000580, n=0, page_addr=1044480) at src/tcg/translate-all.c:1548
#3  0x00005555555d4074 in tb_link_page (tb=0x70000580, phys_pc=1048560, phys_page2=18446744073709551615) at src/tcg/translate-all.c:1591
#4  0x00005555555d46f9 in tb_gen_code (cpu=0x55555596e980 <__x86_cpu>, pc=4294967280, cs_base=4294901760, flags=64, cflags=-16777216) at src/tcg/translate-all.c:1773
#5  0x00005555555cc611 in tb_find (cpu=0x55555596e980 <__x86_cpu>, last_tb=0x0, tb_exit=0, cflags=0) at src/tcg/cpu-exec.c:290
#6  0x00005555555cd0d0 in cpu_exec (cpu=0x55555596e980 <__x86_cpu>) at src/tcg/cpu-exec.c:560
#7  0x000055555561b075 in tcg_cpu_exec (cpu=0x55555596e980 <__x86_cpu>) at src/qemu/cpus.c:122
#8  0x000055555561b2e9 in qemu_tcg_rr_cpu_thread_fn (arg=0x55555596e980 <__x86_cpu>) at src/qemu/cpus.c:235
#9  0x00005555555c7e29 in qemu_boot () at /home/maritns3/core/ld/DuckBuBi/include/sysemu/sysemu.h:34
#10 0x00005555555c8fb4 in test_qemu_init () at src/main.c:159
#11 0x00005555555c926d in wip () at src/main.c:173
#12 0x00005555555ca124 in greatest_run_suite (suite_cb=0x5555555c9215 <wip>, suite_name=0x5555556da6d7 "wip") at src/main.c:176
#13 0x00005555555cb1ee in main (argc=1, argv=0x7fffffffd728) at src/main.c:185
```

## usb 键盘的中断处理
```plain
[ 1075.597619] [<900000000020866c>] show_stack+0x2c/0x100
[ 1075.597621] [<9000000000ec39c8>] dump_stack+0x90/0xc0
[ 1075.597624] [<9000000000c4b1b0>] input_event+0x30/0xc8
[ 1075.597626] [<9000000000ca3ee4>] hidinput_report_event+0x44/0x68
[ 1075.597628] [<9000000000ca1e30>] hid_report_raw_event+0x230/0x470
[ 1075.597631] [<9000000000ca21a4>] hid_input_report+0x134/0x1b0
[ 1075.597632] [<9000000000cb07ac>] hid_irq_in+0x9c/0x280
[ 1075.597634] [<9000000000be9cf0>] __usb_hcd_giveback_urb+0xa0/0x120
[ 1075.597636] [<9000000000c23a7c>] finish_urb+0xac/0x1c0
[ 1075.597638] [<9000000000c24b50>] ohci_work.part.8+0x218/0x550
[ 1075.597640] [<9000000000c27f98>] ohci_irq+0x108/0x320
[ 1075.597642] [<9000000000be96e8>] usb_hcd_irq+0x28/0x40
[ 1075.597644] [<9000000000296430>] __handle_irq_event_percpu+0x70/0x1b8
[ 1075.597645] [<9000000000296598>] handle_irq_event_percpu+0x20/0x88
[ 1075.597647] [<9000000000296644>] handle_irq_event+0x44/0xa8
[ 1075.597648] [<900000000029abfc>] handle_level_irq+0xdc/0x188
[ 1075.597651] [<90000000002952a4>] generic_handle_irq+0x24/0x40
[ 1075.597652] [<900000000081dc50>] extioi_irq_dispatch+0x178/0x210
[ 1075.597654] [<90000000002952a4>] generic_handle_irq+0x24/0x40
[ 1075.597656] [<9000000000ee4eb8>] do_IRQ+0x18/0x28
[ 1075.597658] [<9000000000203ffc>] except_vec_vi_end+0x94/0xb8
[ 1075.597660] [<9000000000203e80>] __cpu_wait+0x20/0x24
[ 1075.597662] [<900000000020fa90>] calculate_cpu_foreign_map+0x148/0x180
```

## qemu 注入中断的过程

1. 在 qemu 中的部分
```c
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
```c
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
```c
#0  extioi_init () at drivers/irqchip/irq-loongarch-extioi.c:165
#1  0x90000000008a3428 in extioi_vec_init (fwnode=0x900000027c011300, cascade=<optimized out>, vec_count=<optimized out>, misc_func=<optimized out>, eio_en_off=<optimiz
ed out>, node_map=1, node=0) at drivers/irqchip/irq-loongarch-extioi.c:376
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
