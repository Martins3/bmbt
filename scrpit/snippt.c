#include <stdio.h>

// --------------------- 一般用于检查都是触发了那些类型的中断 ------------------
static void huxueshi(int irq) { printf("huxueshi:%s %d\n", __FUNCTION__, irq); }

static int irqs[100];
int counter = 0;

static void insert_counter(int irq) {
  for (int i = 0; i < counter; ++i) {
    if (irqs[i] == irq) {
      return;
    }
  }
  huxueshi(irq);
  irqs[counter++] = irq;
}
