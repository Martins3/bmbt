#ifndef INTERFACE_H_I5WZDPYD
#define INTERFACE_H_I5WZDPYD

#include "types.h"

size_t strnlen(const char *s, size_t count);

void early_console_write(const char *s, unsigned n);

int printf(const char *fmt, ...);


#endif /* end of include guard: INTERFACE_H_I5WZDPYD */
