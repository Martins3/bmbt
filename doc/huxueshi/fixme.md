- [ ] translate-all.c : tb 的各种管理工作, 但是管理了很多 page 相关的工作
- tcg.c : 主要是 tb 的一些分配工作在使用，其实几乎没有什么用途了
  - [ ] tcg_prologue_init
  - [ ] 从 tcg_prologue_init 向外发散出来很多东西，不过大大的丰富了 TCGContext 的
  - [ ] tcg.c 在本来的涉及中间，是好几个文件和合集(因为 #include .c 的原因), 到时候拆开 ?
  - [ ] ELF_HOST_MACHINE 到底在干什么 ?
  - [ ] 实际上，将 tcg_prologue_init 包含进来，也只是花费了 1000 行而已，原来的设计中，剩下的代码在干什么 ?
- [ ] cpu.c : 感觉 helper 没有向这里调用，感觉很奇怪啊, 似乎只是一些配置函数 ?
  - [ ] 显然哪里实现还是有考虑有问题，显然 i386/cpu.c 中间还是存在很多我们需要的东西的
- [ ] tcg-all.c : 涉及到初始化 tcg engine, 很短的一个文件

- [ ] 关键问题 : 

# translate-all.c 代码分析
> 最后将这些东西整理到 softmmu 和 translator 中吧


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

## cputlb.c
