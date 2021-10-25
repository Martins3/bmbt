#include "../../include/hw/core/cpu.h"
#include "../../include/qemu/main-loop.h"
#include "../../include/qemu/rcu.h"

struct qemu_work_item {
  struct qemu_work_item *next;
  run_on_cpu_func func;
  run_on_cpu_data data;
  bool free, exclusive, done;
};

// [interface 33]
static QemuCond qemu_work_cond;
static QemuMutex qemu_cpu_list_lock;

CPUTailQ cpus = QTAILQ_HEAD_INITIALIZER(cpus);

void qemu_init_cpu_list(void) {
#ifdef BMBT
  /* This is needed because qemu_init_cpu_list is also called by the
   * child process in a fork.  */
  pending_cpus = 0;

  qemu_cond_init(&exclusive_cond);
  qemu_cond_init(&exclusive_resume);
#endif
  qemu_mutex_init(&qemu_cpu_list_lock);
  qemu_cond_init(&qemu_work_cond);
}

void process_queued_cpu_work(CPUState *cpu) {
  struct qemu_work_item *wi;

  if (cpu->queued_work_first == NULL) {
    return;
  }

  qemu_mutex_lock(&cpu->work_mutex);
  while (cpu->queued_work_first != NULL) {
    wi = cpu->queued_work_first;
    cpu->queued_work_first = wi->next;
    if (!cpu->queued_work_first) {
      cpu->queued_work_last = NULL;
    }
    qemu_mutex_unlock(&cpu->work_mutex);
    if (wi->exclusive) {
      /* Running work items outside the BQL avoids the following deadlock:
       * 1) start_exclusive() is called with the BQL taken while another
       * CPU is running; 2) cpu_exec in the other CPU tries to takes the
       * BQL, so it goes to sleep; start_exclusive() is sleeping too, so
       * neither CPU can proceed.
       */
      qemu_mutex_unlock_iothread();
      start_exclusive();
      wi->func(cpu, wi->data);
      end_exclusive();
      qemu_mutex_lock_iothread();
    } else {
      wi->func(cpu, wi->data);
    }
    qemu_mutex_lock(&cpu->work_mutex);
    if (wi->free) {
      g_free(wi);
    } else {
      atomic_mb_set(&wi->done, true);
    }
  }
  qemu_mutex_unlock(&cpu->work_mutex);
  qemu_cond_broadcast(&qemu_work_cond);
}

static void queue_work_on_cpu(CPUState *cpu, struct qemu_work_item *wi) {
  qemu_mutex_lock(&cpu->work_mutex);
  if (cpu->queued_work_first == NULL) {
    cpu->queued_work_first = wi;
  } else {
    cpu->queued_work_last->next = wi;
  }
  cpu->queued_work_last = wi;
  wi->next = NULL;
  wi->done = false;
  qemu_mutex_unlock(&cpu->work_mutex);

  qemu_cpu_kick(cpu);
}

void async_safe_run_on_cpu(CPUState *cpu, run_on_cpu_func func,
                           run_on_cpu_data data) {
  struct qemu_work_item *wi;

  wi = g_malloc0(sizeof(struct qemu_work_item));
  wi->func = func;
  wi->data = data;
  wi->free = true;
  wi->exclusive = true;

  queue_work_on_cpu(cpu, wi);
}

static bool cpu_index_auto_assigned;

static int cpu_get_free_index(void) {
  CPUState *some_cpu;
  int max_cpu_index = 0;

  cpu_index_auto_assigned = true;
  CPU_FOREACH(some_cpu) {
    if (some_cpu->cpu_index >= max_cpu_index) {
      max_cpu_index = some_cpu->cpu_index + 1;
    }
  }
  return max_cpu_index;
}

void cpu_list_add(CPUState *cpu) {
  QEMU_LOCK_GUARD(&qemu_cpu_list_lock);
  if (cpu->cpu_index == UNASSIGNED_CPU_INDEX) {
    cpu->cpu_index = cpu_get_free_index();
    assert(cpu->cpu_index != UNASSIGNED_CPU_INDEX);
  } else {
    assert(!cpu_index_auto_assigned);
  }
  QTAILQ_INSERT_TAIL_RCU(&cpus, cpu, node);
}

void cpu_list_remove(CPUState *cpu) {
  QEMU_LOCK_GUARD(&qemu_cpu_list_lock);
  if (!QTAILQ_IN_USE(cpu, node)) {
    /* there is nothing to undo since cpu_exec_init() hasn't been called */
    return;
  }

  QTAILQ_REMOVE_RCU(&cpus, cpu, node);
  cpu->cpu_index = UNASSIGNED_CPU_INDEX;
}

void async_run_on_cpu(CPUState *cpu, run_on_cpu_func func,
                      run_on_cpu_data data) {
  struct qemu_work_item *wi;

  wi = g_malloc0(sizeof(struct qemu_work_item));
  wi->func = func;
  wi->data = data;
  wi->free = true;

  queue_work_on_cpu(cpu, wi);
}
