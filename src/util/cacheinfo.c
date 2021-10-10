#include "../../include/qemu/host-utils.h"
#include <assert.h>
#include <unistd.h>

int qemu_icache_linesize = 0;
int qemu_icache_linesize_log;
int qemu_dcache_linesize = 0;
int qemu_dcache_linesize_log;

static void sys_cache_info(int *isize, int *dsize) {
  // @todo of course we will port it
  // 1. before make, run a shell get  _SC_LEVEL1_ICACHE_LINESIZE = 0x64
  // 2. compile it with -D_SC_LEVEL1_ICACHE_LINESIZE=0x64
#ifdef _SC_LEVEL1_ICACHE_LINESIZE
  *isize = sysconf(_SC_LEVEL1_ICACHE_LINESIZE);
#endif
#ifdef _SC_LEVEL1_DCACHE_LINESIZE
  *dsize = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
#endif
}

static void arch_cache_info(int *isize, int *dsize) {}

/*
 * ... and if all else fails ...
 */
static void fallback_cache_info(int *isize, int *dsize) {
  /* If we can only find one of the two, assume they're the same.  */
  if (*isize) {
    if (*dsize) {
      /* Success! */
    } else {
      *dsize = *isize;
    }
  } else if (*dsize) {
    *isize = *dsize;
  } else {
#if defined(_ARCH_PPC)
    /* For PPC, we're going to use the icache size computed for
       flush_icache_range.  Which means that we must use the
       architecture minimum.  */
    *isize = *dsize = 16;
#else
    /* Otherwise, 64 bytes is not uncommon.  */
    *isize = *dsize = 64;
#endif
  }
}

void init_cache_info(void) {
  int isize = 0, dsize = 0;

  sys_cache_info(&isize, &dsize);
  arch_cache_info(&isize, &dsize);
  fallback_cache_info(&isize, &dsize);

  assert((isize & (isize - 1)) == 0);
  assert((dsize & (dsize - 1)) == 0);

  qemu_icache_linesize = isize;
  qemu_icache_linesize_log = ctz32(isize);
  qemu_dcache_linesize = dsize;
  qemu_dcache_linesize_log = ctz32(dsize);

  // atomic64_init();
}
