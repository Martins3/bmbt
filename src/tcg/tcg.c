#include "tcg.h"
#include <exec/cpu-all.h>
#include <exec/cpu-defs.h>
#include <exec/exec-all.h>
#include <exec/tb-hash.h>
#include <exec/tb-lookup.h>
#include <hw/boards.h>
#include <hw/core/cpu.h>
#include <qemu/atomic.h>
#include <qemu/main-loop.h>
#include <qemu/osdep.h>
#include <qemu/thread.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <sysemu/replay.h>

#define TCG_HIGHWATER 1024

#define CPU_TEMP_BUF_NLONGS 128

#include <uglib.h>
struct tcg_region_tree {
  QemuMutex lock;
  GTree *tree;
  /* padding to avoid false sharing is computed at run-time */
};

#if UINTPTR_MAX == UINT32_MAX
#define ELF_CLASS ELFCLASS32
#else
#define ELF_CLASS ELFCLASS64
#endif
#ifdef HOST_WORDS_BIGENDIAN
#define ELF_DATA ELFDATA2MSB
#else
#define ELF_DATA ELFDATA2LSB
#endif

/* Stack frame parameters.  */
#define REG_SIZE (TCG_TARGET_REG_BITS / 8)
#define SAVE_SIZE ((int)ARRAY_SIZE(tcg_target_callee_save_regs) * REG_SIZE)
#define TEMP_SIZE (CPU_TEMP_BUF_NLONGS * (int)sizeof(long))
#define FRAME_SIZE                                                             \
  ((TCG_STATIC_CALL_ARGS_SIZE + TEMP_SIZE + SAVE_SIZE +                        \
    TCG_TARGET_STACK_ALIGN - 1) &                                              \
   -TCG_TARGET_STACK_ALIGN)
#define SAVE_OFS (TCG_STATIC_CALL_ARGS_SIZE + TEMP_SIZE)

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
  size_t size;   /* size of one region */
  size_t stride; /* .size + guard size */

  /* fields protected by the lock */
  size_t current;       /* current region index */
  size_t agg_size_full; /* aggregate size of full regions */
};

static TCGContext **tcg_ctxs;
static unsigned int n_tcg_ctxs;

static struct tcg_region_state region;
/*
 * This is an array of struct tcg_region_tree's, with padding.
 * We use void * to simplify the computation of region_trees[i]; each
 * struct is found every tree_size bytes.
 */
static void *region_trees;
static size_t tree_size;

static void tcg_region_bounds(size_t curr_region, void **pstart, void **pend) {
  void *start, *end;

  start = region.start_aligned + curr_region * region.stride;
  end = start + region.size;

  if (curr_region == 0) {
    start = region.start;
  }
  if (curr_region == region.n - 1) {
    end = region.end;
  }

  *pstart = start;
  *pend = end;
}

static void tcg_region_assign(TCGContext *s, size_t curr_region) {
  void *start, *end;

  tcg_region_bounds(curr_region, &start, &end);

  s->code_gen_buffer = start;
  s->code_gen_ptr = start;
  s->code_gen_buffer_size = end - start;
  s->code_gen_highwater = end - TCG_HIGHWATER;
}

static bool tcg_region_alloc__locked(TCGContext *s) {
  if (region.current == region.n) {
    return true;
  }
  tcg_region_assign(s, region.current);
  region.current++;
  return false;
}

/*
 * Request a new region once the one in use has filled up.
 * Returns true on error.
 */
static bool tcg_region_alloc(TCGContext *s) {
  bool err;
  /* read the region size now; alloc__locked will overwrite it on success */
  size_t size_full = s->code_gen_buffer_size;

  qemu_mutex_lock(&region.lock);
  err = tcg_region_alloc__locked(s);
  if (!err) {
    region.agg_size_full += size_full - TCG_HIGHWATER;
  }
  qemu_mutex_unlock(&region.lock);
  return err;
}

static struct tcg_region_tree *tc_ptr_to_region_tree(void *p) {
  size_t region_idx;

