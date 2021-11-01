## 两个经典的 backtrace
```c
#0  tlb_set_page_with_attrs (cpu=0x7ffff7ffd9e8 <_rtld_global+2440>, vaddr=32767, paddr=140737354006919, attrs=..., prot=0, mmu_idx=-1, size=4096) at src/tcg/cputlb.c:6
82
#1  0x0000555555629469 in handle_mmu_fault (cs=0x55555596ea80 <__x86_cpu>, addr=4294967280, size=0, is_write1=2, mmu_idx=2) at src/i386/excp_helper.c:637
#2  0x0000555555629640 in x86_cpu_tlb_fill (cs=0x55555596ea80 <__x86_cpu>, addr=4294967280, size=0, access_type=MMU_INST_FETCH, mmu_idx=2, probe=false, retaddr=0) at sr
c/i386/excp_helper.c:685
#3  0x00005555555d94d8 in tlb_fill (cpu=0x55555596ea80 <__x86_cpu>, addr=4294967280, size=0, access_type=MMU_INST_FETCH, mmu_idx=2, retaddr=0) at src/tcg/cputlb.c:895
#4  0x00005555555d9d88 in get_page_addr_code_hostp (env=0x5555559771f0 <__x86_cpu+34672>, addr=4294967280, hostp=0x0) at src/tcg/cputlb.c:1075
#5  0x00005555555d9f0e in get_page_addr_code (env=0x5555559771f0 <__x86_cpu+34672>, addr=4294967280) at src/tcg/cputlb.c:1106
#6  0x00005555555cd59e in tb_htable_lookup (cpu=0x55555596ea80 <__x86_cpu>, pc=4294967280, cs_base=4294901760, flags=64, cf_mask=4278190080) at src/tcg/cpu-exec.c:675
#7  0x00005555555cbaab in tb_lookup__cpu_state (cpu=0x55555596ea80 <__x86_cpu>, pc=0x7fffffffd4b8, cs_base=0x7fffffffd4b4, flags=0x7fffffffd4bc, cf_mask=4278190080) at
src/tcg/../../include/exec/tb-lookup.h:44
#8  0x00005555555cc5a1 in tb_find (cpu=0x55555596ea80 <__x86_cpu>, last_tb=0x0, tb_exit=0, cf_mask=0) at src/tcg/cpu-exec.c:285
#9  0x00005555555cd0a6 in cpu_exec (cpu=0x55555596ea80 <__x86_cpu>) at src/tcg/cpu-exec.c:559
#10 0x000055555561afb8 in tcg_cpu_exec (cpu=0x55555596ea80 <__x86_cpu>) at src/qemu/cpus.c:122
#11 0x000055555561b22c in qemu_tcg_rr_cpu_thread_fn (arg=0x55555596ea80 <__x86_cpu>) at src/qemu/cpus.c:235
#12 0x00005555555c7e29 in qemu_boot () at /home/maritns3/core/ld/DuckBuBi/include/sysemu/sysemu.h:34
#13 0x00005555555c8fb4 in test_qemu_init () at src/main.c:159
#14 0x00005555555c926d in wip () at src/main.c:173
#15 0x00005555555ca124 in greatest_run_suite (suite_cb=0x5555555c9215 <wip>, suite_name=0x5555556da6d7 "wip") at src/main.c:176
#16 0x00005555555cb1ee in main (argc=1, argv=0x7fffffffd728) at src/main.c:185
```

```c
#0  0x0000555555619baf in cpu_physical_memory_test_and_clear_dirty (start=1044480, length=4096, client=1) at src/qemu/exec.c:334
#1  0x00005555555d863a in tlb_protect_code (ram_addr=1044480) at src/tcg/cputlb.c:515
#2  0x00005555555d3f78 in tb_page_add (p=0x5555559e03c8, tb=0x70000580, n=0, page_addr=1044480) at src/tcg/translate-all.c:1548
#3  0x00005555555d4074 in tb_link_page (tb=0x70000580, phys_pc=1048560, phys_page2=18446744073709551615) at src/tcg/translate-all.c:1591
#4  0x00005555555d46f9 in tb_gen_code (cpu=0x55555596e980 <__x86_cpu>, pc=4294967280, cs_base=4294901760, flags=64, cflags=-16777216) at src/tcg/translate-all.c:1773
#5  0x00005555555cc611 in tb_find (cpu=0x55555596e980 <__x86_cpu>, last_tb=0x0, tb_exit=0, cf_mask=0) at src/tcg/cpu-exec.c:290
#6  0x00005555555cd0d0 in cpu_exec (cpu=0x55555596e980 <__x86_cpu>) at src/tcg/cpu-exec.c:560
#7  0x000055555561b075 in tcg_cpu_exec (cpu=0x55555596e980 <__x86_cpu>) at src/qemu/cpus.c:122
#8  0x000055555561b2e9 in qemu_tcg_rr_cpu_thread_fn (arg=0x55555596e980 <__x86_cpu>) at src/qemu/cpus.c:235
#9  0x00005555555c7e29 in qemu_boot () at /home/maritns3/core/ld/DuckBuBi/include/sysemu/sysemu.h:34
#10 0x00005555555c8fb4 in test_qemu_init () at src/main.c:159
#11 0x00005555555c926d in wip () at src/main.c:173
#12 0x00005555555ca124 in greatest_run_suite (suite_cb=0x5555555c9215 <wip>, suite_name=0x5555556da6d7 "wip") at src/main.c:176
#13 0x00005555555cb1ee in main (argc=1, argv=0x7fffffffd728) at src/main.c:185
```
