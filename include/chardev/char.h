#ifndef QEMU_CHAR_H
#define QEMU_CHAR_H

#include "char-fe.h"
#include "qemu/bitmap.h"
#include "qemu/option.h"
#include "qemu/thread.h"

typedef enum {
  CHR_EVENT_BREAK,   /* serial break char */
  CHR_EVENT_OPENED,  /* new connection established */
  CHR_EVENT_MUX_IN,  /* mux-focus was set to this terminal */
  CHR_EVENT_MUX_OUT, /* mux-focus will move on */
  CHR_EVENT_CLOSED   /* connection closed.  NOTE: currently this event
                      * is only bound to the read port of the chardev.
                      * Normally the read port and write port of a
                      * chardev should be the same, but it can be
                      * different, e.g., for fd chardevs, when the two
                      * fds are different.  So when we received the
                      * CLOSED event it's still possible that the out
                      * port is still open.  TODO: we should only send
                      * the CLOSED event when both ports are closed.
                      */
} QEMUChrEvent;

#define CHR_READ_BUF_LEN 4096

typedef enum {
  /* Whether the chardev peer is able to close and
   * reopen the data channel, thus requiring support
   * for qemu_chr_wait_connected() to wait for a
   * valid connection */
  QEMU_CHAR_FEATURE_RECONNECTABLE,
  /* Whether it is possible to send/recv file descriptors
   * over the data channel */
  QEMU_CHAR_FEATURE_FD_PASS,
  /* Whether replay or record mode is enabled */
  QEMU_CHAR_FEATURE_REPLAY,
  /* Whether the gcontext can be changed after calling
   * qemu_chr_be_update_read_handlers() */
  QEMU_CHAR_FEATURE_GCONTEXT,

  QEMU_CHAR_FEATURE_LAST,
} ChardevFeature;

#define qemu_chr_replay(chr) qemu_chr_has_feature(chr, QEMU_CHAR_FEATURE_REPLAY)

typedef struct Chardev Chardev;

/* character device */
typedef struct CharBackend CharBackend;

typedef struct ChardevClass {
  // ObjectClass parent_class;

  bool internal; /* TODO: eventually use TYPE_USER_CREATABLE */
  // void (*parse)(QemuOpts *opts, ChardevBackend *backend);

  // void (*open)(Chardev *chr, ChardevBackend *backend,
  //              bool *be_opened);

  // int (*chr_write)(Chardev *s, const uint8_t *buf, int len);
  // int (*chr_sync_read)(Chardev *s, const uint8_t *buf, int len);
  // GSource *(*chr_add_watch)(Chardev *s, GIOCondition cond);
  // void (*chr_update_read_handler)(Chardev *s);
  int (*chr_ioctl)(Chardev *s, int cmd, void *arg);
  // int (*get_msgfds)(Chardev *s, int* fds, int num);
  // int (*set_msgfds)(Chardev *s, int *fds, int num);
  // int (*chr_add_client)(Chardev *chr, int fd);
  // int (*chr_wait_connected)(Chardev *chr);
  // void (*chr_disconnect)(Chardev *chr);
  void (*chr_accept_input)(Chardev *chr);
  // void (*chr_set_echo)(Chardev *chr, bool echo);
  void (*chr_set_fe_open)(Chardev *chr, int fe_open);
  // void (*chr_be_event)(Chardev *s, int event);
  /* Return 0 if succeeded, 1 if failed */
  // int (*chr_machine_done)(Chardev *chr);
} ChardevClass;

struct Chardev {
  // Object parent_obj;

  ChardevClass *class;
  QemuMutex chr_write_lock;
  CharBackend *be;
  char *label;
  char *filename;
  int logfd;
  int be_open;
  FILE *log;
  // GSource *gsource;
  // GMainContext *gcontext;
  DECLARE_BITMAP(features, QEMU_CHAR_FEATURE_LAST);
};

static inline ChardevClass *CHARDEV_GET_CLASS(Chardev *s) {
  duck_check(s->class != NULL);
  return s->class;
}

static inline void CHARDEV_SET_CLASS(Chardev *s, ChardevClass *class) {
  duck_check(class != NULL);
  s->class = class;
}

#endif
