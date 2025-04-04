#ifndef CPU_H_5RAXENPS
#define CPU_H_5RAXENPS

#include <exec/hwaddr.h>
#include <exec/memattrs.h>
#include <exec/memory.h>
#include <inttypes.h> // for VADDR_PRIx
#include <qemu/atomic.h>
#include <qemu/bitmap.h>
#include <qemu/osdep.h>
#include <qemu/queue.h>
#include <qemu/thread.h>
#include <setjmp.h>

/**
 * vaddr:
 * Type wide enough to contain any #target_ulong virtual address.
 */
typedef uint64_t vaddr;
#define VADDR_PRId PRId64
#define VADDR_PRIu PRIu64
#define VADDR_PRIo PRIo64
#define VADDR_PRIx PRIx64
#define VADDR_PRIX PRIX64
#define VADDR_MAX UINT64_MAX

typedef struct CPUBreakpoint {
  vaddr pc;
  int flags; /* BP_* */
  QTAILQ_ENTRY(CPUBreakpoint) entry;
} CPUBreakpoint;

typedef struct CPUWatchpoint {
  vaddr vaddr;
  vaddr len;
  vaddr hitaddr;
  MemTxAttrs hitattrs;
  int flags; /* BP_* */
  QTAILQ_ENTRY(CPUWatchpoint) entry;
} CPUWatchpoint;

typedef enum MMUAccessType {
  MMU_DATA_LOAD = 0,
  MMU_DATA_STORE = 1,
  MMU_INST_FETCH = 2
} MMUAccessType;

#define TB_JMP_CACHE_BITS 12
#define TB_JMP_CACHE_SIZE (1 << TB_JMP_CACHE_BITS)
#define CPU_TRACE_DSTATE_MAX_EVENTS 32

typedef struct CPUAddressSpace {
  AddressSpace *as;
} CPUAddressSpace;

/*
 * Low 16 bits: number of cycles left, used only in icount mode.
 * High 16 bits: Set to -1 to force TCG to stop executing linked TBs
 * for this CPU and return to its top level loop (even in non-icount mode).
 * This allows a single read-compare-cbranch-write sequence to test
 * for both decrementer underflow and exceptions.
 */
typedef union {
  uint32_t u32;
  struct {
#ifdef HOST_WORDS_BIGENDIAN
    uint16_t high;
    uint16_t low;
#else
    uint16_t low;
    uint16_t high;
#endif
  } u16;
} IcountDecr;

/**
 * CPUState:
 * @cpu_index: CPU index (informative).
 * @cluster_index: Identifies which cluster this CPU is in.
 *   For boards which don't define clusters or for "loose" CPUs not assigned
 *   to a cluster this will be UNASSIGNED_CLUSTER_INDEX; otherwise it will
 *   be the same as the cluster-id property of the CPU object's TYPE_CPU_CLUSTER
 *   QOM parent.
 * @tcg_cflags: Pre-computed cflags for this cpu.
 * @nr_cores: Number of cores within this CPU package.
 * @nr_threads: Number of threads within this CPU.
 * @running: #true if CPU is currently running (lockless).
 * @has_waiter: #true if a CPU is currently waiting for the cpu_exec_end;
 * valid under cpu_list_lock.
 * @created: Indicates whether the CPU thread has been successfully created.
 * @interrupt_request: Indicates a pending interrupt request.
 * @halted: Nonzero if the CPU is in suspended state.
 * @stop: Indicates a pending stop request.
 * @stopped: Indicates the CPU has been artificially stopped.
 * @unplug: Indicates a pending CPU unplug request.
 * @crash_occurred: Indicates the OS reported a crash (panic) for this CPU
 * @singlestep_enabled: Flags for single-stepping.
 * @icount_extra: Instructions until next timer event.
 * @can_do_io: Nonzero if memory-mapped IO is safe. Deterministic execution
 * requires that IO only be performed on the last instruction of a TB
 * so that interrupts take effect immediately.
 * @cpu_ases: Pointer to array of CPUAddressSpaces (which define the
 *            AddressSpaces this CPU has)
 * @num_ases: number of CPUAddressSpaces in @cpu_ases
 * @as: Pointer to the first AddressSpace, for the convenience of targets which
 *      only have a single AddressSpace
 * @env_ptr: Pointer to subclass-specific CPUArchState field.
 * @icount_decr_ptr: Pointer to IcountDecr field within subclass.
 * @gdb_regs: Additional GDB registers.
 * @gdb_num_regs: Number of total registers accessible to GDB.
 * @gdb_num_g_regs: Number of registers in GDB 'g' packets.
 * @next_cpu: Next CPU sharing TB cache.
 * @opaque: User data.
 * @mem_io_pc: Host Program Counter at which the memory was accessed.
 * @kvm_fd: vCPU file descriptor for KVM.
 * @work_mutex: Lock to prevent multiple access to @work_list.
 * @work_list: List of pending asynchronous work.
 * @trace_dstate_delayed: Delayed changes to trace_dstate (includes all changes
 *                        to @trace_dstate).
 * @trace_dstate: Dynamic tracing state of events for this vCPU (bitmask).
 * @plugin_mask: Plugin event bitmap. Modified only via async work.
 * @ignore_memory_transaction_failures: Cached copy of the MachineState
 *    flag of the same name: allows the board to suppress calling of the
 *    CPU do_transaction_failed hook function.
 * @kvm_dirty_gfns: Points to the KVM dirty ring for this CPU when KVM dirty
 *    ring is enabled.
 * @kvm_fetch_index: Keeps the index that we last fetched from the per-vCPU
 *    dirty ring structure.
 *
 * State of one CPU core or thread.
 */
