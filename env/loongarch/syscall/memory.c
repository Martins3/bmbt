#include "internal.h"
#include <asm/addrspace.h>
#include <pfn.h>
#include <qemu/config-target.h>
#include <qemu/queue.h>
#include <qemu/units.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/mman.h>

size_t size_code_gen_buffer(size_t tb_size);

#define DEBUG_MEM
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

// TMP_TODO 真的需要这么大吗
// 将 pages 清理一下
#define POOL_SIZE 1000
static FreeMem node_pool[POOL_SIZE];

static void init_node_pool() {
  QLIST_INIT(&free_nodes);
  for (int i = 0; i < POOL_SIZE; ++i) {
    QLIST_INSERT_HEAD(&free_nodes, node_pool + i, mem_next);
  }
}

static FreeMem *get_mem_node(unsigned long start, unsigned long size) {
  duck_assert(!QLIST_EMPTY(&free_nodes));
  FreeMem *node = QLIST_FIRST(&free_nodes);
  node->start = start;
  node->len = size;
  QLIST_REMOVE(node, mem_next);
  return node;
}

static void free_mem_node(FreeMem *node) {
  QLIST_REMOVE(node, mem_next);
  QLIST_INSERT_HEAD(&free_nodes, node, mem_next);
}

static void add_head_sentinel() {
  FreeMem *sentinel = get_mem_node(0, 0);
  if (QLIST_EMPTY(&free_mem) || QLIST_FIRST(&free_mem)->len != 0) {
    QLIST_INSERT_HEAD(&free_mem, sentinel, mem_next);
  }
}

// unlike head_sentinel can be merged in kernel_unmmap
// tail_sentinel added in init_pages and won't changed
static void add_tail_sentinel() {
  FreeMem *tail_sentinel = get_mem_node(-1, 0);
  FreeMem *head_sentinel = QLIST_FIRST(&free_mem);
  QLIST_INSERT_AFTER(head_sentinel, tail_sentinel, mem_next);
}

static const unsigned long valid_prot = (PROT_READ | PROT_WRITE | PROT_EXEC);
static const unsigned long valid_flags = (MAP_PRIVATE | MAP_ANON);

long kernel_mmap(long arg0, long arg1, long arg2, long arg3, long arg4,
                 long arg5, long arg6) {

  unsigned long addr = arg0;
  unsigned long len = arg1;
  unsigned long prot = arg2;
  unsigned long flags = arg3;
  unsigned long fd = arg4;
  unsigned long offset = arg5;
  duck_assert(addr == 0);
  // duck_assert(!(len & PAGE_MASK)); // no alignment guarantee
  duck_assert(offset == 0);
  duck_assert(fd == -1);
  duck_assert(!(flags & (~valid_flags)));
  duck_assert(!(prot & (~valid_prot)));

  unsigned long pages = PFN_UP(len);
  pages = pages << PAGE_SHIFT;
  duck_printf("huxueshi:%s %lx\n", __FUNCTION__, pages);

  FreeMem *mem;
  QLIST_FOREACH(mem, &free_mem, mem_next) {
    duck_printf("len :%s %lx\n", __FUNCTION__, mem->len);
    if (pages > mem->len) {
      continue;
    }

    if (pages == mem->len) {
      free_mem_node(mem);
      return mem->start;
    }
    mem->len -= pages;
    duck_printf("return mem : [%lx]\n", mem->start + mem->len);
    return mem->start + mem->len;
  }
  duck_assert(false);
  return -1;
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
    duck_printf("%016lx -- %016lx\n", mem->start, mem->start + mem->len);
  }
}

long kernel_unmmap(long arg0, long arg1, long arg2, long arg3, long arg4,
                   long arg5, long arg6) {
  unsigned long addr = arg0;
  unsigned long len = arg1;
  duck_assert(!(addr & PAGE_OFFSET_MASK));
  duck_printf("---> %lx", len);
  // duck_assert(!(len & PAGE_OFFSET_MASK));

  unsigned long pages = PFN_UP(len);
  pages = pages << PAGE_SHIFT;

  FreeMem *right = NULL;
  FreeMem *left = NULL;
  QLIST_FOREACH(right, &free_mem, mem_next) {
    if (right->start > addr) {
      break;
    }
    left = right;
  }
  duck_assert(right != NULL);
  duck_assert(left != NULL);
  duck_assert(left->start + left->len <= addr);

  FreeMem *node = get_mem_node(addr, pages);
  QLIST_INSERT_AFTER(left, node, mem_next);

  merge(merge(left, node), right);
  add_head_sentinel();
#ifdef DEBUG_MEM
  print_freemem();
#endif
  return 0;
}

void init_pages() {
  QLIST_INIT(&free_mem);
  init_node_pool();
  add_head_sentinel();
  add_tail_sentinel();
}

void fw_add_mem(unsigned long addr, unsigned long len) {
  unsigned long va = TO_CAC(addr);
  kernel_unmmap(va, len, 0, 0, 0, 0, 0);
}
