#ifndef _ERROR_H_
#define _ERROR_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/xtm-qemu-config.h"

void print_stack_trace(void);

#define lsassert(cond)                                                  \
    do {                                                                \
        if (!(cond)) {                                                  \
            fprintf(stderr,                                             \
                    "\033[31m assertion failed in <%s> %s:%d \033[m\n", \
                    __FUNCTION__, __FILE__, __LINE__);                  \
            print_stack_trace();                                        \
            abort();                                                    \
        }                                                               \
    } while (0)

#define lsassertm(cond, ...)                                                  \
    do {                                                                      \
        if (!(cond)) {                                                        \
            fprintf(stderr, "\033[31m assertion failed in <%s> %s:%d \033[m", \
                    __FUNCTION__, __FILE__, __LINE__);                        \
            fprintf(stderr, __VA_ARGS__);                                     \
            print_stack_trace();                                              \
            abort();                                                          \
        }                                                                     \
    } while (0)

#ifdef CONFIG_SOFTMMU

#include "../include/types.h"

/* generate exception directly */
void tr_gen_excp_illegal_op_addr(ADDRX addr, int end);

/* lsassert to simply codes */
#define lsassert_illop(addr, cond)              \
    do {                                        \
        if (!(cond)) {                          \
            tr_gen_excp_illegal_op_addr(addr, 1);  \
        }                                       \
    } while(0)

#define lsassertm_illop(addr, cond, ...)                                       \
    do {                                                                      \
        if (!(cond)) {                                                        \
            fprintf(stderr, "\033[31m assertion failed in <%s> %s:%d \033[m", \
                    __FUNCTION__, __FILE__, __LINE__);                        \
            fprintf(stderr, __VA_ARGS__);                                     \
            tr_gen_excp_illegal_op_addr(addr, 1);                                \
        }                                                                     \
    } while(0)

#endif /* CONFIG_SOFTMMU */

#endif /* _ERROR_H_ */
