#include <asm/addrspace.h>
#include <asm/io.h>
#include <asm/mach-la64/irq.h>
#include <linux/irqflags.h>
#include <linux/type.h>
#include <stdio.h>

/* Registers */
#define PCH_PIC_MASK 0x20
#define PCH_PIC_HTMSI_EN 0x40
#define PCH_PIC_EDGE 0x60
#define PCH_PIC_CLR 0x80
#define PCH_PIC_AUTO0 0xc0
#define PCH_PIC_AUTO1 0xe0
#define PCH_INT_ROUTE(irq) (0x100 + irq)
#define PCH_INT_HTVEC(irq) (0x200 + irq)
#define PCH_PIC_POL 0x3e0
#define PCH_PIC_STS 0x3a0

#define LIOINTC_PIN_SYSINT0 0
#define PIC_COUNT_PER_REG 64
#define PIC_REG_COUNT 1
#define PIC_COUNT (PIC_COUNT_PER_REG * PIC_REG_COUNT)
#define GUEST_PIC_COUNT 32
#define PIC_REG_IDX(irq_id) ((irq_id) / PIC_COUNT_PER_REG)
#define PIC_REG_BIT(irq_id) ((irq_id) % PIC_COUNT_PER_REG)
#define PCH_PIC_GSI_BASE 64
#define LS7A_VZ_PCH_REG_BASE 0xe0010000000UL

struct pch_pic {
  void *base;
  int model;
};

// TMP_TODO I don't understand we have block the interrupt
// is it already blocked?
static void pch_pic_bitset(struct pch_pic *priv, int offset, int bit) {
  u64 reg;
  unsigned long flags;
  void __iomem *addr = priv->base + offset + PIC_REG_IDX(bit) * 8;

  local_irq_save(flags);
  reg = readq(addr);
  reg |= BIT(PIC_REG_BIT(bit));
  writeq(reg, addr);
  local_irq_restore(flags);
}

static void pch_pic_bitclr(struct pch_pic *priv, int offset, int bit) {
  u64 reg;
  unsigned long flags;
  void __iomem *addr = priv->base + offset + PIC_REG_IDX(bit) * 8;

  local_irq_save(flags);
  reg = readq(addr);
  reg &= ~BIT(PIC_REG_BIT(bit));
  writeq(reg, addr);
  local_irq_restore(flags);
}

void pch_pic_mask_irq(struct pch_pic *priv, int hwirq) {
  pch_pic_bitset(priv, PCH_PIC_MASK, hwirq);
}

void pch_pic_unmask_irq(struct pch_pic *priv, int hwirq) {
  u32 idx = PIC_REG_IDX(hwirq);

  writeq(BIT(PIC_REG_BIT(hwirq)), priv->base + PCH_PIC_CLR + idx * 8);
  pch_pic_bitclr(priv, PCH_PIC_MASK, hwirq);
}

static unsigned int pch_pic_startup(struct pch_pic *priv, int hwirq) {
  pch_pic_unmask_irq(priv, hwirq);
  return 0;
}

void pch_pic_ack_irq(struct pch_pic *priv, int hwirq) {
  u64 reg;
  void __iomem *addr = priv->base + PCH_PIC_EDGE + PIC_REG_IDX(hwirq) * 8;

  reg = readq(addr);
  if (reg & BIT_ULL(hwirq)) {
    writeq(BIT(PIC_REG_BIT(hwirq)),
           priv->base + PCH_PIC_CLR + PIC_REG_IDX(hwirq) * 8);
  }
}

static void pch_pic_reset(struct pch_pic *priv) {
  int i;

  for (i = 0; i < PIC_COUNT; i++) {
    if (priv->model != PCH_IRQ_ROUTE_LINE) {
      /* Write vector ID */
      writeb(i, priv->base + PCH_INT_HTVEC(i));
    }
    /* Hardcode route to HT0 Lo */
    writeb(1, priv->base + PCH_INT_ROUTE(i));
  }

  for (i = 0; i < PIC_REG_COUNT; i++) {
    /* Clear IRQ cause registers, mask all interrupts */
    writeq((u64)-1, priv->base + PCH_PIC_MASK + 8 * i);
    writeq((u64)-1, priv->base + PCH_PIC_CLR + 8 * i);
    /* Clear auto bounce, we don't need that */
    writeq(0, priv->base + PCH_PIC_AUTO0 + 8 * i);
    writeq(0, priv->base + PCH_PIC_AUTO1 + 8 * i);
    if (priv->model != PCH_IRQ_ROUTE_LINE) {
      /* Enable HTMSI transformer */
      writeq((u64)-1, priv->base + PCH_PIC_HTMSI_EN + 8 * i);
    }
  }
}

static struct pch_pic priv = {
    .base = (void *)TO_UNCAC(0x10000000),
    .model = PCH_IRQ_ROUTE_EXT,
};

int pch_pic_init() {
  pch_pic_reset(&priv);
  return 0;
}

static irq_action pch_pic_actions[IOCSR_EXTIOI_VECTOR_NUM];

// pch-pic version handle_level_irq
void handle_pch_pic_irq(int hwirq) {
  /* pch_pic_mask_irq(&priv, hwirq); */
  /* pch_pic_ack_irq(&priv, hwirq); */
  if (pch_pic_actions[hwirq]) {
    pch_pic_actions[hwirq](hwirq);
  } else {
    printf("pch-pci handler[%d] is not installed\n", hwirq);
    abort();
  }
  /* pch_pic_unmask_irq(&priv, hwirq); */
}

void set_pch_pci_action_handler(int hwirq, irq_action action) {
  set_extioi_action_handler(hwirq, handle_pch_pic_irq);
  pch_pic_startup(&priv, hwirq);
  pch_pic_actions[hwirq] = action;
}
