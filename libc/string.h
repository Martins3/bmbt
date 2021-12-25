#ifndef STRING_H_HA9F2PM3
#define STRING_H_HA9F2PM3

#include <features.h>

#include <uapi/libc.h>

#define __NEED_size_t
#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) ||                      \
    defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define __NEED_locale_t
#endif

#include <bits/alltypes.h>

void *memcpy(void *__restrict, const void *__restrict, size_t);
void *memmove(void *, const void *, size_t);
void *memset(void *, int, size_t);
int memcmp(const void *, const void *, size_t);
void *memchr(const void *, int, size_t);

char *strcpy(char *__restrict, const char *__restrict);
char *strncpy(char *__restrict, const char *__restrict, size_t);

char *strcat(char *__restrict, const char *__restrict);
char *strncat(char *__restrict, const char *__restrict, size_t);
char *strstr(const char *, const char *);

int strcmp(const char *, const char *);
int strncmp(const char *, const char *, size_t);

size_t strlen(const char *);

#endif /* end of include guard: STRING_H_HA9F2PM3 */
