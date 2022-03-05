# 大致分析一下 Loongson QEMU 的 irq 的原理

- 发生在 centos-qemu/hw/loongarch/ 中间

- qemu 中断注入的位置:
  - pci_ls7a_set_irq
  - legacy_set_irq : 应该就是这个东西注入普通中断了
    - ioapic_handler
      - kvm_set_irq

注意，其实我们是移植过串口的:
- 如何将 qemu 划分为两个 macro 啊

- [x] 难道在 kvm 中实现了 extioi 中断控制器吗?

似乎是的:
- /home/maritns3/core/linux-4.19-loongson/arch/loongarch/kvm/csr.c


- kvm_misc_set
  - kvm_setup_ls3a_extirq

似乎是通过 ext_irq_update_core 实现发送的

- [ ] 无法理解, kvm_setup_ls3a_extirq 为什么从来没有被调用过
- 我们注意到，在 k 中，是存在 level = 0 和 level = 1 交替出现的，但是在 mr 中，只有 0 ，
- [ ] 是不是因为没有正确的初始化 tty 设备

## 从 kvm_set_irq 一路向下

## 这种东西是谁触发的

```c
huxueshi:ext_irq_update_core X
[83326.847767] huxueshi:ext_irq_update_core B
[83326.847768] huxueshi:ext_irq_update_core X
```
显示出来是 ext_isr 没有设置

- [ ] 是因为我们忘记了调用 ext_set_irq_affinity 吗?

- [ ] 这个东西的调用有一点神奇

```c
[    0.717958] [<9000000000208b80>] show_stack+0x2c/0x134
[    0.717964] [<9000000000f78e34>] dump_stack+0x94/0xc0
[    0.717967] [<9000000000f73ae8>] ext_set_irq_affinity+0x5c/0x39c
[    0.717972] [<9000000000283de0>] irq_do_set_affinity+0x34/0xf0
[    0.717975] [<900000000028422c>] irq_setup_affinity+0x128/0x154
[    0.717977] [<90000000002871f4>] irq_startup+0x7c/0x194
[    0.717980] [<9000000000284d9c>] __setup_irq+0x7cc/0x804
[    0.717983] [<9000000000284fa0>] request_threaded_irq+0xf4/0x1f0
[    0.717987] [<9000000000913320>] acpi_os_install_interrupt_handler+0xac/0x138
[    0.717990] [<900000000092e864>] acpi_ev_install_xrupt_handlers+0x24/0x98
[    0.717995] [<90000000014eb214>] acpi_init+0xc0/0x33c
[    0.717997] [<9000000000200b8c>] do_one_initcall+0x3c/0x1bc
[    0.718001] [<90000000014a8e8c>] kernel_init_freeable+0x2d4/0x39c
[    0.718004] [<9000000000f791e0>] kernel_init+0x10/0xf4
[    0.718006] [<900000000020316c>] ret_from_kernel_thread+0xc/0x10
```

这个似乎是串口中断的:
```c
[    0.000000] [<9000000000208b80>] show_stack+0x2c/0x134
[    0.000000] [<9000000000f78e34>] dump_stack+0x94/0xc0
[    0.000000] [<9000000000f73ae8>] ext_set_irq_affinity+0x5c/0x39c
[    0.000000] [<9000000000283de0>] irq_do_set_affinity+0x34/0xf0
[    0.000000] [<900000000028422c>] irq_setup_affinity+0x128/0x154
[    0.000000] [<90000000002871f4>] irq_startup+0x7c/0x194
[    0.000000] [<9000000000287694>] irq_set_chained_handler_and_data+0x48/0x70
[    0.000000] [<90000000008a48c8>] pch_lpc_init+0x108/0x184
[    0.000000] [<9000000000f6e444>] irqchip_init_default+0x38c/0x3c0
[    0.000000] [<90000000014acea4>] arch_init_irq+0x2c/0x68
[    0.000000] [<90000000014ae708>] init_IRQ+0x50/0xd8
[    0.000000] [<90000000014a8a40>] start_kernel+0x2e8/0x460
[    0.000000] [<9000000000f790dc>] smp_bootstrap+0x0/0x44
```

