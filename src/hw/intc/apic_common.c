#include "../../../include/hw/i386/apic.h"
#include <string.h>

static int apic_irq_delivered;
bool apic_report_tpr_access;

void cpu_set_apic_base(APICCommonState *s, uint64_t val) {
  // bmbt_trace_cpu_set_apic_base(val);

  if (s) {
    APICCommonClass *info = APIC_COMMON_GET_CLASS(s);
    /* switching to x2APIC, reset possibly modified xAPIC ID */
    if (!(s->apicbase & MSR_IA32_APICBASE_EXTD) &&
        (val & MSR_IA32_APICBASE_EXTD)) {
      s->id = s->initial_apic_id;
    }
    info->set_base(s, val);
  }
}

uint64_t cpu_get_apic_base(APICCommonState *s) {
  if (s) {
    // bmbt_trace_cpu_get_apic_base((uint64_t)s->apicbase);
    return s->apicbase;
  } else {
    // bmbt_trace_cpu_get_apic_base(MSR_IA32_APICBASE_BSP);
    return MSR_IA32_APICBASE_BSP;
  }
}

void cpu_set_apic_tpr(APICCommonState *s, uint8_t val) {
  APICCommonClass *info;

  if (!s) {
    return;
  }
  info = APIC_COMMON_GET_CLASS(s);

  info->set_tpr(s, val);
}

uint8_t cpu_get_apic_tpr(APICCommonState *s) {
  APICCommonClass *info;

  if (!s) {
    return 0;
  }
  info = APIC_COMMON_GET_CLASS(s);

  return info->get_tpr(s);
}

void apic_enable_tpr_access_reporting(APICCommonState *s, bool enable) {
  APICCommonClass *info = APIC_COMMON_GET_CLASS(s);

  apic_report_tpr_access = enable;
  if (info->enable_tpr_reporting) {
    info->enable_tpr_reporting(s, enable);
  }
}

void apic_enable_vapic(APICCommonState *s, hwaddr paddr) {
  APICCommonClass *info = APIC_COMMON_GET_CLASS(s);

  s->vapic_paddr = paddr;
  info->vapic_base_update(s);
}

void apic_report_irq_delivered(int delivered) {
  apic_irq_delivered += delivered;

  // bmbt_trace_apic_report_irq_delivered(apic_irq_delivered);
}

void apic_reset_irq_delivered(void) {
  /* Copy this into a local variable to encourage gcc to emit a plain
   * register for a sys/sdt.h marker.  For details on this workaround, see:
   * https://sourceware.org/bugzilla/show_bug.cgi?id=13296
   */
  // volatile int a_i_d = apic_irq_delivered;
  // bmbt_trace_apic_reset_irq_delivered(a_i_d);

  apic_irq_delivered = 0;
}

int apic_get_irq_delivered(void) {
  // bmbt_trace_apic_get_irq_delivered(apic_irq_delivered);

  return apic_irq_delivered;
}

void apic_deliver_nmi(APICCommonState *s) {
  APICCommonClass *info = APIC_COMMON_GET_CLASS(s);

  info->external_nmi(s);
}

bool apic_next_timer(APICCommonState *s, int64_t current_time) {
  int64_t d;

  /* We need to store the timer state separately to support APIC
   * implementations that maintain a non-QEMU timer, e.g. inside the
   * host kernel. This open-coded state allows us to migrate between
   * both models. */
  s->timer_expiry = -1;

  if (s->lvt[APIC_LVT_TIMER] & APIC_LVT_MASKED) {
    return false;
  }

  d = (current_time - s->initial_count_load_time) >> s->count_shift;

  if (s->lvt[APIC_LVT_TIMER] & APIC_LVT_TIMER_PERIODIC) {
    if (!s->initial_count) {
      return false;
    }
    d = ((d / ((uint64_t)s->initial_count + 1)) + 1) *
        ((uint64_t)s->initial_count + 1);
  } else {
    if (d >= s->initial_count) {
      return false;
    }
    d = (uint64_t)s->initial_count + 1;
  }
  s->next_time = s->initial_count_load_time + (d << s->count_shift);
  s->timer_expiry = s->next_time;
  return true;
}

void apic_init_reset(APICCommonState *s) {
  APICCommonClass *info;
  int i;

  if (!s) {
    return;
  }
  s->tpr = 0;
  s->spurious_vec = 0xff;
  s->log_dest = 0;
  s->dest_mode = 0xf;
  memset(s->isr, 0, sizeof(s->isr));
  memset(s->tmr, 0, sizeof(s->tmr));
  memset(s->irr, 0, sizeof(s->irr));
  for (i = 0; i < APIC_LVT_NB; i++) {
    s->lvt[i] = APIC_LVT_MASKED;
  }
  s->esr = 0;
  memset(s->icr, 0, sizeof(s->icr));
  s->divide_conf = 0;
  s->count_shift = 0;
  s->initial_count = 0;
  s->initial_count_load_time = 0;
  s->next_time = 0;
  s->wait_for_sipi = !cpu_is_bsp(s->cpu);

  if (s->timer) {
    timer_del(s->timer);
  }
  s->timer_expiry = -1;

  info = APIC_COMMON_GET_CLASS(s);
  if (info->reset) {
    info->reset(s);
  }
}

