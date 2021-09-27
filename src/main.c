#include "../include/qemu/main-loop.h"

CPUState *init_qemu();

int main() {
  CPUState *cpu = init_qemu();
  qemu_tcg_rr_cpu_thread_fn(cpu);
  return 0;
}
