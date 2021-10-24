#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

/*
 * Print an error message to current monitor if we have one, else to stderr.
 * Format arguments like sprintf().  The resulting message should be
 * a single phrase, with no newline or trailing punctuation.
 * Prepend the current location and append a newline.
 * It's wrong to call this in a QMP monitor.  Use error_setg() there.
 */
void error_report(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  printf("%s", KRED);
  printf(fmt, ap);
  printf("%s", KNRM);
  va_end(ap);
  exit(1);
}

/*
 * Print a warning message to current monitor if we have one, else to stderr.
 * Format arguments like sprintf(). The resulting message should be a
 * single phrase, with no newline or trailing punctuation.
 * Prepend the current location and append a newline.
 */
void warn_report(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  printf("%s", KYEL);
  printf(fmt, ap);
  printf("%s", KNRM);
  va_end(ap);
}
