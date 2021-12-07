#ifndef GSTRFUNCS_H_EBCALTSM
#define GSTRFUNCS_H_EBCALTSM

#include "gprintf.h"
#include "gtestutils.h"

gchar *g_strdup_printf(const gchar *format, ...)
    G_GNUC_PRINTF(1, 2) G_GNUC_MALLOC;
gchar *g_strdup_vprintf(const gchar *format, va_list args)
    G_GNUC_PRINTF(1, 0) G_GNUC_MALLOC;

#endif /* end of include guard: GSTRFUNCS_H_EBCALTSM */
