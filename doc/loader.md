# loader
- [ ] 了解一下 pc.rom 和 isa-bios 的映射规则
    - [ ] 好像进行了一些有趣的操作，让 bios 映射到两个位置 (map the last 128KB of the BIOS in ISA space)

- [ ] pc_system_firmware_init 中初始化各种 rom

- [ ] 看来需要理解一下 isa 地址空间, 1M 的地址空间是个什么玩意儿
- [ ] ROM 到底是不是 readonly 的啊?

- isa 是 bios 的 128k 的部分，然后 1M 的最后的

- [ ] x86_bios_rom_init : 这个只是添加了文件，文件内容什么时候添加的
    - [ ] rom_add_file_fixed 中会进行读去文件, 但是怎么让 guest 读去啊，直接映射过去的，还是要怎么搞 ?
    - [ ] 在进行 rom reset 的时候，会拷贝内容到系统中去
- [ ] 为什么需要将 pc.bios 映射到 4G pci 空间的最上方
- [ ] pc.rom 是什么?
    - 在 pc_memory_init 

```c
#define PC_ROM_MIN_VGA     0xc0000
#define PC_ROM_MIN_OPTION  0xc8000
#define PC_ROM_MAX         0xe0000
#define PC_ROM_ALIGN       0x800
#define PC_ROM_SIZE        (PC_ROM_MAX - PC_ROM_MIN_VGA)
```

```c
/*
#0  rom_add_file (file=file@entry=0x555556fe3010 "/home/maritns3/core/seabios/out/bios.bin", fw_dir=fw_dir@entry=0x0, addr=addr@entry=4294705152, bootindex=bootindex@entry=-1, option_rom=option_rom@entry=false, mr=mr@entry=0x0, as=0x0) at ../hw/core/loader.c:944
#1  0x0000555555b93560 in x86_bios_rom_init (ms=<optimized out>, default_firmware=<optimized out>, rom_memory=0x555556a132d0, isapc_ram_fw=<optimized out>) at ../hw/i386/x86.c:1110
#2  0x0000555555b9b988 in pc_system_firmware_init (pcms=0x5555568a65e0, rom_memory=0x555556a132d0) at /home/maritns3/core/kvmqemu/include/hw/boards.h:24
#3  0x0000555555b8baa0 in pc_memory_init (pcms=pcms@entry=0x5555568a65e0, system_memory=system_memory@entry=0x5555566a9460, rom_memory=rom_memory@entry=0x555556a132d0,ram_memory=ram_memory@entry=0x7fffffffd290) at ../hw/i386/pc.c:945
#4  0x0000555555b9e281 in pc_init1 (machine=0x5555568a65e0, pci_type=0x555555f08869 "i440FX", host_type=0x555555f3773c "i440FX-pcihost") at ../hw/i386/pc_piix.c:185
#5  0x0000555555a9b934 in machine_run_board_init (machine=0x5555568a65e0) at ../hw/core/machine.c:1272
#6  0x0000555555d09ef4 in qemu_init_board () at ../softmmu/vl.c:2618
#7  qmp_x_exit_preconfig (errp=<optimized out>) at ../softmmu/vl.c:2692
#8  0x0000555555d0d6b0 in qemu_init (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/vl.c:3714
#9  0x0000555555940c8d in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at ../softmmu/main.c:49
*/
```

```txt
address-space: memory
  0000000000000000-ffffffffffffffff (prio 0, i/o): system
    0000000000000000-00000000bfffffff (prio 0, ram): alias ram-below-4g @pc.ram 0000000000000000-00000000bfffffff
    0000000000000000-ffffffffffffffff (prio -1, i/o): pci
      00000000000a0000-00000000000bffff (prio 1, i/o): vga-lowmem
      00000000000c0000-00000000000dffff (prio 1, rom): pc.rom
      00000000000e0000-00000000000fffff (prio 1, rom): alias isa-bios @pc.bios 0000000000020000-000000000003ffff
      00000000fd000000-00000000fdffffff (prio 1, ram): vga.vram
      00000000fe000000-00000000fe003fff (prio 1, i/o): virtio-pci
        00000000fe000000-00000000fe000fff (prio 0, i/o): virtio-pci-common-virtio-9p
        00000000fe001000-00000000fe001fff (prio 0, i/o): virtio-pci-isr-virtio-9p
        00000000fe002000-00000000fe002fff (prio 0, i/o): virtio-pci-device-virtio-9p
        00000000fe003000-00000000fe003fff (prio 0, i/o): virtio-pci-notify-virtio-9p
      00000000febc0000-00000000febdffff (prio 1, i/o): e1000-mmio
      00000000febf0000-00000000febf3fff (prio 1, i/o): nvme-bar0
        00000000febf0000-00000000febf1fff (prio 0, i/o): nvme
        00000000febf2000-00000000febf240f (prio 0, i/o): msix-table
        00000000febf3000-00000000febf300f (prio 0, i/o): msix-pba
      00000000febf4000-00000000febf4fff (prio 1, i/o): vga.mmio
        00000000febf4000-00000000febf417f (prio 0, i/o): edid
        00000000febf4400-00000000febf441f (prio 0, i/o): vga ioports remapped
        00000000febf4500-00000000febf4515 (prio 0, i/o): bochs dispi interface
        00000000febf4600-00000000febf4607 (prio 0, i/o): qemu extended regs
      00000000febf5000-00000000febf5fff (prio 1, i/o): virtio-9p-pci-msix
        00000000febf5000-00000000febf501f (prio 0, i/o): msix-table
        00000000febf5800-00000000febf5807 (prio 0, i/o): msix-pba
      00000000fffc0000-00000000ffffffff (prio 0, rom): pc.bios
```
