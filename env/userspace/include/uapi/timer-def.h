#ifndef TIMER_DEF_H_GW9JRSWL
#define TIMER_DEF_H_GW9JRSWL

#include <signal.h>
#include <sys/time.h>
#include <time.h>

// void (*sa_sigaction) (int, siginfo_t *, void *);
typedef void (*TimerHandler)(int sig, siginfo_t *si, void *uc);
typedef timer_t timer_id;

#endif /* end of include guard: TIMER_DEF_H_GW9JRSWL */
