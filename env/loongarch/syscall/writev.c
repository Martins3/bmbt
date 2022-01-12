#include "internal.h"
#include <sys/uio.h>

long kernel_writev(long arg0, long arg1, long arg2, long arg3, long arg4,
                   long arg5, long arg6) {
  int fd = (int)arg0;
  struct iovec *iov = (struct iovec *)arg1;
  int iovcnt = (int)arg2;
  long ret = 0;
  // the only caller is __stdio_write
  duck_assert(fd == 2);
  duck_assert(iovcnt == 2);
  for (int i = 0; i < 2; ++i) {
    struct iovec *io = iov + i;
    early_console_write(io->iov_base, io->iov_len);
    ret += io->iov_len;
  }

  return ret;
}
