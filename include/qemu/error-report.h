#ifndef ERROR_REPORT_H_UHVN2LTE
#define ERROR_REPORT_H_UHVN2LTE

#include <qemu/compiler.h>

void error_report(const char *fmt, ...) GCC_FMT_ATTR(1, 2);
void warn_report(const char *fmt, ...) GCC_FMT_ATTR(1, 2);

#endif /* end of include guard: ERROR_REPORT_H_UHVN2LTE */
