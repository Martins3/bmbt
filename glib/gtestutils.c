#include "gtestutils.h"

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
