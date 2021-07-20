#include "../../include/exec/cpu-all.h"
#include "../../include/exec/cpu-common.h"
#include "../../include/exec/cpu-defs.h"
#include "../../include/exec/exec-all.h"
#include "../../include/exec/memop.h"
#include "../../include/exec/memory.h"
#include "../../include/exec/ramlist.h"
#include "../../include/hw/core/cpu.h"
#include "../../include/qemu/atomic.h"
#include "../../include/qemu/bswap.h"
#include "../../include/qemu/osdep.h"
#include "../../include/qemu/rcu.h"

#include <stdbool.h>

static inline uint32_t address_space_ldl_internal(AddressSpace *as, hwaddr addr,
                                                  MemTxAttrs attrs,
                                                  MemTxResult *result,
                                                  enum device_endian endian) {
  uint8_t *ptr;
  uint64_t val;
  MemoryRegion *mr;
  hwaddr l = 4;
  hwaddr addr1;
  MemTxResult r;
  bool release_lock = false;

  rcu_read_lock();
  mr = address_space_translate(as, addr, &addr1, &l, false, attrs);
  if (l < 4 || !memory_access_is_direct(mr, false)) {
    release_lock |= prepare_mmio_access(mr);

    r = memory_region_dispatch_read(mr, addr1, &val,
                                    MO_32 | devend_memop(endian), attrs);
  } else {

    ptr = qemu_map_ram_ptr(mr->ram_block, addr1);
    switch (endian) {
    case DEVICE_LITTLE_ENDIAN:
      val = ldl_le_p(ptr);
      break;
    case DEVICE_BIG_ENDIAN:
      val = ldl_be_p(ptr);
      break;
    default:
      val = ldl_p(ptr);
      break;
    }
    r = MEMTX_OK;
  }
  if (result) {
    *result = r;
  }
  if (release_lock) {
    qemu_mutex_unlock_iothread();
  }
  rcu_read_unlock();
  return val;
}

uint32_t address_space_ldl(AddressSpace *as, hwaddr addr, MemTxAttrs attrs,
                           MemTxResult *result) {
  return address_space_ldl_internal(as, addr, attrs, result,
                                    DEVICE_NATIVE_ENDIAN);
}

static inline uint64_t address_space_ldq_internal(AddressSpace *as, hwaddr addr,
                                                  MemTxAttrs attrs,
                                                  MemTxResult *result,
                                                  enum device_endian endian) {
  uint8_t *ptr;
  uint64_t val;
  MemoryRegion *mr;
  hwaddr l = 8;
  hwaddr addr1;
  MemTxResult r;
  bool release_lock = false;

  rcu_read_lock();
  mr = address_space_translate(as, addr, &addr1, &l, false, attrs);
  if (l < 8 || !memory_access_is_direct(mr, false)) {
    release_lock |= prepare_mmio_access(mr);

    r = memory_region_dispatch_read(mr, addr1, &val,
                                    MO_64 | devend_memop(endian), attrs);
  } else {

    ptr = qemu_map_ram_ptr(mr->ram_block, addr1);
    switch (endian) {
    case DEVICE_LITTLE_ENDIAN:
      val = ldq_le_p(ptr);
      break;
    case DEVICE_BIG_ENDIAN:
      val = ldq_be_p(ptr);
      break;
    default:
      val = ldq_p(ptr);
      break;
    }
    r = MEMTX_OK;
  }
  if (result) {
    *result = r;
  }
  if (release_lock) {
    qemu_mutex_unlock_iothread();
  }
  rcu_read_unlock();
  return val;
}

uint64_t address_space_ldq(AddressSpace *as, hwaddr addr, MemTxAttrs attrs,
                           MemTxResult *result) {
  return address_space_ldq_internal(as, addr, attrs, result,
                                    DEVICE_NATIVE_ENDIAN);
}

uint32_t address_space_ldub(AddressSpace *as, hwaddr addr, MemTxAttrs attrs,
                            MemTxResult *result) {
  uint8_t *ptr;
  uint64_t val;
  MemoryRegion *mr;
  hwaddr l = 1;
  hwaddr addr1;
  MemTxResult r;
  bool release_lock = false;

  rcu_read_lock();
  mr = address_space_translate(as, addr, &addr1, &l, false, attrs);
  if (!memory_access_is_direct(mr, false)) {
    release_lock |= prepare_mmio_access(mr);

    r = memory_region_dispatch_read(mr, addr1, &val, MO_8, attrs);
  } else {

    ptr = qemu_map_ram_ptr(mr->ram_block, addr1);
    val = ldub_p(ptr);
    r = MEMTX_OK;
  }
  if (result) {
    *result = r;
  }
  if (release_lock) {
    qemu_mutex_unlock_iothread();
  }
  rcu_read_unlock();
  return val;
}

