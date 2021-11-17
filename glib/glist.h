#ifndef GLIST_H_FPMIJSRC
#define GLIST_H_FPMIJSRC

#include "glibconfig.h"
#include "gmem.h"
typedef struct _GList GList;

struct _GList {
  gpointer data;
  GList *next;
  GList *prev;
};

#endif /* end of include guard: GLIST_H_FPMIJSRC */
