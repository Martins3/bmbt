#ifndef _LATX_IBTC_H_
#define _LATX_IBTC_H_

#define IBTC_BIT 10 
#define IBTC_SIZE (1U << IBTC_BIT)
#define IBTC_MASK (IBTC_SIZE - 1) 

#include "latx-types.h"

typedef struct ibtc_entry {
    uint64_t pc; /* hash key */
    void *tb;
} ibtc_entry;

void update_ibtc(ADDR pc, void *tb);
extern __thread ibtc_entry ibtc_table[IBTC_SIZE];
#endif
