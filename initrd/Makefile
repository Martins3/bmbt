# https://www.itread01.com/content/1550477197.html
all:
	gcc -m32 -static hello.c -o hello.out
	echo hello.out | cpio -o -H newc | gzip > ../image/test.cpio.gz
