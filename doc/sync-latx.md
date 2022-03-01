## latx 在主线地方的修改

增加了两个函数:
accel/tcg/cputlb.c:2043:#if defined(TARGET_I386) && defined(CONFIG_LATX)
accel/tcg/cputlb.c:2534:#if defined(TARGET_I386) && defined(CONFIG_LATX)

主要是处理 sigint
accel/tcg/tcg-accel-ops.c:43:#if defined(CONFIG_LATX)
accel/tcg/tcg-accel-ops.c:86:#if defined(CONFIG_SOFTMMU) && defined(CONFIG_LATX)
accel/tcg/tcg-accel-ops.c:110:#if defined(CONFIG_LATX)
accel/tcg/tcg-accel-ops.c:115:#endif /* CONFIG_LATX */

解析 option
softmmu/vl.c:128:#ifdef CONFIG_LATX
softmmu/vl.c:2665:#ifdef CONFIG_LATX
softmmu/vl.c:2715:#ifdef CONFIG_LATX

文件本身是处理 x86_cpu_synchronize_from_tb 之类的 sigint
target/i386/tcg/tcg-cpu.c:31:#if defined(CONFIG_LATX) && defined(CONFIG_USER_ONLY)
target/i386/tcg/tcg-cpu.c:65:#if defined(CONFIG_LATX) && defined(CONFIG_SOFTMMU)

关于 tb_find
accel/tcg/cpu-exec.c:45:#if defined(CONFIG_SOFTMMU) && defined(CONFIG_LATX)
accel/tcg/cpu-exec.c:154:#ifdef CONFIG_LATX
accel/tcg/cpu-exec.c:203:#ifdef CONFIG_LATX_DEBUG
accel/tcg/cpu-exec.c:206:#ifdef CONFIG_LATX
accel/tcg/cpu-exec.c:230:#ifdef CONFIG_LATX_DEBUG
accel/tcg/cpu-exec.c:436:#ifdef CONFIG_LATX
accel/tcg/cpu-exec.c:462:#ifdef CONFIG_LATX
accel/tcg/cpu-exec.c:507:#if defined(CONFIG_SOFTMMU) && defined(CONFIG_LATX)
accel/tcg/cpu-exec.c:519:#ifdef CONFIG_LATX
accel/tcg/cpu-exec.c:897:#ifdef CONFIG_LATX_DEBUG

关于 cpu_loop_exit
accel/tcg/cpu-exec-common.c:25:#if defined(CONFIG_LATX) && defined(CONFIG_SOFTMMU)
accel/tcg/cpu-exec-common.c:73:#if defined(CONFIG_LATX) && defined(CONFIG_SOFTMMU)

因为需要剔除掉 latx 的内容:
accel/tcg/translate-all.c:67:#if defined(CONFIG_LATX)
accel/tcg/translate-all.c:455:#if !defined(CONFIG_LATX) || !defined(CONFIG_SOFTMMU)
accel/tcg/translate-all.c:498:#if !defined(CONFIG_LATX) || !defined(CONFIG_SOFTMMU)
accel/tcg/translate-all.c:1359:#elif defined(CONFIG_SOFTMMU) && defined(__loongarch__) && defined(CONFIG_LATX)
accel/tcg/translate-all.c:1361:#elif defined(__mips__) || defined (__loongarch__) && defined (CONFIG_LATX)
accel/tcg/translate-all.c:1403:#ifdef CONFIG_LATX
accel/tcg/translate-all.c:1409:#if defined(CONFIG_SOFTMMU) && defined(CONFIG_LATX)
accel/tcg/translate-all.c:2040:#ifdef CONFIG_LATX
accel/tcg/translate-all.c:2299:#ifdef CONFIG_LATX
accel/tcg/translate-all.c:2360:#if defined(CONFIG_LATX) && defined(CONFIG_SOFTMMU)
accel/tcg/translate-all.c:2366:#if defined(CONFIG_LATX) && defined(CONFIG_SOFTMMU)
accel/tcg/translate-all.c:2392:#ifndef CONFIG_LATX
accel/tcg/translate-all.c:2406:#ifndef CONFIG_LATX
accel/tcg/translate-all.c:2441:#ifndef CONFIG_LATX
accel/tcg/translate-all.c:2621:#if defined(CONFIG_LATX) && defined(CONFIG_SOFTMMU)
accel/tcg/translate-all.c:3470:#if defined(CONFIG_SOFTMMU) && defined(CONFIG_LATX)

