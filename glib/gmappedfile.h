#ifndef GMAPPEDFILE_H_6O0REWYT
#define GMAPPEDFILE_H_6O0REWYT
#include "glibconfig.h"
#include "gmem.h"
#include <assert.h>
#include <stdio.h>

typedef struct _GMappedFile GMappedFile;
typedef struct _GError GError;
struct _GError {};
struct _GMappedFile {
  gchar *contents;
  gsize length;
  FILE *f;
};

/**
 * g_mapped_file_new:
 * @filename: (type filename): The path of the file to load, in the GLib
 *     filename encoding
 * @writable: whether the mapping should be writable
 * @error: return location for a #GError, or %NULL
 *
 * Maps a file into memory. On UNIX, this is using the mmap() function.
 *
 * If @writable is %TRUE, the mapped buffer may be modified, otherwise
 * it is an error to modify the mapped buffer. Modifications to the buffer
 * are not visible to other processes mapping the same file, and are not
 * written back to the file.
 *
 * Note that modifications of the underlying file might affect the contents
 * of the #GMappedFile. Therefore, mapping should only be used if the file
 * will not be modified, or if all modifications of the file are done
 * atomically (e.g. using g_file_set_contents()).
 *
 * If @filename is the name of an empty, regular file, the function
 * will successfully return an empty #GMappedFile. In other cases of
 * size 0 (e.g. device files such as /dev/null), @error will be set
 * to the #GFileError value #G_FILE_ERROR_INVAL.
 *
 * Returns: a newly allocated #GMappedFile which must be unref'd
 *    with g_mapped_file_unref(), or %NULL if the mapping failed.
 *
 * Since: 2.8
 */
static inline GMappedFile *
g_mapped_file_new(const gchar *filename, gboolean writable, GError **error) {
  FILE *f = fopen(filename, "rb");

  // this is duplicated with get_file_size in x86.c, but we will clear code
  // related with file operation later.
  long where, size;
  where = ftell(f);
  fseek(f, 0, SEEK_END);
  size = ftell(f);
  fseek(f, where, SEEK_SET);

  GMappedFile *file = g_new0(GMappedFile, 1);
  file->contents = g_malloc(size);
  file->length = size;
  file->f = f;
  assert(file->contents != NULL);
  assert(fread(file->contents, 1, size, f) == size);
  fclose(f);

  return file;
}

/**
 * g_mapped_file_get_contents:
 * @file: a #GMappedFile
 *
 * Returns the contents of a #GMappedFile.
 *
 * Note that the contents may not be zero-terminated,
 * even if the #GMappedFile is backed by a text file.
 *
 * If the file is empty then %NULL is returned.
 *
 * Returns: the contents of @file, or %NULL.
 *
 * Since: 2.8
 */
static inline gchar *g_mapped_file_get_contents(GMappedFile *file) {
  return file->contents;
}

/**
 * g_mapped_file_get_length:
 * @file: a #GMappedFile
 *
 * Returns the length of the contents of a #GMappedFile.
 *
 * Returns: the length of the contents of @file.
 *
 * Since: 2.8
 */
static inline gsize g_mapped_file_get_length(GMappedFile *file) {
  assert(file != NULL);
  return file->length;
}

#endif /* end of include guard: GMAPPEDFILE_H_6O0REWYT */
