#include "../../include/exec/memory.h"
#include "../../include/exec/ram_addr.h"
#include "../../include/hw/southbridge/piix.h"
#include "../../include/qemu/bswap.h"
#include "../../include/qemu/units.h"
#include <fcntl.h> // for open
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

bool mr_initialized(MemoryRegion *mr) {
  int is_mmio = mr->ops != NULL;
  int is_ram = mr->ram_block != NULL;
  return is_mmio + is_ram == 1;
}
static void as_add_memory_regoin(AddressSpaceDispatch *dispatch,
                                 MemoryRegion *mr, hwaddr offset) {
  duck_check(mr_initialized(mr));

  mr->offset = offset;
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

    if (mr->offset >= gap.left && mr->offset + mr->size < gap.right) {
      break;
    }
  }

  if (i == dispatch->segment_num) {
    MemoryRegion *last_mr = dispatch[i].segments[dispatch->segment_num - 1];
    duck_check(offset >= last_mr->offset + last_mr->size);
  }

  dispatch->segment_num++;

  for (int k = dispatch->segment_num; k > i; --k) {
    dispatch[k] = dispatch[k - 1];
  }
  dispatch->segments[i] = mr;
}

// @todo initialize them
bool smram_enable;
bool smram_region_enable;

static bool is_smram_access(hwaddr offset) {
  return offset >= SMRAM_C_BASE && offset < SMRAM_C_END;
}

typedef bool (*MemoryRegionMatch)(MemoryRegion *mr, hwaddr offset);

static bool mem_mr_match(MemoryRegion *mr, hwaddr offset) {
  return offset >= mr->offset && offset < mr->offset + mr->size;
}

static bool io_mr_match(MemoryRegion *mr, hwaddr offset) {
  return offset == mr->offset;
}

static MemoryRegion *memory_region_look_up(AddressSpaceDispatch *dispatch,
                                           hwaddr offset,
                                           MemoryRegionMatch mr_match) {
  for (int i = 0; i < dispatch->segment_num; ++i) {
    MemoryRegion *mr = dispatch[i].segments[i];
    if (mr_match(mr, offset)) {
      return mr;
    }
  }
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
  // @todo I think pio always write beginning and toward to end
  // maybe not true for PIIX_RCR_IOPORT
  // maybe PIIX_RCR_IOPORT never got used
  xlat = 0;
  // @todo address_space_io is used in misc_helper.c and jump memory_ldst.c
  // I think it's impossible to cross the memory region
  duck_check(*plen != mr->size);
  *plen = MIN(*plen, mr->size);
  return mr;
}

static MemoryRegion *mem_mr_look_up(struct AddressSpace *as, hwaddr offset,
                                    hwaddr *xlat, hwaddr *plen) {
  if (is_smram_access(offset)) {
    if (!smram_region_enable) {
      return as->dispatch->special_mr;
    }
    if (as->smm && smram_enable) {
      return as->dispatch->special_mr;
    }
  }
  MemoryRegion *mr = memory_region_look_up(as->dispatch, offset, mem_mr_match);
  *xlat = offset - mr->offset;

  if (memory_region_is_ram(mr)) {
    hwaddr diff = mr->size - *xlat;
    *plen = MIN(diff, *plen);
  }
  return mr;
}

void io_add_memory_region(hwaddr offset, MemoryRegion *mr) {
  as_add_memory_regoin(address_space_io.dispatch, mr, offset);
}

