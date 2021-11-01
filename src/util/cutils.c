#include <time.h>
void pstrcpy(char *buf, int buf_size, const char *str) {
  int c;
  char *q = buf;

  if (buf_size <= 0)
    return;

  for (;;) {
    c = *str++;
    if (c == 0 || q >= buf + buf_size - 1)
      break;
    *q++ = c;
  }
  *q = '\0';
}

time_t mktimegm(struct tm *tm) {
  time_t t;
  int y = tm->tm_year + 1900, m = tm->tm_mon + 1, d = tm->tm_mday;
  if (m < 3) {
    m += 12;
    y--;
  }
  t = 86400ULL *
      (d + (153 * m - 457) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 719469);
  t += 3600 * tm->tm_hour + 60 * tm->tm_min + tm->tm_sec;
  return t;
}
