#ifndef EDITOR_H
#define EDITOR_H

#include "../include/types.h"

// Language types for syntax highlighting
typedef enum {
    LANG_PLAIN,
    LANG_C,
    LANG_VERILOG,
    LANG_SYSTEMVERILOG,
    LANG_RISCV_ASM,
    LANG_UNKNOWN
} language_t;

// Token types for syntax highlighting
typedef enum {
    TOKEN_NORMAL,
    TOKEN_KEYWORD,
    TOKEN_COMMENT,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_PREPROCESSOR,
    TOKEN_SIGNAL,        // Verilog signals
    TOKEN_MODULE,        // Verilog modules
    TOKEN_REGISTER,      // RISC-V registers
    TOKEN_INSTRUCTION    // RISC-V instructions
} token_type_t;

// Editor configuration
#define MAX_LINES 1000
#define MAX_LINE_LENGTH 256
#define TAB_SIZE 4

// Editor state structure
typedef struct {
    char filename[256];
    char lines[MAX_LINES][MAX_LINE_LENGTH];
    int line_count;
    int cursor_line;
    int cursor_col;
    int view_start_line;
    int view_height;
    bool modified;
    bool insert_mode;
    language_t language;
} editor_state_t;

// Function prototypes
language_t detect_language(const char* filename);
bool is_c_keyword(const char* word);
bool is_verilog_keyword(const char* word);
bool is_riscv_instruction(const char* word);
bool is_riscv_register(const char* word);
bool is_alpha(char c);
bool is_digit(char c);
bool is_alnum(char c);
char* extract_word(const char* line, int start, int* end);
void editor_print_colored(const char* text, token_type_t type);
void editor_highlight_c(const char* line, int line_num);
void editor_highlight_verilog(const char* line, int line_num);
void editor_highlight_riscv(const char* line, int line_num);
void editor_display_line(int line_num, const char* line);
void editor_show_status(void);
int editor_load_file(const char* filename);
int editor_save_file(void);
void editor_show_help(void);
void editor_insert_char(char c);
void editor_delete_char(void);
void editor_new_line(void);
int editor_start(const char* filename);
void console_put_dec(unsigned int value);

#endif // EDITOR_H