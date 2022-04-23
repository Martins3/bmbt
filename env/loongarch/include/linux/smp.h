#ifndef SMP_H_89FGT7EA
#define SMP_H_89FGT7EA
#include <linux/threads.h>
static inline int raw_smp_processor_id(void) { return 0; }
#define smp_processor_id() raw_smp_processor_id()
#define raw_smp_processor_id raw_smp_processor_id

// [BMBT_MTTCG 1]
/* The reverse map from sequential logical cpu number to cpu id.  */
extern int __cpu_logical_map[NR_CPUS];
#define cpu_logical_map(cpu) __cpu_logical_map[cpu]
#endif /* end of include guard: SMP_H_89FGT7EA */
