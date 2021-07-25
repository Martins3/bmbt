# cross page check
当我们分析 cross page 的时候，应该是一个 guest 的一个 tb 正好在两个 page 上吧

- [ ] 所以，挑战是什么 ?
- [ ] 为什么 LATX 需要特殊处理这一个事情啊 ?
- [ ] 在 core part 的地方已经处理过 cross page 了吧
  - TranslationBlock::page_next
  - [ ] 顺便问一下，PageDesc 是用于存放 x86 的还是 la 的指令的
- [ ] tb_jmp_cache 的工作方式是什么 ?
  - [ ] 查询的过程，是两级查询过程的

```c
  /* first and second physical page containing code. The lower bit
     of the pointer tells the index in page_next[].
     The list is protected by the TB's page('s) lock(s) */
  uintptr_t page_next[2];

  tb_page_addr_t page_addr[2];
```

1. tb::page_addr 分析 page_unlock_tb  就是 x86 代码所在的物理页的地址
2. PageDesc::first_tb : 在 tb_page_add 中初始化, 赋值就是 TranslationBlock

- src/i386/LATX/translator/cross-page-check.c
- src/i386/LATX/include/cross-page-check.h


## 分析一下 cross-page-check.h

- do_tb_flush
  - CPU_FOREACH(cpu) { cpu_tb_jmp_cache_clear(cpu); }
    - xtm_pf_inc_jc_clear
    - *xtm_cpt_flush* : clear Code Page Table (cpt) : 注意，这是一个可选项目
    - `atomic_set(&cpu->tb_jmp_cache[i], NULL);` : 就是直接 tb_jmp_cache 的吗? 难道不会采用更加复杂的东西吗 ?

- tb_lookup__cpu_state
  - *xtm_cpt_insert_tb*
  
- tb_jmp_cache_clear_page
  - *xtm_cpt_flush_page*
