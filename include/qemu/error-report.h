#ifndef ERROR_REPORT_H_UHVN2LTE
#define ERROR_REPORT_H_UHVN2LTE

// FIXME move to compiler.h
#define GCC_FMT_ATTR(n, m) __attribute__((format(printf, n, m)))

void error_report(const char *fmt, ...) GCC_FMT_ATTR(1, 2);

#endif /* end of include guard: ERROR_REPORT_H_UHVN2LTE */
