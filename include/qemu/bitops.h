#ifndef BITOPS_H_Z29X3LYD
#define BITOPS_H_Z29X3LYD
#include <assert.h>
#include <limits.h>
#include <qemu/atomic.h>
#include <qemu/osdep.h>

#define BIT(nr) (1UL << (nr))

#define MAKE_64BIT_MASK(shift, length) (((~0ULL) >> (64 - (length))) << (shift))

#define BITS_PER_BYTE CHAR_BIT
#ifdef ENV_KERNEL
#include <autoconf.h>
#define BITS_PER_LONG _LOONGARCH_SZLONG
#else
#define BITS_PER_LONG (sizeof(unsigned long) * BITS_PER_BYTE)
#endif
#define BIT_WORD(nr) ((nr) / BITS_PER_LONG)
#define BIT_WORD(nr) ((nr) / BITS_PER_LONG)
#define BITS_TO_LONGS(nr) DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

#define BIT_MASK(nr) (1UL << ((nr) % BITS_PER_LONG))

/**
 * test_bit - Determine whether a bit is set
 * @nr: bit number to test
 * @addr: Address to start counting from
 */
static inline int test_bit(long nr, const unsigned long *addr) {
  return 1UL & (addr[BIT_WORD(nr)] >> (nr & (BITS_PER_LONG - 1)));
}

/**
 * clear_bit - Clears a bit in memory
 * @nr: Bit to clear
 * @addr: Address to start counting from
 */
static inline void clear_bit(long nr, unsigned long *addr) {
  unsigned long mask = BIT_MASK(nr);
  unsigned long *p = addr + BIT_WORD(nr);

  *p &= ~mask;
}

/**
 * set_bit - Set a bit in memory
 * @nr: the bit to set
 * @addr: the address to start counting from
 */
static inline void set_bit(long nr, unsigned long *addr) {
  unsigned long mask = BIT_MASK(nr);
  unsigned long *p = addr + BIT_WORD(nr);

  *p |= mask;
}

/**
 * deposit32:
 * @value: initial value to insert bit field into
 * @start: the lowest bit in the bit field (numbered from 0)
 * @length: the length of the bit field
 * @fieldval: the value to insert into the bit field
 *
 * Deposit @fieldval into the 32 bit @value at the bit field specified
 * by the @start and @length parameters, and return the modified
 * @value. Bits of @value outside the bit field are not modified.
 * Bits of @fieldval above the least significant @length bits are
 * ignored. The bit field must lie entirely within the 32 bit word.
 * It is valid to request that all 32 bits are modified (ie @length
 * 32 and @start 0).
 *
 * Returns: the modified @value.
 */
static inline uint32_t deposit32(uint32_t value, int start, int length,
                                 uint32_t fieldval) {
  uint32_t mask;
  assert(start >= 0 && length > 0 && length <= 32 - start);
  mask = (~0U >> (32 - length)) << start;
  return (value & ~mask) | ((fieldval << start) & mask);
}

/**
 * deposit64:
 * @value: initial value to insert bit field into
 * @start: the lowest bit in the bit field (numbered from 0)
 * @length: the length of the bit field
 * @fieldval: the value to insert into the bit field
 *
 * Deposit @fieldval into the 64 bit @value at the bit field specified
 * by the @start and @length parameters, and return the modified
 * @value. Bits of @value outside the bit field are not modified.
 * Bits of @fieldval above the least significant @length bits are
 * ignored. The bit field must lie entirely within the 64 bit word.
 * It is valid to request that all 64 bits are modified (ie @length
 * 64 and @start 0).
 *
 * Returns: the modified @value.
 */
static inline uint64_t deposit64(uint64_t value, int start, int length,
                                 uint64_t fieldval) {
  uint64_t mask;
  assert(start >= 0 && length > 0 && length <= 64 - start);
  mask = (~0ULL >> (64 - length)) << start;
  return (value & ~mask) | ((fieldval << start) & mask);
}

