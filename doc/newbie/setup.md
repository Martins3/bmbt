# 如何复现工作

- 当前的整个工作都是基于 x86-qemu-mips 的 895fdf6776076c7cbb6b18c6703a40c4e03a084e 进行的
  - ssh://git@172.17.103.58:2222/binarytranslation/x86-qemu-mips.git
  - 实际上，x86-qemu-mips 是在 qemu 上修改的，对应的 qemu 版本: 6cdf8c4efa073eac7d5f9894329e2d07743c2955 (tag: v4.2.1)
- musl version : 1e4204d522670a1d8b8ab85f1cfefa960547e8af
- glibc version : 8b0a2fa8312cad636060cf04fbb3d3d29a26322f
- glib version : 979da1da482cfd11e20f0fc6b9c23dcbc2cd0017 2.70.2
- 第二次更新 LATX : 0abbe319f0c79461ec29ccb7808df4ba183e34e6

## 编译 Loongarch64 Host 的内核
- huxueshi@172.17.103.58 linux-4.19-loongson
- commit-id : 5cc11160aaf9e6904aa14f8ebc00597d028447cc

从 58 服务器中但是需要修复一个问题:
- 打开 SCHEDSTATS 的选项
  - kernel hacking : collect schedular statistics
- 打开 `CONFIG_PCIE_LOONGSON`
- 打开调试选项: kernel hacking -> Compile-time checkes and compiler options 打开 `DEBUG_INFO` 选项

实际上这个内核有两个问题需要修复增加两个 patch
- hwbreak.patch : 否则和 qemu 无法调试
- `non_lazy_lbt.patch` : licun 说内核没有考虑到在虚拟机中运行二进制翻译

当然也可以使用本仓库中的 kernel-config/ls-host.config 文件，就不用手动一次次的操作了。

### 找一个可以正确运行 KVM 的 QEMU
clone 这个项目 http://rd.loongson.cn:8081/#/admin/projects/kernel/qemu

- branch : centos8-4.2.0
- commit-id : bc0f70533ca89cc275cfb5b7ab486e45fb70e041
使用项目中的 ./`compile_loongarch_kvm.sh` 或者下面的(主要是删掉了几个无聊的选项，实际上可以更简单)
```c
mkdir build && cd build
../configure --target-list="loongarch64-softmmu"  --enable-werror --enable-kvm \
            --enable-tcg-interpreter --enable-spice --prefix=/usr --enable-libusb \
            --enable-debug \
            --enable-libiscsi --enable-vnc --enable-vnc-jpeg --enable-vnc-png \
            --enable-libssh --disable-capstone --disable-seccomp --disable-virglrenderer \
            --enable-virtfs
```

这个东西是无法在 x86 上编译的，因为 kvm 的之类的 macro 需要系统的头文件。

- 如果需要调试串口，可以给 QEMU 添加上 `patch/qemu_debugcon.patch`
- 如果需要增加 nvme 设备，需要添加上 `patch/qemu_nvme.patch`

## 生成镜像
在 x86 上 make -f make/image.mk 自动生成:
- initrd
- seabios
- pc-bios
- guest linux kernel

### 编译 tiny 内核
- 教程: https://weeraman.com/building-a-tiny-linux-kernel-8c07579ae79d
  - make tinyconfig 将会输出很多参数确认信息，那并不是问题
  - 需要打开的选项
    - TTY
    - prink
    - ELF
    - init RAM disk (initrd)

- 内核下载地址 : https://mirrors.edge.kernel.org/pub/linux/kernel/v4.x/
- 版本 : linux-4.4.142.tar.gz

在 arch/x86/boot/main.c 中将 `set_video` 注释掉就可以继续向下运行了。

## 复现 xqm 的工作
- 从 http://rd.loongson.cn/ 中下载:
```sh
git clone ssh://huxueshi@rd.loongson.cn:29418/LAT/lat && scp -p -P 29418 huxueshi@rd.loongson.cn:hooks/commit-msg lat/.git/hooks/
git checkout latx-sys
```

在 clone 的过程中
```txt
Unable to negotiate with 192.168.1.123 port 22222: no matching cipher found. Their offer: aes128-cbc,3des-cbc
```
在 .ssh/config 中添加:
```sh
Host rd.loongson.cn
  KexAlgorithms +diffie-hellman-group1-sha1
  KexAlgorithms +diffie-hellman-group14-sha1
```

- 获取 image
  - http://old-releases.ubuntu.com/releases/10.04.0/ 中找到 ubuntu-10.04-server-i386.iso 下载，然后可以安装(tcg / kvm 都可以)

- 编译参数
```sh
mkdir build
cd build
../configure --target-list=i386-softmmu --enable-latx --disable-werror
make -j
```

- 运行
```sh
./run_xqm.sh
```

## 硬件环境搭建
找到 grub 的位置，例如
```sh
sudo vim /boot/grub/grub.cfg
```
参考 grub 中其他的内容
```txt
menuentry 'USB Test' --class loongnix_desktop_20 --class gnu-linux --class gnu --class os --unrestricted $menuentry_id_option 'gnulinux-simple-e0289ba3-2285-418a-9068-e98d5800315f' {
        set gfxpayload=keep
        insmod part_msdos
        insmod ext2
        set root='hd0,gpt1'
        if [ x$feature_platform_search_hint = xy ]; then
          search --no-floppy --fs-uuid --set=root --hint-ieee1275='ieee1275//disk@0,msdos2' --hint-bios=hd0,msdos2 --hint-efi=hd0,msdos2 --hint-baremetal=ahci0,msdos2  fb891a8e-d5a5-4e70-98fd-54bb019bd86b
        else
          search --no-floppy --fs-uuid --set=root fb891a8e-d5a5-4e70-98fd-54bb019bd86b
        fi
        echo    'Loading BMBT'
        linux   /boot/bmbt.bin
        boot
}
```

需要修改的地方:
- set root='hd0,gpt1' : hd0,gpt1 需要进入到 grub 查找，就目前的知识量，实在不行，一个个尝试，或者观察插上 U 盘前后的区别
- 两处 UUID 修改为 U 盘的 UUID
- linux /boot/bmbt.bin

将 bmbt.bin 拷贝到 U 盘中 /boot/bmbt.bin 位置。

## 测试盘准备
BMBT U 盘里写入的只是最基本的 32 位系统，在进入到系统后
- 获取 ubuntu server
- - https://releases.ubuntu.com/16.04/

首先找一个 U 盘或硬盘，使用如下命令：
`sudo ./qemu-system-i386 -drive file=/dev/sda,format=raw -cdrom ~/research/image/ubuntu-16.04.6-server-i386.iso -m 2g -boot d --enable-kvm`
这条命令是制作一个 ubuntu server 的启动盘，从 `-cdrom` 写入到 `-drive`，`file` 需要修改成硬盘或 U 盘在自己电脑上的 partitions。
配置好后进入到 bmbt，然后使用如下命令使用 U 盘的中的 ubuntu service 进行测试，
```txt
mknod /dev/abc b 8 3
mount /dev/abc /mnt
chroot /mnt
```
这里 `b 8 3` 需要使用 `cat /proc/partitions` 查看 U 盘的 major 和 minor。

[^1]: https://unix.stackexchange.com/questions/340844/how-to-enable-diffie-hellman-group1-sha1-key-exchange-on-debian-8-0
