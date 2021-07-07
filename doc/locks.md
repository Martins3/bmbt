## 问题
- [ ] 第一实际上，mttcg 上运行 x86 指令的时候，mttcg 是默认支持的, 那么李欣宇到底在干什么
- [ ] 之前的华为的形式化验证是不是就是处理内存序列的 ?

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


Virtio-blk-data-plane 允许块I/O处理操作与其他的虚拟设备并行运行，去除big-qemu-lock的影响，达到高I/O性能的结果。 [^4]

## [^5]
在 A TCG primer 很好的总结了 TCG 的工作模式以及退出的原因。

在 Atomics, 
Save load value/address and check on store, 
Load-link/store-conditional instruction support via SoftMMU,
Link helpers and instrumented stores,
中应该是分析了在 TCG 需要增加的工作

在 Memory coherency 分析的东西，暂时有点迷茫，不知道想要表达什么东西。 @todo

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

[^1]: https://wiki.qemu.org/Features/tcg-multithread
[^2]: https://qemu-project.gitlab.io/qemu/devel/multi-thread-tcg.html?highlight=bql
[^3]: https://www.linux-kvm.org/images/1/17/Kvm-forum-2013-Effective-multithreading-in-QEMU.pdf
[^4]: https://blog.csdn.net/memblaze_2011/article/details/48808147
[^5]: https://lwn.net/Articles/697265/
