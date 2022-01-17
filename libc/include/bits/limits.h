#ifdef ENV_USERSPACE
#define PAGESIZE 0x4000
#elif defined(ENV_KERNEL)
#include <linux/page.h>
#else
#error "unsupported arch"
#endif
