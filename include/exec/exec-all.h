#ifndef EXEC_ALL_H_SFIHOIQZ
#define EXEC_ALL_H_SFIHOIQZ
#include "../hw/core/cpu.h"
#include "../types.h"
#include "cpu-defs.h"


// TODO it seems this is the hacking of xqm
// copy here blindly
// maybe mvoe ExtraBlock to LATX
// 1. typedef char int8; defined by LATX
// 2. use by _top_in and _top_out

/* extra attributes we need in TB */
typedef struct ExtraBlock{
    /* @pc: ID of a ETB.
       It is equal to EIP + CS_BASE in system-mode  */
    uint64_t pc;
    struct IR1_INST *_ir1_instructions;
    int16_t  _ir1_num;
    /* which tb this etb belongs to */
    struct TranslationBlock* tb;
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
  u32 flags;  /* flags defining in which context the code was generated */
  u16 size;   /* size of target code for this block (1 <=
                      size <= TARGET_PAGE_SIZE) */
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

} TranslationBlock;

/* current cflags for hashing/comparison */
static inline u32 curr_cflags(void) {
  // TODO
  return 0;
}

TranslationBlock *tb_gen_code(CPUState *cpu, target_ulong pc,
                              target_ulong cs_base, u32 flags, int cflags);

// TODO why is 16 ?
#define CODE_GEN_ALIGN 16 /* must be >= of the size of a icache line */
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

/* vl.c */
extern int singlestep;

#endif /* end of include guard: EXEC_ALL_H_SFIHOIQZ */
