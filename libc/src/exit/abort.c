#include <errno.h>
#include <features.h>
#include <stdlib.h>

_Noreturn void abort(void) { _Exit(1); }
