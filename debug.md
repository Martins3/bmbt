# bug 1
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
