# cross page check
- [ ] 在 translate-all.h 中间，为什么反而需要 references
```c
#include "../i386/LATX/include/cross-page-check.h"
```

当我们分析 cross page 的时候，应该是一个 guest 的一个 tb 正好在两个 page 上吧

- [ ] 在 core part 的地方已经处理过 cross page 了吧
  - 为什么 LATX 需要特殊处理这一个事情啊 ?

  - TranslationBlock::page_next
  - [x] 顺便问一下，PageDesc 是用于存放 x86 的还是 la 的指令的
    - 因为 PageDesc 是用于 SMC 的，在于根据 ram_addr 找到所有的 TranslationBlock，所以是 la 指令的

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

## 资料收集
tb_find 中的注释
- 因为我们使用虚拟地址来进行直接跳转的时候，如果一个 x86 tb 是本身是 cross page 的
那么其他的 tb 不能直接跳转到这里，也就是不能调用 tb_find 来进行跳转
- [ ] 那么间接跳转是怎么操作的
- [ ] 为什么 x86 tb 在两个 page 上的时候会出现问题

```c
  /* We don't take care of direct jumps when address mapping changes in
   * system emulation. So it's not safe to make a direct jump to a TB
   * spanning two pages because the mapping for the second page can change.
   */
```
