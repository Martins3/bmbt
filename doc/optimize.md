# 一些因为工期原因实际上可以优化的部分被 BMBT_OPTIMIZE 标记出来
1. memory region lookup
  - 其实只要添加上一个缓存就可以让快很多
2. cs_disasm 在快速的调用 remalloc，但是我认为 remalloc 的性能不行，而且感觉让 malloc 的分配碎片化
3. 显然，SAVE_SOME 在被 interrupt 调用的时候没有必要保存如此之多的 CSR 寄存器，尤其是 CSR 寄存器访问很慢
