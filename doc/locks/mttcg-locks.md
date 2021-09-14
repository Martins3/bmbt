# MTTG && Locks

## 问题 && TODO
- [ ] kick 之后可以通过 rr_wait_io_event 进行捕获，kick 如何保证从里面的 while 循环中退出的, 从 interrupt 和 exception 中检查出来的
  - 现在还是 uint16_t IcountDecr::(anonymous struct)::high 中的说明，是可以的
  - [ ] cpu_handle_interrupt 最后的位置是处理了 exit_request 的

- 为什么可以从 cpu_handle_exception 中间退出来，去处理这些蛇皮工作 ?
  - [ ] 始终无法理解 cpu_handle_exception return true 和 return false; 的情况分别是什么

## conditional variable
- 调用 run_on_cpu 的时候需要持有 BQL 的，甚至 cpu_x86_inject_mce 上都是的

## 如何移植 cpus.c 啊
可以留出来接口，但是目前就是只支持单个 vCPU 的。

- 一些初始化的代码需要重新分析 : qemu_tcg_init_vcpu

- cpu_exec_enter 和 cpu_exec_exit 调用 arch 相关的 hook

## 反手看一下 kvm 的 thread 是如何实现的
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

## do_run_on_cpu

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
  - [ ] 猜测，其中的原因是很多 CPU 资源都是 thread private 的

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

