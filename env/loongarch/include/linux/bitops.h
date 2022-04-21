#ifndef BITOPS_H_WOXPLBNG
#define BITOPS_H_WOXPLBNG

#ifndef __always_inline
#define __always_inline inline __attribute__((always_inline))
#endif

/**
 * __ffs - find first bit in word.
 * @word: The word to search
 *
 * Undefined if no bit exists, so code should check against 0 first.
 */
static __always_inline unsigned long __ffs(unsigned long word) {
  return __builtin_ctzl(word);
}

#endif /* end of include guard: BITOPS_H_WOXPLBNG */
