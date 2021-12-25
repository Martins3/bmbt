#ifndef ENV_H_RI3Y14ON
#define ENV_H_RI3Y14ON

#define USE_UEFI_LIBC
#ifdef USE_UEFI_LIBC
#include "uefi-libc-fixup.h"
#endif

int qemu_mprotect_none(void *addr, size_t size);

void *qemu_memalign(size_t alignment, size_t size);
#endif /* end of include guard: ENV_H_RI3Y14ON */
