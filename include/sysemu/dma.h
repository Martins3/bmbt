#ifndef DMA_H_PMLNHKHO
#define DMA_H_PMLNHKHO
#include "../exec/memory.h"
#include "../qemu/atomic.h"
#include "../qemu/bswap.h"

typedef enum {
  DMA_DIRECTION_TO_DEVICE = 0,
  DMA_DIRECTION_FROM_DEVICE = 1,
} DMADirection;

/*
 * When an IOMMU is present, bus addresses become distinct from
 * CPU/memory physical addresses and may be a different size.  Because
 * the IOVA size depends more on the bus than on the platform, we more
 * or less have to treat these as 64-bit always to cover all (or at
 * least most) cases.
 */
typedef uint64_t dma_addr_t;

#define DMA_ADDR_BITS 64
#define DMA_ADDR_FMT "%" PRIx64

static inline void dma_barrier(AddressSpace *as, DMADirection dir) {
  /*
   * This is called before DMA read and write operations
   * unless the _relaxed form is used and is responsible
   * for providing some sane ordering of accesses vs
   * concurrently running VCPUs.
   *
   * Users of map(), unmap() or lower level st/ld_*
   * operations are responsible for providing their own
   * ordering via barriers.
   *
   * This primitive implementation does a simple smp_mb()
   * before each operation which provides pretty much full
   * ordering.
   *
   * A smarter implementation can be devised if needed to
   * use lighter barriers based on the direction of the
   * transfer, the DMA context, etc...
   */
  smp_mb();
}

#ifdef BMBT
/* Checks that the given range of addresses is valid for DMA.  This is
 * useful for certain cases, but usually you should just use
 * dma_memory_{read,write}() and check for errors */
static inline bool dma_memory_valid(AddressSpace *as, dma_addr_t addr,
                                    dma_addr_t len, DMADirection dir) {
  return address_space_access_valid(
      as, addr, len, dir == DMA_DIRECTION_FROM_DEVICE, MEMTXATTRS_UNSPECIFIED);
}
#endif

static inline int dma_memory_rw_relaxed(AddressSpace *as, dma_addr_t addr,
                                        void *buf, dma_addr_t len,
                                        DMADirection dir) {
  return (bool)address_space_rw(as, addr, MEMTXATTRS_UNSPECIFIED, buf, len,
                                dir == DMA_DIRECTION_FROM_DEVICE);
}

static inline int dma_memory_read_relaxed(AddressSpace *as, dma_addr_t addr,
                                          void *buf, dma_addr_t len) {
  return dma_memory_rw_relaxed(as, addr, buf, len, DMA_DIRECTION_TO_DEVICE);
}

static inline int dma_memory_write_relaxed(AddressSpace *as, dma_addr_t addr,
                                           const void *buf, dma_addr_t len) {
  return dma_memory_rw_relaxed(as, addr, (void *)buf, len,
                               DMA_DIRECTION_FROM_DEVICE);
}

static inline int dma_memory_rw(AddressSpace *as, dma_addr_t addr, void *buf,
                                dma_addr_t len, DMADirection dir) {
  dma_barrier(as, dir);

  return dma_memory_rw_relaxed(as, addr, buf, len, dir);
}

static inline int dma_memory_read(AddressSpace *as, dma_addr_t addr, void *buf,
                                  dma_addr_t len) {
  return dma_memory_rw(as, addr, buf, len, DMA_DIRECTION_TO_DEVICE);
}

static inline int dma_memory_write(AddressSpace *as, dma_addr_t addr,
                                   const void *buf, dma_addr_t len) {
  return dma_memory_rw(as, addr, (void *)buf, len, DMA_DIRECTION_FROM_DEVICE);
}

int dma_memory_set(AddressSpace *as, dma_addr_t addr, uint8_t c,
                   dma_addr_t len);

#define DEFINE_LDST_DMA(_lname, _sname, _bits, _end)                           \
  static inline uint##_bits##_t ld##_lname##_##_end##_dma(AddressSpace *as,    \
                                                          dma_addr_t addr) {   \
    uint##_bits##_t val;                                                       \
    dma_memory_read(as, addr, &val, (_bits) / 8);                              \
    return _end##_bits##_to_cpu(val);                                          \
  }                                                                            \
  static inline void st##_sname##_##_end##_dma(                                \
      AddressSpace *as, dma_addr_t addr, uint##_bits##_t val) {                \
    val = cpu_to_##_end##_bits(val);                                           \
    dma_memory_write(as, addr, &val, (_bits) / 8);                             \
  }

static inline uint8_t ldub_dma(AddressSpace *as, dma_addr_t addr) {
  uint8_t val;

  dma_memory_read(as, addr, &val, 1);
  return val;
}

static inline void stb_dma(AddressSpace *as, dma_addr_t addr, uint8_t val) {
  dma_memory_write(as, addr, &val, 1);
}

DEFINE_LDST_DMA(uw, w, 16, le);
DEFINE_LDST_DMA(l, l, 32, le);
DEFINE_LDST_DMA(q, q, 64, le);
DEFINE_LDST_DMA(uw, w, 16, be);
DEFINE_LDST_DMA(l, l, 32, be);
DEFINE_LDST_DMA(q, q, 64, be);

#undef DEFINE_LDST_DMA
#endif /* end of include guard: DMA_H_PMLNHKHO */