  if (p < region.start_aligned) {
    region_idx = 0;
  } else {
    ptrdiff_t offset = p - region.start_aligned;

    if (offset > region.stride * (region.n - 1)) {
      region_idx = region.n - 1;
    } else {
      region_idx = offset / region.stride;
    }
  }
  return region_trees + region_idx * tree_size;
}

void tcg_target_qemu_prologue(TCGContext *s);

#ifdef BMBT
static int32_t encode_imm12(uint32_t imm) { return (imm & 0xfff) << 10; }
#endif

/*
 * Allocate TBs right before their corresponding translated code, making
 * sure that TBs and code are on different cache lines.
 */
TranslationBlock *tcg_tb_alloc(TCGContext *s) {
  uintptr_t align = qemu_icache_linesize;
  TranslationBlock *tb;
  void *next;

retry:
  tb = (void *)ROUND_UP((uintptr_t)s->code_gen_ptr, align);
  next = (void *)ROUND_UP((uintptr_t)(tb + 1), align);

  if (unlikely(next > s->code_gen_highwater)) {
    if (tcg_region_alloc(s)) {
      return NULL;
    }
    goto retry;
  }
  qatomic_set(&s->code_gen_ptr, next);
  s->data_gen_ptr = NULL;
  return tb;
}

void tcg_tb_insert(TranslationBlock *tb) {
  struct tcg_region_tree *rt = tc_ptr_to_region_tree(tb->tc.ptr);

  qemu_mutex_lock(&rt->lock);
  g_tree_insert(rt->tree, &tb->tc, tb);
  qemu_mutex_unlock(&rt->lock);
}

void tcg_tb_remove(TranslationBlock *tb) {
  struct tcg_region_tree *rt = tc_ptr_to_region_tree(tb->tc.ptr);

  qemu_mutex_lock(&rt->lock);
  g_tree_remove(rt->tree, &tb->tc);
  qemu_mutex_unlock(&rt->lock);
}

/*
 * Find the TB 'tb' such that
 * tb->tc.ptr <= tc_ptr < tb->tc.ptr + tb->tc.size
 * Return NULL if not found.
 */
TranslationBlock *tcg_tb_lookup(uintptr_t tc_ptr) {
  struct tcg_region_tree *rt = tc_ptr_to_region_tree((void *)tc_ptr);
  TranslationBlock *tb;
  struct tb_tc s = {.ptr = (void *)tc_ptr};

  qemu_mutex_lock(&rt->lock);
  tb = g_tree_lookup(rt->tree, &s);
  qemu_mutex_unlock(&rt->lock);
  return tb;
}

void tcg_prologue_init(TCGContext *s) {
  size_t prologue_size, total_size;
  void *buf0, *buf1;

  /* Put the prologue at the beginning of code_gen_buffer.  */
  buf0 = s->code_gen_buffer;
  total_size = s->code_gen_buffer_size;
  s->code_ptr = buf0;
  s->code_buf = buf0;
  s->data_gen_ptr = NULL;
#ifdef BMBT
  s->code_gen_prologue = buf0;
#endif

  /* Compute a high-water mark, at which we voluntarily flush the buffer
     and start over.  The size here is arbitrary, significantly larger
     than we expect the code generation for any one opcode to require.  */
  s->code_gen_highwater = s->code_gen_buffer + (total_size - TCG_HIGHWATER);

  /* Generate the prologue.  */
  tcg_target_qemu_prologue(s);

  buf1 = s->code_ptr;
  flush_icache_range((uintptr_t)buf0, (uintptr_t)buf1);

  /* Deduct the prologue from the buffer.  */
  prologue_size = tcg_current_code_size(s);
  s->code_gen_ptr = buf1;
  s->code_gen_buffer = buf1;
  s->code_buf = buf1;
  total_size -= prologue_size;
  s->code_gen_buffer_size = total_size;

#ifdef DEBUG_DISAS
  if (qemu_loglevel_mask(CPU_LOG_TB_OUT_ASM)) {
    qemu_log_lock();
    qemu_log("PROLOGUE: [size=%zu]\n", prologue_size);
    if (s->data_gen_ptr) {
      size_t code_size = s->data_gen_ptr - buf0;
      size_t data_size = prologue_size - code_size;
      size_t i;

      log_disas(buf0, code_size);

      for (i = 0; i < data_size; i += sizeof(tcg_target_ulong)) {
        if (sizeof(tcg_target_ulong) == 8) {
          qemu_log("0x%08" PRIxPTR ":  .quad  0x%016" PRIx64 "\n",
                   (uintptr_t)s->data_gen_ptr + i,
                   *(uint64_t *)(s->data_gen_ptr + i));
        } else {
          qemu_log("0x%08" PRIxPTR ":  .long  0x%08x\n",
                   (uintptr_t)s->data_gen_ptr + i,
                   *(uint32_t *)(s->data_gen_ptr + i));
        }
      }
    } else {
      log_disas(buf0, prologue_size);
    }
    qemu_log("\n");
    qemu_log_flush();
    qemu_log_unlock();
  }
#endif

  // code_gen_epilogue is usuelss
#ifdef BMBT
  /* Assert that goto_ptr is implemented completely.  */
  if (TCG_TARGET_HAS_goto_ptr) {
    tcg_debug_assert(s->code_gen_epilogue != NULL);
  }
#endif
}

