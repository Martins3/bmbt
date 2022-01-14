#ifdef ENV_KERNEL
long kernel_syscall(long arg0, long arg1, long arg2, long arg3, long arg4,
                    long arg5, long arg6, long number);
#define ENV_SYSCALL "bl kernel_syscall"

#define __REGS_CLOBBERS                                                        \
  "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$ra", "memory"

#elif defined(ENV_USERSPACE)
#define ENV_SYSCALL "syscall 0"

#define __REGS_CLOBBERS                                                        \
  "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "memory"
#else
#error unspecified env
#endif

static inline long libc_syscall0(long number) {
  long int _sys_result;

  {
    register long int __a7 asm("$a7") = number;
    register long int __a0 asm("$a0");
    __asm__ volatile(ENV_SYSCALL "\n\t"
                     : "=r"(__a0)
                     : "r"(__a7)
                     : __REGS_CLOBBERS);
    _sys_result = __a0;
  }
  return _sys_result;
}

static inline long libc_syscall1(long number, long arg0) {
  long int _sys_result;

  {
    long int _arg0 = (long int)(arg0);
    register long int __a7 asm("$a7") = number;
    register long int __a0 asm("$a0") = _arg0;
    __asm__ volatile(ENV_SYSCALL "\n\t"
                     : "+r"(__a0)
                     : "r"(__a7)
                     : __REGS_CLOBBERS);
    _sys_result = __a0;
  }
  return _sys_result;
}

static inline long libc_syscall2(long number, long arg0, long arg1) {
  long int _sys_result;

  {
    long int _arg0 = (long int)(arg0);
    long int _arg1 = (long int)(arg1);
    register long int __a7 asm("$a7") = number;
    register long int __a0 asm("$a0") = _arg0;
    register long int __a1 asm("$a1") = _arg1;
    __asm__ volatile(ENV_SYSCALL "\n\t"
                     : "+r"(__a0)
                     : "r"(__a7), "r"(__a1)
                     : __REGS_CLOBBERS);
    _sys_result = __a0;
  }
  return _sys_result;
}

static inline long libc_syscall3(long number, long arg0, long arg1, long arg2) {
  long int _sys_result;

  {
    long int _arg0 = (long int)(arg0);
    long int _arg1 = (long int)(arg1);
    long int _arg2 = (long int)(arg2);
    register long int __a7 asm("$a7") = number;
    register long int __a0 asm("$a0") = _arg0;
    register long int __a1 asm("$a1") = _arg1;
    register long int __a2 asm("$a2") = _arg2;
    __asm__ volatile(ENV_SYSCALL "\n\t"
                     : "+r"(__a0)
                     : "r"(__a7), "r"(__a1), "r"(__a2)
                     : __REGS_CLOBBERS);
    _sys_result = __a0;
  }
  return _sys_result;
}

static inline long libc_syscall4(long number, long arg0, long arg1, long arg2,
                                 long arg3) {
  long int _sys_result;

  {
    long int _arg0 = (long int)(arg0);
    long int _arg1 = (long int)(arg1);
    long int _arg2 = (long int)(arg2);
    long int _arg3 = (long int)(arg3);
    register long int __a7 asm("$a7") = number;
    register long int __a0 asm("$a0") = _arg0;
    register long int __a1 asm("$a1") = _arg1;
    register long int __a2 asm("$a2") = _arg2;
    register long int __a3 asm("$a3") = _arg3;
    __asm__ volatile(ENV_SYSCALL "\n\t"
                     : "+r"(__a0)
                     : "r"(__a7), "r"(__a1), "r"(__a2), "r"(__a3)
                     : __REGS_CLOBBERS);
    _sys_result = __a0;
  }
  return _sys_result;
}

static inline long libc_syscall5(long number, long arg0, long arg1, long arg2,
                                 long arg3, long arg4) {
  long int _sys_result;

  {
    long int _arg0 = (long int)(arg0);
    long int _arg1 = (long int)(arg1);
    long int _arg2 = (long int)(arg2);
    long int _arg3 = (long int)(arg3);
    long int _arg4 = (long int)(arg4);
    register long int __a7 asm("$a7") = number;
    register long int __a0 asm("$a0") = _arg0;
    register long int __a1 asm("$a1") = _arg1;
    register long int __a2 asm("$a2") = _arg2;
    register long int __a3 asm("$a3") = _arg3;
    register long int __a4 asm("$a4") = _arg4;
    __asm__ volatile(ENV_SYSCALL "\n\t"
                     : "+r"(__a0)
                     : "r"(__a7), "r"(__a1), "r"(__a2), "r"(__a3), "r"(__a4)
                     : __REGS_CLOBBERS);
    _sys_result = __a0;
  }
  return _sys_result;
}

static inline long libc_syscall6(long number, long arg0, long arg1, long arg2,
                                 long arg3, long arg4, long arg5) {
  long int _sys_result;

  {
    long int _arg0 = (long int)(arg0);
    long int _arg1 = (long int)(arg1);
    long int _arg2 = (long int)(arg2);
    long int _arg3 = (long int)(arg3);
    long int _arg4 = (long int)(arg4);
    long int _arg5 = (long int)(arg5);
    register long int __a7 asm("$a7") = number;
    register long int __a0 asm("$a0") = _arg0;
    register long int __a1 asm("$a1") = _arg1;
    register long int __a2 asm("$a2") = _arg2;
    register long int __a3 asm("$a3") = _arg3;
    register long int __a4 asm("$a4") = _arg4;
    register long int __a5 asm("$a5") = _arg5;
    __asm__ volatile(ENV_SYSCALL "\n\t"
                     : "+r"(__a0)
                     : "r"(__a7), "r"(__a1), "r"(__a2), "r"(__a3), "r"(__a4),
                       "r"(__a5)
                     : __REGS_CLOBBERS);
    _sys_result = __a0;
  }
  return _sys_result;
}

static inline long libc_syscall7(long number, long arg0, long arg1, long arg2,
                                 long arg3, long arg4, long arg5, long arg6) {
  long int _sys_result;

  {
    long int _arg0 = (long int)(arg0);
    long int _arg1 = (long int)(arg1);
    long int _arg2 = (long int)(arg2);
    long int _arg3 = (long int)(arg3);
    long int _arg4 = (long int)(arg4);
    long int _arg5 = (long int)(arg5);
    long int _arg6 = (long int)(arg6);
    register long int __a7 asm("$a7") = number;
    register long int __a0 asm("$a0") = _arg0;
    register long int __a1 asm("$a1") = _arg1;
    register long int __a2 asm("$a2") = _arg2;
    register long int __a3 asm("$a3") = _arg3;
    register long int __a4 asm("$a4") = _arg4;
    register long int __a5 asm("$a5") = _arg5;
    register long int __a6 asm("$a6") = _arg6;
    __asm__ volatile(ENV_SYSCALL "\n\t"
                     : "+r"(__a0)
                     : "r"(__a7), "r"(__a1), "r"(__a2), "r"(__a3), "r"(__a4),
                       "r"(__a5), "r"(__a6)
                     : __REGS_CLOBBERS);
    _sys_result = __a0;
  }
  return _sys_result;
}
