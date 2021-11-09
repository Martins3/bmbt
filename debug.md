## tech

<!-- vim-markdown-toc GitLab -->

- [bug 1](#bug-1)
- [bug 2](#bug-2)
- [bug 3](#bug-3)
- [bug 4](#bug-4)
- [bug 5](#bug-5)
- [bug 6](#bug-6)
- [bug 7](#bug-7)

<!-- vim-markdown-toc -->

> Then, in another session, run gdb with either out/rom16.o (to debug
> bios 16bit code) or out/rom.o (to debug bios 32bit code). For example:
>
> `gdb out/rom16.o`

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

## bug 4
本来以为时钟都是 1s 之内的，但是实际上，那是一个错误的理解。


就现在(fd99fdcf94cbe9dd4d472f130eea5aff52607a4c)而言，
运行的 timer 只有 rtc 。

```plain
huxueshi:timer_interrupt_handler                        <- 进入 signal handler
huxueshi:timerlist_run_timers [run callback]            <- 首先运行  callback
huxueshi:check_update_timer                             <- callback 为 check_update_timer
huxueshi:timer_mod_ns_locked 1636500309034373000        <- check_update_timer 要求之后的时间
timerlist_deadline_ns: delta=77512154308000
timerlist_deadline_ns: expire_time=1636500309034373000
timerlist_deadline_ns: now=1636422796880151000
huxueshi:timerlistgroup_deadline_ns 0 77512154308000
huxueshi:timerlistgroup_deadline_ns 1 77512154308000
huxueshi:timerlistgroup_deadline_ns 2 77512154308000
huxueshi:timerlistgroup_deadline_ns 3 77512154308000
huxueshi:soonest_timer 77512
```

如果不去添加调试，那么可以得到下面的内容:
```plain
huxueshi:timer_interrupt_handler
timerlist_deadline_ns: delta=999236000
timerlist_deadline_ns: expire_time=1636424963515329000
timerlist_deadline_ns: now=1636424962516099000
huxueshi:timerlistgroup_deadline_ns 0 999236000
huxueshi:timerlistgroup_deadline_ns 1 999236000
huxueshi:timerlistgroup_deadline_ns 2 999236000
huxueshi:timerlistgroup_deadline_ns 3 999236000
huxueshi:soonest_timer 999236000
huxueshi:timer_interrupt_handler
timerlist_deadline_ns: delta=999198000
timerlist_deadline_ns: expire_time=1636424963515329000
timerlist_deadline_ns: now=1636424962516135000
huxueshi:timerlistgroup_deadline_ns 0 999198000
huxueshi:timerlistgroup_deadline_ns 1 999198000
huxueshi:timerlistgroup_deadline_ns 2 999198000
huxueshi:timerlistgroup_deadline_ns 3 999198000
huxueshi:soonest_timer 999198000
huxueshi:timer_interrupt_handler
timerlist_deadline_ns: delta=999177000
timerlist_deadline_ns: expire_time=1636424963515329000
timerlist_deadline_ns: now=1636424962516157000
huxueshi:timerlistgroup_deadline_ns 0 999177000
huxueshi:timerlistgroup_deadline_ns 1 999177000
huxueshi:timerlistgroup_deadline_ns 2 999177000
huxueshi:timerlistgroup_deadline_ns 3 999177000
huxueshi:soonest_timer 999177000
```

之所以 timer_interrupt_handler 在被这么快速的触发

```c
> 1636424962516157000 - 1636424962516135000
ans = 22016.0
```
不是 timer 到时间了，而是因为 timerlist_notify 的原因

和我们预想的一样，之所以现在这个问题没有出现，是因为
程序太快的就结束了，没有运行到 check_update_timer 中触发错误的位置。

check_update_timer 的 timer_mod 并不是一定会触发错误的。

真正的原因在于: get_next_alarm 返回的时间就是 73412

想不到吧，这就是正确的返回值

## bug 5
当 seabios 访问 stack 的时候

```plain
tlb_set_page_with_attrs: vaddr=00006000 paddr=0x0000000000006000 prot=7 idx=2
```
在 store_helper 中将会调用 notdirty_write 的

在原来的 QEMU 中，是会同时管理三种 dirty code 的，其调用

cpu_physical_memory_set_dirty_range(ram_addr, size, DIRTY_CLIENTS_NOCODE);

的时候，被其中的 g_assert_not_reached 检查到了

## bug 6
```c
/*
#0  0x000000012001e204 in stl_he_p (ptr=0xffefbd6ffc, v=0) at src/tcg/../../include/exec/../qemu/bswap.h:185
#1  0x000000012001e4e0 in stl_le_p (ptr=0xffefbd6ffc, v=0) at src/tcg/../../include/exec/../qemu/bswap.h:219
#2  0x0000000120032448 in store_memop (op=MO_32, val=0, haddr=0xffefbd6ffc) at src/tcg/cputlb.c:1723
#3  store_helper (op=MO_32, retaddr=1879061000, oi=34, val=0, addr=28668, env=0x1203fce70 <__x86_cpu+34752>) at src/tcg/cputlb.c:1836
#4  helper_le_stl_mmu (env=0x1203fce70 <__x86_cpu+34752>, addr=28668, val=0, oi=34, retaddr=1879061000) at src/tcg/cputlb.c:1974
```

这是 guest 的 hva
huxueshi:ram_init host ram start 0xffec4a4000

这是访问的地址:
huxueshi:store_memop 0xffec4aaffc

访问的地址正好是:
6ffc

所以访问的地址没有错误的，最后检查到是 mmap 的时候权限不对。

## bug 7
guest 的输出是:

guest ip : ebd4a
failed in [io dispatch] with offset=[402]

但是 debugcon_isa_properties 中定义的 ISADebugconState 的 iobase 是 0xe9 的呀
好吧，那是参数确定的!

而且这里的 guest ip 为什么无法 disass 出来啊

```txt
Dump of assembler code for function handle_post:
   0x000ee770 <+0>:       push   %ebx
   0x000ee771 <+1>:       sub    $0x20,%esp
   0x000ee774 <+4>:       call   0xeaba3 <debug_banner>
   0x000ee779 <+9>:       mov    $0x40000000,%ebx
   0x000ee77e <+14>:      lea    0xc(%esp),%eax
   0x000ee782 <+18>:      push   %eax
   0x000ee783 <+19>:      lea    0xc(%esp),%eax
   0x000ee787 <+23>:      push   %eax
   0x000ee788 <+24>:      lea    0xc(%esp),%ecx
   0x000ee78c <+28>:      lea    0x8(%esp),%edx
   0x000ee790 <+32>:      mov    %ebx,%eax
   0x000ee792 <+34>:      call   0xe8cb1 <cpuid>
   0x000ee797 <+39>:      mov    0xc(%esp),%eax
   0x000ee79b <+43>:      mov    %eax,0x1b(%esp)
   0x000ee79f <+47>:      mov    0x10(%esp),%eax
   0x000ee7a3 <+51>:      mov    %eax,0x1f(%esp)
   0x000ee7a7 <+55>:      mov    0x14(%esp),%eax
   0x000ee7ab <+59>:      mov    %eax,0x23(%esp)
   0x000ee7af <+63>:      movb   $0x0,0x27(%esp)
   0x000ee7b4 <+68>:      mov    $0xf4006,%edx
   0x000ee7b9 <+73>:      lea    0x1b(%esp),%eax
   0x000ee7bd <+77>:      call   0xe8c3e <strcmp>
   0x000ee7c2 <+82>:      pop    %edx
   0x000ee7c3 <+83>:      pop    %ecx
   0x000ee7c4 <+84>:      test   %eax,%eax
   0x000ee7c6 <+86>:      jne    0xee809 <handle_post+153>
   0x000ee7c8 <+88>:      call   0xe99d9 <code_mutable_preinit>
   0x000ee7cd <+93>:      movw   $0xe9,0xf53f4
   0x000ee7d6 <+102>:     call   0xeaba3 <debug_banner>
   0x000ee7db <+107>:     push   %ebx
   0x000ee7dc <+108>:     push   $0xf4013
   0x000ee7e1 <+113>:     call   0xeab31 <__dprintf>
   0x000ee7e6 <+118>:     mov    0x8(%esp),%eax
   0x000ee7ea <+122>:     mov    %eax,%edx
   0x000ee7ec <+124>:     sub    %ebx,%edx
   0x000ee7ee <+126>:     pop    %ecx
   0x000ee7ef <+127>:     pop    %ecx
   0x000ee7f0 <+128>:     cmp    $0x1,%edx
   0x000ee7f3 <+131>:     ja     0xee801 <handle_post+145>
   0x000ee7f5 <+133>:     push   %ebx
   0x000ee7f6 <+134>:     push   %eax
   0x000ee7f7 <+135>:     push   $0xf403a
   0x000ee7fc <+140>:     call   0xea62b <panic>
   0x000ee801 <+145>:     mov    %ebx,0xf45b8
   0x000ee807 <+151>:     jmp    0xee831 <handle_post+193>
   0x000ee809 <+153>:     add    $0x100,%ebx
   0x000ee80f <+159>:     cmp    $0x40010000,%ebx
   0x000ee815 <+165>:     jne    0xee77e <handle_post+14>
   0x000ee81b <+171>:     cmpl   $0x0,0xf45b8
   0x000ee822 <+178>:     jne    0xee831 <handle_post+193>
   0x000ee824 <+180>:     push   $0xf406c
   0x000ee829 <+185>:     call   0xeab31 <__dprintf>
   0x000ee82e <+190>:     pop    %eax
   0x000ee82f <+191>:     jmp    0xee83b <handle_post+203>
   0x000ee831 <+193>:     movl   $0x3,0xf50c0
   0x000ee83b <+203>:     call   0xec551 <make_bios_writable>
   0x000ee840 <+208>:     call   0xe7171 <dopost>
```

```txt
Dump of assembler code for function debug_banner:
   0x000eaba3 <+0>:       push   $0xf4574
   0x000eaba8 <+5>:       push   $0xef07a
   0x000eabad <+10>:      call   0xeab31 <__dprintf>
   0x000eabb2 <+15>:      push   $0xf4520
   0x000eabb7 <+20>:      push   $0xef090
   0x000eabbc <+25>:      call   0xeab31 <__dprintf>
   0x000eabc1 <+30>:      add    $0x10,%esp
   0x000eabc4 <+33>:      ret
```
