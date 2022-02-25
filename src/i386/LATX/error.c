#include "include/error.h"
#ifdef USE_SYSTEM_LIBC
#include <execinfo.h>
#endif
#include <stdlib.h>
#include <stdio.h>

// TMP_TODO doc
void print_stack_trace(void)
{
#ifdef USE_SYSTEM_LIBC
    int size = 64;
    void *array[64];

    int s_nr = backtrace(array, size);
    char **s = backtrace_symbols(array, s_nr);

    int i = 0;

    printf("\n ------ stack trace ------\n");

    for (i = 0; i < s_nr; ++i) {
        printf("%s\n", s[i]);
    }

    free(s);
#endif
}
