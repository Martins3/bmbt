#ifndef HOST_UTILS_H_0IEAPEGH
#define HOST_UTILS_H_0IEAPEGH

#include "../types.h"
#include <limits.h>
#include <stdint.h>

/**
 * clz64 - count leading zeros in a 64-bit value.
 * @val: The value to search
 *
 * Returns 64 if the value is zero.  Note that the GCC builtin is
 * undefined if the value is zero.
 */
static inline int clz64(uint64_t val) {
  return val ? __builtin_clzll(val) : 64;
}

/* compute with 96 bit intermediate result: (a*b)/c */
static inline uint64_t muldiv64(uint64_t a, uint32_t b, uint32_t c) {
  return (__int128_t)a * b / c;
}

/*
 * Return @value rounded up to the nearest power of two modulo 2^64.
 * This is *zero* for @value > 2^63, so be careful.
 */
static inline uint64_t pow2ceil(uint64_t value) {
  int n = clz64(value - 1);

  if (!n) {
    /*
     * @value - 1 has no leading zeroes, thus @value - 1 >= 2^63
     * Therefore, either @value == 0 or @value > 2^63.
     * If it's 0, return 1, else return 0.
     */
    return !value;
  }
  return 0x8000000000000000ull >> (n - 1);
}

/**
 * Return @value rounded down to the nearest power of two or zero.
 */
static inline uint64_t pow2floor(uint64_t value) {
  if (!value) {
    /* Avoid undefined shift by 64 */
    return 0;
  }
  return 0x8000000000000000ull >> clz64(value);
}

/**
 * ctz32 - count trailing zeros in a 32-bit value.
 * @val: The value to search
 *
 * Returns 32 if the value is zero.  Note that the GCC builtin is
 * undefined if the value is zero.
 */
static inline int ctz32(uint32_t val) { return val ? __builtin_ctz(val) : 32; }

/**
 * clz32 - count leading zeros in a 32-bit value.
 * @val: The value to search
 *
 * Returns 32 if the value is zero.  Note that the GCC builtin is
 * undefined if the value is zero.
 */
static inline int clz32(uint32_t val) { return val ? __builtin_clz(val) : 32; }

/**
 * ctpop16 - count the population of one bits in a 16-bit value.
 * @val: The value to search
 */
static inline int ctpop16(uint16_t val) { return __builtin_popcount(val); }

/**
 * ctz64 - count trailing zeros in a 64-bit value.
 * @val: The value to search
 *
 * Returns 64 if the value is zero.  Note that the GCC builtin is
 * undefined if the value is zero.
 */
static inline int ctz64(uint64_t val) {
  return val ? __builtin_ctzll(val) : 64;
}

/* Host type specific sizes of these routines.  */

#if ULONG_MAX == UINT32_MAX
#define clzl clz32
#define ctzl ctz32
#define clol clo32
#define ctol cto32
#define ctpopl ctpop32
#define revbitl revbit32
#elif ULONG_MAX == UINT64_MAX
#define clzl clz64
#define ctzl ctz64
#define clol clo64
#define ctol cto64
#define ctpopl ctpop64
#define revbitl revbit64
#else
#error Unknown sizeof long
#endif

#endif /* end of include guard: HOST_UTILS_H_0IEAPEGH */
