#ifndef CONFIG_H_80Q9AENS
#define CONFIG_H_80Q9AENS

#define DMW_PABITS	48
#define LONGSIZE 8
#define PAGE_SHIFT 14
#define PAGESZ (1 << PAGE_SHIFT)
#define _THREAD_SIZE (PAGESZ * 4)

#ifdef __ASSEMBLY__
#define _CONST64_
#else
#define _CONST64_(x) x##l
#endif

#endif /* end of include guard: CONFIG_H_80Q9AENS */
