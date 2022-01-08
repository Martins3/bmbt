#ifndef STDIO_IMPL_H_QVZME5LC
#define STDIO_IMPL_H_QVZME5LC
#include "syscall.h"
#include <features.h>
#include <stdio.h>

#define UNGET 8

#define FFINALLOCK(f) ((f)->lock >= 0 ? __lockfile((f)) : 0)
#define FLOCK(f) int __need_unlock = ((f)->lock >= 0 ? __lockfile((f)) : 0)
#define FUNLOCK(f)                                                             \
  do {                                                                         \
    if (__need_unlock)                                                         \
      __unlockfile((f));                                                       \
  } while (0)

#define F_PERM 1
#define F_NORD 4
#define F_NOWR 8
#define F_EOF 16
#define F_ERR 32
#define F_SVB 64
#define F_APP 128

struct _IO_FILE {
  unsigned flags;
  unsigned char *rpos, *rend;
  int (*close)(FILE *);
  unsigned char *wend, *wpos;
  unsigned char *mustbezero_1;
  unsigned char *wbase;
  size_t (*read)(FILE *, unsigned char *, size_t);
  size_t (*write)(FILE *, const unsigned char *, size_t);
  off_t (*seek)(FILE *, off_t, int);
  unsigned char *buf;
  size_t buf_size;
  FILE *prev, *next;
  int fd;
  int pipe_pid;
  long lockcount;
  int mode;
  volatile int lock;
  int lbf;
  void *cookie;
  off_t off;
  char *getln_buf;
  void *mustbezero_2;
  unsigned char *shend;
  off_t shlim, shcnt;
  FILE *prev_locked, *next_locked;
  struct __locale_struct *locale;
};

hidden void __unlockfile(FILE *);
hidden int __lockfile(FILE *);
hidden size_t __fwritex(const unsigned char *, size_t, FILE *);
hidden int __towrite(FILE *);

hidden size_t __stdio_write(FILE *, const unsigned char *, size_t);
hidden off_t __stdio_seek(FILE *, off_t, int);
hidden int __stdio_close(FILE *);

int __overflow(FILE *, int);
#define putc_unlocked(c, f)                                                    \
  ((((unsigned char)(c) != (f)->lbf && (f)->wpos != (f)->wend))                \
       ? *(f)->wpos++ = (unsigned char)(c)                                     \
       : __overflow((f), (unsigned char)(c)))

#endif /* end of include guard: STDIO_IMPL_H_QVZME5LC */
