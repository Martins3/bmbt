#ifndef _LOCKSTEP_H_
#define _LOCKSTEP_H_

#include "../include/common.h"
#include "../x86tomips-config.h"

void vcpu_tb_exec(CPUX86State *env);
void lockstep_init(void);

#endif
