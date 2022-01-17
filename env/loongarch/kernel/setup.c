#include <asm/bootinfo.h>
#include <asm/cpu-info.h>

unsigned long fw_arg0, fw_arg1, fw_arg2, fw_arg3;
extern void early_init(void);

void setup_arch(char **cmdline_p) {
  cpu_probe();
  early_init();
  platform_init();
}
