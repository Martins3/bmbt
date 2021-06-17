#ifndef ELF_H_0RO4QZ1K
#define ELF_H_0RO4QZ1K
#include "../types.h"

/* LoongArch relocations */
#define R_LARCH_NONE                           0
#define R_LARCH_32                             1
#define R_LARCH_64                             2
#define R_LARCH_MARK_LA                        20
#define R_LARCH_MARK_PCREL                     21
#define R_LARCH_SOP_PUSH_PCREL                 22
#define R_LARCH_SOP_PUSH_ABSOLUTE              23
#define R_LARCH_SOP_PUSH_PLT_PCREL             29
#define R_LARCH_SOP_SUB                        32
#define R_LARCH_SOP_SL                         33
#define R_LARCH_SOP_SR                         34
#define R_LARCH_SOP_ADD                        35
#define R_LARCH_SOP_AND                        36
#define R_LARCH_SOP_IF_ELSE                    37
#define R_LARCH_SOP_POP_32_S_10_5              38
#define R_LARCH_SOP_POP_32_U_10_12             39
#define R_LARCH_SOP_POP_32_S_10_12             40
#define R_LARCH_SOP_POP_32_S_10_16             41
#define R_LARCH_SOP_POP_32_S_10_16_S2          42
#define R_LARCH_SOP_POP_32_S_5_20              43
#define R_LARCH_SOP_POP_32_S_0_5_10_16_S2      44
#define R_LARCH_SOP_POP_32_S_0_10_10_16_S2     45
#define R_LARCH_SOP_POP_32_U                   46
#define R_LARCH_ADD64                          51
#define R_LARCH_SUB64                          56

#define	ELFCLASSNONE	0		/* EI_CLASS */
#define	ELFCLASS32	1
#define	ELFCLASS64	2
#define	ELFCLASSNUM	3

/* 64-bit ELF base types. */
typedef uint64_t Elf64_Addr;
typedef uint16_t Elf64_Half;
typedef int16_t	 Elf64_SHalf;
typedef uint64_t Elf64_Off;
typedef int32_t	 Elf64_Sword;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;

typedef struct elf64_hdr {
  unsigned char	e_ident[16];		/* ELF "magic number" */
  Elf64_Half e_type;
  Elf64_Half e_machine;
  Elf64_Word e_version;
  Elf64_Addr e_entry;		/* Entry point virtual address */
  Elf64_Off e_phoff;		/* Program header table file offset */
  Elf64_Off e_shoff;		/* Section header table file offset */
  Elf64_Word e_flags;
  Elf64_Half e_ehsize;
  Elf64_Half e_phentsize;
  Elf64_Half e_phnum;
  Elf64_Half e_shentsize;
  Elf64_Half e_shnum;
  Elf64_Half e_shstrndx;
} Elf64_Ehdr;

typedef struct elf64_phdr {
  Elf64_Word p_type;
  Elf64_Word p_flags;
  Elf64_Off p_offset;		/* Segment file offset */
  Elf64_Addr p_vaddr;		/* Segment virtual address */
  Elf64_Addr p_paddr;		/* Segment physical address */
  Elf64_Xword p_filesz;		/* Segment size in file */
  Elf64_Xword p_memsz;		/* Segment size in memory */
  Elf64_Xword p_align;		/* Segment alignment, file & memory */
} Elf64_Phdr;

typedef struct elf64_sym {
  Elf64_Word st_name;		/* Symbol name, index in string tbl */
  unsigned char	st_info;	/* Type and binding attributes */
  unsigned char	st_other;	/* No defined meaning, 0 */
  Elf64_Half st_shndx;		/* Associated section index */
  Elf64_Addr st_value;		/* Value of the symbol */
  Elf64_Xword st_size;		/* Associated symbol size */
} Elf64_Sym;

typedef struct elf64_shdr {
  Elf64_Word sh_name;		/* Section name, index in string tbl */
  Elf64_Word sh_type;		/* Type of section */
  Elf64_Xword sh_flags;		/* Miscellaneous section attributes */
  Elf64_Addr sh_addr;		/* Section virtual addr at execution */
  Elf64_Off sh_offset;		/* Section file offset */
  Elf64_Xword sh_size;		/* Size of section in bytes */
  Elf64_Word sh_link;		/* Index of another section */
  Elf64_Word sh_info;		/* Additional section information */
  Elf64_Xword sh_addralign;	/* Section alignment */
  Elf64_Xword sh_entsize;	/* Entry size if section holds table */
} Elf64_Shdr;

#define	EI_MAG0		0		/* e_ident[] indexes */
#define	EI_MAG1		1
#define	EI_MAG2		2
#define	EI_MAG3		3
#define	EI_CLASS	4
#define	EI_DATA		5
#define	EI_VERSION	6
#define	EI_OSABI	7
#define	EI_PAD		8

#define	ELFMAG0		0x7f		/* EI_MAG */
#define	ELFMAG1		'E'
#define	ELFMAG2		'L'
#define	ELFMAG3		'F'
#define	ELFMAG		"\177ELF"
#define	SELFMAG		4

#define	ELFCLASSNONE	0		/* EI_CLASS */
#define	ELFCLASS32	1
#define	ELFCLASS64	2
#define	ELFCLASSNUM	3

#define ELFDATANONE	0		/* e_ident[EI_DATA] */
#define ELFDATA2LSB	1
#define ELFDATA2MSB	2

#define EV_NONE		0		/* e_version, EI_VERSION */
#define EV_CURRENT	1
#define EV_NUM		2

#ifndef ElfW
# if ELF_CLASS == ELFCLASS32
#  define ElfW(x)  Elf32_ ## x
#  define ELFW(x)  ELF32_ ## x
# else
#  define ElfW(x)  Elf64_ ## x
#  define ELFW(x)  ELF64_ ## x
# endif
#endif

#endif /* end of include guard: ELF_H_0RO4QZ1K */
