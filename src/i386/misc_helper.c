#include "../../include/exec/exec-all.h"
#include "../../include/exec/cpu-lsdt.h"
#include "../../include/fpu/softfloat.h"
#include "../../include/fpu/softfloat-helper.h"
#include "../../include/qemu/bswap.h"
#include "cpu.h"
#include "svm.h"
#include "LATX/x86tomips-config.h"
#include <stddef.h>
#include <string.h>
#include <math.h>


target_ulong helper_read_crN(CPUX86State *env, int reg)
{
    target_ulong val;

    cpu_svm_check_intercept_param(env, SVM_EXIT_READ_CR0 + reg, 0, GETPC());
    switch (reg) {
    default:
        val = env->cr[reg];
        break;
    case 8:
        // FIXME handle this later
        // if (!(env->hflags2 & HF2_VINTR_MASK)) {
            // val = cpu_get_apic_tpr(env_archcpu(env)->apic_state);
        // } else {
            // val = env->v_tpr;
        // }
        break;
    }
    return val;
}

void helper_write_crN(CPUX86State *env, int reg, target_ulong t0)
{
    cpu_svm_check_intercept_param(env, SVM_EXIT_WRITE_CR0 + reg, 0, GETPC());
    switch (reg) {
    case 0:
        cpu_x86_update_cr0(env, t0);
        break;
    case 3:
        cpu_x86_update_cr3(env, t0);
        break;
    case 4:
        cpu_x86_update_cr4(env, t0);
        break;
    case 8:
        // FIXME handle this later
        // if (!(env->hflags2 & HF2_VINTR_MASK)) {
            // qemu_mutex_lock_iothread();
            // cpu_set_apic_tpr(env_archcpu(env)->apic_state, t0);
            // qemu_mutex_unlock_iothread();
        // }
        // env->v_tpr = t0 & 0x0f;
        break;
    default:
        env->cr[reg] = t0;
        break;
    }
}
