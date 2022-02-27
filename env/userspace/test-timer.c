#include <assert.h>
#include <errno.h>
#include <qemu/atomic.h>
#include <qemu/timer.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

#include <time.h>
#include <unistd.h>
#include <unitest/greatest.h>

static int counter;
static int loop_counter;

static void do_something() {
  struct timespec request, remain;
  request.tv_sec = 0;
  request.tv_nsec = rand() % (1000 * 1000);
  for (;;) {
    int s = nanosleep(&request, &remain);
    // if waked up by timer signal, continue the sleeping
    if (s == -1 && errno == EINTR) {
      printf("waked up by timer\n");
    }
    if (s == -1 && errno != EINTR) {
      printf("test finished\n");
    }

    if (s == 0) {
      return;
    }

    request = remain; /* Next sleep is with remaining time */
  }
}

static void handler() {
  printf("execute %s \n", __FUNCTION__);
  loop_counter++;
  if (loop_counter % 1000 == 0) {
    printf("unblocked %d\n", loop_counter);
  }
  assert(qatomic_read(&counter) == 0);

  soonest_interrupt_ns(1000 * 1000 * 100);
}

static void device_simulation() {
  block_interrupt();
  counter++;
  do_something();
  counter--;
  unblock_interrupt();
  do_something();
}

static void loop() {
  for (int i = 0; i < 1000; ++i) {
    device_simulation();
  }
}

TEST test_block_and_unblock() {
  setup_timer(handler);
  soonest_interrupt_ns(1000 * 1000 * 100);
  loop();
  PASS();
}

static bool say_hi;
static void handler_say_hi() {
  printf("hi\n");
  say_hi = true;
}

TEST test_block() {
  say_hi = false;
  setup_timer(handler_say_hi);
  block_interrupt();
  soonest_interrupt_ns(1000 * 1000 * 100);
  // wait for timer to expire
  sleep(1);
  // handler not executed
  assert(say_hi == false);
  unblock_interrupt();
  // handler already executed
  assert(say_hi == true);
  PASS();
}

TEST test_rearm_timer() {
  say_hi = false;
  setup_timer(handler_say_hi);
  soonest_interrupt_ns(100 * NANOSECONDS_PER_SECOND);
  // verify we can reprogram the timer
  soonest_interrupt_ns(1000);
  sleep(1);
  assert(say_hi == true);
  say_hi = false;
  soonest_interrupt_ns(100 * NANOSECONDS_PER_SECOND);
  sleep(1);
  assert(say_hi == false);

  PASS();
}

static int counter = 0;
static bool fire_in_handler = false;
static bool in_handler = false;

static void handler_self_sent(int sig, siginfo_t *si, void *uc) {
  printf("huxueshi:%s \n", __FUNCTION__);
  if (in_handler) {
    assert(false);
  }
  in_handler = true;

  counter++;
  if (fire_in_handler) {
    fire_timer();
    fire_in_handler = false;
  }

  in_handler = false;
}

// fire timer in unblocked context, blocked context and signal handler
TEST test_self_sent_signal() {
  setup_timer(handler_self_sent);
  soonest_interrupt_ns(1000 * 1000 * 500);
  fire_timer();
  sleep(1);
  ASSERT_EQ(counter, 2);
  block_interrupt();
  fire_timer();
  ASSERT_EQ(counter, 2);
  unblock_interrupt();
  ASSERT_EQ(counter, 3);

  fire_in_handler = true;
  fire_timer();
  ASSERT_EQ(counter, 5);

  PASS();
}

SUITE(timer) {
  RUN_TEST(test_block_and_unblock);
  RUN_TEST(test_block);
  RUN_TEST(test_rearm_timer);
  RUN_TEST(test_self_sent_signal);
}
