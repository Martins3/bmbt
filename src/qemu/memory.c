#include "../../include/exec/memory.h"
#include "../../include/exec/ram_addr.h"
#include "../../include/hw/southbridge/piix.h"
#include "../../include/qemu/bswap.h"
#include "../../include/qemu/units.h"
#include <fcntl.h> // for open
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

static bool mr_initialized(const MemoryRegion *mr) {
  duck_check(mr != NULL);
  int is_mmio = mr->ops != NULL;
  int is_ram = mr->ram_block != NULL;
  if (mr->offset == 0) {
    duck_check(strcmp(mr->name, "pc.ram low") == 0);
  }
  return is_mmio + is_ram == 1;
}

static void as_add_memory_regoin(AddressSpaceDispatch *dispatch,
                                 MemoryRegion *mr) {
#ifndef RELEASE_VERSION
  duck_check(mr_initialized(mr));
#endif
  hwaddr offset = mr->offset;

  tcg_commit();

  struct Gap {
    uint64_t left;
    uint64_t right;
  } gap;

  int i;
  for (i = 0; i < dispatch->segment_num; ++i) {
    gap.left = i > 0 ? dispatch->segments[i - 1]->offset +
                           dispatch->segments[i - 1]->size
                     : 0;
    gap.right = dispatch->segments[i]->offset;

    if (mr->offset >= gap.left && mr->offset + mr->size <= gap.right) {
      break;
    }
  }

#ifndef RELEASE_VERSION
  if (i == dispatch->segment_num) {
    if (i > 0) {
      MemoryRegion *last_mr = dispatch->segments[i - 1];
      duck_check(offset >= last_mr->offset + last_mr->size);
    } else {
      duck_check(offset >= 0);
    }
  }
#endif

  for (int k = dispatch->segment_num; k > i; --k) {
    dispatch->segments[k] = dispatch->segments[k - 1];
  }
  dispatch->segments[i] = mr;

  dispatch->segment_num++;
}

// @todo actually in the beginning
// smram_region_enable = 1, smram_enable = 0
// make vga-lowmem available to normal CPU
bool smram_region_enable;
// make cpu in smm mode can ignore smram_region_enable
bool smram_enable;

static bool is_smram_access(hwaddr offset) {
  return offset >= SMRAM_C_BASE && offset < SMRAM_C_END;
}

static bool is_isa_bios_access(hwaddr offset) {
  return offset >= PAM_EXBIOS_BASE && offset < PAM_BIOS_END;
}

typedef bool (*MemoryRegionMatch)(const MemoryRegion *mr, hwaddr offset);

static bool mem_mr_match(const MemoryRegion *mr, hwaddr offset) {
  return offset >= mr->offset && offset < mr->offset + mr->size;
}

static bool io_mr_match(const MemoryRegion *mr, hwaddr offset) {
  return offset == mr->offset;
}

static MemoryRegion *memory_region_look_up(AddressSpaceDispatch *dispatch,
                                           hwaddr offset,
                                           MemoryRegionMatch mr_match) {
  for (int i = 0; i < dispatch->segment_num; ++i) {
    MemoryRegion *mr = dispatch->segments[i];
    duck_check(mr != NULL);
    if (mr_match(mr, offset)) {
      return mr;
    }
  }
  CPUX86State *env = ((CPUX86State *)current_cpu->env_ptr);
  printf("guest ip : %x\n", env->segs[R_CS].base + env->eip);
  printf("failed in [%s] with offset=[%lx]\n", dispatch->name, offset);
  g_assert_not_reached();
}

static MemoryRegion *io_mr_look_up(struct AddressSpace *as, hwaddr offset,
                                   hwaddr *xlat, hwaddr *plen) {
  MemoryRegion *mr;
  if (offset == PIIX_RCR_IOPORT) {
    mr = as->dispatch->special_mr;
  } else {
    mr = memory_region_look_up(as->dispatch, offset, io_mr_match);
  }
  *xlat = offset - mr->offset;
  return mr;
}