void apic_designate_bsp(APICCommonState *s, bool bsp) {
  if (s == NULL) {
    return;
  }

  if (bsp) {
    s->apicbase |= MSR_IA32_APICBASE_BSP;
  } else {
    s->apicbase &= ~MSR_IA32_APICBASE_BSP;
  }
}

APICCommonState __apic;
APICCommonClass __apic_class;

void apic_reset_common() {
  APICCommonState *s = &__apic;
  APICCommonClass *info = APIC_COMMON_GET_CLASS(s);
  uint32_t bsp;

  bsp = s->apicbase & MSR_IA32_APICBASE_BSP;
  s->apicbase = APIC_DEFAULT_ADDRESS | bsp | MSR_IA32_APICBASE_ENABLE;
  s->id = s->initial_apic_id;

  apic_reset_irq_delivered();

  s->vapic_paddr = 0;
  info->vapic_base_update(s);

  apic_init_reset(s);
}

#if BMBT
static const VMStateDescription vmstate_apic_common;
#endif

void apic_common_realize(APICCommonState *s) {
  APICCommonClass *info;
  static VAPICROMState *vapic;
  // int instance_id = s->id;

  info = APIC_COMMON_GET_CLASS(s);
  info->realize(s);

#ifdef BMBT
  /* Note: We need at least 1M to map the VAPIC option ROM */
  if (!vapic && s->vapic_control & VAPIC_ENABLE_MASK && !hax_enabled() &&
      ram_size >= 1024 * 1024) {
    vapic = sysbus_create_simple("kvmvapic", -1, NULL);
  }
#endif
  s->vapic = vapic;
  if (apic_report_tpr_access && info->enable_tpr_reporting) {
    info->enable_tpr_reporting(s, true);
  }

#ifdef BMBT
  if (s->legacy_instance_id) {
    instance_id = -1;
  }
  vmstate_register_with_alias_id(NULL, instance_id, &vmstate_apic_common, s, -1,
                                 0, NULL);
#endif
}

#ifdef BMBT
static void apic_common_unrealize(DeviceState *dev, Error **errp) {
  APICCommonState *s = APIC_COMMON(dev);
  APICCommonClass *info = APIC_COMMON_GET_CLASS(s);

  vmstate_unregister(NULL, &vmstate_apic_common, s);
  info->unrealize(dev, errp);

  if (apic_report_tpr_access && info->enable_tpr_reporting) {
    info->enable_tpr_reporting(s, false);
  }
}

static int apic_pre_load(void *opaque) {
  APICCommonState *s = APIC_COMMON(opaque);

  /* The default is !cpu_is_bsp(s->cpu), but the common value is 0
   * so that's what apic_common_sipi_needed checks for.  Reset to
   * the value that is assumed when the apic_sipi subsection is
   * absent.
   */
  s->wait_for_sipi = 0;
  return 0;
}

static int apic_dispatch_pre_save(void *opaque) {
  APICCommonState *s = APIC_COMMON(opaque);
  APICCommonClass *info = APIC_COMMON_GET_CLASS(s);

  if (info->pre_save) {
    info->pre_save(s);
  }

  return 0;
}

static int apic_dispatch_post_load(void *opaque, int version_id) {
  APICCommonState *s = APIC_COMMON(opaque);
  APICCommonClass *info = APIC_COMMON_GET_CLASS(s);

  if (info->post_load) {
    info->post_load(s);
  }
  return 0;
}

static bool apic_common_sipi_needed(void *opaque) {
  APICCommonState *s = APIC_COMMON(opaque);
  return s->wait_for_sipi != 0;
}

static const VMStateDescription vmstate_apic_common_sipi = {
    .name = "apic_sipi",
    .version_id = 1,
    .minimum_version_id = 1,
    .needed = apic_common_sipi_needed,
    .fields = (VMStateField[]){VMSTATE_INT32(sipi_vector, APICCommonState),
                               VMSTATE_INT32(wait_for_sipi, APICCommonState),
                               VMSTATE_END_OF_LIST()}};

