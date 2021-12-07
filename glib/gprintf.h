#ifndef GPRINTF_H_PTNKPW5G
#define GPRINTF_H_PTNKPW5G

#include "gatomic.h"
#include "gtestutils.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>

gint g_vasprintf(gchar **string, gchar const *format, va_list args)
    G_GNUC_PRINTF(2, 0);

#endif /* end of include guard: GPRINTF_H_PTNKPW5G */