typedef struct CPUState {
  struct CPUClass *cc;

  /*< private >*/
  // DeviceState parent_obj;
  /*< public >*/

  int nr_cores;
  int nr_threads;

  // struct QemuThread *thread;
#ifdef _WIN32
  HANDLE hThread;
#endif
#ifdef BMBT
  int thread_id;
#endif
  bool running, has_waiter;
  // struct QemuCond *halt_cond;
  bool thread_kicked;
  bool created;
  bool stop;
  bool stopped;
  bool unplug;
  bool crash_occurred;
  bool exit_request;
  bool in_exclusive_context;
  uint32_t cflags_next_tb;
  /* updates protected by BQL */
  uint32_t interrupt_request;
  int singlestep_enabled;
  int64_t icount_budget;
  int64_t icount_extra;
  // uint64_t random_seed;
  jmp_buf jmp_env;

  QemuMutex work_mutex;
  struct qemu_work_item *queued_work_first, *queued_work_last;

  CPUAddressSpace *cpu_ases;
  int num_ases;
#ifdef BMBT
  AddressSpace *as;
  MemoryRegion *memory;
#endif

  void *env_ptr; /* CPUArchState */
  IcountDecr *icount_decr_ptr;

  /* Accessed in parallel; all accesses must be atomic */
  struct TranslationBlock *tb_jmp_cache[TB_JMP_CACHE_SIZE];

  struct GDBRegisterState *gdb_regs;
  int gdb_num_regs;
  int gdb_num_g_regs;
  QTAILQ_ENTRY(CPUState) node;

  /* ice debug support */
  QTAILQ_HEAD(, CPUBreakpoint) breakpoints;

  QTAILQ_HEAD(, CPUWatchpoint) watchpoints;
  CPUWatchpoint *watchpoint_hit;

  void *opaque;

#ifdef BMBT
  /* In order to avoid passing too many arguments to the MMIO helpers,
   * we store some rarely used information in the CPU context.
   */
  uintptr_t mem_io_pc;
#endif

  int kvm_fd;
  struct KVMState *kvm_state;
  struct kvm_run *kvm_run;

  /* Used for events with 'vcpu' and *without* the 'disabled' properties */
  DECLARE_BITMAP(trace_dstate_delayed, CPU_TRACE_DSTATE_MAX_EVENTS);
  DECLARE_BITMAP(trace_dstate, CPU_TRACE_DSTATE_MAX_EVENTS);

#ifdef BMBT
  DECLARE_BITMAP(plugin_mask, QEMU_PLUGIN_EV_MAX);

  GArray *plugin_mem_cbs;
#endif

  /* TODO Move common fields from CPUArchState here. */
  int cpu_index;
  int cluster_index;
  uint32_t halted;
  uint32_t can_do_io;
  int32_t exception_index;

  /* shared by kvm, hax and hvf */
  bool vcpu_dirty;

  /* Used to keep track of an outstanding cpu throttle thread for migration
   * autoconverge
   */
  bool throttle_thread_scheduled;

  bool ignore_memory_transaction_failures;

  struct hax_vcpu_state *hax_vcpu;

  int hvf_fd;

#ifdef BMBT
  /* track IOMMUs whose translations we've cached in the TCG TLB */
  GArray *iommu_notifiers;
#endif
} CPUState;

