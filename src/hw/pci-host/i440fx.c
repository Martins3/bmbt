/*
 * QEMU i440FX/PIIX3 PCI Bridge Emulation
 *
 * Copyright (c) 2006 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * I440FX chipset data sheet.
 * https://wiki.qemu.org/File:29054901.pdf
 */
#include <environment/device.h>
#include <exec/memory.h>
#include <exec/ram_addr.h>
#include <hw/i386/pc.h>
#include <hw/pci-host/i440fx.h>
#include <hw/pci-host/pam.h>
#include <hw/pci/pci_host.h>
#include <hw/pci/pci_ids.h>
#include <qemu/osdep.h>
#include <qemu/range.h>

typedef struct I440FXState {
  PCIHostState parent_obj;
  Range pci_hole;
  uint64_t pci_hole64_size;
  bool pci_hole64_fix;
  uint32_t short_root_bus;
} I440FXState;

#ifdef BMBT
#define I440FX_PCI_DEVICE(obj)                                                 \
  OBJECT_CHECK(PCII440FXState, (obj), TYPE_I440FX_PCI_DEVICE)
#endif

struct PCII440FXState {
  /*< private >*/
  PCIDevice dev;
  /*< public >*/

#ifdef BMBT
  MemoryRegion *system_memory;
  MemoryRegion *pci_address_space;
  MemoryRegion *ram_memory;
  MemoryRegion smram_region;
  MemoryRegion smram, low_smram;
#endif
  PAMMemoryRegion pam_regions[13];
};

static inline PCII440FXState *I440FX_PCI_DEVICE(PCIDevice *pci) {
  dynamic_type_check(pci, TYPE_I440FX_PCI_DEVICE);
  return (PCII440FXState *)pci;
}

#define I440FX_PAM 0x59
#define I440FX_PAM_SIZE 7
#define I440FX_SMRAM 0x72

/* Keep it 2G to comply with older win32 guests */
#define I440FX_PCI_HOST_HOLE64_SIZE_DEFAULT (1ULL << 31)

/* Older coreboot versions (4.0 and older) read a config register that doesn't
 * exist in real hardware, to get the RAM size from QEMU.
 */
#define I440FX_COREBOOT_RAM_SIZE 0x57

static void i440fx_update_memory_mappings(PCII440FXState *d) {
  int i;
  PCIDevice *pd = PCI_DEVICE(d);

  for (i = 0; i < ARRAY_SIZE(d->pam_regions); i++) {
    pam_update(&d->pam_regions[i], i,
               pd->config[I440FX_PAM + DIV_ROUND_UP(i, 2)]);
  }

  smram_region_set_enabled(!(pd->config[I440FX_SMRAM] & SMRAM_D_OPEN));
  smram_set_enabled(pd->config[I440FX_SMRAM] & SMRAM_G_SMRAME);

  tcg_commit();
#ifdef BMBT
  memory_region_set_enabled(&d->smram_region,
                            !(pd->config[I440FX_SMRAM] & SMRAM_D_OPEN));
  memory_region_set_enabled(&d->smram,
                            pd->config[I440FX_SMRAM] & SMRAM_G_SMRAME);
#endif
}

static void i440fx_write_config(PCIDevice *dev, uint32_t address, uint32_t val,
                                int len) {
  PCII440FXState *d = I440FX_PCI_DEVICE(dev);

  /* XXX: implement SMRAM.D_LOCK */
  pci_default_write_config(dev, address, val, len);
  if (ranges_overlap(address, len, I440FX_PAM, I440FX_PAM_SIZE) ||
      range_covers_byte(address, len, I440FX_SMRAM)) {
    i440fx_update_memory_mappings(d);
  }
}

#ifdef BMBT
static int i440fx_post_load(void *opaque, int version_id) {
  PCII440FXState *d = opaque;

  i440fx_update_memory_mappings(d);
  return 0;
}

static const VMStateDescription vmstate_i440fx = {
    .name = "I440FX",
    .version_id = 3,
    .minimum_version_id = 3,
    .post_load = i440fx_post_load,
    .fields = (VMStateField[]){
        VMSTATE_PCI_DEVICE(parent_obj, PCII440FXState),
        /* Used to be smm_enabled, which was basically always zero because
         * SeaBIOS hardly uses SMM.  SMRAM is now handled by CPU code.
         */
        VMSTATE_UNUSED(1), VMSTATE_END_OF_LIST()}};

