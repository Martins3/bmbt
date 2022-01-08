#include "stdio_impl.h"
#include <assert.h>
#include <stdbool.h>

// [BMBT_MTTCG 0]
int __lockfile(FILE *f) {
  assert(false);
  return 0;
}
void __unlockfile(FILE *f) { assert(false); }
