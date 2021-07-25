# SMC 
- [ ] TARGET_HAS_PRECISE_SMC : 这个东西是啥效果，对应的支持是什么?

- [ ] dirty page 只是和 smc 相关吗 ? 不存在其他的东西和 dirty page 也是相关的吗 ?

## 流程
- tb_invalidate_phys_page_fast
  - page_find
    - [ ] page_find_alloc(tb_page_addr_t index, int alloc)
      - index 索引的标准是什么 ?
      - 分配空间，还需要考虑 level 什么的
  - build_page_bitmap
  - tb_invalidate_phys_page_range__locked 
    - tb_phys_invalidate__locked
      - do_tb_phys_invalidate
        - do_tb_phys_invalidate(在 chen 的笔记中叫做 tb_phys_invalidate)，在这里完成真正的工作, 将 tb 从 hash 中间移除之类的

- [ ] tb_invalidate_phys_page_range__lock
  - [ ] 参数 pages 到底是做什么的?
  - [ ] 包含了一堆 TARGET_HAS_PRECISE_SMC, 如果不精确，会怎么样 ?
    - [ ] 找一个不适用精确 SMC 的例子 ?

- [x] 真的相信是通过信号机制(SEGV)来防护的吗 ?
  - 用户态是如此处理的，系统态直接在 softmmu 的位置检查

> 在软件 MMU 中通过 TLB_NOTDIRTY 标志和相关逻辑实现这些.

```c
/*
 * Dirty write flag handling
 *
 * When the TCG code writes to a location it looks up the address in
 * the TLB and uses that data to compute the final address. If any of
 * the lower bits of the address are set then the slow path is forced.
 * There are a number of reasons to do this but for normal RAM the
 * most usual is detecting writes to code regions which may invalidate
 * generated code.
 *
 * Other vCPUs might be reading their TLBs during guest execution, so we update
 * te->addr_write with qatomic_set. We don't need to worry about this for
 * oversized guests as MTTCG is disabled for them.
 *
 * Called with tlb_c.lock held.
 */
static void tlb_reset_dirty_range_locked(CPUTLBEntry *tlb_entry,
                                         uintptr_t start, uintptr_t length)
```

## 检测: 当检测
notdirty_write : 每次调用，都是存在检查到 TLB_NOTDIRTY 的时候，所以其作用是当写入一个 TLB_NOTDIRTY 的位置，然后将 page invalidate 掉

## 保护 : 将含有代码的位置使用 TLB_NOTDIRTY 保护起来 
一个没有任何分叉的调用路径

- tb_link_page
  - tb_page_add
    - tlb_protect_code : update the TLBs so that writes to code in the virtual page 'addr' can be detected
      - cpu_physical_memory_test_and_clear_dirty
        - tlb_reset_dirty_range_all
          - tlb_reset_dirty
            - tlb_reset_dirty_range_locked : 这就是设置保护的位置

- [ ] 是否存在一个 page 有一部分是代码，一部分是数据，然后数据的那一部分老是在修改

## - [ ] 类似的问题，如何处理 watchpoint 的

## 参考
[^1]: https://github.com/azru0512/slide/tree/master/QEMU
[^2]: https://qemu.weilnetz.de/w64/2012/2012-06-28/qemu-tech.html#Self_002dmodifying-code-and-translated-code-invalidation
