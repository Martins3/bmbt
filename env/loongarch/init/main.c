#include <asm/addrspace.h>
#include <asm/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unsigned char kernel_stack[_THREAD_SIZE];

// @todo put here temporarily, maybe better place
void setup_arch(char **);

void start_kernel(void) {
  char *command_line;
  setup_arch(&command_line);
  exit(0);
}
