- [ ] translate-all.c : tb 的各种管理工作, 但是管理了很多 page 相关的工作
- tcg.c : 主要是 tb 的一些分配工作在使用，其实几乎没有什么用途了
  - [ ] tcg_prologue_init
  - [ ] 从 tcg_prologue_init 向外发散出来很多东西，不过大大的丰富了 TCGContext 的
  - [ ] tcg.c 在本来的涉及中间，是好几个文件和合集(因为 #include .c 的原因), 到时候拆开 ?
    - [ ] 最恐怖的是，发现这些文件基本处于没有被使用的状态
    - [ ] 实际上，将 tcg_prologue_init 包含进来，也只是花费了 1000 行而已，原来的设计中，剩下的代码在干什么 ?
  - [ ] ELF_HOST_MACHINE 到底在干什么 ?
- [ ] cpu.c : 感觉 helper 没有向这里调用，感觉很奇怪啊, 似乎只是一些配置函数 ?
  - [ ] 哪里有问题，显然 i386/cpu.c 中间还是存在很多我们需要的东西的
- [ ] tcg-all.c : 涉及到初始化 tcg engine, 很短的一个文件

- [ ] qemu_tcg_init_vcpu : 在 cpus.c 中还存在一些代码

- [ ] memory_ldst.c 的结构分析
  - [ ] 分析一下文件结构
  - [ ] 分析一下调用路径

- [ ] atomic 机制谁在使用啊
- [ ] qemu_mutex 机制 ?

- [ ] 多核机制分析其实不仅仅在于此
  - cpus.h
  - CPUState 中的 cpu_index, cluster_index 等

## cpu_exec.c
- [ ] 在 /home/maritns3/core/notes/zhangfuxin/qemu-llvm-docs/QEMU/QEMU-tcg-02.txt
中间提到了, 首先使用虚拟地址查询(fast)，然后使用物理地址查询(slow)，为什么这么设计。
- [ ] captive 说，其使用物理地址索引，所以效率更高之类的
  - [ ] 当使用上物理TLB 之后，使用物理地址作为索引更好吗 ?
- [ ] 同时使用物理地址和虚拟地址作为索引的一个原因是不是因为曾经为了支持 usermode 的二进制翻译

- tb_lookup
  - tb_jmp_cache : 是快路径查询

# translate-all.c 代码分析
(TB) 納入 QEMU 的管理，這是 tb_link_page 做的事。

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

- [ ] build_page_bitmap 居然是对于每一个 byte 建立一个 bit

- [ ] 居然 PageDesc 是给 SMC 用的
  - 一共四个结构体, 去掉一个锁，first_tb 用于获取这个 page 上的所有 tb, 

```c
tb_invalidate_phys_page_fast : 一个 PageDesc 并不会立刻创建 bitmap, 而是发现 tb_invalidate_phys_page_fast 多次被调用才会创建
创建 bitmap 的作用是为了精准定位出来到底是哪一个 page 需要被 invalid。
```



- [ ] page_flush_tb
  - [ ] tb 和 page 大致是怎么关联起来的

- [ ] 结构体 PageDesc 的作用是什么 ?
  - 难道时候首先分配 page，然后这些 tb 都是 page
  - 对于连续的物理空间或者虚拟地址空间，感觉并没有必要如此必要吧
  - TranslationBlock::page_addr
    - 记录了一个 TB 所在的页面
    - 如果页面是连续的，就不应该申请两个
    
- tb_invalidate_phys_page_fast
  - page_find
    - [ ] page_find_alloc(tb_page_addr_t index, int alloc)
      - index 索引的标准是什么 ?
      - 分配空间，还需要考虑 level 什么的
  - build_page_bitmap
  - tb_invalidate_phys_page_range__locked


- [ ] SMC_BITMAP_USE_THRESHOLD
  - 和 highwater 什么关系?

- tb_gen_code : 这是一个关键核心
  - get_page_addr_code : 将虚拟地址的 pc 装换为物理地址
    - get_page_addr_code_hostp
      - 如果命中，就是 TLB 的翻译 `p = (void *)((uintptr_t)addr + entry->addend);`
      - qemu_ram_addr_from_host_nofail
  - tb_link_page
    - tb_page_add
      - [ ] invalidate_page_bitmap : 根本无法理解，link page 的时候为什么会将 bitmap disable 掉
      - page_already_protected : 这个是什么逻辑
      - tlb_protect_code : 指向 exec.c 中间，应该是通过 dirty / clean 的方式来防止代码被修改 ?
        - [ ] 原则上，guest 代码段被修改必然需要让对应的 tb 也是被 invalidate 的呀

- [ ] tb_gen_code 的 cflags 是做什么的 ?
  - compile flags
  - 来控制什么 ?
  - CPUState 中间的 cpu_index, cluster_index 做啥的 ?

- [ ] page_find_alloc 中间为什么需要使用 rcu

# tcg.c 的代码分析

- [ ] tcg_region_state
  - [ ] tcg_region_reset_all

- [ ] tcg_target_qemu_prologue
  - [ ] buf0 和 buf1 在做什么
  - target_x86_to_mips_static_codes
  - tcg_set_frame
  - tcg_out_pool_finalize
  - flush_icache_range : qemu 本身作为用户态的程序，为什么需要进行 flush icache
  - tcg_register_jit

- [ ] TCG_TARGET_NEED_POOL_LABELS

- [ ] tcg_tb_alloc

- [ ] 应该存在一个直接分配一个连续空间才对，之后的将所有分配的 tb 都是放到哪里就可以了

`s->code_gen_highwater` 

`s->code_gen_ptr`

- tcg_exec_init
  - cpu_gen_init
    - tcg_context_init : 各种 ops 的初始化
  - page_init
  - tb_htable_init : 应该是用来处理
  - code_gen_alloc
  - tcg_prologue_init

tcg_region 到底是什么东西呀?

code_gen_ptr 和 data_gen_ptr 都是意思啊
  - [ ] 从 tcg_tb_alloc 中看，就是连续分配的啊
  - 从 tcg_code_size 看， code_gen_ptr  code_gen_buffer 分别是缓冲区的尾和头

将 code_gen_buffer 划分为大小相等的 regions，


## cputlb.c
在 notdirty_write 的作用是什么?

[^2] 中的 Memory maps and TLBs 分析一些问题
- [ ] 为什么 flush TLB 这种事情有的情况必须让这个 cpu 做: 
  - TLB Update (update a CPUTLBEntry, via tlb_set_page_with_attrs) - This is a per-vCPU table - by definition can’t race - updated by its own thread when the slow-path is forced
- [ ] dirty page tracing 到底如何实现这些工作的?
  - Dirty page tracking (for code gen, SMC detection, migration and display)


### 什么是 mmu idx
```c
#define NB_MMU_MODES 3

typedef struct CPUTLB {
    CPUTLBCommon c;
    CPUTLBDesc d[NB_MMU_MODES];
    CPUTLBDescFast f[NB_MMU_MODES];
} CPUTLB;
```

- [x] 深入理解一下 tlb_hit 和 victim_tlb_hit
  - tlb_hit 的实现很容易，通过 cpu_mmu_index 获取 mmu_idx, 然后就可以得到对应的 TLB entry 了，然后比较即可
  - victim_tlb_hit 是一个全相连的 TLB

使用 mmu idx 的原因是，因为为了将各种状态下的 TLB 分类保存，
例如在用户态下，SMAP[^1] 之类的
```c
static inline int cpu_mmu_index(CPUX86State *env, bool ifetch)
{
    return (env->hflags & HF_CPL_MASK) == 3 ? MMU_USER_IDX :
        (!(env->hflags & HF_SMAP_MASK) || (env->eflags & AC_MASK))
        ? MMU_KNOSMAP_IDX : MMU_KSMAP_IDX;
}
```

两个 flush 的接口， tlb_flush_page_by_mmuidx 和 tlb_flush_by_mmuidx 一个用于 flush 一个，一个用于 flush 全部 tlb

### remote tlb shoot
很多时候，需要将 remote 的 TLB 清理掉，但是 remote 的 cpu 还在运行，所以必须确定了 remote cpu 不会使用
TLB 才可以返回。

- [ ] async_run_on_cpu : 首先将代码实现出来
  - qemu_cpu_kick
    - cpu_exit : 如果是 qemu_tcg_mttcg_enabled 那么就对于所有的 cpu 进行 cpu_exit
      - `atomic_set(&cpu_neg(cpu)->icount_decr.u16.high, -1);` : 猜测这个会导致接下来 tb 执行退出 ?
        - [ ] icount_decr 只是在 TB 开始的位置检查，怎么办 ? (tr_gen_tb_start)

## 其他
- [ ] tb_jmp_cache 是个啥
  - [ ] tb_flush_jmp_cache

## 分析 memory_ldst.c.inc

[^1]: https://lwn.net/Articles/517475/
[^2]: https://qemu.readthedocs.io/en/latest/devel/multi-thread-tcg.html
