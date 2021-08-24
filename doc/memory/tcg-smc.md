# SMC 
- [ ] TARGET_HAS_PRECISE_SMC : 这个东西是啥效果，对应的支持是什么?
- [ ] PageDesc 是不是只是用于分析 SMC 的?
- [ ] 类似的问题，如何处理 watchpoint 的
- [ ] 重新理解一下 tb_invalidate_phys_page_fast, 其第二个参数是 ram_addr 的
    - 到底是使用 ram_addr 还是使用 physics memory address 来索引
- [ ] tb_invalidate_phys_page_range__lock
  - [ ] 参数 pages 到底是做什么的?
  - [ ] 包含了一堆 TARGET_HAS_PRECISE_SMC, 如果不精确，会怎么样 ?
    - [ ] 找一个不使用精确 SMC 的例子 ?
- [ ] 是否存在一个 page 有一部分是代码，一部分是数据，然后数据的那一部分老是在修改
    - 必然是存在的，但是需要考虑这些
- [ ] page_find_alloc 中间为什么需要使用 rcu

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

- 用户态是如此处理的 通过信号机制(SEGV)，系统态直接在 softmmu 的位置检查

保护代码的流程:

- tb_link_page
  - tb_page_add
    - tlb_protect_code
      - cpu_physical_memory_test_and_clear_dirty : 这是 ram_addr.h 中一个处理 dirty page 的标准函数
        - memory_region_clear_dirty_bitmap
        - tlb_reset_dirty_range_all
          - tlb_reset_dirty : 将这个范围内的 TLB 全部添加上 TLB_NOTDIRTY
            - tlb_reset_dirty_range_locked : 这就是设置保护的位置

触发错误的流程:
- store_helper
  - notdirty_write : 当写向一个 dirty 的位置的处理
    - cpu_physical_memory_get_dirty_flag
    - tb_invalidate_phys_page_fast : 
    - cpu_physical_memory_set_dirty_range : Set both VGA and migration bits for simplicity and to remove the notdirty callback faster.
    - tlb_set_dirty

## PageDesc
```c
typedef struct PageDesc {
    /* list of TBs intersecting this ram page */
    uintptr_t first_tb;
#ifdef CONFIG_SOFTMMU
    /* in order to optimize self modifying code, we count the number
       of lookups we do to a given page to use a bitmap */
    unsigned long *code_bitmap;
    unsigned int code_write_count;
#else
    unsigned long flags;
    void *target_data;
#endif
#ifndef CONFIG_USER_ONLY
    QemuSpin lock;
#endif
} PageDesc;
```
- [ ] code_bitmap
- [ ] 


- tb_link_page (exec.c) 把新的 TB 加進 tb_phys_hash 和 l1_map 二級頁表。
tb_find_slow 會用 pc 對映的 GPA 的哈希值索引 tb_phys_hash。

- tb_page_add (exec.c) 設置 TB 的 page_addr 和 page_next，並在 l1_map 中配置 PageDesc 給 TB。

似乎将 PageDesc 是将组织方式是靠 l1_map ，形成树状。
page_lock_pair 告诉我们，通过 physics address 作为索引调用函数 page_find_alloc 来查询，
而 page_find_alloc 的查询过程完全类似内核中 page table 的查询过程，只是现在的对象是 PageDesc 而已。

PageDesc 會維護一個 bitmap，這是給 SMC 之用。

在 /home/maritns3/core/notes/zhangfuxin/qemu-llvm-docs/QEMU/QEMU-tcg-01.txt
中，分析 PageDesc 的作用，可以通过 PageDesc 迅速找到这个 guest page 对应的所有的
tb，从而将这些 tb 全部 invalidate 掉。

- [ ] 居然 PageDesc 是给 SMC 用的
  - 一共四个结构体, 去掉一个锁，first_tb 用于获取这个 page 上的所有 tb

tb_invalidate_phys_page_fast : 一个 PageDesc 并不会立刻创建 bitmap, 而是发现 tb_invalidate_phys_page_fast 多次被调用才会创建
创建 bitmap 的作用是为了精准定位出来到底是哪一个 page 需要被 invalid。

- [ ] page_flush_tb
  - [ ] tb 和 page 大致是怎么关联起来的

- [ ] 结构体 PageDesc 的作用是什么 ?
  - 难道时候首先分配 page，然后这些 tb 都是 page
  - 对于连续的物理空间或者虚拟地址空间，感觉并没有必要如此必要吧
  - TranslationBlock::page_addr
    - 记录了一个 TB 所在的页面
    - 如果页面是连续的，就不应该申请两个

- [ ] SMC_BITMAP_USE_THRESHOLD
  - 和 highwater 什么关系?

- tb_gen_code : 这是一个关键核心
  - get_page_addr_code : 将虚拟地址的 pc 装换为物理地址
    - get_page_addr_code_hostp
      - 如果命中，就是 TLB 的翻译 `p = (void *)((uintptr_t)addr + entry->addend);`
      - qemu_ram_addr_from_host_nofail
  - tb_link_page : 将 tb 纳入到 QEMU 的管理中
    - tb_page_add
      - [ ] invalidate_page_bitmap : 根本无法理解，link page 的时候为什么会将 bitmap disable 掉
      - page_already_protected : 这个是什么逻辑
      - tlb_protect_code : 指向 exec.c 中间，应该是通过 dirty / clean 的方式来防止代码被修改 ?
        - [ ] 原则上，guest 代码段被修改必然需要让对应的 tb 也是被 invalidate 的呀

## 参考
[^1]: https://github.com/azru0512/slide/tree/master/QEMU
[^2]: https://qemu.weilnetz.de/w64/2012/2012-06-28/qemu-tech.html#Self_002dmodifying-code-and-translated-code-invalidation
