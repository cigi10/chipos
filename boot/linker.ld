ENTRY(_start)

MEMORY
{
    RAM : ORIGIN = 0x80000000, LENGTH = 128M
}

PHDRS
{
    text PT_LOAD FLAGS(5);    /* R + X (read + execute) */
    rodata PT_LOAD FLAGS(4);  /* R (read only) */
    data PT_LOAD FLAGS(6);    /* R + W (read + write) */
}

SECTIONS
{
    . = 0x80000000;
    
    /* code section - read and execute only */
    .text : {
        *(.text.boot)
        *(.text)
        *(.text.*)
    } > RAM :text
    
    /* page align between different permission segments */
    . = ALIGN(4096);
    
    /* read-only data */
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    } > RAM :rodata
    
    /* page align before writable data */
    . = ALIGN(4096);
    
    /* writable data */
    .data : {
        *(.data)
        *(.data.*)
    } > RAM :data
    
    /* BSS (also writable) */
    .bss : {
        bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        bss_end = .;
    } > RAM :data
    
    /* Stack */
    . = ALIGN(4096);
    . += 4096;
    stack_top = .;
    kernel_end = .;
    
    /* discard unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.eh_frame*)
        *(.debug*)
    }
}
