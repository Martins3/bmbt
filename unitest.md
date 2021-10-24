# 已经进行测试的内容
- [ ] first cpu ..
```c
#define first_cpu QTAILQ_FIRST(&cpus)
#define CPU_NEXT(cpu) QTAILQ_NEXT(cpu, node)
#define CPU_FOREACH(cpu) QTAILQ_FOREACH(cpu, &cpus, node)
```
  - [ ] 如果没有添加，CPU_NEXT 得到是啥? first_cpu 是啥?
  - [ ] 只有一个 cpu, CPU_NEXT 是啥?
- [ ] atomic.h
- [ ] src/qemu/memory.c
- [ ] RAMBLOCK_FOREACH : C 的 for 语言的过于巧妙的使用
- [ ] QEMU options 没有仔细的 review 的
  - https://github.com/Martins3/bmbt/issues/171 : debug 的结果看一下
- [ ] qht
  - 及其快速的移植的，没有正确的保障的，将 QEMU 本身的 unitest 跑一下
  - 无法理解 seqlock 中的 seqlock_write_lock_impl 被注释掉之后还是正确工作，而且 seqlock 似乎没有 writer 吗?
