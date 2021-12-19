#ifndef GSTRING_H_FRGHS8PS
#define GSTRING_H_FRGHS8PS

#include "glibconfig.h"
#include "gmacros.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _GString GString;

struct _GString {
  gchar *str;
  gsize len;
  gsize allocated_len;
};

GString *g_string_new(const gchar *init);
GString *g_string_sized_new(gsize dfl_size);
gchar *g_string_free(GString *string, gboolean free_segment);
GString *g_string_insert_len(GString *string, gssize pos, const gchar *val,
                             gssize len);
GString *g_string_append(GString *string, const gchar *val);
GString *g_string_append_len(GString *string, const gchar *val, gssize len);
GString *g_string_append_c(GString *string, gchar c);
GString *g_string_append_unichar(GString *string, gunichar wc);
GString *g_string_prepend(GString *string, const gchar *val);
GString *g_string_prepend_c(GString *string, gchar c);
GString *g_string_prepend_unichar(GString *string, gunichar wc);
GString *g_string_prepend_len(GString *string, const gchar *val, gssize len);
GString *g_string_insert(GString *string, gssize pos, const gchar *val);
GString *g_string_insert_c(GString *string, gssize pos, gchar c);
GString *g_string_insert_unichar(GString *string, gssize pos, gunichar wc);

void g_string_append_vprintf(GString *string, const gchar *format, va_list args)
    G_GNUC_PRINTF(2, 0);
void g_string_append_printf(GString *string, const gchar *format, ...)
    G_GNUC_PRINTF(2, 3);
GString *g_string_append_uri_escaped(GString *string, const gchar *unescaped,
                                     const gchar *reserved_chars_allowed,
                                     gboolean allow_utf8);
gchar *g_strdup(const gchar *str) G_GNUC_MALLOC;
gint g_vasprintf(gchar **string, gchar const *format, va_list args)
    G_GNUC_PRINTF(2, 0);

gchar *g_strdup_printf(const gchar *format, ...);
int g_strcmp0(const char *str1, const char *str2);

#endif /* end of include guard: GSTRING_H_FRGHS8PS */