```c
[    1.346677] [<9000000000208b80>] show_stack+0x2c/0x134
[    1.347326] [<9000000000f78e34>] dump_stack+0x94/0xc0
[    1.347959] [<9000000000f73ae8>] ext_set_irq_affinity+0x5c/0x39c
[    1.348898] [<9000000000283de0>] irq_do_set_affinity+0x34/0xf0
[    1.349797] [<90000000002872bc>] irq_startup+0x144/0x194
[    1.350627] [<9000000000284d9c>] __setup_irq+0x7cc/0x804
[    1.351317] [<9000000000284fa0>] request_threaded_irq+0xf4/0x1f0
[    1.352084] [<90000000009633d8>] vp_find_vqs_msix+0x144/0x448
[    1.352806] [<9000000000963738>] vp_find_vqs+0x5c/0x1f8
[    1.353462] [<900000000096220c>] vp_modern_find_vqs+0x14/0x5c
[    1.354180] [<9000000000a51ab8>] init_vq+0x1a4/0x334
[    1.354801] [<9000000000a52b6c>] virtblk_probe+0xf0/0x6bc
[    1.355476] [<900000000095f4b0>] virtio_dev_probe+0x180/0x268
[    1.356196] [<9000000000a291f8>] really_probe+0x200/0x298
[    1.356874] [<9000000000a29430>] driver_probe_device+0x64/0x100
[    1.357613] [<9000000000a295d8>] __driver_attach+0x10c/0x110
[    1.358320] [<9000000000a26f2c>] bus_for_each_dev+0x64/0xa0
[    1.359016] [<9000000000a284a8>] bus_add_driver+0x1ec/0x220
[    1.359713] [<9000000000a2a21c>] driver_register+0x64/0x12c
[    1.360413] [<90000000014f2768>] init+0x78/0xb4
[    1.360984] [<9000000000200b8c>] do_one_initcall+0x3c/0x1bc
[    1.361852] [<90000000014a8e8c>] kernel_init_freeable+0x2d4/0x39c
[    1.362785] [<9000000000f791e0>] kernel_init+0x10/0xf4
[    1.363590] [<900000000020316c>] ret_from_kernel_thread+0xc/0x10
```

在 host 的 kvm 中
可以得到这种错误:
```c
[  781.147169] huxueshi:ext_irq_update_core 249
[  781.147170] huxueshi:ext_irq_update_core 250
[  781.147171] huxueshi:ext_irq_update_core 251
[  781.147172] huxueshi:ext_irq_update_core 252
[  781.147172] huxueshi:ext_irq_update_core 253
[  781.147173] huxueshi:ext_irq_update_core 254
[  781.147174] huxueshi:ext_irq_update_core 255
[  781.147176] huxueshi:ext_irq_update_core 19
```

这个 log 让我们相信，必然是注入过的:
```c
[ 1856.855021] kvm [31252]: ls3a_ext_intctl_writeb: addr=0x773,val=0x0
[ 1856.855022] huxueshi:ext_irq_update_core 19
[ 1856.855025] kvm [31252]: ls3a_ext_intctl_writeb: addr=0x774,val=0x0
[ 1856.855026] huxueshi:ext_irq_update_core 20
[ 1857.268685] kvm [31252]: [0x900000000035a1b4] HYPCALL 0x0
```

现在总是判断是没有 isr 的，这才是关键，为什么 kvm 注入的时候，难道不是自己需要去写 isr 的吗，说明是前面的位置搞的:
```c
596] [<9000000000208bc0>] show_stack+0x2c/0x134
[ 2318.364599] [<9000000000fb52ac>] dump_stack+0x94/0xc0
[ 2318.364686] [<ffff800024cc4580>] L0+0x70/0x27c [kvm]
[ 2318.364862] [<ffff800024cbf4e0>] ls3a_ext_intctl_write+0x1f8/0x838 [kvm]
[ 2318.365037] [<ffff800024ca9fe8>] __kvm_io_bus_write.isra.14+0xb4/0x10c [kvm]
[ 2318.365210] [<ffff800024caa07c>] L0+0x3c/0x58 [kvm]
[ 2318.365384] [<ffff800024cc0ce4>] kvm_extioi_set+0x68/0x128 [kvm]
[ 2318.365559] [<ffff800024cc3bb0>] L0+0xe4/0x558 [kvm]
[ 2318.365733] [<ffff800024cbcab8>] lvz_trap_handle_psi+0x318/0x414 [kvm]
[ 2318.365908] [<ffff800024cb4cbc>] L0+0x710/0x8bc [kvm]
[ 2318.365997] [<900000000021718c>] kvm_exit_entry+0xfc/0x118
[ 2318.365999] huxueshi:ext_irq_update_core 20
```

