#ifndef _X86TOMIPS_PROFILE_H_
#define _X86TOMIPS_PROFILE_H_

#include "latx-types.h"

void profile_generate(void);
void profile_dump(int top_n);

/* interact with latx-config.c */
extern int64 context_switch_time;
/* interact with translate.c */
extern int64 ibtc_hit_cnt;
#endif
