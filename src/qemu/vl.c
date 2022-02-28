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
#include <capstone/capstone.h>
#include <hw/i386/ioapic.h>
#include <hw/timer/i8254.h>

#include <stdbool.h>

/* The bytes in qemu_uuid are in the order specified by RFC4122, _not_ in the
 * little-endian "wire format" described in the SMBIOS 2.6 specification.
 */
QemuUUID qemu_uuid;
ReplayMode replay_mode = REPLAY_MODE_NONE;
int singlestep = 0;
int boot_menu = 0;
int fd_bootchk = 1;
int acpi_enabled = 0;
QEMUClockType rtc_clock;
static enum {
  RTC_BASE_UTC,
  RTC_BASE_LOCALTIME,
  RTC_BASE_DATETIME,
} rtc_base_type = RTC_BASE_UTC;
static time_t rtc_ref_start_datetime;
static int rtc_realtime_clock_offset; /* used only with QEMU_CLOCK_REALTIME */
static int rtc_host_datetime_offset = -1; /* valid & used only with
                                             RTC_BASE_DATETIME */
QEMUOptionRom option_rom[MAX_OPTION_ROMS];
int nb_option_roms;
bool boot_strict;

bool machine_init_done;
static NotifierList machine_init_done_notifiers =
    NOTIFIER_LIST_INITIALIZER(machine_init_done_notifiers);

QEMU_BUILD_BUG_ON(offsetof(X86CPUClass, parent_class) != 0);
QEMU_BUILD_BUG_ON(offsetof(X86CPU, parent_obj) != 0);
QEMU_BUILD_BUG_ON(offsetof(PCMachineState, parent_obj) != 0);
QEMU_BUILD_BUG_ON(offsetof(X86MachineState, parent_obj) != 0);
QEMU_BUILD_BUG_ON(offsetof(PCMachineClass, parent_class) != 0);
QEMU_BUILD_BUG_ON(offsetof(X86MachineClass, parent_class) != 0);
QEMU_BUILD_BUG_ON(offsetof(IOAPICCommonState, busdev) != 0);
QEMU_BUILD_BUG_ON(offsetof(PITCommonState, dev) != 0);

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
void rtc_resetdev();
void port92_reset();
void pcibus_reset();
void pit_reset();

