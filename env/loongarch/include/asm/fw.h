#ifndef __ASM_FW_H_
#define __ASM_FW_H_

#include <asm/bootinfo.h>

extern int fw_argc;
extern long *_fw_argv, *_fw_envp;

#define fw_argv(index) ((char *)TO_CAC((long)_fw_argv[(index)]))
#define fw_envp(index) ((char *)TO_CAC((long)_fw_envp[(index)]))

extern void fw_init_cmdline(void);

#endif /* __ASM_FW_H_ */
