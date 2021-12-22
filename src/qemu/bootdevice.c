#include <assert.h>
#include <hw/boards.h>
#include <hw/qdev-core.h>
#include <qemu/error-report.h>
#include <qemu/queue.h>
#include <stdint.h>
#include <stdlib.h>
#include <sysemu/sysemu.h>
#include <uglib.h>

typedef struct FWBootEntry FWBootEntry;

struct FWBootEntry {
  QTAILQ_ENTRY(FWBootEntry) link;
  int32_t bootindex;
  DeviceState *dev;
  char *suffix;
};

static QTAILQ_HEAD(, FWBootEntry)
    fw_boot_order = QTAILQ_HEAD_INITIALIZER(fw_boot_order);

void del_boot_device_path(DeviceState *dev, const char *suffix) {
  FWBootEntry *i;

  if (dev == NULL) {
    return;
  }

  QTAILQ_FOREACH(i, &fw_boot_order, link) {
    if ((!suffix || !g_strcmp0(i->suffix, suffix)) && i->dev == dev) {
      QTAILQ_REMOVE(&fw_boot_order, i, link);
      g_free(i->suffix);
      g_free(i);

      break;
    }
  }
}

void add_boot_device_path(int32_t bootindex, DeviceState *dev,
                          const char *suffix) {
  FWBootEntry *node, *i;

  if (bootindex < 0) {
    del_boot_device_path(dev, suffix);
    return;
  }

  assert(dev != NULL || suffix != NULL);

  del_boot_device_path(dev, suffix);

  node = g_malloc0(sizeof(FWBootEntry));
  node->bootindex = bootindex;
  node->suffix = g_strdup(suffix);
  node->dev = dev;

  QTAILQ_FOREACH(i, &fw_boot_order, link) {
    if (i->bootindex == bootindex) {
      error_report("Two devices with same boot index %d", bootindex);
      exit(1);
    } else if (i->bootindex < bootindex) {
      continue;
    }
    QTAILQ_INSERT_BEFORE(i, node, link);
    return;
  }
  QTAILQ_INSERT_TAIL(&fw_boot_order, node, link);
}

static char *get_boot_device_path(DeviceState *dev, bool ignore_suffixes,
                                  const char *suffix) {
  // char *d;
  char *devpath = NULL, *s = NULL, *bootpath;
  duck_check(dev == NULL);

  if (dev) {
#ifdef BMBT
    devpath = qdev_get_fw_dev_path(dev);
    assert(devpath);
#endif
  }

  if (!ignore_suffixes) {
    if (dev) {
#ifdef BMBT
      d = qdev_get_own_fw_dev_path_from_handler(dev->parent_bus, dev);
      if (d) {
        assert(!suffix);
        s = d;
      } else {
        s = g_strdup(suffix);
      }
#endif
    } else {
      s = g_strdup(suffix);
    }
  }

  bootpath = g_strdup_printf("%s%s", devpath ? devpath : "", s ? s : "");
  g_free(devpath);
  g_free(s);

  return bootpath;
}

/*
 * This function returns null terminated string that consist of new line
 * separated device paths.
 *
 * memory pointed by "size" is assigned total length of the array in bytes
 *
 */
char *get_boot_devices_list(size_t *size) {
  FWBootEntry *i;
  size_t total = 0;
  char *list = NULL;
  MachineClass *mc = MACHINE_GET_CLASS(qdev_get_machine());
  bool ignore_suffixes = mc->ignore_boot_device_suffixes;

  QTAILQ_FOREACH(i, &fw_boot_order, link) {
    char *bootpath;
    size_t len;

    bootpath = get_boot_device_path(i->dev, ignore_suffixes, i->suffix);

    if (total) {
      list[total - 1] = '\n';
    }
    len = strlen(bootpath) + 1;
    list = g_realloc(list, total + len);
    memcpy(&list[total], bootpath, len);
    total += len;
    g_free(bootpath);
  }

  *size = total;

  if (boot_strict && *size > 0) {
    list[total - 1] = '\n';
    list = g_realloc(list, total + 5);
    memcpy(&list[total], "HALT", 5);
    *size = total + 5;
  }
  return list;
}
