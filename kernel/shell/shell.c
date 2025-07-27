#include "shell.h"
#include "../drivers/console.h"
#include "../memory/memory.h"
#include "../include/kernel.h"
#include "../../lib/string.h"
#include "../fs/fs.h"   
#include <stdbool.h>
#include "../editor/editor.h"

#define MAX_COMMAND_LENGTH 256
#define MAX_ARGS 10
#define MAX_FILE_SIZE 4096

#define HISTORY_SIZE 20
static char command_history[HISTORY_SIZE][MAX_COMMAND_LENGTH];
static int history_count = 0;


// ANSI color codes for syntax highlighting
#define ANSI_COLOR_RED     "\033[31m"
#define ANSI_COLOR_GREEN   "\033[32m"
#define ANSI_COLOR_YELLOW  "\033[33m"
#define ANSI_COLOR_BLUE    "\033[34m"
#define ANSI_COLOR_MAGENTA "\033[35m"
#define ANSI_COLOR_CYAN    "\033[36m"
#define ANSI_COLOR_WHITE   "\033[37m"
#define ANSI_COLOR_BRIGHT_GREEN "\033[92m"
#define ANSI_COLOR_BRIGHT_BLUE  "\033[94m"
#define ANSI_COLOR_BRIGHT_YELLOW "\033[93m"
#define ANSI_COLOR_BRIGHT_RED   "\033[91m"
#define ANSI_COLOR_BRIGHT_MAGENTA "\033[95m"
#define ANSI_COLOR_BRIGHT_CYAN  "\033[96m"
#define ANSI_COLOR_RESET   "\033[0m"

static const char* valid_commands[] = {
    "help", "ls", "cd", "pwd", "mkdir", "rmdir", "rm", "touch", "cat",
    "about", "mem", "calc", "clear", "echo", "colortest", "panic", 
    "edit", "code", "compile", "run", "syntax", "cp", "mv", "find", 
    "grep", "exit", "quit", NULL
};

// declarations for helper functions
static void display_line_with_highlighting(const char* line, const char* ext);
static void highlight_word(const char* word, const char* ext);
static const char* get_file_extension(const char* filename);
static char* simple_strtok(char* str, const char* delim) __attribute__((unused));

typedef struct {
    const char* name;
    const char* description;
    void (*handler)(int argc, char* argv[]);
} command_t;

// declarations for commands
static void cmd_help(int argc, char* argv[]);
static void cmd_about(int argc, char* argv[]);
static void cmd_mem(int argc, char* argv[]);
static void cmd_calc(int argc, char* argv[]);
static void cmd_clear(int argc, char* argv[]);
static void cmd_echo(int argc, char* argv[]);
static void cmd_panic(int argc, char* argv[]);
static void cmd_ls(int argc, char* argv[]);
static void cmd_cd(int argc, char* argv[]);
static void cmd_pwd(int argc, char* argv[]);
static void cmd_mkdir(int argc, char* argv[]);
static void cmd_rmdir(int argc, char* argv[]);
static void cmd_rm(int argc, char* argv[]);
static void cmd_touch(int argc, char* argv[]);
static void cmd_cat(int argc, char* argv[]);
static void cmd_cp(int argc, char* argv[]);
static void cmd_mv(int argc, char* argv[]);
static void cmd_find(int argc, char* argv[]);
static void cmd_grep(int argc, char* argv[]);
static void cmd_edit(int argc, char* argv[]);
static void cmd_code(int argc, char* argv[]);
static void cmd_colortest(int argc, char* argv[]);
static void cmd_compile(int argc, char* argv[]);
static void cmd_run(int argc, char* argv[]);
static void cmd_syntax(int argc, char* argv[]);
static void cmd_exit(int argc, char* argv[]);
static void cmd_quit(int argc, char* argv[]);

static int parse_command(char* input, char* args[], int max_args);
static void execute_command(int argc, char* argv[]);
static int simple_atoi(const char* str);

static const char* verilog_keywords[] = {
    "module", "endmodule", "input", "output", "inout", "wire", "reg", "parameter",
    "always", "initial", "begin", "end", "if", "else", "case", "endcase",
    "default", "for", "while", "assign", "posedge", "negedge", "or", "and",
    "not", "nand", "nor", "xor", "xnor", "integer", "real", "time", "genvar",
    "generate", "endgenerate", "function", "endfunction", "task", "endtask",
    "logic", "bit", "byte", "shortint", "int", "longint", "typedef", "struct",
    "union", "enum", "interface", "modport", "clocking", "property", "sequence",
    "always_ff", "always_comb", "always_latch", "unique", "priority",
    NULL
};

static const char* c_keywords[] = {
    "int", "char", "float", "double", "void", "long", "short", "unsigned",
    "signed", "const", "static", "volatile", "extern", "auto", "register",
    "struct", "union", "enum", "typedef", "sizeof", "if", "else", "for",
    "while", "do", "switch", "case", "default", "break", "continue", "return",
    "goto", "inline", "restrict", "bool", "_Bool", "true", "false",
    NULL
};

static const char* asm_keywords[] = {
    "add", "sub", "mul", "div", "and", "or", "xor", "sll", "srl", "sra",
    "beq", "bne", "blt", "bge", "bltu", "bgeu", "jal", "jalr", "lui", "auipc",
    "lb", "lh", "lw", "lbu", "lhu", "sb", "sh", "sw", "addi", "slti", "sltiu",
    "xori", "ori", "andi", "slli", "srli", "srai", "fence", "ecall", "ebreak",
    ".text", ".data", ".bss", ".section", ".global", ".word", ".byte", ".ascii",
    ".string", ".align", "nop", "mv", "li", "la", "ret", "j", "jr",
    "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
    "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19",
    "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29",
    "x30", "x31", "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "fp", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
    "t3", "t4", "t5", "t6",
    NULL
};

