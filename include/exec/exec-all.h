#ifndef EXEC_ALL_H_SFIHOIQZ
#define EXEC_ALL_H_SFIHOIQZ
#include "../../src/i386/cpu.h"
#include "../hw/core/cpu.h"
#include "../qemu/atomic.h"
#include "../sysemu/cpus.h"
#include "../types.h"
#include "cpu-defs.h"

// FIXME it seems this is the hacking of xqm
// copy here blindly
// maybe mvoe ExtraBlock to LATX
// 1. typedef char int8; defined by LATX
// 2. use by _top_in and _top_out

/* extra attributes we need in TB */
typedef struct ExtraBlock {
  /* @pc: ID of a ETB.
     It is equal to EIP + CS_BASE in system-mode  */
  uint64_t pc;
  struct IR1_INST *_ir1_instructions;
  int16_t _ir1_num;
  /* which tb this etb belongs to */
  struct TranslationBlock *tb;
  /* record the last instruction if TB is too large */
  struct IR1_INST *tb_too_large_pir1;

  struct IR1_INST *sys_eob_pir1;

  bool tb_need_cpc[2]; /* cross page check */

  bool end_with_jcc;
  uintptr_t mips_branch_inst_offset;
  bool branch_to_target_direct_in_mips_branch;
  uint32_t mips_branch_backup;

  // TODO what do you mean by historical field
  // FIXME change type from char to int8 back
  /* historical field */
  char _top_in;
  char _top_out;
  void *next_tb[2];
} ETB;

// TODO ???
/*
 * Translation Cache-related fields of a TB.
 * This struct exists just for convenience; we keep track of TB's in a binary
 * search tree, and the only fields needed to compare TB's in the tree are
 * @ptr and @size.
 * Note: the address of search data can be obtained by adding @size to @ptr.
 */
struct tb_tc {
  void *ptr; /* pointer to the translated code */
  size_t size;
};

typedef struct TranslationBlock {

  /* simulated PC corresponding to this block (EIP + CS base) */
  target_ulong pc;
  target_ulong cs_base; /* CS base for this block */
  u32 flags; /* flags defining in which context the code was generated */
  u16 size;  /* size of target code for this block (1 <=
                     size <= TARGET_PAGE_SIZE) */
  // FIXME
  // What's icount, how to handle it?
  // what's the relatetion with icount_decr?
  uint16_t icount;
  u32 cflags; /* compile flags */

#define CF_COUNT_MASK 0x00007fff
#define CF_LAST_IO 0x00008000 /* Last insn may be an IO access.  */
#define CF_NOCACHE 0x00010000 /* To be freed after execution */
#define CF_USE_ICOUNT 0x00020000
#define CF_INVALID 0x00040000      /* TB is stale. Set with @jmp_lock held */
#define CF_PARALLEL 0x00080000     /* Generate code for a parallel context */
#define CF_CLUSTER_MASK 0xff000000 /* Top 8 bits are cluster ID */
#define CF_CLUSTER_SHIFT 24
/* cflags' mask for hashing/comparison */
#define CF_HASH_MASK                                                           \
  (CF_COUNT_MASK | CF_LAST_IO | CF_USE_ICOUNT | CF_PARALLEL | CF_CLUSTER_MASK)

  struct tb_tc tc;

  // TODO deeper
  /* original tb when cflags has CF_NOCACHE */
  struct TranslationBlock *orig_tb;

  /* jmp_lock placed here to fill a 4-byte hole. Its documentation is below */
  QemuSpin jmp_lock;

  tb_page_addr_t page_addr[2];

  /* The following data are used to directly call another TB from
   * the code of this one. This can be done either by emitting direct or
   * indirect native jump instructions. These jumps are reset so that the TB
   * just continues its execution. The TB can be linked to another one by
   * setting one of the jump targets (or patching the jump instruction). Only
   * two of such jumps are supported.
   */
  u16 jmp_reset_offset[2];                 /* offset of original jump target */
#define TB_JMP_RESET_OFFSET_INVALID 0xffff /* indicates no jump generated */
  uintptr_t jmp_target_arg[2];             /* target address or offset */

  /*
   * Each TB has a NULL-terminated list (jmp_list_head) of incoming jumps.
   * Each TB can have two outgoing jumps, and therefore can participate
   * in two lists. The list entries are kept in jmp_list_next[2]. The least
   * significant bit (LSB) of the pointers in these lists is used to encode
   * which of the two list entries is to be used in the pointed TB.
   *
   * List traversals are protected by jmp_lock. The destination TB of each
   * outgoing jump is kept in jmp_dest[] so that the appropriate jmp_lock
   * can be acquired from any origin TB.
   *
   * jmp_dest[] are tagged pointers as well. The LSB is set when the TB is
   * being invalidated, so that no further outgoing jumps from it can be set.
   *
   * jmp_lock also protects the CF_INVALID cflag; a jump must not be chained
   * to a destination TB that has CF_INVALID set.
   */
  uintptr_t jmp_list_head;
  uintptr_t jmp_list_next[2];
  uintptr_t jmp_dest[2];

  /* remember to free these memory when QEMU recycle one TB */
  ETB *extra_tb;

  /* Per-vCPU dynamic tracing state used to generate this TB */
  uint32_t trace_vcpu_dstate;

  /* first and second physical page containing code. The lower bit
     of the pointer tells the index in page_next[].
     The list is protected by the TB's page('s) lock(s) */
  uintptr_t page_next[2];

} TranslationBlock;

