## 试图理解为什么没有输出的呀

似乎表现出现的形式是 uart_write 总是什么都不做的样子

```plain
#0  __start_tx (port=<optimized out>) at drivers/tty/serial/8250/8250_port.c:1586
#1  serial8250_start_tx (port=0x9000000001666b88 <serial8250_ports>) at drivers/tty/serial/8250/8250_port.c:1675
#2  0x90000000009d3780 in uart_write (tty=0x900000027d001000, buf=0x900000027d001409 "\nain", count=0) at drivers/tty/serial/serial_core.c:611
#3  0x90000000009b370c in process_output_block (nr=<optimized out>, buf=<optimized out>, tty=<optimized out>) at drivers/tty/n_tty.c:593
#4  n_tty_write (tty=0x900000027d001000, file=0x900000027d1af800, buf=<optimized out>, nr=10) at drivers/tty/n_tty.c:2331
#5  0x90000000009af038 in do_tty_write (count=10, buf=0x12000702b <error: Cannot access memory at address 0x12000702b>, file=<optimized out>, tty=<optimized out>, writ
e=<optimized out>) at drivers/tty/tty_io.c:949
#6  tty_write (file=0x900000027d1af800, buf=0x12000702b <error: Cannot access memory at address 0x12000702b>, count=10, ppos=<optimized out>) at drivers/tty/tty_io.c:1
044
#7  0x9000000000413244 in do_loop_readv_writev (flags=<optimized out>, type=<optimized out>, ppos=<optimized out>, iter=<optimized out>, filp=<optimized out>) at fs/re
ad_write.c:704
#8  do_loop_readv_writev (flags=<optimized out>, type=<optimized out>, ppos=<optimized out>, iter=<optimized out>, filp=<optimized out>) at fs/read_write.c:688
#9  do_iter_write (flags=<optimized out>, pos=<optimized out>, iter=<optimized out>, file=<optimized out>) at fs/read_write.c:962
#10 do_iter_write (file=0x900000027d1af800, iter=0x900000027cd33d88, pos=0x900000027cd33e68, flags=<optimized out>) at fs/read_write.c:941
#11 0x9000000000413308 in vfs_writev (file=0x900000027d1af800, vec=<optimized out>, vlen=<optimized out>, pos=0x900000027cd33e68, flags=0) at fs/read_write.c:1005
#12 0x9000000000413410 in do_writev (fd=<optimized out>, vec=0xfffb84f1f0, vlen=2, flags=0) at fs/read_write.c:1040
#13 0x900000000020f338 in handle_sys () at arch/loongarch/kernel/scall64-64.S:95
#14 0x0000000120003a84 in ?? ()
Backtrace stopped: frame did not save the PC
```

