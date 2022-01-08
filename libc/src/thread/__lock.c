// #include "pthread_impl.h"
#include <assert.h>
#include <stdbool.h>

/* This lock primitive combines a flag (in the sign bit) and a
 * congestion count (= threads inside the critical section, CS) in a
 * single int that is accessed through atomic operations. The states
 * of the int for value x are:
 *
 * x == 0: unlocked and no thread inside the critical section
 *
 * x < 0: locked with a congestion of x-INT_MIN, including the thread
 * that holds the lock
 *
 * x > 0: unlocked with a congestion of x
 *
 * or in an equivalent formulation x is the congestion count or'ed
 * with INT_MIN as a lock flag.
 */

#define PHONY_LOCK_MAGIC 0x1234
void __lock(volatile int *l) {
  assert(*l == 0);
  *l = PHONY_LOCK_MAGIC;
}

void __unlock(volatile int *l) {
  // it's possible no lock before unlock
  // assert(*l == PHONY_LOCK_MAGIC);
  *l = 0;
}
