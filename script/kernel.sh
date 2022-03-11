#!/usr/bin/env bash
set -eu
USE_MINIMAL_INITRD=false

repo_url=http://pkg.loongnix.cn:8080/loongnix/isos/
img_name=Loongnix-20.mini.loongarch64.rc1.b2.qcow2

# ----------- config ---------------
kernel_dir=~/core/linux-4.19-loongson
qemu_dir=~/core/centos-qemu
initrd=~/core/bmbt/image/initrd.bin
# ----------- config ---------------

kernel=${kernel_dir}/vmlinuz
qemu=${qemu_dir}/build/loongarch64-softmmu/qemu-system-loongarch64
bios=${qemu_dir}/pc-bios/loongarch_bios.bin
img=~/${img_name}

if [[ ! -f ${img} ]]; then
  wget ${repo_url}/${img_name}
  exit 0
fi

launch_gdb=false
debug_kernel=""
debug_qemu=""

while getopts "skd" opt; do
  case $opt in
  s) debug_kernel="-S -s" ;;
  k) launch_gdb=true ;;
  d) debug_qemu="gdb --args" ;;
  *) exit 0 ;;
  esac
done

if [ $launch_gdb = true ]; then
  echo "debug kernel"
  cd ${kernel_dir}
  gdb vmlinux -ex "target remote :1234"
  exit 0
fi

machine_arg="-m 8192M -serial stdio -cpu Loongson-3A5000 -enable-kvm -M loongson7a_v1.0,accel=kvm"
if [[ $USE_MINIMAL_INITRD = true ]]; then
  kernel_arg="-kernel ${kernel} -append \"console=ttyS0 earlyprintk root=/dev/ram rdinit=/hello.out\" -initrd ${initrd}"
  img_arg="-bios ${bios}"
else
  # (user password): (loongson Loongson20)
  # use external kernel is essential, built-in kernel is unable to boot on the latest kvm
  kernel_arg="-kernel ${kernel} -append \"console=ttyS0 earlyprintk root=/dev/vda1 \""
  img_arg="-drive file=${img},if=virtio -bios ${bios}"
fi

cmd="${debug_qemu} ${qemu} ${machine_arg} ${img_arg} ${kernel_arg} ${debug_kernel}"
echo "$cmd"
eval "$cmd"
