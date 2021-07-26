# 头文件的移植

在原先的 QEMU 中，header 的依赖搞的非常诡异的, 在移植的过程中没有分析清楚，这里写下总结

1. 所有的 header 都是使用绝对路径，这是为了让 ccls 可以正确警告，从而可以正确移植
2. 总体来说，bmbt 中的每一个 header 都是和原始 QEMU 对称的

## cpu.h
`include/hw/core/cpu.h`, 这是根，不应该依赖 `src/i386/cpu.h`
`include/exec/cpu-defs.h` 不应该依赖 `src/i386/cpu.h`

但是 `include/exec/exec-all.h` 是可以依赖 `src/i386/cpu.h` 的

## TODO

| TODO                     | 问题描述                                                                        |
|--------------------------|---------------------------------------------------------------------------------|
| `#include <stdbool>`     | 如何让 acpi / kernel / tcg 使用同一个 header                                    |
| 清理 types 的定义        | target_ulong 和各种 u32 i32, 以及 ram_addr_t 定义在 cpu-common.h 中，感觉很随意 |
| 清理头文件的依赖         | 在 i386 下依赖的头文件</br> 两个 cpu.h, tcg/tcg.h                               |
| 写一个所有头文件功能描述 | cpu-all.h exec-all.h cpu.h cpu-defs.h 中间到底有什么，根本不清楚啊              |

6. 需要被重新设计的头文件
  1. seglock.h
  2. thread-posix.h
  3. qht.h
  4. qdist.h

7. cpu-defs.h osdep.h 和 config-host.h / config-target.h 的内容分析整理一下
  - cpu-paras.h

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