static uint i440fx_pcihost_get_pci_hole_start(I440FXState *s) {
  // I440FXState *s = I440FX_PCI_HOST_BRIDGE(obj);
  uint64_t val64;
  uint32_t value;

  val64 = range_is_empty(&s->pci_hole) ? 0 : range_lob(&s->pci_hole);
  value = val64;
  assert(value == val64);
  // visit_type_uint32(v, name, &value, errp);
  return value;
}

static uint i440fx_pcihost_get_pci_hole_end(I440FXState *s, const char *name) {
  // I440FXState *s = I440FX_PCI_HOST_BRIDGE(obj);
  uint64_t val64;
  uint32_t value;

  val64 = range_is_empty(&s->pci_hole) ? 0 : range_upb(&s->pci_hole) + 1;
  value = val64;
  assert(value == val64);
  // visit_type_uint32(v, name, &value, errp);
  return value;
}

/*
 * The 64bit PCI hole start is set by the Guest firmware
 * as the address of the first 64bit PCI MEM resource.
 * If no PCI device has resources on the 64bit area,
 * the 64bit PCI hole will start after "over 4G RAM" and the
 * reserved space for memory hotplug if any.
 */
static uint64_t i440fx_pcihost_get_pci_hole64_start_value(I440FXState *s) {
  PCIHostState *h = &s->parent_obj;
  Range w64;
  uint64_t value;

  pci_bus_get_w64_range(h->bus, &w64);
  value = range_is_empty(&w64) ? 0 : range_lob(&w64);
  if (!value && s->pci_hole64_fix) {
    value = pc_pci_hole64_start();
  }
  return value;
}

static uint64_t i440fx_pcihost_get_pci_hole64_start(I440FXState *s) {
  uint64_t hole64_start = i440fx_pcihost_get_pci_hole64_start_value(s);

  return hole64_start;
  // visit_type_uint64(v, name, &hole64_start, errp);
}

/*
 * The 64bit PCI hole end is set by the Guest firmware
 * as the address of the last 64bit PCI MEM resource.
 * Then it is expanded to the PCI_HOST_PROP_PCI_HOLE64_SIZE
 * that can be configured by the user.
 */
static uint64_t i440fx_pcihost_get_pci_hole64_end(I440FXState *s) {
  PCIHostState *h = &s->parent_obj;
  uint64_t hole64_start = i440fx_pcihost_get_pci_hole64_start_value(s);
  Range w64;
  uint64_t value, hole64_end;

  pci_bus_get_w64_range(h->bus, &w64);
  value = range_is_empty(&w64) ? 0 : range_upb(&w64) + 1;
  hole64_end = ROUND_UP(hole64_start + s->pci_hole64_size, 1ULL << 30);
  if (s->pci_hole64_fix && value < hole64_end) {
    value = hole64_end;
  }
  return value;
  // visit_type_uint64(v, name, &value, errp);
}
#endif

static void i440fx_pcihost_initfn(PCIHostState *s) {
  // PCIHostState *s = PCI_HOST_BRIDGE(obj);

  memory_region_init_io(&s->conf_mem, &pci_host_conf_le_ops, s, "pci-conf-idx",
                        4);
  memory_region_init_io(&s->data_mem, &pci_host_data_le_ops, s, "pci-conf-data",
                        4);

#ifdef BMBT
  object_property_add(obj, PCI_HOST_PROP_PCI_HOLE_START, "uint32",
                      i440fx_pcihost_get_pci_hole_start, NULL, NULL, NULL,
                      NULL);

  object_property_add(obj, PCI_HOST_PROP_PCI_HOLE_END, "uint32",
                      i440fx_pcihost_get_pci_hole_end, NULL, NULL, NULL, NULL);

  object_property_add(obj, PCI_HOST_PROP_PCI_HOLE64_START, "uint64",
                      i440fx_pcihost_get_pci_hole64_start, NULL, NULL, NULL,
                      NULL);

  object_property_add(obj, PCI_HOST_PROP_PCI_HOLE64_END, "uint64",
                      i440fx_pcihost_get_pci_hole64_end, NULL, NULL, NULL,
                      NULL);
#endif
}

