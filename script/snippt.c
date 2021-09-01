#include <stdio.h>

// --------------------- 一般用于检查都是触发了那些类型的中断 ------------------
static void huxueshi(long irq) {
  printf("huxueshi:%s %lx\n", __FUNCTION__, irq);
}

static long database[100];
static int counter = 0;

static int insert_counter(long data) {
  for (int i = 0; i < counter; ++i) {
    if (database[i] == data) {
      return 0;
    }
  }
  huxueshi(data);
  database[counter++] = data;
  return 1;
}
