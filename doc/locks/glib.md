# 官方参考文档

- [ ] 写一个同时监听文件和 stdio 的 g_main_loop 出来
- context 和 loop 是什么关系?

```c
GMainLoop *main_loop = g_main_loop_new(NULL, FALSE);
/* interval, function callback, userdata */
g_timeout_add_seconds(5, callback_function, "callback_function");
g_main_loop_run(main_loop);
```

```c
/** context for a loop */
typedef struct _LOOP_STR {
  char *name;
  int tmo;
  GMainContext *main_context;
  GMainLoop *main_loop;
  GSource *idle;
  GAsyncQueue *Q;
  GThread *TH;
} LOOP_STR;
```

g_source_attach

- GMainLoop : 住循环
- GMainContext : The GMainContext struct is an opaque data type representing a set of sources to be handled in a main loop.
- GSource : 对应一个监听的事件源
- GThread : 调用 g_main_loop_run

## https://docs.gtk.org/glib/main-loop.html
Note that event sources are associated with a particular `GMainContext`,
and will be checked and dispatched for all main loops associated with that GMainContext.

- event source 需要和一个 context 绑定
- 而一个 GMainContext 可以和多个 context 绑定


## gio
https://github.com/chiehmin/gdbus_test

我不知道 gmain 和 gdbus 这两个线程都是是做什么的!


## ref
- https://github.com/wadester/wh_test_glib
- https://stackoverflow.com/questions/26410585/add-callback-for-separate-g-main-loop
    - Q: 如何才可以将 GSource 关联到特定的 GMainLoop 上
    - A:
      - 创建 GMainLoop 的时候，如果参数为 NULL, 那么关联是默认 GMainContext 的。而创建一个 GSource 的时候，也是默认关联默认 GMainContext。如果想要让一个 GSource 关联一个特殊的 GMainContext 就要 explicit 的指出。
    - 注意，这里还给出了一个例子，那就是 GMainLoop 都是在同一个 thread 上运行的，当然也可以让其在不同的 thread 上运行。
- https://stackoverflow.com/questions/23737750/glib-usage-without-mainloop
    - g_main_loop_run 中间实际上调用的是 poll 之类的，但是可以 while(1) + g_main_context_iteration 来实现更加精细的调度了

if you want to run the loop in a different thread. You can create a GMainContext with `g_main_context_new()` and pass it to `g_main_loop_new()` [^1]

## g_main_context_iteration
ref: https://developer.gimp.org/api/2.0/glib/glib-The-Main-Event-Loop.html#g-main-context-iteration

Runs a single iteration for the given main loop. This involves checking to see if any event sources are ready to be processed, then if no events sources are ready and `may_block` is TRUE, waiting for a source to become ready, then dispatching the highest priority events sources that are ready. Otherwise, if `may_block` is FALSE sources are not waited to become ready, only those highest priority events sources will be dispatched (if any), that are ready at this given moment without further waiting.

Note that even when `may_block` is TRUE, it is still possible for `g_main_context_iteration()` to return `FALSE`, since the the wait may be interrupted for other reasons than an event source becoming ready.

> 相当于是说，调用一次

## [ ] https://blog.csdn.net/u010009623/article/details/53101492


## tests/unit/test-aio.c
> 忽然意识到，到处随便查找，还不如去使用直接使用 QEMU 内的测试

这个文件中的 `static AioContext *ctx;` 是在 qemu_init_main_loop 中调用 aio_context_new 的。

#### test_acquire
- event_notifier_init : 默认使用 eventfd，如果没有，使用 pipe
- set_event_notifier : 可以注册对应的 hook 函数
- event_notifier_set : 通知 eventfd 之类的操作

#### test_bh_schedule
```c
#0  bh_test_cb (opaque=0x7fffffffd4b0) at ../tests/unit/test-aio.c:57
#1  0x0000555555687328 in aio_bh_poll (ctx=ctx@entry=0x5555557dc6f0) at ../util/async.c:169
#2  0x0000555555696256 in aio_poll (ctx=<optimized out>, blocking=<optimized out>) at ../util/aio-posix.c:659
#3  0x00005555555b529a in test_bh_schedule () at ../tests/unit/test-aio.c:181
```
- aio_bh_new : 创建一个 bh 的结构体挂到 AioContext::bh_slice_list::bh_list 上
- aio_poll
  - aio_bh_poll
    - aio_bh_dequeue : 从 list 上去除想要内容，然后执行

