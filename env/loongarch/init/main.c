#include <asm/addrspace.h>
#include <asm/config.h>
#include <linux/cpu.h>
#include <linux/page.h>
#include <linux/stack.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unitest/greatest.h>

unsigned char kernel_stack[_THREAD_SIZE];

int main(int argc, char **argv);
void setup_arch(char **);

void start_kernel(void) {
  char *command_line;
  setup_arch(&command_line);
  trap_init();
  char *argv[] = {NULL};
  main(0, argv);
}
