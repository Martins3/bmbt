# port translator

## 定位文件

- [ ] qemu_tcg_mttcg_enabled 支持吗
- [ ] /home/maritns3/core/ld/x86-qemu-mips/accel/tcg : 那些是真正需要的文件
  - /home/maritns3/core/ld/x86-qemu-mips/accel/tcg/tcg-runtime-gvec.c ? 应该是没用的
  - [ ] translator.c
  - cpu-exec.c
  - translate-all.c
- [ ] /home/maritns3/core/ld/x86-qemu-mips/hw/core/cpu.c : CPU 初始化之类的工作


Ask Niugene:

- [ ] 再问一次, ./tcg/ 下真的有作用吗?
  - [ ] /home/maritns3/core/ld/x86-qemu-mips/tcg/tcg.c : 这就是那个包含两个 in.c 的
    - [ ] 这个会被使用上吗 ?
- [ ] 是不是只是支持 x86 32 的指令，其实关系不大 ?
- [ ] xqm 还是支持 user mode 的吗?
- [ ] /home/maritns3/core/ld/x86-qemu-mips/tcg/loongarch 是什么作用呀 ? 


## flow 
- qemu_tcg_rr_cpu_thread_fn
  - tcg_cpu_exec
    - cpu_exec
      - cpu_handle_exception
      - cpu_handle_interrupt
      - tb_find
        - tb_lookup__cpu_state
        - tb_gen_code
          - target_x86_to_mips_host : 原来的入口是 tcg_gen_code, 可惜换不得
            - tr_translate_tb
              - tr_ir2_generate
                - ir1_translate
          - tb_link_page
          - tcg_tb_insert
      - cpu_loop_exec_tb
        - cpu_tb_exec
          - tcg_qemu_tb_exec


This is original code flow(cited form Niugene):
- `tb_gen_code` to translate TB
  - call `gen_intermediate_code` to generate QEMU IR
    - call `translator_loop` which is a standard process to translate
      - call `ops->translate_insn` to translate one instruction
      - `ops` was defined by the specific target such as MIPS
      - for MIPS,  `ops->translate_insn` is `mips_tr_translate_insn`

## 重点分析
tb_lookup__cpu_state
- [ ] 第一级: `cpu->tb_jmp_cache`
- [ ] 第二级: 没看懂, 也没看是怎么添加进去的

- [ ] 如何处理中断的
  - [ ] 这两个函数分别干什么 ?
      - cpu_handle_exception
      - cpu_handle_interrupt
  - [ ] 是没执行完成一个 tb 就进行检查吗 ?

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
- [x] 访问存储也是隐藏的 load ?
  - 这是一个脑残问题，因为指令的读取都是 tb 的事情

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
