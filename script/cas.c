#include <assert.h>
#include <stdio.h>
#include <unistd.h>

int a_cas(volatile int *p, int t, int s) {
  __asm__ __volatile__("lock ; cmpxchg %3, %1"
                       : "=a"(t), "=m"(*p)
                       : "a"(t), "r"(s)
                       : "memory");
  return t;
}

int bmbt_cas(volatile int *p, int t, int s) {
  if (*p == t) {
    *p = s;
  } else {
    t = *p;
  }
  return t;
}

void same(int p, int t, int s) {
  int p1 = p;
  int p2 = p;
  printf("huxueshi:%s %d %d %d\n", __FUNCTION__, p, t, s);
  // printf("huxueshi:%s %d %d\n", __FUNCTION__, a_cas(&p1, t, s), how(&p2, t,
  // s));
  assert(a_cas(&p1, t, s) == bmbt_cas(&p2, t, s));
}

int main(int argc, char *argv[]) {
  int count = 10;
  for (int i = 0; i < count; ++i) {
    for (int j = 0; j < count; ++j) {
      for (int k = 0; k < count; ++k) {
        int x = i;
        int y = j;
        int z = k;
        same(x, y, z);
      }
    }
  }
  return 0;
}
