ENTRY(start_entry)

SECTIONS {
    . = 0x9000000000200000;

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
