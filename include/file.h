#ifndef FILE_H_JVDDTQK4
#define FILE_H_JVDDTQK4

#include <stdio.h>
long get_file_size(FILE *f);

FILE *__real_fopen(const char *__restrict f, const char *__restrict f2);
size_t __real_fread(void *__restrict f, size_t f1, size_t f2,
                    FILE *__restrict f3);
int __real_fclose(FILE *f);
int __real_fseek(FILE *f, long f1, int f2);
long __real_ftell(FILE *f);

#endif /* end of include guard: FILE_H_JVDDTQK4 */
