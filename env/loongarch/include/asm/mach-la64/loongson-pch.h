#ifndef LOONGSON_PCH_H_4FROMZ0V
#define LOONGSON_PCH_H_4FROMZ0V
#include <linux/type.h>

/* ============== LS7A registers =============== */
#define LS7A_PCH_REG_BASE 0x10000000UL
/* CHIPCFG regs */
#define LS7A_CHIPCFG_REG_BASE (LS7A_PCH_REG_BASE + 0x00010000)
/* MISC reg base */
#define LS7A_MISC_REG_BASE (LS7A_PCH_REG_BASE + 0x00080000)
/* ACPI regs */
#define LS7A_ACPI_REG_BASE (LS7A_MISC_REG_BASE + 0x00050000)
/* RTC regs */
#define LS7A_RTC_REG_BASE (LS7A_MISC_REG_BASE + 0x00050100)
#define LS7A_PCI_DC_REG_BASE (LS7A_PCH_REG_BASE + 0x0a000000)

#define LS7A_DMA_CFG ((volatile void *)TO_UNCAC(LS7A_CHIPCFG_REG_BASE + 0x041c))
#define LS7A_DMA_NODE_SHF 8
#define LS7A_DMA_NODE_MASK 0x1F00

/* RTC ram addr */
#define LS7A_RTC_RAM (LS7A_ACPI_REG_BASE + 0x00000050)

#define LS7A_INT_MASK_REG ((volatile void *)TO_UNCAC(LS7A_PCH_REG_BASE + 0x020))
#define LS7A_INT_EDGE_REG ((volatile void *)TO_UNCAC(LS7A_PCH_REG_BASE + 0x060))
#define LS7A_INT_CLEAR_REG                                                     \
  ((volatile void *)TO_UNCAC(LS7A_PCH_REG_BASE + 0x080))
#define LS7A_INT_HTMSI_EN_REG                                                  \
  ((volatile void *)TO_UNCAC(LS7A_PCH_REG_BASE + 0x040))
#define LS7A_INT_ROUTE_ENTRY_REG                                               \
  ((volatile void *)TO_UNCAC(LS7A_PCH_REG_BASE + 0x100))
#define LS7A_INT_HTMSI_VEC_REG                                                 \
  ((volatile void *)TO_UNCAC(LS7A_PCH_REG_BASE + 0x200))
#define LS7A_INT_STATUS_REG                                                    \
  ((volatile void *)TO_UNCAC(LS7A_PCH_REG_BASE + 0x3a0))
#define LS7A_INT_POL_REG ((volatile void *)TO_UNCAC(LS7A_PCH_REG_BASE + 0x3e0))
#define LS7A_LPC_INT_BASE (LS7A_PCH_REG_BASE + 0x2000)
#define LS7A_LPC_INT_SIZE 0x1000
#define LS7A_LPC_CASCADE_IRQ 83

#define LS7A_PMCON_SOC_REG                                                     \
  ((volatile void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x000))
#define LS7A_PMCON_RESUME_REG                                                  \
  ((volatile void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x004))
#define LS7A_PMCON_RTC_REG                                                     \
  ((volatile void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x008))
#define LS7A_PM1_EVT_REG ((volatile void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x00c))
#define LS7A_PM1_ENA_REG ((volatile void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x010))
#define LS7A_PM1_CNT_REG ((volatile void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x014))
#define LS7A_PM1_TMR_REG ((volatile void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x018))
#define LS7A_P_CNT_REG ((volatile void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x01c))
#define LS7A_GPE0_STS_REG                                                      \
  ((volatile void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x028))
#define LS7A_GPE0_ENA_REG                                                      \
  ((volatile void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x02c))
#define LS7A_RST_CNT_REG ((volatile void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x030))
#define LS7A_WD_SET_REG ((volatile void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x034))
#define LS7A_WD_TIMER_REG                                                      \
  ((volatile void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x038))
