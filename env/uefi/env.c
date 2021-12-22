#include <assert.h>
#include <exec/hwaddr.h>
#include <stdlib.h>
#include <uapi/env.h>

void *alloc_ram(hwaddr size) {
  void *host = malloc(size);
  assert(host != NULL);
  return host;
}
