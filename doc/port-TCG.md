# TCG
`target-ARCH/*` 定義了如何將 ARCH binary 反匯編成 TCG IR。tcg/ARCH 定義了如何將 TCG IR 翻譯成 ARCH binary。

## tcg.c 的代码分析
- [ ] tcg_region_state
  - [ ] tcg_region_reset_all

- [ ] tcg_target_qemu_prologue
  - [ ] buf0 和 buf1 在做什么
  - target_x86_to_mips_static_codes
  - tcg_set_frame
  - tcg_out_pool_finalize
  - flush_icache_range : qemu 本身作为用户态的程序，为什么需要进行 flush icache
  - tcg_register_jit

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


## [ ] TCGContext : 如何工作的，如何维护的，作用是什么
- [ ] tcg_context_init 的参数写死了是: tcg_init_ctx, 那么其他都是怎么初始化的啊

- [ ] `static TCGContext **tcg_ctxs;` 和 `extern TCGContext *tcg_ctx;` `extern TCGContext tcg_init_ctx;` 的关系是什么?
  - 都是在什么时候初始化的

tb 的管理是比较清晰的, 所有的人都是使用一个的 qht 的
- 这些 tb 的插入都是靠 tb_link_page 进行的，`qht_insert(&tb_ctx.htable, tb, h, &existing_tb);`
- tb_htable_lookup
- tb_htable_init

static struct tcg_region_state region;

- alloc_code_gen_buffer
  - alloc_code_gen_buffer_splitwx_memfd 中实现的操作

在 tcg_register_thread 初始化了 tcg_ctx

```diff
History:        #0
Commit:         b1311c4acf503dc9c1a310cc40b64f05b08833dc
Author:         Emilio G. Cota <cota@braap.org>
Committer:      Richard Henderson <richard.henderson@linaro.org>
Author Date:    Thu 13 Jul 2017 05:15:52 AM CST
Committer Date: Wed 25 Oct 2017 04:53:42 AM CST

tcg: define tcg_init_ctx and make tcg_ctx a pointer

Groundwork for supporting multiple TCG contexts.

The core of this patch is this change to tcg/tcg.h:

> -extern TCGContext tcg_ctx;
> +extern TCGContext tcg_init_ctx;
> +extern TCGContext *tcg_ctx;

Note that for now we set *tcg_ctx to whatever TCGContext is passed
to tcg_context_init -- in this case &tcg_init_ctx.

Reviewed-by: Richard Henderson <rth@twiddle.net>
Signed-off-by: Emilio G. Cota <cota@braap.org>
Signed-off-by: Richard Henderson <richard.henderson@linaro.org>

diff --git a/tcg/tcg.c b/tcg/tcg.c
index 3a73912827..62f418ac8a 100644
--- a/tcg/tcg.c
+++ b/tcg/tcg.c
@@ -382,6 +382,8 @@ void tcg_context_init(TCGContext *s)
     for (; i < ARRAY_SIZE(tcg_target_reg_alloc_order); ++i) {
         indirect_reg_alloc_order[i] = tcg_target_reg_alloc_order[i];
     }
+
+    tcg_ctx = s;
 }
```
应该是曾经每一个线程都是创建一个 tcg_ctx 的

所以，到底为什么需要 tcg_init_ctx 的

## tcg region
可以分析一下 tcg_tb_alloc, 每个 TCGContext 分配的空间是动态分配的，然后进缓冲区划分为一个个的 block
来实现动态分配的

```c
/*
 * We divide code_gen_buffer into equally-sized "regions" that TCG threads
 * dynamically allocate from as demand dictates. Given appropriate region
 * sizing, this minimizes flushes even when some TCG threads generate a lot
 * more code than others.
 */
struct tcg_region_state {
    QemuMutex lock;

    /* fields set at init time */
    void *start;
    void *start_aligned;
    void *end;
    size_t n;
    size_t size; /* size of one region */
    size_t stride; /* .size + guard size */

    /* fields protected by the lock */
    size_t current; /* current region index */
    size_t agg_size_full; /* aggregate size of full regions */
};

static struct tcg_region_state region;
```

- [x] 找到 TCGContext 将 tcg_region_state 分配满了，一次性全部清空掉掉的证据
  - 在 tb_gen_code 中 tcg_tb_alloc 如果失败，那么调用 tb_flush 来将所有的 TCG 清空掉
  - 从 tcg_region_alloc__locked 看，判断 buffer 是否满，就是将 block 一个个分配出去，block 分配完即可

每次分配的时候进行一下 buffer 的时候
```c
static void tcg_region_assign(TCGContext *s, size_t curr_region)
{
    void *start, *end;

    tcg_region_bounds(curr_region, &start, &end);

    s->code_gen_buffer = start;
    s->code_gen_ptr = start;
    s->code_gen_buffer_size = end - start;
    s->code_gen_highwater = end - TCG_HIGHWATER;
}
```

在 tcg_region_init 使用 tcg_init_ctx.code_gen_buffer 来对于 region 进行赋值 ##

## [ ] TCG_HIGHWATER
似乎特别大的用途

## [ ] TCGTemp
NiuGene 说这个东西在 XQM 中移除掉的

tcg_global_alloc 的调用者都是在哪里的 ?
- tcg_target_qemu_prologue -> tcg_set_frame : 
- tcg_context_init :  `ts = tcg_global_reg_new_internal(s, TCG_TYPE_PTR, TCG_AREG0, "env");`
- tcg_x86_init

