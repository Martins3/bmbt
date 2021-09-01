# 总结一下各种 macro 吧

## TODO
- [ ] `__mips__` : 需要小心的仔细的分析
- [ ] TARGET_I386 出现的位置比想想的少很多，之后看 x64 的移植吧

## 一些 profile 类的就放着吧, 但是打开之后，可能需要新的支持
- DEBUG_DISAS : 出现的次数相当有限
- CONFIG_DEBUG_TCG : 很少出现
- CONFIG_XTM_PROFILE : 出现次数很多, 似乎真的有用，以后在慢慢支持吧

## xqm 特有的
- CONFIG_X86toMIPS
- [ ] CONFIG_LATX : 核实一下 CONFIG_X86toMIPS 和 CONFIG_LATX 同时在一起才是正确版本

## [ ] 需要处理的事情
- [ ] /home/maritns3/core/ld/DuckBuBi/include/qemu/config-host.h 里面有很多 CONFIG 可能到时候是不能使用的
  - [ ] CONFIG_ATOMIC64

## CONFIG_SOFTMMU CONFIG_USER_ONLY
- [x] 可以 disable 掉 tcg, 直接使用 kvm 的?
    - 应该是不可以的，记住 --accel 是 QEMU 运行参数，--target-list 的选择中就会将大家全部放进去
    - 所以可以说，其实 CONFIG_USER_ONLY 和 CONFIG_SOFTMMU 目前相反的一对功能，但是也许以后可以修改好

- [x] 会不会出现 CONFIG_SOFTMMU 和 CONFIG_USER_ONLY 同时出现的情况
  - 不可能吧, 分别出现在 build/x86_64-softmmu-config-target.h 和 build/x86_64-linux-user-config-target.h

- [x] 如果同时支持 softmmu 和 user 版本，如何处理?
```sh
../configure --target-list=x86_64-linux-user,x86_64-softmmu --disable-werror
```
从阅读代码的角度来说，这会导致一个强力的误导，实际上的操作方法，将 build/x86_64-linux-user-config-target.h 这个文件夹可能被 include 两次，
然后分别编译出来 softmmu 版本和 user only 版本，这种情况下 ccls 无法正确处理，所以阅读代码的时候，最好只是配置一个 target-list


一下代码使用 latest 的代码分析的:
helper_syscall

测试:
```sh
../configure --target-list=x86_64-linux-user --disable-werror
```

#### user mode exception / interrupt
从 helper_syscall 的位置离开，然后
```c
void helper_syscall(CPUX86State *env, int next_eip_addend)
{
    CPUState *cs = env_cpu(env);

    cs->exception_index = EXCP_SYSCALL;
    env->exception_is_int = 0;
    env->exception_next_eip = env->eip + next_eip_addend;
    cpu_loop_exit(cs);
}
```
- cpu_handle_exception : 因为  cpu_loop_exit
  - x86_cpu_do_interrupt : 调用对应的 handler
    - do_interrupt_user : 如果参数 is_int 是话，这个函数才有意义

从这两个位置设置 is_int 为 true:
- helper_raise_interrupt
  * gen_interrupt
      * disas_insn : 这里其中是两种情况 : int3 和 int N 其中 int N 需要检查权限的
- helper_into : 跳转到 overflow 的位置

所以，exception 的检查其实是有一定的道理的, 因为 int3, int0, syscall

#### user mode x86_cpu_tlb_fill
- 为什么用户态会出现 cpu_tlb_fill 的?
    - 首先可以确定，用户态的时候，一个 load 前面是不需要增加一堆翻译指令的
    - cpu_tlb_fill 这个操作，

- handle_cpu_signal
  * cpu_signal_handler
    * host_signal_handler
