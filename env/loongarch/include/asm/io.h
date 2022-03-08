#ifndef IO_H_5EZCJBHX
#define IO_H_5EZCJBHX

#include <asm/barrier.h>
#include <autoconf.h>
#include <linux/type.h>
#include <stdlib.h>

#define __iomem

#define __BUILD_MEMORY_SINGLE(pfx, bwlq, type)                                 \
                                                                               \
  static inline void pfx##write##bwlq(type val, volatile void __iomem *mem) {  \
    volatile type *__mem;                                                      \
                                                                               \
    wmb();                                                                     \
                                                                               \
    __mem = (void *)(unsigned long)(mem);                                      \
                                                                               \
    if (sizeof(type) != sizeof(u64) || sizeof(u64) == sizeof(long))            \
      *__mem = val;                                                            \
    else                                                                       \
      abort();                                                                 \
  }                                                                            \
                                                                               \
  static inline type pfx##read##bwlq(const volatile void __iomem *mem) {       \
    volatile type *__mem;                                                      \
    type __val;                                                                \
                                                                               \
    __mem = (void *)(unsigned long)(mem);                                      \
                                                                               \
    if (sizeof(type) != sizeof(u64) || sizeof(u64) == sizeof(long))            \
      __val = *__mem;                                                          \
    else {                                                                     \
      __val = 0;                                                               \
      abort();                                                                 \
    }                                                                          \
                                                                               \
    /* prevent prefetching of coherent DMA data prematurely */                 \
    rmb();                                                                     \
    return __val;                                                              \
  }

#define __BUILD_MEMORY_PFX(bus, bwlq, type)                                    \
                                                                               \
  __BUILD_MEMORY_SINGLE(bus, bwlq, type)

#define BUILDIO_MEM(bwlq, type) __BUILD_MEMORY_PFX(__raw_, bwlq, type)

BUILDIO_MEM(b, u8)
BUILDIO_MEM(w, u16)
BUILDIO_MEM(l, u32)
#ifdef CONFIG_64BIT
BUILDIO_MEM(q, u64)
#endif

#define __raw_readb __raw_readb
#define __raw_readw __raw_readw
#define __raw_readl __raw_readl
#define __raw_writeb __raw_writeb
#define __raw_writew __raw_writew
#define __raw_writel __raw_writel
#ifdef CONFIG_64BIT
#define __raw_readq __raw_readq
#define __raw_writeq __raw_writeq
#endif

#define readb __raw_readb
#define readw __raw_readw
#define readl __raw_readl
#define writeb __raw_writeb
#define writew __raw_writew
#define writel __raw_writel
#ifdef CONFIG_64BIT
#define readq __raw_readq
#define writeq __raw_writeq
#endif
#endif /* end of include guard: IO_H_5EZCJBHX */
