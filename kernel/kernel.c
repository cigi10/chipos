#include "include/kernel.h"
#include "drivers/console.h"
#include "memory/memory.h"
#include "shell/shell.h"
#include "fs/fs.h"            
#include "include/types.h"
#include "../lib/string.h"
#include "../editor/editor.h"

system_info_t g_system_info = {0};

void kernel_panic(const char* message) {
    console_puts("\n*** KERNEL PANIC ***\n");
    console_puts("Error: ");
    console_puts(message);
    console_puts("\nSystem halted.\n");

    CSR_CLEAR(mstatus, MSTATUS_MIE);
    while(1) {
        asm volatile ("wfi"); 
    }
}

void kernel_print_banner(void) {
    console_println("=================================");
    console_println("        ChipOS       ");
    console_println("=================================");
}

void test_memory_allocator(void) {
    console_println("\n--- Testing Memory Allocator ---");

    void* ptr1 = kmalloc(64);
    void* ptr2 = kmalloc(128);
    void* ptr3 = kmalloc(256);

    console_puts("allocated ptr1: ");
    console_put_hex((uintptr_t)ptr1);
    console_puts("\n");

    console_puts("allocated ptr2: ");
    console_put_hex((uintptr_t)ptr2);
    console_puts("\n");

    console_puts("allocated ptr3: ");
    console_put_hex((uintptr_t)ptr3);
    console_puts("\n");

    if (ptr1) {
        memset(ptr1, 0xAA, 64);
        console_println("Wrote test pattern to ptr1");
    }

    kfree(ptr2);
    console_println("Freed ptr2");

    kfree(ptr1);
    kfree(ptr3);
    console_println("Memory allocator test completed");
}

void test_input_output(void) {
    console_println("\n--- Testing Input/Output ---");
    console_println("Testing keyboard input...");

    char test_buffer[64];
    console_prompt("Type something and press Enter: ");
    console_gets(test_buffer, sizeof(test_buffer));

    console_puts("You typed: '");
    console_puts(test_buffer);
    console_println("'");
    console_println("Input/Output test completed!");
}

void kernel_main() {
    console_init();
    memory_init();

    fs_init();
    console_puts("[DEBUG] fs_init() called\n");

    char buf[16];
    console_puts("[DEBUG] Filesystem state before shell: file_count=");
    itoa(fs.file_count, buf, 10);
    console_puts(buf);
    console_puts(", current_dir=");
    itoa(fs.current_dir, buf, 10);
    console_puts(buf);
    console_puts(", next_file_id=");
    itoa(fs.next_file_id, buf, 10);
    console_puts(buf);
    console_puts("\n");

    kernel_print_banner();
    console_println("RISC-V kernel loaded successfully");

    console_println("\n--- System Information ---");
    g_system_info.hart_id = CSR_READ(mhartid);
    g_system_info.misa = CSR_READ(misa);

    console_puts("Hart ID: ");
    console_put_hex(g_system_info.hart_id);
    console_puts("\n");

    console_puts("MISA (ISA info): ");
    console_put_hex(g_system_info.misa);
    console_puts("\n");

    console_puts("Supported extensions: ");
    if (g_system_info.misa & (1 << 0)) console_puts("A ");
    if (g_system_info.misa & (1 << 2)) console_puts("C ");
    if (g_system_info.misa & (1 << 3)) console_puts("D ");
    if (g_system_info.misa & (1 << 5)) console_puts("F ");
    if (g_system_info.misa & (1 << 8)) console_puts("I ");
    if (g_system_info.misa & (1 << 12)) console_puts("M ");
    if (g_system_info.misa & (1 << 18)) console_puts("S ");
    if (g_system_info.misa & (1 << 20)) console_puts("U ");
    console_puts("\n");

    console_println("\n--- Memory Layout ---");
    extern char bss_start[], bss_end[], kernel_end[], stack_top[];

    console_puts("BSS section: ");
    console_put_hex((uintptr_t)bss_start);
    console_puts(" - ");
    console_put_hex((uintptr_t)bss_end);
    console_puts("\n");

    console_puts("Kernel end: ");
    console_put_hex((uintptr_t)kernel_end);
    console_puts("\n");

    console_puts("Stack top: ");
    console_put_hex((uintptr_t)stack_top);
    console_puts("\n");

    test_memory_allocator();
    test_input_output();

    console_println("\n--- Starting Interactive Shell ---");
    shell_init();
    shell_run();
}
