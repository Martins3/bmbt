#include "lsenv.h"
#include "mem.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "shadow-stack.h"

/* global shadow_stack is defined here */
SS shadow_stack;

SS_ITEM* ss_pop(SS* ss) 
{
    ss->_ssi_current--;
    assert(ss->_ssi_current>ss->_ssi_first);
    return ss->_ssi_current;
}

SS_ITEM* ss_top(SS* ss) 
{
    assert(ss->_ssi_current>ss->_ssi_first);
    return ss->_ssi_current-1;
}

void ss_init(SS* ss) 
{
    if (option_shadow_stack)
        ss->_ssi_first = (SS_ITEM*) mm_calloc(800000, sizeof(SS_ITEM));
    else
        ss->_ssi_first = (SS_ITEM*) mm_calloc(800, sizeof(SS_ITEM));
    ss->_ssi_first->return_tb = (void*)0;
    ss->_ssi_first->x86_callee_addr = 0xbfffffff;
    ss->_ssi_first->x86_esp = 0xbfffffff;

    ss->_ssi_last = ss->_ssi_first + 800000;

    ss->_ssi_current = ss->_ssi_first+1;
}

void ss_duplicate(SS* ss, SS* from) 
{
    ss_init(ss);
    if(from->_ssi_first !=NULL)
        memcpy(ss->_ssi_first, from->_ssi_first, (from->_ssi_current-from->_ssi_first-1)*sizeof(SS_ITEM));
}

void ss_fini(SS* ss) 
{
    if (ss->_ssi_first)
        mm_free(ss->_ssi_first);
    ss->_ssi_first = 0;
    ss->_ssi_last = 0;
    ss->_ssi_current = 0;
}


void ss_push(SS* ss, ADDRX x86_esp, ADDRX x86_callee_addr, void *return_tb) 
{
    // 1. make sure ss have enough space
    if (ss->_ssi_current == ss->_ssi_last) {
        int curr_size = ss->_ssi_last - ss->_ssi_first;
        int new_size = curr_size << 1;
        ss->_ssi_first = (SS_ITEM*) mm_realloc(ss->_ssi_first, sizeof(SS_ITEM)*new_size);
        ss->_ssi_last = ss->_ssi_first + new_size;
        ss->_ssi_current = ss->_ssi_first + curr_size;
    }

    // 2. push one ss item
    ss->_ssi_current->x86_esp = x86_esp;
    ss->_ssi_current->x86_callee_addr = x86_callee_addr;
    ss->_ssi_current->return_tb = return_tb;
    ss->_ssi_current ++;
}


void ss_pop_till_find(SS* ss, ADDRX x86_esp) 
{
    // 1. find that ss item
    SS_ITEM *ssi = ss_top(ss);
    while (x86_esp > ssi->x86_esp && ss->_ssi_current > ss->_ssi_first+1) {
        ss_pop(ss);
        ssi = ss_top(ss);
    }
    if(x86_esp == ssi->x86_esp)
        ss_pop(ss);
    return;
}

void dump_shadow_stack(int debug_type) {
    switch (debug_type) {
        case 1: fprintf(stderr,"%dcall      :",debug_type); break;
        case 2: fprintf(stderr,"%dcallin    :",debug_type); break;
        case 3: fprintf(stderr,"%dret chain :",debug_type); break;
        case 4: fprintf(stderr,"%dret null  :",debug_type); break;
        case 5: fprintf(stderr,"%dret fail  :",debug_type); break;
        default: fprintf(stderr,"%derror    :",debug_type);
    }
    CPUArchState* env = (CPUArchState*)(lsenv->cpu_state);
    SS_ITEM* curr_ss = (SS_ITEM*)(env->vregs[4]);//ss
    for (SS_ITEM* p = shadow_stack._ssi_first + 1; p < curr_ss; p++) {
        fprintf(stderr, "%lx ", (uint64_t)p->x86_callee_addr);
    }
    fprintf(stderr, "\n");
}
