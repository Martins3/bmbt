#ifndef SETJMP_H_TRPCQU6N
#define SETJMP_H_TRPCQU6N

#include <bits/setjmp.h>
#include <features.h>

typedef struct __jmp_buf_tag {
  __jmp_buf __jb;
  unsigned long __fl;
  unsigned long __ss[128 / sizeof(long)];
} jmp_buf[1];

int setjmp(jmp_buf);
_Noreturn void longjmp(jmp_buf, int);

#define setjmp setjmp
#endif /* end of include guard: SETJMP_H_TRPCQU6N */
