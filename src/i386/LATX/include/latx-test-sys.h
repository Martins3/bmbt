#ifndef _LATX_TEST_SYS_H_
#define _LATX_TEST_SYS_H_

void latx_test_sys_parse_options(void *opts);

void *latx_test_sys_alloc_tb(void *cpu, void **highwater);
void latx_test_sys_reset_cpu(void *cpu);
void *latx_test_sys_start(void *cpu);

int latx_test_sys_enabled(void);

#endif
