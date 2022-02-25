#include "include/env.h"
#include "include/shadow-stack.h"
#include "x86tomips-options.h"
#include "include/profile.h"

#ifdef BMBT
__thread ENV *lsenv;
#else
ENV *lsenv;
#endif

QHT * etb_cache_qht;

#ifndef CONFIG_SOFTMMU
static QHT etb_cache_qht_real;
#endif
SS ss;//shadow stack


extern void xtm_capstone_init(void);

void x86_to_mips_init(void)
{
    context_switch_bt_to_native = 0;
    context_switch_native_to_bt_ret_0 = 0;
    context_switch_native_to_bt = 0;

    native_rotate_fpu_by = 0;

    options_init();
    xtm_capstone_init();

#ifndef CONFIG_SOFTMMU
    etb_cache_qht = &etb_cache_qht_real;
    etb_cache_qht_init();
    ss_init(&ss);//init shadow stack
#endif

#if defined(CONFIG_XTM_LOCKSTEP) && defined(CONFIG_SOFTMMU)
    lockstep_init();
#endif
}

void x86tomips_fini(void)
{
#ifndef CONFIG_SOFTMMU
    /* This func is called in user-mode only */
    if (option_profile) {
        profile_generate();
        profile_dump(10);
    }
#else
    lsassertm(0, "env.c::x86tomips_fini() should not be executed in system mode.\n");
#endif
}
