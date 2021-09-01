# 头文件的移植

在原先的 QEMU 中，header 的依赖搞的非常诡异的, 在移植的过程中没有分析清楚，这里写下总结

1. 所有的 header 都是使用绝对路径，这是为了让 ccls 可以正确警告，从而可以正确移植
2. 总体来说，bmbt 中的每一个 header 都是和原始 QEMU 对称的

## [ ] TODO
1. `#include <stdbool>` 之类的头文件真的需要清理掉吗 ?
2. 如何让 acpi / kernel / tcg 使用同一个 header
3. qdist.h : 主要是 print_qht_statistics 中间使用的用于分析统计信息的，也许直接删除吧

## 一些记录
1. 在 thread-posix.h 中定义了 QemuMutex QemuCond QemuSemaphore QemuEvent QemuThread, 但是只使用了 QemuMutex 了，将其移动到 thread.h 在和锁相关的问题上一起分析
2. include/exec/helper-head.h 中包含了 /home/maritns3/core/ld/DuckBuBi/include/exec/helper-head.h

## cpu-ldst.h
1. cpu-ldst.h 开始移植的时候，使用了 v6.0 的代码
2. cputlb.c 就是直接拷贝的 v4.2 的，没有做任何的修改
3. 在 v6.0 中，cputlb.c 中是定义了 cpu_ldub_mmuidx_ra 的
4. 还移植了 v6.0 的 seg_helper.h

## 头文件功能描述
| header       | desc                                                                       |
|--------------|----------------------------------------------------------------------------|
| core/cpu.h   | CPUState / CPUClass                                                        |
| i386/cpu.h   | CPUX86State / X86CPUClass / X86CPU                                         |
| cpu-all.h    | 包含 memory_ldst.inc.h  以及一些基础的 macro                               |
| exec-all.h   | TranslationBlock 以及 cputlb.c / exec.c / cpu-exec.c 中定义的函数          |
| cpu-defs.h   | CPUTLBEntry / CPUIOTLBEntry / CPUTLBDescFast 之类的 TLB 定义               |
| cpus.h       | 多核                                                                       |
| cpu-ldst.h   | 这个东西的实现非常的不优雅，将会切换到 v6.0 的实现方法，之后再去慢慢分析吧 |
| cpu-common.h | 定义了 ram_addr_t 类型                                                     |
| cpu-para.h   | 定义了 x86 cpu 的地址空间的属性                                            |

注意:
1. 除了前面两个，其余头文件都是放到 exec 下面的
2. CPUState 的子类居然是 x86CPU 啊

## cpu.h 和 exec-all.h 的依赖分析
`include/hw/core/cpu.h`, 这是根，不应该依赖 `src/i386/cpu.h`
`include/exec/cpu-defs.h` 不应该依赖 `src/i386/cpu.h`

但是 `include/exec/exec-all.h` 是可以依赖 `src/i386/cpu.h` 的


所以，`include/hw/core/cpu.h` / `include/exec/cpu-defs.h` <- `src/i386/cpu.h` <- `include/exec/exec-all.h`

大致含义就是，core/cpu.h 和 cpu-defs.h 都是基本 cpu 的定义，
而 exec-all.h 是操作 CPUArchState 的


# 如何保证 config-host.h 和 config-target.h 被所有的人 include
在 `include/hw/core/cpu.h` 的函数 cpu_tb_jmp_cache_clear 的 CONFIG_X86toMIPS
在是否依赖 qemu/config-host.h 产生差别，但是实际上，一直没有注意到

QEMU 采取的做法是让很多位置全部去 include osdep.h, 然后 osdep include 两者

现在的策略是移动到 types.h 中间，之后在 types.h 重构的时候，可以保证 types.h 总是最开始，

## 来自 header 的终极挑战: NEED_CPU_H

NEED_CPU_H 中间的，这个东西出现的位置
1. helper_head
2. memop.h 中间对于这个东西存在一个临时定义的
3. memory.h 中间定义

从 /home/maritns3/core/kvmqemu/meson.build 中找到 NEED_CPU_H ，其中的
看来是所有在 target 下的代码都是需要 NEED_CPU_H 的。

对于 NEED_CPU_H 的理解:
1. 如果没有 #ifdef NEED_CPU_H ，那么这个 macro 就没有什么意义了
2. 如果只有 #ifdef NEED_CPU_H 没有对应的 else，多定义出来的类型应该问题不大
3. 所以，唯一的挑战就是在 memop.h 导致结构体 MemOp 的含义不同

#### 资料
https://patchwork.kernel.org/project/qemu-devel/patch/1455818725-7647-7-git-send-email-peter.maydell@linaro.org/

> `NEED_CPU_H` is the define we use to distinguish per-target object
compilation from common object compilation. For the former, we must
also include config-target.h so that the .c files see the necessary
`CONFIG_` constants.

看一下几个添加 NEED_CPU_H 的 commit:
1. git show 1c14f162dd92c0448948791531dc82ac277330ae
```diff
Author: Blue Swirl <blauwirbel@gmail.com>
Date:   Mon Mar 29 19:23:47 2010 +0000

    Allow various header files to be included from non-CPU code

    Allow balloon.h, gdbstub.h and kvm.h to be included from
    non-CPU code.

    Signed-off-by: Blue Swirl <blauwirbel@gmail.com>
```
- [ ] 所以, 只要添加了 NEED_CPU_H 就可以叫做是可以被 non-CPU node ?
- [ ] 为什么会引出 non-cpu code 的概念啊

2. git show 13b48fb00e61dc7662da27c020c3263b74374acc

```diff
commit 13b48fb00e61dc7662da27c020c3263b74374acc
Author: Thomas Huth <thuth@redhat.com>
Date:   Wed Apr 14 13:20:01 2021 +0200

    include/sysemu: Poison all accelerator CONFIG switches in common code

    We are already poisoning CONFIG_KVM since this switch is not working
    in common code. Do the same with the other accelerator switches, too
    (except for CONFIG_TCG, which is special, since it is also defined in
    config-host.h).

    Message-Id: <20210414112004.943383-2-thuth@redhat.com>
    Reviewed-by: Philippe Mathieu-Daudé <philmd@redhat.com>
    Signed-off-by: Thomas Huth <thuth@redhat.com>
```
