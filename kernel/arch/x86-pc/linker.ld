ENTRY (multiboot_entry)

SECTIONS
{
    /* Einherjar is loaded at 0x00100000.
	A multi-boot compatible kernel cannot be loaded before the 1MB mark,
	it must be loaded at or above 1MB. */
    . = 0x00100000;

    .multiboot :
    {
        /* The multiboot header must come within the first 8K of
	our kernel  */
	*(.multiboot);

	/* 32 bit aligned as required by the multiboot specs */
	LONG(0);
    }
    . = ALIGN(0x1000);
    /** Start of the kernel */
    __kernel_start = .;

    .text ALIGN (0x1000) :
    {
        *(.text)
    }

    .rodata ALIGN (0x1000) :
    {
        *(.rodata*)
    }

    .data ALIGN (0x1000) :
    {
        *(.data)
    }

    .bss :
    {
        sbss = .;
        *(COMMON)
        *(.bss)
        ebss = .;
    }

    /* End of the kernel */
    __kernel_end = .;
}
