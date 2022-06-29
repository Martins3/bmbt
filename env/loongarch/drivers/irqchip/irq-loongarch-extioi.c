#include <asm/cpu-features.h>
#include <asm/loongarchregs.h>
#include <asm/mach-la64/irq.h>
#include <asm/setup.h>
#include <hw/core/cpu.h>
#include <linux/bitops.h>
#include <linux/cpumask.h>
#include <linux/nodemask.h>
#include <linux/smp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VEC_COUNT_PER_REG 64
#define MAX_EIO_NODES (NR_CPUS / CORES_PER_EXTIOI_NODE)

struct extioi {
  u32 vec_count;
  u32 misc_func;
  u32 eio_en_off;
  struct irq_domain *extioi_domain;
  struct fwnode_handle *domain_handle;
  nodemask_t node_map;
  u32 node;
#ifdef BMBT
  /* struct cpumask cpuspan_map; */
#endif
} * extioi_priv[MAX_EIO_PICS];

unsigned int extioi_en[IOCSR_EXTIOI_VECTOR_NUM / 32];

int nr_extioi;

static void extioi_irq_dispatch(int irq);

static void virt_extioi_set_irq_route(int pos, unsigned int cpu) {
  iocsr_writeb(cpu_logical_map(cpu), LOONGARCH_IOCSR_EXTIOI_ROUTE_BASE + pos);
}

static int group_of_node(int node) {
  int i;
  for (i = 0; i < nr_extioi; i++) {
    if (node_isset(node, extioi_priv[i]->node_map))
      return i;
  }
  return -1;
}

static int cpu_to_eio_node(int cpu) {
  return cpu_logical_map(cpu) / CORES_PER_EXTIOI_NODE;
}

void extioi_vec_init(struct fwnode_handle *fwnode, int cascade, u32 vec_count,
                     u32 misc_func, u32 eio_en_off, u64 node_map, u32 node) {
  int i;
  extioi_priv[nr_extioi] = calloc(1, sizeof(struct extioi));
  if (!extioi_priv[nr_extioi])
    abort();

#ifdef BMBT
  extioi_priv[nr_extioi]->extioi_domain = irq_domain_create_linear(
      fwnode, vec_count, &extioi_domain_ops, extioi_priv[nr_extioi]);
  if (!extioi_priv[nr_extioi]->extioi_domain) {
    pr_err("loongson-extioi: cannot add IRQ domain\n");
    err = -ENOMEM;
    goto failed_exit;
  }
#endif
  extioi_priv[nr_extioi]->misc_func = misc_func;
  extioi_priv[nr_extioi]->eio_en_off = eio_en_off;
  extioi_priv[nr_extioi]->vec_count = vec_count;

  node_map = node_map ? node_map : -1ULL;

  for_each_possible_cpu(i) {
    if (node_map & (1ULL << (cpu_to_eio_node(i)))) {
      node_set(cpu_to_eio_node(i), extioi_priv[nr_extioi]->node_map);
#ifdef BMBT
      cpumask_or(&extioi_priv[nr_extioi]->cpuspan_map,
                 &extioi_priv[nr_extioi]->cpuspan_map, cpumask_of(i));
#endif
    } else {
      abort();
    }
  }

  extioi_priv[nr_extioi]->node = node;

#ifdef BMBT
  irq_set_chained_handler_and_data(cascade, extioi_irq_dispatch,
                                   extioi_priv[nr_extioi]);
  extioi_priv[nr_extioi]->domain_handle = fwnode;
#else
  set_vi_handler(EXCCODE_IP1, extioi_irq_dispatch);
#endif

  if (get_irq_route_model() == PCH_IRQ_ROUTE_EXT_SOC) {
    abort();
    /* irq_set_default_host(extioi_priv[nr_extioi]->extioi_domain); */
  }

  nr_extioi++;
  extioi_init();
}

