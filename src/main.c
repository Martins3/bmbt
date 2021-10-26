#include "../../include/qemu/rcu.h"
#include "../../include/unitest/greatest.h"
#include <qemu/atomic.h>

/* A test runs various assertions, then calls PASS(), FAIL(), or SKIP(). */
TEST x_should_equal_1(void) {
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

TEST atomic(void) {
  int x;
  atomic_rcu_set(&x, 123);
  ASSERT_EQ(x, 123);

  PASS();
};

/* Suites can group multiple tests with common setup. */
SUITE(the_suite) {
  RUN_TEST(x_should_equal_1);
  RUN_TEST(atomic);
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