实现的方法:
```c
static void kvm_ls7a_ioapic_raise(struct kvm *kvm, unsigned long mask)
```

实际上，似乎的确注入了啊:
```c
  pr_info("huxueshi:%s %d %d\n", __FUNCTION__, irq, priority);
[ 4769.358511] huxueshi:kvm_vcpu_ioctl_interrupt intr=3
[ 4769.358512] huxueshi:lvz_queue_irq 3
[ 4769.358533] huxueshi:lvz_irq_deliver 2 3
```

- 都已经跟踪到这个位置了，说不过去了吧!
  - 是的，这是正常的，所以必须过去

正常的效果:
```c
] huxueshi:kvm_vcpu_ioctl_interrupt intr=3
[39339.125609] huxueshi:lvz_queue_irq 3
[39339.125612] huxueshi:lvz_irq_deliver 2 3
[39339.125616] kvm [13106]: ext_irq_handler:irq = 2,level = 0
[39339.126497] kvm [13106]: msi_irq_handler,2,up
[39339.126499] kvm [13106]: ext_irq_handler:irq = 2,level = 1
[39339.126500] huxueshi:msi_irq_handler raise the irq 2
[39339.126500] huxueshi:msi_irq_handler raise
[39339.126502] kvm [13106]: ext_irq_update_core:86  --- vcpu_id 0 irqnum 2 found1 0x100 ipnum 1
[39339.126503] kvm [13106]: ext_irq_update_core:92 --- vcpu_id 0 ipnum 1 raise
[39339.126503] huxueshi:kvm_vcpu_ioctl_interrupt intr=3
[39339.126504] huxueshi:lvz_queue_irq 3
[39339.126507] huxueshi:lvz_irq_deliver 2 3
[39339.126511] kvm [13106]: ext_irq_handler:irq = 2,level = 0
[39339.139846] kvm [13100]: ext_irq_handler:irq = 33,level = 1
[39339.139848] huxueshi:msi_irq_handler raise the irq 33
[39339.139849] huxueshi:msi_irq_handler raise
[39339.139850] kvm [13100]: ext_irq_update_core:86  --- vcpu_id 0 irqnum 33 found1 0x100 ipnum 1
[39339.139851] kvm [13100]: ext_irq_update_core:92 --- vcpu_id 0 ipnum 1 raise
[39339.139852] huxueshi:kvm_vcpu_ioctl_interrupt intr=3
[39339.139853] huxueshi:lvz_queue_irq 3
[39339.139857] huxueshi:lvz_irq_deliver 2 3
[39339.141350] kvm [13100]: ext_irq_handler:irq = 33,level = 1
[39339.141351] huxueshi:msi_irq_handler raise the irq 33
[39339.141352] huxueshi:msi_irq_handler raise
[39339.141353] kvm [13100]: ext_irq_update_core:86  --- vcpu_id 0 irqnum 33 found1 0x100 ipnum 1
[39339.141354] kvm [13100]: ext_irq_update_core:92 --- vcpu_id 0 ipnum 1 raise
[39339.141355] huxueshi:kvm_vcpu_ioctl_interrupt intr=3
[39339.141356] huxueshi:lvz_queue_irq 3
[39339.141359] huxueshi:lvz_irq_deliver 2 3
```

这边只有:
```c
[39413.058325] kvm [13380]: ls3a_ext_intctl_writeb: addr=0x85b,val=0x30000000000
[39413.058327] kvm [13380]: ls3a_ext_intctl_writeb: addr=0x85c,val=0x30000000000
[39413.058328] kvm [13380]: ls3a_ext_intctl_writeb: addr=0x85d,val=0x30000000000
[39413.058329] kvm [13380]: ls3a_ext_intctl_writeb: addr=0x85e,val=0x30000000000
[39413.058329] kvm [13380]: ls3a_ext_intctl_writeb: addr=0x85f,val=0x30000000000
[39413.058332] huxueshi:ext_irq_update_core B
[39413.058333] huxueshi:ext_irq_update_core 0
[39413.058333] huxueshi:ext_irq_update_core B
[39413.058334] huxueshi:ext_irq_update_core 1
[39413.058335] huxueshi:ext_irq_update_core B
```
正常的也是有的啊，好的，现在几乎是完全无法观测到 l

