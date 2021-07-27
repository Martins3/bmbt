# 总结一下各种 macro 吧

## TODO
- [ ] `__mips__` : 需要小心的仔细的分析
- [ ] TARGET_I386 出现的位置比想想的少很多，之后看 x64 的移植吧

## 一些 profile 类的就放着吧

- DEBUG_DISAS : 出现的次数相当有限
- CONFIG_DEBUG_TCG : 很少出现
- CONFIG_XTM_PROFILE : 出现次数很多, 似乎真的有用，以后在慢慢支持吧

## 一些神奇的
- [ ] CONFIG_SOFTMMU CONFIG_USER_ONLY
    - 在 tcg_n_regions 中间的注释以及 CONFIG_USER_ONLY 的效果，其含义就是只有用户态的时候吧
    - x86_cpu_tlb_fill : 中间居然存在 CONFIG_USER_ONLY，非常难以理解

## xqm 特有的
- CONFIG_X86toMIPS
- [ ] CONFIG_LATX : 核实一下 CONFIG_X86toMIPS 和 CONFIG_LATX 才是正确版本

## [ ] 需要处理的事情
- [ ] /home/maritns3/core/ld/DuckBuBi/include/qemu/config-host.h 里面有很多 CONFIG 可能到时候是不能使用的
  - [ ] CONFIG_ATOMIC64

