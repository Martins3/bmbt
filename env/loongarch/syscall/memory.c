#include "buddy.h"
#include "internal.h"
#include <asm/addrspace.h>
#include <asm/debug.h>
#include <linux/pfn.h>
#include <qemu/config-target.h>
#include <qemu/queue.h>
#include <qemu/units.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

// [BMBT_MTTCG 4]
bool memory_ready = false;

static inline unsigned long len_pagesize_up(unsigned long len) {
  return PFN_UP(len) << PAGE_SHIFT;
}

#define USE_BUDDY
/* #define DEBUG_MEM */
/* #define DEBUG_MEM_NODE */
/* #define DEBUG_BUDDY */

#ifdef USE_BUDDY
static struct buddy *buddy;
#define BUDDY_LEVEL 17
#define BUDDY_MAX_PAGE_NUM 16
#endif

/**
 * pages:
 *
 * 0x2000 ram
 * 0x402 bitmap
 * 0x800 code cache
 */
typedef struct FreeMem {
  QLIST_ENTRY(FreeMem) mem_next;
  unsigned long len;
  unsigned long start;
  int idx; // idx in node_pool, for debugging
} FreeMem;

QLIST_HEAD(, FreeMem) free_mem;
QLIST_HEAD(, FreeMem) free_nodes;

#define POOL_SIZE (100)
static FreeMem node_pool[POOL_SIZE];

static void init_node_pool() {
  QLIST_INIT(&free_nodes);
  for (int i = 0; i < POOL_SIZE; ++i) {
    QLIST_INSERT_HEAD(&free_nodes, node_pool + i, mem_next);
    node_pool[i].idx = i;
  }
}

static FreeMem *get_mem_node(unsigned long start, unsigned long size) {
  kern_assert(!QLIST_EMPTY(&free_nodes));
  FreeMem *node = QLIST_FIRST(&free_nodes);
  node->start = start;
  node->len = size;
  QLIST_REMOVE(node, mem_next);
#ifdef DEBUG_MEM_NODE
  kern_printf("huxueshi:%s %d\n", __FUNCTION__, node->idx);
#endif
  return node;
}

#ifdef DEBUG_MEM_NODE
static void check_double_free(FreeMem *node) {
  FreeMem *mem;
  bool in_free_mem = false;
  QLIST_FOREACH(mem, &free_mem, mem_next) {
    if (mem == node) {
      in_free_mem = true;
      break;
    }
  }
  if (!in_free_mem) {
    kern_assert(false);
  }
}
#endif

static void free_mem_node(FreeMem *node) {
#ifdef DEBUG_MEM_NODE
  kern_printf("huxueshi:%s %d \n", __FUNCTION__, node->idx);
  check_double_free(node);
#endif
  QLIST_REMOVE(node, mem_next);
  node->start = -1;
  node->len = 0;
  QLIST_INSERT_HEAD(&free_nodes, node, mem_next);
}

static void add_head_sentinel() {
  kern_assert(!QLIST_EMPTY(&free_mem));
  if (QLIST_FIRST(&free_mem)->len != 0) {
    FreeMem *sentinel = get_mem_node(0, 0);
    QLIST_INSERT_HEAD(&free_mem, sentinel, mem_next);
  }
}

// unlike head_sentinel can be merged in kernel_unmmap
// tail_sentinel added in init_pages and won't changed
static void add_sentinels() {
  kern_assert(QLIST_EMPTY(&free_mem));
  FreeMem *tail_sentinel = get_mem_node(-1, 0);
  QLIST_INSERT_HEAD(&free_mem, tail_sentinel, mem_next);

  FreeMem *head_sentinel = get_mem_node(0, 0);
  QLIST_INSERT_HEAD(&free_mem, head_sentinel, mem_next);
}

static const unsigned long valid_prot = (PROT_READ | PROT_WRITE | PROT_EXEC);
static const unsigned long valid_flags = (MAP_PRIVATE | MAP_ANON);

