## 串口直通

```c
serial_mm_init(address_space_mem, LS7A_UART_BASE, 0, pirq[0], 115200, serial_hd(0), DEVICE_NATIVE_ENDIAN);
```

- [ ] 检查一下，进入到中断处理函数之后就，是自动屏蔽中断的吧

- [ ] 让所有的操作 redirect 到 LS7A_UART_BASE 上，希望可以触发中断
  - [ ] makefile 中增加一个 option : serial_pass through
  - [ ] 比较两个 dmesg 差别
  - [ ] 进入到终端之后，是需要设置 BQL 的
  - [ ] 在 serial_io_ops 中，限制了 access size 为 1 ，在内核中如何实现
    - [ ] 注意，这里申请了 8 个，每一个位置对于都是不同的
    - 参考 /home/loongson/core/linux-4.19-loongson/arch/loongarch/include/asm/io.h 就可以了
  - 如何发送中断，是调用 irq=4 吗?
    - [ ] 但是具体是从哪一个位置开始调用啊
    - [ ] serial_update_irq 调用的时候，可以 raise 和 low 的
      - 开始之前 raise, 连续调用两次 ?
      - 应该是比 gsi_handler 向前一步 ?
      - [ ] 这个 lower 操作 loongson 是怎么响应的
  - 可以先拉低，然后拉高的操作吗?

```c
static const int isa_serial_irq[MAX_ISA_SERIAL_PORTS] = {4, 3, 4, 3};
```
- [ ] 回忆一下，当只是发送的时候，那么得到的永远都是 lower 啊
