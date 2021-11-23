#include "../../include/qemu/log.h"

// [interface 29]
FILE *qemu_logfile;
int qemu_loglevel;

FILE *get_logfile(const char *f) {
#ifdef BUILD_ON_LOONGSON
  char filename[256] = "build_loongson/";
#else
  char filename[256] = "/tmp/build_x86/";
#endif
  strcat(filename, f);
  FILE *file = fopen(filename, "w");
  duck_check(file != NULL);
  return file;
}

/* Return the number of characters emitted.  */
int qemu_log(const char *fmt, ...) {
  int ret = 0;
  if (qemu_logfile) {
    va_list ap;
    va_start(ap, fmt);
    ret = vfprintf(qemu_logfile, fmt, ap);
    va_end(ap);

    /* Don't pass back error results.  */
    if (ret < 0) {
      ret = 0;
    }
  }
  return ret;
}

/* enable or disable low levels log */
void qemu_set_log(int log_flags) {
  qemu_loglevel = log_flags;
  qemu_logfile = get_logfile("bmbt.log");
  qemu_log("start loging\n");
}

void log_cpu_state(CPUState *cpu, int flags) {
  if (qemu_logfile) {
    cpu_dump_state(cpu, qemu_logfile, flags);
  }
}

/* Returns true if addr is in our debug filter or no filter defined
 */
bool qemu_log_in_addr_range(uint64_t addr) { return true; }

/*
 * Print like vprintf().
 * Print to current monitor if we have one, else to stdout.
 */
int qemu_vprintf(const char *fmt, va_list ap) { return vprintf(fmt, ap); }

/*
 * Print like printf().
 * Print to current monitor if we have one, else to stdout.
 */
int qemu_printf(const char *fmt, ...) {
  va_list ap;
  int ret;

  va_start(ap, fmt);
  ret = qemu_vprintf(fmt, ap);
  va_end(ap);
  return ret;
}

/*
 * Print like vfprintf()
 * Print to @stream if non-null, else to current monitor.
 */
int qemu_vfprintf(FILE *stream, const char *fmt, va_list ap) {
  return vfprintf(stream, fmt, ap);
}

/*
 * Print like fprintf().
 * Print to @stream if non-null, else to current monitor.
 */
int qemu_fprintf(FILE *stream, const char *fmt, ...) {
  va_list ap;
  int ret;

  va_start(ap, fmt);
  ret = qemu_vfprintf(stream, fmt, ap);
  va_end(ap);
  return ret;
}
