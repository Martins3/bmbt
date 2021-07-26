# QEMU 中的那些地方需要 lock

emmm 其实就是收集一下那些函数的调用位置而已。

1. tcg_region : 一个 region 只会分配给一个 cpu, 所以防止同时分配给多个 cpu 了
