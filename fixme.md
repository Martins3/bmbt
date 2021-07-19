# 将所有需要分析的 fixme 列举出来

1. memory model
    1. include/sysemu/cpus.h : 定义的为空函数啊
    2. include/exec/ram_addr.h

2. apic
    1. DeviceState 中定义为空
    2. /home/maritns3/core/ld/DuckBuBi/include/hw/i386/apic.h 都是空函数
    3. cpu_get_pic_interrupt 定义在 pc.c 中间了
    4. qemu_irq_raise : 在 fpu_raise_exception 中需要被调用
    5. hw/irq.h 中的东西似乎只会被 fpu_helper 使用，其他的位置在哪里呀


## 代码分析工作
1. --enable-x86tomips-flag-int 是干什么的 ?
2. cpu_cc_compute_all 是做什么 ?
3. cpu_is_bsp
4. do_cpu_sipi
5. do_cpu_init
