#ifndef MALLOC_GLUE_H
#define MALLOC_GLUE_H

#include "atomic.h"
#include "dynlink.h"
#include "libc.h"
#include "lock.h"
#include "syscall.h"
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
// #include <pthread.h>
// #include <elf.h>

// [interface 53]
#ifdef BMBT
// use macros to appropriately namespace these.
#define size_classes __malloc_size_classes
#define ctx __malloc_context
#define alloc_meta __malloc_alloc_meta
#define is_allzero __malloc_allzerop
#define dump_heap __dump_heap

#define malloc __libc_malloc_impl
#define realloc __libc_realloc
#define free __libc_free

#if USE_REAL_ASSERT
#include <assert.h>
#else
#undef assert
#define assert(x)                                                              \
  do {                                                                         \
    if (!(x))                                                                  \
      a_crash();                                                               \
  } while (0)
#endif
#else
#include <assert.h>
#endif

#define brk(p) ((uintptr_t)libc_syscall(SYS_brk, p))

// [interface 53]
#ifdef BMBT
#define mmap __mmap
#define madvise __madvise
#define mremap __mremap
#endif

#define DISABLE_ALIGNED_ALLOC (__malloc_replaced && !__aligned_alloc_replaced)

static inline uint64_t get_random_secret() {
  uint64_t secret = (uintptr_t)&secret * 1103515245;
#ifdef BMBT
  for (size_t i = 0; libc.auxv[i]; i += 2)
    if (libc.auxv[i] == AT_RANDOM)
      memcpy(&secret, (char *)libc.auxv[i + 1] + 8, sizeof secret);
#endif
  return secret;
}

#ifndef PAGESIZE
#define PAGESIZE PAGE_SIZE
#endif

#define MT (libc.need_locks)

#define RDLOCK_IS_EXCLUSIVE 1

hidden extern int __malloc_lock[1];

#define LOCK_OBJ_DEF                                                           \
  int __malloc_lock[1];                                                        \
  void __malloc_atfork(int who) { malloc_atfork(who); }

static inline void rdlock() {
  if (MT)
    LOCK(__malloc_lock);
}
static inline void wrlock() {
  if (MT)
    LOCK(__malloc_lock);
}
static inline void unlock() { UNLOCK(__malloc_lock); }
static inline void upgradelock() {}
static inline void resetlock() { __malloc_lock[0] = 0; }

static inline void malloc_atfork(int who) {
  if (who < 0)
    rdlock();
  else if (who > 0)
    resetlock();
  else
    unlock();
}

#endif