/**
 * CPUClass:
 * @class_by_name: Callback to map -cpu command line model name to an
 * instantiatable CPU type.
 * @parse_features: Callback to parse command line arguments.
 * @reset: Callback to reset the #CPUState to its initial state.
 * @reset_dump_flags: #CPUDumpFlags to use for reset logging.
 * @has_work: Callback for checking if there is work to do.
 * @do_interrupt: Callback for interrupt handling.
 * @do_unaligned_access: Callback for unaligned access handling, if
 * the target defines #TARGET_ALIGNED_ONLY.
 * @do_transaction_failed: Callback for handling failed memory transactions
 * (ie bus faults or external aborts; not MMU faults)
 * @virtio_is_big_endian: Callback to return %true if a CPU which supports
 * runtime configurable endianness is currently big-endian. Non-configurable
 * CPUs can use the default implementation of this method. This method should
 * not be used by any callers other than the pre-1.0 virtio devices.
 * @memory_rw_debug: Callback for GDB memory access.
 * @dump_state: Callback for dumping state.
 * @dump_statistics: Callback for dumping statistics.
 * @get_arch_id: Callback for getting architecture-dependent CPU ID.
 * @get_paging_enabled: Callback for inquiring whether paging is enabled.
 * @get_memory_mapping: Callback for obtaining the memory mappings.
 * @set_pc: Callback for setting the Program Counter register. This
 *       should have the semantics used by the target architecture when
 *       setting the PC from a source such as an ELF file entry point;
 *       for example on Arm it will also set the Thumb mode bit based
 *       on the least significant bit of the new PC value.
 *       If the target behaviour here is anything other than "set
 *       the PC register to the value passed in" then the target must
 *       also implement the synchronize_from_tb hook.
 * @synchronize_from_tb: Callback for synchronizing state from a TCG
 *       #TranslationBlock. This is called when we abandon execution
 *       of a TB before starting it, and must set all parts of the CPU
 *       state which the previous TB in the chain may not have updated.
 *       This always includes at least the program counter; some targets
 *       will need to do more. If this hook is not implemented then the
 *       default is to call @set_pc(tb->pc).
 * @tlb_fill: Callback for handling a softmmu tlb miss or user-only
 *       address fault.  For system mode, if the access is valid, call
 *       tlb_set_page and return true; if the access is invalid, and
 *       probe is true, return false; otherwise raise an exception and
 *       do not return.  For user-only mode, always raise an exception
 *       and do not return.
 * @get_phys_page_debug: Callback for obtaining a physical address.
 * @get_phys_page_attrs_debug: Callback for obtaining a physical address and the
 *       associated memory transaction attributes to use for the access.
 *       CPUs which use memory transaction attributes should implement this
 *       instead of get_phys_page_debug.
 * @asidx_from_attrs: Callback to return the CPU AddressSpace to use for
 *       a memory access with the specified memory transaction attributes.
 * @gdb_read_register: Callback for letting GDB read a register.
 * @gdb_write_register: Callback for letting GDB write a register.
 * @debug_check_watchpoint: Callback: return true if the architectural
 *       watchpoint whose address has matched should really fire.
 * @debug_excp_handler: Callback for handling debug exceptions.
 * @write_elf64_note: Callback for writing a CPU-specific ELF note to a
 * 64-bit VM coredump.
 * @write_elf32_qemunote: Callback for writing a CPU- and QEMU-specific ELF
 * note to a 32-bit VM coredump.
 * @write_elf32_note: Callback for writing a CPU-specific ELF note to a
 * 32-bit VM coredump.
 * @write_elf32_qemunote: Callback for writing a CPU- and QEMU-specific ELF
 * note to a 32-bit VM coredump.
 * @vmsd: State description for migration.
 * @gdb_num_core_regs: Number of core registers accessible to GDB.
 * @gdb_core_xml_file: File name for core registers GDB XML description.
 * @gdb_stop_before_watchpoint: Indicates whether GDB expects the CPU to stop
 *           before the insn which triggers a watchpoint rather than after it.
 * @gdb_arch_name: Optional callback that returns the architecture name known
 * to GDB. The caller must free the returned string with g_free.
 * @gdb_get_dynamic_xml: Callback to return dynamically generated XML for the
 *   gdb stub. Returns a pointer to the XML contents for the specified XML file
 *   or NULL if the CPU doesn't have a dynamically generated content for it.
 * @cpu_exec_enter: Callback for cpu_exec preparation.
 * @cpu_exec_exit: Callback for cpu_exec cleanup.
 * @cpu_exec_interrupt: Callback for processing interrupts in cpu_exec.
 * @disas_set_info: Setup architecture specific components of disassembly info
 * @adjust_watchpoint_address: Perform a target-specific adjustment to an
 * address before attempting to match it against watchpoints.
 *
 * Represents a CPU family or model.
 */
