#include "chardev/char-fe.h"
#include "chardev/char.h"
#include "qemu/error-report.h"
#include "qemu/osdep.h"
#include "sysemu/replay.h"
#include <errno.h>

// this function is copy from char.c
bool qemu_chr_has_feature(Chardev *chr, ChardevFeature feature) {
  return test_bit(feature, chr->features);
}

int qemu_chr_fe_ioctl(CharBackend *be, int cmd, void *arg) {
  Chardev *s = be->chr;
  int res;

  if (!s || !CHARDEV_GET_CLASS(s)->chr_ioctl || qemu_chr_replay(s)) {
    res = -ENOTSUP;
  } else {
    g_assert_not_reached();
    res = CHARDEV_GET_CLASS(s)->chr_ioctl(s, cmd, arg);
  }

  return res;
}

void qemu_chr_fe_accept_input(CharBackend *be) {
  Chardev *s = be->chr;
  ChardevClass *class = CHARDEV_GET_CLASS(s);

  if (!s) {
    return;
  }

  if (class->chr_accept_input) {
    class->chr_accept_input(s);
  }
  // g_assert_not_reached();
  // qemu_notify_event();
}

void qemu_chr_fe_set_handlers_full(
    CharBackend *b, IOCanReadHandler *fd_can_read, IOReadHandler *fd_read,
    IOEventHandler *fd_event, BackendChangeHandler *be_change, void *opaque,
    bool set_open, bool sync_state) {
  Chardev *s;
  int fe_open;

  s = b->chr;
  if (!s) {
    return;
  }

  // for serial.c, opaque and so on is always true
  // if (!opaque && !fd_can_read && !fd_read && !fd_event) {
  //     fe_open = 0;
  //     remove_fd_in_watch(s);
  // } else {
  //     fe_open = 1;
  // }
  fe_open = 1;
  b->chr_can_read = fd_can_read;
  b->chr_read = fd_read;
  b->chr_event = fd_event;
  b->chr_be_change = be_change;
  b->opaque = opaque;

  // for serial, context is null
  // qemu_chr_be_update_read_handlers(s, context);

  if (set_open) {
    qemu_chr_fe_set_open(b, fe_open);
  }

  // we don't need mux
  // if (fe_open) {
  //     qemu_chr_fe_take_focus(b);
  //     /* We're connecting to an already opened device, so let's make sure we
  //        also get the open event */
  //     if (sync_state && s->be_open) {
  //         qemu_chr_be_event(s, CHR_EVENT_OPENED);
  //     }
  // }
}

void qemu_chr_fe_set_handlers(CharBackend *b, IOCanReadHandler *fd_can_read,
                              IOReadHandler *fd_read, IOEventHandler *fd_event,
                              BackendChangeHandler *be_change, void *opaque,
                              bool set_open) {
  qemu_chr_fe_set_handlers_full(b, fd_can_read, fd_read, fd_event, be_change,
                                opaque, set_open, true);
}

void qemu_chr_fe_set_open(CharBackend *be, int fe_open) {
  Chardev *chr = be->chr;

  if (!chr) {
    return;
  }

  if (be->fe_open == fe_open) {
    return;
  }
  be->fe_open = fe_open;
  if (CHARDEV_GET_CLASS(chr)->chr_set_fe_open) {
    CHARDEV_GET_CLASS(chr)->chr_set_fe_open(chr, fe_open);
  }
}
