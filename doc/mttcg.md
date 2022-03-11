# mttcg 的装换记录

<!-- vim-markdown-toc GitLab -->

* [tcg_register_thread](#tcg_register_thread)
* [多核编程需要增加考虑的事情](#多核编程需要增加考虑的事情)
* [出现需要 lock 的位置会被 BMBT_MTTCG 标记出来](#出现需要-lock-的位置会被-bmbt_mttcg-标记出来)

<!-- vim-markdown-toc -->

1. `__thread` 被移除掉了
  - tcg_ctx
    - 因为 main loop 的 tcg_ctx 在完成初始化之后就没有用途了，所以 [tcg_register_thread](#tcg_register_thread) 的写法实际上是没有问题的
    - 在 xqm 中 tcg_context_init 和 tcg_x86_init 不在需要调用 temp_idx, temp_tcgv_i32 和 tcgv_i32_temp 了，这实际上让 tcg_ctx 的初始化仅仅在 tcg_register_thread 中的

## tcg_register_thread
commit-id:5fa309397ae8481eb 的 tcg_register_thread
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
  - head.S 和 genex.S 中的 entry 去掉了 CONFIG_SMP
- access interrupt controller，such as pch_pic_bitclr

## 出现需要 lock 的位置会被 BMBT_MTTCG 标记出来