// command table  
static command_t commands[] = {
    {"help", "Show available commands", cmd_help},
    {"about", "Show system information", cmd_about},
    {"mem", "Show memory usage", cmd_mem},
    {"calc", "Simple calculator (calc 2 + 3)", cmd_calc},
    {"clear", "Clear the screen", cmd_clear},
    {"echo", "Echo text back", cmd_echo},
    {"panic", "Trigger kernel panic (for testing)", cmd_panic},
    {"ls", "List directory contents", cmd_ls},
    {"cd", "Change directory", cmd_cd},
    {"pwd", "Print working directory", cmd_pwd},
    {"mkdir", "Create directory", cmd_mkdir},
    {"rmdir", "Remove directory", cmd_rmdir},
    {"rm", "Remove file", cmd_rm},
    {"touch", "Create empty file", cmd_touch},
    {"cat", "Display file contents", cmd_cat},
    {"colortest", "Test ANSI color support", cmd_colortest},
    {"cp", "Copy file", cmd_cp},
    {"mv", "Move/rename file", cmd_mv},
    {"find", "Find files", cmd_find},
    {"grep", "Search in files", cmd_grep},
    {"edit", "Edit file", cmd_edit},
    {"code", "Code editor", cmd_code},
    {"compile", "Compile code", cmd_compile},
    {"run", "Run program", cmd_run},
    {"syntax", "Check syntax", cmd_syntax},
    {"exit", "Exit shell", cmd_exit},
    {"quit", "Quit shell", cmd_quit},
    {NULL, NULL, NULL} // Sentinel
};

static bool is_valid_command(const char* cmd) {
    if (!cmd || strlen(cmd) == 0) {
        return false;
    }
    
    for (int i = 0; valid_commands[i] != NULL; i++) {
        if (strcmp(cmd, valid_commands[i]) == 0) {
            return true;
        }
    }
    return false;
}

static char* simple_strtok(char* str, const char* delim) {
    static char* last_pos = NULL;
    char* start;
    
    if (str != NULL) {
        last_pos = str;
    } else if (last_pos == NULL) {
        return NULL;
    }

    while (*last_pos && strchr(delim, *last_pos)) {
        last_pos++;
    }
    
    if (*last_pos == '\0') {
        last_pos = NULL;
        return NULL;
    }
    
    start = last_pos;

    while (*last_pos && !strchr(delim, *last_pos)) {
        last_pos++;
    }
    
    if (*last_pos) {
        *last_pos = '\0';
        last_pos++;
    } else {
        last_pos = NULL;
    }
    
    return start;
}

void shell_init(void) {
    console_println("\n=== Welcome to ChipOS Shell ===");
    console_println("Multi-Language Development Environment");
    console_println("Supports: C, Verilog/SystemVerilog, RISC-V Assembly");
    console_println("Type 'help' for available commands");
}

void shell_run(void) {
    char input_buffer[MAX_COMMAND_LENGTH];
    char* args[MAX_ARGS];
    int argc;
    
    while (1) {
        char current_path[MAX_PATH_LENGTH];
        if (fs_get_current_path(current_path, sizeof(current_path)) == FS_SUCCESS) {
            char prompt[MAX_PATH_LENGTH + 32]; 
            char dir_id_hex[16];
            itoa(fs_get_current_dir_id(), dir_id_hex, 16);
            strcpy(prompt, "chip:");
            strcat(prompt, current_path);
            strcat(prompt, " [0x");
            strcat(prompt, dir_id_hex);
            strcat(prompt, "]$ ");
            console_prompt(prompt);
        } else {
            console_prompt("chip> ");
        }
        
        console_gets_with_history(input_buffer, MAX_COMMAND_LENGTH);
        
        if (strlen(input_buffer) == 0) {
            continue;
        }
    
        argc = parse_command(input_buffer, args, MAX_ARGS);
        if (argc > 0) {
            execute_command(argc, args);
        }
    }
}

static int parse_command(char* input, char* args[], int max_args) {
    int argc = 0;
    char* token = input;
    
    // simple tokenization by spaces
    while (*token && argc < max_args) {
        // skip leading spaces
        while (*token == ' ' || *token == '\t') {
            token++;
        }
        
        if (*token == '\0') {
            break;
        }
        
        // mark start of argument
        args[argc] = token;
        argc++;
        
        // gind end of argument
        while (*token && *token != ' ' && *token != '\t') {
            token++;
        }
        
        // null-terminate if not at end of string
        if (*token) {
            *token = '\0';
            token++;
        }
    }
    
    return argc;
}

static int is_likely_accidental_input(const char* input) {
    // common C keywords that users might accidentally type
    const char* c_keywords[] = {
        "int", "char", "void", "if", "else", "for", "while", "do", "switch", 
        "case", "break", "continue", "return", "struct", "union", "enum",
        "typedef", "const", "static", "extern", "auto", "register", "volatile",
        "unsigned", "signed", "long", "short", "double", "float", NULL
    };
    
    // check if it's a single C keyword
    for (int i = 0; c_keywords[i] != NULL; i++) {
        if (strcmp(input, c_keywords[i]) == 0) {
            return 1;
        }
    }
    
    // check if it's obviously not a command (single letters, numbers, etc.)
    if (strlen(input) == 1) {
        return 1;  // single characters are probably accidents
    }
    
    return 0;  // treat as a real command attempt
}

static void execute_command(int argc, char* argv[]) {
    if (argc == 0) return;
    
    if (!is_valid_command(argv[0])) {
        // check if it's likely accidental input
        if (is_likely_accidental_input(argv[0])) {
            // silently ignore obvious non-commands
            return;
        }
        
        // for actual command attempts, show error
        console_puts("Unknown command: ");
        console_puts(argv[0]);
        console_puts("\nType 'help' for available commands.\n");
        return;
    }
    
    // execute the valid command
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            commands[i].handler(argc, argv);
            return;
        }
    }
}

