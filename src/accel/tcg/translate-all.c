#include <exec/cpu-all.h>
#include <exec/cpu-common.h>
#include <exec/cpu-defs.h>
#include <exec/cputlb.h>
#include <exec/exec-all.h>
#include <exec/tb-context.h>
#include <exec/tb-hash.h>
#include <exec/tb-lookup.h>
#include <hw/core/cpu.h>
#include <qemu/atomic.h>
#include <qemu/bitmap.h>
#include <qemu/main-loop.h>
#include <qemu/osdep.h>
#include <qemu/plugin.h>
#include <qemu/qemu-printf.h>
#include <qemu/thread.h>
#include <sysemu/replay.h>
#ifdef CONFIG_LATX
#include "latx-options.h"
#endif

#include "tcg/tcg.h"
#include <assert.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <uglib.h>

int target_latxs_host(CPUState *cpu, TranslationBlock *tb, int max_insns,
                      void *code_hightwater, int *search_size);

/* #define DEBUG_TB_INVALIDATE */
/* #define DEBUG_TB_FLUSH */
/* make various TB consistency checks */
/* #define DEBUG_TB_CHECK */

#ifdef DEBUG_TB_INVALIDATE
#define DEBUG_TB_INVALIDATE_GATE 1
#else
#define DEBUG_TB_INVALIDATE_GATE 0
#endif

#ifdef DEBUG_TB_FLUSH
#define DEBUG_TB_FLUSH_GATE 1
#else
#define DEBUG_TB_FLUSH_GATE 0
#endif

#if !defined(CONFIG_USER_ONLY)
/* TB consistency checks only implemented for usermode emulation.  */
#undef DEBUG_TB_CHECK
#endif

#ifdef DEBUG_TB_CHECK
#define DEBUG_TB_CHECK_GATE 1
#else
#define DEBUG_TB_CHECK_GATE 0
#endif

/* Access to the various translations structures need to be serialised via locks
 * for consistency.
 * In user-mode emulation access to the memory related structures are protected
 * with mmap_lock.
 * In !user-mode we use per-page locks.
 */
#ifdef CONFIG_SOFTMMU
#define assert_memory_lock()
#else
#define assert_memory_lock() tcg_debug_assert(have_mmap_lock())
#endif

#define SMC_BITMAP_USE_THRESHOLD 10

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
#endif
#ifndef CONFIG_USER_ONLY
  QemuSpin lock;
#endif
} PageDesc;

/**
 * struct page_entry - page descriptor entry
 * @pd:     pointer to the &struct PageDesc of the page this entry represents
 * @index:  page index of the page
 * @locked: whether the page is locked
 *
 * This struct helps us keep track of the locked state of a page, without
 * bloating &struct PageDesc.
 *
 * A page lock protects accesses to all fields of &struct PageDesc.
 *
 * See also: &struct page_collection.
 */
struct page_entry {
  PageDesc *pd;
  tb_page_addr_t index;
  bool locked;
};

/**
 * struct page_collection - tracks a set of pages (i.e. &struct page_entry's)
 * @tree:   Binary search tree (BST) of the pages, with key == page index
 * @max:    Pointer to the page in @tree with the highest page index
 *
 * To avoid deadlock we lock pages in ascending order of page index.
 * When operating on a set of pages, we need to keep track of them so that
 * we can lock them in order and also unlock them later. For this we collect
 * pages (i.e. &struct page_entry's) in a binary search @tree. Given that the
 * @tree implementation we use does not provide an O(1) operation to obtain the
 * highest-ranked element, we use @max to keep track of the inserted page
 * with the highest index. This is valuable because if a page is not in
 * the tree and its index is higher than @max's, then we can lock it
 * without breaking the locking order rule.
 *
 * Note on naming: 'struct page_set' would be shorter, but we already have a few
 * page_set_*() helpers, so page_collection is used instead to avoid confusion.
 *
 * See also: page_collection_lock().
 */
struct page_collection {
  GTree *tree;
  struct page_entry *max;
};

/* list iterators for lists of tagged pointers in TranslationBlock */
#define TB_FOR_EACH_TAGGED(head, tb, n, field)                                 \
  for (n = (head)&1, tb = (TranslationBlock *)((head) & ~1); tb;               \
       tb = (TranslationBlock *)tb->field[n], n = (uintptr_t)tb & 1,           \
      tb = (TranslationBlock *)((uintptr_t)tb & ~1))

#define PAGE_FOR_EACH_TB(pagedesc, tb, n)                                      \
  TB_FOR_EACH_TAGGED((pagedesc)->first_tb, tb, n, page_next)

#define TB_FOR_EACH_JMP(head_tb, tb, n)                                        \
  TB_FOR_EACH_TAGGED((head_tb)->jmp_list_head, tb, n, jmp_list_next)

/* In system mode we want L1_MAP to be based on ram offsets,
   while in user mode we want it to be based on virtual addresses.  */
#if !defined(CONFIG_USER_ONLY)
#if HOST_LONG_BITS < TARGET_PHYS_ADDR_SPACE_BITS
#define L1_MAP_ADDR_SPACE_BITS HOST_LONG_BITS
#else
#define L1_MAP_ADDR_SPACE_BITS TARGET_PHYS_ADDR_SPACE_BITS
#endif
#else
#define L1_MAP_ADDR_SPACE_BITS TARGET_VIRT_ADDR_SPACE_BITS
#endif

/* Size of the L2 (and L3, etc) page tables.  */
#define V_L2_BITS 10
#define V_L2_SIZE (1 << V_L2_BITS)

/* Make sure all possible CPU event bits fit in tb->trace_vcpu_dstate */
QEMU_BUILD_BUG_ON(CPU_TRACE_DSTATE_MAX_EVENTS >
                  sizeof_field(TranslationBlock, trace_vcpu_dstate) *
                      BITS_PER_BYTE);

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

/* code generation context */
TCGContext tcg_init_ctx;
TCGContext *tcg_ctx;
TBContext tb_ctx;
bool parallel_cpus;

