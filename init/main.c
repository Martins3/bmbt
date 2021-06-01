#include "../header/config.h"

unsigned long fw_arg0, fw_arg1, fw_arg2, fw_arg3;

unsigned char kernel_stack[_THREAD_SIZE];

void prom_putchar(char c);

void start_kernel() {
  while (1) {
    *(char *)(0x1fe001e0) = 'a';
  }

  // TODO extract acpi
}
