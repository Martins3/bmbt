#ifndef __HAMT_H
#define __HAMT_H
#include <stdbool.h>
#include <stdint.h>

void hamt_invlpg_helper(uint32_t i386_addr);

void hamt_flush_all(void);

void alloc_target_addr_space(void);

void delete_pgtable(uint64_t cr3);

void hamt_set_context(uint64_t new_cr3);

extern uint64_t from_tlb_flush;
extern uint64_t from_tlb_flush_page_locked;
extern uint64_t from_by_mmuidx;
extern uint64_t from_reset_dirty;

#endif
