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

void bitmap_set_atomic(unsigned long *map, long start, long nr)
{
    unsigned long *p = map + BIT_WORD(start);
    const long size = start + nr;
    int bits_to_set = BITS_PER_LONG - (start % BITS_PER_LONG);
    unsigned long mask_to_set = BITMAP_FIRST_WORD_MASK(start);

    assert(start >= 0 && nr >= 0);

    /* First word */
    if (nr - bits_to_set > 0) {
        atomic_or(p, mask_to_set);
        nr -= bits_to_set;
        bits_to_set = BITS_PER_LONG;
        mask_to_set = ~0UL;
        p++;
    }

    /* Full words */
    if (bits_to_set == BITS_PER_LONG) {
        while (nr >= BITS_PER_LONG) {
            *p = ~0UL;
            nr -= BITS_PER_LONG;
            p++;
        }
    }

    /* Last word */
    if (nr) {
        mask_to_set &= BITMAP_LAST_WORD_MASK(size);
        atomic_or(p, mask_to_set);
    } else {
        /* If we avoided the full barrier in atomic_or(), issue a
         * barrier to account for the assignments in the while loop.
         */
        smp_mb();
    }
}
