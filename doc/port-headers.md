# 头文件的移植 #95

在原先的 QEMU 中，header 的依赖搞的非常诡异的
在移植的过程中没有分析清楚，这里写下总结

1. 所有的 header 都是使用绝对路径，这是为了让 ccls 可以正确警告，从而可以正确移植
2. 总体来说，bmbt 中的每一个 header 都是和原始 QEMU 对称的

## cpu.h
`include/hw/core/cpu.h`, 这是根，不应该依赖 `src/i386/cpu.h`
`include/exec/cpu-defs.h` 不应该依赖 `src/i386/cpu.h`

但是 `include/exec/exec-all.h` 是可以依赖 `src/i386/cpu.h` 的

## TODO
| TODO                     | 问题描述                                                                                                                                                       |
|--------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `#include <stdbool>`     | 如何让 acpi / kernel / tcg 使用同一个 header                                                                                                                   |
| 清理 types 的定义        | target_ulong 和各种 u32 i32, 以及 ram_addr_t 定义在 cpu-common.h 中，感觉很随意                                                                                |
| 清理头文件的依赖         | 在 i386 下依赖的头文件</br> 两个 cpu.h, tcg/tcg.h                                                                                                              |
| 写一个所有头文件功能描述 | cpu-all.h exec-all.h cpu.h cpu-defs.h 中间到底有什么，根本不清楚啊                                                                                             |

6. 需要被重新设计的头文件
  1. seglock.h
  2. thread-posix.h
  3. qht.h
  4. qdist.h

7. cpu-defs.h osdep.h 和 config-host.h / config-target.h 的内容分析整理一下
  - cpu-paras.h

# 头文件的依赖和 macro 的问题
在 `include/hw/core/cpu.h`
的函数 cpu_tb_jmp_cache_clear 的 CONFIG_X86toMIPS 
在是否依赖 qemu/config-host.h 产生差别，但是实际上，一直没有注意到
