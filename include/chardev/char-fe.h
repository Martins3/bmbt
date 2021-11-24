#ifndef QEMU_CHAR_FE_H
#define QEMU_CHAR_FE_H

#include "../../src/tcg/glib_stub.h"
#include "char.h"
#include "qemu/main-loop.h"

typedef void IOEventHandler(void *opaque, int event);
typedef int BackendChangeHandler(void *opaque);

/* This is the backend as seen by frontend, the actual backend is
 * Chardev */
/* This structure is introduced so that all frontend will be moved to hold
 * and use a CharBackend. This will allow to better track char usage and
 * allocation, and help prevent some memory leaks or corruption. */
struct CharBackend {
  Chardev *chr;
  IOEventHandler *chr_event;
  IOCanReadHandler *chr_can_read;
  IOReadHandler *chr_read;
  BackendChangeHandler *chr_be_change;
  void *opaque;
  int tag;
  int fe_open;
};

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
int qemu_chr_fe_ioctl(CharBackend *be, int cmd, void *arg);

/**
 * qemu_chr_fe_deinit:
 * @b: a CharBackend
 * @del: if true, delete the chardev backend
 *
 * Dissociate the CharBackend from the Chardev.
 *
 * Safe to call without associated Chardev.
 */
void qemu_chr_fe_deinit(CharBackend *b, bool del);

/**
 * qemu_chr_fe_set_open:
 *
 * Set character frontend open status.  This is an indication that the
 * front end is ready (or not) to begin doing I/O.
 * Without associated Chardev, do nothing.
 */
void qemu_chr_fe_set_open(CharBackend *be, int fe_open);

/**
 * qemu_chr_fe_accept_input:
 *
 * Notify that the frontend is ready to receive data
 */
void qemu_chr_fe_accept_input(CharBackend *be);

/**
 * qemu_chr_fe_set_handlers_full:
 * @b: a CharBackend
 * @fd_can_read: callback to get the amount of data the frontend may
 *               receive
 * @fd_read: callback to receive data from char
 * @fd_event: event callback
 * @be_change: backend change callback; passing NULL means hot backend change
 *             is not supported and will not be attempted
 * @opaque: an opaque pointer for the callbacks
 * @context: a main loop context or NULL for the default
 * @set_open: whether to call qemu_chr_fe_set_open() implicitely when
 * any of the handler is non-NULL
 * @sync_state: whether to issue event callback with updated state
 *
 * Set the front end char handlers. The front end takes the focus if
 * any of the handler is non-NULL.
 *
 * Without associated Chardev, nothing is changed.
 */
void qemu_chr_fe_set_handlers_full(
    CharBackend *b, IOCanReadHandler *fd_can_read, IOReadHandler *fd_read,
    IOEventHandler *fd_event, BackendChangeHandler *be_change, void *opaque,
    bool set_open, bool sync_state);

/**
 * qemu_chr_fe_set_handlers:
 *
 * Version of qemu_chr_fe_set_handlers_full() with sync_state = true.
 */
void qemu_chr_fe_set_handlers(CharBackend *b, IOCanReadHandler *fd_can_read,
                              IOReadHandler *fd_read, IOEventHandler *fd_event,
                              BackendChangeHandler *be_change, void *opaque,
                              bool set_open);

#endif