static void page_table_config_init(void) {
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

static void cpu_gen_init(void) { tcg_context_init(&tcg_init_ctx); }

#ifndef CONFIG_LATX
/* Encode VAL as a signed leb128 sequence at P.
   Return P incremented past the encoded value.  */
static uint8_t *encode_sleb128(uint8_t *p, target_long val) {
  int more, byte;

  do {
    byte = val & 0x7f;
    val >>= 7;
    more = !((val == 0 && (byte & 0x40) == 0) ||
             (val == -1 && (byte & 0x40) != 0));
    if (more) {
      byte |= 0x80;
    }
    *p++ = byte;
  } while (more);

  return p;
}
#endif

/* Decode a signed leb128 sequence at *PP; increment *PP past the
   decoded value.  Return the decoded value.  */
static target_long decode_sleb128(uint8_t **pp) {
  uint8_t *p = *pp;
  target_long val = 0;
  int byte, shift = 0;

  do {
    byte = *p++;
    val |= (target_ulong)(byte & 0x7f) << shift;
    shift += 7;
  } while (byte & 0x80);
  if (shift < TARGET_LONG_BITS && (byte & 0x40)) {
    val |= -(target_ulong)1 << shift;
  }

  *pp = p;
  return val;
}

#ifndef CONFIG_LATX
/* Encode the data collected about the instructions while compiling TB.
   Place the data at BLOCK, and return the number of bytes consumed.

   The logical table consists of TARGET_INSN_START_WORDS target_ulong's,
   which come from the target's insn_start data, followed by a uintptr_t
   which comes from the host pc of the end of the code implementing the insn.

   Each line of the table is encoded as sleb128 deltas from the previous
   line.  The seed for the first line is { tb->pc, 0..., tb->tc.ptr }.
   That is, the first column is seeded with the guest pc, the last column
   with the host pc, and the middle columns with zeros.  */

static int encode_search(TranslationBlock *tb, uint8_t *block) {
  uint8_t *highwater = tcg_ctx->code_gen_highwater;
  uint8_t *p = block;
  int i, j, n;

  for (i = 0, n = tb->icount; i < n; ++i) {
    target_ulong prev;

    for (j = 0; j < TARGET_INSN_START_WORDS; ++j) {
      if (i == 0) {
        prev = (j == 0 ? tb->pc : 0);
      } else {
        prev = tcg_ctx->gen_insn_data[i - 1][j];
      }
      p = encode_sleb128(p, tcg_ctx->gen_insn_data[i][j] - prev);
    }
    prev = (i == 0 ? 0 : tcg_ctx->gen_insn_end_off[i - 1]);
    p = encode_sleb128(p, tcg_ctx->gen_insn_end_off[i] - prev);

    /* Test for (pending) buffer overflow.  The assumption is that any
       one row beginning below the high water mark cannot overrun
       the buffer completely.  Thus we can test for overflow after
       encoding a row without having to check during encoding.  */
    if (unlikely(p > highwater)) {
      return -1;
    }
  }

  return p - block;
}
#endif

/* The cpu state corresponding to 'searched_pc' is restored.
 * When reset_icount is true, current TB will be interrupted and
 * icount should be recalculated.
 */
static int cpu_restore_state_from_tb(CPUState *cpu, TranslationBlock *tb,
                                     uintptr_t searched_pc, bool reset_icount) {
  target_ulong data[TARGET_INSN_START_WORDS] = {tb->pc};
  uintptr_t host_pc = (uintptr_t)tb->tc.ptr;
  CPUArchState *env = cpu->env_ptr;
  uint8_t *p = tb->tc.ptr + tb->tc.size;
  int i, j, num_insns = tb->icount;
#ifdef CONFIG_PROFILER
  TCGProfile *prof = &tcg_ctx->prof;
  int64_t ti = profile_getclock();
#endif

  searched_pc -= GETPC_ADJ;

  if (searched_pc < host_pc) {
    return -1;
  }

  /* Reconstruct the stored insn data while looking for the point at
     which the end of the insn exceeds the searched_pc.  */
  for (i = 0; i < num_insns; ++i) {
    for (j = 0; j < TARGET_INSN_START_WORDS; ++j) {
      data[j] += decode_sleb128(&p);
    }
    host_pc += decode_sleb128(&p);
    if (host_pc > searched_pc) {
      goto found;
    }
  }
  return -1;

found:
  if (reset_icount && (tb_cflags(tb) & CF_USE_ICOUNT)) {
    assert(use_icount);
    /* Reset the cycle counter to the start of the block
       and shift if to the number of actually executed instructions */
    cpu_neg(cpu)->icount_decr.u16.low += num_insns - i;
  }
  restore_state_to_opc(env, tb, data);

#ifdef CONFIG_PROFILER
  qatomic_set(&prof->restore_time, prof->restore_time + profile_getclock() - ti);
  qatomic_set(&prof->restore_count, prof->restore_count + 1);
#endif
  return 0;
}

bool cpu_restore_state(CPUState *cpu, uintptr_t host_pc, bool will_exit) {
  TranslationBlock *tb;
  bool r = false;
  uintptr_t check_offset;

  /* The host_pc has to be in the region of current code buffer. If
   * it is not we will not be able to resolve it here. The two cases
   * where host_pc will not be correct are:
   *
   *  - fault during translation (instruction fetch)
   *  - fault from helper (not using GETPC() macro)
   *
   * Either way we need return early as we can't resolve it here.
   *
   * We are using unsigned arithmetic so if host_pc <
   * tcg_init_ctx.code_gen_buffer check_offset will wrap to way
   * above the code_gen_buffer_size
   */
  check_offset = host_pc - (uintptr_t)tcg_init_ctx.code_gen_buffer;

  if (check_offset < tcg_init_ctx.code_gen_buffer_size) {
    tb = tcg_tb_lookup(host_pc);
    if (tb) {
      cpu_restore_state_from_tb(cpu, tb, host_pc, will_exit);
      if (tb_cflags(tb) & CF_NOCACHE) {
        /* one-shot translation, invalidate it immediately */
        tb_phys_invalidate(tb, -1);
        tcg_tb_remove(tb);
      }
      r = true;
    }
  }

  return r;
}

static void page_init(void) {
  page_size_init();
  page_table_config_init();

#if defined(CONFIG_BSD) && defined(CONFIG_USER_ONLY)
  {
#ifdef HAVE_KINFO_GETVMMAP
    struct kinfo_vmentry *freep;
    int i, cnt;

    freep = kinfo_getvmmap(getpid(), &cnt);
    if (freep) {
      mmap_lock();
      for (i = 0; i < cnt; i++) {
        unsigned long startaddr, endaddr;

        startaddr = freep[i].kve_start;
        endaddr = freep[i].kve_end;
        if (h2g_valid(startaddr)) {
          startaddr = h2g(startaddr) & TARGET_PAGE_MASK;

          if (h2g_valid(endaddr)) {
            endaddr = h2g(endaddr);
            page_set_flags(startaddr, endaddr, PAGE_RESERVED);
          } else {
#if TARGET_ABI_BITS <= L1_MAP_ADDR_SPACE_BITS
            endaddr = ~0ul;
            page_set_flags(startaddr, endaddr, PAGE_RESERVED);
#endif
          }
        }
      }
      free(freep);
      mmap_unlock();
    }
#else
    FILE *f;

    last_brk = (unsigned long)sbrk(0);

    f = fopen("/compat/linux/proc/self/maps", "r");
    if (f) {
      mmap_lock();

      do {
        unsigned long startaddr, endaddr;
        int n;

        n = fscanf(f, "%lx-%lx %*[^\n]\n", &startaddr, &endaddr);

        if (n == 2 && h2g_valid(startaddr)) {
          startaddr = h2g(startaddr) & TARGET_PAGE_MASK;

          if (h2g_valid(endaddr)) {
            endaddr = h2g(endaddr);
          } else {
            endaddr = ~0ul;
          }
          page_set_flags(startaddr, endaddr, PAGE_RESERVED);
        }
      } while (!feof(f));

      fclose(f);
      mmap_unlock();
    }
#endif
  }
#endif
}

static PageDesc *page_find_alloc(tb_page_addr_t index, int alloc) {
  PageDesc *pd;
  void **lp;
  int i;

  /* Level 1.  Always allocated.  */
  lp = l1_map + ((index >> v_l1_shift) & (v_l1_size - 1));

  /* Level 2..N-1.  */
  for (i = v_l2_levels; i > 0; i--) {
    void **p = qatomic_rcu_read(lp);

    if (p == NULL) {
      void *existing;

      if (!alloc) {
        return NULL;
      }
      p = g_new0(void *, V_L2_SIZE);
      existing = qatomic_cmpxchg(lp, NULL, p);
      if (unlikely(existing)) {
        g_free(p);
        p = existing;
      }
    }

    lp = p + ((index >> (i * V_L2_BITS)) & (V_L2_SIZE - 1));
  }

  pd = qatomic_rcu_read(lp);
  if (pd == NULL) {
    void *existing;

    if (!alloc) {
      return NULL;
    }
    pd = g_new0(PageDesc, V_L2_SIZE);
#ifndef CONFIG_USER_ONLY
    {
      int i;

      for (i = 0; i < V_L2_SIZE; i++) {
        qemu_spin_init(&pd[i].lock);
      }
    }
#endif
    existing = qatomic_cmpxchg(lp, NULL, pd);
    if (unlikely(existing)) {
      g_free(pd);
      pd = existing;
    }
  }

  return pd + (index & (V_L2_SIZE - 1));
}

static inline PageDesc *page_find(tb_page_addr_t index) {
  return page_find_alloc(index, 0);
}

static void page_lock_pair(PageDesc **ret_p1, tb_page_addr_t phys1,
                           PageDesc **ret_p2, tb_page_addr_t phys2, int alloc);

/* In user-mode page locks aren't used; mmap_lock is enough */
#ifdef CONFIG_USER_ONLY

#define assert_page_locked(pd) tcg_debug_assert(have_mmap_lock())

static inline void page_lock(PageDesc *pd) {}

static inline void page_unlock(PageDesc *pd) {}

static inline void page_lock_tb(const TranslationBlock *tb) {}

static inline void page_unlock_tb(const TranslationBlock *tb) {}

struct page_collection *page_collection_lock(tb_page_addr_t start,
                                             tb_page_addr_t end) {
  return NULL;
}

void page_collection_unlock(struct page_collection *set) {}
#else /* !CONFIG_USER_ONLY */

#ifdef CONFIG_DEBUG_TCG

static  GHashTable *ht_pages_locked_debug;

static void ht_pages_locked_debug_init(void) {
  if (ht_pages_locked_debug) {
    return;
  }
  ht_pages_locked_debug = g_hash_table_new(NULL, NULL);
}

static bool page_is_locked(const PageDesc *pd) {
  PageDesc *found;

  ht_pages_locked_debug_init();
  found = g_hash_table_lookup(ht_pages_locked_debug, pd);
  return !!found;
}

static void page_lock__debug(PageDesc *pd) {
  ht_pages_locked_debug_init();
  g_assert(!page_is_locked(pd));
  g_hash_table_insert(ht_pages_locked_debug, pd, pd);
}

static void page_unlock__debug(const PageDesc *pd) {
  bool removed;

  ht_pages_locked_debug_init();
  g_assert(page_is_locked(pd));
  removed = g_hash_table_remove(ht_pages_locked_debug, pd);
  g_assert(removed);
}

static void do_assert_page_locked(const PageDesc *pd, const char *file,
                                  int line) {
  if (unlikely(!page_is_locked(pd))) {
    error_report("assert_page_lock: PageDesc %p not locked @ %s:%d", pd, file,
                 line);
    abort();
  }
}

#define assert_page_locked(pd) do_assert_page_locked(pd, __FILE__, __LINE__)

void assert_no_pages_locked(void) {
  ht_pages_locked_debug_init();
  g_assert(g_hash_table_size(ht_pages_locked_debug) == 0);
}

#else /* !CONFIG_DEBUG_TCG */

#define assert_page_locked(pd)

static inline void page_lock__debug(const PageDesc *pd) {}

static inline void page_unlock__debug(const PageDesc *pd) {}

#endif /* CONFIG_DEBUG_TCG */

static inline void page_lock(PageDesc *pd) {
  page_lock__debug(pd);
  qemu_spin_lock(&pd->lock);
}

static inline void page_unlock(PageDesc *pd) {
  qemu_spin_unlock(&pd->lock);
  page_unlock__debug(pd);
}

/* lock the page(s) of a TB in the correct acquisition order */
static inline void page_lock_tb(const TranslationBlock *tb) {
  page_lock_pair(NULL, tb->page_addr[0], NULL, tb->page_addr[1], 0);
}

static inline void page_unlock_tb(const TranslationBlock *tb) {
  PageDesc *p1 = page_find(tb->page_addr[0] >> TARGET_PAGE_BITS);

  page_unlock(p1);
  if (unlikely(tb->page_addr[1] != -1)) {
    PageDesc *p2 = page_find(tb->page_addr[1] >> TARGET_PAGE_BITS);

    if (p2 != p1) {
      page_unlock(p2);
    }
  }
}

static inline struct page_entry *page_entry_new(PageDesc *pd,
                                                tb_page_addr_t index) {
  struct page_entry *pe = g_malloc(sizeof(*pe));

  pe->index = index;
  pe->pd = pd;
  pe->locked = false;
  return pe;
}

static void page_entry_destroy(gpointer p) {
  struct page_entry *pe = p;

  g_assert(pe->locked);
  page_unlock(pe->pd);
  g_free(pe);
}

/* returns false on success */
static bool page_entry_trylock(struct page_entry *pe) {
  bool busy;

  busy = qemu_spin_trylock(&pe->pd->lock);
  if (!busy) {
    g_assert(!pe->locked);
    pe->locked = true;
    page_lock__debug(pe->pd);
  }
  return busy;
}

static void do_page_entry_lock(struct page_entry *pe) {
  page_lock(pe->pd);
  g_assert(!pe->locked);
  pe->locked = true;
}

static gboolean page_entry_lock(gpointer key, gpointer value, gpointer data) {
  struct page_entry *pe = value;

  do_page_entry_lock(pe);
  return FALSE;
}

static gboolean page_entry_unlock(gpointer key, gpointer value, gpointer data) {
  struct page_entry *pe = value;

  if (pe->locked) {
    pe->locked = false;
    page_unlock(pe->pd);
  }
  return FALSE;
}

/*
 * Trylock a page, and if successful, add the page to a collection.
 * Returns true ("busy") if the page could not be locked; false otherwise.
 */
static bool page_trylock_add(struct page_collection *set, tb_page_addr_t addr) {
  tb_page_addr_t index = addr >> TARGET_PAGE_BITS;
  struct page_entry *pe;
  PageDesc *pd;

  pe = g_tree_lookup(set->tree, &index);
  if (pe) {
    return false;
  }

  pd = page_find(index);
  if (pd == NULL) {
    return false;
  }

  pe = page_entry_new(pd, index);
  g_tree_insert(set->tree, &pe->index, pe);

  /*
   * If this is either (1) the first insertion or (2) a page whose index
   * is higher than any other so far, just lock the page and move on.
   */
  if (set->max == NULL || pe->index > set->max->index) {
    set->max = pe;
    do_page_entry_lock(pe);
    return false;
  }
  /*
   * Try to acquire out-of-order lock; if busy, return busy so that we acquire
   * locks in order.
   */
  return page_entry_trylock(pe);
}

static gint tb_page_addr_cmp(gconstpointer ap, gconstpointer bp,
                             gpointer udata) {
  tb_page_addr_t a = *(const tb_page_addr_t *)ap;
  tb_page_addr_t b = *(const tb_page_addr_t *)bp;

  if (a == b) {
    return 0;
  } else if (a < b) {
    return -1;
  }
  return 1;
}

/*
 * Lock a range of pages ([@start,@end[) as well as the pages of all
 * intersecting TBs.
 * Locking order: acquire locks in ascending order of page index.
 */
struct page_collection *page_collection_lock(tb_page_addr_t start,
                                             tb_page_addr_t end) {
  struct page_collection *set = g_malloc(sizeof(*set));
  tb_page_addr_t index;
  PageDesc *pd;

  start >>= TARGET_PAGE_BITS;
  end >>= TARGET_PAGE_BITS;
  g_assert(start <= end);

  set->tree = g_tree_new_full(tb_page_addr_cmp, NULL, NULL, page_entry_destroy);
  set->max = NULL;
  assert_no_pages_locked();

retry:
  g_tree_foreach(set->tree, page_entry_lock, NULL);

  for (index = start; index <= end; index++) {
    TranslationBlock *tb;
    int n;

    pd = page_find(index);
    if (pd == NULL) {
      continue;
    }
    if (page_trylock_add(set, index << TARGET_PAGE_BITS)) {
      g_tree_foreach(set->tree, page_entry_unlock, NULL);
      goto retry;
    }
    assert_page_locked(pd);
    PAGE_FOR_EACH_TB(pd, tb, n) {
      if (page_trylock_add(set, tb->page_addr[0]) ||
          (tb->page_addr[1] != -1 && page_trylock_add(set, tb->page_addr[1]))) {
        /* drop all locks, and reacquire in order */
        g_tree_foreach(set->tree, page_entry_unlock, NULL);
        goto retry;
      }
    }
  }
  return set;
}

void page_collection_unlock(struct page_collection *set) {
  /* entries are unlocked and freed via page_entry_destroy */
  g_tree_destroy(set->tree);
  g_free(set);
}

#endif /* !CONFIG_USER_ONLY */

static void page_lock_pair(PageDesc **ret_p1, tb_page_addr_t phys1,
                           PageDesc **ret_p2, tb_page_addr_t phys2, int alloc) {
  PageDesc *p1, *p2;
  tb_page_addr_t page1;
  tb_page_addr_t page2;

  assert_memory_lock();
  g_assert(phys1 != -1);

  page1 = phys1 >> TARGET_PAGE_BITS;
  page2 = phys2 >> TARGET_PAGE_BITS;

  p1 = page_find_alloc(page1, alloc);
  if (ret_p1) {
    *ret_p1 = p1;
  }
  if (likely(phys2 == -1)) {
    page_lock(p1);
    return;
  } else if (page1 == page2) {
    page_lock(p1);
    if (ret_p2) {
      *ret_p2 = p1;
    }
    return;
  }
  p2 = page_find_alloc(page2, alloc);
  if (ret_p2) {
    *ret_p2 = p2;
  }
  if (page1 < page2) {
    page_lock(p1);
    page_lock(p2);
  } else {
    page_lock(p2);
    page_lock(p1);
  }
}

#if defined(CONFIG_USER_ONLY)
/* Currently it is not recommended to allocate big chunks of data in
   user mode. It will change when a dedicated libc will be used.  */
/* ??? 64-bit hosts ought to have no problem mmaping data outside the
   region in which the guest needs to run.  Revisit this.  */
#define USE_STATIC_CODE_GEN_BUFFER
#endif

/* Minimum size of the code gen buffer.  This number is randomly chosen,
   but not so small that we can't have a fair number of TB's live.  */
#define MIN_CODE_GEN_BUFFER_SIZE (1024u * 1024)

/* Maximum size of the code gen buffer we'd like to use.  Unless otherwise
   indicated, this is constrained by the range of direct branches on the
   host cpu, as used by the TCG implementation of goto_tb.  */
#if defined(__x86_64__)
#define MAX_CODE_GEN_BUFFER_SIZE (2ul * 1024 * 1024 * 1024)
#elif defined(__sparc__)
#define MAX_CODE_GEN_BUFFER_SIZE (2ul * 1024 * 1024 * 1024)
#elif defined(__powerpc64__)
#define MAX_CODE_GEN_BUFFER_SIZE (2ul * 1024 * 1024 * 1024)
#elif defined(__powerpc__)
#define MAX_CODE_GEN_BUFFER_SIZE (32u * 1024 * 1024)
#elif defined(__aarch64__)
#define MAX_CODE_GEN_BUFFER_SIZE (2ul * 1024 * 1024 * 1024)
#elif defined(__s390x__)
/* We have a +- 4GB range on the branches; leave some slop.  */
#define MAX_CODE_GEN_BUFFER_SIZE (3ul * 1024 * 1024 * 1024)
#elif defined(__mips__)
/* We have a 256MB branch region, but leave room to make sure the
   main executable is also within that region.  */
/*exclude one page just avoid cross_256mb*/
#define MAX_CODE_GEN_BUFFER_SIZE (256ul * 1024 * 1024 - 16ul * 1024)
/*# define MAX_CODE_GEN_BUFFER_SIZE  (128ul * 1024 * 1024)*/
#elif defined(__loongarch__)
#define MAX_CODE_GEN_BUFFER_SIZE (128ul * 1024 * 1024 - 16ul * 1024)
#else
#define MAX_CODE_GEN_BUFFER_SIZE ((size_t)-1)
#endif

#define DEFAULT_CODE_GEN_BUFFER_SIZE_1 (32u * 1024 * 1024)

#define DEFAULT_CODE_GEN_BUFFER_SIZE                                           \
  (DEFAULT_CODE_GEN_BUFFER_SIZE_1 < MAX_CODE_GEN_BUFFER_SIZE                   \
       ? DEFAULT_CODE_GEN_BUFFER_SIZE_1                                        \
       : MAX_CODE_GEN_BUFFER_SIZE)

size_t size_code_gen_buffer(size_t tb_size) {
#if defined(CONFIG_SOFTMMU) && defined(CONFIG_LATX)
    if (option_large_code_cache) {
        return tb_size;
    } else {
        /* max code cache size with b */
        return 128 * 1024 * 1024;
    }
#else
    /* Size the buffer.  */
    if (tb_size == 0) {
        size_t phys_mem = qemu_get_host_physmem();
        if (phys_mem == 0) {
            tb_size = DEFAULT_CODE_GEN_BUFFER_SIZE;
        } else {
            tb_size = MIN(DEFAULT_CODE_GEN_BUFFER_SIZE, phys_mem / 8);
        }
    }
    if (tb_size < MIN_CODE_GEN_BUFFER_SIZE) {
        tb_size = MIN_CODE_GEN_BUFFER_SIZE;
    }
    if (tb_size > MAX_CODE_GEN_BUFFER_SIZE) {
        tb_size = MAX_CODE_GEN_BUFFER_SIZE;
    }
    return tb_size;
#endif
}

#ifdef __mips__
/* In order to use J and JAL within the code_gen_buffer, we require
   that the buffer not cross a 256MB boundary.  */
static inline bool cross_256mb(void *addr, size_t size) {
  return ((uintptr_t)addr ^ ((uintptr_t)addr + size)) & ~0x0ffffffful;
}

/* We weren't able to allocate a buffer without crossing that boundary,
   so make do with the larger portion of the buffer that doesn't cross.
   Returns the new base of the buffer, and adjusts code_gen_buffer_size.  */
static inline void *split_cross_256mb(void *buf1, size_t size1) {
  void *buf2 = (void *)(((uintptr_t)buf1 + size1) & ~0x0ffffffful);
  size_t size2 = buf1 + size1 - buf2;

  size1 = buf2 - buf1;
  if (size1 < size2) {
    size1 = size2;
    buf1 = buf2;
  }

  tcg_ctx->code_gen_buffer_size = size1;
  return buf1;
}
#endif

#ifdef USE_STATIC_CODE_GEN_BUFFER
static uint8_t static_code_gen_buffer[DEFAULT_CODE_GEN_BUFFER_SIZE]
    __attribute__((aligned(CODE_GEN_ALIGN)));

static inline void *alloc_code_gen_buffer(void) {
  void *buf = static_code_gen_buffer;
  void *end = static_code_gen_buffer + sizeof(static_code_gen_buffer);
  size_t size;

  /* page-align the beginning and end of the buffer */
  buf = QEMU_ALIGN_PTR_UP(buf, qemu_real_host_page_size);
  end = QEMU_ALIGN_PTR_DOWN(end, qemu_real_host_page_size);

  size = end - buf;

  /* Honor a command-line option limiting the size of the buffer.  */
  if (size > tcg_ctx->code_gen_buffer_size) {
    size = QEMU_ALIGN_DOWN(tcg_ctx->code_gen_buffer_size,
                           qemu_real_host_page_size);
  }
  tcg_ctx->code_gen_buffer_size = size;

#ifdef __mips__
  if (cross_256mb(buf, size)) {
    buf = split_cross_256mb(buf, size);
    size = tcg_ctx->code_gen_buffer_size;
  }
#endif

  if (qemu_mprotect_rwx(buf, size)) {
    abort();
  }
  qemu_madvise(buf, size, QEMU_MADV_HUGEPAGE);

  return buf;
}
#elif defined(_WIN32)
static inline void *alloc_code_gen_buffer(void) {
  size_t size = tcg_ctx->code_gen_buffer_size;
  return VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT,
                      PAGE_EXECUTE_READWRITE);
}
#else
static inline void *alloc_code_gen_buffer(void) {
  int prot = PROT_WRITE | PROT_READ | PROT_EXEC;
  int flags = MAP_PRIVATE | MAP_ANONYMOUS;
  uintptr_t start = 0;
  size_t size = tcg_ctx->code_gen_buffer_size;
  void *buf;

  /* Constrain the position of the buffer based on the host cpu.
     Note that these addresses are chosen in concert with the
     addresses assigned in the relevant linker script file.  */
#if defined(__PIE__) || defined(__PIC__)
  /* Don't bother setting a preferred location if we're building
     a position-independent executable.  We're more likely to get
     an address near the main executable if we let the kernel
     choose the address.  */
#elif defined(__x86_64__) && defined(MAP_32BIT)
  /* Force the memory down into low memory with the executable.
     Leave the choice of exact location with the kernel.  */
  flags |= MAP_32BIT;
  /* Cannot expect to map more than 800MB in low memory.  */
  if (size > 800u * 1024 * 1024) {
    tcg_ctx->code_gen_buffer_size = size = 800u * 1024 * 1024;
  }
#elif defined(__sparc__)
  start = 0x40000000ul;
#elif defined(__s390x__)
  start = 0x90000000ul;
#elif defined(__mips__)
#if _MIPS_SIM == _ABI64
  /*start = 0x128000000ul;*/
  start = 0xc000000000ul;
#else
  start = 0x08000000ul;
#endif
#endif

  buf = mmap((void *)start, size, prot, flags, -1, 0);

  if (buf == MAP_FAILED) {
    return NULL;
  }
#ifdef __mips__
  if (cross_256mb(buf, size)) {
    /* Try again, with the original still mapped, to avoid re-acquiring
       that 256mb crossing.  This time don't specify an address.  */
    size_t size2;
    void *buf2 = mmap(NULL, size, prot, flags, -1, 0);
    switch ((int)(buf2 != MAP_FAILED)) {
    case 1:
      if (!cross_256mb(buf2, size)) {
        /* Success!  Use the new buffer.  */
        munmap(buf, size);
        break;
      }
      /* Failure.  Work with what we had.  */
      munmap(buf2, size);
      /* fallthru */
    default:
      /* Split the original buffer.  Free the smaller half.  */
      buf2 = split_cross_256mb(buf, size);
      size2 = tcg_ctx->code_gen_buffer_size;
      if (buf == buf2) {
        munmap(buf + size2, size - size2);
      } else {
        munmap(buf, size - size2);
      }
      size = size2;
      break;
    }
    buf = buf2;
  }
#endif

  /* Request large pages for the buffer.  */
  // qemu_madvise(buf, size, QEMU_MADV_HUGEPAGE);

  return buf;
}
#endif /* USE_STATIC_CODE_GEN_BUFFER, WIN32, POSIX */