typedef struct CPUClass {
  void (*reset)(CPUState *cpu);
  int reset_dump_flags;
  bool (*has_work)(CPUState *cpu);
  void (*do_interrupt)(CPUState *cpu);
  void (*do_unaligned_access)(CPUState *cpu, vaddr addr,
                              MMUAccessType access_type, int mmu_idx,
                              uintptr_t retaddr);
  void (*do_transaction_failed)(CPUState *cpu, hwaddr physaddr, vaddr addr,
                                unsigned size, MMUAccessType access_type,
                                int mmu_idx, MemTxAttrs attrs,
                                MemTxResult response, uintptr_t retaddr);

  bool (*virtio_is_big_endian)(CPUState *cpu);
  int (*memory_rw_debug)(CPUState *cpu, vaddr addr, uint8_t *buf, int len,
                         bool is_write);
  void (*dump_state)(CPUState *cpu, FILE *, int flags);

  void (*dump_statistics)(CPUState *cpu, int flags);
  int64_t (*get_arch_id)(CPUState *cpu);
  bool (*get_paging_enabled)(const CPUState *cpu);

  void (*set_pc)(CPUState *cpu, vaddr value);
  void (*synchronize_from_tb)(CPUState *cpu, struct TranslationBlock *tb);

  bool (*tlb_fill)(CPUState *cpu, vaddr address, int size,
                   MMUAccessType access_type, int mmu_idx, bool probe,
                   uintptr_t retaddr);
  hwaddr (*get_phys_page_debug)(CPUState *cpu, vaddr addr);
  hwaddr (*get_phys_page_attrs_debug)(CPUState *cpu, vaddr addr,
                                      MemTxAttrs *attrs);

  int (*asidx_from_attrs)(CPUState *cpu, MemTxAttrs attrs);
  int (*gdb_read_register)(CPUState *cpu, uint8_t *buf, int reg);
  int (*gdb_write_register)(CPUState *cpu, uint8_t *buf, int reg);
  bool (*debug_check_watchpoint)(CPUState *cpu, CPUWatchpoint *wp);
  void (*debug_excp_handler)(CPUState *cpu);

  void (*cpu_exec_enter)(CPUState *cpu);
  void (*cpu_exec_exit)(CPUState *cpu);
  bool (*cpu_exec_interrupt)(CPUState *cpu, int interrupt_request);

  vaddr (*adjust_watchpoint_address)(CPUState *cpu, vaddr addr, int len);
  void (*tcg_initialize)(void);
} CPUClass;

static inline CPUClass *CPU_GET_CLASS(const CPUState *cpu) {
  assert(cpu->cc);
  return cpu->cc;
}

