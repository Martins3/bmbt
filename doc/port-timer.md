检查一下:
- [ ] resume_all_vcpus
- [ ] qemu_clock_enable
- [ ] qemu_clock_deadline_ns_all


如何构造出来一个最简单的 timer 的呀:
- 现在 timer timerout 的 callback 访问数据和 vCPU 的数据访问实际上是重合的
  - BQL 必然是需要的，但是现在可以保证都是 callback 了
  - 可以简单的分析一下这些 callback 实际上访问的数据是什么
    - [ ] 是如何访问到 cpu_interrupt 上的呀

- 单核和 main loop 的编程差异：
  - main loop 的执行会等待 vCPU 释放 BQL 的，而 vCPU 为了保证中断的及时响应，需要在每一个 tb 结束的位置检查，从而退出 BQL，来处理 interrupt 的
    - [x] 核查一下，vCPU 线程需要 exit 到故意释放 BQL 的地方
      - 精神出现了问题，显然
    - [x] 核查一下 main loop 从 poll 结束之后，一定需要持有 BQL lock 的呀
    - [ ] 这些 kick vCPU 的方法现在还可以保证正确? 重新思考一下 https://github.com/Martins3/bmbt/issues/163
      - [ ] 这些 callback 最后会调用到来 kick 一下 vCPU 吗?
  - 而 interrupt context 可以直接导致 vCPU 停下来
    - 并不能简单的将 BQL 变为中断屏蔽的，因为一旦屏蔽，vCPU 的执行流程就可能永远不会到释放 BQL 的哪一步上的。
    - 中断应该是随时可以到达的，感觉在单核模式下，BQL 可以重新设置为空的
  - 为什么 BQL 可以保护代码，因为代码只有是一个 thread 运行就可以了，具体是谁，其实不在乎的。
