#ifndef CONSOLE_H
#define CONSOLE_H

// initialize the console (UART)
void console_init(void);

// OUTPUT FUNCTIONS
// output a single character
void console_putchar(char c);

// output a null-terminated string
void console_puts(const char* str);

// output string with newline
void console_println(const char* str);

// output a hexadecimal number (for debugging)
void console_put_hex(unsigned int value);

// INPUT FUNCTIONS
// read a single character (blocking)
char console_getchar(void);

// read a character without blocking (-1 if none available)
int console_getchar_nonblocking(void);

// read a line of input with basic editing (backspace support)
void console_gets(char* buffer, int max_length);

// display a prompt
void console_prompt(const char* prompt_text);

// clear current input line
void console_clear_line(void);

void console_putc(char c);

void console_putc(char c);

#endif
