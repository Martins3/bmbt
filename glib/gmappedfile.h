#ifndef GMAPPEDFILE_H_6KSIXFR5
#define GMAPPEDFILE_H_6KSIXFR5

#include "gerror.h"
#include "glibconfig.h"
#include "gmem.h"
#include <assert.h> // for assert
#include <errno.h>  // for errno
#include <fcntl.h>  // for open
#include <stddef.h>
#include <stdio.h>    // for printf
#include <stdlib.h>   // for exit
#include <string.h>   // for stderr
#include <sys/mman.h> // for MAP_FAILED
#include <sys/stat.h> //for fstat
#include <unistd.h>   // for close

typedef struct _GMappedFile GMappedFile;

GMappedFile *g_mapped_file_new(const gchar *filename, gboolean writable,
                               GError **error);
gchar *g_mapped_file_get_contents(GMappedFile *file);
gsize g_mapped_file_get_length(GMappedFile *file);
void g_mapped_file_unref(GMappedFile *file);

#endif /* end of include guard: GMAPPEDFILE_H_6KSIXFR5 */
