#ifndef MMAN_H_A6CUPKSG
#define MMAN_H_A6CUPKSG

#include "../../../include/sys/mman.h"
#include <features.h>

hidden void __vm_wait(void);
hidden void __vm_lock(void);
hidden void __vm_unlock(void);

hidden void *__mmap(void *, size_t, int, int, int, off_t);
hidden int __munmap(void *, size_t);
hidden void *__mremap(void *, size_t, size_t, int, ...);
hidden int __madvise(void *, size_t, int);
hidden int __mprotect(void *, size_t, int);

hidden const unsigned char *__map_file(const char *, size_t *);

hidden char *__shm_mapname(const char *, char *);
#endif /* end of include guard: MMAN_H_A6CUPKSG */