static inline void code_gen_alloc(size_t tb_size) {
  tcg_ctx->code_gen_buffer_size = size_code_gen_buffer(tb_size);
  tcg_ctx->code_gen_buffer = alloc_code_gen_buffer();
  if (tcg_ctx->code_gen_buffer == NULL) {
    fprintf(stderr, "Could not allocate dynamic translator buffer\n");
    exit(1);
  }
}

static bool tb_cmp(const void *ap, const void *bp) {
  const TranslationBlock *a = ap;
  const TranslationBlock *b = bp;

  return a->pc == b->pc && a->cs_base == b->cs_base && a->flags == b->flags &&
         (tb_cflags(a) & CF_HASH_MASK) == (tb_cflags(b) & CF_HASH_MASK) &&
         a->trace_vcpu_dstate == b->trace_vcpu_dstate &&
         a->page_addr[0] == b->page_addr[0] &&
         a->page_addr[1] == b->page_addr[1];
}

static void tb_htable_init(void) {
  unsigned int mode = QHT_MODE_AUTO_RESIZE;

  qht_init(&tb_ctx.htable, tb_cmp, CODE_GEN_HTABLE_SIZE, mode);
}

/* Must be called before using the QEMU cpus. 'tb_size' is the size
   (in bytes) allocated to the translation buffer. Zero means default
   size. */
