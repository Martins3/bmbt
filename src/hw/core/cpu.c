#include "../../../include/hw/core/cpu.h"
#include "../../../include/qemu/log.h"
#include "../../../include/qemu/main-loop.h"
#include "../../../include/sysemu/tcg.h"

CPUState *cpu_by_arch_id(int64_t id) {
  CPUState *cpu;

  CPU_FOREACH(cpu) {
    CPUClass *cc = CPU_GET_CLASS(cpu);

    if (cc->get_arch_id(cpu) == id) {
      return cpu;
    }
  }
  return NULL;
}

bool cpu_exists(int64_t id) { return !!cpu_by_arch_id(id); }

#ifdef BMBT
CPUState *cpu_create(const char *typename) {
  CPUState *cpu = NULL;
  Error *err = NULL;
  object_property_set_bool(OBJECT(cpu), true, "realized", &err);
  if (err != NULL) {
    error_report_err(err);
    object_unref(OBJECT(cpu));
    exit(EXIT_FAILURE);
  }
  return cpu;
}
#endif

bool cpu_paging_enabled(const CPUState *cpu) {
  CPUClass *cc = CPU_GET_CLASS(cpu);

  return cc->get_paging_enabled(cpu);
}

static bool cpu_common_get_paging_enabled(const CPUState *cpu) { return false; }

#ifdef BMBT
void cpu_get_memory_mapping(CPUState *cpu, MemoryMappingList *list,
                            Error **errp) {
  CPUClass *cc = CPU_GET_CLASS(cpu);

  cc->get_memory_mapping(cpu, list, errp);
}

static void cpu_common_get_memory_mapping(CPUState *cpu,
                                          MemoryMappingList *list,
                                          Error **errp) {
  error_setg(errp, "Obtaining memory mappings is unsupported on this CPU.");
}
#endif

/* Resetting the IRQ comes from across the code base so we take the
 * BQL here if we need to.  cpu_interrupt assumes it is held.*/
void cpu_reset_interrupt(CPUState *cpu, int mask) {
  bool need_lock = !qemu_mutex_iothread_locked();

  if (need_lock) {
    qemu_mutex_lock_iothread();
  }
  cpu->interrupt_request &= ~mask;
  if (need_lock) {
    qemu_mutex_unlock_iothread();
  }
}

#ifdef CONFIG_X86toMIPS
extern int xtm_sigint_opt(void);
#define XTM_SIGINT_SIGNAL 63
#endif

void cpu_exit(CPUState *cpu) {
  atomic_set(&cpu->exit_request, 1);
  /* Ensure cpu_exec will see the exit request after TCG has exited.  */
  smp_wmb();
#ifdef CONFIG_X86toMIPS
  if (xtm_sigint_opt()) {
    g_assert_not_reached();
    // pthread_kill(cpu->thread->thread, XTM_SIGINT_SIGNAL);
    return;
  }
#endif
  atomic_set(&cpu->icount_decr_ptr->u16.high, -1);
}

void cpu_dump_state(CPUState *cpu, FILE *f, int flags) {
  CPUClass *cc = CPU_GET_CLASS(cpu);

  if (cc->dump_state) {
    cpu_synchronize_state(cpu);
    cc->dump_state(cpu, f, flags);
  }
}

void cpu_reset(CPUState *cpu) {
  CPUClass *klass = CPU_GET_CLASS(cpu);

  if (klass->reset != NULL) {
    (*klass->reset)(cpu);
  } else {
    // reset must be registered
    duck_check(false);
  }

  // fuck_trace_guest_cpu_reset(cpu);
}

static void cpu_common_reset(CPUState *cpu) {
  CPUClass *cc = CPU_GET_CLASS(cpu);

  if (qemu_loglevel_mask(CPU_LOG_RESET)) {
    qemu_log("CPU Reset (CPU %d)\n", cpu->cpu_index);
    log_cpu_state(cpu, cc->reset_dump_flags);
  }

  cpu->interrupt_request = 0;
  cpu->halted = 0;
  cpu->mem_io_pc = 0;
  cpu->icount_extra = 0;
  atomic_set(&cpu->icount_decr_ptr->u32, 0);
  cpu->can_do_io = 1;
  cpu->exception_index = -1;
  // cpu->crash_occurred = false;
  cpu->cflags_next_tb = -1;

  if (tcg_enabled()) {
    cpu_tb_jmp_cache_clear(cpu);

    tcg_flush_softmmu_tlb(cpu);
  }
}

