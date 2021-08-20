#ifndef MEMORY_H_E0UHP2JS
#define MEMORY_H_E0UHP2JS

#include "hwaddr.h"
#include "memattrs.h"
#include "memop.h"
#include "ram_addr.h"

// FIXME we will remove this file later, memory model will be redesinged
typedef struct MemoryRegion {

  RAMBlock *ram_block;

} MemoryRegion;

typedef struct AddressSpace {

} AddressSpace;

extern AddressSpace address_space_io;

typedef struct MemoryRegionSection {
  MemoryRegion *mr;

  bool readonly;
} MemoryRegionSection;

#define RAM_ADDR_INVALID (~(ram_addr_t)0)

MemoryRegion *address_space_translate(AddressSpace *as, hwaddr addr,
                                      hwaddr *xlat, hwaddr *len, bool is_write,
                                      MemTxAttrs attrs);

bool memory_access_is_direct(MemoryRegion *mr, bool is_write);

/**
 * memory_region_dispatch_read: perform a read directly to the specified
 * MemoryRegion.
 *
 * @mr: #MemoryRegion to access
 * @addr: address within that region
 * @pval: pointer to uint64_t which the data is written to
 * @op: size, sign, and endianness of the memory operation
 * @attrs: memory transaction attributes to use for the access
 */
MemTxResult memory_region_dispatch_read(MemoryRegion *mr, hwaddr addr,
                                        uint64_t *pval, MemOp op,
                                        MemTxAttrs attrs);

/**
 * memory_region_dispatch_write: perform a write directly to the specified
 * MemoryRegion.
 *
 * @mr: #MemoryRegion to access
 * @addr: address within that region
 * @data: data to write
 * @op: size, sign, and endianness of the memory operation
 * @attrs: memory transaction attributes to use for the access
 */
MemTxResult memory_region_dispatch_write(MemoryRegion *mr, hwaddr addr,
                                         uint64_t data, MemOp op,
                                         MemTxAttrs attrs);

/**
 * memory_region_get_dirty_log_mask: return the clients for which a
 * memory region is logging writes.
 *
 * Returns a bitmap of clients, in which the DIRTY_MEMORY_* constants
 * are the bit indices.
 *
 * @mr: the memory region being queried
 */
uint8_t memory_region_get_dirty_log_mask(MemoryRegion *mr);

/**
 * memory_region_get_ram_addr: Get the ram address associated with a memory
 *                             region
 */
ram_addr_t memory_region_get_ram_addr(MemoryRegion *mr);

#define NEED_CPU_H
#ifdef NEED_CPU_H
/* enum device_endian to MemOp.  */
static inline MemOp devend_memop(enum device_endian end) {
  QEMU_BUILD_BUG_ON(DEVICE_HOST_ENDIAN != DEVICE_LITTLE_ENDIAN &&
                    DEVICE_HOST_ENDIAN != DEVICE_BIG_ENDIAN);

#if defined(HOST_WORDS_BIGENDIAN) != defined(TARGET_WORDS_BIGENDIAN)
  /* Swap if non-host endianness or native (target) endianness */
  return (end == DEVICE_HOST_ENDIAN) ? 0 : MO_BSWAP;
#else
  const int non_host_endianness =
      DEVICE_LITTLE_ENDIAN ^ DEVICE_BIG_ENDIAN ^ DEVICE_HOST_ENDIAN;

  /* In this case, native (target) endianness needs no swap.  */
  return (end == non_host_endianness) ? MO_BSWAP : 0;
#endif
}
#endif
#undef NEED_CPU_H

// FIXME originally defined in exec.c
void *qemu_map_ram_ptr(RAMBlock *ram_block, ram_addr_t addr);
void invalidate_and_set_dirty(MemoryRegion *mr, hwaddr addr, hwaddr length);
bool prepare_mmio_access(MemoryRegion *mr);

/**
 * memory_region_is_ram: check whether a memory region is random access
 *
 * Returns %true if a memory region is random access.
 *
 * @mr: the memory region being queried
 */
static inline bool memory_region_is_ram(MemoryRegion *mr) {
  // FIXME
  return false;
}

/**
 * memory_region_is_romd: check whether a memory region is in ROMD mode
 *
 * Returns %true if a memory region is a ROM device and currently set to allow
 * direct reads.
 *
 * @mr: the memory region being queried
 */
static inline bool memory_region_is_romd(MemoryRegion *mr) {
  // rom device is related with pflash_cfi01_realize
  // we can delete this function, but for simplicity, just return false
  return false;
}

/**
 * memory_region_get_ram_ptr: Get a pointer into a RAM memory region.
 *
 * Returns a host pointer to a RAM memory region (created with
 * memory_region_init_ram() or memory_region_init_ram_ptr()).
 *
 * Use with care; by the time this function returns, the returned pointer is
 * not protected by RCU anymore.  If the caller is not within an RCU critical
 * section and does not hold the iothread lock, it must have other means of
 * protecting the pointer, such as a reference to the region that includes
 * the incoming ram_addr_t.
 *
 * @mr: the memory region being queried.
 */
static inline void *memory_region_get_ram_ptr(MemoryRegion *mr) {
  // FIXME
  return NULL;
}

#endif /* end of include guard: MEMORY_H_E0UHP2JS */
