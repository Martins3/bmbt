#ifndef _X86TOMIPS_PROFILE_H_
#define _X86TOMIPS_PROFILE_H_
#define ETB_ARRAY_SIZE (1<<19)

#include "types.h"
#include "qemu-def.h"

void profile_generate(void);
void profile_dump(int top_n);

extern int64 context_switch_time;
extern int64 indirect_block_cnt;
extern int64 ibtc_hit_cnt;
extern int tb_num;
extern ETB *etb_array[ETB_ARRAY_SIZE];
#endif
