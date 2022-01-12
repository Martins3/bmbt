#include <asm/addrspace.h>
#include <asm/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unsigned long fw_arg0, fw_arg1, fw_arg2, fw_arg3;

unsigned char kernel_stack[_THREAD_SIZE];

void start_kernel() {
  for (int i = 0; i < 10; ++i) {
    printf("huxueshi:%s %d \n", __FUNCTION__, i);
  }
  size_t sz = 100;
  char *buf = (char *)malloc(sz);
  memset(buf, 0, sz);
  sprintf(buf, "%s %x", "hi", 12);
  exit(0);
}
