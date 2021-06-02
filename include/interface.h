#ifndef INTERFACE_H_I5WZDPYD
#define INTERFACE_H_I5WZDPYD

#include "types.h"


void early_console_write(const char *s, unsigned n);

int printf(const char *fmt, ...);

/*
 * Registers a0, a1, a3 and a4 as passed to the kernel entry by firmware
 */
extern unsigned long fw_arg0, fw_arg1, fw_arg2, fw_arg3;

// string.c
void *memcpy(void *dest, const void *src, size_t count);
size_t strlen(const char *s);
size_t strnlen(const char *s, size_t count);
size_t strlcat(char *dest, const char *src, size_t count);

#endif /* end of include guard: INTERFACE_H_I5WZDPYD */
