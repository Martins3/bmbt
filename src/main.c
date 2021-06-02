#include "../include/config.h"
#include "../include/interface.h"
#include "../include/loongarchregs.h"

unsigned long fw_arg0, fw_arg1, fw_arg2, fw_arg3;

unsigned char kernel_stack[_THREAD_SIZE];

// TODO local_irq_disable();
// the memory is setup or ?
void start_kernel() {

  duck_printf("%lx\n", fw_arg0);
  duck_printf("%lx\n", fw_arg1);
  duck_printf("%lx\n", fw_arg2);
  duck_printf("%lx\n", fw_arg3);

  prom_init_env();
  mem_init();

  duck_printf("over!");
  while (1)
    ;
  // TODO extract acpi
}
