#include "gprintf.h"

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
  assert(string != NULL);

  int saved_errno;
  len = vasprintf(string, format, args);
  saved_errno = errno;
  if (len < 0) {
    if (saved_errno == ENOMEM)
      fprintf(stderr, "%s: failed to allocate memory", G_STRLOC);
    else
      *string = NULL;
  }
  return len;
}
