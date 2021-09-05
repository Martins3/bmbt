# MTTG && Locks

## 问题
- [ ] iothread_run 中会调用 g_main_loop_run 和 aio_poll
- [ ] 无法理解 g_main_loop_run 只是别 iothread 调用
- [ ] io_thread 似乎是默认没有启用的，如何打开并且测试一下
  - [ ] 如果使用上 iothread 是不是就不会 main_loop 就不会有了
  - [ ] main_loop 的三个 context 都是做啥的

## Stefan Hajnoczi
http://blog.vmsplice.net/2020/08/qemu-internals-event-loops.html

The most important event sources in QEMU are:
- File descriptors such as sockets and character devices.
- Event notifiers (implemented as eventfds on Linux).
- Timers for delayed function execution.
- *Bottom-halves (BHs) for invoking a function in another thread or deferring a function call to avoid reentrancy.*

- 前面说两个情况是 IO 多路复用的考虑
- [ ] 什么叫做在另一个线程中 invoke a function
  - 将工作放到另一个线程中进行，所以, BH 实际上是将工作挂载到一个队列中的。
- [ ] deferring a function call to avoid reentrancy

QEMU has several different types of threads:
- vCPU threads that execute guest code and perform device emulation synchronously with respect to the vCPU.
- The main loop that runs the event loops (yes, there is more than one!) used by many QEMU components.
- IOThreads that run event loops for device emulation concurrently with vCPUs and "out-of-band" QMP monitor commands.

- [ ] 无法理解的东西: main loop 的 event 既然监控了很多东西，为什么还需要特地创建出来 IOThreads

忽然想到，ioeventfd 这个东西就是给 aio 使用的啊!

> Some devices perform the guest device register access in the main loop thread or an IOThread thanks to ioeventfd.

- [ ] 所以，eventfd 会导致其使用 ioeventfd 吗?

The key point is that vCPU threads do not run an event loop. The main loop thread and IOThreads run event loops. vCPU threads can add event sources to the main loop or IOThread event loops. Callbacks run in the main loop thread or IOThreads.

解释的太好了
The main loop and IOThreads share some code but are fundamentally different.
The common code is called AioContext and is QEMU's native event loop API.
Commonly-used functions include aio_set_fd_handler(), aio_set_event_handler(), aio_timer_init(), and aio_bh_new().

- [ ] 重点关注一下这几个函数，检查从 main event loop 和 io handler 到 AioContext 流程

The main loop actually has a glib GMainContext and two AioContext event loops.
QEMU components can use any of these event loop APIs and the main loop combines them all into a single event loop function `os_host_main_loop_wait()` that calls `qemu_poll_ns()` to wait for event sources.
This makes it possible to combine glib-based code with code using the native QEMU AioContext APIs.

- [ ] 检测一下 main loop 如何同时检测 glib 和 两个 AioContext 的?

The reason why the main loop has two AioContexts is because one, called `iohandler_ctx`,
is used to implement older `qemu_set_fd_handler()` APIs whose handlers should not run when the other AioContext, called `qemu_aio_context`, is run using `aio_poll()`.
The QEMU block layer and newer code uses `qemu_aio_context` while older code uses `iohandler_ctx`. Over time it may be possible to unify the two by converting `iohandler_ctx` handlers to safely execute in `qemu_aio_context`.

- iohandler_ctx : 使用 qemu_set_fd_handler 运行
- qemu_aio_context : 使用 aio_poll 运行
- iohandler_ctx 的 handler 在 qemu_aio_context 运行的时候不可以运行
- iohandler_ctx 之后不会替换掉的。

- [ ] 为什么 main loop 为什么需要持有这么多的 context
    - 但是，这是真的，就是放到 main-loop.c 中间的
    - 具体内容看 qemu_init_main_loop 的初始化
    - [ ] 跟踪一下 qemu_set_fd_handler 这个东西，既然他是原因

IOThreads have an AioContext and a glib GMainContext.
The AioContext is run using the `aio_poll()` API, which enables the advanced features of the event loop.
If a glib event loop is needed then the `GMainContext` can be run using `g_main_loop_run()` and the `AioContext` event sources will be included.

IOThread 也是拥有 AioContext 和 glib GMainContext 的

The key difference between the main loop and IOThreads is that the main loop uses a traditional event loop that calls qemu_poll_ns() while IOThreads AioContext aio_poll() has advanced features that result in better performance.

AioContext has the following event loop features that traditional event loops do not have:

- AioContext 比 GMainContext 是很多好处的
  - AioContext 使用 aio_poll()
  - GMainContext 使用 qemu_poll_ns()

## docs/devel/multiple-iothreads.txt

The default event loop is called the main loop (see main-loop.c).  It is
possible to create additional event loop threads using -object
iothread,id=my-iothread.

- [ ] 原来 iothread 可以用于替换 main_loop 的啊
- [ ] 那么 worker 线程 和 iothread 是一个东西吗


The main loop is also deeply associated with the QEMU global mutex, which is a
scalability bottleneck in itself.  vCPU threads and the main loop use the QEMU
global mutex to serialize execution of QEMU code.  This mutex is necessary
because a lot of QEMU's code historically was not thread-safe.

The AioContext can be obtained from the IOThread using
iothread_get_aio_context() or for the main loop using qemu_get_aio_context().

## related files
- util/async.c
  - aio_context_new
  - aio_bh_poll
- main-loop.c
  - qemu_set_fd_handler
  - event_notifier_set_handler
- iothread.c

main-loop.c 和 iothread.c 中分别部署的就是两个代码，

## QEMU 中的那些地方需要 lock

emmm 其实就是收集一下那些函数的调用位置而已。

1. tcg_region : 一个 region 只会分配给一个 cpu, 所以防止同时分配给多个 cpu 了


## 如何移植 cpus.c 啊

- [ ] tb_flush 中，通过 cpu_in_exclusive_context 来运行
  - [ ] 另一个 cpu 正在运行，此时进行 tb_flush, 如果保证运行的 cpu 没有读取错误的 TLB
- [ ] async_safe_run_on_cpu 的实现原理
  - [ ] 如果哪一个 cpu 正好在运行，和 cpu 没有运行，处理的情况有没有区别 ?

- 一些初始化的代码需要重新分析 : qemu_tcg_init_vcpu


## AioContext
很多内容都是定义在 include/block/aio.h 中的

- AioContext
  - ThreadPool

- aio_set_fd_handler : 一个 AioContext 可以注册多个 IOHandler

- AioContext 通过 new 来创建，是一个单例，注意这些函数都没有加锁

- 向线程池添加任务的函数为 thread_pool_submit_aio

- [ ] 都是谁需要异步的操作。

