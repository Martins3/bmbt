#include <features.h>
#include <stdlib.h>

_Noreturn void exit(int code) { _Exit(code); }
