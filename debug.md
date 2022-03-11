# debug.md

if comment `set_pch_pci_action_handler` in `setup_serial_pass_through`,
then we run so much furture.

```c
huxueshi:serial_ioport_pass_write 1 2
i i i i i i i i i i i i i i i i i i i i i i
```
This write enable the interrupt.

```c
kernel_syscall 于 /home/loongson/core/bmbt/env/loongarch/syscall/syscall.c:78 (discriminator 2)
libc_syscall1 于 /home/loongson/core/bmbt/libc/src/internal/../bits/loongarch/syscall_arch.h:40
_Exit 于 /home/loongson/core/bmbt/libc/src/exit/_Exit.c:5
__assert_fail 于 ??:?
a_ctz_32 于 realloc.c:?
tcg_handle_interrupt 于 /home/loongson/core/bmbt/src/accel/tcg/tcg-all.c:9 (discriminator 2)
cpu_interrupt 于 /home/loongson/core/bmbt/src/hw/intc/../../../include/hw/i386/../../exec/../hw/core/cpu.h:457
apic_local_deliver 于 /home/loongson/core/bmbt/src/hw/intc/apic.c:75
apic_deliver_pic_intr 于 /home/loongson/core/bmbt/src/hw/intc/apic.c:103
pic_irq_request 于 /home/loongson/core/bmbt/src/hw/i386/pc.c:505
qemu_set_irq 于 /home/loongson/core/bmbt/src/hw/core/irq.c:16
qemu_irq_raise 于 /home/loongson/core/bmbt/src/hw/intc/../../../include/hw/irq.h:18
pic_update_irq 于 /home/loongson/core/bmbt/src/hw/intc/i8259.c:65
pic_set_irq 于 /home/loongson/core/bmbt/src/hw/intc/i8259.c:104
qemu_set_irq 于 /home/loongson/core/bmbt/src/hw/core/irq.c:16
gsi_handler 于 /home/loongson/core/bmbt/src/hw/i386/pc.c:432
qemu_set_irq 于 /home/loongson/core/bmbt/src/hw/core/irq.c:16
serial_handler 于 /home/loongson/core/bmbt/env/loongarch/passthrough/serial.c:48
handle_pch_pic_irq 于 /home/loongson/core/bmbt/env/loongarch/drivers/irqchip/irq-loongarch-pch-pic.c:128
extioi_irq_dispatch 于 /home/loongson/core/bmbt/env/loongarch/drivers/irqchip/irq-loongarch-extioi.c:183
except_vec_vi_handler 于 /home/loongson/core/bmbt/env/loongarch/kernel/genex.S:84
qemu_mutex_unlock_iothread 于 /home/loongson/core/bmbt/src/qemu/cpus.c:64
address_space_stb 于 /home/loongson/core/bmbt/src/qemu/memory_ldst.c:318
helper_outb 于 /home/loongson/core/bmbt/src/target/i386/misc_helper.c:24
?? ??:0
cpu_loop_exec_tb 于 /home/loongson/core/bmbt/src/accel/tcg/cpu-exec.c:223
cpu_exec 于 /home/loongson/core/bmbt/src/accel/tcg/cpu-exec.c:568
tcg_cpu_exec 于 /home/loongson/core/bmbt/src/qemu/cpus.c:134 (discriminator 1)
qemu_tcg_rr_cpu_thread_fn 于 /home/loongson/core/bmbt/src/qemu/cpus.c:250
qemu_boot 于 /home/loongson/core/bmbt/include/sysemu/sysemu.h:45 (discriminator 1)
test_qemu_init 于 /home/loongson/core/bmbt/src/main.c:144
wip 于 /home/loongson/core/bmbt/src/main.c:156 (discriminator 3)
greatest_run_suite 于 /home/loongson/core/bmbt/src/main.c:159 (discriminator 1)
main 于 /home/loongson/core/bmbt/src/main.c:168
start_kernel 于 /home/loongson/core/bmbt/env/loongarch/init/main.c:31
```

## who caused the intensive interrupt handler
- [ ] kvm or qemu ?
- [x] don't debug serial while using serial to printf

> this is kind of ridiculous
```c
void serial_handler(int hwirq) {
  /* printf("iii "); */
  qemu_irq_lower(serial_irq);
  qemu_irq_raise(serial_irq);
}
```
recursive interrupt is comming if using serial to printf.

- [ ] Whatever takes, the kernel stucked if we unmask the irq in `handle_pch_pic_irq`

## use debugcon
- it's qemu's problem
- but, how and why so intensive !

if intensive interrupt caused by the kvm, which means we didn't seutp the chip correctly !

```c
[179798.069874] kvm [8279]: ext_irq_handler:irq = 2,level = 0
[179798.069892] huxueshi:kvm_ls7a_ioapic_lower 4 0

[179798.069894] huxueshi:kvm_ls7a_ioapic_raise 4 4
[179798.069895] kvm [8279]: msi_irq_handler,2,up
[179798.069896] kvm [8279]: ext_irq_handler:irq = 2,level = 1

[179798.069901] huxueshi:kvm_ls7a_ioapic_lower 4 4
[179798.069902] kvm [8279]: msi_irq_handler,2,down
[179798.069902] kvm [8279]: ext_irq_handler:irq = 2,level = 0

[179798.069919] huxueshi:kvm_ls7a_ioapic_lower 4 0
[179798.069921] huxueshi:kvm_ls7a_ioapic_raise 4 4
[179798.069922] kvm [8279]: msi_irq_handler,2,up
[179798.069923] kvm [8279]: ext_irq_handler:irq = 2,level = 1
[179798.069928] huxueshi:kvm_ls7a_ioapic_lower 4 4
[179798.069929] kvm [8279]: msi_irq_handler,2,down
[179798.069929] kvm [8279]: ext_irq_handler:irq = 2,level = 0
[179798.069945] huxueshi:kvm_ls7a_ioapic_lower 4 0
[179798.069948] huxueshi:kvm_ls7a_ioapic_raise 4 4
[179798.069949] kvm [8279]: msi_irq_handler,2,up
[179798.069949] kvm [8279]: ext_irq_handler:irq = 2,level = 1
[179798.069954] huxueshi:kvm_ls7a_ioapic_lower 4 4
[179798.069955] kvm [8279]: msi_irq_handler,2,down
[179798.069956] kvm [8279]: ext_irq_handler:irq = 2,level = 0
```

- [x] verify it's caused by `ls7a_ioapic_reg_write`?

- unmask will trigger one interrupt immediately
  - [ ] but, why kernel will not tigger it
  - [ ] any time a mask write will trigger one hook

## this is caused by ioremap or
- [x] use UNCAC_BASE to replace base
- [ ] local irq save ?