- [io_uring in QEMU: high-performance disk IO for Linux](https://archive.fosdem.org/2020/schedule/event/vai_io_uring_in_qemu/attachments/slides/4145/export/events/attachments/vai_io_uring_in_qemu/slides/4145/io_uring_fosdem.pdf)

Qemu event loop is based on AIO context

- [Improving the QEMU Event Loop](http://events17.linuxfoundation.org/sites/events/files/slides/Improving%20the%20QEMU%20Event%20Loop%20-%203.pdf)

- [ ] iothread 到底是哪一个线程

The "original" iothread
* Dispatches fd events
  – aio: block I/O, ioeventfd
  – iohandler: net, nbd, audio, ui, vfio, ...
  – slirp: -net user – chardev: -chardev XXX
* Non-fd services
  – timers
  – bottom halves

这是几个关键的接口:
- [ ] aio_set_fd_handler
- [ ] aio_set_event_handler
- [ ] aio_timer_init
- [ ] aio_bh_new

```c
/*
#0  aio_context_new (errp=0x555556617188 <error_abort>) at ../util/async.c:516
#1  0x0000555555e82510 in iohandler_init () at ../util/main-loop.c:562
#2  0x0000555555e82811 in iohandler_init () at ../util/main-loop.c:568
#3  iohandler_get_aio_context () at ../util/main-loop.c:568
#4  0x0000555555ddb44c in monitor_init_globals_core () at ../monitor/monitor.c:690
#5  0x0000555555cd8aca in monitor_init_globals () at ../monitor/misc.c:1977
#6  0x0000555555c09997 in qemu_init_subsystems () at ../softmmu/runstate.c:768
#7  0x0000555555cfb38f in qemu_init (argc=33, argv=0x7fffffffd618, envp=<optimized out>) at ../softmmu/vl.c:2766
#8  0x0000555555940c8d in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49
```

```c
/*
 * Functions to operate on the I/O handler AioContext.
 * This context runs on top of main loop. We can't reuse qemu_aio_context
 * because iohandlers mustn't be polled by aio_poll(qemu_aio_context).
 */
static AioContext *iohandler_ctx;
```
- [ ] 这个注释我看不懂啊!

这么说，一共就是两个 AioContext:
- iohandler_ctx
- qemu_aio_context

#### AioContext::bh_slice_list
在 aio_bh_poll 有一个局部变量 `BHListSlice slice;`,
也就是 aio_bh_poll 向下调用的时候，还会存在继续调用到 aio_bh_poll 上。


#### aio_poll
- aio_poll
    - aio_bh_poll
      - aio_bh_dequeue
      - aio_bh_call

## IOThread

```c
struct IOThread {
    AioContext *ctx;
    GMainContext *worker_context;
    GMainLoop *main_loop;

    QemuThread thread;
    QemuMutex init_done_lock;
    QemuCond init_done_cond;    /* is thread initialization done? */
    bool stopping;
};
```
- [ ] GMainLoop 是干啥的?
- [ ] GMainContext 是干啥的

## 找到所有的 thread 创建的位置

在 qemu_thread_create 中, 其实 QEMU 还会通过 fork 创建 thread/process 之类的，但是常规流程不是这么使用的:
```plain
huxueshi:qemu_thread_create call_rcu
huxueshi:qemu_thread_create worker
huxueshi:qemu_thread_create worker

huxueshi:qemu_thread_create CPU 0/KVM
huxueshi:qemu_thread_create CPU 1/KVM
```


使用 gdb[^8][^9] 分析一下:
`info thread`
```plain
  Id   Target Id                                             Frame
* 1    Thread 0x7fffeb1d2300 (LWP 1186979) "qemu-system-x86" 0x00007ffff61a6bf6 in __ppoll (fds=0x555556ba96a0, nfds=8, timeout=<optimized out>, timeout@entry=0x7ffffff fd450, sigmask=sigmask@entry=0x0) at ../sysdeps/unix/sysv/linux/ppoll.c:44
  2    Thread 0x7fffeb071700 (LWP 1186983) "qemu-system-x86" syscall () at ../sysdeps/unix/sysv/linux/x86_64/syscall.S:38
  3    Thread 0x7fffea5f9700 (LWP 1186988) "gmain"           0x00007ffff61a6aff in __GI___poll (fds=0x5555569bf770, nfds=1, timeout=-1) at ../sysdeps/unix/sysv/linux/poll.c:29
  4    Thread 0x7fffe9df8700 (LWP 1186989) "gdbus"           0x00007ffff61a6aff in __GI___poll (fds=0x5555569cbfb0, nfds=2, timeout=-1) at ../sysdeps/unix/sysv/linux/poll.c:29
  5    Thread 0x7fffe92f3700 (LWP 1186990) "qemu-system-x86" 0x00007ffff6296618 in futex_abstimed_wait_cancelable (private=0, abstime=0x7fffe92ef220, clockid=0, expected=0, futex_word=0x555556701788) at ../sysdeps/nptl/futex-internal.h:320
  6    Thread 0x7fffe8910700 (LWP 1186993) "qemu-system-x86" 0x00007ffff61a850b in ioctl () at ../sysdeps/unix/syscall-template.S:78
  7    Thread 0x7fffd9ffd700 (LWP 1186994) "qemu-system-x86" 0x00007ffff61a850b in ioctl () at ../sysdeps/unix/syscall-template.S:78
  8    Thread 0x7ffe51629700 (LWP 1186997) "threaded-ml"     0x00007ffff61a6aff in __GI___poll (fds=0x7ffe3c007170, nfds=3, timeout=-1) at ../sysdeps/unix/sysv/linux/po ll.c:29
  9    Thread 0x7ffe26767700 (LWP 1187003) "qemu-system-x86" 0x00007ffff6296618 in futex_abstimed_wait_cancelable (private=0, abstime=0x7ffe26763220, clockid=0, expected=0, futex_word=0x555556701788) at ../sysdeps/nptl/futex-internal.h:320
```
现在一一 backtrace 一下:

#### main loop
main-loop.c 中:
```c
/*
>>> thread 1
[Switching to thread 1 (Thread 0x7fffeb1d2300 (LWP 1186979))]
#0  0x00007ffff61a6bf6 in __ppoll (fds=0x555556ba96a0, nfds=8, timeout=<optimized out>, timeout@entry=0x7fffffffd450, sigmask=sigmask@entry=0x0) at ../sysdeps/unix/sysv
/linux/ppoll.c:44
44      ../sysdeps/unix/sysv/linux/ppoll.c: No such file or directory.
>>> bt
#0  0x00007ffff61a6bf6 in __ppoll (fds=0x555556ba96a0, nfds=8, timeout=<optimized out>, timeout@entry=0x7fffffffd450, sigmask=sigmask@entry=0x0) at ../sysdeps/unix/sysv
/linux/ppoll.c:44
#1  0x0000555555e72675 in ppoll (__ss=0x0, __timeout=0x7fffffffd450, __nfds=<optimized out>, __fds=<optimized out>) at /usr/include/x86_64-linux-gnu/bits/poll2.h:77
#2  qemu_poll_ns (fds=<optimized out>, nfds=<optimized out>, timeout=timeout@entry=4804734) at ../util/qemu-timer.c:348
#3  0x0000555555e82705 in os_host_main_loop_wait (timeout=4804734) at ../util/main-loop.c:250
#4  main_loop_wait (nonblocking=nonblocking@entry=0) at ../util/main-loop.c:531
#5  0x0000555555c09651 in qemu_main_loop () at ../softmmu/runstate.c:726
#6  0x0000555555940c92 in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:50
```
其实，这是很短的一个文件，主要的函数为:
- qemu_init_main_loop
- glib_pollfds_fill
- glib_pollfds_poll
- os_host_main_loop_wait : 这个位置是关键的主循环的
- main_loop_wait : 似乎是一些辅助工具编译的时候会选择这个位置

获取 GMainContext 都是通过调用 g_main_context_default 的，也就是那些默认注册，最后都是放到这里的。


#### call_rcu
```c
/*
>>> thread 2
[Switching to thread 2 (Thread 0x7fffeb071700 (LWP 1186983))]
#0  syscall () at ../sysdeps/unix/sysv/linux/x86_64/syscall.S:38
38      ../sysdeps/unix/sysv/linux/x86_64/syscall.S: No such file or directory.
>>> bt
#0  syscall () at ../sysdeps/unix/sysv/linux/x86_64/syscall.S:38
#1  0x0000555555e7f5b2 in qemu_futex_wait (val=<optimized out>, f=<optimized out>) at /home/maritns3/core/kvmqemu/include/qemu/futex.h:29
#2  qemu_event_wait (ev=ev@entry=0x5555566185c8 <rcu_call_ready_event>) at ../util/qemu-thread-posix.c:480
#3  0x0000555555e84c02 in call_rcu_thread (opaque=opaque@entry=0x0) at ../util/rcu.c:258
#4  0x0000555555e7e5d3 in qemu_thread_start (args=<optimized out>) at ../util/qemu-thread-posix.c:541
#5  0x00007ffff628c609 in start_thread (arg=<optimized out>) at pthread_create.c:477
#6  0x00007ffff61b3293 in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:95
```

#### gmain
```c
/*
>>> thread 3
[Switching to thread 3 (Thread 0x7fffea5f9700 (LWP 1186988))]
#0  0x00007ffff61a6aff in __GI___poll (fds=0x5555569bf770, nfds=1, timeout=-1) at ../sysdeps/unix/sysv/linux/poll.c:29
29      ../sysdeps/unix/sysv/linux/poll.c: No such file or directory.
>>> bt
#0  0x00007ffff61a6aff in __GI___poll (fds=0x5555569bf770, nfds=1, timeout=-1) at ../sysdeps/unix/sysv/linux/poll.c:29
#1  0x00007ffff6ff236e in  () at /lib/x86_64-linux-gnu/libglib-2.0.so.0
#2  0x00007ffff6ff24a3 in g_main_context_iteration () at /lib/x86_64-linux-gnu/libglib-2.0.so.0
#3  0x00007ffff6ff24f1 in  () at /lib/x86_64-linux-gnu/libglib-2.0.so.0
#4  0x00007ffff701bad1 in  () at /lib/x86_64-linux-gnu/libglib-2.0.so.0
#5  0x00007ffff628c609 in start_thread (arg=<optimized out>) at pthread_create.c:477
#6  0x00007ffff61b3293 in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:95
```
#### gdbus
```c
/*
>>> thread 4
[Switching to thread 4 (Thread 0x7fffe9df8700 (LWP 1186989))]
#0  0x00007ffff61a6aff in __GI___poll (fds=0x5555569cbfb0, nfds=2, timeout=-1) at ../sysdeps/unix/sysv/linux/poll.c:29
29      in ../sysdeps/unix/sysv/linux/poll.c
>>> bt
#0  0x00007ffff61a6aff in __GI___poll (fds=0x5555569cbfb0, nfds=2, timeout=-1) at ../sysdeps/unix/sysv/linux/poll.c:29
#1  0x00007ffff6ff236e in  () at /lib/x86_64-linux-gnu/libglib-2.0.so.0
#2  0x00007ffff6ff26f3 in g_main_loop_run () at /lib/x86_64-linux-gnu/libglib-2.0.so.0
#3  0x00007ffff7249f8a in  () at /lib/x86_64-linux-gnu/libgio-2.0.so.0
#4  0x00007ffff701bad1 in  () at /lib/x86_64-linux-gnu/libglib-2.0.so.0
#5  0x00007ffff628c609 in start_thread (arg=<optimized out>) at pthread_create.c:477
#6  0x00007ffff61b3293 in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:95
```

- [ ] https://github.com/chiehmin/gdbus_test

#### worker
thread 9 也是如此的。

KVM forum : [Towards Multi-threaded Device Emulation in QEMU](https://www.linux-kvm.org/images/a/a7/02x04-MultithreadedDevices.pdf)
似懂非懂的样子。

```c
/*
>>> thread 5
[Switching to thread 5 (Thread 0x7fffe92f3700 (LWP 1186990))]
#0  0x00007ffff6296618 in futex_abstimed_wait_cancelable (private=0, abstime=0x7fffe92ef220, clockid=0, expected=0, futex_word=0x555556701788) at ../sysdeps/nptl/futex-
internal.h:320
320     ../sysdeps/nptl/futex-internal.h: No such file or directory.
>>> bt
#0  0x00007ffff6296618 in futex_abstimed_wait_cancelable (private=0, abstime=0x7fffe92ef220, clockid=0, expected=0, futex_word=0x555556701788) at ../sysdeps/nptl/futex-
internal.h:320
#1  do_futex_wait (sem=sem@entry=0x555556701788, abstime=abstime@entry=0x7fffe92ef220, clockid=0) at sem_waitcommon.c:112
#2  0x00007ffff6296743 in __new_sem_wait_slow (sem=sem@entry=0x555556701788, abstime=abstime@entry=0x7fffe92ef220, clockid=0) at sem_waitcommon.c:184
#3  0x00007ffff62967ea in sem_timedwait (sem=sem@entry=0x555556701788, abstime=abstime@entry=0x7fffe92ef220) at sem_timedwait.c:40
#4  0x0000555555e7f36f in qemu_sem_timedwait (sem=sem@entry=0x555556701788, ms=ms@entry=10000) at ../util/qemu-thread-posix.c:327
#5  0x0000555555e7da75 in worker_thread (opaque=opaque@entry=0x555556701710) at ../util/thread-pool.c:91
#6  0x0000555555e7e5d3 in qemu_thread_start (args=<optimized out>) at ../util/qemu-thread-posix.c:541
#7  0x00007ffff628c609 in start_thread (arg=<optimized out>) at pthread_create.c:477
#8  0x00007ffff61b3293 in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:95
```

```c
/*
#0  huxueshi () at ../util/qemu-thread-posix.c:547
#1  0x0000555555e7f765 in qemu_thread_create (thread=thread@entry=0x7fffffffcdd0, name=<optimized out>, name@entry=0x555555e992d0 "worker", start_routine=start_routine@
entry=0x555555e7d980 <worker_thread>, arg=arg@entry=0x555556701710, mode=mode@entry=1) at ../util/qemu-thread-posix.c:560
#2  0x0000555555e7d90d in do_spawn_thread (pool=pool@entry=0x555556701710) at ../util/thread-pool.c:134
#3  0x0000555555e7d965 in spawn_thread_bh_fn (opaque=0x555556701710) at ../util/thread-pool.c:142
#4  0x0000555555e63938 in aio_bh_poll (ctx=ctx@entry=0x55555670ab70) at ../util/async.c:169
#5  0x0000555555e7ad56 in aio_poll (ctx=ctx@entry=0x55555670ab70, blocking=blocking@entry=true) at ../util/aio-posix.c:659
#6  0x0000555555d9b715 in qcow2_open (bs=<optimized out>, options=<optimized out>, flags=<optimized out>, errp=<optimized out>) at ../block/qcow2.c:1909
#7  0x0000555555d7f455 in bdrv_open_driver (bs=bs@entry=0x555556af6400, drv=drv@entry=0x5555565c8560 <bdrv_qcow2>, node_name=<optimized out>, options=options@entry=0x555556b11aa0, open_flags=139266, errp=errp@entry=0x7fffffffd030) at ../block.c:1552
#8  0x0000555555d82524 in bdrv_open_common (errp=0x7fffffffd030, options=0x555556b11aa0, file=0x555556a20ad0, bs=0x555556af6400) at ../block.c:1827
#9  bdrv_open_inherit (filename=<optimized out>, filename@entry=0x555556954a70 "/home/maritns3/core/vn/hack/qemu/x64-e1000/alpine.qcow2", reference=reference@entry=0x0,options=0x555556b11aa0, options@entry=0x555556ac3a90, flags=<optimized out>, flags@entry=0, parent=parent@entry=0x0, child_class=child_class@entry=0x0, child_role=0, errp=0x555556617180 <error_fatal>) at ../block.c:3747
#10 0x0000555555d83607 in bdrv_open (filename=filename@entry=0x555556954a70 "/home/maritns3/core/vn/hack/qemu/x64-e1000/alpine.qcow2", reference=reference@entry=0x0, options=options@entry=0x555556ac3a90, flags=flags@entry=0, errp=errp@entry=0x555556617180 <error_fatal>) at ../block.c:3840
#11 0x0000555555dc80bf in blk_new_open (filename=filename@entry=0x555556954a70 "/home/maritns3/core/vn/hack/qemu/x64-e1000/alpine.qcow2", reference=reference@entry=0x0, options=options@entry=0x555556ac3a90, flags=0, errp=errp@entry=0x555556617180 <error_fatal>) at ../block/block-backend.c:435
#12 0x0000555555d37178 in blockdev_init (file=file@entry=0x555556954a70 "/home/maritns3/core/vn/hack/qemu/x64-e1000/alpine.qcow2", bs_opts=bs_opts@entry=0x555556ac3a90, errp=errp@entry=0x555556617180 <error_fatal>) at ../blockdev.c:608
#13 0x0000555555d3811d in drive_new (all_opts=<optimized out>, block_default_type=<optimized out>, errp=0x555556617180 <error_fatal>) at ../blockdev.c:992
#14 0x0000555555cf8ee6 in drive_init_func (opaque=<optimized out>, opts=<optimized out>, errp=<optimized out>) at ../softmmu/vl.c:617
#15 0x0000555555e6c6e2 in qemu_opts_foreach (list=<optimized out>, func=func@entry=0x555555cf8ed0 <drive_init_func>, opaque=opaque@entry=0x55555681d1b0, errp=errp@entry=0x555556617180 <error_fatal>) at ../util/qemu-option.c:1135
#16 0x0000555555cfd8da in configure_blockdev (bdo_queue=0x5555565641d0 <bdo_queue>, snapshot=0, machine_class=0x55555681d100) at ../softmmu/vl.c:676
#17 qemu_create_early_backends () at ../softmmu/vl.c:1939
#18 qemu_init (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/vl.c:3645
#19 0x0000555555940c8d in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49
```

- [ ] AIO_WAIT_WHILE : 一个有趣的位置，这个会去调用 aio_poll 的
    - [ ] 但是我的龟龟啊，你知不知道，这意味着这个调用 poll 的会一直等待到这个位置上。
      - 但是调用 AIO_WAIT_WHILE 的位置不要太多啊

- [ ] 表示并没有办法理解 worker thread 的作用
  - spawn_thread_bh_fn 产生的
  - 就是在 worker_thread 这个循环中间吧

如果想要提交任务 : 在 thread_pool_submit_aio 中 qemu_sem_post  ThreadPool::sem 这会让 worker_thread 从这个 lock 上醒过来
然后会从 ThreadPool::request_list 中获取需要执行的函数，最后使用 `qemu_bh_schedule(pool->completion_bh)` 通知这个任务结束了

其实整个 thread-pool.c 也就是只有 300 行

#### kvm thread
thread 7 也是
```c
/*
>>> thread 6
[Switching to thread 6 (Thread 0x7fffe8910700 (LWP 1186993))]
#0  0x00007ffff61a850b in ioctl () at ../sysdeps/unix/syscall-template.S:78
78      ../sysdeps/unix/syscall-template.S: No such file or directory.
>>> bt
#0  0x00007ffff61a850b in ioctl () at ../sysdeps/unix/syscall-template.S:78
#1  0x0000555555c38eae in kvm_vcpu_ioctl (cpu=cpu@entry=0x555556b06ca0, type=type@entry=44672) at ../accel/kvm/kvm-all.c:3017
#2  0x0000555555c38ff9 in kvm_cpu_exec (cpu=cpu@entry=0x555556b06ca0) at ../accel/kvm/kvm-all.c:2843
#3  0x0000555555c47265 in kvm_vcpu_thread_fn (arg=arg@entry=0x555556b06ca0) at ../accel/kvm/kvm-accel-ops.c:49
#4  0x0000555555e7e5d3 in qemu_thread_start (args=<optimized out>) at ../util/qemu-thread-posix.c:541
#5  0x00007ffff628c609 in start_thread (arg=<optimized out>) at pthread_create.c:477
#6  0x00007ffff61b3293 in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:95
```


#### threaded-ml
```c
/*
>>> thread 8
[Switching to thread 8 (Thread 0x7ffe51629700 (LWP 1186997))]
#0  0x00007ffff61a6aff in __GI___poll (fds=0x7ffe3c007170, nfds=3, timeout=-1) at ../sysdeps/unix/sysv/linux/poll.c:29
29      ../sysdeps/unix/sysv/linux/poll.c: No such file or directory.
>>> bt
#0  0x00007ffff61a6aff in __GI___poll (fds=0x7ffe3c007170, nfds=3, timeout=-1) at ../sysdeps/unix/sysv/linux/poll.c:29
#1  0x00007ffff6df31d6 in  () at /lib/x86_64-linux-gnu/libpulse.so.0
#2  0x00007ffff6de4841 in pa_mainloop_poll () at /lib/x86_64-linux-gnu/libpulse.so.0
#3  0x00007ffff6de4ec3 in pa_mainloop_iterate () at /lib/x86_64-linux-gnu/libpulse.so.0
#4  0x00007ffff6de4f70 in pa_mainloop_run () at /lib/x86_64-linux-gnu/libpulse.so.0
#5  0x00007ffff6df311d in  () at /lib/x86_64-linux-gnu/libpulse.so.0
#6  0x00007ffff56f272c in  () at /usr/lib/x86_64-linux-gnu/pulseaudio/libpulsecommon-13.99.so
#7  0x00007ffff628c609 in start_thread (arg=<optimized out>) at pthread_create.c:477
#8  0x00007ffff61b3293 in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:95
```




## mmap_lock

```c
static pthread_mutex_t mmap_mutex = PTHREAD_MUTEX_INITIALIZER;
static __thread int mmap_lock_count;

void mmap_lock(void)
{
    if (mmap_lock_count++ == 0) {
        pthread_mutex_lock(&mmap_mutex);
    }
}
```
利用 mmap_lock_count 一个 thread 可以反复上锁，但是可以防止其他 thread 并发访问。

那么只有用户态才需要啊 ?

参考两个资料:
1. https://qemu.readthedocs.io/en/latest/devel/multi-thread-tcg.html
2. tcg_region_init 上面的注释

用户态的线程数量可能很大，所以创建多个 region 是不合适的，所以只创建一个，
而且用户进程的代码大多数都是相同，所以 tb 相关串行也问题不大。
## mttcg
[^1] 指出
1. 如果需要支持 icount 机制将会消失
2. 想要让一个 guest 架构支持 mttcg 需要完成的工作
3. Enabling strong-on-weak memory consistency (e.g. emulate x86 on an ARM host)

## iothread 的 lock 应该只有很少的位置才对啊
- 线程模型通常使用 QEMU 大锁进行同步，获取锁的函数为 qemu_mutex_lock_iothread

主要使用 qemu_mutex_unlock_iothread 的主要在
/home/maritns3/core/ld/DuckBuBi/src/tcg/cpu-exec.c
和
/home/maritns3/core/ld/DuckBuBi/src/qemu/memory_ldst.c.inc

但是 qemu_mutex_lock_iothread 主要只是出现在
/home/maritns3/core/ld/DuckBuBi/src/tcg/cpu-exec.c 中
而且是为了保护 cpu_handle_interrupt

在 io_readx 和 io_writex 当 `mr->global_locking` 时候需要进行

- 主要是在处理中断的时候

在 /home/maritns3/core/ld/DuckBuBi/src/qemu/memory_ldst.c.inc 中间还有一堆 RCU_READ_LOCK

## 如果 mttcg 之外，iothread 之外，还有什么 thread 的挑战
- [ ] 在 translate-all.c 的 page_lock

## [^3]
- QEMU architecture
  - QEMU architecture (up to 0.15) : 这个版本只有同时只有一个 cpu_exec 执行，执行流在处理 io 和翻译之间交替进行
  - QEMU architecture (1.0) : 多个 cpu_exec 可以同时执行，所以只有一个 cpu_exec 可以获取 BQL 从而处理 io 事件。
- virtio-blk-dataplane architecture
- Unlocked memory dispatch
- Unlocked MMIO

RCU is a bulk reference-counting mechanism
- [ ] 我感觉 RCU 在 QEMU 中间的作用是，如果 writer 已经释放了资源，但是这个资源真正释放的时间是 reader 不在使用的时候


Virtio-blk-data-plane 允许块 I/O 处理操作与其他的虚拟设备并行运行，去除 big-qemu-lock 的影响，达到高 I/O 性能的结果。 [^4]

## [^5]
在 A TCG primer 很好的总结了 TCG 的工作模式以及退出的原因。

在 Atomics,
Save load value/address and check on store,
Load-link/store-conditional instruction support via SoftMMU,
Link helpers and instrumented stores,
中应该是分析了在 TCG 需要增加的工作

在 Memory coherency 分析的东西，暂时有点迷茫，不知道想要表达什么东西。 // TODO

## global_locking
```c
/**
 * memory_region_clear_global_locking: Declares that access processing does
 *                                     not depend on the QEMU global lock.
 *
 * By clearing this property, accesses to the memory region will be processed
 * outside of QEMU's global lock (unless the lock is held on when issuing the
 * access request). In this case, the device model implementing the access
 * handlers is responsible for synchronization of concurrency.
 *
 * @mr: the memory region to be updated.
 */
void memory_region_clear_global_locking(MemoryRegion *mr);
```
- 从上下文知道，这里的 QEMU's global lock 就是 QEMU big lock
- 而且 QEMU big lock 就是用于处理 memory region 的
- memory_region_clear_global_locking 从来都不会被调用

- [ ] 用于进一步简化 memory_ldst

## 反手看一下 kvm 的 thread 是如何实现的
kvm_vcpu_thread_fn

- [x] 是哪一个线程来进行处理进行 monitor 的
```c
/*
#0  help_cmd (mon=0x555555f4ce40 <trace_qemu_mutex_unlock+41>, name=0x7fffffffc1e0 "\222\315\rVUU") at ../monitor/hmp.c:277
#1  0x0000555555d23d87 in do_help_cmd (mon=0x555556baf1e0, qdict=0x555557d60000) at ../monitor/misc.c:170
#2  0x0000555555b1ba37 in handle_hmp_command (mon=0x555556baf1e0, cmdline=0x555556c808b4 "") at ../monitor/hmp.c:1105
#3  0x0000555555b19135 in monitor_command_cb (opaque=0x555556baf1e0, cmdline=0x555556c808b0 "help", readline_opaque=0x0) at ../monitor/hmp.c:48
#4  0x0000555555f31fe2 in readline_handle_byte (rs=0x555556c808b0, ch=13) at ../util/readline.c:411
#5  0x0000555555b1c58c in monitor_read (opaque=0x555556baf1e0, buf=0x7fffffffc4a0 "\r", size=1) at ../monitor/hmp.c:1343
#6  0x0000555555e8c5fd in qemu_chr_be_write_impl (s=0x555556c60a80, buf=0x7fffffffc4a0 "\r", len=1) at ../chardev/char.c:201
#7  0x0000555555e8c668 in qemu_chr_be_write (s=0x555556c60a80, buf=0x7fffffffc4a0 "\r", len=1) at ../chardev/char.c:213
#8  0x0000555555e90981 in fd_chr_read (chan=0x555556b8c7f0, cond=G_IO_IN, opaque=0x555556c60a80) at ../chardev/char-fd.c:68
#9  0x0000555555d83ae0 in qio_channel_fd_source_dispatch (source=0x555556b2fa90, callback=0x555555e9084a <fd_chr_read>, user_data=0x555556c60a80) at ../io/channel-watch.c:84
#10 0x00007ffff79d404e in g_main_context_dispatch () at /lib/x86_64-linux-gnu/libglib-2.0.so.0
#11 0x0000555555f4eee9 in glib_pollfds_poll () at ../util/main-loop.c:231
#12 0x0000555555f4ef67 in os_host_main_loop_wait (timeout=0) at ../util/main-loop.c:254
#13 0x0000555555f4f07b in main_loop_wait (nonblocking=0) at ../util/main-loop.c:530
#14 0x0000555555c5c769 in qemu_main_loop () at ../softmmu/runstate.c:731
#15 0x000055555582e57a in main (argc=30, argv=0x7fffffffd748, envp=0x7fffffffd840) at ../softmmu/main.c:50
```

process_queued_cpu_work 用于处理 run_cpu 之类挂载的函数

qemu_wait_io_event 中最后会卡到: `qemu_cond_wait(cpu->halt_cond, &qemu_global_mutex);`

之前 qemu_wait_io_event 总是会被 qemu_cpu_kick 解救一下, 实际上，
之所以如此，是因为有些事情只能通过 run_on_cpu 运行，这个时候线程又是卡住了，
所以在 qemu_cpu_kick 的时候放行，所以，
```c
/*
#0  qemu_cpu_kick (cpu=0x56c0f570) at ../softmmu/cpus.c:456
#1  0x00005555558ca659 in queue_work_on_cpu (cpu=0x555556d5e000, wi=0x7fffffffd3c0) at ../cpus-common.c:131
#2  0x00005555558ca6df in do_run_on_cpu (cpu=0x555556d5e000, func=0x555555d62f9b <do_kvm_cpu_synchronize_post_init>, data=..., mutex=0x5555567a3da0 <qemu_global_mutex>) at ../cpus-common.c:150
#3  0x0000555555c7b522 in run_on_cpu (cpu=0x555556d5e000, func=0x555555d62f9b <do_kvm_cpu_synchronize_post_init>, data=...) at ../softmmu/cpus.c:387
#4  0x0000555555d62ff6 in kvm_cpu_synchronize_post_init (cpu=0x555556d5e000) at ../accel/kvm/kvm-all.c:2730
#5  0x0000555555c7afd9 in cpu_synchronize_post_init (cpu=0x555556d5e000) at ../softmmu/cpus.c:186
#6  0x0000555555c7ae7e in cpu_synchronize_all_post_init () at ../softmmu/cpus.c:156
#7  0x0000555555965d48 in qdev_machine_creation_done () at ../hw/core/machine.c:1261
#8  0x0000555555c69a91 in qemu_machine_creation_done () at ../softmmu/vl.c:2579
#9  0x0000555555c69b64 in qmp_x_exit_preconfig (errp=0x5555567a86e8 <error_fatal>) at ../softmmu/vl.c:2602
#10 0x0000555555c6c266 in qemu_init (argc=30, argv=0x7fffffffd748, envp=0x7fffffffd840) at ../softmmu/vl.c:3637
#11 0x000055555582e575 in main (argc=30, argv=0x7fffffffd748, envp=0x7fffffffd840) at ../softmmu/main.c:49
```

总体来说, 是靠 halt_cond 来让 vcpu thread 运行的，但是实际上还存在别的东西

cpu_thread_is_idle
```c
/*
#0  runstate_set (new_state=21845) at ../softmmu/runstate.c:201
#1  0x0000555555c7bf6d in vm_prepare_start () at ../softmmu/cpus.c:690
#2  0x0000555555c7bfa4 in vm_start () at ../softmmu/cpus.c:697
#3  0x00005555558be926 in qmp_cont (errp=0x0) at ../monitor/qmp-cmds.c:152
#4  0x0000555555c69c36 in qmp_x_exit_preconfig (errp=0x5555567a86e8 <error_fatal>) at ../softmmu/vl.c:2626
#5  0x0000555555c6c202 in qemu_init (argc=30, argv=0x7fffffffd748, envp=0x7fffffffd840) at ../softmmu/vl.c:3635
#6  0x000055555582e575 in main (argc=30, argv=0x7fffffffd748, envp=0x7fffffffd840) at ../softmmu/main.c:49
```

## 为什么 kvm 中的 pio 和 mmio 不需要使用 BQL 保护

去 trace 这个代码的时候，最后发现
flatview_write_continue 中和 memory_ldst.c 的函数都会调用
prepare_mmio_access, 这个就是处理 mmio 的啊。

```diff
History:        #0
Commit:         de7ea885c5394c1fba7443cbf33bd2745d32e6c2
Author:         Paolo Bonzini <pbonzini@redhat.com>
Author Date:    Fri 19 Jun 2015 12:47:26 AM CST
Committer Date: Wed 01 Jul 2015 09:45:51 PM CST

kvm: Switch to unlocked MMIO

Do not take the BQL before dispatching MMIO requests of KVM VCPUs.
Instead, address_space_rw will do it if necessary. This enables completely
BQL-free MMIO handling in KVM mode for upcoming devices with fine-grained
locking.

Signed-off-by: Paolo Bonzini <pbonzini@redhat.com>
Message-Id: <1434646046-27150-10-git-send-email-pbonzini@redhat.com>

diff --git a/kvm-all.c b/kvm-all.c
index ad5ac5e3df..df57da0bf2 100644
--- a/kvm-all.c
+++ b/kvm-all.c
@@ -1814,13 +1814,12 @@ int kvm_cpu_exec(CPUState *cpu)
             break;
         case KVM_EXIT_MMIO:
             DPRINTF("handle_mmio\n");
-            qemu_mutex_lock_iothread();
+            /* Called outside BQL */
             address_space_rw(&address_space_memory,
                              run->mmio.phys_addr, attrs,
                              run->mmio.data,
                              run->mmio.len,
                              run->mmio.is_write);
-            qemu_mutex_unlock_iothread();
             ret = 0;
             break;
         case KVM_EXIT_IRQ_WINDOW_OPEN:
```

## current_cpu
从一般的想法来说，current_cpu 和当前 thread 始终是绑定的，实际上，并不是，
至少没有启动 mttcg 的时候，是一个线程模拟多个 cpu 的，那么 current_cpu 就是在每次切换模拟的核都是需要修改 current_cpu 的
注释也说的很有道理，current_cpu 需要在 cpu_exec 中进行正确赋值的

```c
/* current CPU in the current thread. It is only valid inside
   cpu_exec() */
extern CPUState *current_cpu;
```

观察:
1. current_cpu 永远不会被赋值为 NULL
2. current_cpu 被赋值为空的情况可以用于判断当前 thread 是否为 VCPU thread
3. rr_start_vcpu_thread : 只有会为 CPUState 创建线程，接下来的 CPUState 都是共享这个线程的
4. -accel tcg,thread=single 通过 rr_wait_io_event 来唤醒
5 在 rr_cpu_thread_fn 中如何实现切换不同的 CPU 的

## do_run_on_cpu

问题 1: 真的需要 do_run_on_cpu，我的意思是，使用目标 cpu 作为参数调用一下函数，
  - [ ] 猜测，最好不要让一个线程来访问另一个线程的 cpu
```c
/*
#0  do_run_on_cpu (cpu=0x55555609ef9a, func=0x2155b8da1e, data=..., mutex=0x55555679d7e0 <qemu_global_mutex>) at ../cpus-common.c:136
#1  0x0000555555c79501 in run_on_cpu (cpu=0x555556cf6e00, func=0x555555b8da1e <vapic_do_enable_tpr_reporting>, data=...) at ../softmmu/cpus.c:385
#2  0x0000555555b8dae1 in vapic_enable_tpr_reporting (enable=false) at ../hw/i386/kvmvapic.c:511
#3  0x0000555555b8db5e in vapic_reset (dev=0x555556b40ca0) at ../hw/i386/kvmvapic.c:521
#4  0x0000555555d76093 in device_transitional_reset (obj=0x555556b40ca0) at ../hw/core/qdev.c:1028
#5  0x0000555555d77de5 in resettable_phase_hold (obj=0x555556b40ca0, opaque=0x0, type=RESET_TYPE_COLD) at ../hw/core/resettable.c:182
#6  0x0000555555d70357 in bus_reset_child_foreach (obj=0x555556a50fc0, cb=0x555555d77cb3 <resettable_phase_hold>, opaque=0x0, type=RESET_TYPE_COLD) at ../hw/core/bus.c:97
#7  0x0000555555d77ae0 in resettable_child_foreach (rc=0x5555569bef60, obj=0x555556a50fc0, cb=0x555555d77cb3 <resettable_phase_hold>, opaque=0x0, type=RESET_TYPE_COLD)at ../hw/core/resettable.c:96
#8  0x0000555555d77d6b in resettable_phase_hold (obj=0x555556a50fc0, opaque=0x0, type=RESET_TYPE_COLD) at ../hw/core/resettable.c:173
#9  0x0000555555d77985 in resettable_assert_reset (obj=0x555556a50fc0, type=RESET_TYPE_COLD) at ../hw/core/resettable.c:60
#10 0x0000555555d778c5 in resettable_reset (obj=0x555556a50fc0, type=RESET_TYPE_COLD) at ../hw/core/resettable.c:45
#11 0x0000555555d78135 in resettable_cold_reset_fn (opaque=0x555556a50fc0) at ../hw/core/resettable.c:269
#12 0x0000555555d76576 in qemu_devices_reset () at ../hw/core/reset.c:69
#13 0x0000555555b97730 in pc_machine_reset (machine=0x5555569069e0) at ../hw/i386/pc.c:1644
#14 0x0000555555c59ffc in qemu_system_reset (reason=SHUTDOWN_CAUSE_NONE) at ../softmmu/runstate.c:442
#15 0x0000555555963ddf in qdev_machine_creation_done () at ../hw/core/machine.c:1299
#16 0x0000555555c67aec in qemu_machine_creation_done () at ../softmmu/vl.c:2579
#17 0x0000555555c67bbf in qmp_x_exit_preconfig (errp=0x5555567a2128 <error_fatal>) at ../softmmu/vl.c:2602
#18 0x0000555555c6a28d in qemu_init (argc=29, argv=0x7fffffffd748, envp=0x7fffffffd838) at ../softmmu/vl.c:3635
#19 0x000055555582c575 in main (argc=29, argv=0x7fffffffd748, envp=0x7fffffffd838) at ../softmmu/main.c:49
```

```c
/*
#0  do_run_on_cpu (cpu=0x55555609ef9a, func=0x2100000012, data=..., mutex=0x7fffe888f8e0) at ../cpus-common.c:136
#1  0x0000555555c79501 in run_on_cpu (cpu=0x555556d05800, func=0x555555b8da1e <vapic_do_enable_tpr_reporting>, data=...) at ../softmmu/cpus.c:385
#2  0x0000555555b8dae1 in vapic_enable_tpr_reporting (enable=true) at ../hw/i386/kvmvapic.c:511
#3  0x0000555555b8df25 in vapic_prepare (s=0x555556b40ca0) at ../hw/i386/kvmvapic.c:633
#4  0x0000555555b8e028 in vapic_write (opaque=0x555556b40ca0, addr=0, data=32, size=2) at ../hw/i386/kvmvapic.c:673
#5  0x0000555555d43441 in memory_region_write_accessor (mr=0x555556b40fc0, addr=0, value=0x7fffe888fb08, size=2, shift=0, mask=65535, attrs=...) at ../softmmu/memory.c:489
#6  0x0000555555d43678 in access_with_adjusted_size (addr=0, value=0x7fffe888fb08, size=2, access_size_min=1, access_size_max=4, access_fn=0x555555d43354 <memory_region_write_accessor>, mr=0x555556b40fc0, attrs=...) at ../softmmu/memory.c:550
#7  0x0000555555d46727 in memory_region_dispatch_write (mr=0x555556b40fc0, addr=0, data=32, op=MO_16, attrs=...) at ../softmmu/memory.c:1500
#8  0x0000555555c844a0 in address_space_stw_internal (as=0x55555679d940 <address_space_io>, addr=126, val=32, attrs=..., result=0x0, endian=DEVICE_NATIVE_ENDIAN) at /home/maritns3/core/kvmqemu/memory_ldst.c.inc:415
#9  0x0000555555c845a5 in address_space_stw (as=0x55555679d940 <address_space_io>, addr=126, val=32, attrs=..., result=0x0) at /home/maritns3/core/kvmqemu/memory_ldst.c.inc:446
#10 0x0000555555b51b70 in helper_outw (env=0x555556d0e080, port=126, data=32) at ../target/i386/tcg/sysemu/misc_helper.c:42
#11 0x00007fff54190c8c in code_gen_buffer ()
#12 0x0000555555c5fdd0 in cpu_tb_exec (cpu=0x555556d05800, itb=0x7fff94190b40, tb_exit=0x7fffe88901a0) at ../accel/tcg/cpu-exec.c:190
#13 0x0000555555c60d54 in cpu_loop_exec_tb (cpu=0x555556d05800, tb=0x7fff94190b40, last_tb=0x7fffe88901a8, tb_exit=0x7fffe88901a0) at ../accel/tcg/cpu-exec.c:673
#14 0x0000555555c61045 in cpu_exec (cpu=0x555556d05800) at ../accel/tcg/cpu-exec.c:798
#15 0x0000555555c11160 in tcg_cpus_exec (cpu=0x555556d05800) at ../accel/tcg/tcg-accel-ops.c:67
#16 0x0000555555d31aa9 in rr_cpu_thread_fn (arg=0x555556d05800) at ../accel/tcg/tcg-accel-ops-rr.c:216
#17 0x0000555555f4c216 in qemu_thread_start (args=0x555556ab3f60) at ../util/qemu-thread-posix.c:521
#18 0x00007ffff6298609 in start_thread (arg=<optimized out>) at pthread_create.c:477
#19 0x00007ffff61bd293 in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:95
```

## first_cpu / CPU_NEXT / CPU_FOREACH 的移植
- [x] 这些 cpus 是如何初始化的
  - cpu_list_add 和 cpu_list_remove, 通过 CPUState::node 实现的

- [x] 为什么又要采用 RCU 的机制 : 因为 cpu_list_remove 的原因

- [x] first_cpu 和 CPU_NEXT 都是只有一个使用位置的
  - first_cpu 和 CPU_NEXT 场景分析之后，无论如何，将 RCU 去掉后，其语义不变

在 pic_irq_request 中，利用 first_cpu 来访问 apic_state, 猜测是因为
apic_state 要么在每一个 cpu 都有，要都没有，所以随便选一个就可以了

## run_on_cpu
- [ ] 关于 CPU_FOREACH 的问题，这里存在一个一直都在思考的问题, 为什么不能直接让这个线程持有 CPUState 直接调用
  - [ ] 其实可以找到最开始的 run_on_cpu 的例子的
  - [ ] 如何移除掉整个 run_on_cpu 的机制
    - [ ] 现在的系统到底是如何使用的

- [x] 思考一个问题，run_on_cpu 如果是单核的，是如何处理?
   - 没有本质区别，将任务挂载该线程上，然后让他启动

## CPUState::created
- x86_cpu_realizefn
  - qemu_init_vcpu
    - rr_start_vcpu_thread : 最后只会创建出来一个线程, 但是只要创建出来了，那么就会设置 CPUState::created 为 true
      - 大致是利用 qemu_thread_create 来调用 rr_cpu_thread_fn

使用的位置:
- qemu_init_vcpu : 因为初始化在 qemu_thread_create 中创建的，所以要等待
- tlb_flush_by_mmuidx : 看下面的调用过程，可以在   tlb_flush_by_mmuidx 中最后调用 async_run_on_cpu 的原因是
  - cpu 当然 created, 其次是进行 apic 相关的初始化工作是主线程进行的，所以 qemu_cpu_is_self 必然失败

```c
void tlb_flush_by_mmuidx(CPUState *cpu, uint16_t idxmap)
{
    tlb_debug("mmu_idx: 0x%" PRIx16 "\n", idxmap);

    if (cpu->created && !qemu_cpu_is_self(cpu)) {
        async_run_on_cpu(cpu, tlb_flush_by_mmuidx_async_work,
                         RUN_ON_CPU_HOST_INT(idxmap));
    } else {
        tlb_flush_by_mmuidx_async_work(cpu, RUN_ON_CPU_HOST_INT(idxmap));
    }
}
```
- 是不是，`cpu->created` 之后才可以 async_run_on_cpu
还是因为 tlb_flush_by_mmuidx_async_work 在 cpu 还没有 created 的时候，无所谓
  - 从 rr_cpu_thread_fn 中初始化 qemu_thread_get_self 的，所以应该这就是真正的原因


```c
/*
#0  tlb_flush_by_mmuidx (cpu=0x555556a59630, idxmap=<optimized out>) at ../accel/tcg/cputlb.c:388
#1  0x0000555555b7f6fe in memory_region_transaction_commit () at ../softmmu/memory.c:1104
#2  memory_region_transaction_commit () at ../softmmu/memory.c:1084
#3  0x0000555555a749db in vapic_realize (dev=<optimized out>, errp=<optimized out>) at ../hw/i386/kvmvapic.c:730
#4  0x0000555555bfc3b7 in device_set_realized (obj=<optimized out>, value=true, errp=0x7fffffffce20) at ../hw/core/qdev.c:761
#5  0x0000555555c0016a in property_set_bool (obj=0x5555568828e0, v=<optimized out>, name=<optimized out>, opaque=0x5555565ceed0, errp=0x7fffffffce20) at ../qom/object.c
:2257
#6  0x0000555555c0267c in object_property_set (obj=obj@entry=0x5555568828e0, name=name@entry=0x555555eb5816 "realized", v=v@entry=0x55555689d7f0, errp=errp@entry=0x5555
564ece00 <error_fatal>) at ../qom/object.c:1402
#7  0x0000555555bfedc4 in object_property_set_qobject (obj=obj@entry=0x5555568828e0, name=name@entry=0x555555eb5816 "realized", value=value@entry=0x55555692b460, errp=e
rrp@entry=0x5555564ece00 <error_fatal>) at ../qom/qom-qobject.c:28
#8  0x0000555555c028c9 in object_property_set_bool (obj=0x5555568828e0, name=0x555555eb5816 "realized", value=<optimized out>, errp=0x5555564ece00 <error_fatal>) at ../
qom/object.c:1472
#9  0x0000555555bfb283 in qdev_realize_and_unref (dev=0x5555568828e0, bus=<optimized out>, errp=<optimized out>) at ../hw/core/qdev.c:396
#10 0x000055555583f8dc in sysbus_realize_and_unref (dev=dev@entry=0x5555568828e0, errp=<optimized out>) at /home/maritns3/core/kvmqemu/include/hw/qdev-core.h:17
#11 0x000055555583f951 in sysbus_create_varargs (name=name@entry=0x555555e6ae1b "kvmvapic", addr=addr@entry=18446744073709551615) at ../hw/core/sysbus.c:236
#12 0x0000555555bde8c5 in sysbus_create_simple (irq=0x0, addr=18446744073709551615, name=0x555555e6ae1b "kvmvapic") at /home/maritns3/core/kvmqemu/include/hw/sysbus.h:1
04
#13 apic_common_realize (dev=0x55555673fd70, errp=0x7fffffffcfb0) at ../hw/intc/apic_common.c:301
#14 0x0000555555bfc3b7 in device_set_realized (obj=<optimized out>, value=true, errp=0x7fffffffd030) at ../hw/core/qdev.c:761
#15 0x0000555555c0016a in property_set_bool (obj=0x55555673fd70, v=<optimized out>, name=<optimized out>, opaque=0x5555565ceed0, errp=0x7fffffffd030) at ../qom/object.c
:2257
#16 0x0000555555c0267c in object_property_set (obj=obj@entry=0x55555673fd70, name=name@entry=0x555555eb5816 "realized", v=v@entry=0x55555689d6e0, errp=errp@entry=0x7fff
ffffd100) at ../qom/object.c:1402
#17 0x0000555555bfedc4 in object_property_set_qobject (obj=obj@entry=0x55555673fd70, name=name@entry=0x555555eb5816 "realized", value=value@entry=0x555556945e00, errp=e
rrp@entry=0x7fffffffd100) at ../qom/qom-qobject.c:28
#18 0x0000555555c028c9 in object_property_set_bool (obj=0x55555673fd70, name=name@entry=0x555555eb5816 "realized", value=value@entry=true, errp=errp@entry=0x7fffffffd10
0) at ../qom/object.c:1472
#19 0x0000555555bfb1e2 in qdev_realize (dev=<optimized out>, bus=bus@entry=0x0, errp=errp@entry=0x7fffffffd100) at ../hw/core/qdev.c:389
#20 0x0000555555a58f29 in x86_cpu_apic_realize (cpu=cpu@entry=0x555556a59630, errp=errp@entry=0x7fffffffd100) at /home/maritns3/core/kvmqemu/include/hw/qdev-core.h:17
#21 0x0000555555ae5fb3 in x86_cpu_realizefn (dev=0x555556a59630, errp=0x7fffffffd160) at ../target/i386/cpu.c:3797
#22 0x0000555555bfc3b7 in device_set_realized (obj=<optimized out>, value=true, errp=0x7fffffffd1e0) at ../hw/core/qdev.c:761
#23 0x0000555555c0016a in property_set_bool (obj=0x555556a59630, v=<optimized out>, name=<optimized out>, opaque=0x5555565ceed0, errp=0x7fffffffd1e0) at ../qom/object.c
:2257
#24 0x0000555555c0267c in object_property_set (obj=obj@entry=0x555556a59630, name=name@entry=0x555555eb5816 "realized", v=v@entry=0x55555689c7f0, errp=errp@entry=0x5555
564ece00 <error_fatal>) at ../qom/object.c:1402
#25 0x0000555555bfedc4 in object_property_set_qobject (obj=obj@entry=0x555556a59630, name=name@entry=0x555555eb5816 "realized", value=value@entry=0x55555689b5b0, errp=e
rrp@entry=0x5555564ece00 <error_fatal>) at ../qom/qom-qobject.c:28
#26 0x0000555555c028c9 in object_property_set_bool (obj=0x555556a59630, name=name@entry=0x555555eb5816 "realized", value=value@entry=true, errp=errp@entry=0x5555564ece0
0 <error_fatal>) at ../qom/object.c:1472
#27 0x0000555555bfb1e2 in qdev_realize (dev=<optimized out>, bus=bus@entry=0x0, errp=errp@entry=0x5555564ece00 <error_fatal>) at ../hw/core/qdev.c:389
#28 0x0000555555a93a15 in x86_cpu_new (x86ms=x86ms@entry=0x5555566445e0, apic_id=0, errp=errp@entry=0x5555564ece00 <error_fatal>) at /home/maritns3/core/kvmqemu/include
/hw/qdev-core.h:17
#29 0x0000555555a93afe in x86_cpus_init (x86ms=x86ms@entry=0x5555566445e0, default_cpu_version=<optimized out>) at ../hw/i386/x86.c:138
#30 0x0000555555a6f533 in pc_init1 (machine=0x5555566445e0, pci_type=0x555555e3b03f "i440FX", host_type=0x555555df8cb8 "i440FX-pcihost") at ../hw/i386/pc_piix.c:157
#31 0x0000555555a27a64 in machine_run_board_init (machine=machine@entry=0x5555566445e0) at ../hw/core/machine.c:1239
#32 0x0000555555bc17be in qemu_init_board () at ../softmmu/vl.c:2526
#33 qmp_x_exit_preconfig (errp=<optimized out>) at ../softmmu/vl.c:2600
#34 0x0000555555bc50c5 in qemu_init (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/vl.c:3635
#35 0x0000555555829a7d in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49
```

## hmp
使用 hmp 作为例子。

- [ ] 到底是如何注册上去的，然后最后就被 glib_pollfds_poll 监听了

```c
/*
#0  handle_hmp_command (mon=0x555556a5d190, cmdline=0x555556c03f30 "") at ../monitor/hmp.c:1080
#1  0x000055555598e841 in monitor_command_cb (opaque=0x555556a5d190, cmdline=0x555556c03f30 "", readline_opaque=<optimized out>) at ../monitor/hmp.c:48
#2  0x0000555555e88bc2 in readline_handle_byte (rs=0x555556c03f30, ch=<optimized out>) at ../util/readline.c:411
#3  0x000055555598e893 in monitor_read (opaque=0x555556a5d190, buf=<optimized out>, size=<optimized out>) at ../monitor/hmp.c:1351
#4  0x0000555555ddc7cd in fd_chr_read (chan=0x555556abe860, cond=<optimized out>, opaque=<optimized out>) at ../chardev/char-fd.c:73
#5  0x00007ffff6ff204e in g_main_context_dispatch () at /lib/x86_64-linux-gnu/libglib-2.0.so.0
#6  0x0000555555e82788 in glib_pollfds_poll () at ../util/main-loop.c:232
#7  os_host_main_loop_wait (timeout=<optimized out>) at ../util/main-loop.c:255
#8  main_loop_wait (nonblocking=nonblocking@entry=0) at ../util/main-loop.c:531
#9  0x0000555555c09651 in qemu_main_loop () at ../softmmu/runstate.c:726
#10 0x0000555555940c92 in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:50
```
哇，很复杂啊!

- [ ] 我发理解为什么会触发这一个, **关键入口**
(qemu) huxueshi:aio_bh_schedule_oneshot_full

```c
/*
#0  aio_bh_schedule_oneshot_full (ctx=ctx@entry=0x55555670ab70, cb=cb@entry=0x555555dd9e90 <monitor_accept_input>, opaque=opaque@entry=0x555556a2b170, name=name@entry=0x555556008a4a "monitor_accept_input") at ../util/async.c:113
#1  0x0000555555ddaf47 in monitor_resume (mon=0x555556a2b170) at ../monitor/monitor.c:550
#2  0x0000555555e88be2 in readline_handle_byte (rs=0x555556b26800, ch=<optimized out>) at ../util/readline.c:411
#3  0x000055555598e893 in monitor_read (opaque=0x555556a2b170, buf=<optimized out>, size=<optimized out>) at ../monitor/hmp.c:1351
#4  0x0000555555ddc7dd in fd_chr_read (chan=0x555556a07000, cond=<optimized out>, opaque=<optimized out>) at ../chardev/char-fd.c:73
#5  0x00007ffff6ff204e in g_main_context_dispatch () at /lib/x86_64-linux-gnu/libglib-2.0.so.0
#6  0x0000555555e827a8 in glib_pollfds_poll () at ../util/main-loop.c:232
#7  os_host_main_loop_wait (timeout=<optimized out>) at ../util/main-loop.c:255
#8  main_loop_wait (nonblocking=nonblocking@entry=0) at ../util/main-loop.c:531
#9  0x0000555555c09661 in qemu_main_loop () at ../softmmu/runstate.c:726
#10 0x0000555555940c92 in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:50
```

实际上，难道不这就是 aio 的动作过程吗?
```c
/*
#0  monitor_accept_input (opaque=0x555556a2b170) at ../monitor/monitor.c:523
#1  0x0000555555e63988 in aio_bh_poll (ctx=ctx@entry=0x55555670ab70) at ../util/async.c:170
#2  0x0000555555e7ac62 in aio_dispatch (ctx=0x55555670ab70) at ../util/aio-posix.c:381
#3  0x0000555555e63842 in aio_ctx_dispatch (source=<optimized out>, callback=<optimized out>, user_data=<optimized out>) at ../util/async.c:312
#4  0x00007ffff6ff217d in g_main_context_dispatch () at /lib/x86_64-linux-gnu/libglib-2.0.so.0
#5  0x0000555555e827a8 in glib_pollfds_poll () at ../util/main-loop.c:232
#6  os_host_main_loop_wait (timeout=<optimized out>) at ../util/main-loop.c:255
#7  main_loop_wait (nonblocking=nonblocking@entry=0) at ../util/main-loop.c:531
#8  0x0000555555c09661 in qemu_main_loop () at ../softmmu/runstate.c:726
#9  0x0000555555940c92 in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:50
*/
```
- [ ] aio_ctx_dispatch 的动作居然是 g_main_context_dispatch 来触发的

## os_host_main_loop_wait
仔细分析一些执行流程:
- g_main_context_acquire
- qemu_mutex_unlock_iothread
- qemu_poll_ns
- qemu_mutex_lock_iothread
- glib_pollfds_poll
- g_main_context_release

os_host_main_loop_wait 调用 qemu_poll_ns 来 poll，而使用 glib_pollfds_poll 来收割。
- [ ] 如果这样，iohandler_ctx 和 qemu_aio_context 有啥用啊

- [ ] 这个执行流程摧毁了我对于 qemu_mutex_lock_iothread 的理解
- [ ] 重新理解一下，iothread 的含义，这个是让只有一个 vcpu 才可以进行 IO 操作，现在，让 io 操作都放到 io thread 中间，vcpu 的执行流程就不应该存在 qemu_mutex_lock_iothread 了吧

## io uring
分析一下 io uring 是如何和 aio 整个框架联系在一起的。

猜测，当然是，io uring 提交，利用 aio 机制，将这个事情获取出来的。

```c
/*
#0  raw_co_preadv (bs=0x555556c98ad0, offset=196608, bytes=16384, qiov=0x7fffe93f6e30, flags=0) at ../block/file-posix.c:2083
#1  0x0000555555d3d7f4 in bdrv_driver_preadv (bs=bs@entry=0x555556c98ad0, offset=offset@entry=196608, bytes=bytes@entry=16384, qiov=qiov@entry=0x7fffe93f6e30, qiov_offset=qiov_offset@entry=0, flags=flags@entry=0) at ../block/io.c:1190
#2  0x0000555555d422ce in bdrv_aligned_preadv (child=child@entry=0x555556a893f0, req=req@entry=0x7fffe93f6cb0, offset=196608, bytes=16384, align=<optimized out>, qiov=0x7fffe93f6e30, qiov_offset=0, flags=0) at ../block/io.c:1577
#3  0x0000555555d42a04 in bdrv_co_preadv_part (child=child@entry=0x555556a893f0, offset=<optimized out>, offset@entry=196608, bytes=<optimized out>, bytes@entry=16384, qiov=<optimized out>, qiov@entry=0x7fffe93f6e30, qiov_offset=<optimized out>, qiov_offset@entry=0, flags=flags@entry=0) at ../block/io.c:1848
#4  0x0000555555d42b1f in bdrv_co_preadv (child=child@entry=0x555556a893f0, offset=offset@entry=196608, bytes=bytes@entry=16384, qiov=qiov@entry=0x7fffe93f6e30, flags=flags@entry=0) at ../block/io.c:1798
#5  0x0000555555d288aa in bdrv_preadv (child=0x555556a893f0, offset=196608, bytes=bytes@entry=16384, qiov=qiov@entry=0x7fffe93f6e30, flags=flags@entry=0) at block/block-gen.c:347
#6  0x0000555555d3ffd1 in bdrv_pread (child=<optimized out>, offset=<optimized out>, buf=<optimized out>, bytes=16384) at ../block/io.c:1097
```
在进一步向下，就是到达了 raw_co_prw 这里，在这里根据配置选择是否采用 io uring 的, 如果采用，那么就调用 luring_co_submit


[^1]: https://wiki.qemu.org/Features/tcg-multithread
[^2]: https://qemu-project.gitlab.io/qemu/devel/multi-thread-tcg.html?highlight=bql
[^3]: https://www.linux-kvm.org/images/1/17/Kvm-forum-2013-Effective-multithreading-in-QEMU.pdf
[^4]: https://blog.csdn.net/memblaze_2011/article/details/48808147
[^5]: https://lwn.net/Articles/697265/
[^6]: https://lwn.net/Articles/517475/
[^7]: https://qemu.readthedocs.io/en/latest/devel/multi-thread-tcg.html
[^8]: https://stackoverflow.com/questions/21926549/get-thread-name-in-gdb
[^9]: https://stackoverflow.com/questions/8944236/gdb-how-to-get-thread-name-displayed
