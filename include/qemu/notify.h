#ifndef NOTIFY_H_B3HZ5KGJ
#define NOTIFY_H_B3HZ5KGJ

#include "../types.h"
#include "queue.h"

typedef struct Notifier Notifier;

struct Notifier {
  void (*notify)(Notifier *notifier, void *data);
  QLIST_ENTRY(Notifier) node;
};

typedef struct NotifierList {
  QLIST_HEAD(, Notifier) notifiers;
} NotifierList;

#define NOTIFIER_LIST_INITIALIZER(head)                                        \
  { QLIST_HEAD_INITIALIZER((head).notifiers) }

static inline void notifier_list_add(NotifierList *list, Notifier *notifier) {
  QLIST_INSERT_HEAD(&list->notifiers, notifier, node);
}

#endif /* end of include guard: NOTIFY_H_B3HZ5KGJ */
