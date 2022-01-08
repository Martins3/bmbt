#include "putc.h"
#include <stdio.h>

int fputc(int c, FILE *f) { return do_putc(c, f); }
