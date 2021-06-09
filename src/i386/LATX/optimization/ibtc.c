#include "ibtc.h"
// #include "stdio.h"
#include "xtm-qemu-config.h"

ibtc_entry ibtc_table[IBTC_SIZE];
void update_ibtc(uint64_t pc, void *tb) {
#ifndef CONFIG_SOFTMMU
    int index = (int)(pc & IBTC_MASK);
    ibtc_table[index].pc = pc;
    ibtc_table[index].tb = tb;
    //fprintf(stderr, "ibtc[%d] = %p\n", index, tb);
#endif
}