static inline void CPU_SET_CLASS(CPUState *cpu, CPUClass *cc) {
  assert(cc != NULL);
  cpu->cc = cc;
}

#define CPU_CLASS(oc)                                                          \
  ({                                                                           \
    X86CPUClass *tmp = oc;                                                     \
    (CPUClass *)tmp;                                                           \
  })

/* Since this macro is used a lot in hot code paths and in conjunction with
 * FooCPU *foo_env_get_cpu(), we deviate from usual QOM practice by using
 * an unchecked cast.
 */
#define CPU(obj) ((CPUState *)(obj))

/* current CPU in the current thread. It is only valid inside
   cpu_exec() */
extern CPUState *current_cpu;

/* Breakpoint/watchpoint flags */
#define BP_MEM_READ 0x01
#define BP_MEM_WRITE 0x02
#define BP_MEM_ACCESS (BP_MEM_READ | BP_MEM_WRITE)
#define BP_STOP_BEFORE_ACCESS 0x04
/* 0x08 currently unused */
#define BP_GDB 0x10
#define BP_CPU 0x20
#define BP_ANY (BP_GDB | BP_CPU)
#define BP_WATCHPOINT_HIT_READ 0x40
#define BP_WATCHPOINT_HIT_WRITE 0x80
#define BP_WATCHPOINT_HIT (BP_WATCHPOINT_HIT_READ | BP_WATCHPOINT_HIT_WRITE)

int cpu_breakpoint_insert(CPUState *cpu, vaddr pc, int flags,
                          CPUBreakpoint **breakpoint);
int cpu_breakpoint_remove(CPUState *cpu, vaddr pc, int flags);
void cpu_breakpoint_remove_by_ref(CPUState *cpu, CPUBreakpoint *breakpoint);
void cpu_breakpoint_remove_all(CPUState *cpu, int mask);

int cpu_watchpoint_insert(CPUState *cpu, vaddr addr, vaddr len, int flags,
                          CPUWatchpoint **watchpoint);
int cpu_watchpoint_remove(CPUState *cpu, vaddr addr, vaddr len, int flags);
void cpu_watchpoint_remove_by_ref(CPUState *cpu, CPUWatchpoint *watchpoint);
void cpu_watchpoint_remove_all(CPUState *cpu, int mask);
void cpu_check_watchpoint(CPUState *cpu, vaddr addr, vaddr len,
                          MemTxAttrs attrs, int flags, uintptr_t ra);
int cpu_watchpoint_address_matches(CPUState *cpu, vaddr addr, vaddr len);

/**
 * CPUDumpFlags:
 * @CPU_DUMP_CODE:
 * @CPU_DUMP_FPU: dump FPU register state, not just integer
 * @CPU_DUMP_CCOP: dump info about TCG QEMU's condition code optimization state
 */
enum CPUDumpFlags {
  CPU_DUMP_CODE = 0x00010000,
  CPU_DUMP_FPU = 0x00020000,
  CPU_DUMP_CCOP = 0x00040000,
};

/**
 * cpu_reset:
 * @cpu: The CPU whose state is to be reset.
 */
void cpu_reset(CPUState *cpu);

void tcg_handle_interrupt(CPUState *cpu, int mask);
/**
 * cpu_interrupt:
 * @cpu: The CPU to set an interrupt on.
 * @mask: The interrupts to set.
 *
 * Invokes the interrupt handler.
 */
static inline void cpu_interrupt(CPUState *cpu, int mask) {
  // [interface 32]
  tcg_handle_interrupt(cpu, mask);
}

void QEMU_NORETURN cpu_abort(CPUState *cpu, const char *fmt, ...)
    GCC_FMT_ATTR(2, 3);

/* work queue */

/* The union type allows passing of 64 bit target pointers on 32 bit
 * hosts in a single parameter
 */
typedef union {
  int host_int;
  unsigned long host_ulong;
  void *host_ptr;
  vaddr target_ptr;
} run_on_cpu_data;

