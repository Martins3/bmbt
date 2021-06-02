#ifndef INTERFACE_H_I5WZDPYD
#define INTERFACE_H_I5WZDPYD

#include "types.h"
#include "panic.h"

void early_console_write(const char *s, unsigned n);


/*
 * Registers a0, a1, a3 and a4 as passed to the kernel entry by firmware
 */
extern unsigned long fw_arg0, fw_arg1, fw_arg2, fw_arg3;

// printf.c
int duck_printf(const char *fmt, ...);

// string.c
void *duck_memcpy(void *dest, const void *src, size_t count);
size_t duck_strlen(const char *s);
size_t duck_strnlen(const char *s, size_t count);
size_t duck_strlcat(char *dest, const char *src, size_t count);
int duck_memcmp(const void *cs, const void *ct, size_t count);

// acpi.c
void fw_init_cmdline(void);
void prom_init_env(void);

// mem.c
void mem_init(void);

#endif /* end of include guard: INTERFACE_H_I5WZDPYD */
