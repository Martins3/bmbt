#include <host-timer.h>
#include <unitest/greatest.h>

static inline void wait() {
  while (1) {
    asm("hlt");
  }
}

static void do_something(int _unused) {
  printf("huxueshi:%s \n", __FUNCTION__);
}

TEST test_self_sent_signal() {
  setup_timer(do_something);
  soonest_interrupt_ns(1000);
  PASS();
}

SUITE(timer) {
  // RUN_TEST(test_block_and_unblock);
  // RUN_TEST(test_block);
  // RUN_TEST(test_rearm_timer);
  RUN_TEST(test_self_sent_signal);
}

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN(); /* command-line options, initialization. */

  /* Individual tests can be run directly in main, outside of suites. */
  /* RUN_TEST(x_should_equal_1); */

  /* Tests can also be gathered into test suites. */
  RUN_SUITE(timer);

  wait();
  GREATEST_MAIN_END(); /* display results */
}
