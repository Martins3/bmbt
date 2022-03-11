#include <asm/cpu-features.h>
#include <asm/device.h>
#include <asm/io.h>

static void debugcon_putchar(char c) { writeb(c, LS_ISA_DEBUGCON_IO_BASE); }

void debugcon_puts(const char *s) {
  int num = 0;

  if (!cpu_has_hypervisor) {
    abort();
  }
  while (*s) {
    if (*s == '\n')
      debugcon_putchar('\r');
    debugcon_putchar(*s);
    s++;
    num++;
    if (num > 1000) {
      abort();
    }
  }
}
