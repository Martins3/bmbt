#ifndef CPUTLB_H_DK6FVCXL
#define CPUTLB_H_DK6FVCXL

#include "cpu-common.h"

#if !defined(CONFIG_USER_ONLY)
/* cputlb.c */
void tlb_protect_code(ram_addr_t ram_addr);
void tlb_unprotect_code(ram_addr_t ram_addr);
void tlb_flush_counts(size_t *full, size_t *part, size_t *elide);
#endif

#ifdef HAMT
void tlb_flush_vtlb_page_locked(CPUArchState *env, int mmu_idx,
                                target_ulong page);
bool tlb_hit_page_anyprot(CPUTLBEntry *tlb_entry, target_ulong page);
bool tlb_entry_is_empty(const CPUTLBEntry *te);
void copy_tlb_helper_locked(CPUTLBEntry *d, const CPUTLBEntry *s);
void tlb_n_used_entries_dec(CPUArchState *env, uintptr_t mmu_idx);
void tlb_n_used_entries_inc(CPUArchState *env, uintptr_t mmu_idx);
#endif

#endif /* end of include guard: CPUTLB_H_DK6FVCXL */
