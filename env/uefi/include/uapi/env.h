#ifndef ENV_H_RI3Y14ON
#define ENV_H_RI3Y14ON

#include <setjmp.h>
/**
 * jmp_buf and sigjmp_buf are actually the same thing
 *
 * see musl/include/setjmp.h
 */
typedef jmp_buf sigjmp_buf;

#endif /* end of include guard: ENV_H_RI3Y14ON */
