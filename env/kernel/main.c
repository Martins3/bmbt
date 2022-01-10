#include <kernel/addrspace.h>
#include <kernel/config.h>
#include <unistd.h>

unsigned long fw_arg0, fw_arg1, fw_arg2, fw_arg3;

unsigned char kernel_stack[_THREAD_SIZE];

void start_kernel() {
  getpid();
  for (;;) {
  }
}
