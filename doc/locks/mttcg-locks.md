# MTTG && Locks

## 问题
- [ ] 第一实际上，mttcg 上运行 x86 指令的时候，mttcg 是默认支持的, 那么李欣宇到底在干什么
- [ ] 之前的华为的形式化验证是不是就是处理内存序列的 ?
- [ ] 算了还是首先支持单核的 x86 cpu 吧

- 虽然短期之类不会支持 host 多核，但是支持 guest 多核还是有必要的

## TODO

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


[^1]: https://wiki.qemu.org/Features/tcg-multithread
[^2]: https://qemu-project.gitlab.io/qemu/devel/multi-thread-tcg.html?highlight=bql
[^3]: https://www.linux-kvm.org/images/1/17/Kvm-forum-2013-Effective-multithreading-in-QEMU.pdf
[^4]: https://blog.csdn.net/memblaze_2011/article/details/48808147
[^5]: https://lwn.net/Articles/697265/
