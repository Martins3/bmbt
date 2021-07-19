#ifndef REPLAY_H_HBHMKZQ3
#define REPLAY_H_HBHMKZQ3

#include <stdbool.h>

static inline bool replay_has_exception(void) {
  // FIXME
  return false;
}

// FIXME /home/maritns3/core/ld/x86-qemu-mips/build/qapi/qapi-types-misc.h
// it's kind of weird, defined in build/
// so how generated ?
typedef enum ReplayMode {
  REPLAY_MODE_NONE,
  REPLAY_MODE_RECORD,
  REPLAY_MODE_PLAY,
  REPLAY_MODE__MAX,
} ReplayMode;

extern ReplayMode replay_mode;

static inline bool replay_has_interrupt(void) {
  // FIXME
  return false;
}

static inline bool replay_interrupt(void) {
  // FIXME
  return false;
}

/*! Called by exception handler to write or read
    exception processing events. */
static inline bool replay_exception(void) {
  // FIXME
  return false;
}

#endif /* end of include guard: REPLAY_H_HBHMKZQ3 */
