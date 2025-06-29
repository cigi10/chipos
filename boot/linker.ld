/* Linker script for RISC-V 64-bit kernel */
ENTRY(_start)

MEMORY
{
    RAM : ORIGIN = 0x80000000, LENGTH = 128M
}

/* Define program headers with proper permissions */
PHDRS
{
    text PT_LOAD FLAGS(5);    /* R + X (read + execute) */
    rodata PT_LOAD FLAGS(4);  /* R (read only) */
    data PT_LOAD FLAGS(6);    /* R + W (read + write) */
}

SECTIONS
{
    . = 0x80000000;
    
    /* Code section - read and execute only */
    .text : {
        *(.text.boot)
        *(.text)
        *(.text.*)
    } > RAM :text
    
    /* Page align between different permission segments */
    . = ALIGN(4096);
    
    /* Read-only data */
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    } > RAM :rodata
    
    /* Page align before writable data */
    . = ALIGN(4096);
    
    /* Writable data */
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
    
    /* Discard unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.eh_frame*)
        *(.debug*)
    }
}