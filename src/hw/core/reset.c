#include "../../../include/sysemu/reset.h"
#include "../../../include/qemu/queue.h"
#include "../../../include/types.h"
#include "../../tcg/glib_stub.h"

/* reset/shutdown handler */

typedef struct QEMUResetEntry {
  QTAILQ_ENTRY(QEMUResetEntry) entry;
  QEMUResetHandler *func;
  void *opaque;
} QEMUResetEntry;

static QTAILQ_HEAD(, QEMUResetEntry)
    reset_handlers = QTAILQ_HEAD_INITIALIZER(reset_handlers);

void qemu_register_reset(QEMUResetHandler *func, void *opaque) {
  QEMUResetEntry *re = g_malloc0(sizeof(QEMUResetEntry));

  re->func = func;
  re->opaque = opaque;
  QTAILQ_INSERT_TAIL(&reset_handlers, re, entry);
}

void qemu_unregister_reset(QEMUResetHandler *func, void *opaque) {
  QEMUResetEntry *re;

  QTAILQ_FOREACH(re, &reset_handlers, entry) {
    if (re->func == func && re->opaque == opaque) {
      QTAILQ_REMOVE(&reset_handlers, re, entry);
      g_free(re);
      return;
    }
  }
}

void qemu_devices_reset(void) {
  QEMUResetEntry *re, *nre;

  /* reset all devices */
  QTAILQ_FOREACH_SAFE(re, &reset_handlers, entry, nre) { re->func(re->opaque); }
}
