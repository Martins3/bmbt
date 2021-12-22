#include <assert.h>
#include <exec/hwaddr.h>
#include <stdlib.h>
#include <uapi/env.h>

void *alloc_ram(hwaddr size) {
  void *host = malloc(size);
  assert(host != NULL);
  return host;
}

__attribute__((__noreturn__)) void siglongjmp(sigjmp_buf buf, int ret) {
  longjmp(buf, ret);
  abort();
}

int sigsetjmp(sigjmp_buf buf, int mask) {
  assert(mask == 0);
  return setjmp(buf);
}

void print_stack_trace(void) {
  // edk2 libc doesn't implement "execinfo.h"
}

struct tm *gmtime_r(const time_t *t, struct tm *tm) {
  struct tm *rtm = gmtime(t);
  if (rtm == NULL)
    return NULL;
  *tm = *rtm;
  return tm;
}

struct tm *localtime_r(const time_t *t, struct tm *tm) {
  struct tm *rtm = localtime(t);
  if (rtm == NULL)
    return rtm;
  *tm = *rtm;
  return tm;
}

int qemu_mprotect_none(void *addr, size_t size) { return 0; }

void *qemu_memalign(size_t alignment, size_t size) { return malloc(size); }
