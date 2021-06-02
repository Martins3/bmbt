#include "../include/interface.h"

/*
 * Most firmware pass arguments and environment variables as 32-bit pointers.
 * These take care of sign extension.
 */
#define fw_argv(index) ((char *)(long)_fw_argv[(index)])
#define fw_envp(index) ((char *)(long)_fw_envp[(index)])

int fw_argc;
long *_fw_argv, *_fw_envp;
#define COMMAND_LINE_SIZE 512

char arcs_cmdline[COMMAND_LINE_SIZE];

#define LOONGSON3_BOOT_MEM_MAP_MAX 128

#define LOONGSON_DMA_MASK_BIT 64
#define LOONGSON_MEM_LINKLIST "MEM"
#define LOONGSON_VBIOS_LINKLIST "VBIOS"
#define LOONGSON_EFIBOOT_SIGNATURE "BPI"
#define LOONGSON_SCREENINFO_LINKLIST "SINFO"

struct _extention_list_hdr {
  u64 signature;
  u32 length;
  u8 revision;
  u8 checksum;
  struct _extention_list_hdr *next;
} __attribute__((packed));

struct bootparamsinterface {
  u64 signature; /*{"B", "P", "I", "_", "0", "_", "1"}*/
  void *systemtable;
  struct _extention_list_hdr *extlist;
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
  u8 pcie_wake_enabled;
};

struct bootparamsinterface *efi_bp;
struct loongsonlist_mem_map *loongson_mem_map;
struct loongsonlist_vbios *pvbios;
struct loongson_system_configuration loongson_sysconf;

// TODO remove these supid global variable transfer
// fw_argc _fw_argv and ...
void fw_init_cmdline(void) {
  int i;

  fw_argc = fw_arg0;
  // a array of argv addr
  _fw_argv = (long *)fw_arg1;
  _fw_envp = (long *)fw_arg2;
  // how to change this to me?
}

u8 ext_listhdr_checksum(u8 *buffer, u32 length) {
  u8 sum = 0;
  u8 *end = buffer + length;

  while (buffer < end) {
    sum = (u8)(sum + *(buffer++));
  }

  return (sum);
}

int parse_mem(struct _extention_list_hdr *head) {
  duck_printf("huxueshi:%s \n", __FUNCTION__);
  loongson_mem_map = (struct loongsonlist_mem_map *)head;
  if (ext_listhdr_checksum((u8 *)loongson_mem_map, head->length)) {
    duck_printf("mem checksum error\n");
    return -1;
  }
  return 0;
}

int parse_vbios(struct _extention_list_hdr *head) {
  duck_printf("%s", __FUNCTION__);
  // pvbios = (struct loongsonlist_vbios *)head;
  //
  // if (ext_listhdr_checksum((u8 *)pvbios, head->length)) {
  // printk("vbios_addr checksum error\n");
  // return -EPERM;
  // } else {
  // loongson_sysconf.vgabios_addr = pvbios->vbios_addr;
  // }
  return 0;
}

static int parse_screeninfo(struct _extention_list_hdr *head) {
  struct loongsonlist_screeninfo *pscreeninfo;

  pscreeninfo = (struct loongsonlist_screeninfo *)head;
  if (ext_listhdr_checksum((u8 *)pscreeninfo, head->length)) {
    duck_printf("screeninfo_addr checksum error\n");
    return -1;
  }

  // TODO
  // duck_memcpy(&screen_info, &pscreeninfo->si, sizeof(screen_info));
  return 0;
}

static int list_find(struct _extention_list_hdr *head) {
  struct _extention_list_hdr *fhead = head;

  if (fhead == NULL) {
    duck_printf("the link is empty!\n");
    return -1;
  }

  while (fhead != NULL) {
    duck_printf("huxueshi:%s %lx\n", __FUNCTION__, fhead);
    if (duck_memcmp(&(fhead->signature), LOONGSON_MEM_LINKLIST, 3) == 0) {
      if (parse_mem(fhead) != 0) {
        duck_printf("parse mem failed\n");
        return -1;
      }
    } else if (duck_memcmp(&(fhead->signature), LOONGSON_VBIOS_LINKLIST, 5) ==
               0) {
      if (parse_vbios(fhead) != 0) {
        duck_printf("parse vbios failed\n");
        return -1;
      }
    } else if (duck_memcmp(&(fhead->signature), LOONGSON_SCREENINFO_LINKLIST,
                           5) == 0) {
      if (parse_screeninfo(fhead) != 0) {
        duck_printf("parse screeninfo failed\n");
        return -1;
      }
    }
    fhead = fhead->next;
  }
  return 0;
}

struct loongson_params {
    u64 memory_offset;	/* efi_memory_map_loongson struct offset */
    u64 cpu_offset;		/* efi_cpuinfo_loongson struct offset */
    u64 system_offset;	/* system_loongson struct offset */
    u64 irq_offset; 	/* irq_source_routing_table struct offset */
    u64 interface_offset;	/* interface_info struct offset */
    u64 special_offset;	/* loongson_special_attribute struct offset */
    u64 boarddev_table_offset;  /* board_devices offset */
};

struct smbios_tables {
    u16 vers;     /* version of smbios */
    u16 dummy[3]; /*dump make vga_bios align*/
    u64 vga_bios; /* vga_bios address */
    struct loongson_params lp;
};

struct efi_reset_system_t{
    u64 ResetCold;
    u64 ResetWarm;
    u64 ResetType;
    u64 Shutdown;
    u64 DoSuspend; /* NULL if not support */
};

struct efi_loongson {
    u64 mps;	/* MPS table */
    u64 acpi;	/* ACPI table (IA64 ext 0.71) */
    u64 acpi20;	/* ACPI table (ACPI 2.0) */
    struct smbios_tables smbios;	/* SM BIOS table */
    u64 sal_systab;	/* SAL system table */
    u64 boot_info;	/* boot info table */
};

struct boot_params{
    int magic;
    struct efi_loongson efi;
    struct efi_reset_system_t reset_system;
};

struct boot_params * boot_params;
void prom_init_env(void) {
  boot_params = (struct boot_params *)_fw_envp;

  duck_printf("signature [%lx]\n", boot_params);
  // debug_signature(efi_bp->signature);
  BUG_ON(1);

  if (list_find(efi_bp->extlist))
    duck_printf("Scan bootparm failed\n");
  // A list of loongson_regaddr_set
  // loongson_regaddr_set
}
