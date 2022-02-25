#!/bin/bash
BASE_DIR=$(pwd)
BASE_DIR=/home/loongson/core/bmbt

run_tiny_kernel=true
kernel=~/xqm_images/vmlinuz-2.6.32

kernel=${BASE_DIR}/image/bzImage.bin
initrd=${BASE_DIR}/image/initrd.bin
qemu=~/core/lat/build/i386-softmmu/qemu-system-i386
bios=${BASE_DIR}/seabios/out/bios.bin


arg_bios="-chardev file,path=/tmp/seabios.log,id=seabios -device isa-debugcon,iobase=0x402,chardev=seabios -bios ${bios}"
arg_xqm="-latx select,cpjl=on,intblink=on,njc=on,largecc=on -accel tcg,tb-size=2048"
arg_img="-hda ~/xqm_images/ubuntu10s.test.img.full"
arg_kernel="-kernel ${kernel}"
arg_kernel_cmdline="-append \"console=ttyS0 \""
arg_initrd=""

if [[ $run_tiny_kernel == true ]]; then
	arg_kernel_cmdline="-append \"console=ttyS0 earlyprintk=serial root=/dev/ram rdinit=/hello.out\""
	arg_initrd="-initrd ${initrd}"
	arg_img=""
fi

cmd="${qemu} ${arg_img} ${arg_kernel} ${arg_xqm} ${arg_bios} ${arg_kernel_cmdline} ${arg_initrd} --nographic"
echo "${cmd}"
eval "${cmd}"

# root=/dev/sda1 ro init=/bin/bash tsc=reliable rw
