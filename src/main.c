#include <exec/memory.h>
#include <exec/ram_addr.h>
#include <hw/core/cpu.h>
#include <qemu/atomic.h>
#include <qemu/main-loop.h>
#include <qemu/rcu.h>
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
  PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE(the_suite) {
  RUN_TEST(testx_should_equal_1);
  RUN_TEST(test_atomic);
  RUN_TEST(test_cpu_list);
  RUN_TEST(test_ram_block);
  RUN_TEST(test_qemu_option);
}

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN(); /* command-line options, initialization. */

  /* Individual tests can be run directly in main, outside of suites. */
  /* RUN_TEST(x_should_equal_1); */

  /* Tests can also be gathered into test suites. */
  RUN_SUITE(the_suite);

  GREATEST_MAIN_END(); /* display results */
}
