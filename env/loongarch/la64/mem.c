#include <asm/mach-la64/boot_param.h>
#include <internal.h>
#include <pfh.h>

void fw_init_memory(void) {
  int i;
  u32 mem_type;
  u64 mem_start, mem_end, mem_size;
  static unsigned long num_physpages;
  // unsigned long start_pfn, end_pfn;
  // unsigned long kernel_end_pfn;

  u64 total_mem = 0;
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
      num_physpages += (mem_size >> PAGE_SHIFT);
      total_mem += mem_size;
      duck_printf("mem_start:0x%lx, mem_size:0x%lx Bytes\n", mem_start,
                  mem_size);
      duck_printf("start_pfn:0x%lx, end_pfn:0x%lx\n", mem_start >> PAGE_SHIFT,
                  (mem_start + mem_size) >> PAGE_SHIFT);

      // add_memory_region(mem_start, mem_size, BOOT_MEM_RAM);
      // memblock_set_node(mem_start, mem_size, &memblock.memory, 0);
      break;
    case ADDRESS_TYPE_ACPI:
      // duck_printf("mem_type:%d ", mem_type);
      // duck_printf("mem_start:0x%lx, mem_size:0x%lx Bytes\n", mem_start,
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
      // duck_printf("mem_type:%d ", mem_type);
      // duck_printf("mem_start:0x%lx, mem_size:0x%lx Bytes\n", mem_start,
      // mem_size);
      // add_memory_region(mem_start, mem_size, BOOT_MEM_RESERVED);
      // memblock_reserve(mem_start, mem_size);
      break;
    }
  }
  duck_printf("total_mem %lx bytes\n", total_mem);
}