static void tcg_region_tree_lock_all(void) {
  size_t i;

  for (i = 0; i < region.n; i++) {
    struct tcg_region_tree *rt = region_trees + i * tree_size;

    qemu_mutex_lock(&rt->lock);
  }
}

static void tcg_region_tree_unlock_all(void) {
  size_t i;

  for (i = 0; i < region.n; i++) {
    struct tcg_region_tree *rt = region_trees + i * tree_size;

    qemu_mutex_unlock(&rt->lock);
  }
}

void tcg_tb_foreach(GTraverseFunc func, gpointer user_data) {
  size_t i;

  tcg_region_tree_lock_all();
  for (i = 0; i < region.n; i++) {
    struct tcg_region_tree *rt = region_trees + i * tree_size;

    g_tree_foreach(rt->tree, func, user_data);
  }
  tcg_region_tree_unlock_all();
}

size_t tcg_nb_tbs(void) {
  size_t nb_tbs = 0;
  size_t i;

  tcg_region_tree_lock_all();
  for (i = 0; i < region.n; i++) {
    struct tcg_region_tree *rt = region_trees + i * tree_size;

    nb_tbs += g_tree_nnodes(rt->tree);
  }
  tcg_region_tree_unlock_all();
  return nb_tbs;
}

static void tcg_region_tree_reset_all(void) {
  size_t i;

  tcg_region_tree_lock_all();
  for (i = 0; i < region.n; i++) {
    struct tcg_region_tree *rt = region_trees + i * tree_size;

    /* Increment the refcount first so that destroy acts as a reset */
    g_tree_ref(rt->tree);
    g_tree_destroy(rt->tree);
  }
  tcg_region_tree_unlock_all();
}

/*
 * Perform a context's first region allocation.
 * This function does _not_ increment region.agg_size_full.
 */
static inline bool tcg_region_initial_alloc__locked(TCGContext *s) {
  return tcg_region_alloc__locked(s);
}

/* Call from a safe-work context */
void tcg_region_reset_all(void) {
  unsigned int n_ctxs = qatomic_read(&n_tcg_ctxs);
  unsigned int i;

  qemu_mutex_lock(&region.lock);
  region.current = 0;
  region.agg_size_full = 0;

  for (i = 0; i < n_ctxs; i++) {
    TCGContext *s = qatomic_read(&tcg_ctxs[i]);
    bool err = tcg_region_initial_alloc__locked(s);
    g_assert(!err);
  }
  qemu_mutex_unlock(&region.lock);

  tcg_region_tree_reset_all();
}

static void alloc_tcg_plugin_context(TCGContext *s) {}

void tcg_context_init(TCGContext *s) {
  memset(s, 0, sizeof(*s));

  tcg_ctx = s;
  /*
   * In user-mode we simply share the init context among threads, since we
   * use a single region. See the documentation tcg_region_init() for the
   * reasoning behind this.
   * In softmmu we will have at most max_cpus TCG threads.
   */
#ifdef CONFIG_USER_ONLY
  tcg_ctxs = &tcg_ctx;
  n_tcg_ctxs = 1;
#else
  MachineState *ms = qdev_get_machine();
  unsigned int max_cpus = ms->smp.max_cpus;
  tcg_ctxs = g_new(TCGContext *, max_cpus);
#endif

  tcg_debug_assert(!tcg_regset_test_reg(s->reserved_regs, TCG_AREG0));
}

