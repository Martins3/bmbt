#include "putc.h"
#include <stdio.h>

int putchar(int c) { return do_putc(c, stdout); }
