#include <asm/mach-la64/boot_param.h>
#include <assert.h>
#include <env/memory.h>
#include <exec/hwaddr.h>
#include <exec/ram_addr.h>
#include <linux/pfn.h>
#include <qemu/queue.h>
#include <qemu/units.h>
#include <stdbool.h>
#include <stdio.h>

void setup_buddy();
void fw_add_mem(unsigned long addr, long len);
void init_pages();

typedef struct PhysicalRamDesc {
  RamRange range;
  QTAILQ_ENTRY(PhysicalRamDesc) next;
} PhysicalRamDesc;

static QTAILQ_HEAD(, PhysicalRamDesc) guest_ram_list;
static QTAILQ_HEAD(, PhysicalRamDesc) host_ram_list;

#define RAM_DESC_NUM 40
PhysicalRamDesc ram_pool[RAM_DESC_NUM];
static int idx = -1;
static PhysicalRamDesc *alloc_ram_desc(RamRange range) {
  idx++;
  assert(idx < RAM_DESC_NUM);
  ram_pool[idx].range = range;
  return &ram_pool[idx];
}

RamRange kernel_image_range;
RamRange guest_range = {
    .start = 0,
    .end = 0xe0000000, // 3.5G
};

static void add_host_ram(RamRange range) {
  PhysicalRamDesc *ram = alloc_ram_desc(range);
  QTAILQ_INSERT_TAIL(&host_ram_list, ram, next);
}

static int guest_ram_num;
static void add_guest_ram(RamRange range) {
  PhysicalRamDesc *ram = alloc_ram_desc(range);
  QTAILQ_INSERT_TAIL(&guest_ram_list, ram, next);
  guest_ram_num++;
}

static inline bool is_overlap(RamRange a, RamRange b) {
  return !(a.end <= b.start || b.end <= a.start);
}

static inline void check_cover(RamRange bigger, RamRange smaller) {
  assert(bigger.start <= smaller.start && bigger.end >= smaller.end);
}

static inline void exchange_range(RamRange *a, RamRange *b) {
  RamRange tmp = *a;
  *a = *b;
  *b = tmp;
}

static RamRange get_overlap(RamRange l, RamRange r) {
  if (l.start > r.start) {
    exchange_range(&l, &r);
  }
  /*
   * after exchange:
   * l.start --- r.start --- r.end --- l.end
   * l.start --- r.start --- l.end --- r.end
   */
  RamRange overlap = {.start = r.start, .end = MIN(l.end, r.end)};
  return overlap;
}

static inline PhysicalRamDesc *
overlap_with_guest_ram(PhysicalRamDesc *ram_desc) {
  const RamRange ram = ram_desc->range;

  if (is_overlap(ram, guest_range)) {
    RamRange overlap = get_overlap(ram, guest_range);
    add_guest_ram(overlap);

    if (overlap.start > ram.start && overlap.end < ram.end) {
      ram_desc->range.start = ram.start;
      ram_desc->range.end = overlap.start;

      RamRange range = {.start = overlap.end, .end = ram.end};
      return alloc_ram_desc(range);
    }

    if (ram.start < overlap.start) {
      /* ram.start --- overlap.start --- ram.end / overlap.end */
      ram_desc->range.start = ram.start;
      ram_desc->range.end = overlap.start;
      return NULL;
    }

    if (ram.end > overlap.end) {
      /* ram.start / overlap.start --- overlap.end --- ram.end */
      ram_desc->range.start = overlap.end;
      ram_desc->range.end = ram.end;
      return NULL;
    }

    assert(ram.start == overlap.start && ram.end == overlap.end);
    ram_desc->range.start = 0;
    ram_desc->range.end = 0;
  }
  return NULL;
}

static PhysicalRamDesc *overlap_with_kernel_iamge(PhysicalRamDesc *ram_desc) {
  if (is_overlap(ram_desc->range, kernel_image_range)) {
    check_cover(ram_desc->range, kernel_image_range);

    /* ram.start --- image.start --- image.end --- ram.end */
    RamRange ram = ram_desc->range;
    ram_desc->range.start = ram.start;
    ram_desc->range.end = kernel_image_range.start;
    RamRange range = {.start = kernel_image_range.end, .end = ram.end};
    return alloc_ram_desc(range);
  }
  return NULL;
}

#define PRINT_RAM(head)                                                        \
  printf("%-12s %-12s   size M  size B\n", "start", "end");                    \
  QTAILQ_FOREACH(ram, head, next) {                                            \
    mem_size = ram->range.end - ram->range.start;                              \
    if (mem_size != 0)                                                         \
      printf("0x%-10lx 0x%-10lx   0x%05ld %lx\n", ram->range.start,            \
             ram->range.end, mem_size / MiB, mem_size);                        \
  }

static void reserve_memory() {
  PhysicalRamDesc *ram;
  u64 mem_size;

  printf("-------------- Host RAM --------------------\n");
  PRINT_RAM(&host_ram_list);

  QTAILQ_FOREACH(ram, &host_ram_list, next) {
    PhysicalRamDesc *second = overlap_with_kernel_iamge(ram);
    if (second != NULL) {
      QTAILQ_INSERT_AFTER(&host_ram_list, ram, second, next);
    }
  }

  QTAILQ_FOREACH(ram, &host_ram_list, next) {
    PhysicalRamDesc *second = overlap_with_guest_ram(ram);
    if (second != NULL) {
      QTAILQ_INSERT_AFTER(&host_ram_list, ram, second, next);
      fw_add_mem(second->range.start, second->range.end);
    }
    fw_add_mem(ram->range.start, ram->range.end);
  }

  printf("-------------- BMBT Allocator ----------------\n");
  PRINT_RAM(&host_ram_list);

  printf("-------------- Guest RAM --------------------\n");
  PRINT_RAM(&guest_ram_list);
}

