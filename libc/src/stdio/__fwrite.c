#include "stdio_impl.h"

int __towrite(FILE *f) {
  f->mode |= f->mode - 1;
  if (f->flags & F_NOWR) {
    f->flags |= F_ERR;
    return EOF;
  }
  /* Clear read buffer (easier than summoning nasal demons) */
  f->rpos = f->rend = 0;

  /* Activate write through the buffer. */
  f->wpos = f->wbase = f->buf;
  f->wend = f->buf + f->buf_size;

  return 0;
}
