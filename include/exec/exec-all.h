#ifndef EXEC_ALL_H_SFIHOIQZ
#define EXEC_ALL_H_SFIHOIQZ
#include "cpu.h"
#include "../hw/core/cpu.h"
#include "../qemu/atomic.h"
#include "../qemu/main-loop.h"
#include "cpu-defs.h"
#include "memop.h"

#ifdef CONFIG_LATX
#include "latx-types.h" // for int8
struct IR1_INST;
/* extra attributes we need in TB */
typedef struct ExtraBlock {
  uint64_t pc;                 /* for hash compare, it's ID of a ETB */
  struct TranslationBlock *tb; /* which tb this etb belongs to */
#if defined(CONFIG_LATX_FLAG_PATTERN) || defined(CONFIG_LATX_FLAG_REDUCTION)
  uint8 pending_use; /* indicate which eflags are used but hasn't defined yet */
#endif
#ifdef CONFIG_LATX_FLAG_REDUCTION
  int8 _tb_type;
  struct ExtraBlock *succ[2]; /* successors of this ETB */
  /* flags is used to indicate the state of this ETB
   * bit0: set if succ[2] are set
   * bit1: set if pending_use is set */
  uint8 flags;
#define SUCC_IS_SET_MASK 0x01
#define PENDING_USE_IS_SET_MASK 0x02
#endif
  /* historical field */
  uint16_t size;
} ETB;
#endif

/* Page tracking code uses ram addresses in system mode, and virtual
   addresses in userspace mode.  Define tb_page_addr_t to be an appropriate
   type.  */
#if defined(CONFIG_USER_ONLY)
typedef abi_ulong tb_page_addr_t;
#define TB_PAGE_ADDR_FMT TARGET_ABI_FMT_lx
#else
typedef ram_addr_t tb_page_addr_t;
#define TB_PAGE_ADDR_FMT RAM_ADDR_FMT
#endif

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
  uint32_t flags; /* flags defining in which context the code was generated */
  uint16_t size;  /* size of target code for this block (1 <=
                     size <= TARGET_PAGE_SIZE) */
  uint16_t icount;
  uint32_t cflags; /* compile flags */

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

  /* Per-vCPU dynamic tracing state used to generate this TB */
  uint32_t trace_vcpu_dstate;

  struct tb_tc tc;

  /* original tb when cflags has CF_NOCACHE */
  struct TranslationBlock *orig_tb;

  /* jmp_lock placed here to fill a 4-byte hole. Its documentation is below */
  QemuSpin jmp_lock;

  /* first and second physical page containing code. The lower bit
     of the pointer tells the index in page_next[].
     The list is protected by the TB's page('s) lock(s) */
  uintptr_t page_next[2];
  tb_page_addr_t page_addr[2];

  /* The following data are used to directly call another TB from
   * the code of this one. This can be done either by emitting direct or
   * indirect native jump instructions. These jumps are reset so that the TB
   * just continues its execution. The TB can be linked to another one by
   * setting one of the jump targets (or patching the jump instruction). Only
   * two of such jumps are supported.
   */
  uint16_t jmp_reset_offset[2];            /* offset of original jump target */
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
#ifdef CONFIG_LATX
  struct IR1_INST *_ir1_instructions;
  int8 _top_in;
  int8 _top_out;
  void *next_tb[2];
  /* remember to free these memory when QEMU recycle one TB */
  ETB extra_tb;
#ifdef CONFIG_SOFTMMU
  /* TB ends because of special situation in system-mode */
  struct IR1_INST *sys_eob_pir1;
  /* record the last instruction if TB is too large */
  struct IR1_INST *tb_too_large_pir1;
  int is_indir_tb;
#endif
#endif
} TranslationBlock;

extern bool parallel_cpus;

/* current cflags for hashing/comparison */
static inline uint32_t curr_cflags(void) {
  return (parallel_cpus ? CF_PARALLEL : 0) | (use_icount ? CF_USE_ICOUNT : 0);
}

#define CODE_GEN_ALIGN 16 /* must be >= of the size of a icache line */

/* vl.c */
extern int singlestep;

#define GETPC()                                                                \
  ((uintptr_t)__builtin_extract_return_addr(__builtin_return_address(0)))

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

void tlb_reset_dirty(CPUState *cpu, ram_addr_t start1, ram_addr_t length);
void tlb_set_dirty(CPUState *cpu, target_ulong vaddr);

/* exec.c */
void tb_flush_jmp_cache(CPUState *cpu, target_ulong addr);

bool is_iotlb_mr(MemoryRegion *mr);
MemoryRegion *address_space_translate_for_iotlb(CPUState *cpu, int asidx,
                                                hwaddr addr, hwaddr *xlat,
                                                hwaddr *plen, MemTxAttrs attrs,
                                                int *prot);

static inline hwaddr memory_region_section_get_iotlb(CPUState *cpu,
                                                     MemoryRegion *section) {
  g_assert_not_reached(); // this is not the correct implementation
  return 0;
}
#endif

void tb_phys_invalidate(TranslationBlock *tb, tb_page_addr_t page_addr);

/* Hide the atomic_read to make code a little easier on the eyes */
static inline uint32_t tb_cflags(const TranslationBlock *tb) {
  return qatomic_read(&tb->cflags);
}

#if !defined(CONFIG_USER_ONLY) && defined(CONFIG_DEBUG_TCG)
void assert_no_pages_locked(void);
#else
static inline void assert_no_pages_locked(void) {}
#endif

/* TranslationBlock invalidate API */
#if defined(CONFIG_USER_ONLY)
void tb_invalidate_phys_addr(target_ulong addr);
void tb_invalidate_phys_range(target_ulong start, target_ulong end);
#else
void tb_invalidate_phys_addr(AddressSpace *as, hwaddr addr, MemTxAttrs attrs);
#endif

void tb_set_jmp_target(TranslationBlock *tb, int n, uintptr_t addr);
TranslationBlock *tb_htable_lookup(CPUState *cpu, target_ulong pc,
                                   target_ulong cs_base, uint32_t flags,
                                   uint32_t cf_mask);

static inline void page_size_init(void) {
  // [interface 27]
}

/**
 * iotlb_to_section:
 * @cpu: CPU performing the access
 * @index: TCG CPU IOTLB entry
 *
 * Given a TCG CPU IOTLB entry, return the MemoryRegionSection that
 * it refers to. @index will have been initially created and returned
 * by memory_region_section_get_iotlb().
 */
struct MemoryRegion *iotlb_to_section(CPUState *cpu, hwaddr index,
                                      MemTxAttrs attrs);

/**
 * cpu_address_space_init:
 * @cpu: CPU to add this address space to
 * @asidx: integer index of this address space
 * @prefix: prefix to be used as name of address space
 * @mr: the root memory region of address space
 *
 * Add the specified address space to the CPU's cpu_ases list.
 * The address space added with @asidx 0 is the one used for the
 * convenience pointer cpu->as.
 * The target-specific code which registers ASes is responsible
 * for defining what semantics address space 0, 1, 2, etc have.
 *
 * Before the first call to this function, the caller must set
 * cpu->num_ases to the total number of address spaces it needs
 * to support.
 *
 * Note that with KVM only one address space is supported.
 */
void cpu_address_space_init(CPUState *cpu, int asidx, const char *prefix);

#endif /* end of include guard: EXEC_ALL_H_SFIHOIQZ */
