#include <asm/addrspace.h>
#include <asm/cpu-features.h>
#include <asm/irq.h>
#include <asm/page.h>
#include <asm/time.h>
#include <env/cpu.h>
#include <env/device.h>
#include <linux/cpu.h>
#include <linux/irqflags.h>
#include <linux/pci-ecam.h>
#include <linux/stack.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unitest/greatest.h>

extern void init_IRQ(void);

unsigned char kernel_stack[_THREAD_SIZE];

int main(int argc, char **argv);
void setup_arch(char **);

void start_kernel(void) {
  char *command_line;
  setup_arch(&command_line);
  trap_init();
  init_IRQ();
  time_init();
  loongarch_pci_init();
  local_irq_enable();

  char *argv[] = {NULL};
  main(0, argv);
}

BootParameter get_boot_parameter() {
  if (cpu_has_hypervisor)
    return FROM_INIT_RD;
  else
    return FROM_DISK;
}
