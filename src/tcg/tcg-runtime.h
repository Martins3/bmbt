#include "../../include/exec/helper-head.h"
#include "../i386/cpu.h"

DEF_HELPER_FLAGS_1(lookup_tb, TCG_CALL_NO_WG_SE, ptr, env)
