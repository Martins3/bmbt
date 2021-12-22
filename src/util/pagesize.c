#include "../../include/qemu/osdep.h"
#include <uapi/env.h>
#include <unistd.h>

uintptr_t qemu_real_host_page_size;
intptr_t qemu_real_host_page_mask;

void init_real_host_page_size(void) {
  qemu_real_host_page_size = getpagesize();
  qemu_real_host_page_mask = -(intptr_t)qemu_real_host_page_size;
}
