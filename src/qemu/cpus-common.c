#include "../../include/qemu/main-loop.h"

struct qemu_work_item {
  struct qemu_work_item *next;
  run_on_cpu_func func;
  run_on_cpu_data data;
  bool free, exclusive, done;
};

static QemuCond qemu_work_cond;

CPUState *current_cpu = NULL;

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
