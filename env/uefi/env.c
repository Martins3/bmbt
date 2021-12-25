#include <assert.h>
#include <exec/hwaddr.h>
#include <stdlib.h>
#include <uapi/env.h>

void *alloc_ram(hwaddr size) {
  void *host = malloc(size);
  assert(host != NULL);
  return host;
}

void print_stack_trace(void) {
  // edk2 libc doesn't implement "execinfo.h"
}


int qemu_mprotect_none(void *addr, size_t size) { return 0; }

void *qemu_memalign(size_t alignment, size_t size) { return malloc(size); }