#ifdef CONFIG_USER_ONLY
static size_t tcg_n_regions(void) { return 1; }
#else
/*
 * It is likely that some vCPUs will translate more code than others, so we
 * first try to set more regions than max_cpus, with those regions being of
 * reasonable size. If that's not possible we make do by evenly dividing
 * the code_gen_buffer among the vCPUs.
 */
static size_t tcg_n_regions(void) {
  size_t i;

  /* Use a single region if all we have is one vCPU thread */
#if !defined(CONFIG_USER_ONLY)
  MachineState *ms = qdev_get_machine();
  unsigned int max_cpus = ms->smp.max_cpus;
  assert(max_cpus == 1);
#endif
  if (max_cpus == 1 || !qemu_tcg_mttcg_enabled()) {
    return 1;
  }

  /* Try to have more regions than max_cpus, with each region being >= 2 MB */
  for (i = 8; i > 0; i--) {
    size_t regions_per_thread = i;
    size_t region_size;

    region_size = tcg_init_ctx.code_gen_buffer_size;
    region_size /= max_cpus * regions_per_thread;

    if (region_size >= 2 * 1024u * 1024) {
      return max_cpus * regions_per_thread;
    }
  }
  /* If we can't, then just allocate one region per vCPU thread */
  return max_cpus;
}
#endif

/* compare a pointer @ptr and a tb_tc @s */
static int ptr_cmp_tb_tc(const void *ptr, const struct tb_tc *s) {
  if (ptr >= s->ptr + s->size) {
    return 1;
  } else if (ptr < s->ptr) {
    return -1;
  }
  return 0;
}

static gint tb_tc_cmp(gconstpointer ap, gconstpointer bp) {
  const struct tb_tc *a = ap;
  const struct tb_tc *b = bp;

  /*
   * When both sizes are set, we know this isn't a lookup.
   * This is the most likely case: every TB must be inserted; lookups
   * are a lot less frequent.
   */
  if (likely(a->size && b->size)) {
    if (a->ptr > b->ptr) {
      return 1;
    } else if (a->ptr < b->ptr) {
      return -1;
    }
    /* a->ptr == b->ptr should happen only on deletions */
    g_assert(a->size == b->size);
    return 0;
  }
  /*
   * All lookups have either .size field set to 0.
   * From the glib sources we see that @ap is always the lookup key. However
   * the docs provide no guarantee, so we just mark this case as likely.
   */
  if (likely(a->size == 0)) {
    return ptr_cmp_tb_tc(a->ptr, b);
  }
  return ptr_cmp_tb_tc(b->ptr, a);
}

static void tcg_region_trees_init(void) {
  size_t i;

  tree_size = ROUND_UP(sizeof(struct tcg_region_tree), qemu_dcache_linesize);
  region_trees = qemu_memalign(qemu_dcache_linesize, region.n * tree_size);
  for (i = 0; i < region.n; i++) {
    struct tcg_region_tree *rt = region_trees + i * tree_size;

    qemu_mutex_init(&rt->lock);
    rt->tree = g_tree_new(tb_tc_cmp);
  }
}

/*
 * Initializes region partitioning.
 *
 * Called at init time from the parent thread (i.e. the one calling
 * tcg_context_init), after the target's TCG globals have been set.
 *
 * Region partitioning works by splitting code_gen_buffer into separate regions,
 * and then assigning regions to TCG threads so that the threads can translate
 * code in parallel without synchronization.
 *
 * In softmmu the number of TCG threads is bounded by max_cpus, so we use at
 * least max_cpus regions in MTTCG. In !MTTCG we use a single region.
 * Note that the TCG options from the command-line (i.e. -accel accel=tcg,[...])
 * must have been parsed before calling this function, since it calls
 * qemu_tcg_mttcg_enabled().
 *
 * In user-mode we use a single region.  Having multiple regions in user-mode
 * is not supported, because the number of vCPU threads (recall that each thread
 * spawned by the guest corresponds to a vCPU thread) is only bounded by the
 * OS, and usually this number is huge (tens of thousands is not uncommon).
 * Thus, given this large bound on the number of vCPU threads and the fact
 * that code_gen_buffer is allocated at compile-time, we cannot guarantee
 * that the availability of at least one region per vCPU thread.
 *
 * However, this user-mode limitation is unlikely to be a significant problem
 * in practice. Multi-threaded guests share most if not all of their translated
 * code, which makes parallel code generation less appealing than in softmmu.
 */
