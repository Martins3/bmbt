#!/bin/bash

use32bit=true # compile a 32bit or 64bit
environment=loongarch
environment=x86

WORK_DIR=~/core/
busybox_img=${WORK_DIR}/busybox.tar.bz2
BUILDS=${WORK_DIR}/busybox
initrd=${BUILDS}/initramfs.cpio.gz

if [[ ! -f $busybox_img ]]; then
  wget https://busybox.net/downloads/busybox-1.35.0.tar.bz2 -O $busybox_img
fi

if [[ ! -d $BUILDS ]]; then
  mkdir $BUILDS
  tar -xvf $busybox_img -C ${BUILDS} --strip-components 1
fi

if [[ $environment != loongarch ]]; then
  if [[ $use32bit = true ]]; then
    initramfs=initramfs32
    qemu=$HOME/core/tcgqemu/32bit/i386-softmmu/qemu-system-i386
    kernel=$HOME/core/bmbt_linux/arch/x86/boot/bzImage
  else
    initramfs=initramfs64
    qemu="qemu-system-x86_64"
    kernel=$HOME/ubuntu-linux/arch/x86/boot/bzImage
  fi
else
  initramfs=initramfs64
  qemu=$HOME/core/centos-qemu/build/loongarch64-softmmu/qemu-system-loongarch64
  kernel=$HOME/core/linux-4.19-loongson/arch/x86/boot/bzImage
fi

sure() {
  read -r -p "$1? (y/n)" yn
  case $yn in
  [Yy]*) return ;;
  [Nn]*) exit ;;
  *) echo "Please answer yes or no." ;;
  esac
}

cd $BUILDS || exit 0

if [[ ! -f ${BUILDS}/.config ]]; then
  echo "Modify the ->Setting<-"
  echo "----------------"
  echo "[x] Build static binary (no shared libs)      <-----------"
  echo "[ ]   Build position independent executable"
  echo "[ ] Force NOMMU build"
  echo "[ ] Build shared libbusybox"
  echo "()  Cross compiler prefix"
  echo "()  Path to sysroot"
  echo "(-m32 -march=i386)  Additional CFLAGS        <------------"
  echo "(-m32)  Additional LDFLAGS                   <------------"
  echo "----------------"
  sure "do you remember it ?"
  make menuconfig
  sure "continue to make"
fi

INITRAMFS_BUILD=$BUILDS/$initramfs

if [[ ! -d "$BUILDS"/_install ]]; then
  make -j && make install -j
fi

if [[ ! -d $INITRAMFS_BUILD ]]; then
  mkdir -p $INITRAMFS_BUILD
  cd $INITRAMFS_BUILD || exit 0
  mkdir -p bin sbin etc proc sys usr/bin usr/sbin
  cp -a "$BUILDS"/_install/* .

  cat <<'EOF' >$INITRAMFS_BUILD/init
#!/bin/sh

mount -t proc none /proc
mount -t sysfs none /sys

mknod /dev/ttyS0 c 4 64
mknod /dev/tty c 5 0
mknod dev/tty1 c 4 1
mknod dev/tty2 c 4 2
mknod dev/tty3 c 4 3
mknod dev/tty4 c 4 4

cat <<!


Boot took $(cut -d' ' -f1 /proc/uptime) seconds

        _       _     __ _
  /\/\ (_)_ __ (_)   / /(_)_ __  _   ___  __
 /    \| | '_ \| |  / / | | '_ \| | | \ \/ /
/ /\/\ \ | | | | | / /__| | | | | |_| |>  <
\/    \/_|_| |_|_| \____/_|_| |_|\__,_/_/\_\


Welcome to mini_linux


!
# ifup eth0

mkdir mnt
mknod /dev/nvme b 259 0
mount /dev/nvme /mnt

setsid cttyhack /bin/sh
exec /bin/sh
EOF

  chmod +x $INITRAMFS_BUILD/init
  mkdir -p $INITRAMFS_BUILD/etc/network/

  cat <<_EOF_ >$INITRAMFS_BUILD/etc/network/interfaces
auto eth0
iface eth0 inet dhcp
_EOF_

  find . -print0 | cpio --null -ov --format=newc | gzip -9 >$BUILDS/initramfs.cpio.gz
fi

if [[ $environment != loongarch ]]; then
  cp $BUILDS/initramfs.cpio.gz "$HOME"/core/5000/core/bmbt/image/initrd.bin
  # root=/dev/ram rdinit=/hello.out
  # run this in x86 kvm to verify the busybox img
  $qemu -kernel "$kernel" -initrd $initrd -nographic -append "console=ttyS0" -m 3g
else
  QEMU_DIR=/home/loongson/core/centos-qemu
  LA_BIOS=${QEMU_DIR}/pc-bios/loongarch_bios.bin
  NETWORK_CONFIG="-netdev user,id=n1,ipv6=off -device e1000e,netdev=n1"
  arg_kernel_cmdline="-append \"console=ttyS0 earlyprintk=serial\""
  arg_initrd="-initrd ${initrd}"
  cmd="${qemu} ${NETWORK_CONFIG} -m 8G -cpu Loongson-3A5000 -nographic -bios ${LA_BIOS} --enable-kvm -M loongson7a_v1.0,accel=kvm -kernel ${kernel} ${arg_kernel_cmdline} ${arg_initrd}"
  echo "$cmd"
  eval "$cmd"
fi
