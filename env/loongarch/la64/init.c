#include <asm/fw.h>
#include <asm/mach-la64/loongson.h>

void early_init(void) {
  fw_init_cmdline();
  fw_init_env();
}

void platform_init(void) { fw_init_memory(); }