void tcg_exec_init(unsigned long tb_size) {
  tcg_allowed = true;
  cpu_gen_init();
  page_init();
  tb_htable_init();
  code_gen_alloc(tb_size);
#if defined(CONFIG_SOFTMMU)
  /* There's no guest base to take into account, so go ahead and
     initialize the prologue now.  */
  tcg_prologue_init(tcg_ctx);
#endif
}

/* call with @p->lock held */
static inline void invalidate_page_bitmap(PageDesc *p) {
  assert_page_locked(p);
#ifdef CONFIG_SOFTMMU
  g_free(p->code_bitmap);
  p->code_bitmap = NULL;
  p->code_write_count = 0;
#endif
}

/* Set to NULL all the 'first_tb' fields in all PageDescs. */
static void page_flush_tb_1(int level, void **lp) {
  int i;

  if (*lp == NULL) {
    return;
  }
  if (level == 0) {
    PageDesc *pd = *lp;

    for (i = 0; i < V_L2_SIZE; ++i) {
      page_lock(&pd[i]);
      pd[i].first_tb = (uintptr_t)NULL;
      invalidate_page_bitmap(pd + i);
      page_unlock(&pd[i]);
    }
  } else {
    void **pp = *lp;

    for (i = 0; i < V_L2_SIZE; ++i) {
      page_flush_tb_1(level - 1, pp + i);
    }
  }
}

static void page_flush_tb(void) {
  int i, l1_sz = v_l1_size;

  for (i = 0; i < l1_sz; i++) {
    page_flush_tb_1(v_l2_levels, l1_map + i);
  }
}

static gboolean tb_host_size_iter(gpointer key, gpointer value, gpointer data) {
  const TranslationBlock *tb = value;
  size_t *size = data;

  *size += tb->tc.size;
  return false;
}

/* flush all the translation blocks */
static void do_tb_flush(CPUState *cpu, run_on_cpu_data tb_flush_count) {
  bool did_flush = false;

  mmap_lock();
  /* If it is already been done on request of another CPU,
   * just retry.
   */
  if (tb_ctx.tb_flush_count != tb_flush_count.host_int) {
    goto done;
  }
  did_flush = true;

  if (DEBUG_TB_FLUSH_GATE) {
    size_t nb_tbs = tcg_nb_tbs();
    size_t host_size = 0;

    tcg_tb_foreach(tb_host_size_iter, &host_size);

    printf("qemu: flush code_size=%zu nb_tbs=%zu avg_tb_size=%zu\n",
           tcg_code_size(), nb_tbs, nb_tbs > 0 ? host_size / nb_tbs : 0);
  }

  CPU_FOREACH(cpu) { cpu_tb_jmp_cache_clear(cpu); }

  qht_reset_size(&tb_ctx.htable, CODE_GEN_HTABLE_SIZE);
  page_flush_tb();

  tcg_region_reset_all();
  /* XXX: flush processor icache at this point if cache flush is
     expensive */
  qatomic_mb_set(&tb_ctx.tb_flush_count, tb_ctx.tb_flush_count + 1);

done:
  mmap_unlock();
  if (did_flush) {
    qemu_plugin_flush_cb();
  }
}

void tb_flush(CPUState *cpu) {
  if (tcg_enabled()) {
    unsigned tb_flush_count = qatomic_mb_read(&tb_ctx.tb_flush_count);

    if (cpu_in_exclusive_context(cpu)) {
      do_tb_flush(cpu, RUN_ON_CPU_HOST_INT(tb_flush_count));
    } else {
      async_safe_run_on_cpu(cpu, do_tb_flush,
                            RUN_ON_CPU_HOST_INT(tb_flush_count));
    }
  }
}

/*
 * Formerly ifdef DEBUG_TB_CHECK. These debug functions are user-mode-only,
 * so in order to prevent bit rot we compile them unconditionally in user-mode,
 * and let the optimizer get rid of them by wrapping their user-only callers
 * with if (DEBUG_TB_CHECK_GATE).
 */
#ifdef CONFIG_USER_ONLY

static void do_tb_invalidate_check(void *p, uint32_t hash, void *userp) {
  TranslationBlock *tb = p;
  target_ulong addr = *(target_ulong *)userp;

  if (!(addr + TARGET_PAGE_SIZE <= tb->pc || addr >= tb->pc + tb->size)) {
    printf("ERROR invalidate: address=" TARGET_FMT_lx " PC=%08lx size=%04x\n",
           addr, (long)tb->pc, tb->size);
  }
}

/* verify that all the pages have correct rights for code
 *
 * Called with mmap_lock held.
 */
static void tb_invalidate_check(target_ulong address) {
  address &= TARGET_PAGE_MASK;
  qht_iter(&tb_ctx.htable, do_tb_invalidate_check, &address);
}

static void do_tb_page_check(void *p, uint32_t hash, void *userp) {
  TranslationBlock *tb = p;
  int flags1, flags2;

  flags1 = page_get_flags(tb->pc);
  flags2 = page_get_flags(tb->pc + tb->size - 1);
  if ((flags1 & PAGE_WRITE) || (flags2 & PAGE_WRITE)) {
    printf("ERROR page flags: PC=%08lx size=%04x f1=%x f2=%x\n", (long)tb->pc,
           tb->size, flags1, flags2);
  }
}

