#include "gmappedfile.h"
#include "glibconfig.h"

#ifndef _O_BINARY
#define _O_BINARY 0
#endif

/**
 * GMappedFile:
 *
 * The #GMappedFile represents a file mapping created with
 * g_mapped_file_new(). It has only private members and should
 * not be accessed directly.
 */

struct _GMappedFile {
  gchar *contents;
  gsize length;
  gpointer free_func;
  int ref_count;
#ifdef G_OS_WIN32
  HANDLE mapping;
#endif
};

static void g_mapped_file_destroy(GMappedFile *file) {
  if (file->length) {
#ifdef HAVE_MMAP
    munmap(file->contents, file->length);
#endif
#ifdef G_OS_WIN32
    UnmapViewOfFile(file->contents);
    CloseHandle(file->mapping);
#endif
  }

#ifdef BMBT
  g_slice_free(GMappedFile, file);
#endif
  g_free(GMappedFile);
}

/**
 * g_mapped_file_unref:
 * @file: a #GMappedFile
 *
 * Decrements the reference count of @file by one.  If the reference count
 * drops to 0, unmaps the buffer of @file and frees it.
 *
 * It is safe to call this function from any thread.
 *
 * Since 2.22
 **/
void g_mapped_file_unref(GMappedFile *file) {
  assert(file != NULL);

  if (g_atomic_int_dec_and_test(&file->ref_count))
    g_mapped_file_destroy(file);
}

static GMappedFile *mapped_file_new_from_fd(int fd, gboolean writable,
                                            const gchar *filename,
                                            GError **error) {
  GMappedFile *file;
  struct stat st;

  // the full g_slice_new function is complete and can't port almostly,
  // so use the simplified function.
  // you can find the full implement of g_slice_new in gslice.c.
  file = g_slice_alloc(GMappedFile);
  file->ref_count = 1;
  file->free_func = g_mapped_file_destroy;

  if (fstat(fd, &st) == -1) {
#ifdef BMBT
    int save_errno = errno;
    gchar *display_filename =
        filename ? g_filename_display_name(filename) : NULL;

    g_set_error(
        error, G_FILE_ERROR, g_file_error_from_errno(save_errno),
        _("Failed to get attributes of file “%s%s%s%s”: fstat() failed: %s"),
        display_filename ? display_filename : "fd",
        display_filename ? "' " : "", display_filename ? display_filename : "",
        display_filename ? "'" : "", g_strerror(save_errno));
    g_free(display_filename);
#endif
    fprintf(stderr, "gmappedfile: fstat (fd, &st) == -1: %s\n",
            strerror(errno));
    goto out;
  }

  /* mmap() on size 0 will fail with EINVAL, so we avoid calling mmap()
   * in that case -- but only if we have a regular file; we still want
   * attempts to mmap a character device to fail, for example.
   */
  if (st.st_size == 0 && S_ISREG(st.st_mode)) {
    file->length = 0;
    file->contents = NULL;
    return file;
  }

  file->contents = MAP_FAILED;

#ifdef HAVE_MMAP
  if (sizeof(st.st_size) > sizeof(gsize) && st.st_size > (off_t)G_MAXSIZE) {
    errno = EINVAL;
  } else {
    file->length = (gsize)st.st_size;
    file->contents = (gchar *)mmap(
        NULL, file->length, writable ? PROT_READ | PROT_WRITE : PROT_READ,
        MAP_PRIVATE, fd, 0);
  }
#endif
#ifdef G_OS_WIN32
  file->length = st.st_size;
  file->mapping =
      CreateFileMapping((HANDLE)_get_osfhandle(fd), NULL,
                        writable ? PAGE_WRITECOPY : PAGE_READONLY, 0, 0, NULL);
  if (file->mapping != NULL) {
    file->contents = MapViewOfFile(
        file->mapping, writable ? FILE_MAP_COPY : FILE_MAP_READ, 0, 0, 0);
    if (file->contents == NULL) {
      file->contents = MAP_FAILED;
      CloseHandle(file->mapping);
      file->mapping = NULL;
    }
  }
#endif

  if (file->contents == MAP_FAILED) {
#ifdef BMBT
    int save_errno = errno;
    gchar *display_filename =
        filename ? g_filename_display_name(filename) : NULL;

    g_set_error(error, G_FILE_ERROR, g_file_error_from_errno(save_errno),
                _("Failed to map %s%s%s%s: mmap() failed: %s"),
                display_filename ? display_filename : "fd",
                display_filename ? "' " : "",
                display_filename ? display_filename : "",
                display_filename ? "'" : "", g_strerror(save_errno));
    g_free(display_filename);
#endif
    fprintf(stderr, "gmappedfile: file->contents == MAP_FAILED: %s\n",
            strerror(errno));
    goto out;
  }

  return file;

out:
#ifdef BMBT
  g_slice_free(GMappedFile, file);
#endif
  g_free(GMappedFile);

  return NULL;
}

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
GMappedFile *g_mapped_file_new(const gchar *filename, gboolean writable,
                               GError **error) {
  GMappedFile *file;
  int fd;

// g_open is open in glibc，we use assert rather then g_error
#ifdef BMBT
  g_return_val_if_fail(filename != NULL, NULL);
  g_return_val_if_fail(!error || *error == NULL, NULL);

  fd = g_open(filename, (writable ? O_RDWR : O_RDONLY) | _O_BINARY, 0);
  if (fd == -1) {
    int save_errno = errno;
    gchar *display_filename = g_filename_display_name(filename);

    g_set_error(error, G_FILE_ERROR, g_file_error_from_errno(save_errno),
                _("Failed to open file “%s”: open() failed: %s"),
                display_filename, g_strerror(save_errno));
    g_free(display_filename);
    return NULL;
  }
#endif
  assert(filename != NULL);

  fd = open(filename, (writable ? O_RDWR : O_RDONLY) | _O_BINARY, 0);
  if (!fd) {
    fprintf(stderr, "gmappedfile: could not open file '%s': %s\n", filename,
            strerror(errno));
    exit(1);
  }
  file = mapped_file_new_from_fd(fd, writable, filename, error);

  close(fd);

  return file;
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
gsize g_mapped_file_get_length(GMappedFile *file) {
  assert(file != NULL);

  return file->length;
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
gchar *g_mapped_file_get_contents(GMappedFile *file) {
  assert(file != NULL);

  return file->contents;
}