#define RUN_ON_CPU_HOST_PTR(p) ((run_on_cpu_data){.host_ptr = (p)})
#define RUN_ON_CPU_HOST_INT(i) ((run_on_cpu_data){.host_int = (i)})
#define RUN_ON_CPU_HOST_ULONG(ul) ((run_on_cpu_data){.host_ulong = (ul)})
#define RUN_ON_CPU_TARGET_PTR(v) ((run_on_cpu_data){.target_ptr = (v)})
#define RUN_ON_CPU_NULL RUN_ON_CPU_HOST_PTR(NULL)

typedef void (*run_on_cpu_func)(CPUState *cpu, run_on_cpu_data data);

void async_safe_run_on_cpu(CPUState *cpu, run_on_cpu_func func,
                           run_on_cpu_data data);

typedef struct CPUWatchpoint CPUWatchpoint;

#define SSTEP_ENABLE 0x1  /* Enable simulated HW single stepping */
#define SSTEP_NOIRQ 0x2   /* Do not use IRQ while single stepping */
#define SSTEP_NOTIMER 0x4 /* Do not Timers while single stepping */

#define CPU_UNSET_NUMA_NODE_ID -1
#define CPU_TRACE_DSTATE_MAX_EVENTS 32

typedef QTAILQ_HEAD(CPUTailQ, CPUState) CPUTailQ;
extern CPUTailQ cpus;

#define first_cpu QTAILQ_FIRST(&cpus)
#define CPU_NEXT(cpu)                                                          \
  ({                                                                           \
    assert(cpu != NULL);                                                       \
    QTAILQ_NEXT(cpu, node);                                                    \
  })
#define CPU_FOREACH(cpu) QTAILQ_FOREACH(cpu, &cpus, node)

/**
 * cpu_in_exclusive_context()
 * @cpu: The vCPU to check
 *
 * Returns true if @cpu is an exclusive context, for example running
 * something which has previously been queued via async_safe_run_on_cpu().
 */
static inline bool cpu_in_exclusive_context(const CPUState *cpu) {
  return cpu->in_exclusive_context;
}

static inline void cpu_tb_jmp_cache_clear(CPUState *cpu) {
  unsigned int i;

  for (i = 0; i < TB_JMP_CACHE_SIZE; i++) {
    qatomic_set(&cpu->tb_jmp_cache[i], NULL);
  }
}

/**
 * cpu_reset_interrupt:
 * @cpu: The CPU to clear the interrupt on.
 * @mask: The interrupt mask to clear.
 *
 * Resets interrupts on the vCPU @cpu.
 */
void cpu_reset_interrupt(CPUState *cpu, int mask);

/**
 * cpu_has_work:
 * @cpu: The vCPU to check.
 *
 * Checks whether the CPU has work to do.
 *
 * Returns: %true if the CPU has work, %false otherwise.
 */
static inline bool cpu_has_work(CPUState *cpu) {
  CPUClass *cc = CPU_GET_CLASS(cpu);

  assert(cc->has_work);
  return cc->has_work(cpu);
}

/**
 * async_run_on_cpu:
 * @cpu: The vCPU to run on.
 * @func: The function to be executed.
 * @data: Data to pass to the function.
 *
 * Schedules the function @func for execution on the vCPU @cpu asynchronously.
 */
void async_run_on_cpu(CPUState *cpu, run_on_cpu_func func,
                      run_on_cpu_data data);

static inline void cpu_unaligned_access(CPUState *cpu, vaddr addr,
                                        MMUAccessType access_type, int mmu_idx,
                                        uintptr_t retaddr) {
  g_assert_not_reached();
}

#define qemu_tcg_mttcg_enabled() (0)

/** cpu_asidx_from_attrs:
 * @cpu: CPU
 * @attrs: memory transaction attributes
 *
 * Returns the address space index specifying the CPU AddressSpace
 * to use for a memory access with the given transaction attributes.
 */
static inline int cpu_asidx_from_attrs(CPUState *cpu, MemTxAttrs attrs) {
  CPUClass *cc = CPU_GET_CLASS(cpu);
  int ret = 0;

  if (cc->asidx_from_attrs) {
    ret = cc->asidx_from_attrs(cpu, attrs);
    assert(ret < cpu->num_ases && ret >= 0);
  }
  return ret;
}

