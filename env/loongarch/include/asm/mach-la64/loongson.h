#ifndef LOONGSON_H_S3BK6ULO
#define LOONGSON_H_S3BK6ULO

extern void fw_init_env(void);
extern void fw_init_memory(void);

#define LOONGSON_REG_BASE 0x1fe00000
#define LOONGSON_REG_SIZE 0x00100000 /* 8K */
#define LOONGSON_REG_TOP (LOONGSON_REG_BASE + LOONGSON_REG_SIZE - 1)

#endif /* end of include guard: LOONGSON_H_S3BK6ULO */
