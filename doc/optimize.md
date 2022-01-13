# 一些因为工期原因实际上可以优化的部分被 BMBT_OPTIMIZE 标记出来
1. memory region lookup
  - 其实只要添加上一个缓存就可以让快很多
2. 暂时 mmap 的实现分配物理内存是基于操作
