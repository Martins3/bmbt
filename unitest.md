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
