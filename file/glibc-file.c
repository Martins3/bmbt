#include <file.h>
#include <stdio.h>

long get_file_size(FILE *f) {
  long where, size;

  /* XXX: on Unix systems, using fstat() probably makes more sense */

  where = ftell(f);
  fseek(f, 0, SEEK_END);
  size = ftell(f);
  fseek(f, where, SEEK_SET);
  return size;
}

FILE *__wrap_fopen(const char *__restrict f, const char *__restrict f2) {
  return __real_fopen(f, f2);
}
size_t __wrap_fread(void *__restrict f, size_t f1, size_t f2,
                    FILE *__restrict f3) {
  return __real_fread(f, f1, f2, f3);
}
int __wrap_fclose(FILE *f) { return __real_fclose(f); }
int __wrap_fseek(FILE *f, long f1, int f2) { return __real_fseek(f, f1, f2); }
long __wrap_ftell(FILE *f) { return __real_ftell(f); }
