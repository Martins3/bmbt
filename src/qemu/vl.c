#include "../../include/qemu/notify.h"
#include "../../include/sysemu/tcg.h"
#include <stdbool.h>

bool machine_init_done;

static NotifierList machine_init_done_notifiers =
    NOTIFIER_LIST_INITIALIZER(machine_init_done_notifiers);

void qemu_add_machine_init_done_notifier(Notifier *notify) {
  notifier_list_add(&machine_init_done_notifiers, notify);
  if (machine_init_done) {
    notify->notify(notify, NULL);
  }
}

void qemu_init() { tcg_init(); }
