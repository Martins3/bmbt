#include <asm/io.h>
#include <asm/loongarchregs.h>
#include <asm/mach-la64/irq.h>
#include <asm/mach-la64/loongson-pch.h>
#include <asm/setup.h>
#include <assert.h>
#include <linux/cpumask.h>
#include <linux/nodemask.h>
#include <stdio.h>
#include <stdlib.h>

int nr_pch_pics;
#define for_each_pch_pic(idx) for ((idx) = 0; (idx) < nr_pch_pics; (idx)++)
#define MSI_IRQ_NR_64 64
#define MSI_IRQ_NR_192 192
#define MSI_IRQ_NR_224 224
#define LIOINTC_MEM_SIZE 0x80
#define LIOINTC_VECS_TO_IP2 0x00FFFFFE /* others */
#define LIOINTC_VECS_TO_IP3 0xFF000000 /* HT1 0-7 */
#define PCH_PIC_SIZE 0x400
extern const struct plat_smp_ops *mp_ops;
u64 liointc_base = LIOINTC_DEFAULT_PHYS_BASE;
static int msi_irqbase;

static enum pch_irq_route_model_id pch_irq_route_model = PCH_IRQ_ROUTE_EXT;
enum pch_irq_route_model_id get_irq_route_model(void) {
  return pch_irq_route_model;
}

static struct pch_pic {
  /*
   * # of IRQ routing registers
   */
  int nr_registers;

  /* pch pic config */
  struct pch_pic_config config;
  /* pch pic irq routing info */
  struct pch_pic_irq irq_config;
} pch_pics[MAX_PCH_PICS];

struct pch_pic_irq *pch_pic_irq_routing(int pch_pic_idx) {
  return &pch_pics[pch_pic_idx].irq_config;
}

int pch_pic_id(int pch_pic_idx) {
  return pch_pics[pch_pic_idx].config.pch_pic_id;
}

unsigned long pch_pic_addr(int pch_pic_idx) {
  return pch_pics[pch_pic_idx].config.pch_pic_addr;
}

static int bad_pch_pic(unsigned long address) {
  if (nr_pch_pics >= MAX_PCH_PICS) {
    printf(
        "WARNING: Max # of I/O PCH_PICs (%d) exceeded (found %d), skipping\n",
        MAX_PCH_PICS, nr_pch_pics);
    abort();
    return 1;
  }
  if (!address) {
    printf("WARNING: Bogus (zero) I/O PCH_PIC address found in table, "
           "skipping!\n");
    abort();
    return 1;
  }
  return 0;
}
#define pch_pic_ver(pch_pic_idx) pch_pics[pch_pic_idx].config.pch_pic_ver

void register_pch_pic(int id, u32 address, u32 irq_base) {
  int idx = 0;
  int entries;
  struct pch_pic_irq *irq_cfg;

  if (bad_pch_pic(address))
    return;

  idx = nr_pch_pics;

  pch_pics[idx].config.pch_pic_addr = address;
  if (id) {
    // [BMBT_MTTCG 2]
    printf("only one pch_pic supported");
    abort();
    // pch_pics[idx].config.pch_pic_addr |= nid_to_addrbase(id) | HT1LO_OFFSET;
  }
  pch_pics[idx].config.pch_pic_id = id;
  pch_pics[idx].config.pch_pic_ver = 0;

  /*
   * Build basic GSI lookup table to facilitate lookups
   * and to prevent reprogramming of PCH_PIC pins (PCI GSIs).
   */
  /* irq_base is 32 bit address with acpi method */
  entries = (((unsigned long)ls7a_readq(pch_pic_addr(idx)) >> 48) & 0xff) + 1;

  irq_cfg = pch_pic_irq_routing(idx);
  irq_cfg->irq_base = irq_base;
  irq_cfg->irq_end = irq_base + entries - 1;
  /*
   * The number of PCH_PIC IRQ registers (== #pins):
   */
  pch_pics[idx].nr_registers = entries;

  printf("PCH_PIC[%d]: pch_pic_id %d, version %d, address 0x%lx, IRQ %d-%d\n",
         idx, pch_pic_id(idx), pch_pic_ver(idx), pch_pic_addr(idx),
         irq_cfg->irq_base, irq_cfg->irq_end);

  nr_pch_pics++;
  msi_irqbase = entries;
}

void static pch_pic_domains_init(void) {
  struct fwnode_handle *irq_handle = NULL;
  struct pch_pic_irq *irq_cfg;
  int i;
  /* u64 address; */

  for_each_pch_pic(i) {
    irq_cfg = pch_pic_irq_routing(i);
    if (!irq_cfg)
      continue;
#ifdef BMBT
    address = pch_pic_addr(i);
    irq_handle = irq_domain_alloc_fwnode((void *)address);
    if (!irq_handle) {
      panic("Unable to allocate domain handle  for pch_pic irqdomains.\n");
    }
#endif

    pch_pic_init(irq_handle, pch_pic_addr(i), PCH_PIC_SIZE,
                 get_irq_route_model(), irq_cfg->irq_base);
  }
}

