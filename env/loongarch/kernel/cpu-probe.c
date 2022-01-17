#include <asm/setup.h>
#include <linux/smp.h>

void cpu_probe(void) {
  unsigned int cpu = smp_processor_id();
  per_cpu_trap_init(cpu);
}