## io thread 和 vCPU 线程交互
和设想的没有错误，当 io thread 线程将事情办好了之后，就会将中断注入到 guest 机中，
相关的 backtrace 可以参考 qemu-irq.md 中的 [tcg-msi](#tcg-msi)

vCPU 线程需要不停的去执行，然后不可能使用 aio_poll 的方法来处理。

- 这个就是 tcg 的标准模式，插入一个 flag，然后 qemu_cpu_kick
```c
/* mask must never be zero, except for A20 change call */
void tcg_handle_interrupt(CPUState *cpu, int mask)
{
    g_assert(qemu_mutex_iothread_locked());

    cpu->interrupt_request |= mask;

    /*
     * If called from iothread context, wake the target cpu in
     * case its halted.
     */
    if (!qemu_cpu_is_self(cpu)) {
        qemu_cpu_kick(cpu);
    } else {
        qatomic_set(&cpu_neg(cpu)->icount_decr.u16.high, -1);
    }
}
```

- 而 kvm 的更加容易，在 io thread 线程中间直接调用 kvm_vm_ioctl 就可以了，其他的细节让 kvm 来处理就可以了。

## do_run_on_cpu
```c
void do_run_on_cpu(CPUState *cpu, run_on_cpu_func func, run_on_cpu_data data,
                   QemuMutex *mutex)
{
    struct qemu_work_item wi;

    if (qemu_cpu_is_self(cpu)) {
        func(cpu, data);
        return;
    }

    wi.func = func;
    wi.data = data;
    wi.done = false;
    wi.free = false;
    wi.exclusive = false;

    queue_work_on_cpu(cpu, &wi);
    while (!qatomic_mb_read(&wi.done)) {
        CPUState *self_cpu = current_cpu;

        qemu_cond_wait(&qemu_work_cond, mutex);
        current_cpu = self_cpu;
    }
}
```

- [ ] 如果是多线程的，那么 current_cpu 在什么时候发生改变
  - 除非一个 thread 同时指向多个 cpu 的

```c
/*
#0  process_queued_cpu_work (cpu=0x555556afdf30) at ../cpus-common.c:320
#1  0x0000555555cb2e32 in rr_cpu_thread_fn (arg=arg@entry=0x555556afdf30) at ../accel/tcg/tcg-accel-ops-rr.c:169
#2  0x0000555555e76603 in qemu_thread_start (args=<optimized out>) at ../util/qemu-thread-posix.c:541
#3  0x00007ffff628c609 in start_thread (arg=<optimized out>) at pthread_create.c:477
#4  0x00007ffff61b3293 in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:95
```

- [ ] 如何去处理启动的 thread 的这种 do_run_on_cpu 的操作啊!
```c
/*
#0  do_run_on_cpu (cpu=cpu@entry=0x555556b09690, func=func@entry=0x555555bb06c0 <vapic_do_enable_tpr_reporting>, data=..., mutex=mutex@entry=0x555556611600 <qemu_global
_mutex>) at ../cpus-common.c:136
#1  0x0000555555c4eda0 in run_on_cpu (cpu=cpu@entry=0x555556b09690, func=func@entry=0x555555bb06c0 <vapic_do_enable_tpr_reporting>, data=..., data@entry=...) at ../soft
mmu/cpus.c:387
#2  0x0000555555bb064b in vapic_enable_tpr_reporting (enable=<optimized out>) at ../hw/i386/kvmvapic.c:511
#3  0x0000555555d3e879 in resettable_phase_hold (obj=0x555556a11200, opaque=<optimized out>, type=<optimized out>) at ../hw/core/resettable.c:182
#4  0x0000555555d3a194 in bus_reset_child_foreach (obj=<optimized out>, cb=0x555555d3e790 <resettable_phase_hold>, opaque=0x0, type=RESET_TYPE_COLD) at ../hw/core/bus.c
:97
#5  0x0000555555d3e834 in resettable_child_foreach (rc=0x55555687e4f0, type=RESET_TYPE_COLD, opaque=0x0, cb=0x555555d3e790 <resettable_phase_hold>, obj=0x55555692a260)
at ../hw/core/resettable.c:96
#6  resettable_phase_hold (obj=obj@entry=0x55555692a260, opaque=opaque@entry=0x0, type=type@entry=RESET_TYPE_COLD) at ../hw/core/resettable.c:173
#7  0x0000555555d3f019 in resettable_assert_reset (obj=0x55555692a260, type=<optimized out>) at ../hw/core/resettable.c:60
#8  0x0000555555d3f3ad in resettable_reset (obj=0x55555692a260, type=RESET_TYPE_COLD) at ../hw/core/resettable.c:45
#9  0x0000555555d3e375 in qemu_devices_reset () at ../hw/core/reset.c:69
#10 0x0000555555b9b82f in pc_machine_reset (machine=<optimized out>) at ../hw/i386/pc.c:1653
#11 0x0000555555c1e990 in qemu_system_reset (reason=reason@entry=SHUTDOWN_CAUSE_NONE) at ../softmmu/runstate.c:443
#12 0x0000555555a6c84a in qdev_machine_creation_done () at ../hw/core/machine.c:1333
#13 0x0000555555c64ed0 in qemu_machine_creation_done () at ../softmmu/vl.c:2668
#14 qmp_x_exit_preconfig (errp=<optimized out>) at ../softmmu/vl.c:2691
#15 qmp_x_exit_preconfig (errp=<optimized out>) at ../softmmu/vl.c:2682
#16 0x0000555555c68598 in qemu_init (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/vl.c:3706
#17 0x0000555555940c8d in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49
```
- [ ] 这种启动放到本身很奇怪。


## rr_wait_io_event

- 区分 rr_wait_io_event 和 qemu_wait_io_event 的差异
  - 因为 qemu_wait_io_event 是一个线程的，所以不需要进行对于所有 CPU 的等待的。

```c
static void rr_wait_io_event(void)
{
    CPUState *cpu;

    // 完全无法理解这个逻辑啊?
    // 当没有事情可以做的时候，然后等待到 halt_cond 上
    while (all_cpu_threads_idle()) {
        rr_stop_kick_timer();
        // 这个是在 qemu_cpu_kick 的位置上来让其苏醒过来
        //
        qemu_cond_wait_iothread(first_cpu->halt_cond);
    }

    rr_start_kick_timer();

    CPU_FOREACH(cpu) {
        qemu_wait_io_event_common(cpu);
    }
}
```
将信将疑的分析了一下，实际上，rr_wait_io_event 中等待 cpu_thread_is_idle 主要是因为
stopped 的原因。也就是当 thread 被 stop 之后，就没有必要继续执行下去了。

## qemu_mutex_iothread_locked
- [ ] process_queued_cpu_work : 因为 start_exclusive 的原因，但是需要更加统一的分析，而且 mttcg rr 的 thread fn 都是进一步添加 lock 的

- cputlb.c 中 io_readx 和 io_writex 中会检测，当没有 locked 时候，然后一定上锁
  - [x] io_readx 和 io_writex 只是被 load_helper 和 store_helper 调用的，但是，实际上，应该总是没有持有 lock 的才对啊，我检测过，没有人调用过，可能是 arm 架构相关的，和 NiuGene 核实一下

- 在 memory_region_transaction_commit 中，需要一定保证 qemu_mutex_iothread_locked 的
  - reasonable : 要么是在 main loop 初始化过程中，要么是因为在 mmio hook 中
- pause_all_vcpus : pause_all_vcpus <= vapic_write, 也就是调用 MemoryRegionOps 的时候，就是上锁的
- 在 softmmu/cpu-throttle.c 中，cpu_throttle_thread 作为 async_run_on_cpu 的 hook 函数，当其需要睡眠的时候，需要释放 lock
  - 也就是说，在 async_run_on_cpu 执行的时候，实际上持有 BQL 的
  - 是的，从 mttcg_cpu_thread_fn 中看，tcg_cpus_exec 的两侧都是被 BQL 包围的

- [ ] 其实被忽视的一个问题是 : cpu_exec_step_atomic, 在 rr 和 mttcg 中都是需要特殊处理的
