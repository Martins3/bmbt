#include <assert.h>
#include <limits.h>
#include <qemu/queue.h>
#include <sys/mman.h>
#include <unitest/greatest.h>

void *alloc_pages(int x) {
  void *p =
      mmap(0, x << 14, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
  assert(p != NULL);
  return p;
}

void free_pages(void *addr, size_t x) { munmap(addr, x); }

typedef struct AllocatedMem {
  QLIST_ENTRY(AllocatedMem) mem_next;
  void *p;
} AllocatedMem;

QLIST_HEAD(, AllocatedMem) pool;
// AllocatedMem nodes[1000];
TEST test_mmap(void) {
  QLIST_INIT(&pool);

  printf("huxueshi:%s before \n", __FUNCTION__);
  char *p = (char *)alloc_pages(2);
  for (int i = 0; i < 2; ++i) {
    p[PAGE_SIZE * i] = '0';
  }
  printf("huxueshi:%s after \n", __FUNCTION__);

  // for (int i = 0; i < 1000; ++i) {
  // int x = rand();
  // void *p = alloc_pages(x % 1000);
  // QLIST_INSERT_HEAD(&pool, elm, field)
  // if (x % 7 == 1) {
  // }
  // }
  PASS();
}

TEST test_float(void) {
  printf("huxueshi:%s \n", __FUNCTION__);
  float x = 2.0;
  float y = 3.0;
  printf("huxueshi:%s %f\n", __FUNCTION__, x + y);
  PASS();
}

SUITE(kvm_env) {
  RUN_TEST(test_mmap);
  RUN_TEST(test_float);
}
