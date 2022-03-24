all: hello
	echo hello.out | cpio -o -H newc | gzip > ../image/initrd.bin

SRC=scanf

ifeq ($(shell uname -m), x86_64)
asm-hello:
	gcc  -I /usr/include/x86_64-linux-gnu -nostdlib -m32 -static asm-hello.c -o hello.out

hello:
	gcc -m32 -static $(SRC).c -o hello.out
else
# use this to test loongarch kvm and qemu
hello:
	gcc -static $(SRC).c -o hello.out
endif

clean:
	rm -f hello.out
