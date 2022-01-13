#include <asm/addrspace.h>
#include <asm/fw.h>
#include <asm/mach-la64/boot_param.h>
#include <errno.h>
#include <internal.h>
#include <stdio.h>
#include <string.h>

struct boot_params *efi_bp;

struct boot_params *efi_bp;
struct loongsonlist_mem_map *loongson_mem_map;
struct loongsonlist_vbios *pvbios;
struct loongson_system_configuration loongson_sysconf;

#ifdef BMBT
u64 loongson_chipcfg[MAX_PACKAGES];
u64 loongson_chiptemp[MAX_PACKAGES];
u64 loongson_freqctrl[MAX_PACKAGES];
unsigned long long smp_group[MAX_PACKAGES];

void *loongson_fdt_blob;
EXPORT_SYMBOL(loongson_sysconf);

static void loongson_regaddr_set(u64 *loongson_reg, const u64 addr, int num) {
  u64 i;

  for (i = 0; i < num; i++) {
    *loongson_reg = (i << 44) | addr;
    loongson_reg++;
  }
}
#endif

static u8 ext_listhdr_checksum(u8 *buffer, u32 length) {
  u8 sum = 0;
  u8 *end = buffer + length;

  while (buffer < end) {
    sum = (u8)(sum + *(buffer++));
  }

  return (sum);
}

static int parse_mem(struct _extention_list_hdr *head) {
  loongson_mem_map = (struct loongsonlist_mem_map *)head;
  if (ext_listhdr_checksum((u8 *)loongson_mem_map, head->length)) {
    duck_printf("mem checksum error\n");
    return -EPERM;
  }
  return 0;
}

#ifdef BMBT
static int parse_vbios(struct _extention_list_hdr *head) {
  pvbios = (struct loongsonlist_vbios *)head;

  if (ext_listhdr_checksum((u8 *)pvbios, head->length)) {
    duck_printf("vbios_addr checksum error\n");
    return -EPERM;
  }
  loongson_sysconf.vgabios_addr = (unsigned long)early_memremap_ro(
      pvbios->vbios_addr, sizeof(unsigned long));

  return 0;
}

static int parse_screeninfo(struct _extention_list_hdr *head) {
  struct loongsonlist_screeninfo *pscreeninfo;

  pscreeninfo = (struct loongsonlist_screeninfo *)head;
  if (ext_listhdr_checksum((u8 *)pscreeninfo, head->length)) {
    duck_printf("screeninfo_addr checksum error\n");
    return -EPERM;
  }

  memcpy(&screen_info, &pscreeninfo->si, sizeof(screen_info));
  return 0;
}
#endif

// TMP_TODO what the fuck is BPI ?
static int list_find(struct boot_params *bp) {
  struct _extention_list_hdr *fhead = NULL;
  unsigned long index;

  if (loongson_sysconf.bpi_version >= BPI_VERSION_V3)
    fhead = (struct _extention_list_hdr *)((char *)bp +
                                           bp->ext_location.ext_offset);
  else
    fhead = bp->ext_location.extlist;

  if (!fhead) {
    duck_printf("the bp ext struct empty!\n");
    return -1;
  }

  do {
    if (memcmp(&(fhead->signature), LOONGSON_MEM_SIGNATURE, 3) == 0) {
      if (parse_mem(fhead) != 0) {
        duck_printf("parse mem failed\n");
        return -EPERM;
      }
    } else if (memcmp(&(fhead->signature), LOONGSON_VBIOS_SIGNATURE, 5) == 0) {
#ifdef BMBT
      if (parse_vbios(fhead) != 0) {
        duck_printf("parse vbios failed\n");
        return -EPERM;
      }
#else
      duck_printf("parse vbios failed\n");
#endif
    } else if (memcmp(&(fhead->signature), LOONGSON_SCREENINFO_SIGNATURE, 5) ==
               0) {
#ifdef BMBT
      if (parse_screeninfo(fhead) != 0) {
        duck_printf("parse screeninfo failed\n");
        return -EPERM;
      }
#else
      duck_printf("parse screeninfo failed\n");
#endif
    }
    if (loongson_sysconf.bpi_version >= BPI_VERSION_V3) {
      index = fhead->next_ext.ext_offset;
      fhead = (struct _extention_list_hdr *)((char *)bp +
                                             fhead->next_ext.ext_offset);
    } else {
      fhead = (struct _extention_list_hdr *)fhead->next_ext.extlist;
      index = (unsigned long)fhead;
    }

  } while (index);

  return 0;
}