如果在 qemu 中键入字符，可以得到下面的内容，
```c
[40260.612060] huxueshi:kvm_vm_ioctl_irq_line irq_type=4 vcpu_idx=0 irq_num=2
[40260.612088] huxueshi:kvm_vm_ioctl_irq_line irq_type=4 vcpu_idx=0 irq_num=2
[40260.612097] huxueshi:kvm_vm_ioctl_irq_line irq_type=4 vcpu_idx=0 irq_num=2
[40260.612116] huxueshi:kvm_vm_ioctl_irq_line irq_type=4 vcpu_idx=0 irq_num=2
[40260.612125] huxueshi:kvm_vm_ioctl_irq_line irq_type=4 vcpu_idx=0 irq_num=2
```

```c
  pr_info("huxueshi:%s level=%d state->intedge=%llx mask=%llx last_intirr=%llx intirr=%llx\n", __FUNCTION__, level, state->intedge, mask, state->last_intirr, state->intirr);
```
得到的，不正常的:
```c
[41412.380939] huxueshi:kvm_vm_ioctl_irq_line irq_type=4 vcpu_idx=0 irq_num=2
[41412.380940] huxueshi:kvm_ls7a_ioapic_set_irq level=0 state->intedge=0 mask=4 last_intirr=0 intirr=0
```

正常的完整的一段应该是这样的:
```c
[41565.981837] huxueshi:kvm_ls7a_ioapic_set_irq level=1 state->intedge=8 mask=4 last_intirr=0 intirr=0
[41565.981838] huxueshi:kvm_ls7a_ioapic_raise
[41565.981838] kvm [7368]: msi_irq_handler,2,up
[41565.981839] kvm [7368]: ext_irq_handler:irq = 2,level = 1
[41565.981840] huxueshi:msi_irq_handler raise the irq 2
[41565.981841] huxueshi:msi_irq_handler raise
[41565.981842] kvm [7368]: ext_irq_update_core:86  --- vcpu_id 0 irqnum 2 found1 0x100 ipnum 1
[41565.981843] kvm [7368]: ext_irq_update_core:92 --- vcpu_id 0 ipnum 1 raise
[41565.981844] huxueshi:kvm_vcpu_ioctl_interrupt intr=3
[41565.981845] huxueshi:lvz_queue_irq 3
[41565.981848] huxueshi:lvz_irq_deliver 2 3
```
对比了一下，似乎是 state->intedge=8 没有正确设置吗?

其实还是没有初始化的原因:
- ls7a_ioapic_reg_write

不能理解为什么


```c
typedef struct kvm_ls7a_ioapic_state {
  u64 int_id;
  /* 0x020 interrupt mask register */
  u64 int_mask;
  /* 0x040 1=msi */
  u64 htmsi_en;
  /* 0x060 edge=1 level  =0 */
  u64 intedge;
  /* 0x080 for clean edge int,set 1 clean,set 0 is noused */
  u64 intclr;
  /* 0x0c0 */
  u64 auto_crtl0;
  /* 0x0e0 */
  u64 auto_crtl1;
  /* 0x100 - 0x140 */
  u8 route_entry[64];
  /* 0x200 - 0x240 */
  u8 htmsi_vector[64];
  /* 0x300 */
  u64 intisr_chip0;
  /* 0x320 */
  u64 intisr_chip1;
  /* edge detection */
  u64 last_intirr;
  /* 0x380 interrupt request register */
  u64 intirr;
  /* 0x3a0 interrupt service register */
  u64 intisr;
  /* 0x3e0 interrupt level polarity selection register,
   * 0 for high level tirgger
   */
  u64 int_polarity;
} LS7AApicState;
```

