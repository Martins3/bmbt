/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_BITS_H
#define __LINUX_BITS_H

#define DONT_CONFLIG_WITH_QEMU_BITS_PER_LONG 64

// TMP_TODO : 又是一个经典的冲突的位置
#define BIT(nr) (1UL << (nr))
#define BIT_ULL(nr) (1ULL << (nr))
// bmbt : conflicted with include/qemu/bitops.h
#ifdef BMBT
#define BIT_MASK(nr) (1UL << ((nr) % DONT_CONFLIG_WITH_QEMU_BITS_PER_LONG))
#define BIT_WORD(nr) ((nr) / DONT_CONFLIG_WITH_QEMU_BITS_PER_LONG)
#define BIT_ULL_MASK(nr) (1ULL << ((nr) % DONT_CONFLIG_WITH_QEMU_BITS_PER_LONG))
#define BIT_ULL_WORD(nr) ((nr) / DONT_CONFLIG_WITH_QEMU_BITS_PER_LONG)
#define BITS_PER_BYTE 8
#endif

/*
 * Create a contiguous bitmask starting at bit position @l and ending at
 * position @h. For example
 * GENMASK_ULL(39, 21) gives us the 64bit vector 0x000000ffffe00000.
 */
#define GENMASK(h, l)                                                          \
  (((~0UL) - (1UL << (l)) + 1) &                                               \
   (~0UL >> (DONT_CONFLIG_WITH_QEMU_BITS_PER_LONG - 1 - (h))))

#define GENMASK_ULL(h, l)                                                      \
  (((~0ULL) - (1ULL << (l)) + 1) &                                             \
   (~0ULL >> (DONT_CONFLIG_WITH_QEMU_BITS_PER_LONG - 1 - (h))))

#endif /* __LINUX_BITS_H */
