#ifndef _SHADOW_STACK_H_
#define _SHADOW_STACK_H_

#include "latx-types.h"
#include "qemu-def.h"

typedef struct  {
    ADDRX x86_esp;    // the esp after pushing the return address
    ADDRX x86_callee_addr;
    void *return_tb;
} SS_ITEM;

typedef struct {
    SS_ITEM *_ssi_current;  // the ssi is empty. (the prior ssi contains calling info)
    SS_ITEM *_ssi_last;     // the end of ssi array. (this ssi is out of range; the prior one is available)
    SS_ITEM *_ssi_first;    // the start of ssi array (this ssi is valid)
} SS;

extern SS shadow_stack;

void ss_init(SS* ss);
void ss_duplicate(SS* ss, SS* from);
void ss_fini(SS* ss);
void ss_push(SS* ss, ADDRX x86_esp, ADDRX x86_callee_addr, void *return_tb);
SS_ITEM* ss_pop(SS* ss);
SS_ITEM* ss_top(SS* ss);
void ss_pop_till_find(SS* ss, ADDRX x86_esp);
void dump_shadow_stack(int debug_type);

#endif
