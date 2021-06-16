# Divergence between qemu

## 几个关键的结构体功能和移植差异说明
| struct           |  | divergence                                                                              |
|------------------|--|-----------------------------------------------------------------------------------------|
| CPUState         |  |                                                                                         |
| CPUClass         |  | 在函数 x86_cpu_common_class_init 中已经知道注册的函数, 可以将其直接定义为一个静态函数集 |
| TranslationBlock |  |
| CPUX86State      |  |
| X86CPU           |  |


x86_cpu_common_class_init 中注册的函数:
| function         | 使用位置|
|------------------|
| x86_cpu_tlb_fill | tlb_fill, tlb_vaddr_to_host|

## 注意
1. 存在一种编程方法，将一个头文件 include 两次从而实现 template 的，但是这种方法会影响 ccls 的定位。
  - cpu_ldst_template.h 这个文件在 6.0 版本中被替换掉了，这一部分的代码是按照 6.0 的


## TODO
1. 首先将所有 TODO 整理成为表格再说吧

| TODO                 | 问题描述                                                              |
|----------------------|--------------------------------------------------------------------|
| `#include <stdbool>` | 如何让 acpi / kernel / tcg 使用同一个 header                       |
| 清理 types 的定义    | target_ulong 和各种 u32 i32                                        |
| 分析如何支持多核     | 虽然现在不考虑支持多核，但是也应该进行埋点，为之后支持多核进行分析 </br> 各种调用 CPU_FOREACH 之类的如何处理 |
| 清理头文件的依赖 | 在 i386 下依赖的头文件</br> 两个 cpu.h, tcg/tcg.h|
| NEED_CPU_H | 这个 macro 是干啥的，猜测真正的操作是，一个头文件 a.h，其中的一部分被 NEED_CPU_H 包围，a.h 被不同的 c 文件包含，b.c 和 c.c, 那么 b.c 和 c.c 看到的内容可以不同|

2. 写一个脚本，自动比对出现出入地方
  1. 函数在文件的顺序保持一致
  2. 函数内容
  3. 结构体内容
  4. tcg 之后的设计一定会发生更多的演化，那么靠什么来实现两个

3. 将 g_new 以及 MIN 之类的 macro 放到 exec-all.h 中间了

## 设计 
- 移除掉 memory model
  - 现在的模型没有必要搞得这么复杂, 因为其中支持了 memory migration, memory listener 之类
  - 关于各种地址空间相互覆盖，优先级等问题，可以参考 kvmtool
