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
```c
/*
#0  timerlist_notify (timer_list=0x120430470) at src/util/qemu-timer.c:111
#1  0x0000000120094a9c in timerlist_rearm (timer_list=0x120430470) at src/util/qemu-timer.c:119
#2  0x0000000120094bb0 in timer_mod_ns (ts=0x12044c2f0, expire_time=1636358497725530000) at src/util/qemu-timer.c:134
#3  0x0000000120094c20 in timer_mod (ts=0x12044c2f0, expire_time=1636358497725530000) at src/util/qemu-timer.c:139
#4  0x000000012004f85c in check_update_timer (s=0x12039d580 <__mc146818_rtc>) at src/hw/rtc/mc146818rtc.c:157
#5  0x0000000120052068 in rtc_realizefn (s=0x12039d580 <__mc146818_rtc>) at src/hw/rtc/mc146818rtc.c:778
#6  0x0000000120052248 in mc146818_rtc_init (base_year=2000, intercept_irq=0x12044b9f0) at src/hw/rtc/mc146818rtc.c:832
#7  0x000000012003d9f0 in pc_init1 (machine=0x1203a5f00 <__pcms>, host_type=0x12019b978 "i440FX-pcihost", pci_type=0x12019b970 "i440FX") at src/hw/i386/pc_piix.c:171
#8  0x000000012003db40 in pc_init_v4_2 (machine=0x1203a5f00 <__pcms>) at src/hw/i386/pc_piix.c:272
#9  0x0000000120048eb4 in machine_run_board_init (machine=0x1203a5f00 <__pcms>) at src/hw/core/machine.c:214
#10 0x0000000120086ecc in qemu_init () at src/qemu/vl.c:237
```
timerlist_notify 使用的是发送信号，但是，此时从 signal handler 使用下面的方法获取 timer_id 就是一个错误的操作了。

```c
  timer_t *tidptr = si->si_value.sival_ptr;
```
在 85ceb76 被修复掉了。

## bug 3

正确的:
```plain
[test-trace before] EIP=0xfff0 / CS_BASE=0xffff0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0,0,0x663,0,0,0,0,0
[test-trace after] EIP=0xe05b / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0,0,0x663,0,0,0,0,0

[test-trace before] EIP=0xe05b / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0,0,0x663,0,0,0,0,0
[test-trace after] EIP=0xe062 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0x663,0,0,0,0,0

[test-trace before] EIP=0xe062 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0x663,0,0,0,0,0
[test-trace after] EIP=0xe066 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0x663,0,0,0,0,0

[test-trace before] EIP=0xe066 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0x663,0,0,0,0,0
[test-trace after] EIP=0xe068 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0,0,0,0,0,0

[test-trace before] EIP=0xe068 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0,0,0,0,0,0
[test-trace after] EIP=0xe06a / CS_BASE=0xf0000 / FLAGS=0x48 / EFLAGS=0x46 / REGS=0,0,0,0,0,0,0,0

[test-trace before] EIP=0xe06a / CS_BASE=0xf0000 / FLAGS=0x48 / EFLAGS=0x46 / REGS=0,0,0,0,0,0,0,0
[test-trace after] EIP=0xe070 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0,0,0x7000,0,0,0

[test-trace before] EIP=0xe070 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0,0,0x7000,0,0,0
[test-trace after] EIP=0xe076 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0

[test-trace before] EIP=0xe076 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace after] EIP=0xcf24 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0

[test-trace before] EIP=0xcf24 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace after] EIP=0xcf25 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace before] EIP=0xcf25 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace after] EIP=0xcf26 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace before] EIP=0xcf26 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace after] EIP=0xcf29 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace before] EIP=0xcf29 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace after] EIP=0xcf2f / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0x8f,0,0xf01a2,0,0x7000,0,0,0
[test-trace before] EIP=0xcf2f / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0x8f,0,0xf01a2,0,0x7000,0,0,0

[test-trace after] EIP=0xcf31 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0x8f,0,0xf01a2,0,0x7000,0,0,0
[test-trace before] EIP=0xcf31 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0x8f,0,0xf01a2,0,0x7000,0,0,0
[test-trace after] EIP=0xcf33 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace before] EIP=0xcf33 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace after] EIP=0xcf35 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace before] EIP=0xcf35 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace after] EIP=0xcf37 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0x2,0,0xf01a2,0,0x7000,0,0,0
[test-trace before] EIP=0xcf37 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0x2,0,0xf01a2,0,0x7000,0,0,0
[test-trace after] EIP=0xcf39 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0x2,0,0xf01a2,0,0x7000,0,0,0
[test-trace before] EIP=0xcf39 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0x2,0,0xf01a2,0,0x7000,0,0,0
[test-trace after] EIP=0xcf3c / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace before] EIP=0xcf3c / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace after] EIP=0xcf42 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace before] EIP=0xcf42 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace after] EIP=0xcf48 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace before] EIP=0xcf48 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace after] EIP=0xcf4b / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0,0x60000010,0xf01a2,0,0x7000,0,0,0
```

