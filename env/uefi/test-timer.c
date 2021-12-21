#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <assert.h>
#include <env-timer.h>
#include <time.h>
#include <unistd.h>
#include <unitest/greatest.h>

void enter_interrpt_context();
void leave_interrpt_context();

static inline void wait() {
  while (1) {
    asm("hlt");
  }
}

static void do_something(int _unused) {
  printf("huxueshi:%s \n", __FUNCTION__);
}

TEST basic_test() {
  setup_timer(do_something);
  soonest_interrupt_ns(1000);
  sleep(1);
  PASS();
}

static int counter = 0;
static bool fire_in_handler = false;

static void handler_self_sent() {
  printf("huxueshi:%s \n", __FUNCTION__);
  counter++;
  if (fire_in_handler) {
    fire_timer();
    fire_in_handler = false;
  }
}

// fire timer in unblocked context, blocked context and signal handler
TEST test_self_sent_signal() {
  setup_timer(handler_self_sent);
  soonest_interrupt_ns(1000 * 1000 * 1000);
  fire_timer();
  ASSERT_EQ(counter, 1);
  soonest_interrupt_ns(1000 * 1000 * 500);
  block_interrupt();
  fire_timer();
  ASSERT_EQ(counter, 1);
  unblock_interrupt();
  ASSERT_EQ(counter, 2);

  fire_in_handler = true;
  fire_timer();
  ASSERT_EQ(counter, 4);

  PASS();
}

SUITE(timer) {
  // RUN_TEST(test_block_and_unblock);
  // RUN_TEST(test_block);
  // RUN_TEST(test_rearm_timer);
  RUN_TEST(test_self_sent_signal);
  RUN_TEST(basic_test);
}
SUITE(libc);

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN(); /* command-line options, initialization. */

  /* Individual tests can be run directly in main, outside of suites. */
  /* RUN_TEST(x_should_equal_1); */

  /* Tests can also be gathered into test suites. */
  RUN_SUITE(libc);
  // RUN_SUITE(timer);
  if (0)
    wait();

  GREATEST_MAIN_END(); /* display results */
}