// e0000-fffff (prio 1, rom): alias isa-bios @pc.bios 20000-3ffff
static inline MemoryRegion *isa_bios_access(hwaddr offset, hwaddr *xlat,
                                            hwaddr *plen) {
  MemoryRegion *pc_bios;
  if (!is_isa_bios_access(offset)) {
    return NULL;
  }
  pc_bios = &ram_list.blocks[PC_BIOS_INDEX].mr;

  *xlat = offset - PAM_EXBIOS_BASE + 128 * KiB;
  *plen = MIN(*plen, pc_bios->size - *xlat);

  return pc_bios;
}

static MemoryRegion *mem_mr_look_up(struct AddressSpace *as, hwaddr offset,
                                    hwaddr *xlat, hwaddr *plen) {
  MemoryRegion *mr = isa_bios_access(offset, xlat, plen);
  if (mr) {
    return mr;
  }

  if (is_smram_access(offset)) {
    if (!as->smm && smram_region_enable) {
      mr = as->dispatch->special_mr;
    }

    if (as->smm && !smram_enable && smram_region_enable) {
      mr = as->dispatch->special_mr;
    }
  }

  if (mr == NULL) {
    mr = memory_region_look_up(as->dispatch, offset, mem_mr_match);
  }

  *xlat = offset - mr->offset;

  if (memory_region_is_ram(mr)) {
    duck_check(plen != NULL);
    hwaddr diff = mr->size - *xlat;
    *plen = MIN(diff, *plen);
  }
  return mr;
}

void io_add_memory_region(const hwaddr offset, MemoryRegion *mr) {
  duck_check(offset != 0 && mr->offset == 0);
  duck_check(!memory_region_is_ram(mr));
  mr->offset = offset;
  as_add_memory_regoin(address_space_io.dispatch, mr);
}

void mmio_add_memory_region(const hwaddr offset, MemoryRegion *mr) {
  duck_check(offset != 0 && mr->offset == 0);
  duck_check(!memory_region_is_ram(mr));
  mr->offset = offset;
  as_add_memory_regoin(address_space_io.dispatch, mr);
}

void mem_add_memory_region(MemoryRegion *mr) {
  duck_check(memory_region_is_ram(mr));
  as_add_memory_regoin(address_space_memory.dispatch, mr);
}

void memory_region_init_io(MemoryRegion *mr, const MemoryRegionOps *ops,
                           void *opaque, const char *name, uint64_t size) {
  // QEMU permits ops == NULL. But in BMBT, if can't find a MemoryRegion, there
  // is a runtime error
  duck_check(ops != NULL);
  duck_check(size != 0);

  mr->readonly = false;
  mr->ram = false;
  mr->ops = ops;
  mr->ram_block = NULL;
  mr->opaque = opaque;
  mr->name = name;
  mr->size = size;
}

/**
 * memory_region_get_ram_addr: Get the ram address associated with a memory
 *                             region
 */
ram_addr_t memory_region_get_ram_addr(MemoryRegion *mr) {
  duck_check(mr->ram_block != NULL);
  return mr->ram_block ? mr->ram_block->offset : RAM_ADDR_INVALID;
}

static MemoryRegion iotlb_mr = {};

// attrs and prot are used for iommu
MemoryRegion *address_space_translate_for_iotlb(CPUState *cpu, int asidx,
                                                hwaddr addr, hwaddr *xlat,
                                                hwaddr *plen, MemTxAttrs attrs,
                                                int *prot) {
  // [interface 34]
  RAMBlock *block;
  bool is_ram = false;
  RAMBLOCK_FOREACH(block) {
    if (addr >= block->mr->offset &&
        addr < block->mr->offset + block->max_length) {
      is_ram = true;
      break;
    }
  }
  if (!is_ram) {
    *plen = TARGET_PAGE_SIZE;
    return &iotlb_mr;
  }
  AddressSpace *as = cpu->cpu_ases[asidx].as;
  return as->mr_look_up(as, addr, xlat, plen);
}

// is_write and attrs are used for iommu
MemoryRegion *address_space_translate(AddressSpace *as, hwaddr addr,
                                      hwaddr *xlat, hwaddr *len, bool is_write,
                                      MemTxAttrs attrs) {
  return as->mr_look_up(as, addr, xlat, len);
}

static inline MemoryRegion *flatview_translate(AddressSpace *as, hwaddr addr,
                                               hwaddr *xlat, hwaddr *len,
                                               bool is_write,
                                               MemTxAttrs attrs) {
  return address_space_translate(as, addr, xlat, len, is_write, attrs);
}