#define LS7A_THSENS_CNT_REG                                                    \
  ((volatile void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x04c))
#define LS7A_GEN_RTC_1_REG                                                     \
  ((volatile void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x050))
#define LS7A_GEN_RTC_2_REG                                                     \
  ((volatile void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x054))
#define LS7A_DPM_CFG_REG ((volatile void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x400))
#define LS7A_DPM_STS_REG ((volatile void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x404))
#define LS7A_DPM_CNT_REG ((volatile void *)TO_UNCAC(LS7A_ACPI_REG_BASE + 0x408))

typedef enum {
  ACPI_PCI_HOTPLUG_STATUS = 1 << 1,
  ACPI_CPU_HOTPLUG_STATUS = 1 << 2,
  ACPI_MEMORY_HOTPLUG_STATUS = 1 << 3,
  ACPI_PWRBT_STATUS = 1 << 8,
  ACPI_PCIE_WAKEUP_STATUS = 1 << 14,
  ACPI_WAKE_STATUS = 1 << 15,
} AcpiEventStatusBits;

#define LS7A_PCIE_BAR_BASE(bus, dev, func)                                     \
  readl((void *)TO_UNCAC(LS7A_PCI_DC_REG_BASE | (bus << 16) | (dev << 11) |    \
                         (func << 8) | 0x10))

/* 7A bridge has a gpio controller in DC space */
#define LS7A_DC_CNT_REG_BASE (LS7A_PCIE_BAR_BASE(0x0, 0x6, 0x1) & 0xfffffff0)

/*PCI Configration Space Base*/
#define MCFG_EXT_PCICFG_BASE 0xefe00000000UL

/*PCH OFFSET*/
#define HT1LO_OFFSET 0xe0000000000UL

/* REG ACCESS*/
#define ls7a_readb(addr) (*(volatile unsigned char *)TO_UNCAC(addr))
#define ls7a_readw(addr) (*(volatile unsigned short *)TO_UNCAC(addr))
#define ls7a_readl(addr) (*(volatile unsigned int *)TO_UNCAC(addr))
#define ls7a_readq(addr) (*(volatile unsigned long *)TO_UNCAC(addr))
#define ls7a_writeb(val, addr)                                                 \
  (*(volatile unsigned char *)TO_UNCAC(addr) = (val))
#define ls7a_writew(val, addr)                                                 \
  (*(volatile unsigned short *)TO_UNCAC(addr) = (val))
#define ls7a_writel(val, addr)                                                 \
  (*(volatile unsigned int *)TO_UNCAC(addr) = (val))
#define ls7a_writeq(val, addr)                                                 \
  (*(volatile unsigned long *)TO_UNCAC(addr) = (val))

#define ls7a_write(val, addr) ls7a_write_type(val, addr, uint64_t)

#ifdef BMBT
extern unsigned long ls7a_rwflags;
extern rwlock_t ls7a_rwlock;
#endif
#define ls7a_read(val, addr)                                                   \
  do {                                                                         \
    read_lock_irqsave(&ls7a_rwlock, flags);                                    \
    val = *(volatile unsigned long __force *)TO_UNCAC(addr);                   \
    read_unlock_irqrestore(&ls7a_rwlock, flags);                               \
  } while (0)

#define ls7a_write_type(val, addr, type)                                       \
  do {                                                                         \
    write_lock_irqsave(&ls7a_rwlock, ls7a_rwflags);                            \
    *(volatile type __force *)TO_UNCAC(addr) = (val);                          \
    write_unlock_irqrestore(&ls7a_rwlock, ls7a_rwflags);                       \
  } while (0)

/* ============== Data structrues =============== */

/* gpio data */
struct platform_gpio_data {
  u32 gpio_conf;
  u32 gpio_out;
  u32 gpio_in;
  u32 in_start_bit;
  u32 support_irq;
  char *label;
  int gpio_base;
  int ngpio;
};

#endif /* end of include guard: LOONGSON_PCH_H_4FROMZ0V */
