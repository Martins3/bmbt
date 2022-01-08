#ifndef CHAR_FE_H_YRN3KVYC
#define CHAR_FE_H_YRN3KVYC

#include "chardev/char.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

/* This is the backend as seen by frontend, the actual backend is
 * Chardev */
struct CharBackend {
  Chardev *chr;
#ifdef BMBT
  IOEventHandler *chr_event;
  IOCanReadHandler *chr_can_read;
  IOReadHandler *chr_read;
  BackendChangeHandler *chr_be_change;
  void *opaque;
  int tag;
  int fe_open;
#endif
};
#endif /* end of include guard: CHAR_FE_H_YRN3KVYC */

/**
 * qemu_chr_fe_ioctl:
 * @cmd: see CHR_IOCTL_*
 * @arg: the data associated with @cmd
 *
 * Issue a device specific ioctl to a backend.  This function is thread-safe.
 *
 * Returns: if @cmd is not supported by the backend or there is no
 *          associated Chardev, -ENOTSUP, otherwise the return
 *          value depends on the semantics of @cmd
 */
static inline int qemu_chr_fe_ioctl(CharBackend *be, int cmd, void *arg) {
  return -ENOTSUP;
}

/**
 * qemu_chr_fe_accept_input:
 *
 * Notify that the frontend is ready to receive data
 */
static inline void qemu_chr_fe_accept_input(CharBackend *be) {
  // in bmbt, guest can't receive data
}

static inline bool qemu_chr_fe_init(CharBackend *b, Chardev *s) {
  assert(b != NULL && s != NULL);
  s->be = b;
  b->chr = s;
  return true;
}
