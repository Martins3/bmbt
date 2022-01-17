#ifndef BOOT_PARAM_H_UOQ1F6AT
#define BOOT_PARAM_H_UOQ1F6AT
#include <linux/type.h>

#define ADDRESS_TYPE_SYSRAM 1
#define ADDRESS_TYPE_RESERVED 2
#define ADDRESS_TYPE_ACPI 3
#define ADDRESS_TYPE_NVS 4
#define ADDRESS_TYPE_PMEM 5

#define LOONGSON3_BOOT_MEM_MAP_MAX 128

struct loongson_system_configuration {
  char *cpuname;
  int nr_cpus;
  int nr_nodes;
  int cores_per_node;
  int cores_per_package;
  u16 boot_cpu_id;
  u64 reserved_cpus_mask;
  u64 ht_control_base;
  u64 restart_addr;
  u64 poweroff_addr;
  u64 suspend_addr;
  u64 vgabios_addr;
  u32 dma_mask_bits;
  u32 msi_address_lo;
  u32 msi_address_hi;
  u32 msi_base_irq;
  u32 msi_last_irq;
  u32 io_base_irq;
  u32 io_last_irq;
  u32 bpi_version;
  u8 pcie_wake_enabled;
  u8 is_soc_cpu;
};

#define LOONGSON_DMA_MASK_BIT 64
#define LOONGSON_MEM_SIGNATURE "MEM"
#define LOONGSON_VBIOS_SIGNATURE "VBIOS"
#define LOONGSON_EFIBOOT_SIGNATURE "BPI"
#define LOONGSON_SCREENINFO_SIGNATURE "SINFO"
#define LOONGSON_EFIBOOT_VERSION 1000

enum bpi_version {
  BPI_VERSION_NONE = 0,
  BPI_VERSION_V1 = 1000,
  BPI_VERSION_V2 = 1001,
  BPI_VERSION_V3 = 1002,
};

union extlist_line {
  struct _extention_list_hdr *extlist;
  u64 ext_offset;
};

struct boot_params {
  u64 signature; /* {"BPIXXXXX"} */
  union {
    void *systemtable;
    u64 systab_offset;
  };
  union extlist_line ext_location;
  u64 flags;
} __attribute__((packed));

struct _extention_list_hdr {
  u64 signature;
  u32 length;
  u8 revision;
  u8 checksum;
  union extlist_line next_ext;
} __attribute__((packed));

struct loongsonlist_mem_map {
  struct _extention_list_hdr header; /*{"M", "E", "M"}*/
  u8 map_count;
  struct _loongson_mem_map {
    u32 mem_type;
    u64 mem_start;
    u64 mem_size;
  } __attribute__((packed)) map[LOONGSON3_BOOT_MEM_MAP_MAX];
} __attribute__((packed));

extern struct loongsonlist_mem_map *loongson_mem_map;

#endif /* end of include guard: BOOT_PARAM_H_UOQ1F6AT */
