#ifndef SETUP_H_HSIHSW3K
#define SETUP_H_HSIHSW3K

#define VECSIZE 0x200

extern void set_handler(unsigned long offset, void *addr, unsigned long len);
extern void set_merr_handler(unsigned long offset, void *addr,
                             unsigned long len);

typedef void (*vi_handler_t)(int irq);
extern void set_vi_handler(int n, vi_handler_t addr);

extern unsigned long eentry;
extern unsigned long tlbrentry;
extern void cpu_cache_init(void);
extern void per_cpu_trap_init(int cpu);
extern void set_tlb_handler(void);

#endif /* end of include guard: SETUP_H_HSIHSW3K */