```c
[46951.096055] huxueshi:kvm_vm_ioctl_irq_line irq_type=4 vcpu_idx=0 irq_num=2 level=0
[46951.096057] huxueshi:kvm_ls7a_ioapic_set_irq level=0 state->intedge=8 mask=4 last_intirr=0 intirr=4
[46951.096060] huxueshi:kvm_vm_ioctl_irq_line irq_type=4 vcpu_idx=0 irq_num=2 level=1
[46951.096061] huxueshi:kvm_ls7a_ioapic_set_irq level=1 state->intedge=8 mask=4 last_intirr=0 intirr=0

int kvm_ls7a_ioapic_set_irq(struct kvm *kvm, int irq, int level)
{
  struct ls7a_kvm_ioapic *s;
  struct kvm_ls7a_ioapic_state *state;
  uint64_t mask = 1ULL << irq;
  s = ls7a_ioapic_irqchip(kvm);
  state = &s->ls7a_ioapic;
  BUG_ON(irq < 0 || irq >= LS7A_IOAPIC_NUM_PINS);

  if (state->intedge & mask) {
    /* edge triggered */
    if (level) {
      if ((state->last_intirr & mask) == 0) {
        state->intirr |= mask;
        kvm_ls7a_ioapic_raise(kvm, mask);
      }
      state->last_intirr |= mask;
    } else
      state->last_intirr &= ~mask;
  } else {
    /* level triggered */
    if (!!level) {
      if ((state->intirr & mask) == 0) {
        state->intirr |= mask;
        kvm_ls7a_ioapic_raise(kvm, mask);
      }
    } else {
      if (state->intirr & mask) {
        state->intirr &= ~mask;
        kvm_ls7a_ioapic_lower(kvm, mask);
      }
    }
  }
  kvm->stat.lvz_kvm_ls7a_ioapic_set_irq++;
  return 0;
}
```

- [x] 在 qemu 中检查一下，为什么有时候中断是 level 的，有的时候是 edge 的，而且正好是交替的
  - 确实是没有问题的，似乎是 level 的

```c
[53244.269687] huxueshi:kvm_vm_ioctl_irq_line irq_type=4 vcpu_idx=0 irq_num=2 level=1
[53244.269688] huxueshi:kvm_ls7a_ioapic_set_irq level=1 state->intedge=8 mask=4 last_intirr=0 intirr=0
[53244.269689] huxueshi:kvm_ls7a_ioapic_raise
[53244.269697] huxueshi:kvm_vm_ioctl_irq_line irq_type=4 vcpu_idx=0 irq_num=2 level=0
[53244.269698] huxueshi:kvm_ls7a_ioapic_set_irq level=0 state->intedge=8 mask=4 last_intirr=0 intirr=4
[53244.269699] huxueshi:kvm_ls7a_ioapic_lower
```
实际上，任何一次的 lower 和 raise 都是可以看到的，但是不是每次都可以注入到 msi handler 中。

## 实际上
当 guest 每次输出一个字符的时候，qemu 完成输出之后，都是通过中断通知一下内核的。

