#include <stdio.h>

// --------------------- 一般用于检查都是触发了那些类型的中断 ------------------
static void huxueshi(long irq) {
  printf("huxueshi:%s %lx\n", __FUNCTION__, irq);
}

static long database[100];
static int counter = 0;
static int is_full = 0;

static int insert_counter(long data) {
  if (is_full) {
    printf("%s full\n", __FUNCTION__);
    return 0;
  }
  for (int i = 0; i < counter; ++i) {
    if (database[i] == data) {
      return 0;
    }
  }
  huxueshi(data);
  database[counter++] = data;
  if (counter == 100) {
    is_full = 1;
  }
  return 1;
}

static void debug(const char *x) {
  while (*x != '\0') {
    outb(*x, 0x402);
    x++;
  }
}
