#ifndef UNISTD_H_8IPA2YR7
#define UNISTD_H_8IPA2YR7

#include <features.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#ifdef __cplusplus
#define NULL 0L
#else
#define NULL ((void *)0)
#endif

#define __NEED_size_t
#define __NEED_ssize_t
#define __NEED_uid_t
#define __NEED_gid_t
#define __NEED_off_t
#define __NEED_pid_t
#define __NEED_intptr_t
#define __NEED_useconds_t

#include <bits/alltypes.h>

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
int getpagesize(void);
#endif

unsigned sleep(unsigned);
pid_t getpid(void);

#endif /* end of include guard: UNISTD_H_8IPA2YR7 */