long kernel_mmap(long arg0, long arg1, long arg2, long arg3, long arg4,
                 long arg5, long arg6) {

  kern_assert(memory_ready);
  unsigned long addr = arg0;
  unsigned long len = arg1;
  unsigned long prot = arg2;
  unsigned long flags = arg3;
  unsigned long fd = arg4;
  unsigned long offset = arg5;
  kern_assert(addr == 0);
  // kern_assert(!(len & PAGE_MASK)); // no alignment guarantee
  kern_assert(fd == -1);
  kern_assert(!(flags & (~valid_flags)));
  kern_assert(!(prot & (~valid_prot)));
  kern_assert(offset == 0);

  len = len_pagesize_up(len);
#ifdef USE_BUDDY
  if (len < PAGE_SIZE * BUDDY_MAX_PAGE_NUM) {
    int idx = buddy_alloc(buddy, len >> PAGE_SHIFT);
    if (idx == -1)
      kern_not_reach("alloc failed");
    addr = buddy_base(buddy) + idx * PAGE_SIZE;
#ifdef DEBUG_BUDDY
    kern_printf("---> addr=%lx idx=%d size=%d\n", addr, idx, len >> PAGE_SHIFT);
    buddy_dump(buddy);
#endif
    memset((void *)addr, 0, len);
    return addr;
  }
#endif

  FreeMem *mem;
  QLIST_FOREACH(mem, &free_mem, mem_next) {
    if (len > mem->len) {
      continue;
    }

    unsigned long addr = mem->start;
    if (len == mem->len) {
      free_mem_node(mem);
    } else {
      mem->len -= len;
      mem->start += len;
    }
    memset((void *)addr, 0, len);
    return addr;
  }
  kern_not_reach("mmap never failed !");
}

long alloc_pages(long pages) {
  return kernel_mmap(0, pages << PAGE_SHIFT, 0, 0, -1, 0, 0);
}

static FreeMem *merge(FreeMem *left, FreeMem *node) {
  if (left->start + left->len == node->start) {
    left->len += node->len;
    free_mem_node(node);
    return left;
  }
  return node;
}

void print_freemem() {
  FreeMem *mem = NULL;
  QLIST_FOREACH(mem, &free_mem, mem_next) {
    kern_printf("idx=%d  %016lx -- %016lx %10lx %8lx\n", mem->idx,
                TO_PHYS(mem->start) >> PAGE_SHIFT,
                TO_PHYS((mem->start + mem->len)) >> PAGE_SHIFT, mem->len,
                mem->len >> PAGE_SHIFT);
  }
}

void print_freenodes() {
  FreeMem *mem = NULL;
  QLIST_FOREACH(mem, &free_nodes, mem_next) { kern_printf("%d ", mem->idx); }
  kern_printf("\n");
}

long kernel_unmmap(long arg0, long arg1, long arg2, long arg3, long arg4,
                   long arg5, long arg6) {
  unsigned long addr = arg0;
  unsigned long len = arg1;
  kern_assert(!(addr & PAGE_OFFSET_MASK));
  kern_assert(!(len & (0x1000 - 1))); // no alignment guarantee

  len = len_pagesize_up(len);

#ifdef USE_BUDDY
  if (len < 16 * PAGE_SIZE) {
    int idx = (addr - buddy_base(buddy)) >> PAGE_SHIFT;
    kern_assert(idx < (1 << BUDDY_LEVEL));
    buddy_free(buddy, idx);
    return 0;
  }
#endif

#ifdef DEBUG_MEM
  kern_printf("[huxueshi:%s:%d] %lx %lx %lx\n", __FUNCTION__, __LINE__,
              addr >> PAGE_SHIFT, len, (addr + len) >> PAGE_SHIFT);
#endif

  FreeMem *right = NULL;
  FreeMem *left = NULL;
  QLIST_FOREACH(right, &free_mem, mem_next) {
    if (right->start > addr) {
      break;
    }
    left = right;
  }
  kern_assert(right != NULL);
  kern_assert(left != NULL);
  kern_assert(left->start + left->len <= addr);
  kern_assert(right->start >= addr + len);

  FreeMem *node = get_mem_node(addr, len);
  QLIST_INSERT_AFTER(left, node, mem_next);
  kern_assert(QLIST_NEXT(node, mem_next) == right);

  left = merge(left, node);
  kern_assert(QLIST_NEXT(left, mem_next) == right);
  merge(left, right);

  add_head_sentinel();
#ifdef DEBUG_MEM
  print_freemem();
  /* print_freenodes(); */
#endif
  return 0;
}

void setup_buddy() {
#ifdef USE_BUDDY
  buddy = buddy_new(BUDDY_LEVEL);
#endif
}

void init_pages() {
  QLIST_INIT(&free_mem);
  init_node_pool();
  add_sentinels();
}

void fw_add_mem(unsigned long start, unsigned long end) {
  kern_assert(end >= start);
  kern_assert((start & PAGE_OFFSET_MASK) == 0);
  kern_assert((end & PAGE_OFFSET_MASK) == 0);
  if (start == end)
    return;
  unsigned long va = TO_CAC(start);
  kernel_unmmap(va, end - start, 0, 0, 0, 0, 0);
}