/* verify that all the pages have correct rights for code */
static void tb_page_check(void) {
  qht_iter(&tb_ctx.htable, do_tb_page_check, NULL);
}

#endif /* CONFIG_USER_ONLY */

/*
 * user-mode: call with mmap_lock held
 * !user-mode: call with @pd->lock held
 */
static inline void tb_page_remove(PageDesc *pd, TranslationBlock *tb) {
  TranslationBlock *tb1;
  uintptr_t *pprev;
  unsigned int n1;

  assert_page_locked(pd);
  pprev = &pd->first_tb;
  PAGE_FOR_EACH_TB(pd, tb1, n1) {
    if (tb1 == tb) {
      *pprev = tb1->page_next[n1];
      return;
    }
    pprev = &tb1->page_next[n1];
  }
  g_assert_not_reached();
}

/* remove @orig from its @n_orig-th jump list */
static inline void tb_remove_from_jmp_list(TranslationBlock *orig, int n_orig) {
  uintptr_t ptr, ptr_locked;
  TranslationBlock *dest;
  TranslationBlock *tb;
  uintptr_t *pprev;
  int n;

  /* mark the LSB of jmp_dest[] so that no further jumps can be inserted */
  ptr = qatomic_or_fetch(&orig->jmp_dest[n_orig], 1);
  dest = (TranslationBlock *)(ptr & ~1);
  if (dest == NULL) {
    return;
  }

  qemu_spin_lock(&dest->jmp_lock);
  /*
   * While acquiring the lock, the jump might have been removed if the
   * destination TB was invalidated; check again.
   */
  ptr_locked = qatomic_read(&orig->jmp_dest[n_orig]);
  if (ptr_locked != ptr) {
    qemu_spin_unlock(&dest->jmp_lock);
    /*
     * The only possibility is that the jump was unlinked via
     * tb_jump_unlink(dest). Seeing here another destination would be a bug,
     * because we set the LSB above.
     */
    g_assert(ptr_locked == 1 && dest->cflags & CF_INVALID);
    return;
  }
  /*
   * We first acquired the lock, and since the destination pointer matches,
   * we know for sure that @orig is in the jmp list.
   */
  pprev = &dest->jmp_list_head;
  TB_FOR_EACH_JMP(dest, tb, n) {
    if (tb == orig && n == n_orig) {
      *pprev = tb->jmp_list_next[n];
      /* no need to set orig->jmp_dest[n]; setting the LSB was enough */
      qemu_spin_unlock(&dest->jmp_lock);
      return;
    }
    pprev = &tb->jmp_list_next[n];
  }
  g_assert_not_reached();
}

/* reset the jump entry 'n' of a TB so that it is not chained to
   another TB */
static inline void tb_reset_jump(TranslationBlock *tb, int n) {
  uintptr_t addr = (uintptr_t)(tb->tc.ptr + tb->jmp_reset_offset[n]);
  tb_set_jmp_target(tb, n, addr);
#ifdef CONFIG_LATX
    tb->next_tb[n] = NULL;
#endif
}

/* remove any jumps to the TB */
static inline void tb_jmp_unlink(TranslationBlock *dest) {
  TranslationBlock *tb;
  int n;

  qemu_spin_lock(&dest->jmp_lock);

  TB_FOR_EACH_JMP(dest, tb, n) {
    tb_reset_jump(tb, n);
    qatomic_and(&tb->jmp_dest[n], (uintptr_t)NULL | 1);
    /* No need to clear the list entry; setting the dest ptr is enough */
  }
  dest->jmp_list_head = (uintptr_t)NULL;

  qemu_spin_unlock(&dest->jmp_lock);
}

/*
 * In user-mode, call with mmap_lock held.
 * In !user-mode, if @rm_from_page_list is set, call with the TB's pages'
 * locks held.
 */
static void do_tb_phys_invalidate(TranslationBlock *tb,
                                  bool rm_from_page_list) {
  CPUState *cpu;
  PageDesc *p;
  uint32_t h;
  tb_page_addr_t phys_pc;

  assert_memory_lock();

  /* make sure no further incoming jumps will be chained to this TB */
  qemu_spin_lock(&tb->jmp_lock);
  qatomic_set(&tb->cflags, tb->cflags | CF_INVALID);
  qemu_spin_unlock(&tb->jmp_lock);

  /* remove the TB from the hash list */
  phys_pc = tb->page_addr[0] + (tb->pc & ~TARGET_PAGE_MASK);
  h = tb_hash_func(phys_pc, tb->pc, tb->flags, tb_cflags(tb) & CF_HASH_MASK,
                   tb->trace_vcpu_dstate);
  if (!(tb->cflags & CF_NOCACHE) && !qht_remove(&tb_ctx.htable, tb, h)) {
    return;
  }

  /* remove the TB from the page list */
  if (rm_from_page_list) {
    p = page_find(tb->page_addr[0] >> TARGET_PAGE_BITS);
    tb_page_remove(p, tb);
    invalidate_page_bitmap(p);
    if (tb->page_addr[1] != -1) {
      p = page_find(tb->page_addr[1] >> TARGET_PAGE_BITS);
      tb_page_remove(p, tb);
      invalidate_page_bitmap(p);
    }
  }

  /* remove the TB from the hash list */
  h = tb_jmp_cache_hash_func(tb->pc);
  CPU_FOREACH(cpu) {
    if (qatomic_read(&cpu->tb_jmp_cache[h]) == tb) {
      qatomic_set(&cpu->tb_jmp_cache[h], NULL);
    }
  }

  /* suppress this TB from the two jump lists */
  tb_remove_from_jmp_list(tb, 0);
  tb_remove_from_jmp_list(tb, 1);

  /* suppress any remaining jumps to this TB */
  tb_jmp_unlink(tb);

  qatomic_set(&tcg_ctx->tb_phys_invalidate_count,
             tcg_ctx->tb_phys_invalidate_count + 1);
}

static void tb_phys_invalidate__locked(TranslationBlock *tb) {
  do_tb_phys_invalidate(tb, true);
}

/* invalidate one TB
 *
 * Called with mmap_lock held in user-mode.
 */
void tb_phys_invalidate(TranslationBlock *tb, tb_page_addr_t page_addr) {
  if (page_addr == -1 && tb->page_addr[0] != -1) {
    page_lock_tb(tb);
    do_tb_phys_invalidate(tb, true);
    page_unlock_tb(tb);
  } else {
    do_tb_phys_invalidate(tb, false);
  }
}

#ifdef CONFIG_SOFTMMU
/* call with @p->lock held */
static void build_page_bitmap(PageDesc *p) {
  int n, tb_start, tb_end;
  TranslationBlock *tb;

  assert_page_locked(p);
  p->code_bitmap = bitmap_new(TARGET_PAGE_SIZE);

  PAGE_FOR_EACH_TB(p, tb, n) {
    /* NOTE: this is subtle as a TB may span two physical pages */
    if (n == 0) {
      /* NOTE: tb_end may be after the end of the page, but
         it is not a problem */
      tb_start = tb->pc & ~TARGET_PAGE_MASK;
      tb_end = tb_start + tb->size;
      if (tb_end > TARGET_PAGE_SIZE) {
        tb_end = TARGET_PAGE_SIZE;
      }
    } else {
      tb_start = 0;
      tb_end = ((tb->pc + tb->size) & ~TARGET_PAGE_MASK);
    }
    bitmap_set(p->code_bitmap, tb_start, tb_end - tb_start);
  }
}
#endif

/* add the tb in the target page and protect it if necessary
 *
 * Called with mmap_lock held for user-mode emulation.
 * Called with @p->lock held in !user-mode.
 */
static inline void tb_page_add(PageDesc *p, TranslationBlock *tb,
                               unsigned int n, tb_page_addr_t page_addr) {
#ifndef CONFIG_USER_ONLY
  bool page_already_protected;
#endif

  assert_page_locked(p);

  tb->page_addr[n] = page_addr;
  tb->page_next[n] = p->first_tb;
#ifndef CONFIG_USER_ONLY
  page_already_protected = p->first_tb != (uintptr_t)NULL;
#endif
  p->first_tb = (uintptr_t)tb | n;
  invalidate_page_bitmap(p);

#if defined(CONFIG_USER_ONLY)
  if (p->flags & PAGE_WRITE) {
    target_ulong addr;
    PageDesc *p2;
    int prot;

    /* force the host page as non writable (writes will have a
       page fault + mprotect overhead) */
    page_addr &= qemu_host_page_mask;
    prot = 0;
    for (addr = page_addr; addr < page_addr + qemu_host_page_size;
         addr += TARGET_PAGE_SIZE) {

      p2 = page_find(addr >> TARGET_PAGE_BITS);
      if (!p2) {
        continue;
      }
      prot |= p2->flags;
      p2->flags &= ~PAGE_WRITE;
    }
    mprotect(g2h(page_addr), qemu_host_page_size,
             (prot & PAGE_BITS) & ~PAGE_WRITE);
    if (DEBUG_TB_INVALIDATE_GATE) {
      printf("protecting code page: 0x" TB_PAGE_ADDR_FMT "\n", page_addr);
    }
  }
#else
  /* if some code is already present, then the pages are already
     protected. So we handle the case where only the first TB is
     allocated in a physical page */
  if (!page_already_protected) {
    tlb_protect_code(page_addr);
  }
#endif
}

/* add a new TB and link it to the physical page tables. phys_page2 is
 * (-1) to indicate that only one page contains the TB.
 *
 * Called with mmap_lock held for user-mode emulation.
 *
 * Returns a pointer @tb, or a pointer to an existing TB that matches @tb.
 * Note that in !user-mode, another thread might have already added a TB
 * for the same block of guest code that @tb corresponds to. In that case,
 * the caller should discard the original @tb, and use instead the returned TB.
 */
