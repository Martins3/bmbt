#include "include/mem.h"
#include "include/xtm-qemu-config.h"

#ifdef CONFIG_XTM_MEMWATCH
#include "include/memwatch.h"
#endif

void *mm_malloc(int size)
{
    void *retval = malloc(size);
    lsassertm(retval != NULL, "dbt: cannot allocate memory (%d bytes)\n", size);
    return retval;
}

void *mm_calloc(int num, int size)
{
    void *retval = calloc(num, size);
    lsassertm(retval != NULL, "dbt: cannot allocate memory (%d*%d bytes)\n",
              num, size);
    return retval;
}

void *mm_realloc(void *ptr, int size)
{
    void *retval = realloc(ptr, size);
    lsassertm(retval != NULL, "dbt: cannot allocate memory (%d bytes)\n", size);
    return retval;
}

void mm_free(void *ptr)
{
    if (ptr) {
        free(ptr);
    }
}

void *mmi_palloc(int size, char *file_name, int line_num)
{
    void *retval = malloc(size);
    lsassertm(retval != NULL, "dbt: cannot allocate memory (%d bytes)\n", size);
    return retval;
}
