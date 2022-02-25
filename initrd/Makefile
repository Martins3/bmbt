ifneq ($(shell uname -m), x86_64)
$(error mismatch architecture)
endif

all: hello
	echo hello.out | cpio -o -H newc | gzip > ../image/initrd.bin

asm-hello:
	gcc  -I /usr/include/x86_64-linux-gnu -nostdlib -m32 -static asm-hello.c -o hello.out

hello:
	gcc -m32 -static hello.c -o hello.out

clean:
	rm -f hello.out
