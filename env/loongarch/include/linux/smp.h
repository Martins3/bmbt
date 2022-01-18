#ifndef SMP_H_89FGT7EA
#define SMP_H_89FGT7EA
static inline int raw_smp_processor_id(void) { return 0; }
#define smp_processor_id() raw_smp_processor_id()
#define raw_smp_processor_id raw_smp_processor_id
#endif /* end of include guard: SMP_H_89FGT7EA */
