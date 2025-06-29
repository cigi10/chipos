.section .text.boot
.global _start

_start:
    la sp, stack_top
    
    la t0, bss_start
    la t1, bss_end
    
clear_bss:
    beq t0, t1, bss_done
    sw zero, 0(t0)
    addi t0, t0, 4
    j clear_bss
    
bss_done:
    call kernel_main
    
halt:
    j halt