/**
 * extract64:
 * @value: the value to extract the bit field from
 * @start: the lowest bit in the bit field (numbered from 0)
 * @length: the length of the bit field
 *
 * Extract from the 64 bit input @value the bit field specified by the
 * @start and @length parameters, and return it. The bit field must
 * lie entirely within the 64 bit word. It is valid to request that
 * all 64 bits are returned (ie @length 64 and @start 0).
 *
 * Returns: the value of the bit field extracted from the input value.
 */
static inline uint64_t extract64(uint64_t value, int start, int length) {
  assert(start >= 0 && length > 0 && length <= 64 - start);
  return (value >> start) & (~0ULL >> (64 - length));
}

/**
 * rol32 - rotate a 32-bit value left
 * @word: value to rotate
 * @shift: bits to roll
 */
static inline uint32_t rol32(uint32_t word, unsigned int shift) {
  return (word << shift) | (word >> ((32 - shift) & 31));
}

/**
 * set_bit_atomic - Set a bit in memory atomically
 * @nr: the bit to set
 * @addr: the address to start counting from
 */
static inline void set_bit_atomic(long nr, unsigned long *addr) {
  unsigned long mask = BIT_MASK(nr);
  unsigned long *p = addr + BIT_WORD(nr);

  qatomic_or(p, mask);
}

/**
 * find_last_bit - find the last set bit in a memory region
 * @addr: The address to start the search at
 * @size: The maximum size to search
 *
 * Returns the bit number of the first set bit, or size.
 */
unsigned long find_last_bit(const unsigned long *addr, unsigned long size);

/**
 * find_next_bit - find the next set bit in a memory region
 * @addr: The address to base the search on
 * @offset: The bitnumber to start searching at
 * @size: The bitmap size in bits
 */
unsigned long find_next_bit(const unsigned long *addr, unsigned long size,
                            unsigned long offset);

/**
 * find_next_zero_bit - find the next cleared bit in a memory region
 * @addr: The address to base the search on
 * @offset: The bitnumber to start searching at
 * @size: The bitmap size in bits
 */

unsigned long find_next_zero_bit(const unsigned long *addr, unsigned long size,
                                 unsigned long offset);

/**
 * sextract32:
 * @value: the value to extract the bit field from
 * @start: the lowest bit in the bit field (numbered from 0)
 * @length: the length of the bit field
 *
 * Extract from the 32 bit input @value the bit field specified by the
 * @start and @length parameters, and return it, sign extended to
 * an int32_t (ie with the most significant bit of the field propagated
 * to all the upper bits of the return value). The bit field must lie
 * entirely within the 32 bit word. It is valid to request that
 * all 32 bits are returned (ie @length 32 and @start 0).
 *
 * Returns: the sign extended value of the bit field extracted from the
 * input value.
 */
static inline int32_t sextract32(uint32_t value, int start, int length) {
  assert(start >= 0 && length > 0 && length <= 32 - start);
  /* Note that this implementation relies on right shift of signed
   * integers being an arithmetic shift.
   */
  return ((int32_t)(value << (32 - length - start))) >> (32 - length);
}

/**
 * sextract64:
 * @value: the value to extract the bit field from
 * @start: the lowest bit in the bit field (numbered from 0)
 * @length: the length of the bit field
 *
 * Extract from the 64 bit input @value the bit field specified by the
 * @start and @length parameters, and return it, sign extended to
 * an int64_t (ie with the most significant bit of the field propagated
 * to all the upper bits of the return value). The bit field must lie
 * entirely within the 64 bit word. It is valid to request that
 * all 64 bits are returned (ie @length 64 and @start 0).
 *
 * Returns: the sign extended value of the bit field extracted from the
 * input value.
 */
static inline int64_t sextract64(uint64_t value, int start, int length) {
  assert(start >= 0 && length > 0 && length <= 64 - start);
  /* Note that this implementation relies on right shift of signed
   * integers being an arithmetic shift.
   */
  return ((int64_t)(value << (64 - length - start))) >> (64 - length);
}

#endif /* end of include guard: BITOPS_H_Z29X3LYD */
