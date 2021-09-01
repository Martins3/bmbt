#include "../../include/qemu/bitops.h"
#include "../../include/qemu/host-utils.h"

/*
 * Find the next set bit in a memory region.
 */
unsigned long find_next_bit(const unsigned long *addr, unsigned long size,
                            unsigned long offset) {
  const unsigned long *p = addr + BIT_WORD(offset);
  unsigned long result = offset & ~(BITS_PER_LONG - 1);
  unsigned long tmp;

  if (offset >= size) {
    return size;
  }
  size -= result;
  offset %= BITS_PER_LONG;
  if (offset) {
    tmp = *(p++);
    tmp &= (~0UL << offset);
    if (size < BITS_PER_LONG) {
      goto found_first;
    }
    if (tmp) {
      goto found_middle;
    }
    size -= BITS_PER_LONG;
    result += BITS_PER_LONG;
  }
  while (size >= 4 * BITS_PER_LONG) {
    unsigned long d1, d2, d3;
    tmp = *p;
    d1 = *(p + 1);
    d2 = *(p + 2);
    d3 = *(p + 3);
    if (tmp) {
      goto found_middle;
    }
    if (d1 | d2 | d3) {
      break;
    }
    p += 4;
    result += 4 * BITS_PER_LONG;
    size -= 4 * BITS_PER_LONG;
  }
  while (size >= BITS_PER_LONG) {
    if ((tmp = *(p++))) {
      goto found_middle;
    }
    result += BITS_PER_LONG;
    size -= BITS_PER_LONG;
  }
  if (!size) {
    return result;
  }
  tmp = *p;

found_first:
  tmp &= (~0UL >> (BITS_PER_LONG - size));
  if (tmp == 0UL) {       /* Are any bits set? */
    return result + size; /* Nope. */
  }
found_middle:
  return result + ctzl(tmp);
}
