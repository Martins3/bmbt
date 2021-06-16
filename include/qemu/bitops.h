#ifndef BITOPS_H_Z29X3LYD
#define BITOPS_H_Z29X3LYD
#include <limits.h>

#define MAKE_64BIT_MASK(shift, length) \
    (((~0ULL) >> (64 - (length))) << (shift))


#define BITS_PER_BYTE           CHAR_BIT
#define BITS_PER_LONG           (sizeof (unsigned long) * BITS_PER_BYTE)
#define BIT_WORD(nr)            ((nr) / BITS_PER_LONG)

/**
 * test_bit - Determine whether a bit is set
 * @nr: bit number to test
 * @addr: Address to start counting from
 */
static inline int test_bit(long nr, const unsigned long *addr)
{
    return 1UL & (addr[BIT_WORD(nr)] >> (nr & (BITS_PER_LONG-1)));
}

#endif /* end of include guard: BITOPS_H_Z29X3LYD */
