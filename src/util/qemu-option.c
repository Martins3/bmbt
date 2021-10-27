#include "../../include/qemu/option.h"
#include "../../src/i386/LATX/x86tomips-config-sys.h"
#include <inttypes.h>
#include <stdio.h>

/* print value, escaping any commas in value */
static void escaped_print(const char *value) {
  const char *ptr;

  for (ptr = value; *ptr; ++ptr) {
    if (*ptr == ',') {
      putchar(',');
    }
    putchar(*ptr);
  }
}

void qemu_opts_print(QemuOpts *opts, const char *separator) {
  QemuOpt *opt;
  QemuOptDesc *desc = opts->list->desc;
  const char *sep = "";

  if (opts->id) {
    printf("id=%s", opts->id); /* passed id_wellformed -> no commas */
    sep = separator;
  }

  if (desc[0].name == NULL) {
    QTAILQ_FOREACH(opt, &opts->head, next) {
      printf("%s%s=", sep, opt->name);
      escaped_print(opt->str);
      sep = separator;
    }
    return;
  }
  for (; desc && desc->name; desc++) {
    const char *value;
    opt = qemu_opt_find(opts, desc->name);

    value = opt ? opt->str : desc->def_value_str;
    if (!value) {
      continue;
    }
    if (desc->type == QEMU_OPT_STRING) {
      printf("%s%s=", sep, desc->name);
      escaped_print(value);
    } else if ((desc->type == QEMU_OPT_SIZE || desc->type == QEMU_OPT_NUMBER) &&
               opt) {
      printf("%s%s=%" PRId64, sep, desc->name, opt->value.uint);
    } else {
      printf("%s%s=%s", sep, desc->name, value);
    }
    sep = separator;
  }
}

QemuOpts __xtm_qemu_opts;

static QemuOpt options[] = {
    {.name = "tblink", .value.boolean = true},
    {.name = "lbt", .value.boolean = true},
    {.name = "verbose", .value.boolean = false},
    {.name = "optm", .str = "select"},
};

void init_xtm_options() {
  QemuOptsList *opts_list = &qemu_xtm_opts;
  QemuOpts *opts = &__xtm_qemu_opts;
  opts->list = opts_list;

  QTAILQ_INIT(&opts->head);

  for (int i = 0; i < sizeof(options) / sizeof(QemuOpt); ++i) {
    QTAILQ_INSERT_TAIL(&opts->head, &(options[i]), next);
  }
  x86_to_mips_parse_options(opts);
}
