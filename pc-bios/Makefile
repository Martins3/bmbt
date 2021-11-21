CFLAGS+=-Werror
CFLAGS+=-Wstrict-prototypes
CFLAGS+=-Wredundant-decls
CFLAGS+=-Wall
CFLAGS+=-Wundef
CFLAGS+=-Wwrite-strings
CFLAGS+=-Wmissing-prototypes
CFLAGS+=-Wexpansion-to-defined
CFLAGS+=-Wendif-labels
CFLAGS+=-Wno-shift-negative-value
CFLAGS+=-Wno-missing-include-dirs
CFLAGS+=-Wempty-body
CFLAGS+=-Wnested-externs
CFLAGS+=-Wformat-security
CFLAGS+=-Wformat-y2k
CFLAGS+=-Winit-self
CFLAGS+=-Wignored-qualifiers
CFLAGS+=-Wold-style-declaration
CFLAGS+=-Wold-style-definition
CFLAGS+=-Wtype-limits
CFLAGS+=-ffreestanding
CFLAGS+=-fno-stack-protector
CFLAGS+=-m16
CFLAGS+=-Wa,-32
CFLAGS+=-O2
CFLAGS+=-g
CFLAGS+=-march=i486

all:
	cc $(CFLAGS) -c -o linuxboot_dma.o linuxboot_dma.c
	ld -nopie -m elf_i386 -T flat.lds -s -o linuxboot_dma.img linuxboot_dma.o
	objcopy -O binary -j .text linuxboot_dma.img linuxboot_dma.raw
	python3 -B signrom.py linuxboot_dma.raw ../image/linuxboot_dma.bin