static inline void memory_region_shift_read_access(uint64_t *value,
                                                   signed shift, uint64_t mask,
                                                   uint64_t tmp) {
  if (shift >= 0) {
    *value |= (tmp & mask) << shift;
  } else {
    *value |= (tmp & mask) >> -shift;
  }
}

static inline uint64_t
memory_region_shift_write_access(uint64_t *value, signed shift, uint64_t mask) {
  uint64_t tmp;

  if (shift >= 0) {
    tmp = (*value >> shift) & mask;
  } else {
    tmp = (*value << -shift) & mask;
  }

  return tmp;
}

static MemTxResult memory_region_read_accessor(MemoryRegion *mr, hwaddr addr,
                                               uint64_t *value, unsigned size,
                                               signed shift, uint64_t mask,
                                               MemTxAttrs attrs) {
  uint64_t tmp;

  tmp = mr->ops->read(mr->opaque, addr, size);
#ifdef BMBT
  if (mr->subpage) {
    trace_memory_region_subpage_read(get_cpu_index(), mr, addr, tmp, size);
  } else if (TRACE_MEMORY_REGION_OPS_READ_ENABLED) {
    hwaddr abs_addr = memory_region_to_absolute_addr(mr, addr);
    trace_memory_region_ops_read(get_cpu_index(), mr, abs_addr, tmp, size);
  }
#endif
  memory_region_shift_read_access(value, shift, mask, tmp);
  return MEMTX_OK;
}

static MemTxResult memory_region_write_accessor(MemoryRegion *mr, hwaddr addr,
                                                uint64_t *value, unsigned size,
                                                signed shift, uint64_t mask,
                                                MemTxAttrs attrs) {
  uint64_t tmp = memory_region_shift_write_access(value, shift, mask);

#ifdef BMBT
  if (mr->subpage) {
    trace_memory_region_subpage_write(get_cpu_index(), mr, addr, tmp, size);
  } else if (TRACE_MEMORY_REGION_OPS_WRITE_ENABLED) {
    hwaddr abs_addr = memory_region_to_absolute_addr(mr, addr);
    trace_memory_region_ops_write(get_cpu_index(), mr, abs_addr, tmp, size);
  }
#endif
  mr->ops->write(mr->opaque, addr, tmp, size);
  return MEMTX_OK;
}

static bool memory_region_big_endian(MemoryRegion *mr) {
#ifdef TARGET_WORDS_BIGENDIAN
  return mr->ops->endianness != DEVICE_LITTLE_ENDIAN;
#else
  return mr->ops->endianness == DEVICE_BIG_ENDIAN;
#endif
}

static MemTxResult access_with_adjusted_size(
    hwaddr addr, uint64_t *value, unsigned size, unsigned access_size_min,
    unsigned access_size_max,
    MemTxResult (*access_fn)(MemoryRegion *mr, hwaddr addr, uint64_t *value,
                             unsigned size, signed shift, uint64_t mask,
                             MemTxAttrs attrs),
    MemoryRegion *mr, MemTxAttrs attrs) {
  uint64_t access_mask;
  unsigned access_size;
  unsigned i;
  MemTxResult r = MEMTX_OK;

  if (!access_size_min) {
    access_size_min = 1;
  }
  if (!access_size_max) {
    access_size_max = 4;
  }

  /* FIXME: support unaligned access? */
  access_size = MAX(MIN(size, access_size_max), access_size_min);
  access_mask = MAKE_64BIT_MASK(0, access_size * 8);
  if (memory_region_big_endian(mr)) {
    for (i = 0; i < size; i += access_size) {
      r |= access_fn(mr, addr + i, value, access_size,
                     (size - access_size - i) * 8, access_mask, attrs);
    }
  } else {
    for (i = 0; i < size; i += access_size) {
      r |= access_fn(mr, addr + i, value, access_size, i * 8, access_mask,
                     attrs);
    }
  }
  return r;
}

static MemTxResult memory_region_dispatch_read1(MemoryRegion *mr, hwaddr addr,
                                                uint64_t *pval, unsigned size,
                                                MemTxAttrs attrs) {
  *pval = 0;

  if (mr->ops->read) {
    return access_with_adjusted_size(
        addr, pval, size, mr->ops->impl.min_access_size,
        mr->ops->impl.max_access_size, memory_region_read_accessor, mr, attrs);
  }
  g_assert_not_reached();
}

