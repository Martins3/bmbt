#include <asm/bootinfo.h>
extern void early_init(void);
void setup_arch(char **cmdline_p) {
  early_init();
  platform_init();
}

unsigned long fw_arg0, fw_arg1, fw_arg2, fw_arg3;
