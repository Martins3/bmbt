# QEMU RCU

总结 : 之前一直意味 RCU 只能处理 list 和 hlist，其实还是用于普通对象的声明周期管理
[What is RCU, Fundamentally?](https://lwn.net/Articles/262464/) 中的
Example 1: Maintaining Multiple Versions During Deletion 和 Example 2: Maintaining Multiple Versions During Replacement
用于理解 RCU 的原理算是相当的生动形象了。

| var                  | scope  |                         |
|----------------------|--------|-------------------------|
| rcu_gp_ctr           | global | 一个简单的 unsigned long                         |
| rcu_reader           | local  |                         |
| rcu_call_count       | global |                         |
| rcu_call_ready_event | global | 在 call_rcu1 中间被使用 |
| tail                 |        | try_dequeue enqueue     |

## [^2]
QEMU RCU core has a global counter named 'rcu_gp_ctr' which is used by both readers and updaters.
Every thread has a thread local variable of 'ctr' counter in 'rcu_reader_data' struct.

When the `synchronize_rcu` find that the readers' `ctr` is not the same as the ‘rcu_gp_ctr’
it will set the `rcu_reader_data->waiting` bool variable, and when the `rcu_read_unlock` finds this bool variable
is set it will trigger a event thus notify the `synchronize_rcu` that it leaves the critical section.

> 做法应该是: rcu_read_lock 从 rcu_gp_ctr 从拷贝版本号，当离开的时候，如果发现此时的版本号 和当时拷贝的不同，那么意味着自己之前在使用老的资源，那么需要开始告知


```c
static inline void rcu_read_unlock(void)
{
    // 省略一些代码
    if (unlikely(qatomic_read(&p_rcu_reader->waiting))) {
        qatomic_set(&p_rcu_reader->waiting, false);
        qemu_event_set(&rcu_gp_event);
    }
}
```

- rcu_init
  - rcu_init_complete
    - call_rcu_thread : 启动 rcu 回收线程
      - [ ] qemu_event_wait : 被多次调用
      - synchronize_rcu
        - wait_for_readers : 流程很清晰
          1. `static ThreadList registry = QLIST_HEAD_INITIALIZER(registry);` : 在 rcu_register_thread 的时候，将 thread local 的 rcu_reader 挂到上面去
          2. 对于register 上挂载的 rcu_reader 调用 rcu_gp_ongoing 查询 local 的版本和 global 的版本是否存在差别，如果有，那么设置 rcu_reader_data::waiting 为 true, 如果版本相同，那么从 registry 中移除掉
          3. QLIST_EMPTY(&registry) : 这表示所有的 reader 都离开 critical region 了
          4. qemu_event_wait(&rcu_gp_event); 等待
      - try_dequeue && `node->func(node)` : 从队列中间取出需要执行的函数来

`rcu_gp_ongoing` is used to check whether the there is a read in critical section. 
If it is, the new `rcu_gp_ctr` will not be the same as the `rcu_reader_data->ctr` and will set `rcu_reader_data->waiting` to be true.
If `registry` is empty it means all readers has leaves the critical section and this means no old reader hold the old version pointer
and the RCU thread can call the callback which insert to the RCU queue.

#### 分析一手 call_rcu
qemu_event_set : 让 qemu_event_wait 在这个上的程序可以启动了

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


```c
/* The operands of the minus operator must have the same type,
 * which must be the one that we specify in the cast.
 */
#define call_rcu(head, func, field)                                      \
    call_rcu1(({                                                         \
         char __attribute__((unused))                                    \
            offset_must_be_zero[-offsetof(typeof(*(head)), field)],      \
            func_type_invalid = (func) - (void (*)(typeof(head)))(func); \
         &(head)->field;                                                 \
      }),                                                                \
      (RCUCBFunc *)(func))

#define g_free_rcu(obj, field) \
    call_rcu1(({                                                         \
        char __attribute__((unused))                                     \
            offset_must_be_zero[-offsetof(typeof(*(obj)), field)];       \
        &(obj)->field;                                                   \
      }),                                                                \
      (RCUCBFunc *)g_free);
```
这两个 macro 都是两个简单的封装函数，通过数组下标不能为负数保证 
`struct rcu_head rcu;` 在结构体的开始位置。

在 call_rcu1 中间，
```c
void call_rcu1(struct rcu_head *node, void (*func)(struct rcu_head *node))
{
    node->func = func;
    enqueue(node);
    qatomic_inc(&rcu_call_count); // 让 call_rcu_thread 调用 try_dequeue 的时候知道循环多少次
    qemu_event_set(&rcu_call_ready_event);
}
```

#### qatomic_rcu_read 和 qatomic_rcu_set
qatomic_rcu_read and qatomic_rcu_set replace `rcu_dereference` and
`rcu_assign_pointer`.  They take a _pointer_ to the variable being accessed.[^1]

`rcu_dereference()` should be used at read-side, protected by `rcu_read_lock()` or similar.

```c
address_space_set_flatview
    /* Writes are protected by the BQL.  */
    qatomic_rcu_set(&as->current_map, new_view);

void flatview_unref(FlatView *view)
    call_rcu(view, flatview_destroy, rcu);
```

- address_space_set_flatview 进行 qatomic_rcu_set 的时候被 BQL 保护，一般进行 qatomic_rcu_set 的时候会被更加细粒度的锁保护，例如在 qemu_set_log 中 QEMU_LOCK_GUARD(&qemu_logfile_mutex);

rcu_gp_ctr 只是在 synchronize_rcu 中间见到更新，从 [^2] 的描述中，应该是 call_rcu 的时候，`qatomic_inc(&rcu_call_count);` 让
call_rcu_thread 从一个 while 循环中间退出，开始执行 synchronize_rcu，call_rcu_thread 的这个 while 循环执行的比较复杂，结合注释，应该是为了多等待几个 writer

应该是这样的，qatomic_rcu_set 和 qatomic_rcu_read 其中很重要的一个事情是封装 membarrier 的工作，而 RCU 机制的作用在于，
reader 获取了指针 p 之后，之后通过 p 进行各种操作可以保证 p 指向的空间没有被释放。如果重新 qatomic_rcu_read, 那么可能获取到了新的值。
```c
    rcu_read_lock();
    p = qatomic_rcu_read(&foo);
    /* do something with p. */
    rcu_read_unlock();
```

## [^1] 
In QEMU, when a lock is used, this will often be the "iothread mutex", also known as the "big QEMU lock" (BQL). 

## [ ] 分析一下在当前项目中使用到的 RCU
```
➜  src git:(xqm) ✗ ag rcu
qemu/memory_ldst.c.inc
35:    RCU_READ_LOCK();
65:    RCU_READ_UNLOCK();
103:    RCU_READ_LOCK();
133:    RCU_READ_UNLOCK();
169:    RCU_READ_LOCK();
188:    RCU_READ_UNLOCK();
205:    RCU_READ_LOCK();
235:    RCU_READ_UNLOCK();
274:    RCU_READ_LOCK();
296:    RCU_READ_UNLOCK();
311:    RCU_READ_LOCK();
340:    RCU_READ_UNLOCK();
374:    RCU_READ_LOCK();
392:    RCU_READ_UNLOCK();
407:    RCU_READ_LOCK();
436:    RCU_READ_UNLOCK();
471:    RCU_READ_LOCK();
500:    RCU_READ_UNLOCK();
528:#undef RCU_READ_LOCK
529:#undef RCU_READ_UNLOCK

tcg/cputlb.c
764: * Called from TCG-generated code, which is under an RCU read-side

tcg/cpu-exec.c
8:#include "../../include/qemu/rcu.h"
487:  rcu_read_lock();
545:  rcu_read_unlock();

tcg/translate-all.c
525:        void **p = atomic_rcu_read(lp);
544:    pd = atomic_rcu_read(lp);
```

## [x] QTAILQ_INSERT_TAIL 和 QTAILQ_INSERT_TAIL_RCU 版本差异是什么?
回答，几乎没有任何的区别啊

对比这两个，只是在写的时候是 atomic 的
```c
#define QTAILQ_INSERT_TAIL(head, elm, field) do {                       \
        (elm)->field.tqe_next = NULL;                                   \
        (elm)->field.tqe_circ.tql_prev = (head)->tqh_circ.tql_prev;     \
        (head)->tqh_circ.tql_prev->tql_next = (elm);                    \
        (head)->tqh_circ.tql_prev = &(elm)->field.tqe_circ;             \
} while (/*CONSTCOND*/0)

#define QTAILQ_INSERT_TAIL_RCU(head, elm, field) do {                   \
    (elm)->field.tqe_next = NULL;                                       \
    (elm)->field.tqe_circ.tql_prev = (head)->tqh_circ.tql_prev;         \
    qatomic_rcu_set(&(head)->tqh_circ.tql_prev->tql_next, (elm));       \
    (head)->tqh_circ.tql_prev = &(elm)->field.tqe_circ;                 \
} while (/*CONSTCOND*/0)
```

- [ ] 算了，分析一屁，以后再说了

顺便分析一下，QTAILQ 的实现方式
```c
typedef struct QTailQLink {
    void *tql_next;
    struct QTailQLink *tql_prev;
} QTailQLink;

#define QTAILQ_ENTRY(type)                                              \
union {                                                                 \
        struct type *tqe_next;        /* next element */                \
        QTailQLink tqe_circ;          /* link for circular backwards list */ \
}

struct CPUState {
    // ...
    QTAILQ_ENTRY(CPUState) node;

    // ...
```


[^1]: https://github.com/qemu/qemu/blob/master/docs/devel/rcu.txt
[^2]: https://terenceli.github.io/%E6%8A%80%E6%9C%AF/2021/03/14/qemu-rcu
[^3]: https://stackoverflow.com/questions/39251287/rcu-dereference-vs-rcu-dereference-protected
