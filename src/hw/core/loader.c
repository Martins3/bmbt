#include <hw/nvram/fw_cfg.h>
#include <errno.h>
#include <file.h>
#include <qemu/error-report.h>
#include <sysemu/reset.h>

static FWCfgState *fw_cfg;
static QTAILQ_HEAD(, Rom) roms = QTAILQ_HEAD_INITIALIZER(roms);
static int roms_loaded;

typedef struct Rom Rom;

struct Rom {
  char *name;
  char *path;

  /* datasize is the amount of memory allocated in "data". If datasize is less
   * than romsize, it means that the area from datasize to romsize is filled
   * with zeros.
   */
  size_t romsize;
  size_t datasize;

  uint8_t *data;
  MemoryRegion *mr;
  AddressSpace *as;
  int isrom;
  char *fw_dir;
  char *fw_file;
  GMappedFile *mapped_file;

  bool committed;

  hwaddr addr;
  QTAILQ_ENTRY(Rom) next;
};

void rom_set_fw(FWCfgState *f) { fw_cfg = f; }

static void rom_reset(void *unused) {
  Rom *rom;
#ifdef BMBT
  /*
   * We don't need to fill in the RAM with ROM data because we'll fill
   * the data in during the next incoming migration in all cases.  Note
   * that some of those RAMs can actually be modified by the guest on ARM
   * so this is probably the only right thing to do here.
   */
  if (runstate_check(RUN_STATE_INMIGRATE))
    return;
#endif

  QTAILQ_FOREACH(rom, &roms, next) {
    if (rom->fw_file) {
      continue;
    }
    g_assert_not_reached();
#ifdef BMBT
    if (rom->data == NULL) {
      continue;
    }
    if (rom->mr) {
      void *host = memory_region_get_ram_ptr(rom->mr);
      memcpy(host, rom->data, rom->datasize);
    } else {
      address_space_write_rom(rom->as, rom->addr, MEMTXATTRS_UNSPECIFIED,
                              rom->data, rom->datasize);
    }
    if (rom->isrom) {
      /* rom needs to be written only once */
      rom_free_data(rom);
    }
    /*
     * The rom loader is really on the same level as firmware in the guest
     * shadowing a ROM into RAM. Such a shadowing mechanism needs to ensure
     * that the instruction cache for that new region is clear, so that the
     * CPU definitely fetches its instructions from the just written data.
     */
    cpu_flush_icache_range(rom->addr, rom->datasize);

    trace_loader_write_rom(rom->name, rom->addr, rom->datasize, rom->isrom);
#endif
  }
}

int rom_check_and_register_reset(void) {
  // hwaddr addr = 0;
  // MemoryRegionSection section;
  Rom *rom;
  // AddressSpace *as = NULL;

  QTAILQ_FOREACH(rom, &roms, next) {
    if (rom->fw_file) {
      continue;
    }
#ifdef BMBT
    if (!rom->mr) {
      if ((addr > rom->addr) && (as == rom->as)) {
        fprintf(stderr,
                "rom: requested regions overlap "
                "(rom %s. free=0x" TARGET_FMT_plx ", addr=0x" TARGET_FMT_plx
                ")\n",
                rom->name, addr, rom->addr);
        return -1;
      }
      addr = rom->addr;
      addr += rom->romsize;
      as = rom->as;
    }
    section = memory_region_find(rom->mr ? rom->mr : get_system_memory(),
                                 rom->addr, 1);
    rom->isrom = int128_nz(section.size) && memory_region_is_rom(section.mr);
    memory_region_unref(section.mr);
#endif
  }
  qemu_register_reset(rom_reset, NULL);
  roms_loaded = 1;
  return 0;
}

/*
 * rom->data can be heap-allocated or memory-mapped (e.g. when added with
 * rom_add_elf_program())
 */
static void rom_free_data(Rom *rom) {
  if (rom->mapped_file) {
    g_assert_not_reached();
    // g_mapped_file_unref(rom->mapped_file);
    rom->mapped_file = NULL;
  } else {
    g_free(rom->data);
  }

  rom->data = NULL;
}

static void rom_free(Rom *rom) {
  rom_free_data(rom);
  g_free(rom->path);
  g_free(rom->name);
  g_free(rom->fw_dir);
  g_free(rom->fw_file);
  g_free(rom);
}

