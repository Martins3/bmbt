#include <asm/addrspace.h>
#include <asm/bootinfo.h>

int fw_argc;
long *_fw_argv, *_fw_envp;

void fw_init_cmdline(void) {
  fw_argc = fw_arg0;
  _fw_argv = (long *)TO_CAC(fw_arg1);
  _fw_envp = (long *)fw_arg2;
}