主要关于 CPUX86State 的定义
target/i386/cpu.h:137:#if defined(CONFIG_LATX) && defined(CONFIG_SOFTMMU)
target/i386/cpu.h:1179:#if defined(CONFIG_LATX) && defined(CONFIG_SOFTMMU) && defined(TARGET_X86_64)
target/i386/cpu.h:1424:#ifdef CONFIG_LATX
target/i386/cpu.h:1516:#ifndef CONFIG_LATX
target/i386/cpu.h:1607:#ifndef CONFIG_LATX
target/i386/cpu.h:1714:#ifdef CONFIG_LATX

在函数 tb_target_set_jmp_target 中
tcg/loongarch/tcg-target.c.inc:1791:#ifdef CONFIG_LATX
tcg/loongarch/tcg-target.c.inc:1937:#if defined(CONFIG_SOFTMMU) && defined(CONFIG_LATX)
tcg/loongarch/tcg-target.c.inc:1982:#if defined(CONFIG_SOFTMMU) && defined(CONFIG_LATX)

主要是 sigint 和 latx_lsenv_init
accel/tcg/tcg-accel-ops-rr.c:39:#if defined(CONFIG_LATX)
accel/tcg/tcg-accel-ops-rr.c:56:#if defined(CONFIG_LATX)
accel/tcg/tcg-accel-ops-rr.c:60:#endif /* CONFIG_LATX */
accel/tcg/tcg-accel-ops-rr.c:189:#if defined(CONFIG_LATX)
accel/tcg/tcg-accel-ops-rr.c:237:#ifdef CONFIG_LATX
accel/tcg/tcg-accel-ops-mttcg.c:66:#if defined(CONFIG_LATX)

在 x86_cpu_reset 中 `env->tb_jmp_cache_ptr = s->tb_jmp_cache`
target/i386/cpu.c:67:#if defined(CONFIG_SOFTMMU) && defined(CONFIG_LATX)
target/i386/cpu.c:6256:#ifdef CONFIG_LATX

ExtraBlock 和 TranslationBlock
include/exec/exec-all.h:23:#ifdef CONFIG_LATX
include/exec/exec-all.h:30:#if defined(CONFIG_LATX_FLAG_PATTERN) || defined(CONFIG_LATX_FLAG_REDUCTION)
include/exec/exec-all.h:33:#ifdef CONFIG_LATX_FLAG_REDUCTION
include/exec/exec-all.h:539:#ifdef CONFIG_LATX

用于注销掉 tcg_qemu_tb_exec 的赋值，以及增加两个 helper
tcg/tcg.c:166:#ifndef CONFIG_LATX
tcg/tcg.c:1234:#ifndef CONFIG_LATX
include/tcg/tcg.h:1301:#elif defined CONFIG_LATX
include/tcg/tcg.h:1387:#if defined(TARGET_I386) && defined(CONFIG_LATX)

因为 restore_state_to_opc
target/i386/tcg/translate.c:8653:#if defined(CONFIG_LATX) && !defined(CONFIG_SOFTMMU)
target/i386/tcg/translate.c:8660:#if defined(CONFIG_LATX) && !defined(CONFIG_SOFTMMU)

因为 helper_aad / helper_aam 和存在
target/i386/tcg/int_helper.c:173:#if defined(CONFIG_SOFTMMU) && defined(CONFIG_LATX)
target/i386/tcg/int_helper.c:187:#if defined(CONFIG_SOFTMMU) && defined(CONFIG_LATX)

引入了 latx-options.h 而已
target/i386/tcg/seg_helper.c:34:#if defined(CONFIG_LATX) && defined(CONFIG_SOFTMMU)

这个直接拷贝了，所以不分析
target/i386/tcg/fpu_helper.c:31:#if defined(CONFIG_LATX) && defined(CONFIG_SOFTMMU)
target/i386/tcg/fpu_helper.c:751:#if defined(CONFIG_LATX) && defined(CONFIG_SOFTMMU)
target/i386/tcg/fpu_helper.c:2356:#if defined(CONFIG_LATX)
target/i386/tcg/fpu_helper.c:2414:#if defined(CONFIG_LATX) && defined(CONFIG_SOFTMMU)
target/i386/tcg/fpu_helper.c:2577:#if defined(CONFIG_LATX) && defined(CONFIG_SOFTMMU)

和 tb_exit_to_qemu 相关，那是用户态的
include/exec/translate-all.h:43:#ifdef CONFIG_LATX

重新定义了 TB_JMP_CACHE_BITS 的大小
include/hw/core/cpu.h:249:#ifdef CONFIG_LATX

声明了 qemu_latx_opts 和 qemu_latx_test_opts
include/sysemu/sysemu.h:123:#ifdef CONFIG_LATX
