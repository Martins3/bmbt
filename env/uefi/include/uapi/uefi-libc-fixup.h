#ifndef UEFI_LIBC_FIXUP_H_9415MBHN
#define UEFI_LIBC_FIXUP_H_9415MBHN

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

#define alloca(size) __builtin_alloca(size)
#include "../../../../libc/bits/limits.h"
// <math.h> should put above ../../../../libc/math.h
#include <math.h>
#include "../../../../libc/math.h"

struct tm *gmtime_r(const time_t *t, struct tm *tm);

struct tm *localtime_r(const time_t *t, struct tm *tm);

static inline int getpagesize(void) { return PAGESIZE; }

#endif /* end of include guard: UEFI_LIBC_FIXUP_H_9415MBHN */