static void extioi_set_irq_route(int pos, unsigned int cpu,
                                 nodemask_t *node_map, unsigned int on_node) {
  uint32_t pos_off;
  unsigned int node, dst_node, route_node;
  unsigned char coremap[MAX_EIO_NODES];
  uint32_t data, data_byte, data_mask;
  unsigned int cpu_iter;

  pos_off = pos & ~3;
  data_byte = pos & (3);
  data_mask = ~BIT_MASK(data_byte) & 0xf;
  memset(coremap, 0, sizeof(unsigned char) * MAX_EIO_NODES);

  /* calculate dst node and coremap of target irq */
  dst_node = cpu_to_eio_node(cpu);
  coremap[dst_node] |= (1 << (cpu_logical_map(cpu) % CORES_PER_EXTIOI_NODE));

  for_each_online_cpu(cpu_iter) {
    node = cpu_to_eio_node(cpu_iter);
    assert(cpu_iter == 0 && node == 0);
    if (node_isset(node, *node_map)) {
      data = 0ULL;

      /* extioi node 0 is in charge of inter-node interrupt dispatch */
      route_node = (node == on_node) ? dst_node : node;
      data |= ((coremap[node] | (route_node << 4)) << (data_byte * 8));
      csr_any_send(LOONGARCH_IOCSR_EXTIOI_ROUTE_BASE + pos_off, data, data_mask,
                   node * CORES_PER_EXTIOI_NODE);
    } else {
      abort();
    }
  }
}

int ext_set_irq_affinity(int hwirq) {
  uint32_t vector, pos_off;
  /* unsigned long flags; */

  /* [BMBT_MTTCG 3] */
  /* only one CPU available, there's no need to calculate affinity */
#ifdef BMBT
  struct extioi *priv = (struct extioi *)d->domain->host_data;
  struct cpumask intersect_affinity;

  if (!IS_ENABLED(CONFIG_SMP))
    return -EPERM;

  spin_lock_irqsave(&affinity_lock, flags);
  if (!cpumask_intersects(affinity, cpu_online_mask)) {
    spin_unlock_irqrestore(&affinity_lock, flags);
    return -EINVAL;
  }

  cpumask_and(&intersect_affinity, affinity, cpu_online_mask);
  cpumask_and(&intersect_affinity, &intersect_affinity, &priv->cpuspan_map);
  if (cpumask_empty(&intersect_affinity)) {
    spin_unlock_irqrestore(&affinity_lock, flags);
    return -EINVAL;
  }
  cpu = cpumask_first(&intersect_affinity);
#else
  unsigned int cpu = 0;
#endif
  struct extioi *priv = extioi_priv[0];

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
    extioi_set_irq_route(vector, cpu, &priv->node_map, priv->node);
    csr_any_send(LOONGARCH_IOCSR_EXTIOI_EN_BASE + (pos_off << 2),
                 extioi_en[pos_off], 0x0, 0);
  }

#ifdef BMBT
  irq_data_update_effective_affinity(d, cpumask_of(cpu));
  spin_unlock_irqrestore(&affinity_lock, flags);
#endif

  return 0;
}

/* Initializing a extioi controller */
void extioi_init(void) {
  int i, j;
  uint32_t data;
  uint64_t tmp;

  int extioi_node = cpu_to_eio_node(smp_processor_id());
  int group = group_of_node(extioi_node);

  assert(extioi_node == 0);
  assert(group == 0);

  if (group < 0) {
    printf("Error: no node map for extioi group!\n");
    return;
  }
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

void dump_extioi_state() {
  int i;
  u64 pending;
  int reg_count = IOCSR_EXTIOI_VECTOR_NUM >> 6;
  printf("[huxueshi:%s:%d]\n", __FUNCTION__, __LINE__);

  for (i = 0; i < reg_count; i++) {
    pending = iocsr_readq(LOONGARCH_IOCSR_EXTIOI_ISR_BASE + (i << 3));
    /* Do not write ISR register since it is zero already */
    if (pending == 0)
      continue;

    iocsr_writeq(pending, LOONGARCH_IOCSR_EXTIOI_ISR_BASE + (i << 3));

    while (pending) {
      int bit = __ffs(pending);
      int hwirq = bit + VEC_COUNT_PER_REG * i;
      printf(" %d", hwirq);
      pending &= ~BIT(bit);
    }
  }
  printf("\n");
}

void set_extioi_action_handler(int hwirq, irq_action action) {
  extioi_actions[hwirq] = action;
  /**
   * [BMBT_MTTCG 6]
   * I don't why the following ext_set_irq_affinity make bare metal mode's
   * interrupt work abnormally. If the implement multicore version, the root
   * cause of the problem needs to be carefully analyzed
   */
  if (cpu_has_hypervisor)
    ext_set_irq_affinity(hwirq);
}