lower 的:
```c
#0  0x00000001201af148 in huxueshi () at /home/loongson/core/centos-qemu/hw/loongarch/larch_3a.c:1052
#1  0x00000001201af1d4 in ioapic_handler (opaque=0x120ce1ae0, irq=2, level=0) at /home/loongson/core/centos-qemu/hw/loongarch/larch_3a.c:1066
#2  0x00000001202c66b0 in qemu_set_irq (irq=0x120e78ed0, level=0) at /home/loongson/core/centos-qemu/hw/core/irq.c:44
#3  0x00000001201aedac in legacy_set_irq (opaque=0x120c25010, irq=0, level=0) at /home/loongson/core/centos-qemu/hw/loongarch/larch_3a.c:917
#4  0x00000001202c66b0 in qemu_set_irq (irq=0x12128b830, level=0) at /home/loongson/core/centos-qemu/hw/core/irq.c:44
#5  0x00000001202baa28 in qemu_irq_lower (irq=0x12128b830) at /home/loongson/core/centos-qemu/include/hw/irq.h:17
#6  0x00000001202baf58 in serial_update_irq (s=0x12128b9a0) at /home/loongson/core/centos-qemu/hw/char/serial.c:152
#7  0x00000001202bb58c in serial_xmit (s=0x12128b9a0) at /home/loongson/core/centos-qemu/hw/char/serial.c:261
#8  0x00000001202bba80 in serial_ioport_write (opaque=0x12128b9a0, addr=0, val=74, size=1) at /home/loongson/core/centos-qemu/hw/char/serial.c:373
#9  0x00000001202bcf6c in serial_mm_write (opaque=0x12128b9a0, addr=0, value=74, size=1) at /home/loongson/core/centos-qemu/hw/char/serial.c:1032
#10 0x00000001200d8114 in memory_region_write_accessor (mr=0x12128ba80, addr=0, value=0xffe7ffdfc8, size=1, shift=0, mask=255, attrs=...) at /home/loongson/core/centos-qemu/memory.c:483
#11 0x00000001200d83f0 in access_with_adjusted_size (addr=0, value=0xffe7ffdfc8, size=1, access_size_min=1, access_size_max=8, access_fn=0x1200d8024 <memory_region_write_accessor>, mr=0x12128ba80, attrs=...) at /home/loongson/core/centos-qemu/memory.c:544
#12 0x00000001200db830 in memory_region_dispatch_write (mr=0x12128ba80, addr=0, data=74, op=MO_8, attrs=...) at /home/loongson/core/centos-qemu/memory.c:1475
#13 0x0000000120065a68 in flatview_write_continue (fv=0xffe0a46e70, addr=534774240, attrs=..., buf=0xfff7fcc028 "J", len=1, addr1=0, l=1, mr=0x12128ba80) at /home/loongson/core/centos-qemu/exec.c:3129
#14 0x0000000120065c00 in flatview_write (fv=0xffe0a46e70, addr=534774240, attrs=..., buf=0xfff7fcc028 "J", len=1) at /home/loongson/core/centos-qemu/exec.c:3169
#15 0x0000000120066020 in address_space_write (as=0x120b30e88 <address_space_memory>, addr=534774240, attrs=..., buf=0xfff7fcc028 "J", len=1) at /home/loongson/core/centos-qemu/exec.c:3259
#16 0x00000001200660b4 in address_space_rw (as=0x120b30e88 <address_space_memory>, addr=534774240, attrs=..., buf=0xfff7fcc028 "J", len=1, is_write=true) at /home/loongson/core/centos-qemu/exec.c:3269
#17 0x00000001200f9540 in kvm_cpu_exec (cpu=0x120c91360) at /home/loongson/core/centos-qemu/accel/kvm/kvm-all.c:2386
#18 0x00000001200c5d20 in qemu_kvm_cpu_thread_fn (arg=0x120c91360) at /home/loongson/core/centos-qemu/cpus.c:1318
#19 0x00000001206e2538 in qemu_thread_start (args=0x120cc0790) at /home/loongson/core/centos-qemu/util/qemu-thread-posix.c:519
#20 0x000000fff754489c in start_thread () at /lib/loongarch64-linux-gnu/libpthread.so.0
```