static TranslationBlock *tb_link_page(TranslationBlock *tb,
                                      tb_page_addr_t phys_pc,
                                      tb_page_addr_t phys_page2) {
  PageDesc *p;
  PageDesc *p2 = NULL;

  assert_memory_lock();

  if (phys_pc == -1) {
    /*
     * If the TB is not associated with a physical RAM page then
     * it must be a temporary one-insn TB, and we have nothing to do
     * except fill in the page_addr[] fields.
     */
    assert(tb->cflags & CF_NOCACHE);
    tb->page_addr[0] = tb->page_addr[1] = -1;
    return tb;
  }

  /*
   * Add the TB to the page list, acquiring first the pages's locks.
   * We keep the locks held until after inserting the TB in the hash table,
   * so that if the insertion fails we know for sure that the TBs are still
   * in the page descriptors.
   * Note that inserting into the hash table first isn't an option, since
   * we can only insert TBs that are fully initialized.
   */
  page_lock_pair(&p, phys_pc, &p2, phys_page2, 1);
  tb_page_add(p, tb, 0, phys_pc & TARGET_PAGE_MASK);
  if (p2) {
    tb_page_add(p2, tb, 1, phys_page2);
  } else {
    tb->page_addr[1] = -1;
  }

  if (!(tb->cflags & CF_NOCACHE)) {
    void *existing_tb = NULL;
    uint32_t h;

    /* add in the hash table */
    h = tb_hash_func(phys_pc, tb->pc, tb->flags, tb->cflags & CF_HASH_MASK,
                     tb->trace_vcpu_dstate);
    qht_insert(&tb_ctx.htable, tb, h, &existing_tb);

    /* remove TB from the page(s) if we couldn't insert it */
    if (unlikely(existing_tb)) {
      tb_page_remove(p, tb);
      invalidate_page_bitmap(p);
      if (p2) {
        tb_page_remove(p2, tb);
        invalidate_page_bitmap(p2);
      }
      tb = existing_tb;
    }
  }

  if (p2 && p2 != p) {
    page_unlock(p2);
  }
  page_unlock(p);

#ifdef CONFIG_USER_ONLY
  if (DEBUG_TB_CHECK_GATE) {
    tb_page_check();
  }
#endif
  return tb;
}

