# Divergence between qemu

| struct           | divergence                                                                              |
|------------------|-----------------------------------------------------------------------------------------|
| CPUState         |                                                                                         |
| CPUClass         | 在函数 x86_cpu_common_class_init 中已经知道注册的函数, 可以将其直接定义为一个静态函数集 |
| TranslationBlock |