static const char* get_file_extension(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot;
}

static int simple_atoi(const char* str) {
    int result = 0;
    int sign = 1;
    
    if (*str == '-') {
        sign = -1;
        str++;
    }
    
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result * sign;
}

static int is_asm_keyword(const char* word) {
    for (int i = 0; asm_keywords[i] != NULL; i++) {
        if (strcmp(word, asm_keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static void highlight_word(const char* word, const char* ext) {
    if (ext && (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0)) {
        if (is_c_keyword(word)) {
            console_puts(ANSI_COLOR_BRIGHT_BLUE);
            console_puts(word);
            console_puts(ANSI_COLOR_RESET);
        } else {
            console_puts(word);
        }
    } else if (ext && (strcmp(ext, ".v") == 0 || strcmp(ext, ".sv") == 0)) {
        if (is_verilog_keyword(word)) {
            console_puts(ANSI_COLOR_BRIGHT_MAGENTA);
            console_puts(word);
            console_puts(ANSI_COLOR_RESET);
        } else {
            console_puts(word);
        }
    } else if (ext && (strcmp(ext, ".s") == 0 || strcmp(ext, ".asm") == 0)) {
        if (is_asm_keyword(word)) {
            console_puts(ANSI_COLOR_BRIGHT_CYAN);
            console_puts(word);
            console_puts(ANSI_COLOR_RESET);
        } else {
            console_puts(word);
        }
    } else {
        console_puts(word);
    }
}

// syntax highlighting function for multiple languages
static void display_line_with_highlighting(const char* line, const char* ext) {
    console_puts("    "); // indentation
    
    if (!ext) {
        console_puts(line);
        console_puts("\n");
        return;
    }
    
    // C syntax highlighting
    if (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0) {
        if (strstr(line, "#include") || strstr(line, "#define") || strstr(line, "#ifndef") || 
            strstr(line, "#ifdef") || strstr(line, "#endif") || strstr(line, "#pragma")) {
            console_puts(ANSI_COLOR_CYAN);
            console_puts(line);
            console_puts(ANSI_COLOR_RESET);
        } else if (strstr(line, "//")) {
            char* comment_start = strstr(line, "//");
            int code_len = comment_start - line;
            for (int i = 0; i < code_len; i++) {
                console_putchar(line[i]);
            }
            console_puts(ANSI_COLOR_BLUE);
            console_puts(comment_start);
            console_puts(ANSI_COLOR_RESET);
        } else if (strstr(line, "/*") || strstr(line, "*/") || strstr(line, " * ")) {
            console_puts(ANSI_COLOR_BLUE);
            console_puts(line);
            console_puts(ANSI_COLOR_RESET);
        } else if (strstr(line, "\"") && !strstr(line, "//")) {
            console_puts(ANSI_COLOR_GREEN);
            console_puts(line);
            console_puts(ANSI_COLOR_RESET);
        } else {
            // Word-by-word highlighting for keywords
            char word[64];
            int word_pos = 0;
            const char* ptr = line;
            
            while (*ptr) {
                if ((*ptr >= 'a' && *ptr <= 'z') || (*ptr >= 'A' && *ptr <= 'Z') || 
                    (*ptr >= '0' && *ptr <= '9') || *ptr == '_') {
                    if (word_pos < 63) {
                        word[word_pos++] = *ptr;
                    }
                } else {
                    if (word_pos > 0) {
                        word[word_pos] = '\0';
                        highlight_word(word, ext);
                        word_pos = 0;
                    }
                    console_putchar(*ptr);
                }
                ptr++;
            }
            
            if (word_pos > 0) {
                word[word_pos] = '\0';
                highlight_word(word, ext);
            }
        }
    }
    // verilog syntax highlighting
    else if (strcmp(ext, ".v") == 0 || strcmp(ext, ".sv") == 0) {
        if (strstr(line, "//")) {
            char* comment_start = strstr(line, "//");
            int code_len = comment_start - line;
            for (int i = 0; i < code_len; i++) {
                console_putchar(line[i]);
            }
            console_puts(ANSI_COLOR_BLUE);
            console_puts(comment_start);
            console_puts(ANSI_COLOR_RESET);
        } else if (strstr(line, "/*") || strstr(line, "*/")) {
            console_puts(ANSI_COLOR_BLUE);
            console_puts(line);
            console_puts(ANSI_COLOR_RESET);
        } else {
            // Word-by-word highlighting for Verilog keywords
            char word[64];
            int word_pos = 0;
            const char* ptr = line;
            
            while (*ptr) {
                if ((*ptr >= 'a' && *ptr <= 'z') || (*ptr >= 'A' && *ptr <= 'Z') || 
                    (*ptr >= '0' && *ptr <= '9') || *ptr == '_') {
                    if (word_pos < 63) {
                        word[word_pos++] = *ptr;
                    }
                } else {
                    if (word_pos > 0) {
                        word[word_pos] = '\0';
                        highlight_word(word, ext);
                        word_pos = 0;
                    }
                    console_putchar(*ptr);
                }
                ptr++;
            }
            
            if (word_pos > 0) {
                word[word_pos] = '\0';
                highlight_word(word, ext);
            }
        }
    }
    // assembly syntax highlighting
    else if (strcmp(ext, ".s") == 0 || strcmp(ext, ".asm") == 0) {
        if (strstr(line, "#") || strstr(line, ";")) {
            char* comment_start = strstr(line, "#");
            if (!comment_start) comment_start = strstr(line, ";");
            int code_len = comment_start - line;
            for (int i = 0; i < code_len; i++) {
                console_putchar(line[i]);
            }
            console_puts(ANSI_COLOR_BLUE);
            console_puts(comment_start);
            console_puts(ANSI_COLOR_RESET);
        } else {
            // Word-by-word highlighting for Assembly keywords
            char word[64];
            int word_pos = 0;
            const char* ptr = line;
            
            while (*ptr) {
                if ((*ptr >= 'a' && *ptr <= 'z') || (*ptr >= 'A' && *ptr <= 'Z') || 
                    (*ptr >= '0' && *ptr <= '9') || *ptr == '_' || *ptr == '.') {
                    if (word_pos < 63) {
                        word[word_pos++] = *ptr;
                    }
                } else {
                    if (word_pos > 0) {
                        word[word_pos] = '\0';
                        highlight_word(word, ext);
                        word_pos = 0;
                    }
                    console_putchar(*ptr);
                }
                ptr++;
            }
            
            if (word_pos > 0) {
                word[word_pos] = '\0';
                highlight_word(word, ext);
            }
        }
    }
    else {
        console_puts(line);
    }
    
    console_puts("\n");
}

// command implementations

static void cmd_help(int argc __attribute__((unused)), char* argv[] __attribute__((unused))) {
    console_println("\nChipOS Multi-Language Development Shell");
    console_println("======================================");
    console_println("\nFile Operations:");
    console_println("  ls [dir]     - List directory contents");
    console_println("  cd <dir>     - Change directory");
    console_println("  pwd          - Print working directory");
    console_println("  mkdir <dir>  - Create directory");
    console_println("  rmdir <dir>  - Remove directory");
    console_println("  rm <file>    - Remove file");
    console_println("  touch <file> - Create empty file");
    console_println("  cat <file>   - Display file with syntax highlighting");
    console_println("  code <file>  - Open file in VIM-style advanced editor");
    
    console_println("\nSystem Commands:");
    console_println("  about        - Show system information");
    console_println("  mem          - Show memory usage");
    console_println("  calc <expr>  - Simple calculator");
    console_println("  clear        - Clear screen");
    console_println("  echo <text>  - Echo text");
    console_println("  colortest    - Test color support");
    console_println("  panic        - Trigger kernel panic (testing)");
    
    console_println("\nSupported Languages:");
    console_println("  .c, .h       - C/C++ with keyword highlighting");
    console_println("  .v, .sv      - Verilog/SystemVerilog");
    console_println("  .s, .asm     - RISC-V Assembly");
    console_println("");
}

static void cmd_about(int argc __attribute__((unused)), char* argv[] __attribute__((unused))) {
    console_println("\nChipOS");
    console_println("==================================================");
    console_println("");
    console_println("Features:");
    console_println("• Multi-language syntax highlighting (C, Verilog, Assembly)");
    console_println("• Advanced text editors with language-aware features");
    console_println("• Integrated development environment");
    console_println("• Hardware design workflow support");
    console_println("• RISC-V native execution environment");
    console_println("");
    console_println("Target: RISC-V architecture");
    console_println("Built for: Hardware prototyping and education");
    console_println("");
}

static void cmd_mem(int argc __attribute__((unused)), char* argv[] __attribute__((unused))) {
    memory_print_info();
}

static void cmd_calc(int argc, char* argv[]) {
    if (argc < 4) {
        console_println("Usage: calc <number> <operator> <number>");
        console_println("Example: calc 2 + 3");
        console_println("Operators: +, -, *, /");
        return;
    }
    
    int a = simple_atoi(argv[1]);
    int b = simple_atoi(argv[3]);
    char* op = argv[2];
    int result = 0;
    int valid = 1;
    
    if (strcmp(op, "+") == 0) {
        result = a + b;
    } else if (strcmp(op, "-") == 0) {
        result = a - b;
    } else if (strcmp(op, "*") == 0) {
        result = a * b;
    } else if (strcmp(op, "/") == 0) {
        if (b == 0) {
            console_println("Error: Division by zero");
            return;
        }
        result = a / b;
    } else {
        console_println("Error: Unknown operator");
        valid = 0;
    }
    
    if (valid) {
        console_puts("Result: ");
        console_put_hex(result);
        console_println("");
    }
}

static void cmd_clear(int argc __attribute__((unused)), char* argv[] __attribute__((unused))) {
    console_puts("\033[2J\033[H");
    console_println("ChipOS");
}

static void cmd_echo(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        console_puts(argv[i]);
        if (i < argc - 1) {
            console_puts(" ");
        }
    }
    console_println("");
}

static void cmd_panic(int argc __attribute__((unused)), char* argv[] __attribute__((unused))) {
    kernel_panic("User requested panic via shell command");
}

static void cmd_colortest(int argc __attribute__((unused)), char* argv[] __attribute__((unused))) {
    console_println("\nTesting Multi-Language Syntax Colors:");
    console_println("=====================================");
    console_puts(ANSI_COLOR_RED "Red (errors/warnings)" ANSI_COLOR_RESET "\n");
    console_puts(ANSI_COLOR_GREEN "Green (strings/literals)" ANSI_COLOR_RESET "\n");
    console_puts(ANSI_COLOR_YELLOW "Yellow (control structures)" ANSI_COLOR_RESET "\n");
    console_puts(ANSI_COLOR_BLUE "Blue (comments)" ANSI_COLOR_RESET "\n");
    console_puts(ANSI_COLOR_MAGENTA "Magenta (flow control)" ANSI_COLOR_RESET "\n");
    console_puts(ANSI_COLOR_CYAN "Cyan (preprocessor/directives)" ANSI_COLOR_RESET "\n");
    console_puts(ANSI_COLOR_BRIGHT_GREEN "Bright Green (C types)" ANSI_COLOR_RESET "\n");
    console_puts(ANSI_COLOR_BRIGHT_BLUE "Bright Blue (C keywords)" ANSI_COLOR_RESET "\n");
    console_puts(ANSI_COLOR_BRIGHT_YELLOW "Bright Yellow (numbers)" ANSI_COLOR_RESET "\n");
    console_puts(ANSI_COLOR_BRIGHT_RED "Bright Red (operators)" ANSI_COLOR_RESET "\n");
    console_puts(ANSI_COLOR_BRIGHT_MAGENTA "Bright Magenta (Verilog keywords)" ANSI_COLOR_RESET "\n");
    console_puts(ANSI_COLOR_BRIGHT_CYAN "Bright Cyan (Assembly keywords)" ANSI_COLOR_RESET "\n");
    console_println("\nSyntax Highlighting Examples:");
    console_println("C Code:");
    display_line_with_highlighting("int main() {", ".c");
    display_line_with_highlighting("    printf(\"Hello World\"); // Comment", ".c");
    display_line_with_highlighting("    return 0;", ".c");
    display_line_with_highlighting("}", ".c");
    
    console_println("\nVerilog Code:");
    display_line_with_highlighting("module test (", ".v");
    display_line_with_highlighting("    input clk, // Clock signal", ".v");
    display_line_with_highlighting("    output reg out", ".v");
    display_line_with_highlighting(");", ".v");
    display_line_with_highlighting("always @(posedge clk) begin", ".v");
    display_line_with_highlighting("    out <= ~out;", ".v");
    display_line_with_highlighting("end", ".v");
    display_line_with_highlighting("endmodule", ".v");
    
    console_println("\nRISC-V Assembly:");
    display_line_with_highlighting(".text", ".s");
    display_line_with_highlighting("main:", ".s");
    display_line_with_highlighting("    addi a0, zero, 10  # Load immediate", ".s");
    display_line_with_highlighting("    jal ra, func       # Jump and link", ".s");
    display_line_with_highlighting("    ret                # Return", ".s");
}

// file system command implementations

void cmd_ls(int argc, char** argv) {
    bool long_listing = false;
    const char* path = NULL;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            long_listing = true;
        } else {
            path = argv[i]; // assume anything else is a path
        }
    }

    int dir_id;
    if (path) {
        dir_id = fs_resolve_path(path);
        if (dir_id < 0) {
            console_puts("ls: cannot access '");
            console_puts(path);
            console_puts("'\n");
            return;
        }
    } else {
        dir_id = fs.current_dir;
    }

    fs_list_directory(dir_id, long_listing);
}

static void cmd_cd(int argc, char* argv[]) {
    if (argc < 2) {
        console_println("Usage: cd <directory>");
        return;
    }
    
    console_puts("DEBUG: Raw argument received: '");
    console_puts(argv[1]);
    console_puts("' (length: ");
    char len_buf[16];
    itoa(strlen(argv[1]), len_buf, 10);
    console_puts(len_buf);
    console_puts(")\n");
    
    console_puts("DEBUG: Changing to directory: ");
    console_println(argv[1]);
    
    if (fs_change_directory(argv[1]) != 0) {
        console_puts("cd: ");
        console_puts(argv[1]);
        console_println(": No such directory");
    } else {
        console_println("DEBUG: Directory changed successfully");
    }
}

static void cmd_pwd(int argc __attribute__((unused)), char* argv[] __attribute__((unused))) {
    char path[MAX_PATH_LENGTH];
    if (fs_getcwd(path, sizeof(path)) == FS_SUCCESS) {
        console_println(path);
    } else {
        console_println("Error getting current directory");
    }
}


static void cmd_mkdir(int argc, char* argv[]) {
    if (argc < 2) {
        console_println("Usage: mkdir <directory>");
        return;
    }

    if (fs_make_directory(argv[1]) != 0) {
        console_puts("mkdir: cannot create directory '");
        console_puts(argv[1]);
        console_println("'");
    }
}

static void cmd_rmdir(int argc, char* argv[]) {
    if (argc < 2) {
        console_println("Usage: rmdir <directory>");
        return;
    }
    if (fs_remove_directory(argv[1]) != 0) {
        console_puts("rmdir: cannot remove directory '");
        console_puts(argv[1]);
        console_println("'");
    }
}

static void cmd_rm(int argc, char* argv[]) {
    if (argc < 2) {
        console_println("Usage: rm <file>");
        return;
    }
    if (fs_delete_file(argv[1]) != 0) {
        console_puts("rm: cannot remove file '");
        console_puts(argv[1]);
        console_println("'");
    }
}

static void cmd_cat(int argc, char* argv[]) {
    if (argc < 2) {
        console_println("Usage: cat <file>");
        return;
    }
    
    char buffer[MAX_FILE_SIZE];
    int bytes_read = fs_read_file(argv[1], buffer, MAX_FILE_SIZE - 1);
    
    if (bytes_read < 0) {
        console_puts("cat: cannot read file '");
        console_puts(argv[1]);
        console_println("'");
        return;
    }
    
    buffer[bytes_read] = '\0'; 
    const char* ext = get_file_extension(argv[1]);
    
    console_puts("File: ");
    console_puts(argv[1]);
    console_println("");
    console_println("----------------------------------------");
    
    char line[256];
    int line_pos = 0;
    int buffer_pos = 0;
    
    while (buffer_pos < bytes_read) {
        if (buffer[buffer_pos] == '\n' || buffer[buffer_pos] == '\0') {
            line[line_pos] = '\0';
            if (line_pos > 0) {
                display_line_with_highlighting(line, ext);
            }
            line_pos = 0;
        } else if (line_pos < 255) {
            line[line_pos++] = buffer[buffer_pos];
        }
        buffer_pos++;
    }
    
    if (line_pos > 0) {
        line[line_pos] = '\0';
        display_line_with_highlighting(line, ext);
    }
    
    console_println("----------------------------------------");
}

static void cmd_cp(int argc, char* argv[]) {
    if (argc < 3) {
        console_println("Usage: cp <source> <destination>");
        return;
    }
    
    char buffer[MAX_FILE_SIZE];
    int bytes_read = fs_read_file(argv[1], buffer, MAX_FILE_SIZE);
    
    if (bytes_read < 0) {
        console_puts("cp: cannot read source file '");
        console_puts(argv[1]);
        console_println("'");
        return;
    }
    
    if (fs_write_file(argv[2], buffer, bytes_read) != 0) {
        console_puts("cp: cannot write to destination file '");
        console_puts(argv[2]);
        console_println("'");
        return;
    }
    
    console_puts("File '");
    console_puts(argv[1]);
    console_puts("' copied to '");
    console_puts(argv[2]);
    console_println("'");
}

static void cmd_mv(int argc, char* argv[]) {
    if (argc < 3) {
        console_println("Usage: mv <source> <destination>");
        return;
    }
    
    // First copy the file
    char buffer[MAX_FILE_SIZE];
    int bytes_read = fs_read_file(argv[1], buffer, MAX_FILE_SIZE);
    
    if (bytes_read < 0) {
        console_puts("mv: cannot read source file '");
        console_puts(argv[1]);
        console_println("'");
        return;
    }
    
    if (fs_write_file(argv[2], buffer, bytes_read) != 0) {
        console_puts("mv: cannot write to destination file '");
        console_puts(argv[2]);
        console_println("'");
        return;
    }
    
    // Then delete the original
    if (fs_delete_file(argv[1]) != 0) {
        console_puts("mv: warning - could not remove source file '");
        console_puts(argv[1]);
        console_println("'");
        return;
    }
    
    console_puts("File '");
    console_puts(argv[1]);
    console_puts("' moved to '");
    console_puts(argv[2]);
    console_println("'");
}

static void cmd_find(int argc, char* argv[]) {
    if (argc < 2) {
        console_println("Usage: find <filename>");
        console_println("Note: This is a simple find that searches in current directory");
        return;
    }
    
    console_puts("Searching for files matching '");
    console_puts(argv[1]);
    console_println("'...");
    
    char buffer[256];
    if (fs_read_file(argv[1], buffer, 1) >= 0) {
        console_puts("Found: ./");
        console_println(argv[1]);
    } else {
        console_puts("No files found matching '");
        console_puts(argv[1]);
        console_println("'");
    }
}

static void cmd_grep(int argc, char* argv[]) {
    if (argc < 3) {
        console_println("Usage: grep <pattern> <file>");
        console_println("Simple grep - searches for pattern in file");
        return;
    }
    
    char* pattern = argv[1];
    char* filename = argv[2];
    
    char buffer[MAX_FILE_SIZE];
    int bytes_read = fs_read_file(filename, buffer, MAX_FILE_SIZE - 1);
    
    if (bytes_read < 0) {
        console_puts("grep: cannot read file '");
        console_puts(filename);
        console_println("'");
        return;
    }
    
    buffer[bytes_read] = '\0';
    
    // simple line-based search
    char line[256];
    int line_pos = 0;
    int buffer_pos = 0;
    int line_number = 1;
    int matches_found = 0;
    
    while (buffer_pos <= bytes_read) {
        if (buffer[buffer_pos] == '\n' || buffer[buffer_pos] == '\0') {
            line[line_pos] = '\0';
            
            if (strstr(line, pattern) != NULL) {
                console_put_hex(line_number);
                console_puts(": ");
                console_println(line);
                matches_found++;
            }
            
            line_pos = 0;
            line_number++;
        } else if (line_pos < 255) {
            line[line_pos++] = buffer[buffer_pos];
        }
        buffer_pos++;
    }
    
    if (matches_found == 0) {
        console_puts("grep: no matches found for '");
        console_puts(pattern);
        console_puts("' in '");
        console_puts(filename);
        console_println("'");
    }
}

static void cmd_edit(int argc, char* argv[]) {
    if (argc < 2) {
        console_println("Usage: edit <filename>");
        return;
    }
    
    char* filename = argv[1];
    const char* ext = get_file_extension(filename);
    
    console_puts("Text Editor - Editing: ");
    console_puts(filename);
    console_println("");
    console_println("Enter text line by line. Commands:");
    console_println("  :w    - save file");
    console_println("  :q    - quit without saving"); 
    console_println("  :wq   - save and quit");
    console_println("  (empty line) - finish editing and save");
    console_println("----------------------------------------");
    
    // read existing content if file exists
    char file_buffer[MAX_FILE_SIZE];
    int existing_size = fs_read_file(filename, file_buffer, MAX_FILE_SIZE - 1);
    int total_size = 0;
    
    if (existing_size > 0) {
        file_buffer[existing_size] = '\0';
        console_println("Existing content:");
        
        // display with syntax highlighting
        char line[256];
        int line_pos = 0;
        
        for (int i = 0; i < existing_size; i++) {
            if (file_buffer[i] == '\n') {
                line[line_pos] = '\0';
                display_line_with_highlighting(line, ext);
                line_pos = 0;
            } else if (line_pos < 255) {
                line[line_pos++] = file_buffer[i];
            }
        }
        
        if (line_pos > 0) {
            line[line_pos] = '\0';
            display_line_with_highlighting(line, ext);
        }
        
        console_println("----------------------------------------");
        console_println("Append new content:");
        total_size = existing_size;
    }
    
    // edit loop
    char input_line[256];
    int should_save = 1;
    
    while (1) {
        console_puts("> ");
        console_gets(input_line, 256);
        
        // check for editor commands
        if (strcmp(input_line, ":q") == 0) {
            console_println("Quit without saving");
            should_save = 0;
            break;
        } else if (strcmp(input_line, ":w") == 0) {
            // save current buffer
            if (fs_write_file(filename, file_buffer, total_size) == 0) {
                console_println("File saved");
            } else {
                console_println("Error saving file");
            }
            continue;
        } else if (strcmp(input_line, ":wq") == 0) {
            console_println("Save and quit");
            break;
        } else if (strlen(input_line) == 0) {
            // empty line - finish editing
            console_println("Finished editing");
            break;
        }
        
        // add line to buffer
        int line_len = strlen(input_line);
        if (total_size + line_len + 1 < MAX_FILE_SIZE) {
            // copy line to buffer
            for (int i = 0; i < line_len; i++) {
                file_buffer[total_size + i] = input_line[i];
            }
            file_buffer[total_size + line_len] = '\n';
            total_size += line_len + 1;
            
            // show the line with syntax highlighting
            display_line_with_highlighting(input_line, ext);
        } else {
            console_println("Warning: File buffer full, line not added");
        }
    }
    
    // save file
    if (should_save) {
        if (fs_write_file(filename, file_buffer, total_size) == 0) {
            console_puts("File '");
            console_puts(filename);
            console_println("' saved successfully");
        } else {
            console_puts("Error: Could not save file '");
            console_puts(filename);
            console_println("'");
        }
    }
}

static void cmd_code(int argc, char* argv[]) {
    if (argc < 2) {
        console_println("Usage: code <filename>");
        console_println("Advanced code editor with language-specific features");
        return;
    }
    
    char* filename = argv[1];
    const char* ext = get_file_extension(filename);
    
    console_puts("ChipOS Code Editor - ");
    console_puts(filename);
    console_println("");
    
    // show language-specific info
    if (ext && (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0)) {
        console_println("Language: C/C++");
        console_println("Features: Keyword highlighting, syntax checking");
    } else if (ext && (strcmp(ext, ".v") == 0 || strcmp(ext, ".sv") == 0)) {
        console_println("Language: Verilog/SystemVerilog");
        console_println("Features: Module syntax, always block highlighting");
    } else if (ext && (strcmp(ext, ".s") == 0 || strcmp(ext, ".asm") == 0)) {
        console_println("Language: RISC-V Assembly");
        console_println("Features: Instruction highlighting, register names");
    } else {
        console_println("Language: Plain text");
    }
    
    console_println("Commands: :syntax - check syntax, :help - show help");
    console_println("         :w - save, :q - quit, :wq - save and quit");
    console_println("========================================");
    
    editor_start(filename);
}

static void cmd_compile(int argc, char* argv[]) {
    if (argc < 2) {
        console_println("Usage: compile <source_file>");
        console_println("Supported: .c, .v, .s files");
        return;
    }
    
    char* filename = argv[1];
    const char* ext = get_file_extension(filename);
    
    console_puts("Compiling: ");
    console_puts(filename);
    console_println("");
    
    char buffer[256];
    if (fs_read_file(filename, buffer, 1) < 0) {
        console_puts("Error: Source file '");
        console_puts(filename);
        console_println("' not found");
        return;
    }
    
    if (ext && strcmp(ext, ".c") == 0) {
        console_println("C Compiler: gcc-riscv64 (simulated)");
        console_println("Status: Compilation successful");
        console_println("Output: a.out");
    } else if (ext && (strcmp(ext, ".v") == 0 || strcmp(ext, ".sv") == 0)) {
        console_println("Verilog Compiler: iverilog (simulated)");
        console_println("Status: Synthesis successful"); 
        console_println("Output: design.vvp");
    } else if (ext && (strcmp(ext, ".s") == 0 || strcmp(ext, ".asm") == 0)) {
        console_println("Assembler: riscv64-as (simulated)");
        console_println("Status: Assembly successful");
        console_println("Output: program.o");
    } else {
        console_println("Error: Unsupported file type");
        console_println("Supported: .c (C), .v/.sv (Verilog), .s/.asm (Assembly)");
    }
}

static void cmd_run(int argc, char* argv[]) {
    if (argc < 2) {
        console_println("Usage: run <program>");
        console_println("Run compiled programs or scripts");
        return;
    }
    
    char* program = argv[1];
    
    console_puts("Running: ");
    console_puts(program);
    console_println("");
    console_println("========================================");
    
    // check if it's a common program name
    if (strcmp(program, "a.out") == 0) {
        console_println("Hello, ChipOS World!");
        console_println("Program executed successfully");
        console_println("Exit code: 0");
    } else if (strstr(program, ".vvp")) {
        console_println("Verilog simulation starting...");
        console_println("VCD file: dump.vcd");
        console_println("Simulation completed");
    } else if (strstr(program, ".o")) {
        console_println("Object file executed");
        console_println("RISC-V program completed");
    } else {
        // try to find and execute the file
        char buffer[256];
        if (fs_read_file(program, buffer, 256) >= 0) {
            console_println("Script/program found and executed");
        } else {
            console_puts("Error: Program '");
            console_puts(program);
            console_println("' not found");
        }
    }
    
    console_println("========================================");
}

static void cmd_syntax(int argc, char* argv[]) {
    if (argc < 2) {
        console_println("Usage: syntax <source_file>");
        console_println("Check syntax for C, Verilog, or Assembly files");
        return;
    }
    
    char* filename = argv[1];
    const char* ext = get_file_extension(filename);
    
    console_puts("Syntax checking: ");
    console_puts(filename);
    console_println("");
    
    char buffer[MAX_FILE_SIZE];
    int bytes_read = fs_read_file(filename, buffer, MAX_FILE_SIZE - 1);
    
    if (bytes_read < 0) {
        console_puts("Error: Cannot read file '");
        console_puts(filename);
        console_println("'");
        return;
    }
    
    buffer[bytes_read] = '\0';
    
    console_println("Basic syntax analysis:");
    console_println("----------------------");
    
    if (ext && (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0)) {
        // basic C syntax checking
        int brace_count = 0;
        int paren_count = 0;
        int line_num = 1;
        
        for (int i = 0; i < bytes_read; i++) {
            if (buffer[i] == '{') brace_count++;
            else if (buffer[i] == '}') brace_count--;
            else if (buffer[i] == '(') paren_count++;
            else if (buffer[i] == ')') paren_count--;
            else if (buffer[i] == '\n') line_num++;
        }
        
        console_println("C/C++ syntax check:");
        if (brace_count == 0) {
            console_println("✓ Braces balanced");
        } else {
            console_puts("✗ Unbalanced braces: ");
            console_put_hex(brace_count);
            console_println("");
        }
        
        if (paren_count == 0) {
            console_println("✓ Parentheses balanced");
        } else {
            console_puts("✗ Unbalanced parentheses: ");
            console_put_hex(paren_count);
            console_println("");
        }
        
        console_puts("Lines: ");
        console_put_hex(line_num);
        console_println("");
        
    } else if (ext && (strcmp(ext, ".v") == 0 || strcmp(ext, ".sv") == 0)) {
        console_println("Verilog syntax check:");
        
        // check for basic Verilog structure
        if (strstr(buffer, "module") && strstr(buffer, "endmodule")) {
            console_println("✓ Module structure found");
        } else {
            console_println("✗ Missing module/endmodule");
        }
        
        // check for begin/end balance
        char* ptr = buffer;
        int begin_count = 0;
        while ((ptr = strstr(ptr, "begin")) != NULL) {
            begin_count++;
            ptr += 5;
        }
        
        ptr = buffer;
        int end_count = 0;
        while ((ptr = strstr(ptr, "end")) != NULL) {
            
            if (strncmp(ptr, "endmodule", 9) != 0) {
                end_count++;
            }
            ptr += 3;
        }
        
        if (begin_count == end_count) {
            console_println("✓ Begin/end blocks balanced");
        } else {
            console_println("✗ Unbalanced begin/end blocks");
        }
        
    } else if (ext && (strcmp(ext, ".s") == 0 || strcmp(ext, ".asm") == 0)) {
        console_println("Assembly syntax check:");
        console_println("✓ Assembly file format");
        
        // count instructions vs directives
        int instruction_count = 0;
        int directive_count = 0;
        
        char line[256];
        int line_pos = 0;
        
        for (int i = 0; i <= bytes_read; i++) {
            if (buffer[i] == '\n' || buffer[i] == '\0') {
                line[line_pos] = '\0';
                
                // skipa empty lines and comments
                if (line_pos > 0 && line[0] != '#' && line[0] != ';') {
                    if (line[0] == '.') {
                        directive_count++;
                    } else {
                        instruction_count++;
                    }
                }
                
                line_pos = 0;
            } else if (line_pos < 255) {
                line[line_pos++] = buffer[i];
            }
        }
        
        console_puts("Instructions: ");
        console_put_hex(instruction_count);
        console_println("");
        console_puts("Directives: ");
        console_put_hex(directive_count);
        console_println("");
        
    } else {
        console_println("Unknown file type - basic text analysis:");
        console_puts("File size: ");
        console_put_hex(bytes_read);
        console_println(" bytes");
    }
    
    console_println("Syntax check completed");
}

static void cmd_exit(int argc __attribute__((unused)), char* argv[] __attribute__((unused))) {
    console_println("Goodbye!");
}

static void cmd_quit(int argc __attribute__((unused)), char* argv[] __attribute__((unused))) {
    cmd_exit(argc, argv);
}

static void cmd_touch(int argc, char* argv[]) {
    if (argc < 2) {
        console_println("Usage: touch <filename>");
        return;
    }
    
    char buffer[1];
    if (fs_read_file(argv[1], buffer, 1) >= 0) {
        console_puts("File '");
        console_puts(argv[1]);
        console_println("' already exists");
        return;
    }
    
    if (fs_write_file(argv[1], "", 0) == 0) {
        console_puts("Created file '");
        console_puts(argv[1]);
        console_println("'");
    } else {
        console_puts("touch: cannot create file '");
        console_puts(argv[1]);
        console_println("'");
    }
}

void console_gets_with_history(char* buffer, int max_len) {
    int pos = 0;
    int current_history = -1;
    char temp_buffer[MAX_COMMAND_LENGTH] = {0};
    
    while (1) {
        char c = console_getchar();
        
        if (c == '\n' || c == '\r') {
            buffer[pos] = '\0';
            console_putc('\n');
            
            // add to history if not empty
            if (pos > 0) {
                strcpy(command_history[history_count % HISTORY_SIZE], buffer);
                history_count++;
            }
            return;
        }
        else if (c == 0x1B) { // ESC sequence
            char seq1 = console_getchar();
            char seq2 = console_getchar();
            
            if (seq1 == '[') {
                if (seq2 == 'A' && history_count > 0) { // UP arrow
                    if (current_history == -1) {
                        strcpy(temp_buffer, buffer); // Save current input
                        current_history = (history_count - 1) % HISTORY_SIZE;
                    } else if (current_history != (history_count) % HISTORY_SIZE) {
                        current_history = (current_history - 1 + HISTORY_SIZE) % HISTORY_SIZE;
                    }
                    
                    // clear line and show history
                    for (int i = 0; i < pos; i++) console_putc('\b');
                    strcpy(buffer, command_history[current_history]);
                    pos = strlen(buffer);
                    console_puts(buffer);
                }
                else if (seq2 == 'B') { // DOWN arrow
                    if (current_history != -1) {
                        current_history = (current_history + 1) % HISTORY_SIZE;
                        
                        // clear line
                        for (int i = 0; i < pos; i++) console_putc('\b');
                        
                        if (current_history == history_count % HISTORY_SIZE) {
                            strcpy(buffer, temp_buffer); // restore original
                            current_history = -1;
                        } else {
                            strcpy(buffer, command_history[current_history]);
                        }
                        pos = strlen(buffer);
                        console_puts(buffer);
                    }
                }
            }
        }
        else if (c == '\b' || c == 127) { // backspace
            if (pos > 0) {
                pos--;
                console_putc('\b');
                console_putc(' ');
                console_putc('\b');
            }
        }
        else if (pos < max_len - 1) {
            buffer[pos++] = c;
            console_putc(c);
        }
    }
}
