#ifndef GSTRING_H_FRGHS8PS
#define GSTRING_H_FRGHS8PS

#include "glibconfig.h"
#include "gmacros.h"
#include <stdarg.h>
#include <stdlib.h>

typedef struct _GString GString;

struct _GString {
  gchar *str;
  gsize len;
  gsize allocated_len;
};

GString *g_string_new(

    const gchar *init);
GString *g_string_new_len(const gchar *init, gssize len);
GString *g_string_sized_new(gsize dfl_size);
gchar *g_string_free(GString *string, gboolean free_segment);
gboolean g_string_equal(const GString *v, const GString *v2);
guint g_string_hash(const GString *str);
GString *g_string_assign(GString *string, const gchar *rval);
GString *g_string_truncate(GString *string, gsize len);
GString *g_string_set_size(GString *string, gsize len);
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
GString *g_string_overwrite(GString *string, gsize pos, const gchar *val);
GString *g_string_overwrite_len(GString *string, gsize pos, const gchar *val,
                                gssize len);
GString *g_string_erase(GString *string, gssize pos, gssize len);
GString *g_string_ascii_down(GString *string);
GString *g_string_ascii_up(GString *string);
/* void         g_string_vprintf           (GString         *string,
                                         const gchar     *format,
                                         va_list          args)
                                         G_GNUC_PRINTF(2, 0);
void         g_string_printf            (GString         *string,
                                         const gchar     *format,
                                         ...) G_GNUC_PRINTF (2, 3); */
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

// this function is glibc function, it's a mistake
// extern int vasprintf(char **__restrict __ptr, const char *__restrict __f,
//                      __gnuc_va_list __arg) __THROWNL
//     __attribute__((__format__(__printf__, 2, 0))) __wur;

#endif /* end of include guard: GSTRING_H_FRGHS8PS */
