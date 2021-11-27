#ifndef QEMU_CHAR_H
#define QEMU_CHAR_H
#include <stdio.h>

typedef struct CharBackend CharBackend;

typedef struct Chardev {
#ifdef BMBT
  Object parent_obj;

  QemuMutex chr_write_lock;
  char *label;
  char *filename;
  int logfd;
  int be_open;
  GSource *gsource;
  GMainContext *gcontext;
  DECLARE_BITMAP(features, QEMU_CHAR_FEATURE_LAST);
#endif
  CharBackend *be;
  FILE *log;
} Chardev;
#endif