static void i440fx_pcihost_realize(PCIHostState *s) {
#ifdef BMBT
  PCIHostState *s = PCI_HOST_BRIDGE(dev);
  SysBusDevice *sbd = SYS_BUS_DEVICE(dev);

  sysbus_add_io(sbd, 0xcf8, &s->conf_mem);
  sysbus_init_ioports(sbd, 0xcf8, 4);

  sysbus_add_io(sbd, 0xcfc, &s->data_mem);
  sysbus_init_ioports(sbd, 0xcfc, 4);

  /* register i440fx 0xcf8 port as coalesced pio */
  memory_region_set_flush_coalesced(&s->data_mem);
  memory_region_add_coalescing(&s->conf_mem, 0, 4);
#endif
  io_add_memory_region(0xcf8, &s->conf_mem);
  io_add_memory_region(0xcfc, &s->data_mem);
}

static void i440fx_realize(PCIDevice *dev) {
  dev->config[I440FX_SMRAM] = 0x02;

#ifdef BMBT
  if (object_property_get_bool(qdev_get_machine(), "iommu", NULL)) {
    warn_report("i440fx doesn't support emulated iommu");
  }
#endif
}

I440FXState __pci_host_state;
PCIHostState *QOM_init_pci_host_bridge() {
  I440FXState *pci_host_state = &__pci_host_state;
  i440fx_pcihost_initfn(&pci_host_state->parent_obj);
  // i440fx_pcihost_class_init();
  pci_host_state->pci_hole64_size = I440FX_PCI_HOST_HOLE64_SIZE_DEFAULT;
  pci_host_state->short_root_bus = 0;
  pci_host_state->pci_hole64_fix = true;
  return &pci_host_state->parent_obj;
}

PCIBus *i440fx_init(const char *host_type, const char *pci_type,
                    PCII440FXState **pi440fx_state,
                    MemoryRegion *address_space_mem,
                    MemoryRegion *address_space_io, ram_addr_t ram_size,
                    ram_addr_t below_4g_mem_size, ram_addr_t above_4g_mem_size,
                    MemoryRegion *pci_address_space, MemoryRegion *ram_memory) {
  // DeviceState *dev;
  PCIBus *b;
  PCIDevice *d;
  PCIHostState *s;
  PCII440FXState *f;
  // unsigned i;
  // I440FXState *i440fx;

  // host_type : i440FX-pcihost
  // pci_type  : i440FX

#ifdef BMBT
  dev = qdev_create(NULL, host_type);
  s = PCI_HOST_BRIDGE(dev);
#endif
  s = QOM_init_pci_host_bridge();
  b = pci_root_bus_new(NULL, NULL, pci_address_space, address_space_io, 0,
                       TYPE_PCI_BUS);
#ifdef BMBT
  object_property_add_child(qdev_get_machine(), "i440fx", OBJECT(dev), NULL);
  qdev_init_nofail(dev);
#endif
  // [interface 42]
  i440fx_pcihost_realize(s);

  s->bus = b;

  d = pci_create_simple(b, 0, pci_type);
  *pi440fx_state = I440FX_PCI_DEVICE(d);
  f = *pi440fx_state;
#ifdef BMBT
  f->system_memory = address_space_mem;
  f->pci_address_space = pci_address_space;
  f->ram_memory = ram_memory;

  i440fx = I440FX_PCI_HOST_BRIDGE(dev);
  range_set_bounds(&i440fx->pci_hole, below_4g_mem_size,
                   IO_APIC_DEFAULT_ADDRESS - 1);

  /* setup pci memory mapping */
  pc_pci_as_mapping_init(OBJECT(f), f->system_memory, f->pci_address_space);

  /* if *disabled* show SMRAM to all CPUs */
  memory_region_init_alias(&f->smram_region, OBJECT(d), "smram-region",
                           f->pci_address_space, 0xa0000, 0x20000);
  memory_region_add_subregion_overlap(f->system_memory, 0xa0000,
                                      &f->smram_region, 1);
  memory_region_set_enabled(&f->smram_region, true);

  /* smram, as seen by SMM CPUs */
  memory_region_init(&f->smram, OBJECT(d), "smram", 1ull << 32);
  memory_region_set_enabled(&f->smram, true);
  memory_region_init_alias(&f->low_smram, OBJECT(d), "smram-low", f->ram_memory,
                           0xa0000, 0x20000);
  memory_region_set_enabled(&f->low_smram, true);
  memory_region_add_subregion(&f->smram, 0xa0000, &f->low_smram);
  object_property_add_const_link(qdev_get_machine(), "smram", OBJECT(&f->smram),
                                 &error_abort);
#endif

#ifdef BMBT
  init_pam(dev, f->ram_memory, f->system_memory, f->pci_address_space,
           &f->pam_regions[0], PAM_BIOS_BASE, PAM_BIOS_SIZE);
  for (i = 0; i < ARRAY_SIZE(f->pam_regions) - 1; ++i) {
    init_pam(dev, f->ram_memory, f->system_memory, f->pci_address_space,
             &f->pam_regions[i + 1], PAM_EXPAN_BASE + i * PAM_EXPAN_SIZE,
             PAM_EXPAN_SIZE);
  }
#endif
  init_pam(&f->pam_regions[0], get_ram_mr(PAM_BIOS_INDEX));
  for (int i = 0; i < PAM_EXPAN_NUM + PAM_EXBIOS_NUM; ++i) {
    init_pam(&f->pam_regions[i + 1], get_ram_mr(PAM_INDEX + i));
  }

  ram_size = ram_size / 8 / 1024 / 1024;
  if (ram_size > 255) {
    ram_size = 255;
  }
  d->config[I440FX_COREBOOT_RAM_SIZE] = ram_size;

  i440fx_update_memory_mappings(f);

  if (pci_pass_through) {
    pci_pass_through_init();
  }

  return b;
}

