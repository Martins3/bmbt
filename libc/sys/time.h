#ifndef TIME_H_OA6NCVTK
#define TIME_H_OA6NCVTK

#include <features.h>

#define __NEED_time_t
#define __NEED_suseconds_t
#define __NEED_struct_timeval
#include <bits/alltypes.h>

#ifdef BMBT
#include <sys/select.h>
#endif

int gettimeofday(struct timeval *__restrict, void *__restrict);

#define ITIMER_REAL 0
#define ITIMER_VIRTUAL 1
#define ITIMER_PROF 2

struct itimerval {
  struct timeval it_interval;
  struct timeval it_value;
};

int getitimer(int, struct itimerval *);
int setitimer(int, const struct itimerval *__restrict,
              struct itimerval *__restrict);

#endif /* end of include guard: TIME_H_OA6NCVTK */