static void adjust_endianness(MemoryRegion *mr, uint64_t *data, MemOp op) {
  if ((op & MO_BSWAP) != devend_memop(mr->ops->endianness)) {
    switch (op & MO_SIZE) {
    case MO_8:
      break;
    case MO_16:
      *data = bswap16(*data);
      break;
    case MO_32:
      *data = bswap32(*data);
      break;
    case MO_64:
      *data = bswap64(*data);
      break;
    default:
      g_assert_not_reached();
    }
  }
}

bool memory_region_access_valid(MemoryRegion *mr, hwaddr addr, unsigned size,
                                bool is_write, MemTxAttrs attrs) {
  int access_size_min, access_size_max;
  int access_size, i;

  if (!mr->ops->valid.unaligned && (addr & (size - 1))) {
    return false;
  }

  if (!mr->ops->valid.accepts) {
    return true;
  }

  access_size_min = mr->ops->valid.min_access_size;
  if (!mr->ops->valid.min_access_size) {
    access_size_min = 1;
  }

  access_size_max = mr->ops->valid.max_access_size;
  if (!mr->ops->valid.max_access_size) {
    access_size_max = 4;
  }

  access_size = MAX(MIN(size, access_size_max), access_size_min);
  for (i = 0; i < size; i += access_size) {
    // @todo mem review code here
    // why we need accepts
    if (!mr->ops->valid.accepts(mr->opaque, addr + i, access_size, is_write,
                                attrs)) {
      return false;
    }
  }

  return true;
}

MemTxResult memory_region_dispatch_read(MemoryRegion *mr, hwaddr addr,
                                        uint64_t *pval, MemOp op,
                                        MemTxAttrs attrs) {
  unsigned size = memop_size(op);
  MemTxResult r;

  if (!memory_region_access_valid(mr, addr, size, false, attrs)) {
    g_assert_not_reached();
    // *pval = unassigned_mem_read(mr, addr, size);
    return MEMTX_DECODE_ERROR;
  }

  r = memory_region_dispatch_read1(mr, addr, pval, size, attrs);
  adjust_endianness(mr, pval, op);
  return r;
}

MemTxResult memory_region_dispatch_write(MemoryRegion *mr, hwaddr addr,
                                         uint64_t data, MemOp op,
                                         MemTxAttrs attrs) {
  unsigned size = memop_size(op);

  if (!memory_region_access_valid(mr, addr, size, true, attrs)) {
    g_assert_not_reached();
    // unassigned_mem_write(mr, addr, data, size);
    return MEMTX_DECODE_ERROR;
  }

  adjust_endianness(mr, &data, op);

  // no virtio eventfd notifier
#ifdef BMBT
  if ((!kvm_eventfds_enabled()) &&
      memory_region_dispatch_write_eventfds(mr, addr, data, size, attrs)) {
    return MEMTX_OK;
  }
#endif

  if (mr->ops->write) {
    return access_with_adjusted_size(
        addr, &data, size, mr->ops->impl.min_access_size,
        mr->ops->impl.max_access_size, memory_region_write_accessor, mr, attrs);
  }
  g_assert_not_reached();
}

static int memory_access_size(MemoryRegion *mr, unsigned l, hwaddr addr) {
  unsigned access_size_max = mr->ops->valid.max_access_size;

  /* Regions are assumed to support 1-4 byte accesses unless
     otherwise specified.  */
  if (access_size_max == 0) {
    access_size_max = 4;
  }

  /* Bound the maximum access by the alignment of the address.  */
  if (!mr->ops->impl.unaligned) {
    unsigned align_size_max = addr & -addr;
    if (align_size_max != 0 && align_size_max < access_size_max) {
      access_size_max = align_size_max;
    }
  }

  /* Don't attempt accesses larger than the maximum.  */
  if (l > access_size_max) {
    l = access_size_max;
  }
  l = pow2floor(l);

  return l;
}

/* Return a host pointer to guest's ram. Similar to qemu_map_ram_ptr
 * but takes a size argument.
 *
 * Called within RCU critical section.
 */
