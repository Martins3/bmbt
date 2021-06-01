#include "../header/types.h"

size_t strnlen(const char *s, size_t count) {
  const char *sc;

  for (sc = s; count-- && *sc != '\0'; ++sc)
    /* nothing */;
  return sc - s;
}

void prom_putchar(char c) { *(char *)(0x9000000000000000 + 0x1fe001e0) = c; }

void early_console_write(const char *s, unsigned n)
{
	while (n-- && *s) {
		if (*s == '\n')
			prom_putchar('\r');
		prom_putchar(*s);
		s++;
	}
}