- tcg_set_frame 创建出来 `TCGTemp *frame_temp`，该成员的使用位置 : temp_allocate_frame 

```c
TCGv_env cpu_env = 0;

// tcg_context_init 中
ts = tcg_global_reg_new_internal(s, TCG_TYPE_PTR, TCG_AREG0, "env"); // 从 TCGContext::temps 分配并且初始化一个槽位
cpu_env = temp_tcgv_ptr(ts); // cpu_env 现在是 TCGContext 的偏移量, 虽然不是完全清楚，但是
```

## tb_gen_code : 让我们来分析一下这个狗东西

1. 了解一下 TCGContext::tb_cflags
    - 这个只是在 tcg/tcg-op.c 中间使用, 但是现在 xqm 中，这个东西直接被移除掉了, 暂时不用考虑

2. cflags 相关的 macro 的引用位置吧

| FLAGS                              | desc                                                                                          |
|------------------------------------|-----------------------------------------------------------------------------------------------|
| CF_LAST_IO                         | 在 cpu_io_recompile 插入，而 cpu_io_recompile 需要在 io_readx 和 io_writex 两个函数的地方使用 |
| CF_NOCACHE                         | cpu_exec_nocache 中插入，检测位置都是在 translate-all.h 中间的                                |
| CF_COUNT_MASK                      | 似乎存储是这个 TB 中间到底有多少指令                                                          |
| CF_USE_ICOUNT                      |                                                                                               |
| CF_PARALLEL                        | mttcg 相关的，还没有开始支持                                                                  |
| CF_CLUSTER_MASK / CF_CLUSTER_SHIFT |                                                                                               |
| CF_INVALID                         | - [ ] 应该追查一下到底什么时候，以及 invalid 一个 tb 所需要进行的操作是什么                   |

3. CPUState::cflags_next_tb 作用?

在 cpu_handle_exception 的下面，是这个数值唯一读取的时候: 
```c
      /* When requested, use an exact setting for cflags for the next
         execution.  This is used for icount, precise smc, and stop-
         after-access watchpoints.  Since this request should never
         have CF_INVALID set, -1 is a convenient invalid value that
         does not require tcg headers for cpu_common_reset.  */
```
此外，cflags_next_tb 将会 tb_find 的参数
而 tb_find 可能会查询以及生成 tb
只有 cpu_io_recompile 和 TARGET_HAS_PRECISE_SMC 特别的初始化这个东西

4. 如何理解 curr_cflags ? 
  - 其实就是封装 parallel_cpus 和 icount 在目前的配置，因为 mttcg 和 icount 都不支持，只是返回 0 了
  - 其实是相当于标准 cflags 了

5. CF_CLUSTER_MASK 的作用
  - 在 tb_gen_code 和 tb_lookup__cpu_state 中都有一个要求将 cflags 的 mask 初始化为当前 cpu 的 cluster 的操作
  - [ ] 加深一下 cluster index 的理解之后再说


6. tb_gen_code 中间有两个 label ： buffer_overflow tb_overflow 分别表示发生了什么事情
    - buffer_overflow : 表示 tb 块太多了，该刷新了
    - tb_overflow : 一个 tb 中间的指令太多了

## `TCGContext::code_gen_*`

```c
typedef struct TCGContext {

  /* Code generation.  Note that we specifically do not use tcg_insn_unit
     here, because there's too much arithmetic throughout that relies
     on addition and subtraction working on bytes.  Rely on the GCC
     extension that allows arithmetic on void*.  */
  void *code_gen_prologue;
  void *code_gen_epilogue;
  void *code_gen_buffer;
  size_t code_gen_buffer_size;
  void *code_gen_ptr;
  void *data_gen_ptr;

  /* Threshold to flush the translated code buffer.  */
  void *code_gen_highwater;

  /* goto_tb support */
  tcg_insn_unit *code_buf;
```

- tcg_prologue_init
  - tcg_target_qemu_prologue

不如深入理解一下 tcg_prologue_init 在干什么 ?

- code_ptr 和 code_buf 出现的位置相当有限，应该是删除的
- data_gen_ptr : 只有两次赋值为 NULL


#### code_gen_buffer 
赋值的地方:
- code_gen_alloc <- tcg_exec_init : 这个位置初始化是需要调用到 mmap 的, 这是发生在主线程中间的, 其实实际上，这是分配一个全局的
- tcg_prologue_init : 因为生成 prologue 和 epilogue 所以需要调整属性
- tcg_region_assign : 在 tcg_tb_alloc 的路径下调用, 这是每一个 thread 获取到 region 之后，来初始化自己的 memory region


#### code_gen_ptr
code_gen_ptr : 下一个 tb 应该存放的位置

从下面可以得到验证:
```c
  atomic_set(
      &tcg_ctx->code_gen_ptr,
      (void *)ROUND_UP((uintptr_t)gen_code_buf + gen_code_size + search_size,
                       CODE_GEN_ALIGN));
```

- 上面的 search_size 是做啥的? 
  - 用于实现精确异常的, 具体可以参考 tb_encode_search 

## tcg_register_jit


[^1]: https://wiki.qemu.org/Documentation/TCG/frontend-ops
[^2]: https://github.com/S2E/libtcg