错误的
```plain
// 第一条指令
[test-trace before] EIP=0xfff0 / CS_BASE=0xffff0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0,0,0x663,0,0,0,0,0
[test-trace after] EIP=0xe05b / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0,0,0x663,0,0,0,0,0

// cmpl : 访问代码以及访问 HaveRunPost，所以存在两个
tlb_set_page_with_attrs: vaddr=000fe000 paddr=0x00000000000fe000 prot=7 idx=2
[test-trace before] EIP=0xe05b / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x2 / REGS=0,0,0x663,0,0,0,0,0
tlb_set_page_with_attrs: vaddr=000f6000 paddr=0x00000000000f6000 prot=7 idx=2
[test-trace after] EIP=0xe062 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0x663,0,0,0,0,0

// jnz
[test-trace before] EIP=0xe062 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0x663,0,0,0,0,0
[test-trace after] EIP=0xe066 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0x663,0,0,0,0,0

// xorw
[test-trace before] EIP=0xe066 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0x663,0,0,0,0,0
[test-trace after] EIP=0xe068 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0,0,0,0,0,0

// movw
[test-trace before] EIP=0xe068 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0,0,0,0,0,0
[test-trace after] EIP=0xe06a / CS_BASE=0xf0000 / FLAGS=0x48 / EFLAGS=0x46 / REGS=0,0,0,0,0,0,0,0

// movl $ BUILD_STACK_ADDR , %esp
[test-trace before] EIP=0xe06a / CS_BASE=0xf0000 / FLAGS=0x48 / EFLAGS=0x46 / REGS=0,0,0,0,0,0,0,0
[test-trace after] EIP=0xe070 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0,0,0x7000,0,0,0

// movl $ \cfunc , %edx
[test-trace before] EIP=0xe070 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0,0,0x7000,0,0,0
[test-trace after] EIP=0xe076 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0

// jmp transition32
[test-trace before] EIP=0xe076 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace after] EIP=0xcf24 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0

tlb_set_page_with_attrs: vaddr=000fc000 paddr=0x00000000000fc000 prot=7 idx=2
[test-trace before] EIP=0xcf24 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace after] EIP=0xcf25 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0

[test-trace before] EIP=0xcf25 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace after] EIP=0xcf26 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0

[test-trace before] EIP=0xcf26 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace after] EIP=0xcf29 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0

[test-trace before] EIP=0xcf29 / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0,0,0xf01a2,0,0x7000,0,0,0
[test-trace after] EIP=0xcf2f / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0x8f,0,0xf01a2,0,0x7000,0,0,0

[test-trace before] EIP=0xcf2f / CS_BASE=0xf0000 / FLAGS=0x40 / EFLAGS=0x46 / REGS=0x8f,0,0xf01a2,0,0x7000,0,0,0
```

对应的源代码:
```asm
        // Reset stack, transition to 32bit mode, and call a C function.
        .macro ENTRY_INTO32 cfunc
        xorw %dx, %dx
        movw %dx, %ss
        movl $ BUILD_STACK_ADDR , %esp
        movl $ \cfunc , %edx
        jmp transition32
        .endm
```

```asm
entry_post:
        cmpl $0, %cs:HaveRunPost                // Check for resume/reboot
        jnz entry_resume
        ENTRY_INTO32 _cfunc32flat_handle_post   // Normal entry point

        ORG 0xe2c3
```

```asm
reset_vector:
        ljmpw $SEG_BIOS, $entry_post

        // 0xfff5 - BiosDate in misc.c

        // 0xfffe - BiosModelId in misc.c

        // 0xffff - BiosChecksum in misc.c

        .end
```
好吧，是我看错了，根本就没有 bug，之前对于 seabios 的执行流程理解错了。
