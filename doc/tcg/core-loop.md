# TCG
在 notes/zhangfuxin/qemu-llvm-docs/QEMU/QEMU-tcg-02.txt 中存在一些简单的描述 tcg
的大致执行流程，但是这些内容有点老，很多问题也没有分析清楚，下面重新分析一下

## 问题
- [ ] how cross page works?
- [ ] cross-page-check.h 算是少数从 LATX 入侵到公共部分的代码了
- [ ] tb 查询到底如何操作的 ?
- [ ] 从 translate-all.c 到 tcg.c 的调用图制作一下
- [ ] TCG_HIGHWATER 的作用和内核中的 high water 不是一个意思，起作用是防止分配的时候越过边界
- [ ] 中断之前不是说和 信号 存在关联吗?
- [ ] 每次执行完成一个 tb 就进行检查中断吗?
    - [ ] 对应的检查代码在哪里? 根本没有看到啊
    - 在 tr_gen_tb_start 生成了 icount 的检查代码，那个并不是 interrupt 的检查机制。

## 总体的执行流程

- qemu_tcg_rr_cpu_thread_fn
  - tcg_cpu_exec
    - cpu_exec
      - cpu_handle_exception
      - cpu_handle_interrupt
      - tb_find
        - tb_lookup__cpu_state
        - tb_gen_code
          - target_x86_to_mips_host : 原来的 tcg 入口是 tcg_gen_code, 现在移除了 tcg
            - tr_disasm
              - get_ir1_list
                - `__disasm_one_ir1`
                  - cpu_read_code_via_qemu
                    - cpu_ldub_code : x86-qemu-mips/include/exec/cpu_ldst_template.h 中间生成的
                  - ir1_disasm
                    - cs_disasm : 这个就是 capstone 的代码了
            - tr_translate_tb
              - tr_ir2_generate
                - ir1_translate
          - tb_link_page
          - tcg_tb_insert
      - cpu_loop_exec_tb
        - cpu_tb_exec
          - tcg_qemu_tb_exec

在 cpu_exec 的 while 循环中，tb_find 找到 tb, 然后 cpu_loop_exec_tb 执行 tb

## 文件结构
> `target-ARCH/*` 定義了如何將 ARCH binary 反匯編成 TCG IR。tcg/ARCH 定義了如何將 TCG IR 翻譯成 ARCH binary。

所以 ./tcg 还存在作用只有 tcg.c 了,
tcg/tcg.c 中分别 include 下面几个文件，因为 xqm 抛弃了 tcg, 都是没有作用的了:
```c
#include "tcg-ldst.inc.c"
#include "tcg-target.inc.c"
#include "../tcg-pool.inc.c"
```

目前将 accel/tcg 和 tcg 下的内容合并到一起了:
| Filename        | desc                                          |
|-----------------|-----------------------------------------------|
| cpu-exec.c      | tb 的执行, cpu_exec 的所在地                  |
| translate-all.c | 主要处理 tb 和 page 的关系，smc, tb_jmp_cache |
| cputlb.c        | softmmu 管理                                  |
| tcg.c           | tb 的内存管理，tb region 之类的               |

## tb 查找的过程
- tb_lookup__cpu_state(在 v6.0 叫做 tb_lookup)
  - tb_jmp_cache : 是快路径查询查询，使用虚拟地址 tb_jmp_cache_hash_func 计算 hash，在 `cpu->tb_jmp_cache` 中间直接查询出来
  - tb_htable_lookup : 慢路径，通过 get_page_addr_code 获取物理地址，然后通过 tb_hash_func 计算 hash 值, 最后调用 qht_lookup_custom 来查询
  - `atomic_set(&cpu->tb_jmp_cache[hash], tb);` : 如果在 慢路径 上查询成功，那么更新 tb_jmp_cache

首先使用虚拟地址查询(fast)，然后使用物理地址查询(slow), 这么设计的原因为:
    - 在执行 tb 的时候，进行跳转，显然是使用虚拟地址来查询 tb, 如果没有 tb_jmp_cache，那么就首先软件 page walk 计算出来物理地址，使用物理地址来查询

> Translated Code Management Captive employs a code cache, similar to QEMU, which maintains the translated code sequences. The key difference is that we index our translations by guest physical address, while QEMU indexes by guest virtual address. The consequence of this is that our translations are retained and re-used for longer, whereas QEMU must invalidate all translations when the guest page tables are changed. In contrast, we only invalidate translations when self-modifying code is detected. We utilize our ability to write-protect virtual pages to efficiently detect when a guest memory write may modify translated code, and hence invalidate translations only when necessary. A further benefit is that translated code is re-used across different virtual mappings to the same physical address, e.g. when using shared libraries.[^3]

这里 Tom Spink 实际上说的并没有什么道理, 对此猜测可能这些东西都是基于早期的 QEMU 吧:
1. 当 guest page table 发生改变的时候，QEMU 不会 invalidate all translations
2. 而且 shared libraries 对应的 tb QEMU 也是共享的

## tcg region
初始化的工作一直发生在 init thread 中间了:

- x86_cpu_realizefn
  - qemu_init_vcpu : 这里进行选择执行引擎
    - qemu_tcg_init_vcpu : 在 exec thread 中进行执行的, 这里检查了一下，保证即使是多个 cpu ，也只会发生
      - tcg_region_init
          - tcg_n_regions : 计算出来创建多少个 region 出来，因为不同的 cpu 生成 tb 的数量不同，所以一般让 region 多于 cpu 的数量
          - 在 tcg_region_init 使用 tcg_init_ctx.code_gen_buffer 来对于 region 进行赋值
          - 初始化 static struct tcg_region_state region; 关于 region 的所有信息都是放到此处的
          - tcg_region_trees_init : 每一个 tcg_region 建立一个 tb_tc 的 gtree

- tb_gen_code
    - tcg_tb_alloc
      - tcg_region_alloc
    - tb_flush
      - do_tb_flush
        - cpu_tb_jmp_cache_clear : quick cache
        - qht_reset_size : slow cache 清理
        - page_flush_tb : PageDesc 的清理
        - tcg_region_reset_all
            - tcg_region_initial_alloc__locked
              - tcg_region_alloc__locked
                - tcg_region_assign : 将一个 region 分配给一个 TCGContext
            - tcg_region_tree_reset_all

总之，region 的创建是因为多核，每次分配一个 region，从而用于修改 TCGContext::code_gen_buffer

## [ ] cpu_exec_nocache
- [x] nocache 到底指的是什么东西?
    - 这个容易，执行代码，当场翻译，这个 tb 不会给之后复用
- [ ] 为什么会存在这种诡异的需求啊?
    - 好吧，出现的位置都是和 icount / replay 有关啊

调用位置
- cpu_exec_nocache
  - cpu_exec
- cpu_handle_exception

[^1]: https://wiki.qemu.org/Documentation/TCG/frontend-ops
[^2]: https://github.com/S2E/libtcg
[^3]: https://www.usenix.org/system/files/atc19-spink.pdf
