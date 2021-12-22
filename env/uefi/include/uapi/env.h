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
// Notice : in glibc, uint64_t is long int, but in edk2, it's long long int, so
// PRIx64 is llx
#define PRIx64 "llx"
#define PRIu32 "u"
#define PRIu64 "llu"
#define PRId64 "lld"
#define PRIx16 "x"
#define UEFI_APPLICATION 1

static inline struct tm *gmtime_r(const time_t *t, struct tm *tm) {
  struct tm *rtm = gmtime(t);
  *tm = *rtm;
  return tm;
}

#endif /* end of include guard: ENV_H_RI3Y14ON */
