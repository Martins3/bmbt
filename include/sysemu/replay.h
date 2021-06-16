#ifndef REPLAY_H_HBHMKZQ3
#define REPLAY_H_HBHMKZQ3
// FIXME I'm considering implement record and replay or not

bool replay_has_exception(void);


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

bool replay_has_interrupt(void);

bool replay_interrupt(void);

#endif /* end of include guard: REPLAY_H_HBHMKZQ3 */
