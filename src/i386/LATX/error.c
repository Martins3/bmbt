#include "error.h"
#include <execinfo.h>
#include <stdlib.h>
#include <stdio.h>

void print_stack_trace(void)
{
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
}
