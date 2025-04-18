#ifndef TB_HASH_H_HPADT4RC
#define TB_HASH_H_HPADT4RC

#include "../exec/cpu-defs.h"
#include "../exec/exec-all.h"
#include "../qemu/xxhash.h"

#ifdef CONFIG_SOFTMMU

/* Only the bottom TB_JMP_PAGE_BITS of the jump cache hash bits vary for
   addresses on the same page.  The top bits are the same.  This allows
   TLB invalidation to quickly clear a subset of the hash table.  */
#define TB_JMP_PAGE_BITS (TB_JMP_CACHE_BITS / 2)
#define TB_JMP_PAGE_SIZE (1 << TB_JMP_PAGE_BITS)
#define TB_JMP_ADDR_MASK (TB_JMP_PAGE_SIZE - 1)
#define TB_JMP_PAGE_MASK (TB_JMP_CACHE_SIZE - TB_JMP_PAGE_SIZE)

static inline unsigned int tb_jmp_cache_hash_page(target_ulong pc) {
  target_ulong tmp;
  tmp = pc ^ (pc >> (TARGET_PAGE_BITS - TB_JMP_PAGE_BITS));
  return (tmp >> (TARGET_PAGE_BITS - TB_JMP_PAGE_BITS)) & TB_JMP_PAGE_MASK;
}

static inline unsigned int tb_jmp_cache_hash_func(target_ulong pc) {
  target_ulong tmp;
  tmp = pc ^ (pc >> (TARGET_PAGE_BITS - TB_JMP_PAGE_BITS));
  return (((tmp >> (TARGET_PAGE_BITS - TB_JMP_PAGE_BITS)) & TB_JMP_PAGE_MASK) |
          (tmp & TB_JMP_ADDR_MASK));
}

#else

/* In user-mode we can get better hashing because we do not have a TLB */
static inline unsigned int tb_jmp_cache_hash_func(target_ulong pc) {
  return (pc ^ (pc >> TB_JMP_CACHE_BITS)) & (TB_JMP_CACHE_SIZE - 1);
}

#endif /* CONFIG_SOFTMMU */

static inline uint32_t tb_hash_func(tb_page_addr_t phys_pc, target_ulong pc,
                                    uint32_t flags, uint32_t cf_mask,
                                    uint32_t trace_vcpu_dstate) {
  return qemu_xxhash7(phys_pc, pc, flags, cf_mask, trace_vcpu_dstate);
}

#endif /* end of include guard: TB_HASH_H_HPADT4RC */
