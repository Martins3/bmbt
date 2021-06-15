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
| function         |
|------------------|
| x86_cpu_tlb_fill |

## 注意
1. 存在一种编程方法，将一个头文件 include 两次从而实现 template 的，但是这种方法会影响 ccls 的定位。
  - cpu_ldst_template.h 这个文件在 6.0 版本中被替换掉了，这一部分的代码是按照 6.0 的


## TODO
1. 写一个脚本，自动比对出现出入地方
  1. 函数在文件的顺序保持一致
  2. 函数内容
  3. 结构体内容

## 设计 
- 移除掉 memory model
  - 现在的模型没有必要搞得这么复杂, 因为其中支持了 memory migration, memory listener 之类
  - 关于各种地址空间相互覆盖，优先级等问题，可以参考 kvmtool
