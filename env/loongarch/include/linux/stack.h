#ifndef STACK_H_WBBVTY6S
#define STACK_H_WBBVTY6S

#include <asm/page.h>
#define _THREAD_SIZE (4 * PAGE_SIZE)

#ifndef __ASSEMBLY__
extern unsigned char kernel_stack[_THREAD_SIZE];
#endif
#endif /* end of include guard: STACK_H_WBBVTY6S */
