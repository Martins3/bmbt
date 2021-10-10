#include "../../include/qemu/osdep.h"
#include "../../include/qemu/error-report.h"
#include "../tcg/glib_stub.h"
#include <sys/mman.h>

static int qemu_mprotect__osdep(void *addr, size_t size, int prot) {
  g_assert(!((uintptr_t)addr & ~qemu_real_host_page_mask));
  g_assert(!(size & ~qemu_real_host_page_mask));

#ifdef _WIN32
  DWORD old_protect;

  if (!VirtualProtect(addr, size, prot, &old_protect)) {
    error_report("%s: VirtualProtect failed with error code %ld", __func__,
                 GetLastError());
    return -1;
  }
  return 0;
#else
  if (mprotect(addr, size, prot)) {
    error_report("%s: mprotect failed", __func__);
    return -1;
  }
  return 0;
#endif
}

int qemu_mprotect_none(void *addr, size_t size) {
#ifdef _WIN32
  return qemu_mprotect__osdep(addr, size, PAGE_NOACCESS);
#else
  return qemu_mprotect__osdep(addr, size, PROT_NONE);
#endif
}

// ------------- originally defined in /util/oslib-posix.c begin --------------
void *qemu_oom_check(void *ptr) {
  if (ptr == NULL) {
    fprintf(stderr, "Failed to allocate memory\n");
    abort();
  }
  return ptr;
}

void *qemu_try_memalign(size_t alignment, size_t size) {
  void *ptr;

  if (alignment < sizeof(void *)) {
    alignment = sizeof(void *);
  }

#if defined(CONFIG_POSIX_MEMALIGN)
  int ret;
  ret = posix_memalign(&ptr, alignment, size);
  if (ret != 0) {
    ptr = NULL;
  }
#elif defined(CONFIG_BSD)
  ptr = valloc(size);
#else
  ptr = memalign(alignment, size);
#endif
  // fuck_trace_qemu_memalign(alignment, size, ptr);
  return ptr;
}

void *qemu_memalign(size_t alignment, size_t size) {
  return qemu_oom_check(qemu_try_memalign(alignment, size));
}
// ------------- originally defined in /util/oslib-posix.c begin --------------