static inline uint32_t
address_space_lduw_internal(AddressSpace *as, hwaddr addr, MemTxAttrs attrs,
                            MemTxResult *result, enum device_endian endian) {
  uint8_t *ptr;
  uint64_t val;
  MemoryRegion *mr;
  hwaddr l = 2;
  hwaddr addr1;
  MemTxResult r;
  bool release_lock = false;

  rcu_read_lock();
  mr = address_space_translate(as, addr, &addr1, &l, false, attrs);
  if (l < 2 || !memory_access_is_direct(mr, false)) {
    release_lock |= prepare_mmio_access(mr);

    r = memory_region_dispatch_read(mr, addr1, &val,
                                    MO_16 | devend_memop(endian), attrs);
  } else {

    ptr = qemu_map_ram_ptr(mr->ram_block, addr1);
    switch (endian) {
    case DEVICE_LITTLE_ENDIAN:
      val = lduw_le_p(ptr);
      break;
    case DEVICE_BIG_ENDIAN:
      val = lduw_be_p(ptr);
      break;
    default:
      val = lduw_p(ptr);
      break;
    }
    r = MEMTX_OK;
  }
  if (result) {
    *result = r;
  }
  if (release_lock) {
    qemu_mutex_unlock_iothread();
  }
  rcu_read_unlock();
  return val;
}

uint32_t address_space_lduw(AddressSpace *as, hwaddr addr, MemTxAttrs attrs,
                            MemTxResult *result) {
  return address_space_lduw_internal(as, addr, attrs, result,
                                     DEVICE_NATIVE_ENDIAN);
}

void address_space_stl_notdirty(AddressSpace *as, hwaddr addr, uint32_t val,
                                MemTxAttrs attrs, MemTxResult *result) {
  uint8_t *ptr;
  MemoryRegion *mr;
  hwaddr l = 4;
  hwaddr addr1;
  MemTxResult r;
  uint8_t dirty_log_mask;
  bool release_lock = false;

  rcu_read_lock();
  mr = address_space_translate(as, addr, &addr1, &l, true, attrs);
  if (l < 4 || !memory_access_is_direct(mr, true)) {
    release_lock |= prepare_mmio_access(mr);

    r = memory_region_dispatch_write(mr, addr1, val, MO_32, attrs);
  } else {
    ptr = qemu_map_ram_ptr(mr->ram_block, addr1);
    stl_p(ptr, val);

    dirty_log_mask = memory_region_get_dirty_log_mask(mr);
    dirty_log_mask &= ~(1 << DIRTY_MEMORY_CODE);
    cpu_physical_memory_set_dirty_range(memory_region_get_ram_addr(mr) + addr,
                                        4, dirty_log_mask);
    r = MEMTX_OK;
  }
  if (result) {
    *result = r;
  }
  if (release_lock) {
    qemu_mutex_unlock_iothread();
  }
  rcu_read_unlock();
}

static inline void address_space_stl_internal(AddressSpace *as, hwaddr addr,
                                              uint32_t val, MemTxAttrs attrs,
                                              MemTxResult *result,
                                              enum device_endian endian) {
  uint8_t *ptr;
  MemoryRegion *mr;
  hwaddr l = 4;
  hwaddr addr1;
  MemTxResult r;
  bool release_lock = false;

  rcu_read_lock();
  mr = address_space_translate(as, addr, &addr1, &l, true, attrs);
  if (l < 4 || !memory_access_is_direct(mr, true)) {
    release_lock |= prepare_mmio_access(mr);
    r = memory_region_dispatch_write(mr, addr1, val,
                                     MO_32 | devend_memop(endian), attrs);
  } else {

    ptr = qemu_map_ram_ptr(mr->ram_block, addr1);
    switch (endian) {
    case DEVICE_LITTLE_ENDIAN:
      stl_le_p(ptr, val);
      break;
    case DEVICE_BIG_ENDIAN:
      stl_be_p(ptr, val);
      break;
    default:
      stl_p(ptr, val);
      break;
    }
    invalidate_and_set_dirty(mr, addr1, 4);
    r = MEMTX_OK;
  }
  if (result) {
    *result = r;
  }
  if (release_lock) {
    qemu_mutex_unlock_iothread();
  }
  rcu_read_unlock();
}

