#ifndef SHELL_H
#define SHELL_H

// Initialize the shell system
void shell_init(void);

// Main shell loop - runs interactive commands
void shell_run(void);

void console_gets_with_history(char* buffer, int max_len);

#endif