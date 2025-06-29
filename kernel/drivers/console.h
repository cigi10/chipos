#ifndef CONSOLE_H
#define CONSOLE_H

// Initialize the console (UART)
void console_init(void);

// OUTPUT FUNCTIONS
// Output a single character
void console_putchar(char c);

// Output a null-terminated string
void console_puts(const char* str);

// Output string with newline
void console_println(const char* str);

// Output a hexadecimal number (for debugging)
void console_put_hex(unsigned int value);

// INPUT FUNCTIONS
// Read a single character (blocking)
char console_getchar(void);

// Read a character without blocking (-1 if none available)
int console_getchar_nonblocking(void);

// Read a line of input with basic editing (backspace support)
void console_gets(char* buffer, int max_length);

// Display a prompt
void console_prompt(const char* prompt_text);

// Clear current input line
void console_clear_line(void);

void console_putc(char c);

void console_putc(char c);

#endif