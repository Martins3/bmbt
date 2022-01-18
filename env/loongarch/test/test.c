#include <assert.h>
#include <limits.h>
#include <qemu/queue.h>
#include <sys/mman.h>
#include <unitest/greatest.h>

typedef struct AllocatedMem {
  QLIST_ENTRY(AllocatedMem) mem_next;
  void *p;
  int pages;
} AllocatedMem;

QLIST_HEAD(, AllocatedMem) free_nodes;
QLIST_HEAD(, AllocatedMem) allocated_pool;

#define NODE_NUM 1000
#define MAX_SIZE 10
static AllocatedMem nodes[NODE_NUM];

static void *mmap_pages(int x) {
  void *p = mmap(0, x << PAGE_SHIFT, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANON, -1, 0);
  assert(p != NULL);
  return p;
}

static AllocatedMem *alloc_pages(int num) {
  AllocatedMem *node = QLIST_FIRST(&free_nodes);
  if (node == NULL)
    return NULL;

  char *p = (char *)mmap_pages(num);
  for (int i = 0; i < num; ++i) {
    p[PAGE_SIZE * i] = '0';
  }

  QLIST_INSERT_HEAD(&allocated_pool, node, mem_next);
  return node;
}

static void free_pages() {
  if (QLIST_EMPTY(&allocated_pool)) {
  }

  AllocatedMem *node;
  QLIST_FOREACH(node, &allocated_pool, mem_next) {}
}

static void init_pools() {
  QLIST_INIT(&free_nodes);
  QLIST_INIT(&allocated_pool);
  for (int i = 0; i < NODE_NUM; ++i) {
    QLIST_INSERT_HEAD(&free_nodes, &nodes[i], mem_next);
  }
}

// TMP_TODO it's unfinished
TEST test_mmap(void) {
  init_pools();

  for (int i = 0; i < NODE_NUM; ++i) {
    alloc_pages(rand() % MAX_SIZE);
    if (i % 7 == 1) {
      free_pages();
    }
  }
  PASS();
}

TEST test_float(void) {
  float x = 2.0;
  float y = 3.0;
  printf("%s %f\n", __FUNCTION__, x + y);
  PASS();
}

TEST test_interrupt(void) {
  printf("CSR ecfg: %08lx	", csr_readq(LOONGARCH_CSR_ECFG));
  ASSERT_EQ(csr_readq(LOONGARCH_CSR_ECFG), 0x70000);
  PASS();
}

SUITE(kvm_env) {
  // RUN_TEST(test_mmap);
  RUN_TEST(test_float);
  RUN_TEST(test_interrupt);
}
