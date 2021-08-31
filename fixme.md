## 主要需要分析的问题
重构的函数全部使用 [interface num] 标准出来, 然后在一个文件中专门索引为何如此设计

5. 关键的接口和重构

| TODO                       | 描述                                                                                                                                                                                           |
|----------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| apic                       | include/hw/i386/apic.h 中定义了一些列的函数，具体的还没有分析，但是 原来的 QEMU 中间                                                                                                           |
| 多核                       | 现在为了代码的方便执行，也没有太搞清楚其中的作用，在 cputlb.c 的 async_run_on_cpu 以及各种 atomic 函数，qemu_spin_lock, 以及 RCU. 现在的操作是，先将接口保存下来，之后需要支持多核，有一个参考 |
| icount / record replay     | 没有 record replay 是不是很难调试，使用 record replay 会不会很难集成                                                                                                                           |
| trace                      |                                                                                                                                                                                                |
| 存在好几个数据结构需要重构 |qht.h 和 glib 的 qtree                                                                                                                                                                |

1. memory model
    1. include/exec/ram_addr.h
    3. memory.h
    5. /home/maritns3/core/ld/DuckBuBi/include/exec/address-space.h 中一堆接口
    6. 实际上，io_readx 和 io_writex 中间的还是存在一些骚操作的啊
    8. 在 translate-all.c 中的一些逻辑需要仔细分析一下
        - page_unlock : 根本不理解为什么会出现将 page lock 的操作
    9. 4k 和 16k 页的问题如何解决 ?
    10. 在地址上，有没有什么骚操作:
        1. TB 需要走 TLB 吗 ? 这样就可以进行自动属性检查
        2. 但是核心代码不走 TLB 从而加速一下
    11. 思考一下用户态和内核态的安全性如何
        1. 地址空间的防护
            - 在进程里面，访问总是自己的地址空间，如果 TLB 没有，那么让 QEMU page walk, 如果 walk 没有就是 page fault, 所以其实问题不大
        2. 生成指令
            - 生成的时候，就可以产生 exception 还是到执行 ？(精确异常，所以执行的时候吧)
    12. 分析 cpu-ldst.h 一方面重构为 v6.0 的做法，同时分析 mmuidx 的具体实现啊
    1. 移除掉 softmmu.c 之类的对于 memory model 的依赖 (主要出现在 include/exec/memory.h 中)
    1. 重新构建 iotlb
    1. understand how smm works in tcg
    1. CPUAddressSpace : 这是 tcg 下单独搞出来的吗 ?
    1. exec/ram_addr.h : 定义了几个 physical memory dirty 相关的问题
    2. stl_le_phys : 这个在 memory_ldst.c 中并没有配套的实现
2. apic
    1. DeviceState 中定义为空
    2. /home/maritns3/core/ld/DuckBuBi/include/hw/i386/apic.h 都是空函数
    3. cpu_get_pic_interrupt 定义在 pc.c 中间了
    4. qemu_irq_raise : 在 fpu_raise_exception 中需要被调用
    5. hw/irq.h 中的东西似乎只会被 fpu_helper 使用，其他的位置在哪里呀
    6. tcg-all.c : 涉及到初始化 tcg engine, 很短的一个文件
        - 虽然主要是处理 interrupt 的，也是很麻烦的啊
    7. /home/maritns3/core/ld/DuckBuBi/src/i386/excp_helper.c 中为什么有那么多的处理 exception 的函数
    8. qemu_irq_raise
3. locks
    1. include/sysemu/cpus.h : 定义的为空函数啊
    1. qemu_mutex_lock : 在 qemu_mutex_lock 只会出现在 tcg.c 这是 QEMU 的失误吗 ?
      - 关注一下，为什么单独这里是需要处理 lock 的
    2. helper_atomic_cmpxchgq_le_mmu : 这个最后会导入一个很烦人的 lock 中间去
    3. big qemu lock
    4. include/qemu/atomic.h : 这里定义了一堆空函数
    1. 感觉 Qemu 关于多核的接口不是很统一啊
        1. QemuSpin
        2. QemuMutex
        3. qemu_spin_lock
        4. 在 include/qemu 下存在 thread.h thread-posix.h 等
        6. lockable
    2. 那些数据结构需要 RCU 来保护 
    4. 统计一下 `__thread` 出现的次数
    1. atomic 机制谁在使用啊
    2. qemu_mutex 机制的出现位置
    4. 多核机制分析其实不仅仅在于此
      - cpus.h
      - CPUState 中的 cpu_index, cluster_index 等
    1. thread.h 中间的接口需要补齐
    2. cpus.h 中间接口
    3. first_cpu / CPU_NEXT / CPU_NEXT
    1. TCG region : 比如 tcg_tb_lookup
    1. TB hash table : hash table RCU 机制
    1. iothread
    1. async_run_on_cpu
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
    7. qemu_log_mask_and_addr : 和 qemu_log_mask 的区别在于只有在地址范围内才会打印日志
6. 处理一下满天飞羽的各种 type 定义
    1. ExtraBlock 是 LATX 中特殊需求的，其中 `_top_in` 的类型是 int8, 而 int8 又是定义在 src/i386/LATX/include/types.h 中间了，着很难受啊
    2. HOST_LONG_BITS 和 TARGET_LONG_BITS 的定义位置
    3. target_long 的定义
7. log printf 和 qemu printf
    1. include/qemu/log-for-trace.h 和 include/qemu/log-for-trace.h : 一起搞出来 mask 基于日志的角度
    2. qemu_printf 和 printf 的区别在于当前是否存在 monitor
8. 非常不统一的 assert
  - [ ] tcg_debug_assert
  - [ ] assert
  - [ ] 一些 unreachable 之类的
  - [ ] tcg_abort
  - [ ] 在 bitmap.c 中间是直接从
  - [ ] g_assert
  - error_report
9. pit 和 hpet 需要模拟?

## 移植方案
| function                          | 作用                                                                                                          | 方案 |
|-----------------------------------|---------------------------------------------------------------------------------------------------------------|------|
| address_space_translate_for_iotlb | 根据 addr 得到 memory region 的                                                                               |      |
| memory_region_section_get_iotlb   | 计算出来当前的 section 是 AddressSpaceDispatch 中的第几个 section, 之后就可以通过 addr 获取 section 了        |      |
| qemu_map_ram_ptr                  | 这是一个神仙设计的接口，如果参数 ram_block 的接口为 NULL, 那么 addr 是 ram addr， 如果不是，那么是 ram 内偏移 |      |

flush 的函数的异步运行其实可以好好简化一下。

实际上整个 ram_addr.h 都是处理 dirty page 的问题，而至于 RAMBlock 的概念具体如何设计，
需要等到之后在处理。
