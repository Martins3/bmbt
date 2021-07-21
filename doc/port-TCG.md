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


[^1]: https://wiki.qemu.org/Documentation/TCG/frontend-ops
[^2]: https://github.com/S2E/libtcg
