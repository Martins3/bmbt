#ifndef RCU_H_LHWIHDTM
#define RCU_H_LHWIHDTM

// @todo how does rcu works?
void rcu_read_unlock(void);
void rcu_read_lock(void);

/*
 * Reader thread registration.
 */
void rcu_register_thread(void);
void rcu_unregister_thread(void);

#define WITH_RCU_READ_LOCK_GUARD() {}
#define RCU_READ_LOCK_GUARD() {}

#endif /* end of include guard: RCU_H_LHWIHDTM */
