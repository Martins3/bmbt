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