#ifdef NEED_LATER
static int get_bpi_version(u64 *signature) {
  u8 data[9];
  int version = BPI_VERSION_NONE;
  data[8] = 0;
  memcpy(data, signature, sizeof(*signature));
  if (kstrtoint(&data[3], 10, &version))
    return BPI_VERSION_NONE;
  return version;
}

static void parse_bpi_flags(void) {
  if (efi_bp->flags & BPI_FLAGS_UEFI_SUPPORTED) {
    set_bit(EFI_BOOT, &efi.flags);
  } else {
    clear_bit(EFI_BOOT, &efi.flags);
  }

  if (efi_bp->flags & BPI_FLAGS_SOC_CPU)
    loongson_sysconf.is_soc_cpu = 1;
}
#endif

void fw_init_env(void) {
  efi_bp = (struct boot_params *)TO_CAC((unsigned long)_fw_envp);
  /// TMP_TODO ??
  // loongson_sysconf.bpi_version = get_bpi_version(&efi_bp->signature);
  duck_printf("BPI%d with boot flags %lx.\n", loongson_sysconf.bpi_version,
              efi_bp->flags);
  if (loongson_sysconf.bpi_version == BPI_VERSION_NONE)
    duck_printf("Fatal error, incorrect BPI version: %d\n",
                loongson_sysconf.bpi_version);

  else if (loongson_sysconf.bpi_version >= BPI_VERSION_V2) {
    // TMP_TODO ???
    // parse_bpi_flags();
  }

#ifdef BMBT
  loongson_regaddr_set(smp_group, 0x800000001fe01000, 16);

  loongson_sysconf.ht_control_base = 0x80000EFDFB000000;

  loongson_regaddr_set(loongson_chipcfg, 0x800000001fe00180, 16);

  loongson_regaddr_set(loongson_chiptemp, 0x800000001fe0019c, 16);
  loongson_regaddr_set(loongson_freqctrl, 0x800000001fe001d0, 16);

  loongson_sysconf.io_base_irq = LOONGSON_PCH_IRQ_BASE;
  loongson_sysconf.io_last_irq = LOONGSON_PCH_IRQ_BASE + 256;
  loongson_sysconf.msi_base_irq = LOONGSON_PCI_MSI_IRQ_BASE;
  loongson_sysconf.msi_last_irq = LOONGSON_PCI_MSI_IRQ_BASE + 192;
  loongson_sysconf.msi_address_hi = 0;
  loongson_sysconf.msi_address_lo = 0x2FF00000;
  loongson_sysconf.dma_mask_bits = LOONGSON_DMA_MASK_BIT;
  if (!loongson_sysconf.is_soc_cpu)
    loongson_sysconf.pcie_wake_enabled =
        !(readw(LS7A_PM1_ENA_REG) & ACPI_PCIE_WAKEUP_STATUS);
#endif

  if (list_find(efi_bp))
    duck_printf("Scan bootparm failed\n");
}

#ifdef BMBT
static int init_cpu_fullname(void) {
  int cpu;

  if (loongson_sysconf.cpuname &&
      !strncmp(loongson_sysconf.cpuname, "Loongson", 8)) {
    for (cpu = 0; cpu < NR_CPUS; cpu++) {
      __cpu_full_name[cpu] = loongson_sysconf.cpuname;
    }
  }
  return 0;
}
arch_initcall(init_cpu_fullname);
#endif
