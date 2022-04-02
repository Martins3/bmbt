ENTRY(start_entry)

SECTIONS {
    . = 0x9000000008000000;

    __text_start = .;
    .text :
    {
        *(.text)
    }

    . = ALIGN(8);
    __bss_start = .;
    .bss :
    {
        *(.bss)
    }
    __bss_stop = .;
}
