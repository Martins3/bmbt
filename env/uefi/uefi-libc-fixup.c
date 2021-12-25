#include <uapi/env.h>

_Noreturn void siglongjmp(sigjmp_buf buf, int ret) {
  longjmp(buf, ret);
  abort();
}

int sigsetjmp(sigjmp_buf buf, int mask) {
  assert(mask == 0);
  return setjmp(buf);
}

struct tm *gmtime_r(const time_t *t, struct tm *tm) {
  struct tm *rtm = gmtime(t);
  if (rtm == NULL)
    return NULL;
  *tm = *rtm;
  return tm;
}

struct tm *localtime_r(const time_t *t, struct tm *tm) {
  struct tm *rtm = localtime(t);
  if (rtm == NULL)
    return rtm;
  *tm = *rtm;
  return tm;
}
