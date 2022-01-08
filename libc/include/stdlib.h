#ifndef STDLIB_H_HT0XBCOT
#define STDLIB_H_HT0XBCOT
#include <features.h>

#ifdef __cplusplus
#define NULL 0L
#error fuck
#else
#define NULL ((void *)0)
#endif

#define __NEED_size_t
#define __NEED_wchar_t

#include <bits/alltypes.h>

void *malloc(size_t);
void *calloc(size_t, size_t);
void *realloc(void *, size_t);
void free(void *);
_Noreturn void abort(void);
_Noreturn void exit(int);
_Noreturn void _Exit(int);
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
void qsort(void *, size_t, size_t, int (*)(const void *, const void *));
void *bsearch(const void *, const void *, size_t, size_t,
              int (*)(const void *, const void *));
int posix_memalign(void **, size_t, size_t);
void *aligned_alloc(size_t, size_t);
int rand(void);

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#include <alloca.h>
#endif

#endif /* end of include guard: STDLIB_H_HT0XBCOT */
