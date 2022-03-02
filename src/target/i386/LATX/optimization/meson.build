i386_ss.add(when: 'CONFIG_LATX', if_true: files(
  'flag-lbt.c',
  'flag-pattern.c',
  'flag-reduction.c',
  'ibtc.c',
  'ir2-optimization.c',
  'shadow-stack.c'
))

i386_softmmu_ss.add(when: 'CONFIG_LATX', if_true: files(
  'sys-lsfpu.c',
  'sys-staticcs.c',
  'sys-njc.c',
  'sys-sigint.c',
  'sys-em.c'
))