static void *qemu_ram_ptr_length(RAMBlock *ram_block, ram_addr_t addr,
                                 hwaddr *size, bool lock) {
  RAMBlock *block = ram_block;
#ifdef BMBT
  if (*size == 0) {
    return NULL;
  }

  if (block == NULL) {
    block = qemu_get_ram_block(addr);
    addr -= block->offset;
  }
#endif
  duck_check(block != NULL);
  *size = MIN(*size, block->max_length - addr);

#ifdef BMBT
  if (xen_enabled() && block->host == NULL) {
    /* We need to check if the requested address is in the RAM
     * because we don't want to map the entire memory in QEMU.
     * In that case just map the requested area.
     */
    if (block->offset == 0) {
      return xen_map_cache(addr, *size, lock, lock);
    }

    block->host = xen_map_cache(block->offset, block->max_length, 1, lock);
  }
#endif

  return ramblock_ptr(block, addr);
}

/* Called within RCU critical section.  */
static MemTxResult flatview_write_continue(AddressSpace *fv, hwaddr addr,
                                           MemTxAttrs attrs, const uint8_t *buf,
                                           hwaddr len, hwaddr addr1, hwaddr l,
                                           MemoryRegion *mr) {
  uint8_t *ptr;
  uint64_t val;
  MemTxResult result = MEMTX_OK;
  bool release_lock = false;

  for (;;) {
    if (!memory_access_is_direct(mr, true)) {
      release_lock |= prepare_mmio_access(mr);
      l = memory_access_size(mr, l, addr1);
      /* XXX: could force current_cpu to NULL to avoid
         potential bugs */
      val = ldn_he_p(buf, l);
      result |=
          memory_region_dispatch_write(mr, addr1, val, size_memop(l), attrs);
    } else {
      /* RAM case */
      ptr = qemu_ram_ptr_length(mr->ram_block, addr1, &l, false);
      memcpy(ptr, buf, l);
      invalidate_and_set_dirty(mr, addr1, l);
    }

    if (release_lock) {
      qemu_mutex_unlock_iothread();
      release_lock = false;
    }

    len -= l;
    buf += l;
    addr += l;

    if (!len) {
      break;
    }

    l = len;
    mr = flatview_translate(fv, addr, &addr1, &l, true, attrs);
  }

  return result;
}

/* Called from RCU critical section.  */
static MemTxResult flatview_write(AddressSpace *fv, hwaddr addr,
                                  MemTxAttrs attrs, const uint8_t *buf,
                                  hwaddr len) {
  hwaddr l;
  hwaddr addr1;
  MemoryRegion *mr;
  MemTxResult result = MEMTX_OK;

  l = len;
  mr = flatview_translate(fv, addr, &addr1, &l, true, attrs);
  result = flatview_write_continue(fv, addr, attrs, buf, len, addr1, l, mr);

  return result;
}

/* Called within RCU critical section.  */
MemTxResult flatview_read_continue(AddressSpace *fv, hwaddr addr,
                                   MemTxAttrs attrs, uint8_t *buf, hwaddr len,
                                   hwaddr addr1, hwaddr l, MemoryRegion *mr) {
  uint8_t *ptr;
  uint64_t val;
  MemTxResult result = MEMTX_OK;
  bool release_lock = false;

  for (;;) {
    if (!memory_access_is_direct(mr, false)) {
      /* I/O case */
      release_lock |= prepare_mmio_access(mr);
      l = memory_access_size(mr, l, addr1);
      result |=
          memory_region_dispatch_read(mr, addr1, &val, size_memop(l), attrs);
      stn_he_p(buf, l, val);
    } else {
      /* RAM case */
      ptr = qemu_ram_ptr_length(mr->ram_block, addr1, &l, false);
      memcpy(buf, ptr, l);
    }

    if (release_lock) {
      qemu_mutex_unlock_iothread();
      release_lock = false;
    }

    len -= l;
    buf += l;
    addr += l;

    if (!len) {
      break;
    }

    l = len;
    mr = flatview_translate(fv, addr, &addr1, &l, false, attrs);
  }

  return result;
}

/* Called from RCU critical section.  */
static MemTxResult flatview_read(AddressSpace *fv, hwaddr addr,
                                 MemTxAttrs attrs, uint8_t *buf, hwaddr len) {
  hwaddr l;
  hwaddr addr1;
  MemoryRegion *mr;

  l = len;
  mr = flatview_translate(fv, addr, &addr1, &l, false, attrs);
  return flatview_read_continue(fv, addr, attrs, buf, len, addr1, l, mr);
}

