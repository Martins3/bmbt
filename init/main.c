#include "../header/config.h"
#include "../header/interface.h"

unsigned long fw_arg0, fw_arg1, fw_arg2, fw_arg3;

unsigned char kernel_stack[_THREAD_SIZE];

// TODO local_irq_disable();
//
// TODO in linux kernel, how they write mmio port ?
void start_kernel() {
  while (1) {
    printf("hello this%x\n", 0x1233);
  }

  // TODO extract acpi
}