extern bool parallel_cpus;

/* current cflags for hashing/comparison */
static inline u32 curr_cflags(void) {
  return (parallel_cpus ? CF_PARALLEL : 0) | (use_icount ? CF_USE_ICOUNT : 0);
}

// FIXME why is 16 ?
#define CODE_GEN_ALIGN 16 /* must be >= of the size of a icache line */

/* vl.c */
extern int singlestep;

#define GETPC()                                                                \
  ((uintptr_t)__builtin_extract_return_addr(__builtin_return_address(0)))

void tlb_flush(CPUState *cpu);

// FIXME wanna cry
void QEMU_NORETURN cpu_loop_exit_noexc(CPUState *cpu);

// FIXME copied from log.h begin ========================
/* log only if a bit is set on the current loglevel mask:
 * @mask: bit to check in the mask
 * @fmt: printf-style format string
 * @args: optional arguments for format string
 */
#define qemu_log_mask(MASK, FMT, ...)                                          \
  do {                                                                         \
  } while (0)

#define CPU_LOG_TB_OUT_ASM (1 << 0)
#define CPU_LOG_TB_IN_ASM (1 << 1)
#define CPU_LOG_TB_OP (1 << 2)
#define CPU_LOG_TB_OP_OPT (1 << 3)
#define CPU_LOG_INT (1 << 4)
#define CPU_LOG_EXEC (1 << 5)
#define CPU_LOG_PCALL (1 << 6)
#define CPU_LOG_TB_CPU (1 << 8)
#define CPU_LOG_RESET (1 << 9)
#define LOG_UNIMP (1 << 10)
#define LOG_GUEST_ERROR (1 << 11)
#define CPU_LOG_MMU (1 << 12)
#define CPU_LOG_TB_NOCHAIN (1 << 13)
#define CPU_LOG_PAGE (1 << 14)
/* LOG_TRACE (1 << 15) is defined in log-for-trace.h */
#define CPU_LOG_TB_OP_IND (1 << 16)
#define CPU_LOG_TB_FPU (1 << 17)
#define CPU_LOG_PLUGIN (1 << 18)

/* Return the number of characters emitted.  */
int qemu_log(const char *fmt, ...);

// FIXME implement it
void log_cpu_state(CPUState *cpu, int flags);
// FIXME copied from log.h end ======================

/**
 * cpu_restore_state:
 * @cpu: the vCPU state is to be restore to
 * @searched_pc: the host PC the fault occurred at
 * @will_exit: true if the TB executed will be interrupted after some
               cpu adjustments. Required for maintaining the correct
               icount valus
 * @return: true if state was restored, false otherwise
 *
 * Attempt to restore the state for a fault occurring in translated
 * code. If the searched_pc is not in translated code no state is
 * restored and the function returns false.
 */
