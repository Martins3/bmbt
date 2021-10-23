#ifndef OPTION_INT_H_HVEKQS1U
#define OPTION_INT_H_HVEKQS1U

#include "../types.h"
#include "./queue.h"
#include <string.h>

enum QemuOptType {
  QEMU_OPT_STRING = 0, /* no parsing (use string as-is) */
  QEMU_OPT_BOOL,   /* on/off                                               */
  QEMU_OPT_NUMBER, /* simple number                                        */
  QEMU_OPT_SIZE,   /* size, accepts (K)ilo, (M)ega, (G)iga, (T)era postfix */
};

typedef struct QemuOptDesc {
  const char *name;
  enum QemuOptType type;
  const char *help;
  const char *def_value_str;
} QemuOptDesc;

typedef struct QemuOptsList {
  const char *name;
  const char *implied_opt_name;
  bool merge_lists; /* Merge multiple uses of option into a single list? */
  QTAILQ_HEAD(, QemuOpts) head;
  QemuOptDesc desc[];
} QemuOptsList;

// option_int.h
struct QemuOpts;

typedef struct QemuOpt {
  char *name;
  char *str;

  const QemuOptDesc *desc;
  union {
    bool boolean;
    uint64_t uint;
  } value;

  struct QemuOpts *opts;
  QTAILQ_ENTRY(QemuOpt) next;
} QemuOpt;

typedef struct QemuOpts {
  char *id;
  QemuOptsList *list;
  // Location loc;
  QTAILQ_HEAD(, QemuOpt) head;
  // QTAILQ_ENTRY(QemuOpts) next;
} QemuOpts;

static inline QemuOpt *qemu_opt_find(QemuOpts *opts, const char *name) {
  QemuOpt *opt;

  QTAILQ_FOREACH_REVERSE(opt, &opts->head, next) {
    if (strcmp(opt->name, name) != 0)
      continue;
    return opt;
  }
  return NULL;
}

void qemu_opts_print(QemuOpts *opts, const char *separator);

#endif /* end of include guard: OPTION_INT_H_HVEKQS1U */