/**
 * cpu_get_phys_page_attrs_debug:
 * @cpu: The CPU to obtain the physical page address for.
 * @addr: The virtual address.
 * @attrs: Updated on return with the memory transaction attributes to use
 *         for this access.
 *
 * Obtains the physical page corresponding to a virtual one, together
 * with the corresponding memory transaction attributes to use for the access.
 * Use it only for debugging because no protection checks are done.
 *
 * Returns: Corresponding physical page address or -1 if no page found.
 */
static inline hwaddr cpu_get_phys_page_attrs_debug(CPUState *cpu, vaddr addr,
                                                   MemTxAttrs *attrs) {
  CPUClass *cc = CPU_GET_CLASS(cpu);

  if (cc->get_phys_page_attrs_debug) {
    return cc->get_phys_page_attrs_debug(cpu, addr, attrs);
  }
  /* Fallback for CPUs which don't implement the _attrs_ hook */
  *attrs = MEMTXATTRS_UNSPECIFIED;
  return cc->get_phys_page_debug(cpu, addr);
}

// [interface 36]
bool qemu_cpu_is_self(CPUState *cpu);
void enter_interrpt_context();
void leave_interrpt_context();

/**
 * qemu_cpu_kick:
 * @cpu: The vCPU to kick.
 *
 * Kicks @cpu's thread.
 */
void qemu_cpu_kick(CPUState *cpu);

/**
 * cpu_exec_start:
 * @cpu: The CPU for the current thread.
 *
 * Record that a CPU has started execution and can be interrupted with
 * cpu_exit.
 */
static inline void cpu_exec_start(CPUState *cpu) {
  // [interface 15]
}

/**
 * cpu_exec_end:
 * @cpu: The CPU for the current thread.
 *
 * Record that a CPU has stopped execution and exclusive sections
 * can be executed without interrupting it.
 */
static inline void cpu_exec_end(CPUState *cpu) {
  // [interface 15]
}

/**
 * process_queued_cpu_work() - process all items on CPU work queue
 * @cpu: The CPU which work queue to process.
 */
void process_queued_cpu_work(CPUState *cpu);

/**
 * start_exclusive:
 *
 * Wait for a concurrent exclusive section to end, and then start
 * a section of work that is run while other CPUs are not running
 * between cpu_exec_start and cpu_exec_end.  CPUs that are running
 * cpu_exec are exited immediately.  CPUs that call cpu_exec_start
 * during the exclusive section go to sleep until this CPU calls
 * end_exclusive.
 */
static inline void start_exclusive(void) {
  // [interface 15]
}

void cpu_exit(CPUState *cpu);

/**
 * end_exclusive:
 *
 * Concludes an exclusive execution section started by start_exclusive.
 */
static inline void end_exclusive(void) {
  // [interface 15]
}

#define UNASSIGNED_CPU_INDEX -1
#define UNASSIGNED_CLUSTER_INDEX -1

void cpu_exec_initfn(CPUState *cpu);
void cpu_exec_realizefn(CPUState *cpu);

/**
 * qemu_init_vcpu:
 * @cpu: The vCPU to initialize.
 *
 * Initializes a vCPU.
 */
void qemu_init_vcpu(CPUState *cpu);

void cpu_class_init(CPUClass *k);
void cpu_common_initfn(CPUState *cpu);

/**
 * cpu_list_add:
 * @cpu: The CPU to be added to the list of CPUs.
 */
void cpu_list_add(CPUState *cpu);

/**
 * cpu_list_remove:
 * @cpu: The CPU to be removed from the list of CPUs.
 */
void cpu_list_remove(CPUState *cpu);

// originally put into cpu-common.h, put here because of header cycle dependence
void tcg_flush_softmmu_tlb(CPUState *cs);

static inline void cpu_synchronize_state(CPUState *cpu) {
  // tcg doesn't need synchronization
}

/**
 * cpu_dump_state:
 * @cpu: The CPU whose state is to be dumped.
 * @f: If non-null, dump to this stream, else to current print sink.
 *
 * Dumps CPU state.
 */
void cpu_dump_state(CPUState *cpu, FILE *f, int flags);

#endif /* end of include guard: CPU_H_5RAXENPS */
