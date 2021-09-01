#include "../../include/qemu/bitmap.h"
#include "../../include/qemu/atomic.h"

bool bitmap_test_and_clear_atomic(unsigned long *map, long start, long nr) {
  unsigned long *p = map + BIT_WORD(start);
  const long size = start + nr;
  int bits_to_clear = BITS_PER_LONG - (start % BITS_PER_LONG);
  unsigned long mask_to_clear = BITMAP_FIRST_WORD_MASK(start);
  unsigned long dirty = 0;
  unsigned long old_bits;

  assert(start >= 0 && nr >= 0);

  /* First word */
  if (nr - bits_to_clear > 0) {
    old_bits = atomic_fetch_and(p, ~mask_to_clear);
    dirty |= old_bits & mask_to_clear;
    nr -= bits_to_clear;
    bits_to_clear = BITS_PER_LONG;
    mask_to_clear = ~0UL;
    p++;
  }

  /* Full words */
  if (bits_to_clear == BITS_PER_LONG) {
    while (nr >= BITS_PER_LONG) {
      if (*p) {
        old_bits = atomic_xchg(p, 0);
        dirty |= old_bits;
      }
      nr -= BITS_PER_LONG;
      p++;
    }
  }

  /* Last word */
  if (nr) {
    mask_to_clear &= BITMAP_LAST_WORD_MASK(size);
    old_bits = atomic_fetch_and(p, ~mask_to_clear);
    dirty |= old_bits & mask_to_clear;
  } else {
    if (!dirty) {
      smp_mb();
    }
  }

  return dirty != 0;
}
