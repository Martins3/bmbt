#ifndef _X86toMIPS_CONFIG_SYS_H_
#define _X86toMIPS_CONFIG_SYS_H_


void x86_to_mips_parse_options(QemuOpts *opts);
void x86_to_mips_parse_tests(QemuOpts *opts);
QemuOptsList qemu_xtm_opts;

#endif
