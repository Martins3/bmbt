// in order to use vasprintf
// https://stackoverflow.com/questions/67157429/warning-implicit-declaration-of-function-vasprintf
#define __STDC_WANT_LIB_EXT2__ 1
#include "gstring.h"
#include "glibconfig.h"
#include "gmem.h"
#include "gmessages.h"
#include "gslice.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/**
 * g_string_new: (constructor)
 * @init: (nullable): the initial text to copy into the string, or %NULL to
 *   start with an empty string
 *
 * Creates a new #GString, initialized with the given string.
 *
 * Returns: (transfer full): the new #GString
 */
GString *g_string_new(const gchar *init) {
  GString *string;

  if (init == NULL || *init == '\0')
    string = g_string_sized_new(2);
  else {
    gint len;

    len = strlen(init);
    string = g_string_sized_new(len + 2);

    g_string_append_len(string, init, len);
  }

  return string;
}

#define MY_MAXSIZE ((gsize)-1)
static inline gsize nearest_power(gsize base, gsize num) {
  if (num > MY_MAXSIZE / 2) {
    return MY_MAXSIZE;
  } else {
    gsize n = base;

    while (n < num)
      n <<= 1;

    return n;
  }
}

static void g_string_maybe_expand(GString *string, gsize len) {
  if (string->len + len >= string->allocated_len) {
    string->allocated_len = nearest_power(1, string->len + len + 1);
    string->str = g_realloc(string->str, string->allocated_len);
  }
}

/**
 * g_string_sized_new: (constructor)
 * @dfl_size: the default size of the space allocated to hold the string
 *
 * Creates a new #GString, with enough space for @dfl_size
 * bytes. This is useful if you are going to add a lot of
 * text to the string and don't want it to be reallocated
 * too often.
 *
 * Returns: (transfer full): the new #GString
 */
GString *g_string_sized_new(gsize dfl_size) {
  GString *string = g_slice_new(GString);

  string->allocated_len = 0;
  string->len = 0;
  string->str = NULL;

  g_string_maybe_expand(string, MAX(dfl_size, 64));
  string->str[0] = 0;

  return string;
}

/**
 * g_string_append_len:
 * @string: a #GString
 * @val: bytes to append
 * @len: number of bytes of @val to use, or -1 for all of @val
 *
 * Appends @len bytes of @val to @string.
 *
 * If @len is positive, @val may contain embedded nuls and need
 * not be nul-terminated. It is the caller's responsibility to
 * ensure that @val has at least @len addressable bytes.
 *
 * If @len is negative, @val must be nul-terminated and @len
 * is considered to request the entire string length. This
 * makes g_string_append_len() equivalent to g_string_append().
 *
 * Returns: (transfer none): @string
 */
GString *g_string_append_len(GString *string, const gchar *val, gssize len) {
  return g_string_insert_len(string, -1, val, len);
}

/**
 * g_string_insert_len:
 * @string: a #GString
 * @pos: position in @string where insertion should
 *       happen, or -1 for at the end
 * @val: bytes to insert
 * @len: number of bytes of @val to insert, or -1 for all of @val
 *
 * Inserts @len bytes of @val into @string at @pos.
 *
 * If @len is positive, @val may contain embedded nuls and need
 * not be nul-terminated. It is the caller's responsibility to
 * ensure that @val has at least @len addressable bytes.
 *
 * If @len is negative, @val must be nul-terminated and @len
 * is considered to request the entire string length.
 *
 * If @pos is -1, bytes are inserted at the end of the string.
 *
 * Returns: (transfer none): @string
 */
