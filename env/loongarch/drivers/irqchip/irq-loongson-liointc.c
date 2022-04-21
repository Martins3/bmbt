#include <asm/addrspace.h>
#include <asm/io.h>
#include <asm/mach-la64/irq.h>
#include <asm/setup.h>
#include <assert.h>
#include <autoconf.h>
#include <hw/core/cpu.h>
#include <linux/bitops.h>
#include <linux/bits.h>
#include <linux/ioport.h>
#include <linux/smp.h>
#include <linux/type.h>
#include <stdio.h>

#define LIOINTC_CHIP_IRQ 32
#define LIOINTC_NUM_PARENT 2

#define LIOINTC_INTC_CHIP_START 0x20
#define LIOINTC_REG_INTC_STATUS(cpuid)                                         \
  (LIOINTC_INTC_CHIP_START + 0x20 + (cpuid)*8)
#define LIOINTC_REG_INTC_EN_STATUS (LIOINTC_INTC_CHIP_START + 0x04)
#define LIOINTC_REG_INTC_ENABLE (LIOINTC_INTC_CHIP_START + 0x08)
#define LIOINTC_REG_INTC_DISABLE (LIOINTC_INTC_CHIP_START + 0x0c)
#define LIOINTC_REG_INTC_EDGE (LIOINTC_INTC_CHIP_START + 0x14)

#define LIOINTC_SHIFT_INTx 4
#define LIOINTC_VECS_TO_IP2 0x00FFFFFE    /* others */
#define LIOINTC_VECS_TO_IP3_V1 0xFF000000 /* HT1 0-7 */
#define LIOINTC_VECS_TO_IP3_V0 0x0F000001 /* HT1 0-3 and sys int 0 */

struct liointc_handler_data {
  struct liointc_priv *priv;
  u32 parent_int_map;
};

// TMP_TODO 增加一个 interface 描述这个东西的实现
struct bmbt_gc {
  void __iomem *reg_base;
  void *private;
};

struct liointc_priv {
  struct bmbt_gc *gc;
  struct liointc_handler_data handler[LIOINTC_NUM_PARENT];
  u8 map_cache[LIOINTC_CHIP_IRQ];
};

struct bmbt_gc __bmbt_gc;
struct liointc_priv __liointc_priv;

static int gg;
static int kk;
void dump_interrupt() {
  printf("[huxueshi:%s:%d] gg=%d kk=%d\n", __FUNCTION__, __LINE__, gg, kk);
}

void shutup_uart();

void irq_gc_unmask_enable_reg() {
  struct bmbt_gc *gc = &__bmbt_gc;
  // TMP_TODO
  writel(0xffffffff, gc->reg_base + LIOINTC_REG_INTC_ENABLE);
}

void irq_gc_mask_disable_reg() {
  struct bmbt_gc *gc = &__bmbt_gc;
  // TMP_TODO 0xffffffff to more accurate mask
  writel(0xffffffff, gc->reg_base + LIOINTC_REG_INTC_DISABLE);
}

#define LIOINTC_UART_IRQ 10
void serial_handler(int hwirq);

static irq_action liointc_actions[IOCSR_EXTIOI_VECTOR_NUM];
void set_liointc_action_handler(int hwirq, irq_action action) {
  assert(hwirq < 32 && hwirq > 0);
  liointc_actions[hwirq] = action;
}

static void liointc_chained_handle_irq(int irq) {
  struct bmbt_gc *gc = &__bmbt_gc;
  u32 pending;
  int cpuid = cpu_logical_map(smp_processor_id());
  assert(cpuid == 0);

  // TMP_TODO : how to handle this ?
  /* chained_irq_enter(chip, desc); */
  kk++;

  pending = readl(gc->reg_base + LIOINTC_REG_INTC_STATUS(cpuid));

  while (pending) {
    int bit = __ffs(pending);
    if (liointc_actions[bit]) {
      enter_interrpt_context();
      liointc_actions[bit](bit);
      leave_interrpt_context();
    } else {
      printf("unexpected liointc %d\n", bit);
      abort();
    }
    /* u32 isr = readl(gc->reg_base + LIOINTC_INTC_CHIP_START); */
    /* printf("[huxueshi:%s:%d] %x\n", __FUNCTION__, __LINE__, isr); */
    /* shutup_uart(); */
    /* generic_handle_irq(irq_find_mapping(gc->domain, bit)); */
    gg++;
    pending &= ~BIT(bit);
  }

  irq_gc_mask_disable_reg();
  writel(0xffffffff, gc->reg_base + LIOINTC_INTC_CHIP_START);
  writel(0xffffffff, gc->reg_base + LIOINTC_REG_INTC_STATUS(cpuid));
  irq_gc_unmask_enable_reg();

  // TMP_TODO
  /* chained_irq_exit(chip, desc); */
}

