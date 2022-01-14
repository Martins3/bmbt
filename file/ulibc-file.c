#include <assert.h>
#include <qemu/compiler.h>
#include <qemu/units.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// https://stackoverflow.com/questions/4158900/embedding-resources-in-executable-using-gcc

// #define DEBUG_UFILE

typedef struct {
  FILE *_f;
  const char filename[200];
  char *start;
  const char *end;
  int pos;
} ULIBC_FILE;

extern char _binary_image_bios_bin_start[];
extern char _binary_image_bios_bin_end[];

extern char _binary_image_initrd_bin_start[];
extern char _binary_image_initrd_bin_end[];

extern char _binary_image_bzImage_bin_start[];
extern char _binary_image_bzImage_bin_end[];

extern char _binary_image_linuxboot_dma_bin_start[];
extern char _binary_image_linuxboot_dma_bin_end[];

static ULIBC_FILE ulibc_files[] = {
    {
        ._f = (FILE *)0,
        .filename = "image/stub.bin",
        .start = NULL,
        .end = NULL,
    },
    {
        ._f = (FILE *)1,
        .filename = "image/bios.bin",
        .start = _binary_image_bios_bin_start,
        .end = _binary_image_bios_bin_end,
    },
    {
        ._f = (FILE *)2,
        .filename = "image/initrd.bin",
        .start = _binary_image_initrd_bin_start,
        .end = _binary_image_initrd_bin_end,
    },
    {
        ._f = (FILE *)3,
        .filename = "image/bzImage.bin",
        .start = _binary_image_bzImage_bin_start,
        .end = _binary_image_bzImage_bin_end,
    },
    {
        ._f = (FILE *)4,
        .filename = "image/linuxboot_dma.bin",
        .start = _binary_image_linuxboot_dma_bin_start,
        .end = _binary_image_linuxboot_dma_bin_end,
    }};

const int ULIBC_FILE_NUM = sizeof(ulibc_files) / sizeof(ULIBC_FILE);

static inline ULIBC_FILE *get_uflie(FILE *f) {
  size_t idx = (size_t)((void *)f);
  assert(idx < ULIBC_FILE_NUM);
  return ulibc_files + idx;
}

long get_file_size(FILE *f) {
  ULIBC_FILE *uf = get_uflie(f);
  long sz = uf->end - uf->start;
  assert(sz != 0);
  return sz;
}

FILE *__wrap_fopen(const char *__restrict filename, const char *__restrict f2) {
#ifdef DEBUG_UFILE
  printf("huxueshi:%s %s\n", __FUNCTION__, filename);
#endif
  for (size_t i = 0; i < ULIBC_FILE_NUM; ++i) {
    ULIBC_FILE *uf = get_uflie((FILE *)i);
    if (strcmp(filename, uf->filename) == 0) {
      printf("idx=%ld \n", i);
      return uf->_f;
    }
  }
  assert(false);
}

size_t __wrap_fread(void *__restrict f, size_t size, size_t memmb,
                    FILE *__restrict f3) {
  assert(size == 1);
  ULIBC_FILE *uf = get_uflie((FILE *)f3);
  memcpy(f, uf->start + uf->pos, memmb);
  uf->pos += memmb;
  return memmb;
}
int __wrap_fclose(FILE *f) { return 0; }

int __wrap_fseek(FILE *f, long offset, int whence) {
  ULIBC_FILE *uf = get_uflie((FILE *)f);
  if (offset == 0 && whence == SEEK_SET) {
    uf->pos = 0;
  } else {
    // this is hard coded for x86.c:x86_load_linux
    assert(false);
  }
  return 0;
}
