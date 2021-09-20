#ifndef DMA_H_PMLNHKHO
#define DMA_H_PMLNHKHO
#include "../types.h"

/*
 * When an IOMMU is present, bus addresses become distinct from
 * CPU/memory physical addresses and may be a different size.  Because
 * the IOVA size depends more on the bus than on the platform, we more
 * or less have to treat these as 64-bit always to cover all (or at
 * least most) cases.
 */
typedef uint64_t dma_addr_t;

#endif /* end of include guard: DMA_H_PMLNHKHO */