// DeviceClass::reset
void qbus_reset_all_fn(void *opaque) {
  fw_cfg_reset();
  pic_reset();
  apic_reset_common();
  iopaic_reset();
  rtc_resetdev();
  port92_reset();
  pcibus_reset();
  pit_reset();
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

static void configure_rtc() {
  /* Set defaults */
  rtc_clock = QEMU_CLOCK_HOST;
}

/* RTC reference time/date access */
static time_t qemu_ref_timedate(QEMUClockType clock) {
  time_t value = qemu_clock_get_ms(clock) / 1000;
  switch (clock) {
  case QEMU_CLOCK_REALTIME:
    value -= rtc_realtime_clock_offset;
    /* fall through */
  case QEMU_CLOCK_VIRTUAL:
    value += rtc_ref_start_datetime;
    break;
  case QEMU_CLOCK_HOST:
    if (rtc_base_type == RTC_BASE_DATETIME) {
      value -= rtc_host_datetime_offset;
    }
    break;
  default:
    assert(0);
  }
  return value;
}

void qemu_get_timedate(struct tm *tm, int offset) {
  time_t ti = qemu_ref_timedate(rtc_clock);

  ti += offset;

  switch (rtc_base_type) {
  case RTC_BASE_DATETIME:
  case RTC_BASE_UTC:
    gmtime_r(&ti, tm);
    break;
  case RTC_BASE_LOCALTIME:
    g_assert_not_reached();
    // localtime_r(&ti, tm);
    break;
  }
}

static int num_serial_hds;
static Chardev **serial_hds;
Chardev *serial_hd(int i) {
  assert(i >= 0);
  if (i < num_serial_hds) {
    return serial_hds[i];
  }
  return NULL;
}

static void init_serial_chardev() {
  serial_hds = g_new0(Chardev *, 1);
  Chardev *serial_stub = g_new0(Chardev, 1);
  serial_stub->log = get_logfile("serial.log");
  serial_hds[0] = serial_stub;
  num_serial_hds++;
}

static PCMachineState __pcms;
static PCMachineClass __pcmc;

static bool machine_inited = false;

MachineState *qdev_get_machine() {
  bmbt_check(machine_inited);
  return MACHINE(&__pcms);
}

PCMachineState *QOM_machine_init() {
  PCMachineState *pcms = &__pcms;
  PCMachineClass *pcmc = &__pcmc;

  MachineState *ms = MACHINE(pcms);
  MachineClass *mc = MACHINE_CLASS(pcmc);

  X86MachineState *x86ms = X86_MACHINE(&__pcms);
  X86MachineClass *x86mc = X86_MACHINE_CLASS(mc);

  PC_MACHINE_SET_CLASS(pcms, pcmc);
  MACHINE_SET_CLASS(ms, mc);
  X86_MACHINE_SET_CLASS(x86ms, x86mc);

  machine_class_init(mc);
  machine_class_base_init(mc);
  x86_machine_class_init(x86mc);
  pc_machine_class_init(pcmc);
  pc_machine_v4_2_class_init(mc);

  machine_initfn(ms);
  x86_machine_initfn(x86ms);
  pc_machine_initfn(pcms);

  machine_inited = true;
  return pcms;
}

PCMachineState *machine_init() {
  MachineState *current_machine = MACHINE(QOM_machine_init());
  MachineClass *machine_class = MACHINE_GET_CLASS(current_machine);

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
  current_machine->ram_size = CONFIG_GUEST_RAM_SIZE;
  current_machine->maxram_size = CONFIG_GUEST_RAM_SIZE;
  current_machine->ram_slots = 0;

  current_machine->boot_order = "cad";

  // parse features once if machine provides default cpu_type
  current_machine->cpu_type = machine_class->default_cpu_type;
  // parse_numa_opts(current_machine);

  return &__pcms;
}

// call it before latx_init
static void capstone_init(void) {
  cs_opt_mem opt;
  opt.malloc = malloc;
  opt.calloc = calloc;
  opt.realloc = realloc;
  opt.free = free;
  opt.vsnprintf = vsnprintf;
  cs_option(CS_ARCH_X86, CS_OPT_MEM, (size_t)&opt);
}

void init_real_host_page_size(void);
void init_cache_info(void);
void softfloat_init(void);

void latx_init(void);
void latxs_tr_fldst_init(void);
void sys_devel_init(void);
void latxs_cs_lock_init(void);
void x86tomisp_ir1_init(void);
void tcg_sigint_init(void);

void call_constructor() {
  init_real_host_page_size();
  init_cache_info();
  softfloat_init();

  capstone_init();
  latx_init();
  latxs_tr_fldst_init();
  sys_devel_init();
  latxs_cs_lock_init();
  x86tomisp_ir1_init();
  tcg_sigint_init();
}

void qemu_init() {
  MachineState *current_machine;
  MachineClass *machine_class;
  call_constructor();

  init_xtm_options();

  configure_rtc();

  bmbt_check(first_cpu == NULL);

  qemu_init_cpu_loop();
  qemu_mutex_lock_iothread();

  cpu_ticks_init();

  init_serial_chardev();

  setup_timer_interrupt();

  qemu_init_cpu_list();

  memory_map_init();

  qemu_set_log(0);

  machine_init();
  tcg_init();

  current_machine = MACHINE(&__pcms);
  machine_run_board_init(current_machine);

  QOM_init_debugcon();

  // qdev_machine_creation_done();
  qemu_register_reset(qbus_reset_all_fn, NULL);
  qemu_run_machine_init_done_notifiers();

  machine_class = MACHINE_GET_CLASS(current_machine);
  qemu_system_reset(machine_class, current_machine);

  vm_start();

  qemu_mutex_unlock_iothread();
}
