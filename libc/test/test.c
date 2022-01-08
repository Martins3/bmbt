#include <assert.h>
#include <errno.h>
#include <math.h>
#include <setjmp.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <unitest/greatest.h>

TEST basic() {
  int sz = 10;
  char *x = malloc(sz);
  memset(x, 'a', sz);
  for (int i = 0; i < sz; ++i) {
    assert(x[i] == 'a');
  }
  x[sz - 1] = '\0';
  printf("huxueshi:%s %s\n", __FUNCTION__, x);
  putchar('a');
  putchar('\n');

  struct timespec tv = {.tv_sec = 0, .tv_nsec = 1000};
  nanosleep(&tv, &tv);

  for (int i = 0; i <= 3; ++i) {
    ASSERT_NEQ(printf("%s\n", strerror(i)), 0);
  }
  puts("hi loongson");

  PASS();
}

TEST memory_alloc() {
  void *ptr = NULL;
  for (int align = 3; align <= 10; ++align) {
    int size = 12;
    ASSERT_EQ(posix_memalign(&ptr, 1 << align, size), 0);
    ASSERT_EQ(((long)ptr & ((1 << align) - 1)), 0);
  }
  PASS();
}

TEST math() {
  double x = 4.0;
  ASSERT_EQ(2, sqrt(x));
  PASS();
}

TEST test_setjmp() {
  volatile int x = 0, r;
  jmp_buf jb;

  if (!setjmp(jb)) {
    x = 1;
    longjmp(jb, 1);
  }
  ASSERT_EQ(x, 1);

  x = 0;
  r = setjmp(jb);
  if (!x) {
    x = 1;
    longjmp(jb, 0);
  }
  ASSERT_EQ(r, 1);
  PASS();
}

SUITE(wip) {
  RUN_TEST(basic);
  RUN_TEST(math);
  RUN_TEST(test_setjmp);
  RUN_TEST(memory_alloc);
}

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char *argv[]) {
  GREATEST_MAIN_BEGIN(); /* command-line options, initialization. */
  RUN_SUITE(wip);
  GREATEST_MAIN_END(); /* display results */
  return 0;
}

void _start() {
  char *argv[] = {NULL};
  main(0, argv);
  exit(0);
}
