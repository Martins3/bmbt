#include <exec/memory.h>
#include <exec/ram_addr.h>
#include <exec/tb-context.h>
#include <hw/core/cpu.h>
#include <qemu/atomic.h>
#include <qemu/main-loop.h>
#include <qemu/qht.h>
#include <qemu/rcu.h>
#include <qemu/xxhash.h>
#include <sysemu/cpus.h>
#include <test/test.h>
#include <unitest/greatest.h>

/* A test runs various assertions, then calls PASS(), FAIL(), or SKIP(). */
TEST testx_should_equal_1(void) {
  int x = 1;
  /* Compare, with an automatic "1 != x" failure message */
  ASSERT_EQ(1, x);

  /* Compare, with a custom failure message */
  ASSERT_EQm("Yikes, x doesn't equal 1", 1, x);

  /* Compare, and if they differ, print both values,
   * formatted like `printf("Expected: %d\nGot: %d\n", 1, x);` */
  ASSERT_EQ_FMT(1, x, "%d");
  PASS();
}

TEST test_atomic(void) {
  int x;
  atomic_rcu_set(&x, 123);
  ASSERT_EQ(x, 123);

  PASS();
};

TEST test_cpu_list(void) {
  qemu_init_cpu_list();
  CPUState cpu0;
  cpu_list_add(&cpu0);
  ASSERT_EQ(first_cpu, &cpu0);

  CPUState cpu1;
  cpu_list_add(&cpu1);
  ASSERT_EQ(first_cpu, &cpu0);

  ASSERT_EQ(CPU_NEXT(&cpu0), &cpu1);

  CPUState *cpu;
  cpu0.cluster_index = 0x123;
  cpu1.cluster_index = 0x456;
  int x = 0;
  CPU_FOREACH(cpu) { x += cpu->cluster_index; }
  ASSERT_EQ(x, cpu0.cluster_index + cpu1.cluster_index);

  cpu_list_remove(&cpu1);
  CPU_FOREACH(cpu) { x -= cpu->cluster_index; }
  ASSERT_EQ(x, cpu1.cluster_index);

  cpu_list_remove(&cpu1);
  ASSERT_EQ(first_cpu, &cpu0);
  ASSERT_EQ(CPU_NEXT(&cpu0), NULL);
  ASSERT_EQ(CPU_NEXT(&cpu1), NULL);

  cpu_list_remove(&cpu0);
  ASSERT_EQ(first_cpu, NULL);

  PASS();
};

TEST test_ram_block(void) {
  RAMBlock *block;
  int i = RAM_BLOCK_NUM;
  int total_length = 0;
  ram_list.blocks[0].block.max_length = 12;
  ram_list.blocks[1].block.max_length = 12;

  RAMBLOCK_FOREACH(block) {
    i--;
    total_length += block->max_length;
  }
  ASSERT_EQ(i, 0);
  ASSERT_EQ(total_length, 24);
  PASS();
}

TEST test_qemu_option(void) {
  void call_constructor();
  call_constructor();
  init_xtm_options();

  extern int xtm_sigint_opt(void);
  ASSERT_EQ(xtm_sigint_opt(), 0);

  PASS();
}

static bool str_cmp(const void *ap, const void *bp) {
  const char *a = ap;
  const char *b = bp;
  return strcmp(a, b) == 0;
}

static bool custom_str_cmp(const void *ap, const void *bp) {
  const char *a = ap;
  const char *b = bp;
  return strcmp(a, "ok") == 0 || strcmp(b, "ok") == 0;
}

static unsigned long hash_str(const char *str) {
  unsigned long hash = 5381;
  int c;

  while ((0 != (c = *str++)))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

TEST test_qht(void) {
  struct qht htable;
  qht_init(&htable, str_cmp, CODE_GEN_HTABLE_SIZE, QHT_MODE_AUTO_RESIZE);
  char *str1 = "abc1";
  char *str2 = "abc2";
  unsigned long hash1 = hash_str(str1);
  unsigned long hash2 = hash_str(str2);

  void *res = qht_lookup(&htable, str1, hash1);
  ASSERT_EQ(res, NULL);
  qht_insert(&htable, str1, hash1, &res);
  res = qht_lookup(&htable, str1, hash1);
  ASSERT_EQ(res, str1);

  qht_insert(&htable, str2, hash2, &res);
  res = qht_lookup(&htable, str2, hash2);
  ASSERT_EQ(res, str2);

  res = qht_lookup(&htable, str2, hash1);
  ASSERT_EQ(res, NULL);

  res = qht_lookup(&htable, "fadfa", hash2);
  ASSERT_EQ(res, NULL);

  res = qht_lookup_custom(&htable, "ok", hash2, custom_str_cmp);

  for (int i = 1; i < 100; i *= 10) {
    ASSERT_EQ(qht_reset_size(&htable, i * 4), true);
  }

  qht_test_default();
  qht_test_resize();

  PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE(basic_test) {
  RUN_TEST(testx_should_equal_1);
  RUN_TEST(test_atomic);
  RUN_TEST(test_cpu_list);
  RUN_TEST(test_ram_block);
  RUN_TEST(test_qemu_option);
  RUN_TEST(test_qht);
}

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN(); /* command-line options, initialization. */

  /* Individual tests can be run directly in main, outside of suites. */
  /* RUN_TEST(x_should_equal_1); */

  /* Tests can also be gathered into test suites. */
  RUN_SUITE(basic_test);
  RUN_SUITE(memory_model_test);

  GREATEST_MAIN_END(); /* display results */
}
