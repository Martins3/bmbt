#ifdef ENV_USERSPACE
#define PAGESIZE 0x4000
#elif defined(ENV_KERNEL)
#include <asm/page.h>
#else
#error "unsupported arch"
#endif

/*
 * TMP_TODO PAGESIZE 总是 0x4000 ，没有必要定义两次
 *
 * 而且既然需要使用 PS_SHIFT，那么就直接在 Host 中处理
 * void *p = mmap(0, x << PAGE_SHIFT, PROT_READ | PROT_WRITE,
 *                        ^~~~~~~~~~
 *                        PS_SHIFT
 */
