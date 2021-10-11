#ifndef CPUTLB_H_DK6FVCXL
#define CPUTLB_H_DK6FVCXL

#include "cpu-common.h"

#if !defined(CONFIG_USER_ONLY)
/* cputlb.c */
void tlb_protect_code(ram_addr_t ram_addr);
void tlb_unprotect_code(ram_addr_t ram_addr);
void tlb_flush_counts(size_t *full, size_t *part, size_t *elide);
#endif

#endif /* end of include guard: CPUTLB_H_DK6FVCXL */
