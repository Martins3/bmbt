#ifndef SIGNAL_H_LCKDKED3
#define SIGNAL_H_LCKDKED3

#include <features.h>

#include <bits/signal.h>

#define SIG_ERR ((void (*)(int)) - 1)
#define SIG_DFL ((void (*)(int))0)
#define SIG_IGN ((void (*)(int))1)

typedef int sig_atomic_t;

void (*signal(int, void (*)(int)))(int);
int raise(int);

#endif /* end of include guard: SIGNAL_H_LCKDKED3 */