// used by hotplug
#ifdef BMBT
PCIBus *find_i440fx(void) {
  PCIHostState *s =
      OBJECT_CHECK(PCIHostState, object_resolve_path("/machine/i440fx", NULL),
                   TYPE_PCI_HOST_BRIDGE);
  return s ? s->bus : NULL;
}
#endif

static void i440fx_class_init(PCIDeviceClass *k) {
  k->realize = i440fx_realize;
  k->config_write = i440fx_write_config;
  k->vendor_id = PCI_VENDOR_ID_INTEL;
  k->device_id = PCI_DEVICE_ID_INTEL_82441;
  k->revision = 0x02;
  k->class_id = PCI_CLASS_BRIDGE_HOST;
#ifdef BMBT
  dc->desc = "Host bridge";
  dc->vmsd = &vmstate_i440fx;
  /*
   * PCI-facing part of the host bridge, not usable without the
   * host-facing part, which can't be device_add'ed, yet.
   */
  dc->user_creatable = false;
  dc->hotpluggable = false;
#endif
}

static PCII440FXState __pci_i440fx;
static PCIDeviceClass __pci_i440fx_class;
PCII440FXState *QOM_init_pci_i440fx(PCIBus *bus) {
  PCII440FXState *i440fx = &__pci_i440fx;
  PCIDeviceClass *c = &__pci_i440fx_class;
  strcpy(i440fx->dev.type, TYPE_I440FX_PCI_DEVICE);
  PCI_DEVICE_SET_CLASS(&i440fx->dev, c);
  i440fx_class_init(c);
  i440fx->dev.bus = bus;
  return i440fx;
}

bool pam_type_is_pci(hwaddr offset) {
  unsigned idx = -1;
  if (offset >= PAM_BIOS_BASE && offset < PAM_BIOS_BASE + PAM_BIOS_SIZE) {
    idx = 0;
  } else if (offset >= PAM_EXPAN_BASE && offset < PAM_BIOS_BASE) {
    idx = (offset - PAM_EXPAN_BASE) / PAM_EXPAN_SIZE + 1;
  } else {
    g_assert_not_reached();
  }

  return __pci_i440fx.pam_regions[idx].current == PAM_PCI;
}

#ifdef BMBT
static const TypeInfo i440fx_info = {
    .name = TYPE_I440FX_PCI_DEVICE,
    .parent = TYPE_PCI_DEVICE,
    .instance_size = sizeof(PCII440FXState),
    .class_init = i440fx_class_init,
    .interfaces =
        (InterfaceInfo[]){
            {INTERFACE_CONVENTIONAL_PCI_DEVICE},
            {},
        },
};

/* IGD Passthrough Host Bridge. */
typedef struct {
  uint8_t offset;
  uint8_t len;
} IGDHostInfo;

/* Here we just expose minimal host bridge offset subset. */
static const IGDHostInfo igd_host_bridge_infos[] = {
    {0x08, 2}, /* revision id */
    {0x2c, 2}, /* sybsystem vendor id */
    {0x2e, 2}, /* sybsystem id */
    {0x50, 2}, /* SNB: processor graphics control register */
    {0x52, 2}, /* processor graphics control register */
    {0xa4, 4}, /* SNB: graphics base of stolen memory */
    {0xa8, 4}, /* SNB: base of GTT stolen memory */
};

