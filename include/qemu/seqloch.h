/*
 * Seqlock implementation for QEMU
 *
 * Copyright Red Hat, Inc. 2013
 *
 * Author:
 *  Paolo Bonzini <pbonzini@redhat.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 *
 */

#ifndef QEMU_SEQLOCK_H
#define QEMU_SEQLOCK_H

#include "atomic.h"
#include "thread.h"
// #include "lockable.h"

typedef struct QemuSeqLock QemuSeqLock;

struct QemuSeqLock {
  unsigned sequence;
};

static inline void seqlock_init(QemuSeqLock *sl) { sl->sequence = 0; }

/* Lock out other writers and update the count.  */
static inline void seqlock_write_begin(QemuSeqLock *sl) {
  qatomic_set(&sl->sequence, sl->sequence + 1);

  /* Write sequence before updating other fields.  */
  smp_wmb();
}

static inline void seqlock_write_end(QemuSeqLock *sl) {
  /* Write other fields before finalizing sequence.  */
  smp_wmb();

  qatomic_set(&sl->sequence, sl->sequence + 1);
}

// [interface 48]
static inline void qemu_lockable_lock(QemuSpin *lock) {
  assert(lock->lock == false);
  lock->lock = true;
}

static inline void qemu_lockable_unlock(QemuSpin *lock) {
  assert(lock->lock == true);
  lock->lock = false;
}

/* Lock out other writers and update the count.  */
static inline void seqlock_write_lock_impl(QemuSeqLock *sl, QemuSpin *lock) {
  qemu_lockable_lock(lock);
  seqlock_write_begin(sl);
}
#define seqlock_write_lock(sl, lock) seqlock_write_lock_impl(sl, lock)

/* Lock out other writers and update the count.  */
static inline void seqlock_write_unlock_impl(QemuSeqLock *sl, QemuSpin *lock) {
  qemu_lockable_unlock(lock);
  seqlock_write_begin(sl);
}
#define seqlock_write_unlock(sl, lock) seqlock_write_unlock_impl(sl, lock)

static inline unsigned seqlock_read_begin(const QemuSeqLock *sl) {
  /* Always fail if a write is in progress.  */
  unsigned ret = qatomic_read(&sl->sequence);

  /* Read sequence before reading other fields.  */
  smp_rmb();
  return ret & ~1;
}

static inline int seqlock_read_retry(const QemuSeqLock *sl, unsigned start) {
  /* Read other fields before reading final sequence.  */
  smp_rmb();
  assert(qatomic_read(&sl->sequence) == start);
  return unlikely(qatomic_read(&sl->sequence) != start);
}

#endif
