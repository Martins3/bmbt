#ifndef PAGE_H_WY7U6KLT
#define PAGE_H_WY7U6KLT

#include <asm/loongarchregs.h>
#include <linux/const.h>

// TMP_TODO : 这个文件在 asm 下面的
#ifdef CONFIG_PAGE_SIZE_4KB
#define PAGE_SHIFT 12
#endif
#ifdef CONFIG_PAGE_SIZE_16KB
#define PAGE_SHIFT 14
#endif
#ifdef CONFIG_PAGE_SIZE_64KB
#define PAGE_SHIFT 16
#endif
#define PAGE_SIZE (_AC(1, UL) << PAGE_SHIFT)
#define PAGE_MASK (~(PAGE_SIZE - 1))
#define PAGE_OFFSET_MASK (PAGE_SIZE - 1)

#endif /* end of include guard: PAGE_H_WY7U6KLT */
