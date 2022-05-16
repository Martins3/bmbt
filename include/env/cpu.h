#ifndef CPU_H_XDCOWUJZ
#define CPU_H_XDCOWUJZ

void cpu_wait(void);

typedef enum {
  FROM_INIT_RD,
  FROM_DISK,
} BootParameter;

BootParameter get_boot_parameter();

#endif /* end of include guard: CPU_H_XDCOWUJZ */
