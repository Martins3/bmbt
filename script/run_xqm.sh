#!/bin/bash
LATX=true
run_tiny_kernel=true
debug_qemu=""
show_seabios_log=false

BASE_DIR=$(pwd)
BASE_DIR=/home/loongson/core/bmbt

kernel=~/xqm_images/vmlinuz-2.6.32
kernel=${BASE_DIR}/image/bzImage.bin
initrd=${BASE_DIR}/image/initrd.bin

function usage() {
  echo "Usage :   [options] [--]

  Options:
  -h|help       Display this message
  -x            RUN xqm
  -l            Run latx"
}

while getopts "hxld" opt; do
  case $opt in
  h)
    usage
    exit 0
    ;;
  x) LATX=false ;;
  l) LATX=true ;;
  d) debug_qemu="gdb --args" ;;
  *)
    echo -e "\n  Option does not exist : OPTARG\n"
    usage
    exit 1
    ;;
  esac # --- end of case ---
done
shift $((OPTIND - 1))

if [[ $LATX = true ]]; then
  qemu=~/core/lat/build/i386-softmmu/qemu-system-i386
  arg_xqm="-latx select,cpjl=on,intblink=on,njc=on,largecc=on -accel tcg,tb-size=2048"
else
  qemu=~/ld/x86-qemu-mips/build/i386-softmmu/qemu-system-i386
  arg_xqm="-xtm select,tblink,lbt,verbose"
fi
bios=${BASE_DIR}/seabios/out/bios.bin

network="-netdev user,id=n1,ipv6=off -device e1000e,netdev=n1"

# assign stdio to debugcon and serial, then qemu complaints like this:
# > qemu-system-i386: -serial stdio: cannot use stdio by multiple character devices
# > qemu-system-i386: -serial stdio: could not connect serial device to character backend 'stdio'
if [[ $show_seabios_log = true ]]; then
  arg_bios="-chardev stdio,id=seabios -device isa-debugcon,iobase=0x402,chardev=seabios -bios ${bios}"
  arg_serial=""
else
  arg_bios="-chardev file,path=/tmp/seabios.log,id=seabios -device isa-debugcon,iobase=0x402,chardev=seabios -bios ${bios}"
  arg_serial="-nographic"
fi

arg_img="-hda ~/xqm_images/ubuntu10s.test.img.full"
arg_kernel="-kernel ${kernel}"
arg_kernel_cmdline="-append \"console=ttyS0 \""
arg_initrd=""

if [[ $run_tiny_kernel == true ]]; then
  arg_kernel_cmdline="-append \"console=ttyS0 earlyprintk=serial root=/dev/ram hpet=disable rdinit=/hello.out\""
  arg_initrd="-initrd ${initrd}"
  arg_img=""
fi

cmd="${debug_qemu} ${qemu} ${arg_img} ${arg_kernel} ${arg_xqm} ${arg_bios} ${arg_kernel_cmdline} ${arg_initrd} ${network} ${arg_serial}"
echo "${cmd}"
eval "${cmd}"
