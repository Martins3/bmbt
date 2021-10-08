#include "../../include/exec/hwaddr.h"
#include "../../include/hw/irq.h"
#include "../../include/hw/isa/i8259_internal.h"
#include "../../include/qemu/log.h"

#define TYPE_I8259 "isa-i8259"
PICCommonState *isa_pic;
static PICCommonState *slave_pic;

/* return the highest priority found in mask (highest = smallest
   number). Return 8 if no irq */
static int get_priority(PICCommonState *s, int mask) {
  int priority;

  if (mask == 0) {
    return 8;
  }
  priority = 0;
  while ((mask & (1 << ((priority + s->priority_add) & 7))) == 0) {
    priority++;
  }
  return priority;
}

/* return the pic wanted interrupt. return -1 if none */
static int pic_get_irq(PICCommonState *s) {
  int mask, cur_priority, priority;

  mask = s->irr & ~s->imr;
  priority = get_priority(s, mask);
  if (priority == 8) {
    return -1;
  }
  /* compute current priority. If special fully nested mode on the
     master, the IRQ coming from the slave is not taken into account
     for the priority computation. */
  mask = s->isr;
  if (s->special_mask) {
    mask &= ~s->imr;
  }
  if (s->special_fully_nested_mode && s->master) {
    mask &= ~(1 << 2);
  }
  cur_priority = get_priority(s, mask);
  if (priority < cur_priority) {
    /* higher priority found: an irq should be generated */
    return (priority + s->priority_add) & 7;
  } else {
    return -1;
  }
}

/* Update INT output. Must be called every time the output may have changed. */
static void pic_update_irq(PICCommonState *s) {
  int irq;

  irq = pic_get_irq(s);
  if (irq >= 0) {
    // fuck_trace_pic_update_irq(s->master, s->imr, s->irr, s->priority_add);
    qemu_irq_raise(s->int_out[0]);
  } else {
    qemu_irq_lower(s->int_out[0]);
  }
}

/* set irq level. If an edge is detected, then the IRR is set to 1 */
static void pic_set_irq(void *opaque, int irq, int level) {
  PICCommonState *s = opaque;
  int mask = 1 << irq;
  int irq_index = s->master ? irq : irq + 8;

  // fuck_trace_pic_set_irq(s->master, irq, level);
  // [interface 18]
  // pic_stat_update_irq(irq_index, level);

#ifdef DEBUG_IRQ_LATENCY
  if (level) {
    irq_time[irq_index] = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
  }
#endif

  if (s->elcr & mask) {
    /* level triggered */
    if (level) {
      s->irr |= mask;
      s->last_irr |= mask;
    } else {
      s->irr &= ~mask;
      s->last_irr &= ~mask;
    }
  } else {
    /* edge triggered */
    if (level) {
      if ((s->last_irr & mask) == 0) {
        s->irr |= mask;
      }
      s->last_irr |= mask;
    } else {
      s->last_irr &= ~mask;
    }
  }
  pic_update_irq(s);
}

/* acknowledge interrupt 'irq' */
static void pic_intack(PICCommonState *s, int irq) {
  if (s->auto_eoi) {
    if (s->rotate_on_auto_eoi) {
      s->priority_add = (irq + 1) & 7;
    }
  } else {
    s->isr |= (1 << irq);
  }
  /* We don't clear a level sensitive interrupt here */
  if (!(s->elcr & (1 << irq))) {
    s->irr &= ~(1 << irq);
  }
  pic_update_irq(s);
}

int pic_read_irq(PICCommonState *s) {
  int irq, irq2, intno;

  irq = pic_get_irq(s);
  if (irq >= 0) {
    if (irq == 2) {
      irq2 = pic_get_irq(slave_pic);
      if (irq2 >= 0) {
        pic_intack(slave_pic, irq2);
      } else {
        /* spurious IRQ on slave controller */
        irq2 = 7;
      }
      intno = slave_pic->irq_base + irq2;
    } else {
      intno = s->irq_base + irq;
    }
    pic_intack(s, irq);
  } else {
    /* spurious IRQ on host controller */
    irq = 7;
    intno = s->irq_base + irq;
  }

  if (irq == 2) {
    irq = irq2 + 8;
  }

#ifdef DEBUG_IRQ_LATENCY
  printf("IRQ%d latency=%0.3fus\n", irq,
         (double)(qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) - irq_time[irq]) *
             1000000.0 / NANOSECONDS_PER_SECOND);
#endif

  // fuck_trace_pic_interrupt(irq, intno);
  return intno;
}

void pic_init_reset(PICCommonState *s) {
  pic_reset_common(s);
  pic_update_irq(s);
}

static void pic_reset(PICCommonState *s) {
  s->elcr = 0;
  pic_init_reset(s);
}

