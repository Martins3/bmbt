<p align="center">
  <h1 align="center">BMBT</h1>
  <p align="center">
    <img src ="https://github.com/martins3/BMBT/actions/workflows/compiler-check.yml/badge.svg?branch=dev">
    <img src = "https://github.com/martins3/BMBT/actions/workflows/lint-md.yml/badge.svg?branch=dev">
  </p>
</p>

<!-- vim-markdown-toc GitLab -->

* [实现的思路](#实现的思路)
* [开发环境搭建](#开发环境搭建)
* [获取源码](#获取源码)
* [环境搭建](#环境搭建)
  * [给编译机安装 Loongarch64 Host 的内核](#给编译机安装-loongarch64-host-的内核)
    * [获取源代码](#获取源代码)
    * [打开特定的选项](#打开特定的选项)
    * [增加 patch](#增加-patch)
    * [编译安装](#编译安装)
    * [验证](#验证)
  * [初始化 QEMU](#初始化-qemu)
  * [编译 Guest 资源](#编译-guest-资源)
  * [硬件环境搭建](#硬件环境搭建)
* [代码的来源](#代码的来源)
* [git](#git)
* [调试](#调试)

<!-- vim-markdown-toc -->

## 实现的思路
动机和具体细节参考: https://github.com/Martins3/Bare-Metal-Binary-Translator

其实就是让 QEMU 直接运行在裸机上运行，但是 QEMU 本来是一个用户态程序，需要以来 glib 和 glibc，进一步依赖操作系统的提供的系统调用。
观察到大多数的

## 开发环境搭建
**因为在写 BMBT 的时候龙芯平台上生态环境匮乏，之后也许有更好的选择**

一共需要 3 台机器
- x86 机器: 用于编辑 BMBT 的源码和编译 Guest 资源
- Loongarch 编译机: 用于编译 BMBT 内核，同时执行用户态和 KVM 版本的 BMBT
- Loongarch 测试机: 用于执行裸机版本 BMBT

因为我实在是没有办法在 3A5000 上搭建一个可用的编码环境，这里是我[挣扎过的记录](https://martins3.github.io/loongarch/neovim.html)，因为 luajit 总是崩溃，导致
nvim 几乎没有办法使用。

目前的开发流程是:
1. 在 x86 机器上 ssh 登入 Loongarch 编译机
```sh
la='ssh -X -t loongson@10.90.50.30 "tmux attach || /usr/bin/tmux"'
```

2. 在 x86 机器上重新打开一个中断，将 Loongarch 编译机的 /home/loongson 挂载到 x86 机器的 ~/core/5000 上
```plain
sshfs_la='sshfs loongson@10.90.50.30:/home/loongson ~/core/5000'
```
如果 bmbt 在 /home/loongson/core/bmbt 中，那么在 x86 机器中，进入目录 ~/core/5000/core/bmbt ，打开 nvim，开始工作。

当然，为了可以正确索引，在 Loongarch 编译机中使用 bear make 生成 `compile_commands.json` ，然后将 `compile_commands.json` 中的目录替换。

## 获取源码
```sh
git clone https://github.com/Martins3/bmbt --recursive --shallow-submodules
```
因为 seabios 是仓库的子模块，所以需要选项 `--recursive`

## 环境搭建
环境的搭建并不容易:
1. BMBT 可以在多种模式中运行
    - 将 BMBT 编译为 x86 程序，在 x86 中一直部分代码
    - 在 Loongarch 的用户态中执行，其中可以选择是否以来
    - 在 Loongarch 的 QEMU 中执行
    - 在 Loongarch 的裸机上执行
2. Guest 执行的过程中需要 4 个资源:
    - seabios
    - linuxbootdma : 用于加载 Linux 镜像的，从 QEMU 中继承过来的
    - initrd
    - Linux 内核

### 给编译机安装 Loongarch64 Host 的内核

#### 获取源代码
- 登录 58 服务器: `ssh huxueshi@172.17.103.58` 密码 `longson`
- 仓库为: linux-4.19-loongson
- commit-id : 5cc11160aaf9e6904aa14f8ebc00597d028447cc

#### 打开特定的选项

为了能够正确编译:
- 打开 SCHEDSTATS 的选项
  - kernel hacking : collect schedular statistics
- 打开 `CONFIG_PCIE_LOONGSON`

打开调试选项
- kernel hacking -> Compile-time checkes and compiler options 打开 `DEBUG_INFO` 选项

当然也可以使用本仓库中的 kernel-config/ls-host.config 文件，就不用手动一次次的操作了。

#### 增加 patch
- hwbreak.patch : 否则和 qemu 无法调试
- `non_lazy_lbt.patch` : licun 说内核没有考虑到在虚拟机中运行二进制翻译

#### 编译安装
```sh
sudo make install && sudo make modules_install
```

#### 验证

```sh
uname -a
```

### 初始化 QEMU
clone 这个项目 http://rd.loongson.cn:8081/#/admin/projects/kernel/qemu

- branch : centos8-4.2.0
- commit-id : bc0f70533ca89cc275cfb5b7ab486e45fb70e041

使用项目中的 `./compile_loongarch_kvm.sh` 或者下面的(主要是删掉了几个无聊的选项，实际上可以更简单) 编译
```sh
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

为什么大费周章的选择特定的 QEMU，是因为内核和 QEMU 必须对应，其 KVM 才可以正确运行和调试。

### 编译 Guest 资源
编译 Guest 资源被自动化了，但是其内容在 make/image.mk 中

其中 tiny kernel 如何编译的解释如下:
- 教程: https://weeraman.com/building-a-tiny-linux-kernel-8c07579ae79d
  - make tinyconfig 将会输出很多参数确认信息，那并不是报错。
  - 需要打开的选项
    - TTY
    - prink
    - ELF
    - init RAM disk (initrd)

```sh
make -f make/image.mk
make run -j
```
如果你是按照 [开发环境搭建](#开发环境搭建) 来挂载 sshfs 的，那么 Guest 内核的源码是自动下载到 ~/core 目录的，具体原因参考 make/kernel.mk 的源码:

### 硬件环境搭建
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
- set root='hd0,gpt1' : hd0,gpt1 需要进入到 grub 查找，实在不行，一个个尝试，或者观察插上 U 盘前后的区别
- 两处 UUID 修改为 U 盘的 UUID
- 记得是 linux /boot/bmbt.bin

同时，需要将
- 在 Makefile 中，将 `USB_DIR` 修改为正确的 UUID
- `machine_initfn` : 将其中的 UUID 修改为**硬盘**的 UUID

使用 `make usb`，也许是 `sudo make sub`，可以自动将 bmbt.bin 拷贝到 U 盘中 /boot/bmbt.bin 位置。

## 代码的来源
- 当前的整个工作都是基于 x86-qemu-mips 的 895fdf6776076c7cbb6b18c6703a40c4e03a084e 进行的
  - ssh://git@172.17.103.58:2222/binarytranslation/x86-qemu-mips.git
  - 实际上，x86-qemu-mips 是在 qemu 上修改的，对应的 qemu 版本: 6cdf8c4efa073eac7d5f9894329e2d07743c2955 (tag: v4.2.1)
- musl version : 1e4204d522670a1d8b8ab85f1cfefa960547e8af
- glibc version : 8b0a2fa8312cad636060cf04fbb3d3d29a26322f
- glib version : 979da1da482cfd11e20f0fc6b9c23dcbc2cd0017 2.70.2
- 第二次更新 LATX : 0abbe319f0c79461ec29ccb7808df4ba183e34e6

## git
参考 [开发者手册](./CONTRIBUTING.md)

## 调试
参考 [开发者手册](./doc/debug.md)

## LAT
https://github.com/lat-opensource/lat

## 文档
https://github.com/Martins3/bmbt-doc

[^1]: https://unix.stackexchange.com/questions/340844/how-to-enable-diffie-hellman-group1-sha1-key-exchange-on-debian-8-0
