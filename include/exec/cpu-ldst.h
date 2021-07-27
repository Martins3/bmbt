/*
 *  Software MMU support
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Generate inline load/store functions for all MMU modes (typically
 * at least _user and _kernel) as well as _data versions, for all data
 * sizes.
 *
 * Used by target op helpers.
 *
 * The syntax for the accessors is:
 *
 * load: cpu_ld{sign}{size}_{mmusuffix}(env, ptr)
 *
 * store: cpu_st{sign}{size}_{mmusuffix}(env, ptr, val)
 *
 * sign is:
 * (empty): for 32 and 64 bit sizes
 *   u    : unsigned
 *   s    : signed
 *
 * size is:
 *   b: 8 bits
 *   w: 16 bits
 *   l: 32 bits
 *   q: 64 bits
 *
 * mmusuffix is one of the generic suffixes "data" or "code", or
 * (for softmmu configs)  a target-specific MMU mode suffix as defined
 * in target cpu.h.
 */
#ifndef CPU_LDST_H
#define CPU_LDST_H
#include "../qemu/atomic.h"
#include "../qemu/bswap.h"
#include "cpu-all.h"

typedef target_ulong abi_ptr;
#define TARGET_ABI_FMT_ptr TARGET_ABI_FMT_lx

/* The memory helpers for tcg-generated code need tcg_target_long etc.  */
#include "../../src/tcg/tcg.h"

static inline target_ulong tlb_addr_write(const CPUTLBEntry *entry) {
#if TCG_OVERSIZED_GUEST
  return entry->addr_write;
#else
  return atomic_read(&entry->addr_write);
#endif
}

/* Find the TLB index corresponding to the mmu_idx + address pair.  */
static inline uintptr_t tlb_index(CPUArchState *env, uintptr_t mmu_idx,
                                  target_ulong addr) {
  uintptr_t size_mask = env_tlb(env)->f[mmu_idx].mask >> CPU_TLB_ENTRY_BITS;

  return (addr >> TARGET_PAGE_BITS) & size_mask;
}

static inline size_t tlb_n_entries(CPUArchState *env, uintptr_t mmu_idx) {
  return (env_tlb(env)->f[mmu_idx].mask >> CPU_TLB_ENTRY_BITS) + 1;
}

/* Find the TLB entry corresponding to the mmu_idx + address pair.  */
static inline CPUTLBEntry *tlb_entry(CPUArchState *env, uintptr_t mmu_idx,
                                     target_ulong addr) {
  return &env_tlb(env)->f[mmu_idx].table[tlb_index(env, mmu_idx, addr)];
}

#ifdef MMU_MODE0_SUFFIX
#define CPU_MMU_INDEX 0
#define MEMSUFFIX MMU_MODE0_SUFFIX
#define DATA_SIZE 1
#include "cpu_ldst_template.h"

#define DATA_SIZE 2
#include "cpu_ldst_template.h"

#define DATA_SIZE 4
#include "cpu_ldst_template.h"

#define DATA_SIZE 8
#include "cpu_ldst_template.h"
#undef CPU_MMU_INDEX
#undef MEMSUFFIX
#endif

#if (NB_MMU_MODES >= 2) && defined(MMU_MODE1_SUFFIX)
#define CPU_MMU_INDEX 1
#define MEMSUFFIX MMU_MODE1_SUFFIX
#define DATA_SIZE 1
#include "cpu_ldst_template.h"

#define DATA_SIZE 2
#include "cpu_ldst_template.h"

#define DATA_SIZE 4
#include "cpu_ldst_template.h"

#define DATA_SIZE 8
#include "cpu_ldst_template.h"
#undef CPU_MMU_INDEX
#undef MEMSUFFIX
#endif

#if (NB_MMU_MODES >= 3) && defined(MMU_MODE2_SUFFIX)

#define CPU_MMU_INDEX 2
#define MEMSUFFIX MMU_MODE2_SUFFIX
#define DATA_SIZE 1
#include "cpu_ldst_template.h"

#define DATA_SIZE 2
#include "cpu_ldst_template.h"

#define DATA_SIZE 4
#include "cpu_ldst_template.h"

#define DATA_SIZE 8
#include "cpu_ldst_template.h"
#undef CPU_MMU_INDEX
#undef MEMSUFFIX
#endif /* (NB_MMU_MODES >= 3) */

/* these access are slower, they must be as rare as possible */
#define CPU_MMU_INDEX (cpu_mmu_index(env, false))
#define MEMSUFFIX _data
#define DATA_SIZE 1
#include "cpu_ldst_template.h"

#define DATA_SIZE 2
#include "cpu_ldst_template.h"

#define DATA_SIZE 4
#include "cpu_ldst_template.h"

#define DATA_SIZE 8
#include "cpu_ldst_template.h"
#undef CPU_MMU_INDEX
#undef MEMSUFFIX

/*
 * Code access is deprecated in favour of translator_ld* functions
 * (see translator.h). However there are still users that need to
 * converted so for now these stay.
 */

#define CPU_MMU_INDEX (cpu_mmu_index(env, true))
#define MEMSUFFIX _code
#define SOFTMMU_CODE_ACCESS

#define DATA_SIZE 1
#include "cpu_ldst_template.h"

#define DATA_SIZE 2
#include "cpu_ldst_template.h"

#define DATA_SIZE 4
#include "cpu_ldst_template.h"

#define DATA_SIZE 8
#include "cpu_ldst_template.h"

#undef CPU_MMU_INDEX
#undef MEMSUFFIX
#undef SOFTMMU_CODE_ACCESS

/**
 * tlb_vaddr_to_host:
 * @env: CPUArchState
 * @addr: guest virtual address to look up
 * @access_type: 0 for read, 1 for write, 2 for execute
 * @mmu_idx: MMU index to use for lookup
 *
 * Look up the specified guest virtual index in the TCG softmmu TLB.
 * If we can translate a host virtual address suitable for direct RAM
 * access, without causing a guest exception, then return it.
 * Otherwise (TLB entry is for an I/O access, guest software
 * TLB fill required, etc) return NULL.
 */
void *tlb_vaddr_to_host(CPUArchState *env, abi_ptr addr,
                        MMUAccessType access_type, int mmu_idx);

#endif /* CPU_LDST_H */