void init_ram_range() {
  extern char __bss_stop[];
  extern char __text_start[];
  kernel_image_range.start = TO_PHYS(PFN_ALIGN((u64)__text_start));
  kernel_image_range.end = TO_PHYS(PFN_ALIGN((u64)__bss_stop));

  QTAILQ_INIT(&guest_ram_list);
  QTAILQ_INIT(&host_ram_list);
}

extern bool memory_ready;

/*
 * 1. all memory in range (0 ~ 3.5G) belows. As for 3.5G, see
 *    hw/i386/pc_piix.c:pc_init1
 * 2. 32bit guest unable to access ram above 4G
 * 3. bmbt is put to 4G, so make sure the memory is not ram memory hole
 *
 * physical:
 *
 * [0x200000       0xf000000   ] : 0xee00000   Bytes 0xee    Mib
 * [0x90200000     0xc0000000  ] : 0x2fe00000  Bytes 0x2fe   Mib
 * [0xc0020000     0xfcb4b000  ] : 0x3cb2b000  Bytes 0x3cb   Mib
 * [0xfcb94000     0xfd34d000  ] : 0x7b9000    Bytes 0x7     Mib
 * [0xfd3b8000     0xfe438000  ] : 0x1080000   Bytes 0x10    Mib
 * [0xfe490000     0xff000000  ] : 0xb70000    Bytes 0xb     Mib
 * [0xff000000     0x480000000 ] : 0x381000000 Bytes 0x3810  Mib
 *
 * guest
 *
 * [0x      200000 0xf000000] : 0x     ee00000 Bytes
 * [0x    c0000000 0x280000000] : 0x   1c0000000 Bytes
 * [0x    90000000 0xbc5d0000] : 0x    2c5d0000 Bytes
 * [0x    bc630000 0xbf900000] : 0x     32d0000 Bytes
 * [0x    bfc40000 0xc0000000] : 0x      3c0000 Bytes
 *
 */
void fw_init_memory(void) {
  int i;
  u32 mem_type;
  u64 mem_start, mem_end, mem_size;

  init_pages();
#ifndef HAMT
  init_ram_range();
  bool check_bios_memory = false;
#else
  // memory occupied by the kernel
  u64 text_start = TO_PHYS(PFN_ALIGN((u64)__text_start));
  u64 bss_end = TO_PHYS(PFN_ALIGN((u64)__bss_stop));

  bool kernel_img_in_range = false;
  // allocate address space for data_storage and code_storage
  // before memory parse, because in `kernel_mmap`, 'addr' must be 0.
  uint64_t data_storage;
  uint64_t code_storage;
  // allocate for data_storage
  // data_storage = 0x9000000000de4000
  data_storage = TO_CAC(PFN_ALIGN(bss_end));
  bss_end += 4096;
  bss_end = PFN_ALIGN(bss_end);
  // allocate for code_storage
  // code_storage = 0x9000000000de8000
  code_storage = TO_CAC(PFN_ALIGN(bss_end));
  bss_end += 4096;
  bss_end = PFN_ALIGN(bss_end);
  printf("data_storage:0x%lx, code_storage:0x%lx\n", data_storage,
         code_storage);
#endif

  /* parse memory information */
  for (i = 0; i < loongson_mem_map->map_count; i++) {
    mem_type = loongson_mem_map->map[i].mem_type;
    mem_start = loongson_mem_map->map[i].mem_start;
    mem_size = loongson_mem_map->map[i].mem_size;
    mem_end = mem_start + mem_size;

    switch (mem_type) {
    case ADDRESS_TYPE_SYSRAM:
      mem_start = PFN_ALIGN(mem_start);
      mem_end = PFN_ALIGN(mem_end - PAGE_SIZE + 1);
      if (mem_start >= mem_end)
        break;

      if (mem_start == 0x200000) {
        check_bios_memory = true;
        mem_start = 0;
      }
      RamRange ram = {.start = mem_start, .end = mem_end};
      add_host_ram(ram);

      break;
    case ADDRESS_TYPE_ACPI:
      break;
    case ADDRESS_TYPE_RESERVED:
      break;
    }
  }
  assert(check_bios_memory);
  reserve_memory();
  memory_ready = true;
  setup_buddy();
}

int get_guest_ram_num() {
  assert(MAX_PC_RAM_NUM > guest_ram_num);
  return guest_ram_num;
}

uint64_t get_guest_total_ram() {
  uint64_t total_ram_size = 0;
  for (int i = 0; i < get_guest_ram_num(); ++i) {
    total_ram_size += guest_ram(i).end - guest_ram(i).start;
  }
  return total_ram_size;
}

RamRange guest_ram(int idx) {
  PhysicalRamDesc *ram;
  QTAILQ_FOREACH(ram, &guest_ram_list, next) {
    if (idx == 0) {
      return ram->range;
    }
    idx--;
  }
  printf("out of range");
  abort();
}

uint64_t get_host_offset() { return TO_CAC(0); }
