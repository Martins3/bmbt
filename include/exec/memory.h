#ifndef MEMORY_H_E0UHP2JS
#define MEMORY_H_E0UHP2JS

#include "cpu-common.h"
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

// FIXME define a empty one
extern AddressSpace address_space_io;

typedef struct MemoryRegionSection {

} MemoryRegionSection;

#define RAM_ADDR_INVALID (~(ram_addr_t)0)

static MemoryRegion *address_space_translate(AddressSpace *as, hwaddr addr,
                                             hwaddr *xlat, hwaddr *len,
                                             bool is_write, MemTxAttrs attrs);

static bool memory_access_is_direct(MemoryRegion *mr, bool is_write);

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

// FIXME the stupid NEED_CPU_H
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

// FIXME originally defined in exec.c
void *qemu_map_ram_ptr(RAMBlock *ram_block, ram_addr_t addr);

// FIXME originally defined in exec.c
static void invalidate_and_set_dirty(MemoryRegion *mr, hwaddr addr,
                                     hwaddr length);

// FIXME originally defined in exec.c
static bool prepare_mmio_access(MemoryRegion *mr);

#endif /* end of include guard: MEMORY_H_E0UHP2JS */
