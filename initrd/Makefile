# https://www.itread01.com/content/1550477197.html
all:
	gcc  -I /usr/include/x86_64-linux-gnu -nostdlib -m32 -static hello.c -o hello.out
	echo hello.out | cpio -o -H newc | gzip > ../image/initrd.bin
la:
	gcc -static la.c -o hello.out
	echo hello.out | cpio -o -H newc | gzip > ../initrd.bin
