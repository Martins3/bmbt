#include "lock.h"
#include "stdio_impl.h"

static FILE *ofl_head;
static volatile int ofl_lock[1];
volatile int *const __stdio_ofl_lockptr = ofl_lock;

FILE **__ofl_lock() {
  LOCK(ofl_lock);
  return &ofl_head;
}

void __ofl_unlock() { UNLOCK(ofl_lock); }
