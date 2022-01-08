#include "stdio_impl.h"
#include "syscall.h"
#include <unistd.h>

static inline off_t __lseek(int fd, off_t offset, int whence) {
#ifdef SYS__llseek
  off_t result;
  return syscall(SYS__llseek, fd, offset >> 32, offset, &result, whence)
             ? -1
             : result;
#else
  return syscall(SYS_lseek, fd, offset, whence);
#endif
}

off_t __stdio_seek(FILE *f, off_t off, int whence) {
  return __lseek(f->fd, off, whence);
}
