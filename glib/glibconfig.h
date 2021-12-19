#ifndef GLIBCONFIG_H_QZSYMJDV
#define GLIBCONFIG_H_QZSYMJDV

#include <limits.h>
#include <stdbool.h>

typedef unsigned long gsize;
typedef signed long gssize;
typedef void *gpointer;
typedef bool gboolean;
typedef signed char gint8;
typedef unsigned char guint8;
typedef unsigned int guint;
typedef int gint;
typedef unsigned long guint64;
typedef const void *gconstpointer;
typedef char gchar;
typedef unsigned int guint32;
typedef guint32 gunichar;

#define FALSE false
#define TRUE true
#define G_GSIZE_FORMAT "lu"
#define G_MAXSIZE G_MAXULONG
#define G_MAXULONG ULONG_MAX

#define USE_SYSTEM_PRINTF
#define HAVE_VASPRINTF 1
#endif /* end of include guard: GLIBCONFIG_H_QZSYMJDV */
