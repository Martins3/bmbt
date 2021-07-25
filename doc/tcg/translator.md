# accel/tcg/

## 基本文件摸底

| Filename           | desc                                                           |
|--------------------|----------------------------------------------------------------|
| cpu-exec.c         | tb 的执行                                                      |
| translate-all.c    | tb 的管理, tb 和 page 的关系 以及 ..., 总之是核心文件          |
| cpu-tlb.c          | softmmu 管理                                                   |

## Ask Niugene
- [ ] 再问一次, ./tcg/ 下真的有作用吗?
  - [ ] /home/maritns3/core/ld/x86-qemu-mips/tcg/tcg.c : 这就是那个包含两个 in.c 的
    - [ ] 这个会被使用上吗 ?
- [ ] /home/maritns3/core/ld/x86-qemu-mips/tcg/loongarch 是什么作用呀 ? 
- [ ] 中断之前不是说和 信号 存在关联吗?

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

- [ ] 每次执行完成一个 tb 就进行检查中断吗?
    - [ ] 对应的检查代码在哪里?

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