GString *g_string_insert_len(GString *string, gssize pos, const gchar *val,
                             gssize len) {
  gsize len_unsigned, pos_unsigned;

  g_return_val_if_fail(string != NULL, NULL);
  g_return_val_if_fail(len == 0 || val != NULL, string);

  if (len == 0)
    return string;

  if (len < 0)
    len = strlen(val);
  len_unsigned = len;

  if (pos < 0)
    pos_unsigned = string->len;
  else {
    pos_unsigned = pos;
    g_return_val_if_fail(pos_unsigned <= string->len, string);
  }

  /* Check whether val represents a substring of string.
   * This test probably violates chapter and verse of the C standards,
   * since ">=" and "<=" are only valid when val really is a substring.
   * In practice, it will work on modern archs.
   */
  if (G_UNLIKELY(val >= string->str && val <= string->str + string->len)) {
    gsize offset = val - string->str;
    gsize precount = 0;

    g_string_maybe_expand(string, len_unsigned);
    val = string->str + offset;
    /* At this point, val is valid again.  */

    /* Open up space where we are going to insert.  */
    if (pos_unsigned < string->len)
      memmove(string->str + pos_unsigned + len_unsigned,
              string->str + pos_unsigned, string->len - pos_unsigned);

    /* Move the source part before the gap, if any.  */
    if (offset < pos_unsigned) {
      precount = MIN(len_unsigned, pos_unsigned - offset);
      memcpy(string->str + pos_unsigned, val, precount);
    }

    /* Move the source part after the gap, if any.  */
    if (len_unsigned > precount)
      memcpy(string->str + pos_unsigned + precount,
             val + /* Already moved: */ precount +
                 /* Space opened up: */ len_unsigned,
             len_unsigned - precount);
  } else {
    g_string_maybe_expand(string, len_unsigned);

    /* If we aren't appending at the end, move a hunk
     * of the old string to the end, opening up space
     */
    if (pos_unsigned < string->len)
      memmove(string->str + pos_unsigned + len_unsigned,
              string->str + pos_unsigned, string->len - pos_unsigned);

    /* insert the new string */
    if (len_unsigned == 1)
      string->str[pos_unsigned] = *val;
    else
      memcpy(string->str + pos_unsigned, val, len_unsigned);
  }

  string->len += len_unsigned;

  string->str[string->len] = 0;

  return string;
}

/**
 * g_strdup:
 * @str: (nullable): the string to duplicate
 *
 * Duplicates a string. If @str is %NULL it returns %NULL.
 * The returned string should be freed with g_free()
 * when no longer needed.
 *
 * Returns: a newly-allocated copy of @str
 */
gchar *g_strdup(const gchar *str) {
  gchar *new_str;
  gsize length;

  if (str) {
    length = strlen(str) + 1;
    new_str = g_new(char, length);
    memcpy(new_str, str, length);
  } else
    new_str = NULL;

  return new_str;
}

/**
 * g_string_append_printf:
 * @string: a #GString
 * @format: the string format. See the printf() documentation
 * @...: the parameters to insert into the format string
 *
 * Appends a formatted string onto the end of a #GString.
 * This function is similar to g_string_printf() except
 * that the text is appended to the #GString.
 */
void g_string_append_printf(GString *string, const gchar *format, ...) {
  va_list args;

  va_start(args, format);
  g_string_append_vprintf(string, format, args);
  va_end(args);
}

/**
 * g_string_append_vprintf:
 * @string: a #GString
 * @format: (not nullable): the string format. See the printf() documentation
 * @args: the list of arguments to insert in the output
 *
 * Appends a formatted string onto the end of a #GString.
 * This function is similar to g_string_append_printf()
 * except that the arguments to the format string are passed
 * as a va_list.
 *
 * Since: 2.14
 */
void g_string_append_vprintf(GString *string, const gchar *format,
                             va_list args) {
  gchar *buf;
  gint len;

  g_return_if_fail(string != NULL);
  g_return_if_fail(format != NULL);

  len = g_vasprintf(&buf, format, args);

  if (len >= 0) {
    g_string_maybe_expand(string, len);
    memcpy(string->str + string->len, buf, len + 1);
    string->len += len;
    g_free(buf);
  }
}

/**
 * g_vasprintf:
 * @string: (not optional) (nullable): the return location for the
 *newly-allocated string.
 * @format: (not nullable): a standard printf() format string, but notice
 *          [string precision pitfalls][string-precision]
 * @args: the list of arguments to insert in the output.
 *
 * An implementation of the GNU vasprintf() function which supports
 * positional parameters, as specified in the Single Unix Specification.
 * This function is similar to g_vsprintf(), except that it allocates a
 * string to hold the output, instead of putting the output in a buffer
 * you allocate in advance.
 *
 * The returned value in @string is guaranteed to be non-NULL, unless
 * @format contains `%lc` or `%ls` conversions, which can fail if no
 * multibyte representation is available for the given character.
 *
 * `glib/gprintf.h` must be explicitly included in order to use this function.
 *
 * Returns: the number of bytes printed.
 *
 * Since: 2.4
 **/