static bool cpu_common_has_work(CPUState *cs) { return false; }

#ifdef BMBT
static void cpu_common_realizefn(DeviceState *dev, Error **errp) {
  CPUState *cpu = CPU(dev);
  Object *machine = qdev_get_machine();

  /* qdev_get_machine() can return something that's not TYPE_MACHINE
   * if this is one of the user-only emulators; in that case there's
   * no need to check the ignore_memory_transaction_failures board flag.
   */
  if (object_dynamic_cast(machine, TYPE_MACHINE)) {
    ObjectClass *oc = object_get_class(machine);
    MachineClass *mc = MACHINE_CLASS(oc);

    if (mc) {
      cpu->ignore_memory_transaction_failures =
          mc->ignore_memory_transaction_failures;
    }
  }

  if (dev->hotplugged) {
    cpu_synchronize_post_init(cpu);
    cpu_resume(cpu);
  }

  /* NOTE: latest generic point where the cpu is fully realized */
  trace_init_vcpu(cpu);
}
#endif

void cpu_common_initfn(CPUState *cpu) {
  CPUClass *cc = CPU_GET_CLASS(cpu);

  cpu->cpu_index = UNASSIGNED_CPU_INDEX;
  cpu->cluster_index = UNASSIGNED_CLUSTER_INDEX;
  // cpu->gdb_num_regs = cpu->gdb_num_g_regs = cc->gdb_num_core_regs;
  /* *-user doesn't have configurable SMP topology */
  /* the default value is changed by qemu_init_vcpu() for softmmu */
  cpu->nr_cores = 1;
  cpu->nr_threads = 1;

  qemu_mutex_init(&cpu->work_mutex);
  QTAILQ_INIT(&cpu->breakpoints);
  QTAILQ_INIT(&cpu->watchpoints);

  cpu_exec_initfn(cpu);
}

static int64_t cpu_common_get_arch_id(CPUState *cpu) { return cpu->cpu_index; }

static vaddr cpu_adjust_watchpoint_address(CPUState *cpu, vaddr addr, int len) {
  return addr;
}

void cpu_class_init(CPUClass *k) {
  // k->parse_features = cpu_common_parse_features;
  k->reset = cpu_common_reset;
  k->get_arch_id = cpu_common_get_arch_id;
  k->has_work = cpu_common_has_work;
  k->get_paging_enabled = cpu_common_get_paging_enabled;
  // k->get_memory_mapping = cpu_common_get_memory_mapping;
  // k->write_elf32_qemunote = cpu_common_write_elf32_qemunote;
  // k->write_elf32_note = cpu_common_write_elf32_note;
  // k->write_elf64_qemunote = cpu_common_write_elf64_qemunote;
  // k->write_elf64_note = cpu_common_write_elf64_note;
  // k->gdb_read_register = cpu_common_gdb_read_register;
  // k->gdb_write_register = cpu_common_gdb_write_register;
  // k->virtio_is_big_endian = cpu_common_virtio_is_big_endian;
  // k->debug_excp_handler = cpu_common_noop;
  // k->debug_check_watchpoint = cpu_common_debug_check_watchpoint;
  // k->cpu_exec_enter = cpu_common_noop;
  // k->cpu_exec_exit = cpu_common_noop;
  // k->cpu_exec_interrupt = cpu_common_exec_interrupt;
  k->adjust_watchpoint_address = cpu_adjust_watchpoint_address;
  // set_bit(DEVICE_CATEGORY_CPU, dc->categories);
  // dc->realize = cpu_common_realizefn;
  // dc->unrealize = cpu_common_unrealizefn;
  // dc->props = cpu_common_props;
  /*
   * Reason: CPUs still need special care by board code: wiring up
   * IRQs, adding reset handlers, halting non-first CPUs, ...
   */
  // dc->user_creatable = false;
}

#ifdef BMBT
static const TypeInfo cpu_type_info = {
    .name = TYPE_CPU,
    .parent = TYPE_DEVICE,
    .instance_size = sizeof(CPUState),
    .instance_init = cpu_common_initfn,
    .instance_finalize = cpu_common_finalize,
    .abstract = true,
    .class_size = sizeof(CPUClass),
    .class_init = cpu_class_init,
};

static void cpu_register_types(void) { type_register_static(&cpu_type_info); }

type_init(cpu_register_types)
#endif
