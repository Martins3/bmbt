## tech

gdb out/rom16.o

## bug 1
```plain
tlb_set_page_with_attrs: vaddr=fffff000 paddr=0x00000000fffff000 prot=7 idx=2
[test-trace] EIP=0xfff0 / CS_BASE=0xffff0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0,0,0x663,0,0,0,0,0
[test-trace] EIP=0xe05b / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0,0,0x663,0,0,0,0,0
tlb_set_page_with_attrs: vaddr=000fe000 paddr=0x00000000000fe000 prot=7 idx=2
[test-trace] EIP=0xe05b / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0,0,0x663,0,0,0,0,0
tlb_set_page_with_attrs: vaddr=00009000 paddr=0x0000000000009000 prot=7 idx=2

Program received signal SIGSEGV, Segmentation fault.
0x000000012001dfc0 in ldub_p (ptr=0xffefbd9c24) at src/tcg/../../include/exec/../qemu/bswap.h:146
146     static inline int ldub_p(const void *ptr) { return *(uint8_t *)ptr; }
(gdb) lg
Undefined command: "lg".  Try "help".
(gdb) q
A debugging session is active.

        Inferior 1 [process 713] will be killed.

Quit anyway? (y or n) n
Not confirmed.
(gdb) bt
#0  0x000000012001dfc0 in ldub_p (ptr=0xffefbd9c24) at src/tcg/../../include/exec/../qemu/bswap.h:146
#1  0x0000000120025b10 in load_memop (op=MO_8, haddr=0xffefbd9c24) at src/tcg/cputlb.c:1382
#2  load_helper (full_load=0x120024df8 <full_ldub_mmu>, code_read=false, op=MO_8, retaddr=1879050300, oi=66, addr=39972, env=0x1203f20a0 <__x86_cpu+34784>)
    at src/tcg/cputlb.c:1562
#3  full_ldub_mmu (env=0x1203f20a0 <__x86_cpu+34784>, addr=39972, oi=66, retaddr=1879050300) at src/tcg/cputlb.c:1577
#4  0x0000000120025d50 in helper_ret_ldub_mmu (env=0x1203f20a0 <__x86_cpu+34784>, addr=39972, oi=66, retaddr=1879050300) at src/tcg/cputlb.c:1582
#5  0x000000012002bbc4 in helper_ret_ldsb_mmu (env=0x1203f20a0 <__x86_cpu+34784>, addr=39972, oi=66, retaddr=1879050300) at src/tcg/cputlb.c:1669
#6  0x0000000070001074 in ?? ()
```
为啥然后立刻就跳转到 vaddr=00009000 ?

因为 xlat 的地址偏移计算错误了，在 34b51de58fdd85c199f3784adc8b3064a2bf0596 中修复了。

## bug 2
```plain
// ...
tlb_flush_by_mmuidx_async_work: mmu_idx:0x0007
locked src/qemu/cpus.c:158
huxueshi:tb_htable_lookup fffffff0
tlb_set_page_with_attrs: vaddr=fffff000 paddr=0x00000000fffff000 prot=7 idx=2
[test-trace] EIP=0xfff0 / CS_BASE=0xffff0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0,0,0x663,0,0,0,0,0
[test-trace] EIP=0xe05b / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0,0,0x663,0,0,0,0,0
huxueshi:tb_htable_lookup fe05b
tlb_set_page_with_attrs: vaddr=000fe000 paddr=0x00000000000fe000 prot=7 idx=2
[test-trace] EIP=0xe05b / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0,0,0x663,0,0,0,0,0
tlb_set_page_with_attrs: vaddr=000f6000 paddr=0x00000000000f6000 prot=7 idx=2
[test-trace] EIP=0xe066 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0x663,0,0,0,0,0
huxueshi:tb_htable_lookup fe066
[test-trace] EIP=0xe066 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0x663,0,0,0,0,0
[test-trace] EIP=0xe06a / CS_BASE=0xf0000 / FLAGS=0x48 / EFLAGS=0x46 / REGS=0,0,0,0,0,0,0,0
huxueshi:tb_htable_lookup fe06a
[test-trace] EIP=0xe06a / CS_BASE=0xf0000 / FLAGS=0x48 / EFLAGS=0x46 / REGS=0,0,0,0,0,0,0,0
[test-trace] EIP=0xe070 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0,0,0x7000,0,0,0
huxueshi:tb_htable_lookup fe070
[test-trace] EIP=0xe070 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0,0,0x7000,0,0,0
[test-trace] EIP=0xcf24 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
huxueshi:tb_htable_lookup fcf24
tlb_set_page_with_attrs: vaddr=000fc000 paddr=0x00000000000fc000 prot=7 idx=2
[test-trace] EIP=0xcf24 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
guest ip : fcf24
failed in [io dispatch] with offset=[70]
**
ERROR:src/qemu/memory.c:105:memory_region_look_up: code should not be reached
make: *** [Makefile:146: run] Aborted
```
actually, doesn't jump to fcf24

```c
Line 322 of "./src/post.c" starts at address 0xee770 <handle_post> and ends at 0xee774 <handle_post+4>.
```

```plain
Line 25 of "./src/resume.c" starts at address 0xc4da <handle_resume> and ends at 0xc4e0 <handle_resume+6>.
```

```c
/*
#0  huxueshi () at src/qemu/memory.c:93
#1  0x0000000120084e60 in memory_region_look_up (dispatch=0x1203111c0 <__io_dispatch>, offset=112, mr_match=0x120084cc4 <io_mr_match>) at src/qemu/memory.c:105
#2  0x0000000120084fbc in io_mr_look_up (as=0x120417410 <address_space_io>, offset=112, xlat=0xffffff2dc8, plen=0xffffff2dd0) at src/qemu/memory.c:119
#3  0x0000000120085af8 in address_space_translate (as=0x120417410 <address_space_io>, addr=112, xlat=0xffffff2dc8, len=0xffffff2dd0, is_write=true, attrs=...)
    at src/qemu/memory.c:244
#4  0x0000000120016190 in address_space_stb (as=0x120417410 <address_space_io>, addr=112, val=143, attrs=..., result=0x0) at src/tcg/memory_ldst.c:301
#5  0x00000001200a9530 in helper_outb (env=0x1203f21d0 <__x86_cpu+34784>, port=112, data=143) at src/i386/misc_helper.c:21
#6  0x0000000070000f9c in ?? ()
```