MemTxResult address_space_read_full(AddressSpace *fv, hwaddr addr,
                                    MemTxAttrs attrs, uint8_t *buf,
                                    hwaddr len) {
  MemTxResult result = MEMTX_OK;

  if (len > 0) {
    RCU_READ_LOCK_GUARD();
    result = flatview_read(fv, addr, attrs, buf, len);
  }

  return result;
}

MemTxResult address_space_write(AddressSpace *fv, hwaddr addr, MemTxAttrs attrs,
                                const uint8_t *buf, hwaddr len) {
  MemTxResult result = MEMTX_OK;

  if (len > 0) {
    RCU_READ_LOCK_GUARD();
    result = flatview_write(fv, addr, attrs, buf, len);
  }

  return result;
}

MemTxResult address_space_rw(AddressSpace *as, hwaddr addr, MemTxAttrs attrs,
                             uint8_t *buf, hwaddr len, bool is_write) {

  if (is_write) {
    return address_space_write(as, addr, attrs, buf, len);
  } else {
    return address_space_read_full(as, addr, attrs, buf, len);
  }
}

bool prepare_mmio_access(MemoryRegion *mr) {
  bool unlocked = !qemu_mutex_iothread_locked();
  bool release_lock = false;

  if (unlocked) {
    qemu_mutex_lock_iothread();
    unlocked = false;
    release_lock = true;
  }
#ifdef BMBT
  if (mr->flush_coalesced_mmio) {
    if (unlocked) {
      qemu_mutex_lock_iothread();
    }
    qemu_flush_coalesced_mmio_buffer();
    if (unlocked) {
      qemu_mutex_unlock_iothread();
    }
  }
#endif

  return release_lock;
}

uint8_t memory_region_get_dirty_log_mask(MemoryRegion *mr) {
  uint8_t mask = 0;
  if (mr->ram_block) {
    mask |= (1 << DIRTY_MEMORY_MIGRATION);
  }
  return mask;
}

// originally defined in dirty_memory_extend
static void setup_dirty_memory(hwaddr total_ram_size) {
  ram_addr_t new_num_blocks =
      DIV_ROUND_UP(total_ram_size, DIRTY_MEMORY_BLOCK_SIZE);

  DirtyMemoryBlocks *new_blocks = g_malloc(
      sizeof(*new_blocks) + sizeof(new_blocks->blocks[0]) * new_num_blocks);

  for (int j = 0; j < new_num_blocks; j++) {
    new_blocks->blocks[j] = bitmap_new(DIRTY_MEMORY_BLOCK_SIZE);
  }
  ram_list.dirty_memory[DIRTY_MEMORY_CODE] = new_blocks;
}

static char __pc_bios[PC_BIOS_IMG_SIZE];

static ram_addr_t x86_bios_rom_init() {
  int fd = open("./seabios/out/bios.bin", O_RDONLY);
  duck_check(fd != -1);

  lseek(fd, 0, SEEK_SET);
  int rc = read(fd, __pc_bios, PC_BIOS_IMG_SIZE);
  duck_check(rc == PC_BIOS_IMG_SIZE);
  close(fd);

  RAMBlock *block = &ram_list.blocks[PC_BIOS_INDEX].block;
  block->host = (void *)(&__pc_bios[0]);

  // isa-bios is handled in function isa_bios_access
  return PC_BIOS_IMG_SIZE;
}

static void *alloc_ram(hwaddr size) {
  // (qemu) qemu_ram_mmap size=0x180200000 flags=0x22 guardfd=-1
  void *host = mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  duck_check(host != (void *)-1);
  return host;
}

static inline void init_ram_block(const char *name, unsigned int index,
                                  bool readonly, hwaddr offset, uint64_t size) {
  MemoryRegion *mr = &ram_list.blocks[index].mr;
  mr->readonly = readonly;
  mr->offset = offset;
  mr->size = size;
  mr->name = name;
}
/*
 * in pc_memory_init initialized
 *  - pc.ram
 *  - pc.rom
 *
 * in x86_bios_rom_init
 *  - pc.bios
 *  - isa-bios
 *
 * in cpu_address_space_init
 *  - smm
 */
