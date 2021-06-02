#include "../include/addrspace.h"
#include "../include/panic.h"
#include "../include/types.h"

void *duck_memcpy(void *dest, const void *src, size_t count) {
  char *tmp = dest;
  const char *s = src;

  while (count--)
    *tmp++ = *s++;
  return dest;
}

size_t duck_strlen(const char *s) {
  const char *sc;

  for (sc = s; *sc != '\0'; ++sc)
    /* nothing */;
  return sc - s;
}

size_t duck_strnlen(const char *s, size_t count) {
  const char *sc;

  for (sc = s; count-- && *sc != '\0'; ++sc)
    /* nothing */;
  return sc - s;
}

void prom_putchar(char c) {
#define LOONGSON_REG_BASE 0x1fe00000
  *(char *)(TO_UNCAC(LOONGSON_REG_BASE + 0x1e0)) = c;
}

void early_console_write(const char *s, unsigned n) {
  while (n-- && *s) {
    if (*s == '\n')
      prom_putchar('\r');
    prom_putchar(*s);
    s++;
  }
}

size_t duck_strlcat(char *dest, const char *src, size_t count) {
  size_t dsize = duck_strlen(dest);
  size_t len = duck_strlen(src);
  size_t res = dsize + len;

  /* This would be a bug */
  BUG_ON(dsize >= count);

  dest += dsize;
  count -= dsize;
  if (len >= count)
    len = count - 1;
  duck_memcpy(dest, src, len);
  dest[len] = 0;
  return res;
}

int duck_memcmp(const void *cs, const void *ct, size_t count) {
  const unsigned char *su1, *su2;
  int res = 0;

  for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
    if ((res = *su1 - *su2) != 0)
      break;
  return res;
}