raise 的:
```c
#0  0x00000001201af148 in huxueshi () at /home/loongson/core/centos-qemu/hw/loongarch/larch_3a.c:1052
#1  0x00000001201af1ec in ioapic_handler (opaque=0x120ce1ae0, irq=2, level=1) at /home/loongson/core/centos-qemu/hw/loongarch/larch_3a.c:1067
#2  0x00000001202c66c8 in qemu_set_irq (irq=0x120e78ed0, level=1) at /home/loongson/core/centos-qemu/hw/core/irq.c:44
#3  0x00000001201aedac in legacy_set_irq (opaque=0x120c25010, irq=0, level=1) at /home/loongson/core/centos-qemu/hw/loongarch/larch_3a.c:917
#4  0x00000001202c66c8 in qemu_set_irq (irq=0x12128b830, level=1) at /home/loongson/core/centos-qemu/hw/core/irq.c:44
#5  0x00000001202baa0c in qemu_irq_raise (irq=0x12128b830) at /home/loongson/core/centos-qemu/include/hw/irq.h:12
#6  0x00000001202baf5c in serial_update_irq (s=0x12128b9a0) at /home/loongson/core/centos-qemu/hw/char/serial.c:150
#7  0x00000001202bb5a4 in serial_xmit (s=0x12128b9a0) at /home/loongson/core/centos-qemu/hw/char/serial.c:261
#8  0x00000001202bba98 in serial_ioport_write (opaque=0x12128b9a0, addr=0, val=111, size=1) at /home/loongson/core/centos-qemu/hw/char/serial.c:373
#9  0x00000001202bcf84 in serial_mm_write (opaque=0x12128b9a0, addr=0, value=111, size=1) at /home/loongson/core/centos-qemu/hw/char/serial.c:1032
#10 0x00000001200d8114 in memory_region_write_accessor (mr=0x12128ba80, addr=0, value=0xffe7ffdfc8, size=1, shift=0, mask=255, attrs=...) at /home/loongson/core/centos-qemu/memory.c:483
#11 0x00000001200d83f0 in access_with_adjusted_size (addr=0, value=0xffe7ffdfc8, size=1, access_size_min=1, access_size_max=8, access_fn=0x1200d8024 <memory_region_write_accessor>, mr=0x12128ba80, attrs=...) at /home/loongson/core/centos-qemu/memory.c:544
#12 0x00000001200db830 in memory_region_dispatch_write (mr=0x12128ba80, addr=0, data=111, op=MO_8, attrs=...) at /home/loongson/core/centos-qemu/memory.c:1475
#13 0x0000000120065a68 in flatview_write_continue (fv=0xffe0012880, addr=534774240, attrs=..., buf=0xfff7fcc028 "o", len=1, addr1=0, l=1, mr=0x12128ba80) at /home/loongson/core/centos-qemu/exec.c:3129
#14 0x0000000120065c00 in flatview_write (fv=0xffe0012880, addr=534774240, attrs=..., buf=0xfff7fcc028 "o", len=1) at /home/loongson/core/centos-qemu/exec.c:3169
#15 0x0000000120066020 in address_space_write (as=0x120b30e88 <address_space_memory>, addr=534774240, attrs=..., buf=0xfff7fcc028 "o", len=1) at /home/loongson/core/centos-qemu/exec.c:3259
#16 0x00000001200660b4 in address_space_rw (as=0x120b30e88 <address_space_memory>, addr=534774240, attrs=..., buf=0xfff7fcc028 "o", len=1, is_write=true) at /home/loongson/core/centos-qemu/exec.c:3269
#17 0x00000001200f9540 in kvm_cpu_exec (cpu=0x120c91360) at /home/loongson/core/centos-qemu/accel/kvm/kvm-all.c:2386
#18 0x00000001200c5d20 in qemu_kvm_cpu_thread_fn (arg=0x120c91360) at /home/loongson/core/centos-qemu/cpus.c:1318
#19 0x00000001206e2550 in qemu_thread_start (args=0x120cc0790) at /home/loongson/core/centos-qemu/util/qemu-thread-posix.c:519
#20 0x000000fff754489c in start_thread () at /lib/loongarch64-linux-gnu/libpthread.so.0
#21 0x000000fff74aafe4 in  () at /lib/loongarch64-linux-gnu/libc.so.6
```

让 qemu 的 tty 模拟注入 raise 中断之后，那么正常出发中断了。

- 所以 ttyS0 对于 extioi 而言，是一个 2 号中断
```c
[    4.704918] extioi: huxueshi:extioi_irq_dispatch i=0 bit=2 virq=19
[    4.709109] extioi: huxueshi:extioi_irq_dispatch i=0 bit=2 virq=19
[    4.713247] extioi: huxueshi:extioi_irq_dispatch i=0 bit=2 virq=19
[    4.717329] extioi: huxueshi:extioi_irq_dispatch i=0 bit=2 virq=19
[    4.721443] extioi: huxueshi:extioi_irq_dispatch i=0 bit=2 virq=19
[    4.725543] extioi: huxueshi:extioi_irq_dispatch i=0 bit=2 virq=19
[    4.729707] extioi: huxueshi:extioi_irq_dispatch i=0 bit=2 virq=19
[    4.733878] extioi: huxueshi:extioi_irq_dispatch i=0 bit=2 virq=19
[    4.738068] extioi: huxueshi:extioi_irq_dispatch i=0 bit=2 virq=19
[    4.742180] extioi: huxueshi:extioi_irq_dispatch i=0 bit=2 virq=19
[    4.745744] extioi: huxueshi:extioi_irq_dispatch i=0 bit=2 virq=19
[    4.749116] extioi: huxueshi:extioi_irq_dispatch i=0 bit=2 virq=19
[    4.752501] extioi: huxueshi:extioi_irq_dispatch i=0 bit=2 virq=19
[    4.755973] extioi: huxueshi:extioi_irq_dispatch i=0 bit=2 virq=19
[    4.759987] extioi: huxueshi:extioi_irq_dispatch i=0 bit=2 virq=19
```