static void ram_init(ram_addr_t total_ram_size) {
  ram_addr_t rom_size;
  void *host = alloc_ram(total_ram_size);
  for (int i = 0; i < RAM_BLOCK_NUM; ++i) {
    RAMBlock *block = &ram_list.blocks[i].block;
    MemoryRegion *mr = &ram_list.blocks[i].mr;

    block->mr = mr;
    mr->ram_block = block;
    mr->ram = true;
    mr->opaque = NULL;
    mr->ops = NULL;
  }

  init_ram_block("pc.ram low", PC_LOW_RAM_INDEX, false, 0, SMRAM_C_BASE);
  init_ram_block("smram", SMRAM_INDEX, false, SMRAM_C_BASE, SMRAM_C_SIZE);

  // pam expan and pam exbios
  duck_check(PAM_EXPAN_SIZE == PAM_EXBIOS_SIZE);
  for (int i = 0; i < PAM_EXPAN_NUM + PAM_EXBIOS_NUM; ++i) {
    const char *name = i < PAM_EXPAN_NUM ? "pam expan" : "pam exbios";
    hwaddr offset = SMRAM_C_END + i * PAM_EXPAN_SIZE;
    init_ram_block(name, PAM_INDEX + i, true, offset, PAM_EXPAN_SIZE);
  }

  init_ram_block("system bios", PAM_BIOS_INDEX, true, PAM_BIOS_BASE,
                 PAM_BIOS_SIZE);

  duck_check(SMRAM_C_BASE + SMRAM_C_SIZE +
                 PAM_EXPAN_SIZE * (PAM_EXPAN_NUM + PAM_EXBIOS_NUM) ==
             PAM_BIOS_BASE);
  duck_check(ram_list.blocks[PAM_BIOS_INDEX].mr.offset +
                 ram_list.blocks[PAM_BIOS_INDEX].mr.size ==
             X86_BIOS_MEM_SIZE);

  init_ram_block("pc.ram", PC_RAM_INDEX, false, X86_BIOS_MEM_SIZE,
                 total_ram_size - X86_BIOS_MEM_SIZE);
  init_ram_block("pc.bios", PC_BIOS_INDEX, true, 4 * GiB - PC_BIOS_IMG_SIZE,
                 PC_BIOS_IMG_SIZE);

  for (int i = 0; i < RAM_BLOCK_NUM; ++i) {
    RAMBlock *block = &ram_list.blocks[i].block;
    MemoryRegion *mr = &ram_list.blocks[i].mr;

    block->offset = mr->offset;
    block->max_length = mr->size;
    block->host = host + block->offset;
  }

  // pc.bios's block::offset is not same with it's mr.offset
  RAMBlock *block = &ram_list.blocks[PC_BIOS_INDEX].block;
  block->offset = total_ram_size;

  for (int i = 0; i < RAM_BLOCK_NUM; ++i) {
    MemoryRegion *mr = &ram_list.blocks[i].mr;
    mem_add_memory_region(mr);
  }

  // isa-bios / pc.bios's host point to file
  rom_size = x86_bios_rom_init();
  setup_dirty_memory(total_ram_size + rom_size);
}

static AddressSpaceDispatch __memory_dispatch = {.name = "memory dispatch"};
static AddressSpaceDispatch __io_dispatch = {.name = "io dispatch"};

void memory_map_init(ram_addr_t size) {
#ifdef BMBT
  system_memory = g_malloc(sizeof(*system_memory));

  memory_region_init(system_memory, NULL, "system", UINT64_MAX);
  address_space_init(&address_space_memory, system_memory, "memory");

  system_io = g_malloc(sizeof(*system_io));
  memory_region_init_io(system_io, NULL, &unassigned_io_ops, NULL, "io", 65536);
  address_space_init(&address_space_io, system_io, "I/O");
#endif
  address_space_io.mr_look_up = io_mr_look_up;
  address_space_io.dispatch = &__io_dispatch;

  address_space_memory.mr_look_up = mem_mr_look_up;
  address_space_memory.dispatch = &__memory_dispatch;

  address_space_smm_memory.mr_look_up = mem_mr_look_up;
  address_space_smm_memory.dispatch = &__memory_dispatch;
  address_space_smm_memory.smm = true;

  ram_init(size);
}
