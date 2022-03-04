all: hello
	echo hello.out | cpio -o -H newc | gzip > ../image/initrd.bin

ifeq ($(shell uname -m), x86_64)
asm-hello:
	gcc  -I /usr/include/x86_64-linux-gnu -nostdlib -m32 -static asm-hello.c -o hello.out

hello:
	gcc -m32 -static scanf.c -o hello.out
else
hello:
	gcc -static scanf.c -o hello.out
endif

clean:
	rm -f hello.out