/* Route */
int liointc_init(struct resource *res, int parent_irq_num, u32 *parent_irq,
                 u32 *parent_int_map, struct fwnode_handle *domain_handle,
                 int model) {
  struct bmbt_gc *gc = &__bmbt_gc;
  struct liointc_priv *priv;
  void __iomem *base;
  int i;

  priv = &__liointc_priv;
  base = (void *)TO_UNCAC(res->start);

  for (i = 0; i < parent_irq_num; i++)
    priv->handler[i].parent_int_map = parent_int_map[i];

#ifdef BMBT
  domain = irq_domain_create_linear(domain_handle, LIOINTC_CHIP_IRQ,
                                    &irq_generic_chip_ops, priv);
  err = irq_alloc_domain_generic_chips(domain, LIOINTC_CHIP_IRQ, 1, "LIOINTC",
                                       handle_level_irq, 0, IRQ_NOPROBE, 0);
#endif

  /* Disable all IRQs */
  writel(0xffffffff, base + LIOINTC_REG_INTC_DISABLE);
  /* Set to level triggered */
  writel(0x0, base + LIOINTC_REG_INTC_EDGE);

  writel(0xffffffff, base + LIOINTC_REG_INTC_EDGE);

  /* Generate parent INT part of map cache */
  for (i = 0; i < parent_irq_num; i++) {
    u32 pending = priv->handler[i].parent_int_map;

    while (pending) {
      int bit = __ffs(pending);

      priv->map_cache[bit] = BIT(i) << LIOINTC_SHIFT_INTx;
      pending &= ~BIT(bit);
    }
  }

  for (i = 0; i < LIOINTC_CHIP_IRQ; i++) {
    /* Generate core part of map cache */
    priv->map_cache[i] |= BIT(cpu_logical_map(0));
    writeb(priv->map_cache[i], base + i);
  }

  gc->private = priv;
  gc->reg_base = base;

#ifdef BMBT
  ct = gc->chip_types;
  ct->regs.enable = LIOINTC_REG_INTC_ENABLE;
  ct->regs.disable = LIOINTC_REG_INTC_DISABLE;
  ct->chip.irq_unmask = irq_gc_unmask_enable_reg;
  ct->chip.irq_mask = irq_gc_mask_disable_reg;
  ct->chip.irq_mask_ack = irq_gc_mask_disable_reg;

  gc->mask_cache = 0;
#endif
  priv->gc = gc;

  for (i = 0; i < parent_irq_num; i++) {
    if (parent_irq[i] <= 0)
      continue;
    if (parent_irq[i] == LOONGSON_BRIDGE_IRQ && model == PCH_IRQ_ROUTE_EXT)
      continue;
    priv->handler[i].priv = priv;
#ifdef BMBT
    irq_set_chained_handler_and_data(parent_irq[i], liointc_chained_handle_irq,
                                     &priv->handler[i]);
#else
    assert(parent_irq[i] == LOONGSON_LINTC_IRQ);
    // TMP_TODO 重新 review 一下为什么是 EXCCODE_IP0，而不是 LOONGSON_LINTC_IRQ
    set_vi_handler(EXCCODE_IP0, liointc_chained_handle_irq);
#endif
  }

  writel(0xffffffff, base + LIOINTC_REG_INTC_ENABLE);

  // TMP_TODO 和 pch_pic_init 统一一下
  set_liointc_action_handler(LIOINTC_UART_IRQ, serial_handler);
  return 0;
}
