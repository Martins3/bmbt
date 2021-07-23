## 多核启动[^1]
QEMU 中 cpu_is_bsp 和 do_cpu_sipi

在x86-64多核系统的初始化中，需要有一个核作为bootstrap processor (BSP)，由这个BSP来执行操作系统初始化代码。

下面就是加冕仪式了，新晋的BSP需要带上王冠（设置自己IA32_APIC_BASE寄存器的BSP标志位为1）来表明自己的身份。

那其他的APs可不可以也这样做呢，当然不行，否则岂不是要谋反么。此时APs进入wait for SIPI的状态，等待着BSP的发号施令。

[^1]: https://zhuanlan.zhihu.com/p/67989330