## log
```plain
[    0.000000] Linux version 4.19.190+ (loongson@loongson-pc) (gcc version 8.3.0 (Loongnix 8.3.0-6.lnd.vec.27)) #10 SMP Wed Jan 12 17:09:22 CST 2022
[    0.000000] Primary instruction cache 64kB, 4-way, VIPT, linesize 64 bytes.
[    0.000000] Primary data cache 64kB, 4-way, VIPT, no aliases, linesize 64 bytes
[    0.000000] Unified victim cache 256kB 16-way, linesize 64 bytes.
[    0.000000] Unified secondary cache 16384kB 16-way, linesize 64 bytes.
[    0.000000] Generic 64-bit Loongson Processor probed
[    0.000000] CPU0 revision is: 0014c010 (Loongson-64bit)
[    0.000000] FPU0 revision is: 00000000
[    0.000000] BPI0 with boot flags 4d454d.
[    0.000000] Fatal error, incorrect BPI version: 0
[    0.000000] efi:  SMBIOS 3.0=0x90000000bfb50000  ACPI 2.0=0xbfc30000
[    0.000000] ACPI: Early table checksum verification disabled
[    0.000000] ACPI: RSDP 0x00000000BFC30000 000024 (v02 LOONGS)
[    0.000000] ACPI: XSDT 0x00000000BFC20000 000044 (v01 LOONGS TP-R00   00000001      01000013)
[    0.000000] ACPI: FACP 0x00000000BC600000 0000F4 (v03 LOONGS TP-R00   00000001 BXPC 00000001)
[    0.000000] ACPI: DSDT 0x00000000BC610000 0010A5 (v01 LARCH  LOONGARC 00000001 BXPC 00000001)
[    0.000000] ACPI: FACS 0x00000000BC620000 000040
[    0.000000] ACPI: APIC 0x00000000BC5F0000 00004C (v01 LARCH  LOONGARC 00000001 BXPC 00000001)
[    0.000000] ACPI: SRAT 0x00000000BC5E0000 000090 (v03 LARCH  LOONGARC 00000001 BXPC 00000001)
[    0.000000] ACPI: MCFG 0x00000000BC5D0000 00003C (v01 BOCHS  BXPCMCFG 00000001 BXPC 00000001)
[    0.000000] PCH_PIC[0]: pch_pic_id 0, version 0, address 0x10000000, IRQ 64-95
[    0.000000] SRAT: PXM 0 -> CPU 0x00 -> Node 0
[    0.000000] ACPI: SRAT: Node 0 PXM 0 [mem 0x00000000-0x0fffffff]
[    0.000000] ACPI: SRAT: Node 0 PXM 0 [mem 0x90000000-0x27fffffff]
[    0.000000] Debug: node_id:0, mem_start:0x200000, mem_size:0xee00000 Bytes
[    0.000000]        start_pfn:0x80, end_pfn:0x3c00, num_physpages:0x3b80
[    0.000000] Debug: mem_type:2, mem_start:0xf000000, mem_size:0x1000000 Bytes
[    0.000000] Debug: node_id:0, mem_start:0xc0000000, mem_size:0x1c0000000 Bytes
[    0.000000]        start_pfn:0x30000, end_pfn:0xa0000, num_physpages:0x73b80
[    0.000000] Debug: node_id:0, mem_start:0x90000000, mem_size:0x2c5d0000 Bytes
[    0.000000]        start_pfn:0x24000, end_pfn:0x2f174, num_physpages:0x7ecf4
[    0.000000] Debug: node_id:0, mem_start:0xbc630000, mem_size:0x32d0000 Bytes
[    0.000000]        start_pfn:0x2f18c, end_pfn:0x2fe40, num_physpages:0x7f9a8
[    0.000000] Debug: node_id:0, mem_start:0xbfc40000, mem_size:0x3c0000 Bytes
[    0.000000]        start_pfn:0x2ff10, end_pfn:0x30000, num_physpages:0x7fa98
[    0.000000] Debug: mem_type:2, mem_start:0xbf900000, mem_size:0x320000 Bytes
[    0.000000] Debug: mem_type:2, mem_start:0x27fff0000, mem_size:0x5000 Bytes
[    0.000000] Node0's addrspace_offset is 0x0
[    0.000000] Node0: start_pfn=0x80, end_pfn=0xa0000
[    0.000000] SMBIOS 3.0.0 present.
[    0.000000] DMI: Loongson KVM, BIOS 0.0.0 02/06/2015
[    0.000000] CpuClock = 2000000000
[    0.000000] The BIOS Version: 0.0.0
[    0.000000] Initial ramdisk at: 0x900000027fff0000 (21502 bytes)
[    0.000000] software IO TLB: mapped [mem 0x016c4000-0x056c4000] (64MB)
[    0.000000] PM: Registered nosave memory: [mem 0x013ac000-0x013affff]
[    0.000000] SMP: Allowing 1 CPUs, 0 hotplug CPUs
[    0.000000] Zone ranges:
[    0.000000]   DMA32    [mem 0x0000000000200000-0x00000000ffffffff]
[    0.000000]   Normal   [mem 0x0000000100000000-0x000000027fffffff]
[    0.000000] Movable zone start for each node
[    0.000000] Early memory node ranges
[    0.000000]   node   0: [mem 0x0000000000200000-0x000000000effffff]
[    0.000000]   node   0: [mem 0x0000000090000000-0x00000000bc5cffff]
[    0.000000]   node   0: [mem 0x00000000bc630000-0x00000000bf8fffff]
[    0.000000]   node   0: [mem 0x00000000bfc40000-0x000000027fffffff]
[    0.000000] Zeroed struct page in unavailable ranges: 1384 pages
[    0.000000] Initmem setup node 0 [mem 0x0000000000200000-0x000000027fffffff]
[    0.000000] percpu: Embedded 7 pages/cpu s63632 r8192 d42864 u33554432
[    0.000000] CPU0 __my_cpu_offset: 4a6c000
[    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 520861
[    0.000000] Policy zone: Normal
[    0.000000] Kernel command line:  rd_start=0x900000027fff0000 rd_size=21502 console=ttyS0 earlyprintk root=/dev/ram rdinit=/hello.out e1000e.InterruptThrottleRate=4
,4,4,4
[    0.000000] Memory: 8180704K/8366464K available (14203K kernel code, 1416K rwdata, 3852K rodata, 672K init, 1093K bss, 185760K reserved, 0K cma-reserved)
[    0.000000] SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=1, Nodes=1
[    0.000000] rcu: Hierarchical RCU implementation.
[    0.000000] rcu:     RCU restricting CPUs from NR_CPUS=64 to nr_cpu_ids=1.
[    0.000000] rcu: Adjusting geometry for rcu_fanout_leaf=16, nr_cpu_ids=1
[    0.000000] NR_IRQS: 4160, nr_irqs: 4160, preallocated irqs: 16
[    0.000000] Support EXT interrupt.
[    0.000000] Constant clock event device register
[    0.000000] clocksource: Constant: mask: 0xffffffffffffffff max_cycles: 0x171024e7e0, max_idle_ns: 440795205315 ns
[    0.000002] sched_clock: 64 bits at 100MHz, resolution 10ns, wraps every 4398046511100ns
[    0.000003] Constant clock source device register
[    0.010897] Console: colour dummy device 80x25
[    0.010942] ACPI: Core revision 20180810
[    0.011063] Calibrating delay loop (skipped), value calculated using timer frequency.. 200.00 BogoMIPS (lpj=400000)
[    0.011067] pid_max: default: 32768 minimum: 301
[    0.011111] Security Framework initialized
[    0.011117] AppArmor: AppArmor disabled by boot time parameter
[    0.012525] Dentry cache hash table entries: 1048576 (order: 9, 8388608 bytes)
[    0.022741] Inode-cache hash table entries: 524288 (order: 8, 4194304 bytes)
[    0.022785] Mount-cache hash table entries: 16384 (order: 3, 131072 bytes)
[    0.022804] Mountpoint-cache hash table entries: 16384 (order: 3, 131072 bytes)
[    0.023467] Performance counters: loongarch/loongson64 PMU enabled, 4 64-bit counters available to each CPU.
[    0.023528] rcu: Hierarchical SRCU implementation.
[    0.023647] smp: Bringing up secondary CPUs ...
[    0.023649] smp: Brought up 1 node, 1 CPU
[    0.024006] devtmpfs: initialized
[    0.024697] random: get_random_u32 called from bucket_table_alloc.isra.9+0x94/0x20c with crng_init=0
[    0.024837] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 7645041785100000 ns
[    0.024846] futex hash table entries: 256 (order: 0, 16384 bytes)
[    0.024889] xor: measuring software checksum speed
[    0.063371]    8regs     : 11720.000 MB/sec
[    0.103364]    8regs_prefetch: 11688.000 MB/sec
[    0.143365]    32regs    : 11712.000 MB/sec
[    0.183365]    32regs_prefetch: 11684.000 MB/sec
[    0.183367] xor: using function: 8regs (11720.000 MB/sec)
[    0.183776] NET: Registered protocol family 16
[    0.184021] audit: initializing netlink subsys (disabled)
[    0.184502] cpuidle: using governor menu
[    0.184635] ACPI: bus type PCI registered
[    0.191398] audit: type=2000 audit(1641990516.180:1): state=initialized audit_enabled=0 res=1
[    0.195256] HugeTLB registered 32.0 MiB page size, pre-allocated 0 pages
[    0.263367] raid6: int64x1  gen()  2616 MB/s
[    0.331369] raid6: int64x1  xor()  1590 MB/s
[    0.399367] raid6: int64x2  gen()  3956 MB/s
[    0.467371] raid6: int64x2  xor()  2165 MB/s
[    0.535371] raid6: int64x4  gen()  3772 MB/s
[    0.603375] raid6: int64x4  xor()  2284 MB/s
[    0.671375] raid6: int64x8  gen()  2772 MB/s
[    0.739380] raid6: int64x8  xor()  2325 MB/s
[    0.739381] raid6: using algorithm int64x2 gen() 3956 MB/s
[    0.739383] raid6: .... xor() 2165 MB/s, rmw enabled
[    0.739384] raid6: using intx1 recovery algorithm
[    0.740177] ACPI: Added _OSI(Module Device)
[    0.740179] ACPI: Added _OSI(Processor Device)
[    0.740181] ACPI: Added _OSI(3.0 _SCP Extensions)
[    0.740183] ACPI: Added _OSI(Processor Aggregator Device)
[    0.740185] ACPI: Added _OSI(Linux-Dell-Video)
[    0.740187] ACPI: Added _OSI(Linux-Lenovo-NV-HDMI-Audio)
[    0.740968] ACPI: 1 ACPI AML tables successfully acquired and loaded
[    0.741459] ACPI: Interpreter enabled
[    0.741476] ACPI: (supports S0 S5)
[    0.741479] ACPI: Using IOAPIC for interrupt routing
[    0.741540] ACPI: MCFG table detected, 1 entries
[    0.741612] ACPI: Enabled 2 GPEs in block 00 to 1F
[    0.742509] ACPI: PCI Root Bridge [PCI0] (domain 0000 [bus 00-7f])
[    0.742517] acpi PNP0A08:00: _OSC: OS supports [ExtendedConfig ASPM ClockPM Segments MSI]
[    0.742640] acpi PNP0A08:00: _OSC: platform does not support [LTR]
[    0.742732] acpi PNP0A08:00: _OSC: OS now controls [PCIeHotplug SHPCHotplug PME AER PCIeCapability]
[    0.742945] acpi PNP0A08:00: ECAM at [mem 0x20000000-0x27ffffff] for [bus 00-7f]
[    0.742962] Remapped I/O 0x0000000018004000 to [io  0x4000-0xffff window]
[    0.743015] PCI host bridge to bus 0000:00
[    0.743018] pci_bus 0000:00: root bus resource [io  0x4000-0xffff window]
[    0.743021] pci_bus 0000:00: root bus resource [mem 0x40000000-0x7fffffff window]
[    0.743023] pci_bus 0000:00: root bus resource [bus 00-7f]
[    0.766398] pci 0000:00:02.0: BAR 0: assigned to efifb
[    0.766623] pci 0000:00:02.0: BAR 0: assigned [mem 0x40000000-0x41ffffff pref]
[    0.766640] pci 0000:00:01.0: BAR 4: assigned [mem 0x42000000-0x42003fff 64bit pref]
[    0.766675] pci 0000:00:01.0: BAR 1: assigned [mem 0x42004000-0x42004fff]
[    0.766686] pci 0000:00:02.0: BAR 1: assigned [mem 0x42005000-0x42005fff]
[    0.767647] pci 0000:00:01.0: BAR 0: assigned [io  0x4000-0x401f]
[    0.768209] pci 0000:00:02.0: vgaarb: setting as boot VGA device
[    0.768213] pci 0000:00:02.0: vgaarb: VGA device added: decodes=io+mem,owns=io+mem,locks=none
[    0.768216] pci 0000:00:02.0: vgaarb: bridge control possible
[    0.768218] vgaarb: loaded
[    0.768336] SCSI subsystem initialized
[    0.768671] ACPI: bus type USB registered
[    0.768715] usbcore: registered new interface driver usbfs
[    0.768738] usbcore: registered new interface driver hub
[    0.768749] usbcore: registered new device driver usb
[    0.768871] pps_core: LinuxPPS API ver. 1 registered
[    0.768873] pps_core: Software ver. 5.3.6 - Copyright 2005-2007 Rodolfo Giometti <giometti@linux.it>
[    0.768889] PTP clock support registered
[    0.768945] Registered efivars operations
[    0.769240] Advanced Linux Sound Architecture Driver Initialized.
[    0.769548] clocksource: Switched to clocksource Constant
[    0.796136] VFS: Disk quotas dquot_6.6.0
[    0.796155] VFS: Dquot-cache hash table entries: 2048 (order 0, 16384 bytes)
[    0.796187] FS-Cache: Loaded
[    0.796231] pnp: PnP ACPI init
[    0.796367] pnp: PnP ACPI: found 1 devices
[    0.798878] NET: Registered protocol family 2
[    0.799036] tcp_listen_portaddr_hash hash table entries: 4096 (order: 2, 65536 bytes)
[    0.799056] TCP established hash table entries: 65536 (order: 5, 524288 bytes)
[    0.799163] TCP bind hash table entries: 65536 (order: 6, 1048576 bytes)
[    0.799346] TCP: Hash tables configured (established 65536 bind 65536)
[    0.799380] UDP hash table entries: 4096 (order: 3, 131072 bytes)
[    0.799401] UDP-Lite hash table entries: 4096 (order: 3, 131072 bytes)
[    0.799451] NET: Registered protocol family 1
[    0.800281] RPC: Registered named UNIX socket transport module.
[    0.800283] RPC: Registered udp transport module.
[    0.800285] RPC: Registered tcp transport module.
[    0.800286] RPC: Registered tcp NFSv4.1 backchannel transport module.
[    0.800408] Trying to unpack rootfs image as initramfs...
[    0.800794] Freeing initrd memory: 16K
[    0.800896] ACPI S3 is not support!
[    0.801481] Initialise system trusted keyrings
[    0.801559] workingset: timestamp_bits=40 max_order=19 bucket_order=0
[    0.803968] squashfs: version 4.0 (2009/01/31) Phillip Lougher
[    0.804290] NFS: Registering the id_resolver key type
[    0.804297] Key type id_resolver registered
[    0.804298] Key type id_legacy registered
[    0.804302] nfs4filelayout_init: NFSv4 File Layout Driver Registering...
[    0.804307] nfs4flexfilelayout_init: NFSv4 Flexfile Layout Driver Registering...
[    0.804309] Installing knfsd (copyright (C) 1996 okir@monad.swb.de).
[    0.804610] SGI XFS with security attributes, no debug enabled
[    0.806102] 9p: Installing v9fs 9p2000 file system support
[    0.965546] random: fast init done
[    1.097053] Key type asymmetric registered
[    1.097056] Asymmetric key parser 'x509' registered
[    1.097068] Block layer SCSI generic (bsg) driver version 0.4 loaded (major 250)
[    1.097117] io scheduler noop registered
[    1.097148] io scheduler cfq registered (default)
[    1.097149] io scheduler mq-deadline registered
[    1.097151] io scheduler kyber registered
[    1.097286] shpchp: Standard Hot Plug PCI Controller Driver version: 0.4
[    1.097324] efifb: probing for efifb
[    1.097335] efifb: framebuffer at 0x40000000, using 1888k, total 1875k
[    1.097337] efifb: mode is 800x600x32, linelength=3200, pages=1
[    1.097338] efifb: scrolling: redraw
[    1.097340] efifb: Truecolor: size=8:8:8:8, shift=24:16:8:0
[    1.098663] Console: switching to colour frame buffer device 100x37
[    1.099512] fb0: EFI VGA frame buffer device
[    1.099597] input: Power Button as /devices/LNXSYSTM:00/LNXPWRBN:00/input/input0
[    1.099682] ACPI: Power Button [PWRF]
[    1.100143] virtio-pci 0000:00:01.0: enabling device (0005 -> 0007)
[    1.103267] Serial: 8250/16550 driver, 16 ports, IRQ sharing enabled
[    1.127244] 00:00: ttyS0 at MMIO 0x1fe001e0 (irq = 19, base_baud = 115200) is a 16550A
[    1.274437] console [ttyS0] enabled
[    1.278214] brd: module loaded
[    1.280871] loop: module loaded
[    1.281419] megaraid cmm: 2.20.2.7 (Release Date: Sun Jul 16 00:01:03 EST 2006)
[    1.282481] megaraid: 2.20.5.1 (Release Date: Thu Nov 16 15:32:35 EST 2006)
[    1.283309] megasas: 07.706.03.00-rc1
[    1.283777] mpt3sas version 26.100.00.00 loaded
[    1.284991] libphy: Fixed MDIO Bus: probed
[    1.285501] e1000: Intel(R) PRO/1000 Network Driver - version 7.3.21-k8-NAPI
[    1.286281] e1000: Copyright (c) 1999-2006 Intel Corporation.
[    1.287177] e1000e: Intel(R) PRO/1000 Network Driver - 3.2.6-k
[    1.287856] e1000e: Copyright(c) 1999 - 2015 Intel Corporation.
[    1.288561] igb: Intel(R) Gigabit Ethernet Network Driver - version 5.4.0-k
[    1.289357] igb: Copyright (c) 2007-2014 Intel Corporation.
[    1.290101] ixgbe: Intel(R) 10 Gigabit PCI Express Network Driver - version 5.1.0-k
[    1.290971] ixgbe: Copyright (c) 1999-2016 Intel Corporation.
[    1.291805] ixgb: Intel(R) PRO/10GbE Network Driver - version 1.0.135-k2-NAPI
[    1.292594] ixgb: Copyright (c) 1999-2008 Intel Corporation.
[    1.293447] ehci_hcd: USB 2.0 'Enhanced' Host Controller (EHCI) Driver
[    1.294185] ehci-pci: EHCI PCI platform driver
[    1.294731] ehci-platform: EHCI generic platform driver
[    1.295356] ohci_hcd: USB 1.1 'Open' Host Controller (OHCI) Driver
[    1.296077] ohci-pci: OHCI PCI platform driver
[    1.296600] ohci-platform: OHCI generic platform driver
[    1.297326] i8042: PNP: No PS/2 controller found.
[    1.298014] mousedev: PS/2 mouse device common for all mice
[    1.298791] rtc rtc0: invalid alarm value: 1900-1-12 0:0:0
[    1.299558] ls2x-rtc LOON0001:00: rtc core: registered LOON0001:00 as rtc0
[    1.300613] i2c /dev entries driver
[    1.301178] device-mapper: ioctl: 4.39.0-ioctl (2018-04-03) initialised: dm-devel@redhat.com
[    1.302278] hidraw: raw HID events driver (C) Jiri Kosina
[    1.303051] usbcore: registered new interface driver usbhid
[    1.303695] usbhid: USB HID core driver
[    1.304199] loongson_generic_laptop: Not yet supported Loongson Generic Laptop/All-in-one detected!
[    1.305782] Initializing XFRM netlink socket
[    1.306442] NET: Registered protocol family 10
[    1.307246] Segment Routing with IPv6
[    1.307723] sit: IPv6, IPv4 and MPLS over IPv4 tunneling driver
[    1.308600] NET: Registered protocol family 17
[    1.309133] NET: Registered protocol family 15
[    1.309790] NET: Registered protocol family 21
[    1.310320] 9pnet: Installing 9P2000 support
[    1.310797] NET: Registered protocol family 37
[    1.311324] Key type dns_resolver registered
[    1.312041] Loading compiled-in X.509 certificates
[    1.312969] Btrfs loaded, crc32c=crc32c-generic
[    1.313588] ls2x-rtc LOON0001:00: setting system clock to 2022-01-12 12:28:37 UTC (1641990517)
[    1.314693] ALSA device list:
[    1.315051]   No soundcards found.
[    1.315786] Freeing unused kernel memory: 672K
[    1.316307] This architecture does not have kernel memory protection.
[    1.317044] Run /hello.out as init process
```

## 如何获取基本的 apci 信息的
- acpi_locate_initial_tables
  - acpi_initialize_tables