void address_space_stl(AddressSpace *as, hwaddr addr, uint32_t val,
                       MemTxAttrs attrs, MemTxResult *result) {
  address_space_stl_internal(as, addr, val, attrs, result,
                             DEVICE_NATIVE_ENDIAN);
}

void address_space_stb(AddressSpace *as, hwaddr addr, uint32_t val,
                       MemTxAttrs attrs, MemTxResult *result) {
  uint8_t *ptr;
  MemoryRegion *mr;
  hwaddr l = 1;
  hwaddr addr1;
  MemTxResult r;
  bool release_lock = false;

  rcu_read_lock();
  mr = address_space_translate(as, addr, &addr1, &l, true, attrs);
  if (!memory_access_is_direct(mr, true)) {
    release_lock |= prepare_mmio_access(mr);
    r = memory_region_dispatch_write(mr, addr1, val, MO_8, attrs);
  } else {

    ptr = qemu_map_ram_ptr(mr->ram_block, addr1);
    stb_p(ptr, val);
    invalidate_and_set_dirty(mr, addr1, 1);
    r = MEMTX_OK;
  }
  if (result) {
    *result = r;
  }
  if (release_lock) {
    qemu_mutex_unlock_iothread();
  }
  rcu_read_unlock();
}

static inline void address_space_stw_internal(AddressSpace *as, hwaddr addr,
                                              uint32_t val, MemTxAttrs attrs,
                                              MemTxResult *result,
                                              enum device_endian endian) {
  uint8_t *ptr;
  MemoryRegion *mr;
  hwaddr l = 2;
  hwaddr addr1;
  MemTxResult r;
  bool release_lock = false;

  rcu_read_lock();
  mr = address_space_translate(as, addr, &addr1, &l, true, attrs);
  if (l < 2 || !memory_access_is_direct(mr, true)) {
    release_lock |= prepare_mmio_access(mr);
    r = memory_region_dispatch_write(mr, addr1, val,
                                     MO_16 | devend_memop(endian), attrs);
  } else {

    ptr = qemu_map_ram_ptr(mr->ram_block, addr1);
    switch (endian) {
    case DEVICE_LITTLE_ENDIAN:
      stw_le_p(ptr, val);
      break;
    case DEVICE_BIG_ENDIAN:
      stw_be_p(ptr, val);
      break;
    default:
      stw_p(ptr, val);
      break;
    }
    invalidate_and_set_dirty(mr, addr1, 2);
    r = MEMTX_OK;
  }
  if (result) {
    *result = r;
  }
  if (release_lock) {
    qemu_mutex_unlock_iothread();
  }
  rcu_read_unlock();
}

void address_space_stw(AddressSpace *as, hwaddr addr, uint32_t val,
                       MemTxAttrs attrs, MemTxResult *result) {
  address_space_stw_internal(as, addr, val, attrs, result,
                             DEVICE_NATIVE_ENDIAN);
}

static void address_space_stq_internal(AddressSpace *as, hwaddr addr,
                                       uint64_t val, MemTxAttrs attrs,
                                       MemTxResult *result,
                                       enum device_endian endian) {
  uint8_t *ptr;
  MemoryRegion *mr;
  hwaddr l = 8;
  hwaddr addr1;
  MemTxResult r;
  bool release_lock = false;

  rcu_read_lock();
  mr = address_space_translate(as, addr, &addr1, &l, true, attrs);
  if (l < 8 || !memory_access_is_direct(mr, true)) {
    release_lock |= prepare_mmio_access(mr);
    r = memory_region_dispatch_write(mr, addr1, val,
                                     MO_64 | devend_memop(endian), attrs);
  } else {

    ptr = qemu_map_ram_ptr(mr->ram_block, addr1);
    switch (endian) {
    case DEVICE_LITTLE_ENDIAN:
      stq_le_p(ptr, val);
      break;
    case DEVICE_BIG_ENDIAN:
      stq_be_p(ptr, val);
      break;
    default:
      stq_p(ptr, val);
      break;
    }
    invalidate_and_set_dirty(mr, addr1, 8);
    r = MEMTX_OK;
  }
  if (result) {
    *result = r;
  }
  if (release_lock) {
    qemu_mutex_unlock_iothread();
  }
  rcu_read_unlock();
}

void address_space_stq(AddressSpace *as, hwaddr addr, uint64_t val,
                       MemTxAttrs attrs, MemTxResult *result) {
  address_space_stq_internal(as, addr, val, attrs, result,
                             DEVICE_NATIVE_ENDIAN);
}
