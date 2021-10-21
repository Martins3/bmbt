#ifndef MEMORY_H_E0UHP2JS
#define MEMORY_H_E0UHP2JS

#include "../../src/tcg/glib_stub.h"
#include "cpu-common.h"
#include "hwaddr.h"
#include "memattrs.h"
#include "memop.h"

// #include "ram_addr.h"

struct AddressSpace;
struct RAMBlock;

typedef struct MemoryRegion {
  bool readonly;
  // BMBT: In QEMU ram_block != NULL doesn't mean ram == true
  // see memory_region_init_rom_device_nomigrate, but BMBT is simplified
  bool ram;

  struct RAMBlock *ram_block;

  hwaddr offset_in_ram_block;
  struct AddressSpace *as; // memory region is shared between AddressSpace
} MemoryRegion;

typedef struct PioRegion {
} PioRegion;

typedef struct AddressSpace {
  // [interface 8]
  MemoryRegion *segments;
} AddressSpace;

// move include/exec/address-spaces.h code here
extern AddressSpace address_space_io;
extern AddressSpace address_space_memory;

static inline void io_add_memory_region(hwaddr offset, MemoryRegion *mr) {
  // FIXME
}

static inline void mem_add_memory_region(hwaddr offset, MemoryRegion *mr) {
  // FIXME
}

#define RAM_ADDR_INVALID (~(ram_addr_t)0)

MemoryRegion *address_space_translate(AddressSpace *as, hwaddr addr,
                                      hwaddr *xlat, hwaddr *len, bool is_write,
                                      MemTxAttrs attrs);

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

/* address_space_access_valid: check for validity of accessing an address
 * space range
 *
 * Check whether memory is assigned to the given address space range, and
 * access is permitted by any IOMMU regions that are active for the address
 * space.
 *
 * For now, addr and len should be aligned to a page size.  This limitation
 * will be lifted in the future.
 *
 * @as: #AddressSpace to be accessed
 * @addr: address within that address space
 * @len: length of the area to be checked
 * @is_write: indicates the transfer direction
 * @attrs: memory attributes
 */
bool address_space_access_valid(AddressSpace *as, hwaddr addr, hwaddr len,
                                bool is_write, MemTxAttrs attrs);

/**
 * address_space_rw: read from or write to an address space.
 *
 * Return a MemTxResult indicating whether the operation succeeded
 * or failed (eg unassigned memory, device rejected the transaction,
 * IOMMU fault).
 *
 * @as: #AddressSpace to be accessed
 * @addr: address within that address space
 * @attrs: memory transaction attributes
 * @buf: buffer with the data transferred
 * @len: the number of bytes to read or write
 * @is_write: indicates the transfer direction
 */
MemTxResult address_space_rw(AddressSpace *as, hwaddr addr, MemTxAttrs attrs,
                             uint8_t *buf, hwaddr len, bool is_write);

#define NEED_CPU_H
#ifdef NEED_CPU_H
/* enum device_endian to MemOp.  */
static inline MemOp devend_memop(enum device_endian end) {
  const int non_host_endianness = DEVICE_BIG_ENDIAN;
  /* In this case, native (target) endianness needs no swap.  */
  return (end == non_host_endianness) ? MO_BSWAP : 0;
}
#endif
#undef NEED_CPU_H
void invalidate_and_set_dirty(MemoryRegion *mr, hwaddr addr, hwaddr length);
bool prepare_mmio_access(MemoryRegion *mr);

/**
 * memory_region_is_ram: check whether a memory region is random access
 *
 * Returns %true if a memory region is random access.
 *
 * @mr: the memory region being queried
 */
static inline bool memory_region_is_ram(MemoryRegion *mr) { return mr->ram; }

ram_addr_t memory_region_get_ram_addr(MemoryRegion *mr);
void *qemu_map_ram_ptr(struct RAMBlock *ram_block, ram_addr_t addr);

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
  // [interface 4]
  void *ptr;
  uint64_t offset = 0;
  ptr = qemu_map_ram_ptr(mr->ram_block, offset);
  return ptr;
}

static inline bool memory_access_is_direct(MemoryRegion *mr, bool is_write) {
  // [interface 3]
  if (is_write) {
    return memory_region_is_ram(mr) && !mr->readonly;
  } else {
    return memory_region_is_ram(mr);
  }
}

#endif /* end of include guard: MEMORY_H_E0UHP2JS */
