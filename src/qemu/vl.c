#include "../../include/hw/i386/pc.h"
#include "../../include/qemu/error-report.h"
#include "../../include/qemu/log.h"
#include "../../include/qemu/main-loop.h"
#include "../../include/qemu/notify.h"
#include "../../include/qemu/units.h"
#include "../../include/qemu/uuid.h"
#include "../../include/sysemu/cpus.h"
#include "../../include/sysemu/replay.h"
#include "../../include/sysemu/reset.h"
#include "../../include/sysemu/tcg.h"

#include <stdbool.h>

/* The bytes in qemu_uuid are in the order specified by RFC4122, _not_ in the
 * little-endian "wire format" described in the SMBIOS 2.6 specification.
 */
QemuUUID qemu_uuid;
ram_addr_t ram_size;
ReplayMode replay_mode = REPLAY_MODE_NONE;

bool machine_init_done;

static NotifierList machine_init_done_notifiers =
    NOTIFIER_LIST_INITIALIZER(machine_init_done_notifiers);

void qemu_add_machine_init_done_notifier(Notifier *notify) {
  notifier_list_add(&machine_init_done_notifiers, notify);
  if (machine_init_done) {
    notify->notify(notify, NULL);
  }
}

static void qemu_run_machine_init_done_notifiers(void) {
  machine_init_done = true;
  notifier_list_notify(&machine_init_done_notifiers, NULL);
}

void fw_cfg_reset();
void pic_reset();
void apic_reset_common();
void iopaic_reset();

void qbus_reset_all_fn(void *opaque) {
  fw_cfg_reset();
  pic_reset();
  apic_reset_common();
  iopaic_reset();
}

/*
 * Reset the VM. Issue an event unless @reason is SHUTDOWN_CAUSE_NONE.
 */
void qemu_system_reset(MachineClass *mc, MachineState *current_machine) {
#ifdef BMBT
  MachineClass *mc;

  mc = current_machine ? MACHINE_GET_CLASS(current_machine) : NULL;

  cpu_synchronize_all_states();
#endif

  if (mc && mc->reset) {
    mc->reset(current_machine);
  } else {
    g_assert_not_reached();
    // qemu_devices_reset();
  }
#ifdef BMBT
  if (reason && reason != SHUTDOWN_CAUSE_SUBSYSTEM_RESET) {
    qapi_event_send_reset(shutdown_caused_by_guest(reason), reason);
  }
  cpu_synchronize_all_post_reset();
#endif
}

static PCMachineState __pcms;
static PCMachineClass __pcmc;

static bool machine_inited = false;

MachineState *qdev_get_machine() {
  duck_check(machine_inited);
  return MACHINE(&__pcms);
}

void QOM_machine_init() {
  PCMachineState *pcms = &__pcms;
  PCMachineClass *pcmc = &__pcmc;

  MachineState *ms = MACHINE(pcms);
  MachineClass *mc = MACHINE_CLASS(pcmc);

  X86MachineState *x86ms = X86_MACHINE(&__pcms);
  X86MachineClass *x86mc = X86_MACHINE_CLASS(mc);

  pcms->pcmc = pcmc;
  ms->mc = mc;
  x86ms->x86mc = x86mc;

  machine_class_init(mc);
  machine_class_base_init(mc);
  machine_initfn(ms);

  x86_machine_class_init(x86mc);
  x86_machine_initfn(x86ms);

  pc_machine_class_init(pcmc);
  pc_machine_initfn(pcms);

  pc_machine_v4_2_class_init(mc);

  machine_inited = true;
}

PCMachineState *machine_init() {
  MachineState *current_machine = MACHINE(&__pcms);
  MachineClass *machine_class = MACHINE_GET_CLASS(current_machine);

  QOM_machine_init();

  /* machine_class: default to UP */
  machine_class->max_cpus = machine_class->max_cpus ?: 1;
  machine_class->min_cpus = machine_class->min_cpus ?: 1;
  machine_class->default_cpus = machine_class->default_cpus ?: 1;

  /* default to machine_class->default_cpus */
  current_machine->smp.cpus = machine_class->default_cpus;
  current_machine->smp.max_cpus = machine_class->default_cpus;
  current_machine->smp.cores = 1;
  current_machine->smp.threads = 1;

  machine_class->smp_parse(current_machine);

  /* sanity-check smp_cpus and max_cpus against machine_class */
  if (current_machine->smp.cpus < machine_class->min_cpus) {
    error_report("Invalid SMP CPUs %d. The min CPUs "
                 "supported by machine '%s' is %d",
                 current_machine->smp.cpus, machine_class->name,
                 machine_class->min_cpus);
    exit(1);
  }
  if (current_machine->smp.max_cpus > machine_class->max_cpus) {
    error_report("Invalid SMP CPUs %d. The max CPUs "
                 "supported by machine '%s' is %d",
                 current_machine->smp.max_cpus, machine_class->name,
                 machine_class->max_cpus);
    exit(1);
  }

// [interface 24]
// [firmware == bios-256k.bin]
// [kernel == /home/maritns3/core/ubuntu-linux/arch/x86/boot/bzImage]
// [append == root=/dev/sda3 nokaslr ]
// [firmware == /home/maritns3/core/seabios/out/bios.bin]
// [type == pc]
// [accel == kvm]
// [kernel-irqchip == on]
#ifdef BMBT
  machine_opts = qemu_get_machine_opts();
  qemu_opt_foreach(machine_opts, machine_set_property, current_machine,
                   &error_fatal);
#endif
  current_machine->ram_size = ram_size;
  current_machine->maxram_size = ram_size;
  current_machine->ram_slots = 0;

  current_machine->boot_order = "cad";

  // parse features once if machine provides default cpu_type
  current_machine->cpu_type = machine_class->default_cpu_type;
  // parse_numa_opts(current_machine);

  machine_run_board_init(current_machine);

  // qdev_machine_creation_done();
  qemu_register_reset(qbus_reset_all_fn, NULL);
  qemu_run_machine_init_done_notifiers();

  qemu_system_reset(machine_class, current_machine);

  return &__pcms;
}

void qemu_init() {
  ram_size = 128 * MiB;
  duck_check(first_cpu == NULL);

  qemu_init_cpu_loop();
  qemu_init_cpu_list();

  memory_map_init(ram_size);
  qemu_mutex_lock_iothread();
  tcg_init();

  init_real_host_page_size();
  init_cache_info();

  qemu_set_log(0);

  machine_init();

  resume_all_vcpus();

  qemu_mutex_unlock_iothread();
  duck_check(first_cpu != NULL);
  qemu_tcg_rr_cpu_thread_fn(first_cpu);
}
