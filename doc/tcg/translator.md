# accel/tcg/

## 基本文件摸底

## Ask Niugene
- [ ] 再问一次, ./tcg/ 下真的有作用吗?
  - [ ] /home/maritns3/core/ld/x86-qemu-mips/tcg/tcg.c : 这就是那个包含两个 in.c 的
    - [ ] 这个会被使用上吗 ?
- [ ] /home/maritns3/core/ld/x86-qemu-mips/tcg/loongarch 是什么作用呀 ? 
- [ ] 中断之前不是说和 信号 存在关联吗?


## 重点分析
tb_lookup__cpu_state
- [ ] 第一级: `cpu->tb_jmp_cache`
- [ ] 第二级: 没看懂, 也没看是怎么添加进去的

- [ ] 每次执行完成一个 tb 就进行检查中断吗?
    - [ ] 对应的检查代码在哪里?

在 tr_gen_tb_start 生成了 icount 的检查代码，那个并不是 interrupt 的检查机制。
