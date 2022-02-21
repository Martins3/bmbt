# 如何复现工作

- 当前的整个工作都是基于 QEMU 的 895fdf6776076c7cbb6b18c6703a40c4e03a084e 进行的
- 对应的 qemu 版本: 6cdf8c4efa073eac7d5f9894329e2d07743c2955 (tag: v4.2.1)
- musl version : 1e4204d522670a1d8b8ab85f1cfefa960547e8af
- glibc version : 8b0a2fa8312cad636060cf04fbb3d3d29a26322f
- glib version  : 979da1da482cfd11e20f0fc6b9c23dcbc2cd0017 2.70.2

## 编译 Loongarch64 Host 的内核
- huxueshi@172.17.103.58 linux-4.19-loongson
- commit-id : 5cc11160aaf9e6904aa14f8ebc00597d028447cc

从 58 服务器中但是需要修复一个问题:
- 打开 SCHEDSTATS 的选项
  - kernel hacking : collect schedular statistics
- 打开 CONFIG_PCIE_LOONGSON
- 打开调试选项: kernel hacking -> Compile-time checkes and compiler options 打开 DEBUG_INFO 选项

当然也可以使用本仓库中的 ./.config 文件，就不用手动一次次的操作了。

实际上这个内核有两个问题需要修复增加两个 patch
- hwbreak.patch
- non_lazy_lbt.patch

### 找一个可以正确运行 KVM 的 QEMU
clone 这个项目 http://rd.loongson.cn:8081/#/admin/projects/kernel/qemu

- branch : centos8-4.2.0
- commit-id : bc0f70533ca89cc275cfb5b7ab486e45fb70e041
使用项目中的 ./compile_loongarch_kvm.sh 或者下面的(主要是删掉了几个无聊的选项，实际上可以更简单)
```c
mkdir build && cd build
../configure --target-list="loongarch64-softmmu"  --enable-werror --enable-kvm \
            --enable-tcg-interpreter --enable-spice --prefix=/usr --enable-libusb \
            --enable-debug \
            --enable-libiscsi --enable-vnc --enable-vnc-jpeg --enable-vnc-png \
            --enable-libssh --disable-capstone --disable-seccomp --disable-virglrenderer \
            --enable-virtfs
```

## 编译 tiny 内核
- 教程: https://weeraman.com/building-a-tiny-linux-kernel-8c07579ae79d
  - make tinyconfig 将会输出很多参数确认信息，那并不是问题
  - 需要打开的选项
    - TTY
    - prink
    - ELF
    - init RAM disk (initrd)

- 内核下载地址 : https://mirrors.edge.kernel.org/pub/linux/kernel/v4.x/
- 版本 : linux-4.4.142.tar.gz

在 arch/x86/boot/main.c 中将 set_video 注释掉就可以继续向下运行了。

## 复现 xqm 的工作
- 获取 image
  - http://old-releases.ubuntu.com/releases/10.04.0/ 中找到 ubuntu-10.04-server-i386.iso 下载，然后可以安装(tcg / kvm 都可以)

- 编译参数
```sh
mkdir build
../configure --target-list=i386-softmmu --enable-latx --disable-werror
```
- 运行
```sh
./run_xqm.sh
```
