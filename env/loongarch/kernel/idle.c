#include <asm/loongarchregs.h>
extern void __cpu_wait(void);

void cpu_wait(void) { __cpu_wait(); }
