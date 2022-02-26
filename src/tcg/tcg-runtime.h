#include "../../include/exec/helper-head.h"
#include "../i386/cpu.h"

DEF_HELPER_FLAGS_1(lookup_tb, TCG_CALL_NO_WG_SE, ptr, env)

#ifdef CONFIG_SOFTMMU
DEF_HELPER_FLAGS_5(atomic_cmpxchgb, TCG_CALL_NO_WG, i32, env, tl, i32, i32, i32)
DEF_HELPER_FLAGS_5(atomic_cmpxchgw_be, TCG_CALL_NO_WG, i32, env, tl, i32, i32,
                   i32)
DEF_HELPER_FLAGS_5(atomic_cmpxchgw_le, TCG_CALL_NO_WG, i32, env, tl, i32, i32,
                   i32)
DEF_HELPER_FLAGS_5(atomic_cmpxchgl_be, TCG_CALL_NO_WG, i32, env, tl, i32, i32,
                   i32)
DEF_HELPER_FLAGS_5(atomic_cmpxchgl_le, TCG_CALL_NO_WG, i32, env, tl, i32, i32,
                   i32)
#ifdef CONFIG_ATOMIC64
DEF_HELPER_FLAGS_5(atomic_cmpxchgq_be, TCG_CALL_NO_WG, i64, env, tl, i64, i64,
                   i32)
DEF_HELPER_FLAGS_5(atomic_cmpxchgq_le, TCG_CALL_NO_WG, i64, env, tl, i64, i64,
                   i32)
#endif

#ifdef CONFIG_ATOMIC64
#define GEN_ATOMIC_HELPERS(NAME)                                               \
  DEF_HELPER_FLAGS_4(glue(glue(atomic_, NAME), b), TCG_CALL_NO_WG, i32, env,   \
                     tl, i32, i32)                                             \
  DEF_HELPER_FLAGS_4(glue(glue(atomic_, NAME), w_le), TCG_CALL_NO_WG, i32,     \
                     env, tl, i32, i32)                                        \
  DEF_HELPER_FLAGS_4(glue(glue(atomic_, NAME), w_be), TCG_CALL_NO_WG, i32,     \
                     env, tl, i32, i32)                                        \
  DEF_HELPER_FLAGS_4(glue(glue(atomic_, NAME), l_le), TCG_CALL_NO_WG, i32,     \
                     env, tl, i32, i32)                                        \
  DEF_HELPER_FLAGS_4(glue(glue(atomic_, NAME), l_be), TCG_CALL_NO_WG, i32,     \
                     env, tl, i32, i32)                                        \
  DEF_HELPER_FLAGS_4(glue(glue(atomic_, NAME), q_le), TCG_CALL_NO_WG, i64,     \
                     env, tl, i64, i32)                                        \
  DEF_HELPER_FLAGS_4(glue(glue(atomic_, NAME), q_be), TCG_CALL_NO_WG, i64,     \
                     env, tl, i64, i32)
#else
#define GEN_ATOMIC_HELPERS(NAME)                                               \
  DEF_HELPER_FLAGS_4(glue(glue(atomic_, NAME), b), TCG_CALL_NO_WG, i32, env,   \
                     tl, i32, i32)                                             \
  DEF_HELPER_FLAGS_4(glue(glue(atomic_, NAME), w_le), TCG_CALL_NO_WG, i32,     \
                     env, tl, i32, i32)                                        \
  DEF_HELPER_FLAGS_4(glue(glue(atomic_, NAME), w_be), TCG_CALL_NO_WG, i32,     \
                     env, tl, i32, i32)                                        \
  DEF_HELPER_FLAGS_4(glue(glue(atomic_, NAME), l_le), TCG_CALL_NO_WG, i32,     \
                     env, tl, i32, i32)                                        \
  DEF_HELPER_FLAGS_4(glue(glue(atomic_, NAME), l_be), TCG_CALL_NO_WG, i32,     \
                     env, tl, i32, i32)
#endif /* CONFIG_ATOMIC64 */

#else
  //
#endif /* CONFIG_SOFTMMU */

GEN_ATOMIC_HELPERS(fetch_add)
GEN_ATOMIC_HELPERS(fetch_and)
GEN_ATOMIC_HELPERS(fetch_or)
GEN_ATOMIC_HELPERS(fetch_xor)

GEN_ATOMIC_HELPERS(xchg)