实际上，这种注入方法是非常粗糙的，因为过早注入，导致内核 crash 掉了
```c
[    2.405590] [<9000000000208b80>] show_stack+0x2c/0x134
[    2.406284] [<9000000000f78d58>] dump_stack+0x94/0xc0
[    2.406965] [<9000000000f704e0>] __report_bad_irq+0x54/0x124
[    2.407734] [<9000000000285a34>] note_interrupt+0x294/0x2e8
[    2.408486] [<900000000028230c>] handle_irq_event_percpu+0x68/0x80
[    2.409317] [<9000000000282368>] handle_irq_event+0x44/0xac
[    2.410065] [<9000000000286590>] handle_level_irq+0xe0/0x188
[    2.410826] [<9000000000280f00>] generic_handle_irq+0x24/0x3c
[    2.411605] [<90000000008a2b0c>] extioi_irq_dispatch+0xbc/0x1a0
[    2.412399] [<9000000000280f00>] generic_handle_irq+0x24/0x3c
[    2.413172] [<9000000000f858d8>] do_IRQ+0x18/0x24
[    2.413808] [<90000000002033d4>] except_vec_vi_handler+0xb0/0xdc
```

## 分析 irq domain
上一次装换是什么样子的?
- plat_irq_dispatch : irq_linear_revmap : 实现从 64 + 3 -> 50 + 3 的装换
- extioi_irq_dispatch : irq_linear_revmap : 实现从 2 映射到 virq = 19 的操作

其中，virq = 19 就是 linux irq 了。

## 找到这两个 irq 映射的注册的位置
```c
/**
 * irq_linear_revmap() - Find a linux irq from a hw irq number.
 * @domain: domain owning this hardware interrupt
 * @hwirq: hardware irq number in that domain space
 *
 * This is a fast path alternative to irq_find_mapping() that can be
 * called directly by irq controller code to save a handful of
 * instructions. It is always safe to call, but won't find irqs mapped
 * using the radix tree.
 */
static inline unsigned int irq_linear_revmap(struct irq_domain *domain,
               irq_hw_number_t hwirq)
{
  return hwirq < domain->revmap_size ? domain->linear_revmap[hwirq] : 0;
}
```

- 注册的位置: irq_domain_set_mapping

```c
#0  0x900000000028aeb4 in irq_domain_set_mapping (irq_data=<optimized out>, hwirq=<optimized out>, domain=<optimized out>) at kernel/irq/irqdomain.c:558
#1  irq_domain_associate (domain=0x900000027c03ec00, virq=50, hwirq=0) at kernel/irq/irqdomain.c:559
#2  0x900000000028b060 in irq_domain_associate_many (domain=0x0, irq_base=<optimized out>, hwirq_base=<optimized out>, count=<optimized out>) at kernel/irq/irqdomain.c:579
#3  0x90000000014e6d54 in __loongarch_cpu_irq_init (of_node=<optimized out>) at drivers/irqchip/irq-loongarch-cpu.c:97
#4  0x90000000014e6db0 in loongarch_cpu_irq_init () at drivers/irqchip/irq-loongarch-cpu.c:102
#5  0x9000000000f6e134 in irqchip_init_default () at arch/loongarch/la64/irq.c:279
#6  0x90000000014ace78 in setup_IRQ () at arch/loongarch/la64/irq.c:311
#7  0x90000000014acea4 in arch_init_irq () at arch/loongarch/la64/irq.c:360
#8  0x90000000014ae708 in init_IRQ () at arch/loongarch/kernel/irq.c:59
#9  0x90000000014a8a40 in start_kernel () at init/main.c:636
#10 0x9000000000f7911c in kernel_entry () at arch/loongarch/kernel/head.S:129
Backtrace stopped: frame did not save the PC
```

- [ ] 控制器的映射和操作?