gint g_vasprintf(gchar **string, gchar const *format, va_list args) {
  gint len;
  g_return_val_if_fail(string != NULL, -1);

#if !defined(USE_SYSTEM_PRINTF)

  len = _g_gnulib_vasprintf(string, format, args);
  if (len < 0)
    *string = NULL;

#elif defined(HAVE_VASPRINTF)

  {
    int saved_errno;
    len = vasprintf(string, format, args);
    saved_errno = errno;
    if (len < 0) {
      if (saved_errno == ENOMEM)
        g_error("%s: failed to allocate memory", G_STRLOC);
      else
        *string = NULL;
    }
  }

#else

  {
    va_list args2;

    G_VA_COPY(args2, args);

    *string = g_new(gchar, g_printf_string_upper_bound(format, args));

    len = _g_vsprintf(*string, format, args2);
    va_end(args2);
  }
#endif

  return len;
}

/**
 * g_strdup_vprintf:
 * @format: (not nullable): a standard printf() format string, but notice
 *     [string precision pitfalls][string-precision]
 * @args: the list of parameters to insert into the format string
 *
 * Similar to the standard C vsprintf() function but safer, since it
 * calculates the maximum space required and allocates memory to hold
 * the result. The returned string should be freed with g_free() when
 * no longer needed.
 *
 * The returned string is guaranteed to be non-NULL, unless @format
 * contains `%lc` or `%ls` conversions, which can fail if no multibyte
 * representation is available for the given character.
 *
 * See also g_vasprintf(), which offers the same functionality, but
 * additionally returns the length of the allocated string.
 *
 * Returns: a newly-allocated string holding the result
 */
gchar *g_strdup_vprintf(const gchar *format, va_list args) {
  gchar *string = NULL;

  g_vasprintf(&string, format, args);

  return string;
}

/**
 * g_strdup_printf:
 * @format: (not nullable): a standard printf() format string, but notice
 *     [string precision pitfalls][string-precision]
 * @...: the parameters to insert into the format string
 *
 * Similar to the standard C sprintf() function but safer, since it
 * calculates the maximum space required and allocates memory to hold
 * the result. The returned string should be freed with g_free() when no
 * longer needed.
 *
 * The returned string is guaranteed to be non-NULL, unless @format
 * contains `%lc` or `%ls` conversions, which can fail if no multibyte
 * representation is available for the given character.
 *
 * Returns: a newly-allocated string holding the result
 */
gchar *g_strdup_printf(const gchar *format, ...) {
  gchar *buffer;
  va_list args;

  va_start(args, format);
  buffer = g_strdup_vprintf(format, args);
  va_end(args);

  return buffer;
}

/**
 * g_strcmp0:
 * @str1: (nullable): a C string or %NULL
 * @str2: (nullable): another C string or %NULL
 *
 * Compares @str1 and @str2 like strcmp(). Handles %NULL
 * gracefully by sorting it before non-%NULL strings.
 * Comparing two %NULL pointers returns 0.
 *
 * Returns: an integer less than, equal to, or greater than zero, if @str1 is <,
 * == or > than @str2.
 *
 * Since: 2.16
 */
int g_strcmp0(const char *str1, const char *str2) {
  if (!str1)
    return -(str1 != str2);
  if (!str2)
    return str1 != str2;
  return strcmp(str1, str2);
}

/**
 * g_string_free:
 * @string: (transfer full): a #GString
 * @free_segment: if %TRUE, the actual character data is freed as well
 *
 * Frees the memory allocated for the #GString.
 * If @free_segment is %TRUE it also frees the character data.  If
 * it's %FALSE, the caller gains ownership of the buffer and must
 * free it after use with g_free().
 *
 * Returns: (nullable): the character data of @string
 *          (i.e. %NULL if @free_segment is %TRUE)
 */
