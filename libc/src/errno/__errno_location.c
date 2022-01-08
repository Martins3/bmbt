#include <errno.h>
static int bmbt_errno;

// [interface 52]
int *__errno_location(void) { return &bmbt_errno; }
