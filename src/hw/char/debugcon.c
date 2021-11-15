#include <exec/hwaddr.h>
#include <exec/memory.h>
#include <hw/isa/isa.h>
#include <qemu/log.h>

#define TYPE_ISA_DEBUGCON_DEVICE "isa-debugcon"

// #define DEBUG_DEBUGCON

typedef struct DebugconState {
  MemoryRegion io;
  // CharBackend chr;
  FILE *log;
  uint32_t readback;
} DebugconState;

typedef struct ISADebugconState {
  ISADevice parent_obj;

  uint32_t iobase;
  DebugconState state;
} ISADebugconState;

static void debugcon_ioport_write(void *opaque, hwaddr addr, uint64_t val,
                                  unsigned width) {
  DebugconState *s = opaque;
  unsigned char ch = val;

#ifdef DEBUG_DEBUGCON
  printf(" [debugcon: write addr=0x%04" HWADDR_PRIx " val=0x%02" PRIx64 "]\n",
         addr, val);
#endif

#ifdef BMBT
  /* XXX this blocks entire thread. Rewrite to use
   * qemu_chr_fe_write and background I/O callbacks */
  qemu_chr_fe_write_all(&s->chr, &ch, 1);
#endif
  assert(fprintf(s->log, "%c", ch) == 1);
  fflush(s->log);
}

static uint64_t debugcon_ioport_read(void *opaque, hwaddr addr,
                                     unsigned width) {
  DebugconState *s = opaque;

  g_assert_not_reached();

#ifdef DEBUG_DEBUGCON
  printf("debugcon: read addr=0x%04" HWADDR_PRIx "\n", addr);
#endif

  return s->readback;
}

static const MemoryRegionOps debugcon_ops = {
    .read = debugcon_ioport_read,
    .write = debugcon_ioport_write,
    .valid.min_access_size = 1,
    .valid.max_access_size = 1,
    .endianness = DEVICE_LITTLE_ENDIAN,
};

#ifdef BMBT
static void debugcon_realize_core(DebugconState *s, Error **errp) {
  if (!qemu_chr_fe_backend_connected(&s->chr)) {
    error_setg(errp, "Can't create debugcon device, empty char device");
    return;
  }

  qemu_chr_fe_set_handlers(&s->chr, NULL, NULL, NULL, NULL, s, NULL, true);
}
#endif

static void debugcon_isa_realizefn(ISADebugconState *isa) {
#ifdef BMBT
  ISADevice *d = ISA_DEVICE(dev);
  ISADebugconState *isa = ISA_DEBUGCON_DEVICE(dev);
  DebugconState *s = &isa->state;
  Error *err = NULL;

  debugcon_realize_core(s, &err);
  if (err != NULL) {
    error_propagate(errp, err);
    return;
  }
  memory_region_init_io(&s->io, OBJECT(dev), &debugcon_ops, s,
                        TYPE_ISA_DEBUGCON_DEVICE, 1);
  memory_region_add_subregion(isa_address_space_io(d), isa->iobase, &s->io);
#endif
  isa->state.log = get_logfile("seabios.log");

  DebugconState *s = &isa->state;
  memory_region_init_io(&s->io, &debugcon_ops, s, TYPE_ISA_DEBUGCON_DEVICE, 1);
  io_add_memory_region(isa->iobase, &s->io);
  // memory_region_add_subregion(isa_address_space_io(d), isa->iobase, &s->io);
}

static ISADebugconState __isa;
void QOM_init_debugcon() {
  ISADebugconState *isa = &__isa;
  isa->iobase = 0x402;
  isa->state.readback = 0xe9;
  debugcon_isa_realizefn(isa);
}

#ifdef BMBT
static Property debugcon_isa_properties[] = {
    DEFINE_PROP_UINT32("iobase", ISADebugconState, iobase, 0xe9),
    DEFINE_PROP_CHR("chardev", ISADebugconState, state.chr),
    DEFINE_PROP_UINT32("readback", ISADebugconState, state.readback, 0xe9),
    DEFINE_PROP_END_OF_LIST(),
};

static void debugcon_isa_class_initfn(ObjectClass *klass, void *data) {
  DeviceClass *dc = DEVICE_CLASS(klass);

  dc->realize = debugcon_isa_realizefn;
  dc->props = debugcon_isa_properties;
  set_bit(DEVICE_CATEGORY_MISC, dc->categories);
}

static const TypeInfo debugcon_isa_info = {
    .name = TYPE_ISA_DEBUGCON_DEVICE,
    .parent = TYPE_ISA_DEVICE,
    .instance_size = sizeof(ISADebugconState),
    .class_init = debugcon_isa_class_initfn,
};

static void debugcon_register_types(void) {
  type_register_static(&debugcon_isa_info);
}

type_init(debugcon_register_types)
#endif
