#ifndef ENV_H_RI3Y14ON
#define ENV_H_RI3Y14ON

#include <setjmp.h>
#include <time.h>
/**
 * jmp_buf and sigjmp_buf are actually the same thing
 *
 * see musl/include/setjmp.h
 */
typedef jmp_buf sigjmp_buf;
int sigsetjmp(sigjmp_buf, int);
__attribute__((__noreturn__)) void siglongjmp(sigjmp_buf, int);

// Notice : in glibc, uint64_t is long int, but in edk2, it's long long int, so
// PRIx64 is llx
#define PRIx64 "llx"
#define PRIu32 "u"
#define PRIu64 "llu"
#define PRId64 "lld"
#define PRIx16 "x"
#define PRIx64 "llx"
#define UEFI_APPLICATION 1

struct tm *gmtime_r(const time_t *t, struct tm *tm);

struct tm *localtime_r(const time_t *t, struct tm *tm);

#define alloca(size) __builtin_alloca(size)

int qemu_mprotect_none(void *addr, size_t size);

void *qemu_memalign(size_t alignment, size_t size);

static inline int env_getpagesize(void) {
  // 16k
  return 0x4000;
}
#endif /* end of include guard: ENV_H_RI3Y14ON */