void tcg_region_init(void) {
  void *buf = tcg_init_ctx.code_gen_buffer;
  void *aligned;
  size_t size = tcg_init_ctx.code_gen_buffer_size;
  size_t page_size = qemu_real_host_page_size;
  size_t region_size;
  size_t n_regions;
  size_t i;

  n_regions = tcg_n_regions();

  /* The first region will be 'aligned - buf' bytes larger than the others */
  aligned = QEMU_ALIGN_PTR_UP(buf, page_size);
  g_assert(aligned < tcg_init_ctx.code_gen_buffer + size);
  /*
   * Make region_size a multiple of page_size, using aligned as the start.
   * As a result of this we might end up with a few extra pages at the end of
   * the buffer; we will assign those to the last region.
   */
  region_size = (size - (aligned - buf)) / n_regions;
  region_size = QEMU_ALIGN_DOWN(region_size, page_size);

  /* A region must have at least 2 pages; one code, one guard */
  g_assert(region_size >= 2 * page_size);

  /* init the region struct */
  qemu_mutex_init(&region.lock);
  region.n = n_regions;
  region.size = region_size - page_size;
  region.stride = region_size;
  region.start = buf;
  region.start_aligned = aligned;
  /* page-align the end, since its last page will be a guard page */
  region.end = QEMU_ALIGN_PTR_DOWN(buf + size, page_size);
  /* account for that last guard page */
  region.end -= page_size;

  /* set guard pages */
  for (i = 0; i < region.n; i++) {
    void *start, *end;
    int rc;

    tcg_region_bounds(i, &start, &end);
    rc = qemu_mprotect_none(end, page_size);
    g_assert(!rc);
  }

  tcg_region_trees_init();

  /* In user-mode we support only one ctx, so do the initial allocation now */
#ifdef CONFIG_USER_ONLY
  {
    bool err = tcg_region_initial_alloc__locked(tcg_ctx);

    g_assert(!err);
  }
#endif
}

/*
 * All TCG threads except the parent (i.e. the one that called tcg_context_init
 * and registered the target's TCG globals) must register with this function
 * before initiating translation.
 *
 * In user-mode we just point tcg_ctx to tcg_init_ctx. See the documentation
 * of tcg_region_init() for the reasoning behind this.
 *
 * In softmmu each caller registers its context in tcg_ctxs[]. Note that in
 * softmmu tcg_ctxs[] does not track tcg_ctx_init, since the initial context
 * is not used anymore for translation once this function is called.
 *
 * Not tracking tcg_init_ctx in tcg_ctxs[] in softmmu keeps code that iterates
 * over the array (e.g. tcg_code_size() the same for both softmmu and user-mode.
 */
void tcg_register_thread(void) {
  MachineState *ms = qdev_get_machine();
  TCGContext *s = g_malloc(sizeof(*s));
  // unsigned int i;
  unsigned int n;
  bool err;

  *s = tcg_init_ctx;

  /* Claim an entry in tcg_ctxs */
  n = qatomic_fetch_inc(&n_tcg_ctxs);
  g_assert(n < ms->smp.max_cpus);
  qatomic_set(&tcg_ctxs[n], s);

  if (n > 0) {
    alloc_tcg_plugin_context(s);
  }

  tcg_ctx = s;
  qemu_mutex_lock(&region.lock);
  err = tcg_region_initial_alloc__locked(tcg_ctx);
  g_assert(!err);
  qemu_mutex_unlock(&region.lock);
}
