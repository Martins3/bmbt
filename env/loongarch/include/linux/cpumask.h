#ifndef CPUMASK_H_QTH9015G
#define CPUMASK_H_QTH9015G

// TMP_TODO we will fix the weird header problem
#include "../../../include/qemu/bitmap.h"
#include <autoconf.h>
#include <linux/threads.h>

typedef struct cpumask {
  DECLARE_BITMAP(bits, NR_CPUS);
} cpumask_t;

#if NR_CPUS == 1
#define nr_cpu_ids 1U
#define for_each_cpu(cpu) for ((cpu) = 0; (cpu) < 1; (cpu)++)

#define for_each_possible_cpu(cpu) for_each_cpu((cpu))
#define for_each_online_cpu(cpu) for_each_cpu((cpu))
#define for_each_present_cpu(cpu) for_each_cpu((cpu))
#else
#error "BMBT Only support one CPU"
#endif

#endif /* end of include guard: CPUMASK_H_QTH9015G */