static void pch_msi_domain_init(int start, int count) {
#ifdef BMBT
  struct fwnode_handle *irq_handle, *parent_handle;
#endif
  u64 msg_address;
  int i;
  for_each_pch_pic(i) {
    assert(i == 0); // only support one pch_pic
    msg_address = loongson_sysconf.msi_address_lo;
    msg_address |= ((u64)loongson_sysconf.msi_address_hi << 32);
#ifdef BMBT
    irq_handle = irq_domain_alloc_fwnode((void *)msg_address);
    irq_handle = irq_domain_alloc_named_id_fwnode("msintc", 0);
    if (!irq_handle) {
      panic("Unable to allocate domain handle  for pch_msi irqdomain.\n");
    }

    if (get_irq_route_model() == PCH_IRQ_ROUTE_EXT) {
      parent_handle = eiointc_get_fwnode(i);
    } else {
      parent_handle = htvec_get_fwnode();
    }
#endif
    pch_msi_init(msg_address, get_irq_route_model() == PCH_IRQ_ROUTE_EXT, start,
                 count);
  }
}

static void eiointc_domain_init(void) {
  struct fwnode_handle *irq_fwnode = NULL;
  int i, j;

  u64 node_map;
  u32 on_node[2] = {0, 5};

  // [interface 65]
#ifdef BMBT
  nodemask_t possible_nodes = node_possible_map;
  for_each_pch_pic(i) { node_clear(on_node[i], possible_nodes); }
#endif

  for_each_pch_pic(i) {
    node_map = 0;
    node_map |= (1 << on_node[i]);
    for_each_possible_cpu(j) {
      if (((j / CORES_PER_EXTIOI_NODE) % nr_pch_pics == i) &&
          (j % CORES_PER_EXTIOI_NODE == 0))
        node_map |= (1 << (j / CORES_PER_EXTIOI_NODE));
    }

#ifdef BMBT
    irq_fwnode = irq_domain_alloc_named_id_fwnode("eiointc", 0);
    if (!irq_fwnode) {
      panic("Unable to allocate domain handle for eiointc irqdomain.\n");
    }
#endif
    assert(node_map == 1);
    extioi_vec_init(irq_fwnode, LOONGSON_BRIDGE_IRQ + i,
                    IOCSR_EXTIOI_VECTOR_NUM, 0, 0, node_map, on_node[i]);
  }
}

static void liointc_domain_init(void) {
#ifdef CONFIG_LOONGSON_LIOINTC
  struct fwnode_handle *irq_handle = NULL;
  struct resource res;
  u32 parent_int_map[2] = {LIOINTC_VECS_TO_IP2, LIOINTC_VECS_TO_IP3};
  u32 parent_irq[2] = {LOONGSON_LINTC_IRQ, LOONGSON_BRIDGE_IRQ};

#ifdef BMBT
  irq_handle = irq_domain_alloc_fwnode((void *)liointc_base);
  if (!irq_handle) {
    panic("Unable to allocate domain handle for liointc irqdomain.\n");
  }
#endif
  res.start = liointc_base;
  res.end = liointc_base + LIOINTC_MEM_SIZE;
  liointc_init(&res, 2, parent_irq, parent_int_map, irq_handle,
               pch_irq_route_model);
#endif
}

static void irqchip_init_default(void) {
  /* loongarch_cpu_irq_init(); */
  liointc_domain_init();
  printf("Support EXT interrupt.\n");
#ifdef CONFIG_LOONGARCH_EXTIOI
  eiointc_domain_init();
  pch_msi_domain_init(msi_irqbase, 256 - msi_irqbase);
#endif
  pch_pic_domains_init();
  /* pch_lpc_domain_init(); */ // TMP_TODO
}

void setup_IRQ(void) {
  u64 node;

  if (loongson_sysconf.is_soc_cpu) {
    pch_irq_route_model = PCH_IRQ_ROUTE_EXT_SOC;
    abort();
  } else {
    printf("[huxueshi:%s:%d] \n", __FUNCTION__, __LINE__);
    for_each_node(node) {
      writel(0x40000000 | (node << 12),
             (volatile void __iomem *)(((node << 44) | LOONGSON_HT1_CFG_BASE) +
                                       0x274));
    }
    printf("[huxueshi:%s:%d] \n", __FUNCTION__, __LINE__);
  }

  // [interface 59]
#ifdef BMBT
  if (!acpi_disabled) {
    if (loongson_sysconf.bpi_version > BPI_VERSION_V1) {
      irqchip_init();
    } else {
      irqchip_init_default();
    }
  } else if (loongson_fdt_blob != NULL) {
    irqchip_init();
  } else {
    register_pch_pic(0, LS7A_PCH_REG_BASE, LOONGSON_PCH_IRQ_BASE);
    irqchip_init_default();
  }
#endif
  register_pch_pic(0, LS7A_PCH_REG_BASE, LOONGSON_PCH_IRQ_BASE);
  irqchip_init_default();
}

void arch_init_irq(void) {
  /*
   * Clear all of the interrupts while we change the able around a bit.
   * int-handler is not on bootstrap
   */
  clear_csr_ecfg(ECFG0_IM);
  clear_csr_estat(ESTATF_IP);

  /* machine specific irq init */
  setup_IRQ();

  set_csr_ecfg(ECFG0_IM);
}
