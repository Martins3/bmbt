#include <asm/mach-la64/irq.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int nr_msi;
struct pch_msi_data {
  u32 irq_first; /* The vector number that MSIs starts */
  u32 num_irqs;  /* The number of vectors for MSIs */
  u64 msg_address;
} pch_msi_priv;

void msix_pass_apic_mem_write(int irq);

static void msix_irq_passthrogh(int hwirq) {
  int guest_irq = hwirq - pch_msi_priv.irq_first;
  msix_pass_apic_mem_write(guest_irq);
}

int pch_msi_init(u64 msg_address, bool ext, int start, int count) {
  pch_msi_priv.irq_first = start;
  pch_msi_priv.num_irqs = count;
  pch_msi_priv.msg_address = msg_address;

  printf("Registering %d MSIs, starting at %d\n", pch_msi_priv.num_irqs,
         pch_msi_priv.irq_first);

  for (int hwirq = pch_msi_priv.irq_first;
       hwirq < pch_msi_priv.irq_first + pch_msi_priv.num_irqs; ++hwirq) {
    set_extioi_action_handler(hwirq, msix_irq_passthrogh);
  }
  return 0;
}

// exported function used by pci_mmio_pass_write
u64 msi_message_addr() { return pch_msi_priv.msg_address; }

u32 pch_msi_allocate_hwirq(unsigned int irq) {
  assert(irq < pch_msi_priv.num_irqs);
  return pch_msi_priv.irq_first + irq;
}