static const VMStateDescription vmstate_apic_common = {
    .name = "apic",
    .version_id = 3,
    .minimum_version_id = 3,
    .pre_load = apic_pre_load,
    .pre_save = apic_dispatch_pre_save,
    .post_load = apic_dispatch_post_load,
    .fields =
        (VMStateField[]){
            VMSTATE_UINT32(apicbase, APICCommonState),
            VMSTATE_UINT8(id, APICCommonState),
            VMSTATE_UINT8(arb_id, APICCommonState),
            VMSTATE_UINT8(tpr, APICCommonState),
            VMSTATE_UINT32(spurious_vec, APICCommonState),
            VMSTATE_UINT8(log_dest, APICCommonState),
            VMSTATE_UINT8(dest_mode, APICCommonState),
            VMSTATE_UINT32_ARRAY(isr, APICCommonState, 8),
            VMSTATE_UINT32_ARRAY(tmr, APICCommonState, 8),
            VMSTATE_UINT32_ARRAY(irr, APICCommonState, 8),
            VMSTATE_UINT32_ARRAY(lvt, APICCommonState, APIC_LVT_NB),
            VMSTATE_UINT32(esr, APICCommonState),
            VMSTATE_UINT32_ARRAY(icr, APICCommonState, 2),
            VMSTATE_UINT32(divide_conf, APICCommonState),
            VMSTATE_INT32(count_shift, APICCommonState),
            VMSTATE_UINT32(initial_count, APICCommonState),
            VMSTATE_INT64(initial_count_load_time, APICCommonState),
            VMSTATE_INT64(next_time, APICCommonState),
            VMSTATE_INT64(timer_expiry,
                          APICCommonState), /* open-coded timer state */
            VMSTATE_END_OF_LIST()},
    .subsections =
        (const VMStateDescription *[]){&vmstate_apic_common_sipi, NULL}};
#endif

#ifdef BMBT
static Property apic_properties_common[] = {
    DEFINE_PROP_UINT8("version", APICCommonState, version, 0x14),
    DEFINE_PROP_BIT("vapic", APICCommonState, vapic_control, VAPIC_ENABLE_BIT,
                    true),
    DEFINE_PROP_BOOL("legacy-instance-id", APICCommonState, legacy_instance_id,
                     false),
    DEFINE_PROP_END_OF_LIST(),
};

static void apic_common_get_id(Object *obj, Visitor *v, const char *name,
                               void *opaque, Error **errp) {
  APICCommonState *s = APIC_COMMON(obj);
  uint32_t value;

  value = s->apicbase & MSR_IA32_APICBASE_EXTD ? s->initial_apic_id : s->id;
  visit_type_uint32(v, name, &value, errp);
}
#endif

void apic_common_set_id(APICCommonState *s, uint32_t value) {
#ifdef BMBT
  APICCommonState *s = APIC_COMMON(obj);
  DeviceState *dev = DEVICE(obj);
  Error *local_err = NULL;
  uint32_t value;

  if (dev->realized) {
    qdev_prop_set_after_realize(dev, name, errp);
    return;
  }

  visit_type_uint32(v, name, &value, &local_err);
  if (local_err) {
    error_propagate(errp, local_err);
    return;
  }
#endif

  s->initial_apic_id = value;
  s->id = (uint8_t)value;
}

static void apic_common_initfn(APICCommonState *s) {
  // APICCommonState *s = APIC_COMMON(obj);

  s->id = s->initial_apic_id = -1;

  // port apic_properties_common
  s->version = 0x14;
  s->vapic_control = 0;
  s->legacy_instance_id = false;

#ifdef BMBT
  object_property_add(obj, "id", "uint32", apic_common_get_id,
                      apic_common_set_id, NULL, NULL, NULL);
#endif
}

static void apic_common_class_init() {
  // DeviceClass *dc = DEVICE_CLASS(klass);

  // dc->reset = apic_reset_common;
  // dc->props = apic_properties_common;
  // dc->realize = apic_common_realize;
  // dc->unrealize = apic_common_unrealize;
  /*
   * Reason: APIC and CPU need to be wired up by
   * x86_cpu_apic_create()
   */
  // dc->user_creatable = false;
}

APICCommonState *QOM_apic_init() {
  APICCommonState *apic = &__apic;
  APICCommonClass *apic_class = &__apic_class;

  apic_common_class_init();
  apic_class_init(apic_class);
  APIC_COMMON_SET_CLASS(apic, apic_class);

  apic_common_initfn(apic);

  return apic;
}

#ifdef BMBT
static const TypeInfo apic_common_type = {
    .name = TYPE_APIC_COMMON,
    .parent = TYPE_DEVICE,
    .instance_size = sizeof(APICCommonState),
    .instance_init = apic_common_initfn,
    .class_size = sizeof(APICCommonClass),
    .class_init = apic_common_class_init,
    .abstract = true,
};

static void apic_common_register_types(void) {
  type_register_static(&apic_common_type);
}

type_init(apic_common_register_types)
#endif
