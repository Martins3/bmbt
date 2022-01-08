#include "stdio_impl.h"
#include <assert.h>
#include <stdbool.h>

static inline int do_putc(int c, FILE *f) {
  int l = f->lock;
  if (l < 0)
    return putc_unlocked(c, f);
  assert(false);
}
