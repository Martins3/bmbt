# QEMU RCU

| var        | scope  |
| rcu_gp_ctr | global |
| rcu_reader | local  |

## [^2]

QEMU RCU core has a global counter named 'rcu_gp_ctr' which is used by both readers and updaters.
Every thread has a thread local variable of 'ctr' counter in 'rcu_reader_data' struct.


The reader will copy ‘rcu_gp_ctr’ to his own `rcu::rcu_reader_data::ctr` varaible when calling `rcu_read_lock`.
```c
extern __thread struct rcu_reader_data rcu_reader;

static inline void rcu_read_lock(void)
{
    struct rcu_reader_data *p_rcu_reader = &rcu_reader;
    unsigned ctr;

    if (p_rcu_reader->depth++ > 0) {
        return;
    }

    ctr = qatomic_read(&rcu_gp_ctr);      // 全局的 rcu_gp_ctr
    qatomic_set(&p_rcu_reader->ctr, ctr); //

    /* Write p_rcu_reader->ctr before reading RCU-protected pointers.  */
    smp_mb_placeholder();
}
```

When the `synchronize_rcu` find that the readers’ ‘ctr’ is not the same as the ‘rcu_gp_ctr’ it will set the ‘rcu_reader_data->waiting’ bool variable, and when the ‘rcu_read_unlock’ finds this bool variable is set it will trigger a event thus notify the ‘synchronize_rcu’ that it leaves the critical section. 

- rcu_init
  - rcu_init_complete
    - call_rcu_thread : 启动 rcu 回收线程
      - synchronize_rcu
        - wait_for_readers

‘rcu_gp_ongoing’ is used to check whether the there is a read in critical section. 
If it is, the new ‘rcu_gp_ctr’ will not be the same as the ‘rcu_reader_data->ctr’ and will set ‘rcu_reader_data->waiting’ to be true. If ‘registry’ is empty it means all readers has leaves the critical section and this means no old reader hold the old version pointer and the RCU thread can call the callback which insert to the RCU queue.

- [ ] 为什么 reader lock 没有指定具体是哪一个数据结构
  - [ ] 而且在 cpu_exec 的前后两个位置上设置 rcu_read_unlock 和 rcu_read_lock 和设想的很不一样, 内核中的粒度很小

```c
void address_space_destroy(AddressSpace *as)
{
    MemoryRegion *root = as->root;

    /* Flush out anything from MemoryListeners listening in on this */
    memory_region_transaction_begin();
    as->root = NULL;
    memory_region_transaction_commit();
    QTAILQ_REMOVE(&address_spaces, as, address_spaces_link);

    /* At this point, as->dispatch and as->current_map are dummy
     * entries that the guest should never use.  Wait for the old
     * values to expire before freeing the data.
     */
    as->root = root;
    call_rcu(as, do_address_space_destroy, rcu);
}
```
## [^1] 
In QEMU, when a lock is used, this will often be the "iothread mutex", also known as the "big QEMU lock" (BQL). 



[^1]: https://github.com/qemu/qemu/blob/master/docs/devel/rcu.txt
[^2]: https://terenceli.github.io/%E6%8A%80%E6%9C%AF/2021/03/14/qemu-rcu
