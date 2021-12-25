#ifndef STDLIB_H_HT0XBCOT
#define STDLIB_H_HT0XBCOT
#include <features.h>

#include <uapi/libc.h>

#define __NEED_size_t
#define __NEED_wchar_t

#include <bits/alltypes.h>

void *malloc(size_t);
void *calloc(size_t, size_t);
void *realloc(void *, size_t);
void free(void *);
_Noreturn void abort(void);
_Noreturn void exit(int);
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
void qsort(void *, size_t, size_t, int (*)(const void *, const void *));
void *bsearch(const void *, const void *, size_t, size_t,
              int (*)(const void *, const void *));

int rand(void);

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#include <alloca.h>
#endif

#endif /* end of include guard: STDLIB_H_HT0XBCOT */
