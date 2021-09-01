#ifndef BITMAP_H_ENRPTXFD
#define BITMAP_H_ENRPTXFD

#include "../../src/tcg/glib_stub.h"
#include "bitops.h"

#include <stdlib.h>

static inline unsigned long *bitmap_try_new(long nbits) {
  long len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
  return g_try_malloc0(len);
  return 0;
}

static inline unsigned long *bitmap_new(long nbits) {
  unsigned long *ptr = bitmap_try_new(nbits);
  if (ptr == NULL) {
    abort();
  }
  return ptr;
}

static inline void bitmap_set(unsigned long *map, long i, long len) {
  // FIXME interface
}

#define DECLARE_BITMAP(name, bits) unsigned long name[BITS_TO_LONGS(bits)]

bool bitmap_test_and_clear_atomic(unsigned long *map, long start, long nr);

#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) & (BITS_PER_LONG - 1)))
#define BITMAP_LAST_WORD_MASK(nbits) (~0UL >> (-(nbits) & (BITS_PER_LONG - 1)))

void bitmap_set_atomic(unsigned long *map, long start, long nr);

#endif /* end of include guard: BITMAP_H_ENRPTXFD */
