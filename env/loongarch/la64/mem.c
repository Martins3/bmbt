#include <asm/mach-la64/boot_param.h>
#include <assert.h>
#include <exec/hwaddr.h>
#include <exec/ram_addr.h>
#include <linux/pfn.h>
#include <qemu/config-target.h> // CONFIG_GUEST_RAM_SIZE
#include <stdbool.h>
#include <stdio.h>

void fw_add_mem(unsigned long addr, unsigned long len);
void init_pages();
extern bool mmap_ready;

extern char __bss_stop[];
extern char __text_start[];

static hwaddr pc_ram_offset;
hwaddr get_pc_ram_offset() {
  assert(pc_ram_offset != 0);
  return pc_ram_offset;
}

void fw_init_memory(void) {
  int i;
  u32 mem_type;
  u64 mem_start, mem_end, mem_size;
  // static unsigned long num_physpages;
  // unsigned long start_pfn, end_pfn;
  // unsigned long kernel_end_pfn;
  u64 total_mem = 0;

  init_pages();
  // memory occupied by the kernel
  u64 text_start = TO_PHYS(PFN_ALIGN((u64)__text_start));
  u64 bss_end = TO_PHYS(PFN_ALIGN((u64)__bss_stop));

  bool kernel_img_in_range = false;

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

      printf("mem_start:0x%lx, mem_size:0x%lx Bytes\n", mem_start, mem_size);

      if (mem_start <= text_start && mem_end >= bss_end) {
        kernel_img_in_range = true;
        // @todo physical memory are hard coded
        // 1. 0 ~ 2M is occupied by the bios
        // 2. and kernel image in the 128M
        // 3. 2M ~ 128M used by guest
        assert(mem_start == 0x200000);
        assert(text_start == 0x8000000);
        assert(mem_end > bss_end);
        fw_add_mem(bss_end, mem_end - bss_end);
        total_mem += mem_end - bss_end;

        pc_ram_offset = 0x200000;
        break;
      }

      // printf("mem_start:0x%lx, mem_size:0x%lx Bytes\n", mem_start,
      // mem_size);
      printf("start_pfn:0x%lx, end_pfn:0x%lx\n", mem_start >> PAGE_SHIFT,
             (mem_start + mem_size) >> PAGE_SHIFT);
      printf("pfn number %lx\n", mem_size >> PAGE_SHIFT);

      fw_add_mem(mem_start, mem_size);
      total_mem += mem_size;

      // add_memory_region(mem_start, mem_size, BOOT_MEM_RAM);
      // memblock_set_node(mem_start, mem_size, &memblock.memory, 0);
      break;
    case ADDRESS_TYPE_ACPI:
      // printf("mem_type:%d ", mem_type);
      // printf("mem_start:0x%lx, mem_size:0x%lx Bytes\n", mem_start,
      // mem_size);
      // add_memory_region(mem_start, mem_size, BOOT_MEM_RESERVED);
      mem_start = PFN_ALIGN(mem_start - PAGE_SIZE + 1);
      mem_end = PFN_ALIGN(mem_end);
      mem_size = mem_end - mem_start;
      // memblock_add(mem_start, mem_size);
      // memblock_mark_nomap(mem_start, mem_size);
      // memblock_set_node(mem_start, mem_size, &memblock.memory, 0);
      // memblock_reserve(mem_start, mem_size);
      break;
    case ADDRESS_TYPE_RESERVED:
      // printf("mem_type:%d ", mem_type);
      // printf("mem_start:0x%lx, mem_size:0x%lx Bytes\n", mem_start,
      // mem_size);
      // add_memory_region(mem_start, mem_size, BOOT_MEM_RESERVED);
      // memblock_reserve(mem_start, mem_size);
      break;
    }
  }
  mmap_ready = true;
  printf("total_mem %lx bytes\n", total_mem);
  assert(kernel_img_in_range);
}