static inline bool rom_order_compare(Rom *rom, Rom *item) {
  return ((uintptr_t)(void *)rom->as > (uintptr_t)(void *)item->as) ||
         (rom->as == item->as && rom->addr >= item->addr);
}

static void rom_insert(Rom *rom) {
  Rom *item;

  if (roms_loaded) {
    error_report("ROM images must be loaded at startup\n");
  }

  /* The user didn't specify an address space, this is the default */
  if (!rom->as) {
    rom->as = &address_space_memory;
  }

  rom->committed = false;

  /* List is ordered by load address in the same address space */
  QTAILQ_FOREACH(item, &roms, next) {
    if (rom_order_compare(rom, item)) {
      continue;
    }
    QTAILQ_INSERT_BEFORE(item, rom, next);
    return;
  }
  QTAILQ_INSERT_TAIL(&roms, rom, next);
}

static char *get_image_path(const char *file) {
  const int bufsize = 100;
  char *buf = g_malloc0(bufsize * sizeof(char));
  assert(snprintf(buf, bufsize, "image/%s", file) < bufsize);
  return buf;
}

int rom_add_file(const char *file, const char *fw_dir, hwaddr addr,
                 int32_t bootindex, bool option_rom, MemoryRegion *mr,
                 AddressSpace *as) {
  MachineClass *mc = MACHINE_GET_CLASS(qdev_get_machine());
  Rom *rom;
  int rc;
  FILE *f;
  char devpath[100];

  if (as && mr) {
    fprintf(stderr, "Specifying an Address Space and Memory Region is "
                    "not valid when loading a rom\n");
    /* We haven't allocated anything so we don't need any cleanup */
    return -1;
  }

  rom = g_malloc0(sizeof(*rom));
  rom->name = g_strdup(file);
  rom->path = get_image_path(file);
  rom->as = as;
  if (rom->path == NULL) {
    rom->path = g_strdup(file);
  }

  f = fopen(rom->path, "rb");
  if (f == NULL) {
    fprintf(stderr, "Could not open option rom '%s': %s\n", rom->path,
            strerror(errno));
    goto err;
  }

  if (fw_dir) {
    rom->fw_dir = g_strdup(fw_dir);
    rom->fw_file = g_strdup(file);
  }
  rom->addr = addr;
  rom->romsize = get_file_size(f);
  if (rom->romsize == -1) {
    fprintf(stderr, "rom: file %-20s: get size error: %s\n", rom->name,
            strerror(errno));
    goto err;
  }

  rom->datasize = rom->romsize;
  rom->data = g_malloc0(rom->datasize);
  rc = fread(rom->data, sizeof(char), rom->datasize, f);
  if (rc != rom->datasize) {
    fprintf(stderr, "rom: file %-20s: read error: rc=%d (expected %zd)\n",
            rom->name, rc, rom->datasize);
    goto err;
  }
  fclose(f);
  rom_insert(rom);
  if (rom->fw_file && fw_cfg) {
    const char *basename;
    char fw_file_name[FW_CFG_MAX_FILE_PATH];
    void *data;

    basename = strrchr(rom->fw_file, '/');
    if (basename) {
      basename++;
    } else {
      basename = rom->fw_file;
    }
    snprintf(fw_file_name, sizeof(fw_file_name), "%s/%s", rom->fw_dir,
             basename);
    snprintf(devpath, sizeof(devpath), "/rom@%s", fw_file_name);

    if ((!option_rom || mc->option_rom_has_mr) && mc->rom_file_has_mr) {
      g_assert_not_reached();
      // data = rom_set_mr(rom, OBJECT(fw_cfg), devpath, true);
    } else {
      data = rom->data;
    }

    fw_cfg_add_file(fw_cfg, fw_file_name, data, rom->romsize);
  } else {
    if (mr) {
      rom->mr = mr;
      snprintf(devpath, sizeof(devpath), "/rom@%s", file);
    } else {
      snprintf(devpath, sizeof(devpath), "/rom@" TARGET_FMT_plx, addr);
    }
  }

  add_boot_device_path(bootindex, NULL, devpath);
  return 0;

err:
  if (f != NULL)
    fclose(f);

  rom_free(rom);
  return -1;
}

int rom_add_option(const char *file, int32_t bootindex) {
  return rom_add_file(file, "genroms", 0, bootindex, true, NULL, NULL);
}
