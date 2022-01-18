#include <asm/addrspace.h>
#include <asm/irq.h>
#include <asm/page.h>
#include <linux/cpu.h>
#include <linux/irqflags.h>
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

  local_irq_enable();
  while (1)
    ;

  char *argv[] = {NULL};
  main(0, argv);
}
