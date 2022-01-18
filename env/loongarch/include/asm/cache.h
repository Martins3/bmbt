#ifndef CACHE_H_YTI1FWAN
#define CACHE_H_YTI1FWAN

#include <autoconf.h>

#define L1_CACHE_SHIFT CONFIG_L1_CACHE_SHIFT
#define L1_CACHE_BYTES (1 << L1_CACHE_SHIFT)
#define ARCH_DMA_MINALIGN L1_CACHE_BYTES

#endif /* end of include guard: CACHE_H_YTI1FWAN */
