# mttcg 的装换记录

<!-- vim-markdown-toc GitLab -->

* [`tcg_register_thread`](#tcg_register_thread)
* [多核编程需要增加考虑的事情](#多核编程需要增加考虑的事情)
  * [内核的初始化需要考虑多核的初始化了](#内核的初始化需要考虑多核的初始化了)
* [`BMBT_MTTCG`](#bmbt_mttcg)
* [中断对于多核的挑战](#中断对于多核的挑战)

<!-- vim-markdown-toc -->

1. `__thread` 被移除掉了
  - `tcg_ctx`
    - 因为 main loop 的 `tcg_ctx` 在完成初始化之后就没有用途了，所以 [`tcg_register_thread`](#tcg_register_thread) 的写法实际上是没有问题的
    - 在 xqm 中 `tcg_context_init` 和 `tcg_x86_init` 不在需要调用 `temp_idx`, `temp_tcgv_i32` 和 `tcgv_i32_temp` 了，这实际上让 `tcg_ctx` 的初始化仅仅在 `tcg_register_thread` 中的

## `tcg_register_thread`
commit-id:5fa309397ae8481eb 的 `tcg_register_thread`
```c
void tcg_register_thread(void) {
  MachineState *ms = qdev_get_machine();
  TCGContext *s = g_malloc(sizeof(*s));
  unsigned int i, n;
  bool err;

  *s = tcg_init_ctx;

  /* Claim an entry in tcg_ctxs */
  n = atomic_fetch_inc(&n_tcg_ctxs);
  g_assert(n < ms->smp.max_cpus);
  atomic_set(&tcg_ctxs[n], s);

  if (n > 0) {
    alloc_tcg_plugin_context(s);
  }

  tcg_ctx = s;
  qemu_mutex_lock(&region.lock);
  err = tcg_region_initial_alloc__locked(tcg_ctx);
  g_assert(!err);
  qemu_mutex_unlock(&region.lock);
}
```

## 多核编程需要增加考虑的事情
- soft tlb 的访问
- 将 libc 的锁被简化了
- errno
- qht
- 没有再考虑内核中多核
  - 没有多核启动
  - head.S 和 genex.S 中的 entry 去掉了 `CONFIG_SMP`
- access interrupt controller，such as `pch_pic_bitclr`

### 内核的初始化需要考虑多核的初始化了
- 比如在 `cpu_logical_map` 根本不初始化，应为我们知道 logical cpu 和 physical cpu 存在 0 到 0 的映射

## `BMBT_MTTCG`
1. 目前对于单核 logical cpu 和 physical cpu 的映射直接就是 0 -> 0，但是多核就不一定了
2. 最多支持一个 `pch_pic` 控制器，但是对于 NUMA 是需要支持多个的
3. 因为只有 CPU，所以 affinity 的计算结果总是 0，但是在多核中，就不一定了。
  - 如何正确的反映 Guest 的 CPU affinity 似乎也是一个问题。 如果 Guest 设置 Guest CPU 0 来接受中断，但是实际上中断是 Host CPU 1 接受的，也许需要让 Host CPU 1 将中断注入到 Host CPU 0 中间
4. 这个物理内存分配器无法支持多核
  - 一个最简单的方法给每一个 CPU 物理内存一个分配池，类似 percpu 的那种
5. `atomic_common.c.inc`
6. `set_extioi_action_handler`
  - 这个存在一个临时的 hacking ，在裸机状态中，不要执行 `ext_set_irq_affinity` ，其根本原因是什么，现在并没有理解清楚
  - 虽然不去调查也是可以维持生活的，但是在实现多核版本的时候，这些问题无法逃躲的。

## 中断对于多核的挑战
- 中断处理函数访问的内容需要上锁的
- 多核的出现导致需要考虑中断路由，如何模拟出来 guest 的中断路由出来
- Guest 多核的 ipi 如何装换为 host 多核 ipi
- Remote TLB flush 操作的模拟
