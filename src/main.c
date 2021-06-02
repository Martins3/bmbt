#include "../include/config.h"
#include "../include/interface.h"

unsigned long fw_arg0, fw_arg1, fw_arg2, fw_arg3;

unsigned char kernel_stack[_THREAD_SIZE];

// TODO local_irq_disable();
// the memory is setup or ?
void start_kernel() {

  printf("%lx\n", fw_arg0);
  printf("%lx\n", fw_arg1);
  printf("%lx\n", fw_arg2);
  printf("%lx\n", fw_arg3);

  while (1)
    ;
  // TODO extract acpi
}
