#ifdef ENV_USERSPACE
#define PAGESIZE 0x4000
#elif defined(ENV_KERNEL)
#include <page.h>
#else
#error "unsupported arch"
#endif