static void pic_ioport_write(void *opaque, hwaddr addr64, uint64_t val64,
                             unsigned size) {
  PICCommonState *s = opaque;
  uint32_t addr = addr64;
  uint32_t val = val64;
  int priority, cmd, irq;

  // fuck_trace_pic_ioport_write(s->master, addr, val);

  if (addr == 0) {
    if (val & 0x10) {
      pic_init_reset(s);
      s->init_state = 1;
      s->init4 = val & 1;
      s->single_mode = val & 2;
      if (val & 0x08) {
        qemu_log_mask(LOG_UNIMP, "i8259: level sensitive irq not supported\n");
      }
    } else if (val & 0x08) {
      if (val & 0x04) {
        s->poll = 1;
      }
      if (val & 0x02) {
        s->read_reg_select = val & 1;
      }
      if (val & 0x40) {
        s->special_mask = (val >> 5) & 1;
      }
    } else {
      cmd = val >> 5;
      switch (cmd) {
      case 0:
      case 4:
        s->rotate_on_auto_eoi = cmd >> 2;
        break;
      case 1: /* end of interrupt */
      case 5:
        priority = get_priority(s, s->isr);
        if (priority != 8) {
          irq = (priority + s->priority_add) & 7;
          s->isr &= ~(1 << irq);
          if (cmd == 5) {
            s->priority_add = (irq + 1) & 7;
          }
          pic_update_irq(s);
        }
        break;
      case 3:
        irq = val & 7;
        s->isr &= ~(1 << irq);
        pic_update_irq(s);
        break;
      case 6:
        s->priority_add = (val + 1) & 7;
        pic_update_irq(s);
        break;
      case 7:
        irq = val & 7;
        s->isr &= ~(1 << irq);
        s->priority_add = (irq + 1) & 7;
        pic_update_irq(s);
        break;
      default:
        /* no operation */
        break;
      }
    }
  } else {
    switch (s->init_state) {
    case 0:
      /* normal mode */
      s->imr = val;
      pic_update_irq(s);
      break;
    case 1:
      s->irq_base = val & 0xf8;
      s->init_state = s->single_mode ? (s->init4 ? 3 : 0) : 2;
      break;
    case 2:
      if (s->init4) {
        s->init_state = 3;
      } else {
        s->init_state = 0;
      }
      break;
    case 3:
      s->special_fully_nested_mode = (val >> 4) & 1;
      s->auto_eoi = (val >> 1) & 1;
      s->init_state = 0;
      break;
    }
  }
}

static uint64_t pic_ioport_read(void *opaque, hwaddr addr, unsigned size) {
  PICCommonState *s = opaque;
  int ret;

  if (s->poll) {
    ret = pic_get_irq(s);
    if (ret >= 0) {
      pic_intack(s, ret);
      ret |= 0x80;
    } else {
      ret = 0;
    }
    s->poll = 0;
  } else {
    if (addr == 0) {
      if (s->read_reg_select) {
        ret = s->isr;
      } else {
        ret = s->irr;
      }
    } else {
      ret = s->imr;
    }
  }
  // fuck_trace_pic_ioport_read(s->master, addr, ret);
  return ret;
}

int pic_get_output(PICCommonState *s) { return (pic_get_irq(s) >= 0); }

static void elcr_ioport_write(void *opaque, hwaddr addr, uint64_t val,
                              unsigned size) {
  PICCommonState *s = opaque;
  s->elcr = val & s->elcr_mask;
}

static uint64_t elcr_ioport_read(void *opaque, hwaddr addr, unsigned size) {
  PICCommonState *s = opaque;
  return s->elcr;
}

#ifdef MEM_TODO
static const MemoryRegionOps pic_base_ioport_ops = {
    .read = pic_ioport_read,
    .write = pic_ioport_write,
    .impl =
        {
            .min_access_size = 1,
            .max_access_size = 1,
        },
};

static const MemoryRegionOps pic_elcr_ioport_ops = {
    .read = elcr_ioport_read,
    .write = elcr_ioport_write,
    .impl =
        {
            .min_access_size = 1,
            .max_access_size = 1,
        },
};
#endif

void pic_realize(PICCommonState *s) {
  PICClass *pc = PIC_GET_CLASS(s);

#ifdef MEM_TODO
  memory_region_init_io(&s->base_io, OBJECT(s), &pic_base_ioport_ops, s, "pic",
                        2);
  memory_region_init_io(&s->elcr_io, OBJECT(s), &pic_elcr_ioport_ops, s, "elcr",
                        1);
#endif

  qdev_init_gpio_out(&s->gpio, s->int_out, ARRAY_SIZE(s->int_out));
  qdev_init_gpio_in(&s->gpio, pic_set_irq, s, 8);

  duck_check(pc->parent_realize == pic_common_realize);
  pc->parent_realize(s);
}

qemu_irq *i8259_init(qemu_irq parent_irq) {
  qemu_irq *irq_set;
  PICCommonState *isadev;
  int i;

  irq_set = g_new0(qemu_irq, ISA_NUM_IRQS);

  isadev = i8259_init_chip(TYPE_I8259, true);

  qdev_connect_gpio_out(&isadev->gpio, 0, parent_irq);
  duck_check(parent_irq == isadev->int_out[0]);
  for (i = 0; i < 8; i++) {
    irq_set[i] = qdev_get_gpio_in(&isadev->gpio, i);
  }

  isa_pic = isadev;

  isadev = i8259_init_chip(TYPE_I8259, false);

  qdev_connect_gpio_out(&isadev->gpio, 0, irq_set[2]);
  for (i = 0; i < 8; i++) {
    irq_set[i + 8] = qdev_get_gpio_in(&isadev->gpio, i);
  }

  slave_pic = isadev;

  return irq_set;
}

void i8259_class_init(PICClass *k) {
  // DeviceClass *dc = DEVICE_CLASS(klass);

  k->parent_realize = pic_common_realize;
  // device_class_set_parent_realize(dc, pic_realize, &k->parent_realize);
  // dc->reset = pic_reset;
}

#ifdef BMBT
static const TypeInfo i8259_info = {
    .name = TYPE_I8259,
    .instance_size = sizeof(PICCommonState),
    .parent = TYPE_PIC_COMMON,
    .class_init = i8259_class_init,
    .class_size = sizeof(PICClass),
};

static void pic_register_types(void) { type_register_static(&i8259_info); }

type_init(pic_register_types)
#endif
