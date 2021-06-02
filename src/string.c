#include "../include/types.h"
#include "../include/addrspace.h"

size_t strnlen(const char *s, size_t count) {
  const char *sc;

  for (sc = s; count-- && *sc != '\0'; ++sc)
    /* nothing */;
  return sc - s;
}

void prom_putchar(char c) { 
#define LOONGSON_REG_BASE	0x1fe00000
  *(char *)(TO_UNCAC(LOONGSON_REG_BASE + 0x1e0)) = c;
}

void early_console_write(const char *s, unsigned n)
{
	while (n-- && *s) {
		if (*s == '\n')
			prom_putchar('\r');
		prom_putchar(*s);
		s++;
	}
}
