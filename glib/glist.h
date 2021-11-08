#include "glibconfig.h"
#include "gmem.h"
#ifndef USE_SYSTEM_GLIB
typedef struct _GList GList;

struct _GList
{
  gpointer data;
  GList *next;
  GList *prev;
};

#else
#include <glib.h>
#endif

