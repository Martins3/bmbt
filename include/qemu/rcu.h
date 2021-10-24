#ifndef RCU_H_LHWIHDTM
#define RCU_H_LHWIHDTM

static inline void rcu_read_unlock(void) {}
static inline void rcu_read_lock(void) {}

/*
 * Reader thread registration.
 */
static inline void rcu_register_thread(void) {}
static inline void rcu_unregister_thread(void) {}

#define WITH_RCU_READ_LOCK_GUARD()                                             \
  {}
#define RCU_READ_LOCK_GUARD()                                                  \
  {}

// [interface 18]
#define QTAILQ_INSERT_TAIL_RCU QTAILQ_INSERT_TAIL
#define QTAILQ_REMOVE_RCU QTAILQ_REMOVE

// @todo
struct rcu_head {
  struct rcu_head *next;
  // RCUCBFunc *func;
};

#define atomic_rcu_set(ptr, i)                                                 \
  do {                                                                         \
    *ptr = i;                                                                  \
  } while (0)

// @todo
#define call_rcu(x, y, z)                                                      \
  {}

#endif /* end of include guard: RCU_H_LHWIHDTM */
