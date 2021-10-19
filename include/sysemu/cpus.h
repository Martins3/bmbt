#ifndef CPUS_H_XUGZP1FI
#define CPUS_H_XUGZP1FI

void resume_all_vcpus(void);
void qemu_init_cpu_loop(void);
void *qemu_tcg_rr_cpu_thread_fn(void *arg);

// code originally belongs to include/exec/cpu-common.h
void qemu_init_cpu_list(void);
#endif /* end of include guard: CPUS_H_XUGZP1FI */
