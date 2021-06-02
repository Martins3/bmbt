#include "../include/firmware.h"
#include "../include/interface.h"

#define SPEC_VERS 0x2
#define SYSTEM_RAM_LOW 1
#define SYSTEM_RAM_HIGH 2
#define MEM_RESERVED 3
#define PCI_IO 4
#define PCI_MEM 5
#define LOONGSON_CFG_REG 6
#define VIDEO_ROM 7
#define ADAPTER_ROM 8
#define ACPI_TABLE 9
#define SMBIOS_TABLE 10
#define UMA_VIDEO_RAM 11
#define VUMA_VIDEO_RAM 12
#define MAX_MEMORY_TYPE 13

void memblock_add(u64 mem_start, u64 mem_size) {
  duck_printf("0x%lx: 0x%lx MB\n", mem_start, mem_size);
}

void mem_init(void) {
  duck_printf("huxueshi:%s \n", __FUNCTION__);

  int i;
  u32 mem_type;
  u64 mem_start, mem_end, mem_size;

  /* parse memory information */
  for (i = 0; i < loongson_mem_map->nr_map; i++) {

    mem_type = loongson_mem_map->map[i].mem_type;
    mem_start = loongson_mem_map->map[i].mem_start;
    mem_size = loongson_mem_map->map[i].mem_size;
    mem_end = mem_start + mem_size;
    // TODO wolf book P155 :  why qemu init highmem init_memory_map ?

    switch (mem_type) {
    case SYSTEM_RAM_LOW:
      memblock_add(mem_start, mem_size);
      break;
    case SYSTEM_RAM_HIGH:
      memblock_add(mem_start, mem_size);
      break;
    }
  }
  // memblock_set_current_limit(PFN_PHYS(max_low_pfn));
}