static void host_pci_config_read(int pos, int len, uint32_t *val,
                                 Error **errp) {
  int rc, config_fd;
  /* Access real host bridge. */
  char *path = g_strdup_printf("/sys/bus/pci/devices/%04x:%02x:%02x.%d/%s", 0,
                               0, 0, 0, "config");

  config_fd = open(path, O_RDWR);
  if (config_fd < 0) {
    error_setg_errno(errp, errno, "Failed to open: %s", path);
    goto out;
  }

  if (lseek(config_fd, pos, SEEK_SET) != pos) {
    error_setg_errno(errp, errno, "Failed to seek: %s", path);
    goto out_close_fd;
  }

  do {
    rc = read(config_fd, (uint8_t *)val, len);
  } while (rc < 0 && (errno == EINTR || errno == EAGAIN));
  if (rc != len) {
    error_setg_errno(errp, errno, "Failed to read: %s", path);
  }

out_close_fd:
  close(config_fd);
out:
  g_free(path);
}

static void igd_pt_i440fx_realize(PCIDevice *pci_dev, Error **errp) {
  uint32_t val = 0;
  int i, num;
  int pos, len;
  Error *local_err = NULL;

  num = ARRAY_SIZE(igd_host_bridge_infos);
  for (i = 0; i < num; i++) {
    pos = igd_host_bridge_infos[i].offset;
    len = igd_host_bridge_infos[i].len;
    host_pci_config_read(pos, len, &val, &local_err);
    if (local_err) {
      error_propagate(errp, local_err);
      return;
    }
    pci_default_write_config(pci_dev, pos, val, len);
  }
}

static void igd_passthrough_i440fx_class_init(ObjectClass *klass, void *data) {
  DeviceClass *dc = DEVICE_CLASS(klass);
  PCIDeviceClass *k = PCI_DEVICE_CLASS(klass);

  k->realize = igd_pt_i440fx_realize;
  dc->desc = "IGD Passthrough Host bridge";
}

static const TypeInfo igd_passthrough_i440fx_info = {
    .name = TYPE_IGD_PASSTHROUGH_I440FX_PCI_DEVICE,
    .parent = TYPE_I440FX_PCI_DEVICE,
    .instance_size = sizeof(PCII440FXState),
    .class_init = igd_passthrough_i440fx_class_init,
};

static const char *i440fx_pcihost_root_bus_path(PCIHostState *host_bridge,
                                                PCIBus *rootbus) {
  I440FXState *s = I440FX_PCI_HOST_BRIDGE(host_bridge);

  /* For backwards compat with old device paths */
  if (s->short_root_bus) {
    return "0000";
  }
  return "0000:00";
}

static Property i440fx_props[] = {
    DEFINE_PROP_SIZE(PCI_HOST_PROP_PCI_HOLE64_SIZE, I440FXState,
                     pci_hole64_size, I440FX_PCI_HOST_HOLE64_SIZE_DEFAULT),
    DEFINE_PROP_UINT32("short_root_bus", I440FXState, short_root_bus, 0),
    DEFINE_PROP_BOOL("x-pci-hole64-fix", I440FXState, pci_hole64_fix, true),
    DEFINE_PROP_END_OF_LIST(),
};

static void i440fx_pcihost_class_init(ObjectClass *klass, void *data) {
  DeviceClass *dc = DEVICE_CLASS(klass);
  PCIHostBridgeClass *hc = PCI_HOST_BRIDGE_CLASS(klass);

  hc->root_bus_path = i440fx_pcihost_root_bus_path;
  dc->realize = i440fx_pcihost_realize;
  dc->fw_name = "pci";
  dc->props = i440fx_props;
  /* Reason: needs to be wired up by pc_init1 */
  dc->user_creatable = false;
}

static const TypeInfo i440fx_pcihost_info = {
    .name = TYPE_I440FX_PCI_HOST_BRIDGE,
    .parent = TYPE_PCI_HOST_BRIDGE,
    .instance_size = sizeof(I440FXState),
    .instance_init = i440fx_pcihost_initfn,
    .class_init = i440fx_pcihost_class_init,
};

static void i440fx_register_types(void) {
  type_register_static(&i440fx_info);
  type_register_static(&igd_passthrough_i440fx_info);
  type_register_static(&i440fx_pcihost_info);
}

type_init(i440fx_register_types)
#endif