为了容易 async 的控制，所以需要使用这些 flag 来操作:
```c
/* QEMUBH::flags values */
enum {
    /* Already enqueued and waiting for aio_bh_poll() */
    BH_PENDING   = (1 << 0),

    /* Invoke the callback */
    BH_SCHEDULED = (1 << 1),

    /* Delete without invoking callback */
    BH_DELETED   = (1 << 2),

    /* Delete after invoking callback */
    BH_ONESHOT   = (1 << 3),

    /* Schedule periodically when the event loop is idle */
    BH_IDLE      = (1 << 4),
};
```

#### test_queue_chaining
- qemu_coroutine_create
- qemu_coroutine_enter : 进入执行 coroutine
- aio_co_enter : 将 coroutine 挂到 list 上
    - 通过 aio_poll 可以让挂载到 list 上的行为被执行
- qemu_coroutine_yield : coroutine 放弃执行

- aio_co_enter 写的似乎很有道理:
  - 如果 aio_co_enter 加入的 AioContext 不是当前线程的，那么加入到该 AioContext
  - 如果是递归的 aio_co_enter，那么挂载到 list 上
  - 否则可以直接执行了

## tests/unit/test-aio-multithread.c

#### test_lifecycle
- create_aio_contexts
    - iothread_new
      - 使用 qemu_thread_create 创建一个 thread，在该线程中间执行 iothread_run
      - iothread_run 中间创建出来两个 context 来:
          - aio_context_new : 创建 aio 出来
          - iothread_init_gcontext : 创建 GMainContext
    - aio_bh_schedule_oneshot : 其实就是前面的 aio bh 添加到 queue 上的操作
- join_aio_contexts

#### test_multi_co_schedule
- qemu_coroutine_create : 不考虑 coroutine pool 的话，这个就是初始化一下结构体中间的字段而已
- aio_co_schedule : 调用一下 qemu_bh_schedule

## tests/unit/test-thread-pool.c

#### test_submit
- thread_pool_submit 来进行提交, 提交完成之后，使用 aio_poll 来等待 worker thread 搞完
    - [ ] aio_poll 具体是等待在什么位置上的

将 worker_cb 添加添加一个死循环:
```c
/*
#0  0x00007ffff730bbf6 in __ppoll (fds=0x5555557df6d0, nfds=1, timeout=<optimized out>, timeout@entry=0x0, sigmask=sigmask@entry=0x0) at ../sysdeps/unix/sysv/linux/ppol
l.c:44
#1  0x00005555556a9a39 in ppoll (__ss=0x0, __timeout=0x0, __nfds=<optimized out>, __fds=<optimized out>) at /usr/include/x86_64-linux-gnu/bits/poll2.h:77
#2  qemu_poll_ns (fds=<optimized out>, nfds=<optimized out>, timeout=timeout@entry=-1) at ../util/qemu-timer.c:336
#3  0x0000555555697fa5 in fdmon_poll_wait (ctx=0x5555557dc6f0, ready_list=0x7fffffffd538, timeout=-1) at ../util/fdmon-poll.c:80
#4  0x00005555556971e3 in aio_poll (ctx=<optimized out>, blocking=<optimized out>) at ../util/aio-posix.c:612
#5  0x00005555555b5e59 in test_submit () at ../tests/unit/test-thread-pool.c:54
#6  0x00007ffff7dcf58e in ?? () from /lib/x86_64-linux-gnu/libglib-2.0.so.0
#7  0x00007ffff7dcf334 in ?? () from /lib/x86_64-linux-gnu/libglib-2.0.so.0
#8  0x00007ffff7dcfa7a in g_test_run_suite () from /lib/x86_64-linux-gnu/libglib-2.0.so.0
#9  0x00007ffff7dcfa95 in g_test_run () from /lib/x86_64-linux-gnu/libglib-2.0.so.0
#10 0x00005555555b450f in main (argc=<optimized out>, argv=<optimized out>) at ../tests/unit/test-thread-pool.c:250
```
可以知道:
- 使用 FDMonOps 来 poll 的
- 看看分别各家的实现:
    - fdmon_epoll_wait : 使用 AioContext::epollfd
    - fdmon_poll_wait : 使用全局变量 pollfds, 这个东西是在 fdmon_poll_wait 从 AioContext::aio_handlers 初始化得到的
    - fdmon_io_uring_wait : 使用 AioContext::fdmon_io_uring