void mem_add_memory_region(hwaddr offset, MemoryRegion *mr) {
  as_add_memory_regoin(address_space_memory.dispatch, mr, offset);
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
// is_write and attrs are used for iommu
MemoryRegion *address_space_translate(AddressSpace *as, hwaddr addr,
                                      hwaddr *xlat, hwaddr *len, bool is_write,
                                      MemTxAttrs attrs) {
  return as->mr_look_up(as, addr, xlat, len);
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

static char __bios[BIOS_FILE_SIZE];

static void x86_bios_rom_init() {
  int fd = open("/home/maritns3/core/seabios/out/bios.bin", O_RDONLY);
  duck_check(fd != -1);

  lseek(fd, 0, SEEK_SET);
  int rc = read(fd, __bios, BIOS_FILE_SIZE);
  duck_check(rc == BIOS_FILE_SIZE);

  RAMBlock *block = &ram_list.blocks[PC_BIOS_INDEX].block;
  block->host = (void *)(&__bios[0]);

  // e0000-fffff (prio 1, rom): alias isa-bios @pc.bios 20000-3ffff
  for (int i = 0; i < PAM_EXBIOS_NUM; ++i) {
    block = &ram_list.blocks[PAM_EXBIOS_INDEX].block;
    block->host = (void *)(&__bios[0]) + 128 * KiB + PAM_EXBIOS_SIZE * i;
  }
  block = &ram_list.blocks[PAM_BIOS_INDEX].block;
  block->host = (void *)(&__bios[0]) + 128 * KiB + PAM_EXBIOS_SIZE * 4;

  duck_check(PAM_EXBIOS_SIZE * 4 + PAM_BIOS_SIZE == 128 * KiB);
}

static void *alloc_ram(hwaddr size) {
  // (qemu) qemu_ram_mmap size=0x180200000 flags=0x22 guardfd=-1
  void *host = mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  duck_check(host != (void *)-1);
  return host;
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
// @todo change endless assignment to function
static void ram_init(hwaddr total_ram_size) {
  RAMBlock *block;
  MemoryRegion *mr;

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

  // pc.ram low
  mr = &ram_list.blocks[PC_LOW_RAM_INDEX].mr;
  mr->readonly = false;
  mr->offset = 0;
  mr->size = SMRAM_C_BASE;
  mr->name = "pc.ram low";

  // smram
  mr = &ram_list.blocks[PC_LOW_RAM_INDEX].mr;
  mr->readonly = false;
  mr->offset = SMRAM_C_BASE;
  mr->size = SMRAM_C_SIZE;
  mr->name = "smram";

  // pam expan and pam exbios
  duck_check(PAM_EXPAN_SIZE == PAM_EXBIOS_SIZE);
  for (int i = 0; i < PAM_EXPAN_NUM + PAM_EXBIOS_NUM; ++i) {
    mr = &ram_list.blocks[PAM_INDEX].mr;
    mr->readonly = true;
    mr->offset = SMRAM_C_END + i * PAM_EXPAN_SIZE;
    mr->size = PAM_EXPAN_SIZE;
    mr->name = i < PAM_EXPAN_NUM ? "pam expan" : "pam exbios";
  }

  mr = &ram_list.blocks[PAM_BIOS_INDEX].mr;
  mr->readonly = true;
  mr->offset = SMRAM_C_END * (PAM_EXPAN_NUM + PAM_EXBIOS_NUM);
  mr->size = PAM_BIOS_SIZE;
  mr->name = "pam bios";

  duck_check(mr->offset + mr->size == BIOS_MEM_SIZE);

  // pc.ram
  mr = &ram_list.blocks[PC_RAM_INDEX].mr;
  mr->readonly = false;
  mr->offset = BIOS_MEM_SIZE;
  mr->size = total_ram_size - BIOS_MEM_SIZE;
  mr->name = "pc.ram";

  // pc.bios
  mr = &ram_list.blocks[PC_BIOS_INDEX].mr;
  mr->readonly = true;
  mr->offset = 4 * GiB - 256 * KiB;
  mr->size = total_ram_size - BIOS_MEM_SIZE;
  mr->name = "pc.ram";

  for (int i = 0; i < RAM_BLOCK_NUM; ++i) {
    RAMBlock *block = &ram_list.blocks[i].block;
    MemoryRegion *mr = &ram_list.blocks[i].mr;

    block->offset = mr->offset;
    block->max_length = mr->size;
    block->host = host + block->offset;
  }

  // pc.bios offset is not same with mr.offset
  block = &ram_list.blocks[PC_BIOS_INDEX].block;
  block->offset = total_ram_size;

  // isa-bios / pc.bios's host point to file
  x86_bios_rom_init();
}

static AddressSpaceDispatch __memory_dispatch;
static AddressSpaceDispatch __io_dispatch;

void memory_map_init(hwaddr size) {
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
