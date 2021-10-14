#ifndef _XTM_CROSS_PAGE_CHECK_H_
#define _XTM_CROSS_PAGE_CHECK_H_

#include "types.h"
/* code page table */

#define CODE_PAGE_TABLE_BITS 10
#define CODE_PAGE_TABLE_SIZE (1 << CODE_PAGE_TABLE_BITS)
#define CODE_PAGE_TABLE_MASK (CODE_PAGE_TABLE_SIZE - 1)

typedef struct {
    uint32_t gpa;
} cpt_entry_t;

typedef struct {
    cpt_entry_t cpt_e[CODE_PAGE_TABLE_SIZE];
    uint8_t cpt_v[CODE_PAGE_TABLE_SIZE];
} cpt_t;

void xtm_cpt_flush(void);
void xtm_cpt_insert_tb(void *tb);
void xtm_cpt_flush_page(uint32_t addr);

#endif