首先不考虑 io_uring 的事情，epollfd 是如何添加进去的。

- aio_set_fd_handler 会调用 FDMonOps::update 来需要监听的 fd 更新到 epollfd 中。
- 而 poll 不需要做任何事情，正如其注释所说，都是放到

```c
static void fdmon_poll_update(AioContext *ctx,
                              AioHandler *old_node,
                              AioHandler *new_node)
{
    /* Do nothing, AioHandler already contains the state we'll need */
}
```

#### test_submit_aio
- thread_pool_submit_aio 在 thread_pool_submit 的基础上多出来了配置 BlockCompletionFunc 的操作
    - BlockCompletionFunc 这个 hook 的调用位置就是在靠 thread_pool_completion_bh 执行的

```c
/*
#0  thread_pool_completion_bh (opaque=0x5555557cd3e0) at ../util/thread-pool.c:163
#1  0x0000555555687e28 in aio_bh_poll (ctx=ctx@entry=0x5555557dc6f0) at ../util/async.c:169
#2  0x0000555555696da0 in aio_poll (ctx=<optimized out>, blocking=<optimized out>) at ../util/aio-posix.c:665
#3  0x00005555555b5e59 in test_submit () at ../tests/unit/test-thread-pool.c:53
#4  0x00007ffff7dcf58e in ?? () from /lib/x86_64-linux-gnu/libglib-2.0.so.0
#5  0x00007ffff7dcf334 in ?? () from /lib/x86_64-linux-gnu/libglib-2.0.so.0
#6  0x00007ffff7dcfa7a in g_test_run_suite () from /lib/x86_64-linux-gnu/libglib-2.0.so.0
#7  0x00007ffff7dcfa95 in g_test_run () from /lib/x86_64-linux-gnu/libglib-2.0.so.0
#8  0x00005555555b450f in main (argc=<optimized out>, argv=<optimized out>) at ../tests/unit/test-thread-pool.c:249
```

#### test_submit_co
- thread_pool_submit_co : 调用 thread_pool_submit_aio 之外，还是需要调用 qemu_coroutine_yield 的
- qemu_coroutine_enter 可以开始执行一个 coroutine, 如果 coroutine 执行了 qemu_coroutine_yield 之后，那么 qemu_coroutine_enter 就可以返回了，使用 aio_poll 可以让 coroutine 继续执行

- [ ] 将 coroutine 提交给另一个 thread 的意义是什么?

coroutine 是可以在另一个 thread 上执行的，但是不能在另一个 ctx 上执行

#### test_cancel
- [ ] 以后再看了

## tests/unit/test-block-backend

#### test_drain_aio_error

[14/24] Linking target tests/unit/test-coroutine
[15/24] Linking target tests/unit/test-blockjob-txn
[16/24] Linking target tests/unit/test-blockjob
[17/24] Linking target tests/unit/test-replication
[18/24] Linking target tests/unit/test-write-threshold
[19/24] Linking target
[20/24] Linking target tests/unit/test-io-task
[21/24] Linking target tests/unit/test-bdrv-graph-mod
[22/24] Linking target tests/unit/test-block-iothread
[23/24] Linking target tests/unit/test-image-locking

## tests/unit/iothread
- [ ] 在使用 iothread 之前, 这个函数的运行方式 ?

[^1]: https://stackoverflow.com/questions/42395844/glib-gmaincontext-in-a-thread