// FIXME translate-all.c will be supported soon, maybe not every one is needed
bool cpu_restore_state(CPUState *cpu, uintptr_t searched_pc, bool will_exit);

void QEMU_NORETURN cpu_loop_exit_noexc(CPUState *cpu);
void QEMU_NORETURN cpu_io_recompile(CPUState *cpu, uintptr_t retaddr);
TranslationBlock *tb_gen_code(CPUState *cpu, target_ulong pc,
                              target_ulong cs_base, uint32_t flags, int cflags);

void QEMU_NORETURN cpu_loop_exit(CPUState *cpu);
void QEMU_NORETURN cpu_loop_exit_restore(CPUState *cpu, uintptr_t pc);
void QEMU_NORETURN cpu_loop_exit_atomic(CPUState *cpu, uintptr_t pc);

/* cputlb.c */
/**
 * tlb_init - initialize a CPU's TLB
 * @cpu: CPU whose TLB should be initialized
 */
void tlb_init(CPUState *cpu);
/**
 * tlb_flush_page:
 * @cpu: CPU whose TLB should be flushed
 * @addr: virtual address of page to be flushed
 *
 * Flush one page from the TLB of the specified CPU, for all
 * MMU indexes.
 */
void tlb_flush_page(CPUState *cpu, target_ulong addr);
/**
 * tlb_flush_page_all_cpus:
 * @cpu: src CPU of the flush
 * @addr: virtual address of page to be flushed
 *
 * Flush one page from the TLB of the specified CPU, for all
 * MMU indexes.
 */
void tlb_flush_page_all_cpus(CPUState *src, target_ulong addr);
/**
 * tlb_flush_page_all_cpus_synced:
 * @cpu: src CPU of the flush
 * @addr: virtual address of page to be flushed
 *
 * Flush one page from the TLB of the specified CPU, for all MMU
 * indexes like tlb_flush_page_all_cpus except the source vCPUs work
 * is scheduled as safe work meaning all flushes will be complete once
 * the source vCPUs safe work is complete. This will depend on when
 * the guests translation ends the TB.
 */
void tlb_flush_page_all_cpus_synced(CPUState *src, target_ulong addr);
/**
 * tlb_flush:
 * @cpu: CPU whose TLB should be flushed
 *
 * Flush the entire TLB for the specified CPU. Most CPU architectures
 * allow the implementation to drop entries from the TLB at any time
 * so this is generally safe. If more selective flushing is required
 * use one of the other functions for efficiency.
 */
void tlb_flush(CPUState *cpu);
/**
 * tlb_flush_all_cpus:
 * @cpu: src CPU of the flush
 */
void tlb_flush_all_cpus(CPUState *src_cpu);
/**
 * tlb_flush_all_cpus_synced:
 * @cpu: src CPU of the flush
 *
 * Like tlb_flush_all_cpus except this except the source vCPUs work is
 * scheduled as safe work meaning all flushes will be complete once
 * the source vCPUs safe work is complete. This will depend on when
 * the guests translation ends the TB.
 */
void tlb_flush_all_cpus_synced(CPUState *src_cpu);
/**
 * tlb_flush_page_by_mmuidx:
 * @cpu: CPU whose TLB should be flushed
 * @addr: virtual address of page to be flushed
 * @idxmap: bitmap of MMU indexes to flush
 *
 * Flush one page from the TLB of the specified CPU, for the specified
 * MMU indexes.
 */
void tlb_flush_page_by_mmuidx(CPUState *cpu, target_ulong addr,
                              uint16_t idxmap);
/**
 * tlb_flush_page_by_mmuidx_all_cpus:
 * @cpu: Originating CPU of the flush
 * @addr: virtual address of page to be flushed
 * @idxmap: bitmap of MMU indexes to flush
 *
 * Flush one page from the TLB of all CPUs, for the specified
 * MMU indexes.
 */
void tlb_flush_page_by_mmuidx_all_cpus(CPUState *cpu, target_ulong addr,
                                       uint16_t idxmap);
