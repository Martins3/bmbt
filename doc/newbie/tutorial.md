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
[41565.981837] huxueshi:kvm_ls7a_ioapic_set_irq level=1 state->intedge=8 mask=4 last_intirr=0 intirr=0
[41565.981838] huxueshi:kvm_ls7a_ioapic_raise
[41565.981838] kvm [7368]: msi_irq_handler,2,up
[41565.981839] kvm [7368]: ext_irq_handler:irq = 2,level = 1
[41565.981841] huxueshi:msi_irq_handler raise
[41565.981842] kvm [7368]: ext_irq_update_core:86
[41565.981844] huxueshi:kvm_vcpu_ioctl_interrupt intr=3
[41565.981845] huxueshi:lvz_queue_irq 3
[41565.981848] huxueshi:lvz_irq_deliver 2 3
```
