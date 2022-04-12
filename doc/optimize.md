# 一些因为工期原因实际上可以优化的部分被 BMBT_OPTIMIZE 标记出来
1. memory region lookup
  - 其实只要添加上一个缓存就可以让快很多
2. cs_disasm 在快速的调用 remalloc，但是我认为 remalloc 的性能不行，而且感觉让 malloc 的分配碎片化
3. 显然，SAVE_SOME 在被 interrupt 调用的时候没有必要保存如此之多的 CSR 寄存器，尤其是 CSR 寄存器访问很慢
4. 在分析 msi 的时候，检测到了很高频率的 apic_send_msi，但是 apic_send_msi 最后触发的效果是固定，根本没有必要重新经过一次 memory region 的查询
5. 被 RELEASE_VERSION 包围的地方
6. 动态分配这个 msi table 的数量。
  - 从原则上说，一个 PC 的 pcie 设备一共就只有几个，而且一般确定了 msi table 之后不会再去修改的
  - 但是，如果一个操作系统反复修改 msix table 的位置，这里是没有回收机制的
7. 真的有必要写的这么丑吗?
8. `qemu_timer.c` 是可以简化一下的了