/**
 * tlb_flush_page_by_mmuidx_all_cpus_synced:
 * @cpu: Originating CPU of the flush
 * @addr: virtual address of page to be flushed
 * @idxmap: bitmap of MMU indexes to flush
 *
 * Flush one page from the TLB of all CPUs, for the specified MMU
 * indexes like tlb_flush_page_by_mmuidx_all_cpus except the source
 * vCPUs work is scheduled as safe work meaning all flushes will be
 * complete once  the source vCPUs safe work is complete. This will
 * depend on when the guests translation ends the TB.
 */
void tlb_flush_page_by_mmuidx_all_cpus_synced(CPUState *cpu, target_ulong addr,
                                              uint16_t idxmap);
/**
 * tlb_flush_by_mmuidx:
 * @cpu: CPU whose TLB should be flushed
 * @wait: If true ensure synchronisation by exiting the cpu_loop
 * @idxmap: bitmap of MMU indexes to flush
 *
 * Flush all entries from the TLB of the specified CPU, for the specified
 * MMU indexes.
 */
void tlb_flush_by_mmuidx(CPUState *cpu, uint16_t idxmap);
/**
 * tlb_flush_by_mmuidx_all_cpus:
 * @cpu: Originating CPU of the flush
 * @idxmap: bitmap of MMU indexes to flush
 *
 * Flush all entries from all TLBs of all CPUs, for the specified
 * MMU indexes.
 */
void tlb_flush_by_mmuidx_all_cpus(CPUState *cpu, uint16_t idxmap);
/**
 * tlb_flush_by_mmuidx_all_cpus_synced:
 * @cpu: Originating CPU of the flush
 * @idxmap: bitmap of MMU indexes to flush
 *
 * Flush all entries from all TLBs of all CPUs, for the specified
 * MMU indexes like tlb_flush_by_mmuidx_all_cpus except except the source
 * vCPUs work is scheduled as safe work meaning all flushes will be
 * complete once  the source vCPUs safe work is complete. This will
 * depend on when the guests translation ends the TB.
 */
void tlb_flush_by_mmuidx_all_cpus_synced(CPUState *cpu, uint16_t idxmap);
/**
 * tlb_set_page_with_attrs:
 * @cpu: CPU to add this TLB entry for
 * @vaddr: virtual address of page to add entry for
 * @paddr: physical address of the page
 * @attrs: memory transaction attributes
 * @prot: access permissions (PAGE_READ/PAGE_WRITE/PAGE_EXEC bits)
 * @mmu_idx: MMU index to insert TLB entry for
 * @size: size of the page in bytes
 *
 * Add an entry to this CPU's TLB (a mapping from virtual address
 * @vaddr to physical address @paddr) with the specified memory
 * transaction attributes. This is generally called by the target CPU
 * specific code after it has been called through the tlb_fill()
 * entry point and performed a successful page table walk to find
 * the physical address and attributes for the virtual address
 * which provoked the TLB miss.
 *
 * At most one entry for a given virtual address is permitted. Only a
 * single TARGET_PAGE_SIZE region is mapped; the supplied @size is only
 * used by tlb_flush_page.
 */
void tlb_set_page_with_attrs(CPUState *cpu, target_ulong vaddr, hwaddr paddr,
                             MemTxAttrs attrs, int prot, int mmu_idx,
                             target_ulong size);
/* tlb_set_page:
 *
 * This function is equivalent to calling tlb_set_page_with_attrs()
 * with an @attrs argument of MEMTXATTRS_UNSPECIFIED. It's provided
 * as a convenience for CPUs which don't use memory transaction attributes.
 */
void tlb_set_page(CPUState *cpu, target_ulong vaddr, hwaddr paddr, int prot,
                  int mmu_idx, target_ulong size);

// FIXME copied from /usr/include/glib-2.0/glib/gmem.h
// of course, we will rewrite all the
#define g_new(type, num) (type *)NULL
#define g_try_new(type, num) (type *)NULL
#define g_free(type)                                                           \
  {}
#define g_assert(expr)                                                         \
  {}
#define g_assert_not_reached()                                                 \
  {}

#define g_new0(struct_type, n_structs) (struct_type *)NULL
#define g_malloc(size) NULL

