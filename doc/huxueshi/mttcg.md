## mttcg
[^1] 指出
1. 如果需要支持 icount 机制将会消失
2. 想要让一个 guest 架构支持 mttcg 需要完成的工作
3. Enabling strong-on-weak memory consistency (e.g. emulate x86 on an ARM host)

- [ ] `static TCGContext **tcg_ctxs;` 和 `extern TCGContext *tcg_ctx;` `extern TCGContext tcg_init_ctx;` 的关系是什么?
  - 都是在什么时候初始化的

看 v6.0 的 qemu_tcg_mttcg_enabled 的调用位置，只有两个位置而已

- [ ] 之前的华为的形式化验证是不是就是处理内存序列的 ?

- [ ] 如果 mttcg 的话，对于同一份 guest code 会每一个 cpu 都会生成 tb 吗?
  - [ ] 完全就是分开管理的吗 ?

实际上，就是需要理解 cpus.h

- [ ] cputlb.c 这么需要 async_run_on_cpu 的使用

- 现在的区别是 : 模拟多个核 和 本身就是多线程的区别
  - 因为同时执行，那么就真的需要另一个 tcg 停止下来
  - [ ] 如果是模拟，那么需要多个 mmu 吗 ?

## iothread 的 lock 应该只有很少的位置才对啊
- [ ] qemu_tcg_rr_wait_io_event

- [ ] 线程模型通常使用 QEMU 大锁进行同步，获取锁的函数为 qemu_mutex_lock_iothread

- [ ] 当持有 BQL 的时候，到底可以做什么事情 ?

- [ ] https://lwn.net/Articles/697265/
- [ ] https://www.linux-kvm.org/images/1/17/Kvm-forum-2013-Effective-multithreading-in-QEMU.pdf

主要使用 qemu_mutex_unlock_iothread 的主要在
/home/maritns3/core/ld/DuckBuBi/src/tcg/cpu-exec.c
和
/home/maritns3/core/ld/DuckBuBi/src/qemu/memory_ldst.c.inc

但是 qemu_mutex_lock_iothread 主要只是出现在
/home/maritns3/core/ld/DuckBuBi/src/tcg/cpu-exec.c 中
而且是为了保护 cpu_handle_interrupt

在 io_readx 和 io_writex 当 `mr->global_locking` 时候需要进行

- 主要是在处理中断的时候

## 如果 mttcg 之外，iothread 之外，还有什么 thread 的挑战

[^1]: https://wiki.qemu.org/Features/tcg-multithread
[^2]: https://qemu-project.gitlab.io/qemu/devel/multi-thread-tcg.html?highlight=bql
