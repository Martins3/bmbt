## 主要需要分析的问题

1. memory model
    1. include/sysemu/cpus.h : 定义的为空函数啊
    2. include/exec/ram_addr.h
    3. memory.h
    4. 在 exec-all.h 中间 memory_region_section_get_iotlb
    5. /home/maritns3/core/ld/DuckBuBi/include/exec/address-space.h 中一堆接口
    6. 实际上，io_readx 和 io_writex 中间的还是存在一些骚操作的啊
2. apic
    1. DeviceState 中定义为空
    2. /home/maritns3/core/ld/DuckBuBi/include/hw/i386/apic.h 都是空函数
    3. cpu_get_pic_interrupt 定义在 pc.c 中间了
    4. qemu_irq_raise : 在 fpu_raise_exception 中需要被调用
    5. hw/irq.h 中的东西似乎只会被 fpu_helper 使用，其他的位置在哪里呀
3. locks
    1. qemu_mutex_lock : 在 qemu_mutex_lock 只会出现在 tcg.c 这是 QEMU 的失误吗 ?
      - 关注一下，为什么单独这里是需要处理 lock 的
    2. helper_atomic_cmpxchgq_le_mmu : 这个最后会导入一个很烦人的 lock 中间去
    3. big qemu lock
    4. include/qemu/atomic.h : 这里定义了一堆空函数
4. icount 机制
    1. cpu_exec
    2. TranslationBlock::icount
    3. 在 CPUX86State::neg 似乎也有这个东西
    4. 在 /home/maritns3/core/ld/DuckBuBi/include/sysemu/replay.h 中间定义一些 reply 相关的函数，和 icount 到底是什么关系
    5. CF_USE_ICOUNT ?
    6. use_icount : 这个变量的解释似乎很清晰，不过可以和具体的效果对比一下
5. log debug 和 trace : 其实暂时可以补全的, 都是一些 printf 而已
    1. tlb_debug
    2. do_tb_flush 中间又是直接使用 printf 的
6. 处理一下满天飞羽的各种 type 定义
    1. ExtraBlock 是 LATX 中特殊需求的，其中 `_top_in` 的类型是 int8, 而 int8 又是定义在 src/i386/LATX/include/types.h 中间了，着很难受啊
    2. HOST_LONG_BITS 和 TARGET_LONG_BITS 的定义位置
    3. target_long 的定义
7. log printf 和 qemu printf
    1. include/qemu/log-for-trace.h 和 include/qemu/log-for-trace.h : 一起搞出来 mask 基于日志的角度
    2. qemu_printf 和printf 的区别在于当前是否存在 monitor

## 一些简单的代码分析工作
1. --enable-x86tomips-flag-int 是干什么的 ?
2. cpu_cc_compute_all 是做什么 ?
3. cpu_is_bsp
4. do_cpu_sipi
5. do_cpu_init
6. tcg.c 中间的
    - tcg_context_init : 这里初始化了一堆 tcg_op ，不是很确定这个是否真的有用的
7. qemu_log_mask_and_addr
8. cpu_exec_nocache : 为什么需要将所有的 tb 清空然后来运行
10. `__builtin___clear_cache` 在系统态还可以用吗 ?
11. tb_gen_code 是如何运行的 ?
12. bswap.h 中间的，当 CONFIG_MACHINE_BSWAP_H 可以调查一下 C 库中间是否存在 bswap 的支持，因为是用于 reference C 库的
13. cpu_unaligned_access : x86 对应的 handler 没有赋值啊
14. how cross page works?
      - cross-page-check.h 算是少数从 LATX 入侵到公共部分的代码了
15. why is mmap_lock empty in system mode? In another word, why mmap_lock is necessary for user mode ? 实际上，mmap 的使用位置相当有限
20. [ ] 有件事情没有想明白，调用 helper 的时候就进入到 qemu 中间了，是什么时候调用的 prologue 的离开 tb 执行的环境的。(heler_inw 之类的) (写一个进入 tb 环境 和 离开的小专题，顺便分析一下如何是 setjmp 的使用方法)
     - Niugenen 说切到 helper 这里实际上取决于是否破坏环境，有的不用处理的
21. tb_jmp_cache 是个啥
    - [ ] tb_flush_jmp_cache