#undef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#undef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/* The true return address will often point to a host insn that is part of
   the next translated guest insn.  Adjust the address backward to point to
   the middle of the call insn.  Subtracting one would do the job except for
   several compressed mode architectures (arm, mips) which set the low bit
   to indicate the compressed mode; subtracting two works around that.  It
   is also the case that there are no host isas that contain a call insn
   smaller than 4 bytes, so we don't worry about special-casing this.  */
#define GETPC_ADJ 2

/* exec.c */
void tb_flush_jmp_cache(CPUState *cpu, target_ulong addr);

#if defined(CONFIG_USER_ONLY)
void mmap_lock(void);
void mmap_unlock(void);
bool have_mmap_lock(void);

/**
 * get_page_addr_code() - user-mode version
 * @env: CPUArchState
 * @addr: guest virtual address of guest code
 *
 * Returns @addr.
 */
static inline tb_page_addr_t get_page_addr_code(CPUArchState *env,
                                                target_ulong addr) {
  return addr;
}

/**
 * get_page_addr_code_hostp() - user-mode version
 * @env: CPUArchState
 * @addr: guest virtual address of guest code
 *
 * Returns @addr.
 *
 * If @hostp is non-NULL, sets *@hostp to the host address where @addr's content
 * is kept.
 */
static inline tb_page_addr_t
get_page_addr_code_hostp(CPUArchState *env, target_ulong addr, void **hostp) {
  if (hostp) {
    *hostp = g2h(addr);
  }
  return addr;
}
#else
// FIXME why is mmap_lock empty in system mode?
// In another word, why mmap_lock is necessary for user mode ?
static inline void mmap_lock(void) {}
static inline void mmap_unlock(void) {}

/**
 * get_page_addr_code() - full-system version
 * @env: CPUArchState
 * @addr: guest virtual address of guest code
 *
 * If we cannot translate and execute from the entire RAM page, or if
 * the region is not backed by RAM, returns -1. Otherwise, returns the
 * ram_addr_t corresponding to the guest code at @addr.
 *
 * Note: this function can trigger an exception.
 */
tb_page_addr_t get_page_addr_code(CPUArchState *env, target_ulong addr);

/**
 * get_page_addr_code_hostp() - full-system version
 * @env: CPUArchState
 * @addr: guest virtual address of guest code
 *
 * See get_page_addr_code() (full-system version) for documentation on the
 * return value.
 *
 * Sets *@hostp (when @hostp is non-NULL) as follows.
 * If the return value is -1, sets *@hostp to NULL. Otherwise, sets *@hostp
 * to the host address where @addr's content is kept.
 *
 * Note: this function can trigger an exception.
 */
tb_page_addr_t get_page_addr_code_hostp(CPUArchState *env, target_ulong addr,
                                        void **hostp);

// FIXME I don't know who needs them and where they are defined.
#if 0
void tlb_reset_dirty(CPUState *cpu, ram_addr_t start1, ram_addr_t length);
void tlb_set_dirty(CPUState *cpu, target_ulong vaddr);

/* exec.c */
void tb_flush_jmp_cache(CPUState *cpu, target_ulong addr);

MemoryRegionSection *
address_space_translate_for_iotlb(CPUState *cpu, int asidx, hwaddr addr,
                                  hwaddr *xlat, hwaddr *plen,
                                  MemTxAttrs attrs, int *prot);
hwaddr memory_region_section_get_iotlb(CPUState *cpu,
                                       MemoryRegionSection *section);
#endif
#endif

void tb_phys_invalidate(TranslationBlock *tb, tb_page_addr_t page_addr);

/* Hide the atomic_read to make code a little easier on the eyes */
static inline uint32_t tb_cflags(const TranslationBlock *tb) {
  return atomic_read(&tb->cflags);
}

TranslationBlock *tb_htable_lookup(CPUState *cpu, target_ulong pc,
                                   target_ulong cs_base, uint32_t flags,
                                   uint32_t cf_mask);

#if !defined(CONFIG_USER_ONLY) && defined(CONFIG_DEBUG_TCG)
void assert_no_pages_locked(void);
#else
static inline void assert_no_pages_locked(void) {}
#endif

void tb_set_jmp_target(TranslationBlock *tb, int n, uintptr_t addr);

#endif /* end of include guard: EXEC_ALL_H_SFIHOIQZ */