/* Called with mmap_lock held for user mode emulation.  */
TranslationBlock *tb_gen_code(CPUState *cpu, target_ulong pc,
                              target_ulong cs_base, uint32_t flags,
                              int cflags) {

  CPUArchState *env = cpu->env_ptr;
  TranslationBlock *tb, *existing_tb;
  tb_page_addr_t phys_pc, phys_page2;
  target_ulong virt_page2;
  tcg_insn_unit *gen_code_buf;
  int max_insns;
  int gen_code_size, search_size;

  phys_pc = get_page_addr_code(env, pc);

  if (phys_pc == -1) {
    /* Generate a temporary TB with 1 insn in it */
    cflags &= ~CF_COUNT_MASK;
    cflags |= CF_NOCACHE | 1;
  }

  cflags &= ~CF_CLUSTER_MASK;
  cflags |= cpu->cluster_index << CF_CLUSTER_SHIFT;

  max_insns = cflags & CF_COUNT_MASK;
  if (max_insns == 0) {
    max_insns = CF_COUNT_MASK;
  }
  if (max_insns > TCG_MAX_INSNS) {
    max_insns = TCG_MAX_INSNS;
  }

  if (cpu->singlestep_enabled || singlestep) {
    max_insns = 1;
  }

#if defined(CONFIG_LATX) && defined(CONFIG_SOFTMMU)
    int latxs_is_buffer_overflow = 0;
#endif

buffer_overflow:
  tb = tcg_tb_alloc(tcg_ctx);
#if defined(CONFIG_LATX) && defined(CONFIG_SOFTMMU)
  if (unlikely(!tb) || latxs_is_buffer_overflow) {
#else
  if (unlikely(!tb)) {
#endif
    tb_flush(cpu);
    /* Make the execution loop process the flush as soon as possible.  */
    cpu->exception_index = EXCP_INTERRUPT;
    cpu_loop_exit(cpu);
  }

  gen_code_buf = tcg_ctx->code_gen_ptr;
  tb->tc.ptr = gen_code_buf;
  tb->pc = pc;
  tb->cs_base = cs_base;
  tb->flags = flags;
  tb->cflags = cflags;
  tb->orig_tb = NULL;
  tb->trace_vcpu_dstate = *cpu->trace_dstate;
  // tcg_ctx->tb_cflags = cflags;

#ifndef CONFIG_LATX
 tb_overflow:
#else
    tb->_top_out = -1;
    tb->_top_in = -1;
#ifdef CONFIG_SOFTMMU
    tb->next_tb[0] = NULL;
    tb->next_tb[1] = NULL;
    tb->sys_eob_pir1 = NULL;
    tb->tb_too_large_pir1 = NULL;
    tb->is_indir_tb = 0;
#endif
#endif
  /* generate machine code */
  tb->jmp_reset_offset[0] = TB_JMP_RESET_OFFSET_INVALID;
  tb->jmp_reset_offset[1] = TB_JMP_RESET_OFFSET_INVALID;
  tcg_ctx->tb_jmp_reset_offset = tb->jmp_reset_offset;
  if (TCG_TARGET_HAS_direct_jump) {
    tcg_ctx->tb_jmp_insn_offset = tb->jmp_target_arg;
    tcg_ctx->tb_jmp_target_addr = NULL;
  } else {
    tcg_ctx->tb_jmp_insn_offset = NULL;
    tcg_ctx->tb_jmp_target_addr = tb->jmp_target_arg;
  }

  latxs_is_buffer_overflow = 0;
  gen_code_size = target_latxs_host(cpu, tb, max_insns,
                                    tcg_ctx->code_gen_highwater, &search_size);

  if (unlikely(gen_code_size < 0)) {
    switch (gen_code_size) {
    case -1:
      /*
       * Overflow of code_gen_buffer, or the current slice of it.
       */
      latxs_is_buffer_overflow = 1;
      goto buffer_overflow;
    case -2:
      // NiuGenen comment the code
      g_assert_not_reached();
    default:
      g_assert_not_reached();
    }
  }
  tb->tc.size = gen_code_size;
  qatomic_set(
      &tcg_ctx->code_gen_ptr,
      (void *)ROUND_UP((uintptr_t)gen_code_buf + gen_code_size + search_size,
                       CODE_GEN_ALIGN));

  /* init jump list */
  qemu_spin_init(&tb->jmp_lock);
  tb->jmp_list_head = (uintptr_t)NULL;
  tb->jmp_list_next[0] = (uintptr_t)NULL;
  tb->jmp_list_next[1] = (uintptr_t)NULL;
  tb->jmp_dest[0] = (uintptr_t)NULL;
  tb->jmp_dest[1] = (uintptr_t)NULL;

    /* init original jump addresses which have been set during tcg_gen_code() */
#if defined(CONFIG_LATX) && defined(CONFIG_SOFTMMU)
    /* indirect jmp is already linked to jmp glue 2, no need to reset it */
    if (!(tb->is_indir_tb)) {
        if (tb->jmp_reset_offset[0] != TB_JMP_RESET_OFFSET_INVALID) {
            tb_reset_jump(tb, 0);
        }
        if (tb->jmp_reset_offset[1] != TB_JMP_RESET_OFFSET_INVALID) {
            tb_reset_jump(tb, 1);
        }
    }
#endif

  /* check next page if needed */
  virt_page2 = (pc + tb->size - 1) & TARGET_PAGE_MASK;
  phys_page2 = -1;
  if ((pc & TARGET_PAGE_MASK) != virt_page2) {
    phys_page2 = get_page_addr_code(env, virt_page2);
  }
  /*
   * No explicit memory barrier is required -- tb_link_page() makes the
   * TB visible in a consistent state.
   */
  existing_tb = tb_link_page(tb, phys_pc, phys_page2);
  /* if the TB already exists, discard what we just translated */
  if (unlikely(existing_tb != tb)) {
    uintptr_t orig_aligned = (uintptr_t)gen_code_buf;

    orig_aligned -= ROUND_UP(sizeof(*tb), qemu_icache_linesize);
    qatomic_set(&tcg_ctx->code_gen_ptr, (void *)orig_aligned);
    return existing_tb;
  }
  tcg_tb_insert(tb);
  return tb;
}
/*
 * @p must be non-NULL.
 * user-mode: call with mmap_lock held.
 * !user-mode: call with all @pages locked.
 */
static void tb_invalidate_phys_page_range__locked(struct page_collection *pages,
                                                  PageDesc *p,
                                                  tb_page_addr_t start,
                                                  tb_page_addr_t end,
                                                  uintptr_t retaddr) {
  TranslationBlock *tb;
  tb_page_addr_t tb_start, tb_end;
  int n;
#ifdef TARGET_HAS_PRECISE_SMC
  CPUState *cpu = current_cpu;
  CPUArchState *env = NULL;
  bool current_tb_not_found = retaddr != 0;
  bool current_tb_modified = false;
  TranslationBlock *current_tb = NULL;
  target_ulong current_pc = 0;
  target_ulong current_cs_base = 0;
  uint32_t current_flags = 0;
#endif /* TARGET_HAS_PRECISE_SMC */

  assert_page_locked(p);

#if defined(TARGET_HAS_PRECISE_SMC)
  if (cpu != NULL) {
    env = cpu->env_ptr;
  }
#endif

  /* we remove all the TBs in the range [start, end[ */
  /* XXX: see if in some cases it could be faster to invalidate all
     the code */
  PAGE_FOR_EACH_TB(p, tb, n) {
    assert_page_locked(p);
    /* NOTE: this is subtle as a TB may span two physical pages */
    if (n == 0) {
      /* NOTE: tb_end may be after the end of the page, but
         it is not a problem */
      tb_start = tb->page_addr[0] + (tb->pc & ~TARGET_PAGE_MASK);
      tb_end = tb_start + tb->size;
    } else {
      tb_start = tb->page_addr[1];
      tb_end = tb_start + ((tb->pc + tb->size) & ~TARGET_PAGE_MASK);
    }
    if (!(tb_end <= start || tb_start >= end)) {
#ifdef TARGET_HAS_PRECISE_SMC
      if (current_tb_not_found) {
        current_tb_not_found = false;
        /* now we have a real cpu fault */
        current_tb = tcg_tb_lookup(retaddr);
      }
      if (current_tb == tb && (tb_cflags(current_tb) & CF_COUNT_MASK) != 1) {
        /*
         * If we are modifying the current TB, we must stop
         * its execution. We could be more precise by checking
         * that the modification is after the current PC, but it
         * would require a specialized function to partially
         * restore the CPU state.
         */
        current_tb_modified = true;
        cpu_restore_state_from_tb(cpu, current_tb, retaddr, true);
        cpu_get_tb_cpu_state(env, &current_pc, &current_cs_base,
                             &current_flags);
      }
#endif /* TARGET_HAS_PRECISE_SMC */
      tb_phys_invalidate__locked(tb);
    }
  }
#if !defined(CONFIG_USER_ONLY)
  /* if no code remaining, no need to continue to use slow writes */
  if (!p->first_tb) {
    invalidate_page_bitmap(p);
    tlb_unprotect_code(start);
  }
#endif
#ifdef TARGET_HAS_PRECISE_SMC
  if (current_tb_modified) {
    page_collection_unlock(pages);
    /* Force execution of one insn next time.  */
    cpu->cflags_next_tb = 1 | curr_cflags();
    mmap_unlock();
    cpu_loop_exit_noexc(cpu);
  }
#endif
}

/*
 * Invalidate all TBs which intersect with the target physical address range
 * [start;end[. NOTE: start and end must refer to the *same* physical page.
 * 'is_cpu_write_access' should be true if called from a real cpu write
 * access: the virtual CPU will exit the current TB if code is modified inside
 * this TB.
 *
 * Called with mmap_lock held for user-mode emulation
 */
void tb_invalidate_phys_page_range(tb_page_addr_t start, tb_page_addr_t end) {
  struct page_collection *pages;
  PageDesc *p;

  assert_memory_lock();

  p = page_find(start >> TARGET_PAGE_BITS);
  if (p == NULL) {
    return;
  }
  pages = page_collection_lock(start, end);
  tb_invalidate_phys_page_range__locked(pages, p, start, end, 0);
  page_collection_unlock(pages);
}

/*
 * Invalidate all TBs which intersect with the target physical address range
 * [start;end[. NOTE: start and end may refer to *different* physical pages.
 * 'is_cpu_write_access' should be true if called from a real cpu write
 * access: the virtual CPU will exit the current TB if code is modified inside
 * this TB.
 *
 * Called with mmap_lock held for user-mode emulation.
 */
#ifdef CONFIG_SOFTMMU
void tb_invalidate_phys_range(ram_addr_t start, ram_addr_t end)
#else
void tb_invalidate_phys_range(target_ulong start, target_ulong end)
#endif
{
  struct page_collection *pages;
  tb_page_addr_t next;

  assert_memory_lock();

  pages = page_collection_lock(start, end);
  for (next = (start & TARGET_PAGE_MASK) + TARGET_PAGE_SIZE; start < end;
       start = next, next += TARGET_PAGE_SIZE) {
    PageDesc *pd = page_find(start >> TARGET_PAGE_BITS);
    tb_page_addr_t bound = MIN(next, end);

    if (pd == NULL) {
      continue;
    }
    tb_invalidate_phys_page_range__locked(pages, pd, start, bound, 0);
  }
  page_collection_unlock(pages);
}

#ifdef CONFIG_SOFTMMU
/* len must be <= 8 and start must be a multiple of len.
 * Called via softmmu_template.h when code areas are written to with
 * iothread mutex not held.
 *
 * Call with all @pages in the range [@start, @start + len[ locked.
 */
void tb_invalidate_phys_page_fast(struct page_collection *pages,
                                  tb_page_addr_t start, int len,
                                  uintptr_t retaddr) {
  PageDesc *p;

  assert_memory_lock();

  p = page_find(start >> TARGET_PAGE_BITS);
  if (!p) {
    return;
  }

  assert_page_locked(p);
  if (!p->code_bitmap && ++p->code_write_count >= SMC_BITMAP_USE_THRESHOLD) {
    build_page_bitmap(p);
  }
  if (p->code_bitmap) {
    unsigned int nr;
    unsigned long b;

    nr = start & ~TARGET_PAGE_MASK;
    b = p->code_bitmap[BIT_WORD(nr)] >> (nr & (BITS_PER_LONG - 1));
    if (b & ((1 << len) - 1)) {
      goto do_invalidate;
    }
  } else {
  do_invalidate:
    tb_invalidate_phys_page_range__locked(pages, p, start, start + len,
                                          retaddr);
  }
}
#else
/* Called with mmap_lock held. If pc is not 0 then it indicates the
 * host PC of the faulting store instruction that caused this invalidate.
 * Returns true if the caller needs to abort execution of the current
 * TB (because it was modified by this store and the guest CPU has
 * precise-SMC semantics).
 */
static bool tb_invalidate_phys_page(tb_page_addr_t addr, uintptr_t pc) {
  TranslationBlock *tb;
  PageDesc *p;
  int n;
#ifdef TARGET_HAS_PRECISE_SMC
  TranslationBlock *current_tb = NULL;
  CPUState *cpu = current_cpu;
  CPUArchState *env = NULL;
  int current_tb_modified = 0;
  target_ulong current_pc = 0;
  target_ulong current_cs_base = 0;
  uint32_t current_flags = 0;
#endif

  assert_memory_lock();

  addr &= TARGET_PAGE_MASK;
  p = page_find(addr >> TARGET_PAGE_BITS);
  if (!p) {
    return false;
  }

#ifdef TARGET_HAS_PRECISE_SMC
  if (p->first_tb && pc != 0) {
    current_tb = tcg_tb_lookup(pc);
  }
  if (cpu != NULL) {
    env = cpu->env_ptr;
  }
#endif
  assert_page_locked(p);

#if defined(CONFIG_LATX) && defined(CONFIG_USER_ONLY)
  etb_cache_clear();
#endif

  PAGE_FOR_EACH_TB(p, tb, n) {
#ifdef TARGET_HAS_PRECISE_SMC
    if (current_tb == tb && (tb_cflags(current_tb) & CF_COUNT_MASK) != 1) {
      /* If we are modifying the current TB, we must stop
         its execution. We could be more precise by checking
         that the modification is after the current PC, but it
         would require a specialized function to partially
         restore the CPU state */

      current_tb_modified = 1;
      cpu_restore_state_from_tb(cpu, current_tb, pc, true);
      cpu_get_tb_cpu_state(env, &current_pc, &current_cs_base, &current_flags);
    }
#endif /* TARGET_HAS_PRECISE_SMC */
    tb_phys_invalidate(tb, addr);
  }
  p->first_tb = (uintptr_t)NULL;
#ifdef TARGET_HAS_PRECISE_SMC
  if (current_tb_modified) {
    /* Force execution of one insn next time.  */
    cpu->cflags_next_tb = 1 | curr_cflags();
    return true;
  }
#endif

  return false;
}
#endif

/* user-mode: call with mmap_lock held */
void tb_check_watchpoint(CPUState *cpu, uintptr_t retaddr) {
  TranslationBlock *tb;

  assert_memory_lock();

  tb = tcg_tb_lookup(retaddr);
  if (tb) {
    /* We can use retranslation to find the PC.  */
    cpu_restore_state_from_tb(cpu, tb, retaddr, true);
    tb_phys_invalidate(tb, -1);
  } else {
    /* The exception probably happened in a helper.  The CPU state should
       have been saved before calling it. Fetch the PC from there.  */
    CPUArchState *env = cpu->env_ptr;
    target_ulong pc, cs_base;
    tb_page_addr_t addr;
    uint32_t flags;

    cpu_get_tb_cpu_state(env, &pc, &cs_base, &flags);
    addr = get_page_addr_code(env, pc);
    if (addr != -1) {
      tb_invalidate_phys_range(addr, addr + 1);
    }
  }
}

#ifndef CONFIG_USER_ONLY
/* in deterministic execution mode, instructions doing device I/Os
 * must be at the end of the TB.
 *
 * Called by softmmu_template.h, with iothread mutex not held.
 */
void cpu_io_recompile(CPUState *cpu, uintptr_t retaddr) {
  // reached here when cpu->can_do_io == false, but we don't support icount yet
  g_assert_not_reached();
}

static void tb_jmp_cache_clear_page(CPUState *cpu, target_ulong page_addr) {
  unsigned int i, i0 = tb_jmp_cache_hash_page(page_addr);

  for (i = 0; i < TB_JMP_PAGE_SIZE; i++) {
    qatomic_set(&cpu->tb_jmp_cache[i0 + i], NULL);
  }
}

void tb_flush_jmp_cache(CPUState *cpu, target_ulong addr) {
  /* Discard jump cache entries for any tb which might potentially
     overlap the flushed page.  */
  tb_jmp_cache_clear_page(cpu, addr - TARGET_PAGE_SIZE);
  tb_jmp_cache_clear_page(cpu, addr);
}

#ifdef BMBT
static void print_qht_statistics(struct qht_stats hst) {
  uint32_t hgram_opts;
  size_t hgram_bins;
  char *hgram;

  if (!hst.head_buckets) {
    return;
  }
  qemu_printf("TB hash buckets     %zu/%zu (%0.2f%% head buckets used)\n",
              hst.used_head_buckets, hst.head_buckets,
              (double)hst.used_head_buckets / hst.head_buckets * 100);

  hgram_opts = QDIST_PR_BORDER | QDIST_PR_LABELS;
  hgram_opts |= QDIST_PR_100X | QDIST_PR_PERCENT;
  if (qdist_xmax(&hst.occupancy) - qdist_xmin(&hst.occupancy) == 1) {
    hgram_opts |= QDIST_PR_NODECIMAL;
  }
  hgram = qdist_pr(&hst.occupancy, 10, hgram_opts);
  qemu_printf("TB hash occupancy   %0.2f%% avg chain occ. Histogram: %s\n",
              qdist_avg(&hst.occupancy) * 100, hgram);
  g_free(hgram);

  hgram_opts = QDIST_PR_BORDER | QDIST_PR_LABELS;
  hgram_bins = qdist_xmax(&hst.chain) - qdist_xmin(&hst.chain);
  if (hgram_bins > 10) {
    hgram_bins = 10;
  } else {
    hgram_bins = 0;
    hgram_opts |= QDIST_PR_NODECIMAL | QDIST_PR_NOBINRANGE;
  }
  hgram = qdist_pr(&hst.chain, hgram_bins, hgram_opts);
  qemu_printf("TB hash avg chain   %0.3f buckets. Histogram: %s\n",
              qdist_avg(&hst.chain), hgram);
  g_free(hgram);
}
#endif

struct tb_tree_stats {
  size_t nb_tbs;
  size_t host_size;
  size_t target_size;
  size_t max_target_size;
#ifdef CONFIG_LATX
  size_t host_inst_nr;
  size_t target_inst_nr;
  size_t max_target_inst_nr;
  size_t cpl0_nb_tbs;
  size_t cpl0_host_inst_nr;
  size_t cpl0_target_inst_nr;
  size_t cpl0_max_target_inst_nr;
  size_t cpl3_nb_tbs;
  size_t cpl3_host_inst_nr;
  size_t cpl3_target_inst_nr;
  size_t cpl3_max_target_inst_nr;
#endif
  size_t direct_jmp_count;
  size_t direct_jmp2_count;
  size_t cross_page;
};

#ifdef BMBT
static gboolean tb_tree_stats_iter(gpointer key, gpointer value,
                                   gpointer data) {
  const TranslationBlock *tb = value;
  struct tb_tree_stats *tst = data;

  tst->nb_tbs++;
  tst->host_size += tb->tc.size;
  tst->target_size += tb->size;
#ifdef CONFIG_LATX
  tst->host_inst_nr += (tb->tc.size) >> 2;
  tst->target_inst_nr += tb->icount;
  if (tb->icount > tst->max_target_inst_nr) {
    tst->max_target_inst_nr = tb->icount;
  }
  if ((tb->flags & 0x3) == 0x3) {
    tst->cpl3_nb_tbs++;
    tst->cpl3_host_inst_nr += (tb->tc.size) >> 2;
    tst->cpl3_target_inst_nr += tb->icount;
    if (tb->icount > tst->cpl3_max_target_inst_nr) {
      tst->cpl3_max_target_inst_nr = tb->icount;
    }
  } else {
    tst->cpl0_nb_tbs++;
    tst->cpl0_host_inst_nr += (tb->tc.size) >> 2;
    tst->cpl0_target_inst_nr += tb->icount;
    if (tb->icount > tst->cpl0_max_target_inst_nr) {
      tst->cpl0_max_target_inst_nr = tb->icount;
    }
  }
#endif
  if (tb->size > tst->max_target_size) {
    tst->max_target_size = tb->size;
  }
  if (tb->page_addr[1] != -1) {
    tst->cross_page++;
  }
  if (tb->jmp_reset_offset[0] != TB_JMP_RESET_OFFSET_INVALID) {
    tst->direct_jmp_count++;
    if (tb->jmp_reset_offset[1] != TB_JMP_RESET_OFFSET_INVALID) {
      tst->direct_jmp2_count++;
    }
  }
  return false;
}
#endif

#if 0
void dump_exec_info(void)
{
    struct tb_tree_stats tst = {};
    struct qht_stats hst;
    size_t nb_tbs, flush_full, flush_part, flush_elide;
#ifdef CONFIG_LATX
    size_t cpl0_nb_tbs, cpl3_nb_tbs;
#endif

    tcg_tb_foreach(tb_tree_stats_iter, &tst);
    nb_tbs = tst.nb_tbs;
#ifdef CONFIG_LATX
    cpl0_nb_tbs = tst.cpl0_nb_tbs;
    cpl3_nb_tbs = tst.cpl3_nb_tbs;
#endif
    /* XXX: avoid using doubles ? */
    qemu_printf("Translation buffer state:\n");
    /*
     * Report total code size including the padding and TB structs;
     * otherwise users might think "-tb-size" is not honoured.
     * For avg host size we use the precise numbers from tb_tree_stats though.
     */
    qemu_printf("gen code size            %zu/%zu\n",
                tcg_code_size(), tcg_code_capacity());
    qemu_printf("TB count                 %zu\n", nb_tbs);
    qemu_printf("TB avg target size       %zu max=%zu bytes\n",
                nb_tbs ? tst.target_size / nb_tbs : 0,
                tst.max_target_size);
    qemu_printf("TB avg host size         %zu bytes (expansion ratio: %0.1f)\n",
                nb_tbs ? tst.host_size / nb_tbs : 0,
                tst.target_size ? (double)tst.host_size / tst.target_size : 0);
#ifdef CONFIG_LATX
    qemu_printf("TB avg target inst       %zu max=%zu\n",
                nb_tbs ? tst.target_inst_nr / nb_tbs : 0,
                tst.max_target_inst_nr);
    qemu_printf("TB avg host inst         %zu (expansion ratio: %0.1f)\n",
                nb_tbs ? tst.host_inst_nr / nb_tbs : 0,
                tst.target_inst_nr ? (double)tst.host_inst_nr / tst.target_inst_nr : 0);

    qemu_printf("TB CPL0 count            %zu\n", cpl0_nb_tbs);
    qemu_printf("TB CPL0 avg target inst  %zu max=%zu\n",
                cpl0_nb_tbs ? tst.cpl0_target_inst_nr / cpl0_nb_tbs : 0,
                tst.cpl0_max_target_inst_nr);
    qemu_printf("TB CPL0 avg host inst    %zu (expansion ratio: %0.1f)\n",
                cpl0_nb_tbs ? tst.cpl0_host_inst_nr / cpl0_nb_tbs : 0,
                tst.cpl0_target_inst_nr ? (double)tst.cpl0_host_inst_nr / tst.cpl0_target_inst_nr : 0);

    qemu_printf("TB CPL3 count            %zu\n", cpl3_nb_tbs);
    qemu_printf("TB CPL3 avg target inst  %zu max=%zu\n",
                cpl3_nb_tbs ? tst.cpl3_target_inst_nr / cpl3_nb_tbs : 0,
                tst.cpl3_max_target_inst_nr);
    qemu_printf("TB CPL3 avg host inst    %zu (expansion ratio: %0.1f)\n",
                cpl3_nb_tbs ? tst.cpl3_host_inst_nr / cpl3_nb_tbs : 0,
                tst.cpl3_target_inst_nr ? (double)tst.cpl3_host_inst_nr / tst.cpl3_target_inst_nr : 0);
#endif
    qemu_printf("cross page TB count      %zu (%zu%%)\n", tst.cross_page,
                nb_tbs ? (tst.cross_page * 100) / nb_tbs : 0);
    qemu_printf("direct jump count        %zu (%zu%%) (2 jumps=%zu %zu%%)\n",
                tst.direct_jmp_count,
                nb_tbs ? (tst.direct_jmp_count * 100) / nb_tbs : 0,
                tst.direct_jmp2_count,
                nb_tbs ? (tst.direct_jmp2_count * 100) / nb_tbs : 0);

    qht_statistics_init(&tb_ctx.htable, &hst);
    print_qht_statistics(hst);
    qht_statistics_destroy(&hst);

    qemu_printf("\nStatistics:\n");
    qemu_printf("TB flush count      %u\n",
                qatomic_read(&tb_ctx.tb_flush_count));
    qemu_printf("TB invalidate count %zu\n",
                tcg_tb_phys_invalidate_count());

    tlb_flush_counts(&flush_full, &flush_part, &flush_elide);
    qemu_printf("TLB full flushes    %zu\n", flush_full);
    qemu_printf("TLB partial flushes %zu\n", flush_part);
    qemu_printf("TLB elided flushes  %zu\n", flush_elide);
    tcg_dump_info();
}
#endif

#else  /* CONFIG_USER_ONLY */
#endif /* CONFIG_USER_ONLY */

/* This is a wrapper for common code that can not use CONFIG_SOFTMMU */
void tcg_flush_softmmu_tlb(CPUState *cs) {
#ifdef CONFIG_SOFTMMU
  tlb_flush(cs);
#endif
}

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_LATX)
void *latx_test_sys_alloc_tb(void *_cpu, void **highwater) {
  CPUState *cpu = _cpu;
  CPUX86State *env = cpu->env_ptr;

  target_ulong pc;
  target_ulong cs_base;
  uint32_t flags;
  cpu_get_tb_cpu_state(env, &pc, &cs_base, &flags);

  TranslationBlock *tb = tcg_tb_alloc(tcg_ctx);
  tb->tc.ptr = tcg_ctx->code_gen_ptr;
  tb->pc = pc;
  tb->cs_base = cs_base;
  tb->flags = flags;
  tb->cflags = 0;

  tb->jmp_reset_offset[0] = TB_JMP_RESET_OFFSET_INVALID;
  tb->jmp_reset_offset[1] = TB_JMP_RESET_OFFSET_INVALID;

  /* init jump list */
  qemu_spin_init(&tb->jmp_lock);
  tb->jmp_list_head = (uintptr_t)NULL;
  tb->jmp_list_next[0] = (uintptr_t)NULL;
  tb->jmp_list_next[1] = (uintptr_t)NULL;
  tb->jmp_dest[0] = (uintptr_t)NULL;
  tb->jmp_dest[1] = (uintptr_t)NULL;

  /* init original jump addresses which have been set during tcg_gen_code() */
  if (tb->jmp_reset_offset[0] != TB_JMP_RESET_OFFSET_INVALID) {
    tb_reset_jump(tb, 0);
  }
  if (tb->jmp_reset_offset[1] != TB_JMP_RESET_OFFSET_INVALID) {
    tb_reset_jump(tb, 1);
  }

  *highwater = tcg_ctx->code_gen_highwater;

  return tb;
}
#endif
