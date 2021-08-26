# SMC 

- [ ] 类似的问题，如何处理 watchpoint 的
- [ ] tb_invalidate_phys_page_range__lock
  - [ ] 参数 pages 到底是做什么的?
- [ ] 类似 page desc 的 lock 的作用是干什么的，比如 page_lock_pair, 为什么需要对于 page desc 进行上锁的啊
  - [ ] 似乎为此构建出来了 page collection 的操作

## TARGET_HAS_PRECISE_SMC
TARGET_HAS_PRECISE_SMC 的使用位置只有 tb_invalidate_phys_page_range__locked

为了处理当前的 tb 正好被 SMC 了

## 流程
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

#### PageDesc::first_tb 
在 tb_page_add 中，只要新的 tb 添加进来，那么 PageDesc::first_tb 就会指向其, 
TranslationBlock::page_next[2] 中

> 注意，TranslationBlock::page_next[2] 存在两个项目，当一个 tb 跨页之后，那么这个 tb 就需要分别添加到
> 两个 PageDesc::first 的数组中。

只要 PageDesc::first_tb page_already_protected

- PageDesc::first_tb 指针的最低两位可以保存这个 PageDesc 是 tb 的第一个 PageDesc 还是第二个。
  - 比如 build_page_bitmap 就需要这个的。

- tb_link_page (exec.c) 把新的 TB 加進 tb_phys_hash 和 l1_map 二級頁表。
tb_find_slow 會用 pc 對映的 GPA 的哈希值索引 tb_phys_hash。

- tb_page_add (exec.c) 設置 TB 的 page_addr 和 page_next，並在 l1_map 中配置 PageDesc 給 TB。

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
      - tlb_protect_code : 指向 exec.c 中间，应该是通过 dirty / clean 的方式来防止代码被修改 ?
        - [ ] 原则上，guest 代码段被修改必然需要让对应的 tb 也是被 invalidate 的呀

#### PageDesc::code_bitmap / PageDesc::code_write_count
主要的使用位置 : tb_invalidate_phys_page_fast 

逻辑非常简单，如果一个 PageDesc 对应的 page 反复被 invalidate 的时候，那么就会建立 bitmap 将其中真正有代码的位置确认，
只有命中了翻译了 tb 的位置，才会真正的 invalidate 的，而一般的处理是，直接 invalidate 所有的。

## page desc tree
- PageDesc 的 lock 是基于什么的 ?

创建:
- page_lock_pair
  - page_find_alloc

查询:
- page_find
  - page_find_alloc

将 tb 放到 PageDesc 的管理中:
- tb_link_page
  - tb_page_add : 需要将 bitmap invalidate 掉

```c
/* Size of the L2 (and L3, etc) page tables.  */
#define V_L2_BITS 10
#define V_L2_SIZE (1 << V_L2_BITS)

# define L1_MAP_ADDR_SPACE_BITS  TARGET_PHYS_ADDR_SPACE_BITS

/*
 * L1 Mapping properties
 */
static int v_l1_size;
static int v_l1_shift;
static int v_l2_levels;

/* The bottom level has pointers to PageDesc, and is indexed by
 * anything from 4 to (V_L2_BITS + 3) bits, depending on target page size.
 */
#define V_L1_MIN_BITS 4
#define V_L1_MAX_BITS (V_L2_BITS + 3)
#define V_L1_MAX_SIZE (1 << V_L1_MAX_BITS)

static void *l1_map[V_L1_MAX_SIZE];

static void page_table_config_init(void)
{
    uint32_t v_l1_bits;

    assert(TARGET_PAGE_BITS);
    /* The bits remaining after N lower levels of page tables.  */
    v_l1_bits = (L1_MAP_ADDR_SPACE_BITS - TARGET_PAGE_BITS) % V_L2_BITS;

    if (v_l1_bits < V_L1_MIN_BITS) {
        v_l1_bits += V_L2_BITS;
    }

    v_l1_size = 1 << v_l1_bits;
    v_l1_shift = L1_MAP_ADDR_SPACE_BITS - TARGET_PAGE_BITS - v_l1_bits;
    v_l2_levels = v_l1_shift / V_L2_BITS - 1;

    assert(v_l1_bits <= V_L1_MAX_BITS);
    assert(v_l1_shift % V_L2_BITS == 0);
    assert(v_l2_levels >= 0);
}
```

- 需要覆盖架构支持的所有的物理地址
- 保证 V_L2_BITS 总是 10

## tb_invalidate_phys_page_fast
- tb_invalidate_phys_page_fast
  - page_find
    - [ ] page_find_alloc(tb_page_addr_t index, int alloc)
      - 分配空间，还需要考虑 level 什么的
      - [ ] page_find_alloc 中间为什么需要使用 rcu
  - build_page_bitmap
  - tb_invalidate_phys_page_range__locked 
    - tb_phys_invalidate__locked
      - do_tb_phys_invalidate
        - do_tb_phys_invalidate(在 chen 的笔记中叫做 tb_phys_invalidate)，在这里完成真正的工作, 将 tb 从 hash 中间移除之类的

- tb_invalidate_phys_page_range__locked : 这是真正进行工作的位置
  * tb_invalidate_phys_range
      * invalidate_and_set_dirty : 调用这个的位置超级多
      * tb_check_watchpoint
  * tb_invalidate_phys_page_range
    * tb_invalidate_phys_addr : 没有用户, 或者说是一个很奇怪的架构需要这个东西

## 参考
[^1]: https://github.com/azru0512/slide/tree/master/QEMU
[^2]: https://qemu.weilnetz.de/w64/2012/2012-06-28/qemu-tech.html#Self_002dmodifying-code-and-translated-code-invalidation
