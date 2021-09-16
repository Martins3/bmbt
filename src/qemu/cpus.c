#include "../../include/exec/cpu-all.h"
#include "../../include/qemu/main-loop.h"
#include "../../include/qemu/rcu.h"
#include "../i386/LATX/x86tomips-config.h"
#include "../tcg/tcg.h"

// [interface 11]
static bool iothread_locked = false;

/* cpu creation */
static QemuCond qemu_cpu_cond;
/* system init */
static QemuCond qemu_pause_cond;

bool qemu_mutex_iothread_locked(void) { return iothread_locked; }

static void start_tcg_kick_timer(void) {
  // [interface 13]
}

static bool cpu_can_run(CPUState *cpu) {
  // [interface 14]
  return true;
}

static void deal_with_unplugged_cpus(void) {
  // vCPU is unpluggable
}

static void prepare_icount_for_run(CPUState *cpu) {
  // [interface 16]
}

static void process_icount_data(CPUState *cpu) {
  // [interface 16]
}

static void cpu_handle_guest_debug(CPUState *cpu) {
  // support gdb is a rocket science
}

/*
 * The BQL is taken from so many places that it is worth profiling the
 * callers directly, instead of funneling them all through a single function.
 */
void qemu_mutex_lock_iothread_impl(const char *file, int line) {
  printf("locked %s:%d\n", file, line);
  g_assert(!qemu_mutex_iothread_locked());
  iothread_locked = true;
}

void qemu_mutex_unlock_iothread(void) {
  g_assert(qemu_mutex_iothread_locked());
  iothread_locked = false;
}

static void qemu_wait_io_event_common(CPUState *cpu) {
#ifdef BMBT
  atomic_mb_set(&cpu->thread_kicked, false);
  if (cpu->stop) {
    qemu_cpu_stop(cpu, false);
  }
#endif
  process_queued_cpu_work(cpu);
}

static void qemu_tcg_rr_wait_io_event(void) {
  CPUState *cpu;

#if BMBT
  while (all_cpu_threads_idle()) {
    stop_tcg_kick_timer();
    qemu_cond_wait(first_cpu->halt_cond, &qemu_global_mutex);
  }

  start_tcg_kick_timer();
#endif

  CPU_FOREACH(cpu) { qemu_wait_io_event_common(cpu); }
}

static int tcg_cpu_exec(CPUState *cpu) {
  int ret;
#ifdef CONFIG_PROFILER
  int64_t ti;
#endif

  assert(tcg_enabled());
#ifdef CONFIG_PROFILER
  ti = profile_getclock();
#endif
  cpu_exec_start(cpu);
  ret = cpu_exec(cpu);
  cpu_exec_end(cpu);
#ifdef CONFIG_PROFILER
  atomic_set(&tcg_ctx->prof.cpu_exec_time,
             tcg_ctx->prof.cpu_exec_time + profile_getclock() - ti);
#endif
  return ret;
}

/* Single-threaded TCG
 *
 * In the single-threaded case each vCPU is simulated in turn. If
 * there is more than a single vCPU we create a simple timer to kick
 * the vCPU and ensure we don't get stuck in a tight loop in one vCPU.
 * This is done explicitly rather than relying on side-effects
 * elsewhere.
 */

void *qemu_tcg_rr_cpu_thread_fn(void *arg) {
  CPUState *cpu = arg;

  assert(tcg_enabled());
  rcu_register_thread();
  tcg_register_thread();

#ifdef CONFIG_BTMMU
  if (btmmu_enabled() && sysconf(_SC_NPROCESSORS_ONLN) >= 4) {
    cpu_set_t cpuset;

    /* only runs on given core*/
    CPU_ZERO(&cpuset);
    CPU_SET(btmmu_cpu, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
  }
#endif

  qemu_mutex_lock_iothread();
  qemu_thread_get_self(cpu->thread);

  cpu->thread_id = qemu_get_thread_id();
  cpu->created = true;
  cpu->can_do_io = 1;
  qemu_cond_signal(&qemu_cpu_cond);
  // qemu_guest_random_seed_thread_part2(cpu->random_seed);

#ifdef CONFIG_X86toMIPS
  x86_to_mips_init_env((CPUArchState *)cpu->env_ptr);
  x86_to_mips_init_thread_signal(cpu);
#ifdef CONFIG_BTMMU
  build_func_xqm();
  build_data_xqm();
#endif
#endif

  /* wait for initial kick-off after machine start */
#ifdef BMBT
  while (first_cpu->stopped) {
    qemu_cond_wait(first_cpu->halt_cond, &qemu_global_mutex);

    /* process any pending work */
    CPU_FOREACH(cpu) {
      current_cpu = cpu;
      qemu_wait_io_event_common(cpu);
    }
  }
#endif

  start_tcg_kick_timer();

  cpu = first_cpu;

  /* process any pending work */
  cpu->exit_request = 1;

  while (1) {
#ifdef BMBT
    qemu_mutex_unlock_iothread();
    replay_mutex_lock();
    qemu_mutex_lock_iothread();
    /* Account partial waits to QEMU_CLOCK_VIRTUAL.  */
    qemu_account_warp_timer();

    /* Run the timers here.  This is much more efficient than
     * waking up the I/O thread and waiting for completion.
     */
    handle_icount_deadline();

    replay_mutex_unlock();
#endif

    if (!cpu) {
      cpu = first_cpu;
      g_assert_not_reached();
    }

    while (cpu && !cpu->queued_work_first && !cpu->exit_request) {

#ifdef BMBT
      // [interface 13]
      atomic_mb_set(&tcg_current_rr_cpu, cpu);
      // [interface 16]
      current_cpu = cpu;
#endif

#ifdef BMBT
      qemu_clock_enable(QEMU_CLOCK_VIRTUAL,
                        (cpu->singlestep_enabled & SSTEP_NOTIMER) == 0);
#endif

      if (cpu_can_run(cpu)) {
        int r;

        qemu_mutex_unlock_iothread();
        prepare_icount_for_run(cpu);

        r = tcg_cpu_exec(cpu);

        process_icount_data(cpu);
        qemu_mutex_lock_iothread();

        if (r == EXCP_DEBUG) {
          cpu_handle_guest_debug(cpu);
          break;
        } else if (r == EXCP_ATOMIC) {
          qemu_mutex_unlock_iothread();
          cpu_exec_step_atomic(cpu);
          qemu_mutex_lock_iothread();
          break;
        }
      }
#if BMBT
      else if (cpu->stop) {
        if (cpu->unplug) {
          cpu = CPU_NEXT(cpu);
        }
        break;
      }
#endif

      cpu = CPU_NEXT(cpu);
    } /* while (cpu && !cpu->exit_request).. */

#ifdef BMBT
    // [interface 13]
    /* Does not need atomic_mb_set because a spurious wakeup is okay.  */
    atomic_set(&tcg_current_rr_cpu, NULL);
#endif

    if (cpu && cpu->exit_request) {
      atomic_mb_set(&cpu->exit_request, 0);
    }

#ifdef BMBT
    if (use_icount && all_cpu_threads_idle()) {
      /*
       * When all cpus are sleeping (e.g in WFI), to avoid a deadlock
       * in the main_loop, wake it up in order to start the warp timer.
       */
      qemu_notify_event();
    }
#endif

    qemu_tcg_rr_wait_io_event();
    deal_with_unplugged_cpus();
  }

  rcu_unregister_thread();
  return NULL;
}
