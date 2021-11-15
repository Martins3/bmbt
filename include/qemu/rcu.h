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

struct rcu_head;
typedef void RCUCBFunc(struct rcu_head *head);

struct rcu_head {
  struct rcu_head *next;
  RCUCBFunc *func;
};

static inline void call_rcu1(struct rcu_head *node, RCUCBFunc func) {
  func(node);
}

#define call_rcu(head, func, field)                                            \
  call_rcu1(({                                                                 \
              char __attribute__((unused))                                     \
                  offset_must_be_zero[-offsetof(typeof(*(head)), field)],      \
                  func_type_invalid = (func) - (void (*)(typeof(head)))(func); \
              &(head)->field;                                                  \
            }),                                                                \
            (RCUCBFunc *)(func))

#endif /* end of include guard: RCU_H_LHWIHDTM */
