#ifndef REPLAY_H_HBHMKZQ3
#define REPLAY_H_HBHMKZQ3

#include <stdbool.h>

typedef enum ReplayMode {
  REPLAY_MODE_NONE,
  REPLAY_MODE_RECORD,
  REPLAY_MODE_PLAY,
  REPLAY_MODE__MAX,
} ReplayMode;

extern ReplayMode replay_mode;

static inline bool replay_has_interrupt(void) { return false; }

static inline bool replay_has_exception(void) { return false; }

static inline bool replay_interrupt(void) { return true; }

/*! Called by exception handler to write or read
    exception processing events. */
static inline bool replay_exception(void) { return true; }

#endif /* end of include guard: REPLAY_H_HBHMKZQ3 */
