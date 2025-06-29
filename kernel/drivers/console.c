#include "console.h"

// QEMU RISC-V UART base address
#define UART_BASE 0x10000000

// UART register offsets (16550 UART)
#define UART_THR 0x00  // Transmit Holding Register (write-only)
#define UART_RHR 0x00  // Receive Holding Register (read-only, same address as THR)
#define UART_LSR 0x05  // Line Status Register (read-only)

// Line Status Register bits
#define LSR_THRE 0x20  // Transmit Holding Register Empty
#define LSR_DR 0x01    // Data Ready (received data available)

// Memory-mapped I/O helpers
static inline void mmio_write8(unsigned long addr, unsigned char value) {
    *(volatile unsigned char*)addr = value;
}

static inline unsigned char mmio_read8(unsigned long addr) {
    return *(volatile unsigned char*)addr;
}

void console_init(void) {
    // For QEMU, the UART is already initialized
    // In a real system, you'd set baud rate, configure pins, etc.
    // Let's just send a test to verify it works
    console_puts("UART initialized - input/output ready\n");
}

void console_putchar(char c) {
    // Wait until transmit buffer is empty
    while (!(mmio_read8(UART_BASE + UART_LSR) & LSR_THRE)) {
        // Busy wait - in a real OS you'd yield to other tasks here
    }
    // Send the character
    mmio_write8(UART_BASE + UART_THR, c);
}

// Alias for console_putchar to match fs.c usage
void console_putc(char c) {
    console_putchar(c);
}

void console_puts(const char* str) {
    while (*str) {
        console_putchar(*str);
        str++;
    }
}

void console_println(const char* str) {
    console_puts(str);
    console_putchar('\n');
}

void console_put_hex(unsigned int value) {
    console_puts("0x");
    // Print each hex digit, starting from most significant
    for (int i = 28; i >= 0; i -= 4) {
        unsigned int digit = (value >> i) & 0xF;
        if (digit < 10) {
            console_putchar('0' + digit);
        } else {
            console_putchar('A' + digit - 10);
        }
    }
}

// INPUT FUNCTIONS
char console_getchar(void) {
    // Wait until data is available - pure polling, no WFI
    while (!(mmio_read8(UART_BASE + UART_LSR) & LSR_DR)) {
        // Busy wait - removed WFI as it might not work properly in QEMU
    }
    // Read the character
    return mmio_read8(UART_BASE + UART_RHR);
}

int console_getchar_nonblocking(void) {
    // Check if data is available without waiting
    if (mmio_read8(UART_BASE + UART_LSR) & LSR_DR) {
        return mmio_read8(UART_BASE + UART_RHR);
    }
    return -1;  // No data available
}

void console_gets(char* buffer, int max_length) {
    int pos = 0;
    char c;
    
    while (pos < max_length - 1) {
        c = console_getchar();
        
        // Handle special characters
        if (c == '\r' || c == '\n') {
            // Enter key - end input
            console_putchar('\n');
            break;
        } else if (c == '\b' || c == 127) {
            // Backspace or DEL
            if (pos > 0) {
                pos--;
                console_puts("\b \b");  // Move back, space, move back
            }
        } else if (c >= 32 && c <= 126) {
            // Printable character
            buffer[pos] = c;
            console_putchar(c);  // Echo the character
            pos++;
        }
        // Ignore other control characters
    }
    
    buffer[pos] = '\0';  // Null terminate
}

void console_prompt(const char* prompt_text) {
    console_puts(prompt_text);
}

void console_clear_line(void) {
    console_puts("\r\033[K");  // ANSI: carriage return + clear to end of line
}