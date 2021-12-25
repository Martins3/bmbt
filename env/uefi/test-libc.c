#include <qemu/compiler.h>
#include <stdio.h>
#include <stdlib.h>
#include <unitest/greatest.h>

TEST test_mmap() {
  // alloc 1Gb
  char *x = malloc(1024LL * 1024 * 1024);
  if (x == NULL)
    printf("unbale to allocate such a big memory");
  ASSERT_NEQ(x, NULL);
  PASS();
}

TEST test_open() {
  char a[] = "hello";
  FILE *f = fopen("a.txt", "wb");
  ASSERT_NEQ(f, NULL);
  ASSERT_EQ(fwrite(a, sizeof(char), strlen(a), f), strlen(a));
  fclose(f);

  f = fopen("a.txt", "rb");
  fseek(f, 0L, SEEK_END);
  long sz = ftell(f);
  fseek(f, 0L, SEEK_SET);
  ASSERT_EQ(sz, strlen(a));
  char buf[1024];
  memset(buf, 0, sizeof(buf));
  sz = fread(buf, sizeof(char), strlen(a), f);
  printf("huxueshi:%s %d\n", __FUNCTION__, sz);
  printf("huxueshi:%s %s\n", __FUNCTION__, buf);
  ASSERT_EQ(sz, 5);
  ASSERT_EQ(strcmp(a, buf), 0);
  PASS();
}

SUITE(libc) {
  RUN_TEST(test_mmap);
  RUN_TEST(test_open);
}
