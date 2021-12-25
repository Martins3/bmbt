#ifndef TIME_H_NQOKK2GV
#define TIME_H_NQOKK2GV

#include <features.h>

#include <uapi/libc.h>

#define __NEED_size_t
#define __NEED_time_t
#define __NEED_clock_t
#define __NEED_struct_timespec

#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) ||                      \
    defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define __NEED_clockid_t
#define __NEED_timer_t
#define __NEED_pid_t
#define __NEED_locale_t
#endif

#include <bits/alltypes.h>

#if defined(_BSD_SOURCE) || defined(_GNU_SOURCE)
#define __tm_gmtoff tm_gmtoff
#define __tm_zone tm_zone
#endif

struct tm {
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
  long __tm_gmtoff;
  const char *__tm_zone;
};

clock_t clock(void);
time_t time(time_t *);
double difftime(time_t, time_t);
time_t mktime(struct tm *);
size_t strftime(char *__restrict, size_t, const char *__restrict,
                const struct tm *__restrict);
struct tm *gmtime(const time_t *);
struct tm *localtime(const time_t *);
char *asctime(const struct tm *);
char *ctime(const time_t *);
int timespec_get(struct timespec *, int);

#define CLOCKS_PER_SEC 1000000L

#define TIME_UTC 1

#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) ||                      \
    defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE)

size_t strftime_l(char *__restrict, size_t, const char *__restrict,
                  const struct tm *__restrict, locale_t);

struct tm *gmtime_r(const time_t *__restrict, struct tm *__restrict);
struct tm *localtime_r(const time_t *__restrict, struct tm *__restrict);
char *asctime_r(const struct tm *__restrict, char *__restrict);
char *ctime_r(const time_t *, char *);

void tzset(void);

struct itimerspec {
  struct timespec it_interval;
  struct timespec it_value;
};

#endif /* end of include guard: TIME_H_NQOKK2GV */
#endif
