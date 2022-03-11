#include <asm/cpu-features.h>
#include <asm/loongarchregs.h>
#include <asm/mach-la64/irq.h>
#include <asm/setup.h>
#include <hw/core/cpu.h>
#include <linux/smp.h>
#include <stdio.h>
#include <stdlib.h>

#define VEC_COUNT_PER_REG 64

struct extioi {
  u32 vec_count;
  u32 misc_func;
  u32 eio_en_off;
  struct irq_domain *extioi_domain;
  struct fwnode_handle *domain_handle;
#ifdef BMBT
  nodemask_t node_map;
#endif
  u32 node;
#ifdef BMBT
  /* struct cpumask cpuspan_map; */
#endif
} * extioi_priv[MAX_EIO_PICS];

unsigned int extioi_en[IOCSR_EXTIOI_VECTOR_NUM / 32];

static void extioi_irq_dispatch(int irq);

int nr_extioi;
void extioi_vec_init() {
  extioi_priv[nr_extioi] = calloc(1, sizeof(struct extioi));
  nr_extioi++;
  extioi_init();

  set_vi_handler(EXCCODE_IP1, extioi_irq_dispatch);
}

static void virt_extioi_set_irq_route(int pos, unsigned int cpu) {
  iocsr_writeb(cpu_logical_map(cpu), LOONGARCH_IOCSR_EXTIOI_ROUTE_BASE + pos);
}

void ext_set_irq_affinity(int hwirq) {
  uint32_t vector, pos_off;
  unsigned int cpu = 0;

  /*
   * control interrupt enable or disalbe through cpu 0
   * which is reponsible for dispatching interrupts.
   */
  vector = hwirq;
  pos_off = vector >> 5;

  if (cpu_has_hypervisor) {
    iocsr_writel(extioi_en[pos_off] & (~((1 << (vector & 0x1F)))),
                 LOONGARCH_IOCSR_EXTIOI_EN_BASE + (pos_off << 2));
    virt_extioi_set_irq_route(vector, cpu);
    iocsr_writel(extioi_en[pos_off],
                 LOONGARCH_IOCSR_EXTIOI_EN_BASE + (pos_off << 2));
  } else {
    csr_any_send(LOONGARCH_IOCSR_EXTIOI_EN_BASE + (pos_off << 2),
                 extioi_en[pos_off] & (~((1 << (vector & 0x1F)))), 0x0, 0);
    // TMP_TODO : I don't how to implement this yet
    /* extioi_set_irq_route(vector, cpu, &priv->node_map, priv->node); */
    abort();
    csr_any_send(LOONGARCH_IOCSR_EXTIOI_EN_BASE + (pos_off << 2),
                 extioi_en[pos_off], 0x0, 0);
  }
}

/* Initializing a extioi controller */
void extioi_init(void) {
  int i, j;
  uint32_t data;
  uint64_t tmp;

  int extioi_node = 0;
  // TMP_TODO 没太看懂 group 的内容
  int group = 0;

  /* init irq en bitmap */
  if (extioi_node == 0) {
    for (i = 0; i < IOCSR_EXTIOI_VECTOR_NUM / 32; i++)
      extioi_en[i] = -1;
  }

  /* Only the first cpu of a extioi node initializes this extioi controller */
  if (smp_processor_id() % CORES_PER_EXTIOI_NODE == 0) {
    if (!extioi_priv[0]->misc_func) {
      tmp = iocsr_readq(LOONGARCH_IOCSR_MISC_FUNC) | IOCSR_MISC_FUNC_EXT_IOI_EN;
      iocsr_writeq(tmp, LOONGARCH_IOCSR_MISC_FUNC);
    } else {
      tmp = iocsr_readq(extioi_priv[0]->misc_func) |
            (1 << extioi_priv[0]->eio_en_off);
      iocsr_writeq(tmp, extioi_priv[0]->misc_func);
    }

    for (j = 0; j < 8; j++) {
      data = (((1 << (j * 2 + 1)) << 16) | (1 << (j * 2)));
      iocsr_writel(data, LOONGARCH_IOCSR_EXTIOI_NODEMAP_BASE + j * 4);
    }

    for (j = 0; j < 2; j++) {
      data = (1 << (1 + group)) | ((1 << (1 + group)) << 8) |
             ((1 << (1 + group)) << 16) | ((1 << (1 + group)) << 24);

      iocsr_writel(data, LOONGARCH_IOCSR_EXTIOI_IPMAP_BASE + j * 4);
    }

    for (j = 0; j < IOCSR_EXTIOI_VECTOR_NUM / 4; j++) {
      if (!group) {
        // TMP_TODO
        // 1. cpu_has_hypervisor 需要测试一下
        // 2. 为什么 kvm 模式下和正常模式下存在差别
        if (cpu_has_hypervisor) {
          tmp = cpu_logical_map(0);
        } else {
          tmp = BIT(cpu_logical_map(0));
        }

        /* route to node 0 logical core 0 */
        data = tmp | (tmp << 8) | (tmp << 16) | (tmp << 24);
        iocsr_writel(data, LOONGARCH_IOCSR_EXTIOI_ROUTE_BASE + j * 4);
      } else {

        /* route to node connected to bridge */
        tmp = (extioi_priv[group]->node << 4) | 1;
        data = tmp | (tmp << 8) | (tmp << 16) | (tmp << 24);
        iocsr_writel(data, LOONGARCH_IOCSR_EXTIOI_ROUTE_BASE + j * 4);
      }
    }

    for (j = 0; j < IOCSR_EXTIOI_VECTOR_NUM / 32; j++) {
      data = -1;
      iocsr_writel(data, LOONGARCH_IOCSR_EXTIOI_BOUNCE_BASE + j * 4);
      iocsr_writel(data, LOONGARCH_IOCSR_EXTIOI_EN_BASE + j * 4);
    }
  }
}

#ifndef __always_inline
#define __always_inline inline __attribute__((always_inline))
#endif

/**
 * __ffs - find first bit in word.
 * @word: The word to search
 *
 * Undefined if no bit exists, so code should check against 0 first.
 */
static __always_inline unsigned long __ffs(unsigned long word) {
  return __builtin_ctzl(word);
}

static irq_action extioi_actions[IOCSR_EXTIOI_VECTOR_NUM];

static void extioi_irq_dispatch(int irq) {
  int i;
  u64 pending;
  int reg_count = IOCSR_EXTIOI_VECTOR_NUM >> 6;

  for (i = 0; i < reg_count; i++) {
    pending = iocsr_readq(LOONGARCH_IOCSR_EXTIOI_ISR_BASE + (i << 3));
    /* Do not write ISR register since it is zero already */
    if (pending == 0)
      continue;

    iocsr_writeq(pending, LOONGARCH_IOCSR_EXTIOI_ISR_BASE + (i << 3));

    while (pending) {
      int bit = __ffs(pending);
      int hwirq = bit + VEC_COUNT_PER_REG * i;
      if (extioi_actions[hwirq]) {
        enter_interrpt_context();
        extioi_actions[hwirq](hwirq);
        leave_interrpt_context();
      } else {
        printf("extioi handler[%d] is not installed\n", hwirq);
        abort();
      }

      pending &= ~BIT(bit);
    }
  }
}

void set_extioi_action_handler(int hwirq, irq_action action) {
  extioi_actions[hwirq] = action;
  ext_set_irq_affinity(hwirq);
}
