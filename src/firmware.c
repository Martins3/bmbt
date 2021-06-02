#include "../include/interface.h"
#include "../include/firmware.h"

struct efi_memory_map_loongson *loongson_mem_map;
struct efi_cpuinfo_loongson *efi_cpuinfo_loongson;
struct system_loongson *system_loongson;
struct irq_source_routing_table *irq_source_routing_table;
struct interface_info *interface_info;
struct board_devices *board_devices;
struct loongson_special_attribute *loongson_special_attribute;

static inline void dump_loongson_params(struct loongson_params *lp) {
  duck_printf("memory_offset = 0x%lx;cpu_offset = 0x%lx; system_offset = "
              "0x%lx; irq_offset = 0x%lx; interface_offset = 0x%lx;\n",
              lp->memory_offset, lp->cpu_offset, lp->system_offset,
              lp->irq_offset, lp->interface_offset);
}

void prom_init_env(void) {
  struct boot_params *boot_params = (struct boot_params *)fw_arg2;
  struct loongson_params *lp = &boot_params->efi.smbios.lp;
  void *base = (void *)lp;

  BUG_ON(boot_params->magic != 0x12345678);
  dump_loongson_params(lp);

  loongson_mem_map =
      (struct efi_memory_map_loongson *)(base + lp->memory_offset);
  efi_cpuinfo_loongson = (struct efi_cpuinfo_loongson *)(base + lp->cpu_offset);
  system_loongson = (struct system_loongson *)(base + lp->system_offset);
  irq_source_routing_table =
      (struct irq_source_routing_table *)(base + lp->irq_offset);
  interface_info = (struct interface_info *)(base + lp->interface_offset);
  board_devices = (struct board_devices *)(base + lp->boarddev_table_offset);

  loongson_special_attribute =
      (struct loongson_special_attribute *)(base + lp->special_offset);
}

