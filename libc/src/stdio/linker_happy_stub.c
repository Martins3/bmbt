#include <setjmp.h>
#include <stdio.h>
#include <time.h>

FILE *fopen(const char *__restrict f, const char *__restrict f2) {
  return NULL;
}
int fclose(FILE *f) { return 0; }
size_t fread(void *__restrict f, size_t f1, size_t f2, FILE *__restrict f3) {
  return 0;
}
int fseek(FILE *f, long f1, int f2) { return 0; }
long ftell(FILE *f) { return 0; }
int fflush(FILE *f) { return 0; }

_Noreturn void siglongjmp(sigjmp_buf buf, int val) {
  for (;;) {
  }
}

int sigsetjmp(sigjmp_buf buf, int val) { return 0; }

int timer_settime(timer_t a, int b, const struct itimerspec *__restrict c,
                  struct itimerspec *__restrict d) {
  return 9;
}

int timer_create(clockid_t a, struct sigevent *__restrict b,
                 timer_t *__restrict c) {
  return 0;
}
