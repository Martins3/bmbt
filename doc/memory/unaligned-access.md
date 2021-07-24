# 非对其访问

- [x] 所以，x86 为什么不害怕这个非对其访问的问题 ?

1. 搜索符号，convert_to_tcgmemop 上的注释对于 
MemOp 这个 enum 做出来一些解释, 包含了 size / signed / endian / aligned
2. get_alignment_bits 的逻辑看，就是检查一下 MemOp 的 6:4 的 bit 而已
3. 目前的代码，所有的位置都没有插入过 `MO_ALIGN_x` 的

所以 addr & ((1 << a_bits) - 1) 的检查永远失败啊


cpu_unaligned_access : x86 对应的 handler 没有赋值
但是一些 risc 平台是赋值的，
```c
void riscv_cpu_do_unaligned_access(CPUState *cs, vaddr addr,
                                   MMUAccessType access_type, int mmu_idx,
                                   uintptr_t retaddr)
{
    RISCVCPU *cpu = RISCV_CPU(cs);
    CPURISCVState *env = &cpu->env;
    switch (access_type) {
    case MMU_INST_FETCH:
        cs->exception_index = RISCV_EXCP_INST_ADDR_MIS;
        break;
    case MMU_DATA_LOAD:
        cs->exception_index = RISCV_EXCP_LOAD_ADDR_MIS;
        break;
    case MMU_DATA_STORE:
        cs->exception_index = RISCV_EXCP_STORE_AMO_ADDR_MIS;
        break;
    default:
        g_assert_not_reached();
    }
    env->badaddr = addr;
    env->two_stage_lookup = riscv_cpu_virt_enabled(env) ||
                            riscv_cpu_two_stage_lookup(mmu_idx);
    riscv_raise_exception(env, cs->exception_index, retaddr);
}
```
原因在于，X86 的根本没有非对其访问的 exception 啊

```c
static inline MemOp get_memop(TCGMemOpIdx oi) { return oi >> 4; }

static inline unsigned get_mmuidx(TCGMemOpIdx oi) { return oi & 15; }
```

- [ ] 如果 x86 本身是非对其访问，但是我们又是不支持非对其，那怎么办 ?
    - 我猜测是被模拟成为两次访问
