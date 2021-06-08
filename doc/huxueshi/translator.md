# accel/tcg/

## 基本文件摸底
| Filename           | desc                                                           |
|--------------------|----------------------------------------------------------------|
| tcg-runtime-gvec.c |                                                                |
| translator.c       | 只有一个函数，但是其调用者是 gen_intermediate_code(deprecated) |
| cpu-exec.c         | tb 的执行                                                      |
| translate-all.c    | tb 的管理, tb 和 page 的关系 以及 ..., 总之是核心文件          |
| cpu-tlb.c          | softmmu 管理                                                   |

## Ask Niugene:
- [ ] 再问一次, ./tcg/ 下真的有作用吗?
  - [ ] /home/maritns3/core/ld/x86-qemu-mips/tcg/tcg.c : 这就是那个包含两个 in.c 的
    - [ ] 这个会被使用上吗 ?
- [ ] 是不是只是支持 x86 32 的指令，其实关系不大 ?
- [ ] xqm 还是支持 user mode 的吗?
- [ ] /home/maritns3/core/ld/x86-qemu-mips/tcg/loongarch 是什么作用呀 ? 
- [ ] 中断之前不是说和 信号 存在关联吗?
- [ ] 连 translate_vmrun 都要运行，真的有必要在二进制翻译中间还支持模拟虚拟机吗，有进行过测试吗?
  - [ ] 但是 translate_vmcall 又是支持的，真的让人迷惑啊
- [ ] qemu_tcg_mttcg_enabled 支持吗

## flow 
- qemu_tcg_rr_cpu_thread_fn
  - tcg_cpu_exec
    - cpu_exec
      - cpu_handle_exception
      - cpu_handle_interrupt
      - tb_find
        - tb_lookup__cpu_state
        - tb_gen_code
          - target_x86_to_mips_host : 原来的 tcg 入口是 tcg_gen_code, 现在移除了 tcg
            - tr_translate_tb
              - tr_ir2_generate
                - ir1_translate
          - tb_link_page
          - tcg_tb_insert
      - cpu_loop_exec_tb
        - cpu_tb_exec
          - tcg_qemu_tb_exec

## 重点分析
tb_lookup__cpu_state
- [ ] 第一级: `cpu->tb_jmp_cache`
- [ ] 第二级: 没看懂, 也没看是怎么添加进去的

- [ ] 如何处理中断的
  - [ ] 这两个函数分别干什么 ?
      - cpu_handle_exception
      - cpu_handle_interrupt
  - [ ] 每次执行完成一个 tb 就进行检查吗?
      - [ ] 对应的检查代码在哪里?
      - [ ] 检查一下 tb 
  - [ ] 为什么 cpu_handle_exception 是在内部的，cpu_handle_interrupt 在循环的外部
    - [ ] 实际上，根本不知道再说什么

## interrupt 机制
- apic_local_deliver : 在 apic 中存在大量的模拟
  - cpu_interrupt
    - generic_handle_interrupt
      - `cpu->interrupt_request |= mask;` 

在 cpu_handle_interrupt 中，会处理几种特殊情况，默认是 `cc->cpu_exec_interrupt(cpu, interrupt_request)`
也就是 x86_cpu_exec_interrupt, 在这里根据 idt 之类的中断处理需要的地址, 然后跳转过去执行的


在 tr_gen_tb_start 生成了 icount 的检查代码，那个并不是 interrupt 的检查机制。

## 二进制文件的反汇编阶段
- target_x86_to_mips_host
  - tr_disasm
    - get_ir1_list
      - `__disasm_one_ir1`
        - cpu_read_code_via_qemu
          - cpu_ldub_code : x86-qemu-mips/include/exec/cpu_ldst_template.h 中间生成的
        - ir1_disasm
          - cs_disasm : 这个就是 capstone 的代码了

## How softmmu works
Q: 其实，访问存储也是隐藏的 load，是如何被 softmmu 处理的?

A: 指令的读取都是 tb 的事情


- gen_ldst_softmmu_helper
  - `__gen_ldst_softmmu_helper_native`
    - tr_gen_lookup_qemu_tlb : TLB 比较查询
    - tr_gen_ldst_slow_path : 无奈，只能跳转到 slow path 去
      - td_rcd_softmmu_slow_path

- tr_ir2_generate
  - tr_gen_softmmu_slow_path
    - `__tr_gen_softmmu_sp_rcd`
      - helper_ret_stb_mmu : 跳转的入口通过 helper_ret_stb_mmu 实现, 当前在 accel/tcg/cputlb.c 中
        - store_helper
          - io_writex
            - memory_region_dispatch_write