gchar *g_string_free(GString *string, gboolean free_segment) {
  gchar *segment;

  g_return_val_if_fail(string != NULL, NULL);

  if (free_segment) {
    g_free(string->str);
    segment = NULL;
  } else
    segment = string->str;

  g_slice_free(GString, string);

  return segment;
}

GString *g_string_append_c(GString *string, gchar c) {
  g_return_val_if_fail(string != NULL, NULL);

  return g_string_insert_c(string, -1, c);
}

/**
 * g_string_append_unichar:
 * @string: a #GString
 * @wc: a Unicode character
 *
 * Converts a Unicode character into UTF-8, and appends it
 * to the string.
 *
 * Returns: (transfer none): @string
 */
GString *g_string_append_unichar(GString *string, gunichar wc) {
  g_return_val_if_fail(string != NULL, NULL);

  return g_string_insert_unichar(string, -1, wc);
}

/**
 * g_string_insert_c:
 * @string: a #GString
 * @pos: the position to insert the byte
 * @c: the byte to insert
 *
 * Inserts a byte into a #GString, expanding it if necessary.
 *
 * Returns: (transfer none): @string
 */
GString *g_string_insert_c(GString *string, gssize pos, gchar c) {
  gsize pos_unsigned;

  g_return_val_if_fail(string != NULL, NULL);

  g_string_maybe_expand(string, 1);

  if (pos < 0)
    pos = string->len;
  else
    g_return_val_if_fail((gsize)pos <= string->len, string);
  pos_unsigned = pos;

  /* If not just an append, move the old stuff */
  if (pos_unsigned < string->len)
    memmove(string->str + pos_unsigned + 1, string->str + pos_unsigned,
            string->len - pos_unsigned);

  string->str[pos_unsigned] = c;

  string->len += 1;

  string->str[string->len] = 0;

  return string;
}

/**
 * g_string_insert_unichar:
 * @string: a #GString
 * @pos: the position at which to insert character, or -1
 *     to append at the end of the string
 * @wc: a Unicode character
 *
 * Converts a Unicode character into UTF-8, and insert it
 * into the string at the given position.
 *
 * Returns: (transfer none): @string
 */
GString *g_string_insert_unichar(GString *string, gssize pos, gunichar wc) {
  gint charlen, first, i;
  gchar *dest;

  g_return_val_if_fail(string != NULL, NULL);

  /* Code copied from g_unichar_to_utf() */
  if (wc < 0x80) {
    first = 0;
    charlen = 1;
  } else if (wc < 0x800) {
    first = 0xc0;
    charlen = 2;
  } else if (wc < 0x10000) {
    first = 0xe0;
    charlen = 3;
  } else if (wc < 0x200000) {
    first = 0xf0;
    charlen = 4;
  } else if (wc < 0x4000000) {
    first = 0xf8;
    charlen = 5;
  } else {
    first = 0xfc;
    charlen = 6;
  }
  /* End of copied code */

  g_string_maybe_expand(string, charlen);

  if (pos < 0)
    pos = string->len;
  else
    g_return_val_if_fail((gsize)pos <= string->len, string);

  /* If not just an append, move the old stuff */
  if ((gsize)pos < string->len)
    memmove(string->str + pos + charlen, string->str + pos, string->len - pos);

  dest = string->str + pos;
  /* Code copied from g_unichar_to_utf() */
  for (i = charlen - 1; i > 0; --i) {
    dest[i] = (wc & 0x3f) | 0x80;
    wc >>= 6;
  }
  dest[0] = wc | first;
  /* End of copied code */

  string->len += charlen;

  string->str[string->len] = 0;

  return string;
}

/**
 * g_string_append:
 * @string: a #GString
 * @val: the string to append onto the end of @string
 *
 * Adds a string onto the end of a #GString, expanding
 * it if necessary.
 *
 * Returns: (transfer none): @string
 */
GString *g_string_append(GString *string, const gchar *val) {
  return g_string_insert_len(string, -1, val, -1);
}
