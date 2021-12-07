#ifndef GERROR_H_7R0AX4FY
#define GERROR_H_7R0AX4FY

#include "glibconfig.h"

typedef guint32 GQuark;

/**
 * GError:
 * @domain: error domain, e.g. #G_FILE_ERROR
 * @code: error code, e.g. %G_FILE_ERROR_NOENT
 * @message: human-readable informative error message
 *
 * The `GError` structure contains information about
 * an error that has occurred.
 */
typedef struct _GError GError;

struct _GError {
  GQuark domain;
  gint code;
  gchar *message;
};

#endif /* end of include guard: GERROR_H_7R0AX4FY */
