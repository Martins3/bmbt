#include "tcg.h"
#include "../../include/exec/cpu-all.h"
#include "../../include/exec/cpu-defs.h"
#include "../../include/exec/exec-all.h"
#include "../../include/exec/tb-hash.h"
#include "../../include/exec/tb-lookup.h"
#include "../../include/hw/core/cpu.h"
#include "../../include/qemu/atomic.h"
#include "../../include/qemu/osdep.h"
#include "../../include/sysemu/cpus.h"
#include "../../include/sysemu/replay.h"
#include "../../include/types.h"
#include <stdbool.h>
#include <stddef.h>
#include "../../include/qemu/thread-posix.h"
#include <glib/gtree.h> // remove glib

// FIXME we define similar functios at head of cpu-tlb.c
#define qemu_mutex_lock(m) ({})
#define qemu_mutex_unlock(m) ({})

#define TCG_HIGHWATER 1024

struct tcg_region_tree {
  QemuMutex lock;
  GTree *tree;
  /* padding to avoid false sharing is computed at run-time */
};

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

static struct tcg_region_state region;
/*
 * This is an array of struct tcg_region_tree's, with padding.
 * We use void * to simplify the computation of region_trees[i]; each
 * struct is found every tree_size bytes.
 */
static void *region_trees;
static size_t tree_size;
static TCGRegSet tcg_target_available_regs[TCG_TYPE_COUNT];
static TCGRegSet tcg_target_call_clobber_regs;

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
  atomic_set(&s->code_gen_ptr, next);
  s->data_gen_ptr = NULL;
  return tb;
}

void tcg_tb_insert(TranslationBlock *tb) {
  struct tcg_region_tree *rt = tc_ptr_to_region_tree(tb->tc.ptr);

  qemu_mutex_lock(&rt->lock);
  // FIXME of course, it's impossible to include glib
  // but, understand how this file work and make a reconstruction
  g_tree_insert(rt->tree, &tb->tc, tb);
  qemu_mutex_unlock(&rt->lock);
}

void tcg_tb_remove(TranslationBlock *tb)
{
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
TranslationBlock *tcg_tb_lookup(uintptr_t tc_ptr)
{
    struct tcg_region_tree *rt = tc_ptr_to_region_tree((void *)tc_ptr);
    TranslationBlock *tb;
    struct tb_tc s = { .ptr = (void *)tc_ptr };

    qemu_mutex_lock(&rt->lock);
    tb = g_tree_lookup(rt->tree, &s);
    qemu_mutex_unlock(&rt->lock);
    return tb;
}
