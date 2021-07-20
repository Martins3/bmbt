# Divergence between qemu

## 几个关键的结构体功能和移植差异说明
- [ ] 暂时无法区分 CPUX86State 和 X86CPU 的关系

| struct           | explaination                                                                            |
|------------------|-----------------------------------------------------------------------------------------|
| CPUClass         | 在函数 x86_cpu_common_class_init 中已经知道注册的函数, 可以将其直接定义为一个静态函数集 |
| CPUState         |                                                                                         |
| CPUX86State      | 和 CPUState 没有父子关系，而是靠 CPUState::env_ptr 决定的 |
| X86CPU           |                                                                                         |
| TranslationBlock | 1. size x86 代码的 size 还是 la 代码段的 size </br> 2.                                  |
| TBContext        | 一个统计，一个 qht, 似乎只是定义了一个全局变量                                                                        |
| TCGContext       | TODO 每一个 thread 定义了一个，同时存在一个全局的 tcg_init_ctx                         |

x86_cpu_common_class_init 中注册的函数:
本来在 CPUState 中持有 CPUClass 的指针，并且可以通过 CPU_GET_CLASS 来获取
| function                               | 使用位置                    |
|----------------------------------------|-----------------------------|
| x86_cpu_tlb_fill                       | tlb_fill, tlb_vaddr_to_host |
| x86_cpu_synchronize_from_tb            | cpu_tb_exec                 |
| x86_cpu_exec_exit & x86_cpu_exec_enter | cpu_exec                    |

- [ ] 应该感到非常奇怪才对，注册了这么多函数，为什么只有这么一点点才在使用啊

注意 : 这里是分析的是 CPUClass ，而 X86CPUClass 持有的内容很少

## 注意
1. 存在一种编程方法，将一个头文件 include 两次从而实现 template 的，但是这种方法会影响 ccls 的定位。
  - cpu_ldst_template.h 这个文件在 6.0 版本中被替换掉了，这一部分的代码是按照 6.0 的

2. 在文件夹的组织上
  1. 将原来的 tcg 和 accel/tcg 都合并到一个位置了
  2. include/elf.h => include/qemu/elf.h

## TODO
2. 写一个脚本，自动比对出现出入地方
  1. 函数在文件的顺序保持一致
  2. 函数内容
  3. 结构体内容
  4. tcg 之后的设计一定会发生更多的演化，那么靠什么来实现两个

3. 将 g_new 以及 MIN 之类的 macro 放到 exec-all.h 中间了
4. 暂时将原来在 qemu 中放到顶层目录中的文件放到 qeum 下面了，为了防止破坏原来的一点点 firmware 的代码
  - 而且，将大量文件放到顶层本来就不好

5. 关键的接口和重构

| TODO                       | 描述                                                                                                                                                                                           |
|----------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| apic                       | include/hw/i386/apic.h 中定义了一些列的函数，具体的还没有分析，但是 原来的 QEMU 中间                                                                                                           |
| 多核                       | 现在为了代码的方便执行，也没有太搞清楚其中的作用，在 cputlb.c 的 async_run_on_cpu 以及各种 atomic 函数，qemu_spin_lock, 以及 RCU. 现在的操作是，先将接口保存下来，之后需要支持多核，有一个参考 |
| icount / record replay     | 没有 record replay 是不是很难调试，使用 record replay 会不会很难集成                                                                                                                           |
| trace                      |                                                                                                                                                                                                |
| 存在好几个数据结构需要重构 | queue.h, qht.h 和 glib 的 qtree                                                                                                                                                                |

8. 多核:
  1. 感觉 Qemu 关于多核的接口不是很统一啊
      1. QemuSpin
      2. QemuMutex
      3. qemu_spin_lock
      4. 在 include/qemu 下存在 thread.h thread-posix.h 等
      5. seqlock.h
      6. lockable
  2. 据说处理 io 的 thread 和执行是两个 thread

9. 非常不统一的 assert
  - [ ] tcg_debug_assert
  - [ ] assert
  - [ ] 一些 unreachable 之类的
  - [ ] tcg_abort
  - [ ] 在 bitmap.c 中间是直接从
  - [ ] g_assert
  - error_report

## 应该被小心 review 一下的
- [ ] dirty page
- [ ] 一个 tb 分布在两个 page 上
- [ ] 为什么需要使用 glib 来维护 tb
- [ ] 那些数据结构需要 RCU 来保护 

- [ ] 从 translate-all.c 到 tcg.c 的调用图制作一下
  - tcg_context_init
  - tcg_prologue_init

- [ ] tcg_op_defs : 在 tcg.c 中间定义，具体在 tcg_context_init 中间初始化了，但是按照道理来说，xqm 将这些事情都处理了

## 设计 
- 到底是否保存 icount / record replay 机制
  - [ ] 在此之前，首先深入理解一下这个是怎么工作的吧

- 我希望，这些 port 的代码可以使用脚本自动生成，从而就可以保持和 mainline 的代码同步, 但是暂时不用太考虑，先把想法验证出来再说。

## 内核态编程的挑战是什么
- [ ] icache_flush : 如果一个 tb 被重新生成了，是不是需要对于 icache 进行重新刷新，还是说这件事是自动完成的
