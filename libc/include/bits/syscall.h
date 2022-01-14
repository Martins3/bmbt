#ifndef SYSCALL_H_WX1BHDPV
#define SYSCALL_H_WX1BHDPV

// [interface 54]
#define SYS_brk 214
#define SYS_mmap 222
#define SYS_munmap 215
#define SYS_mprotect 226
#define SYS_madvise 233

#define SYS_exit_group 94
#define SYS_exit 93
#define SYS_clock_gettime 113
#define SYS_gettimeofday 169
#define SYS_writev 66

#define SYS_nanosleep 101
#define SYS_clock_nanosleep 115

// used by signal timer
#define SYS_getpid 172
#define SYS_rt_sigaction 134
#define SYS_rt_sigprocmask 135
#define SYS_kill 129
#define SYS_timer_settime 110
#define SYS_timer_create 107

#endif /* end of include guard: SYSCALL_H_WX1BHDPV */
