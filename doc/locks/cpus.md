# 如何移植 cpus.c 啊

- [ ] tb_flush 中，通过 cpu_in_exclusive_context 来运行
  - [ ] 另一个 cpu 正在运行，此时进行 tb_flush, 如果保证运行的 cpu 没有读取错误的 TLB
- [ ] async_safe_run_on_cpu 的实现原理
  - [ ] 如果哪一个 cpu 正好在运行，和 cpu 没有运行，处理的情况有没有区别 ?

