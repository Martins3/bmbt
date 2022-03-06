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
