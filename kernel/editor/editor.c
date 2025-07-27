#include "editor.h"
#include "../drivers/console.h"
#include "../fs/fs.h"
#include "../../lib/string.h"

// global editor state
static editor_state_t editor;

// C keywords
static const char* c_keywords[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "inline", "int", "long", "register", "restrict", "return", "short",
    "signed", "sizeof", "static", "struct", "switch", "typedef", "union",
    "unsigned", "void", "volatile", "while", "_Bool", "_Complex", "_Imaginary",
    "include", "define", "ifdef", "ifndef", "endif", "pragma", NULL
};

// verilog keywords
static const char* verilog_keywords[] = {
    "always", "and", "assign", "begin", "buf", "bufif0", "bufif1", "case",
    "casex", "casez", "cmos", "deassign", "default", "defparam", "disable",
    "edge", "else", "end", "endcase", "endfunction", "endmodule", "endprimitive",
    "endspecify", "endtable", "endtask", "event", "for", "force", "forever",
    "fork", "function", "highz0", "highz1", "if", "ifnone", "initial", "inout",
    "input", "integer", "join", "large", "macromodule", "medium", "module",
    "nand", "negedge", "nmos", "nor", "not", "notif0", "notif1", "or", "output",
    "parameter", "pmos", "posedge", "primitive", "pull0", "pull1", "pulldown",
    "pullup", "rcmos", "real", "realtime", "reg", "release", "repeat", "rnmos",
    "rpmos", "rtran", "rtranif0", "rtranif1", "scalared", "small", "specify",
    "specparam", "strong0", "strong1", "supply0", "supply1", "table", "task",
    "time", "tran", "tranif0", "tranif1", "tri", "tri0", "tri1", "triand",
    "trior", "trireg", "vectored", "wait", "wand", "weak0", "weak1", "while",
    "wire", "wor", "xnor", "xor", "logic", "bit", "byte", NULL
};

// RISC-V instructions
static const char* riscv_instructions[] = {
    "add", "addi", "sub", "lui", "auipc", "xor", "xori", "or", "ori", "and", "andi",
    "sll", "slli", "srl", "srli", "sra", "srai", "slt", "slti", "sltu", "sltiu",
    "beq", "bne", "blt", "bge", "bltu", "bgeu", "jal", "jalr", "lb", "lh", "lw",
    "lbu", "lhu", "sb", "sh", "sw", "fence", "fence.i", "ecall", "ebreak",
    "csrrw", "csrrs", "csrrc", "csrrwi", "csrrsi", "csrrci", "mul", "mulh",
    "mulhsu", "mulhu", "div", "divu", "rem", "remu", "lr.w", "sc.w", "amoswap.w",
    "amoadd.w", "amoxor.w", "amoand.w", "amoor.w", "amomin.w", "amomax.w",
    "amominu.w", "amomaxu.w", NULL
};

// RISC-V registers
static const char* riscv_registers[] = {
    "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11",
    "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19", "x20", "x21", "x22",
    "x23", "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31",
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1",
    "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6", "fp", NULL
};

// console utility function
void console_put_dec(unsigned int value) {
    if (value == 0) {
        console_putchar('0');
        return;
    }
    
    char digits[16];
    int i = 0;
    
    while (value > 0) {
        digits[i++] = (value % 10) + '0';
        value /= 10;
    }
    
    // print digits in reverse order
    while (i > 0) {
        console_putchar(digits[--i]);
    }
}

language_t detect_language(const char* filename) {
    if (!filename) return LANG_PLAIN;
    
    int len = strlen(filename);
    if (len < 2) return LANG_PLAIN;
    
    // find the last dot in the filename
    const char* ext = NULL;
    for (int i = len - 1; i >= 0; i--) {
        if (filename[i] == '.') {
            ext = &filename[i];
            break;
        }
    }
    
    // if no extension found, return plain text
    if (!ext) return LANG_PLAIN;
    
    // DEBUH: Print the detected extension (remove this after testing)
    console_puts("DEBUG: Detected extension: ");
    console_puts(ext);
    console_puts("\n");
    
    // compare extensions (make sure we're comparing the full extension including the dot)
    if (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0) {
        return LANG_C;
    }
    if (strcmp(ext, ".cpp") == 0 || strcmp(ext, ".cxx") == 0 || 
        strcmp(ext, ".cc") == 0 || strcmp(ext, ".hpp") == 0) {
        return LANG_C;
    }
    if (strcmp(ext, ".v") == 0) {
        return LANG_VERILOG;
    }
    if (strcmp(ext, ".sv") == 0 || strcmp(ext, ".svh") == 0) {
        return LANG_SYSTEMVERILOG;
    }
    if (strcmp(ext, ".s") == 0 || strcmp(ext, ".asm") == 0) {
        return LANG_RISCV_ASM;
    }
    
    return LANG_PLAIN;
}

bool is_c_keyword(const char* word) {
    for (int i = 0; c_keywords[i] != NULL; i++) {
        if (strcmp(word, c_keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool is_verilog_keyword(const char* word) {
    for (int i = 0; verilog_keywords[i] != NULL; i++) {
        if (strcmp(word, verilog_keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool is_riscv_instruction(const char* word) {
    for (int i = 0; riscv_instructions[i] != NULL; i++) {
        if (strcmp(word, riscv_instructions[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool is_riscv_register(const char* word) {
    for (int i = 0; riscv_registers[i] != NULL; i++) {
        if (strcmp(word, riscv_registers[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool is_alnum(char c) {
    return is_alpha(c) || is_digit(c);
}

char* extract_word(const char* line, int start, int* end) {
    static char word[64];
    int i = 0;
    
    while (start < strlen(line) && is_alnum(line[start]) && i < 63) {
        word[i++] = line[start++];
    }
    word[i] = '\0';
    *end = start;
    return word;
}

void editor_print_colored(const char* text, token_type_t type) {
    // simple color coding using text markers
    switch (type) {
        case TOKEN_KEYWORD:
            console_puts("[K]");
            console_puts(text);
            console_puts("[/K]");
            break;
        case TOKEN_COMMENT:
            console_puts("[C]");
            console_puts(text);
            console_puts("[/C]");
            break;
        case TOKEN_STRING:
            console_puts("[S]");
            console_puts(text);
            console_puts("[/S]");
            break;
        case TOKEN_NUMBER:
            console_puts("[N]");
            console_puts(text);
            console_puts("[/N]");
            break;
        case TOKEN_REGISTER:
            console_puts("[R]");
            console_puts(text);
            console_puts("[/R]");
            break;
        case TOKEN_INSTRUCTION:
            console_puts("[I]");
            console_puts(text);
            console_puts("[/I]");
            break;
        case TOKEN_PREPROCESSOR:
            console_puts("[P]");
            console_puts(text);
            console_puts("[/P]");
            break;
        default:
            console_puts(text);
            break;
    }
}

void editor_highlight_c(const char* line, int line_num) {
    int len = strlen(line);
    int i = 0;
    
    console_put_dec(line_num + 1);
    console_puts(": ");
    
    while (i < len) {
        if (line[i] == '/' && i + 1 < len && line[i + 1] == '/') {
            // single line comment
            char comment[MAX_LINE_LENGTH];
            strcpy(comment, line + i);
            editor_print_colored(comment, TOKEN_COMMENT);
            break;
        } else if (line[i] == '/' && i + 1 < len && line[i + 1] == '*') {
            // multi-line comment start
            console_puts("[C]/*");
            i += 2;
            while (i < len) {
                console_putchar(line[i]);
                if (line[i] == '*' && i + 1 < len && line[i + 1] == '/') {
                    console_puts("/[/C]");
                    i += 2;
                    break;
                }
                i++;
            }
        } else if (line[i] == '"') {
            // string literal
            console_puts("[S]\"");
            i++;
            while (i < len && line[i] != '"') {
                console_putchar(line[i]);
                if (line[i] == '\\' && i + 1 < len) {
                    i++;
                    console_putchar(line[i]);
                }
                i++;
            }
            if (i < len) console_puts("\"[/S]");
            i++;
        } else if (line[i] == '#') {
            // preprocessor
            char prep[MAX_LINE_LENGTH];
            strcpy(prep, line + i);
            editor_print_colored(prep, TOKEN_PREPROCESSOR);
            break;
        } else if (is_alpha(line[i])) {
            // word (potential keyword)
            int end;
            char* word = extract_word(line, i, &end);
            if (is_c_keyword(word)) {
                editor_print_colored(word, TOKEN_KEYWORD);
            } else {
                console_puts(word);
            }
            i = end;
        } else if (is_digit(line[i])) {
            // number
            char num[32];
            int j = 0;
            while (i < len && (is_digit(line[i]) || line[i] == '.') && j < 31) {
                num[j++] = line[i++];
            }
            num[j] = '\0';
            editor_print_colored(num, TOKEN_NUMBER);
        } else {
            console_putchar(line[i]);
            i++;
        }
    }
    console_puts("\n");
}

void editor_highlight_verilog(const char* line, int line_num) {
    int len = strlen(line);
    int i = 0;
    
    console_put_dec(line_num + 1);
    console_puts(": ");
    
    while (i < len) {
        if (line[i] == '/' && i + 1 < len && line[i + 1] == '/') {
            // single line comment
            char comment[MAX_LINE_LENGTH];
            strcpy(comment, line + i);
            editor_print_colored(comment, TOKEN_COMMENT);
            break;
        } else if (is_alpha(line[i])) {
            // word (potential keyword)
            int end;
            char* word = extract_word(line, i, &end);
            if (is_verilog_keyword(word)) {
                editor_print_colored(word, TOKEN_KEYWORD);
            } else {
                console_puts(word);
            }
            i = end;
        } else if (is_digit(line[i])) {
            // number or bit vector
            char num[64];
            int j = 0;
            while (i < len && (is_digit(line[i]) || line[i] == '\'' || 
                   line[i] == 'b' || line[i] == 'h' || line[i] == 'd' || 
                   line[i] == 'o' || (line[i] >= 'a' && line[i] <= 'f') ||
                   (line[i] >= 'A' && line[i] <= 'F')) && j < 63) {
                num[j++] = line[i++];
            }
            num[j] = '\0';
            editor_print_colored(num, TOKEN_NUMBER);
        } else {
            console_putchar(line[i]);
            i++;
        }
    }
    console_puts("\n");
}

void editor_highlight_riscv(const char* line, int line_num) {
    int len = strlen(line);
    int i = 0;
    
    console_put_dec(line_num + 1);
    console_puts(": ");
    
    while (i < len) {
        if (line[i] == '#') {
            // comment
            char comment[MAX_LINE_LENGTH];
            strcpy(comment, line + i);
            editor_print_colored(comment, TOKEN_COMMENT);
            break;
        } else if (is_alpha(line[i])) {
            // word (instruction or register)
            int end;
            char* word = extract_word(line, i, &end);
            if (is_riscv_instruction(word)) {
                editor_print_colored(word, TOKEN_INSTRUCTION);
            } else if (is_riscv_register(word)) {
                editor_print_colored(word, TOKEN_REGISTER);
            } else {
                console_puts(word);
            }
            i = end;
        } else if (is_digit(line[i]) || line[i] == '-') {
            // number or immediate
            char num[32];
            int j = 0;
            if (line[i] == '-') num[j++] = line[i++];
            while (i < len && (is_digit(line[i]) || line[i] == 'x' || 
                   (line[i] >= 'a' && line[i] <= 'f') ||
                   (line[i] >= 'A' && line[i] <= 'F')) && j < 31) {
                num[j++] = line[i++];
            }
            num[j] = '\0';
            editor_print_colored(num, TOKEN_NUMBER);
        } else {
            console_putchar(line[i]);
            i++;
        }
    }
    console_puts("\n");
}

void editor_display_line(int line_num, const char* line) {
    switch (editor.language) {
        case LANG_C:
            editor_highlight_c(line, line_num);
            break;
        case LANG_VERILOG:
        case LANG_SYSTEMVERILOG:
            editor_highlight_verilog(line, line_num);
            break;
        case LANG_RISCV_ASM:
            editor_highlight_riscv(line, line_num);
            break;
        default:
            console_put_dec(line_num + 1);
            console_puts(": ");
            console_puts(line);
            console_puts("\n");
            break;
    }
}

void editor_show_status(void) {
    console_puts("\n--- ChipOS Multi-Language Editor ---\n");
    console_puts("File: ");
    console_puts(editor.filename);
    console_puts(" | Language: ");
    
    switch (editor.language) {
        case LANG_C: console_puts("C/C++"); break;
        case LANG_VERILOG: console_puts("Verilog"); break;
        case LANG_SYSTEMVERILOG: console_puts("SystemVerilog"); break;
        case LANG_RISCV_ASM: console_puts("RISC-V Assembly"); break;
        default: console_puts("Plain Text"); break;
    }
    
    console_puts(" | Line: ");
    console_put_dec(editor.cursor_line + 1);
    console_puts("/");
    console_put_dec(editor.line_count);
    if (editor.modified) console_puts(" [MODIFIED]");
    console_puts("\n");
    console_puts("Commands: :w (save) :q (quit) :h (help) i (insert) ESC (command)\n");
    console_puts("========================================\n");
}

int editor_load_file(const char* filename) {
    char buffer[4096];
    int bytes_read = fs_read_file(filename, buffer, sizeof(buffer) - 1);
    
    if (bytes_read < 0) {
        // new file
        editor.line_count = 1;
        editor.lines[0][0] = '\0';
        return 0;
    }
    
    buffer[bytes_read] = '\0';
    editor.line_count = 0;
    
    // parse buffer into lines
    int i = 0, line_pos = 0;
    while (i < bytes_read && editor.line_count < MAX_LINES) {
        if (buffer[i] == '\n' || buffer[i] == '\0') {
            editor.lines[editor.line_count][line_pos] = '\0';
            editor.line_count++;
            line_pos = 0;
        } else if (line_pos < MAX_LINE_LENGTH - 1) {
            editor.lines[editor.line_count][line_pos++] = buffer[i];
        }
        i++;
    }
    
    if (editor.line_count == 0) {
        editor.line_count = 1;
        editor.lines[0][0] = '\0';
    }
    
    return 0;
}

int editor_save_file(void) {
    char buffer[4096];
    int pos = 0;
    
    for (int i = 0; i < editor.line_count && pos < sizeof(buffer) - 1; i++) {
        int line_len = strlen(editor.lines[i]);
        if (pos + line_len + 1 < sizeof(buffer)) {
            strcpy(buffer + pos, editor.lines[i]);
            pos += line_len;
            if (i < editor.line_count - 1) {
                buffer[pos++] = '\n';
            }
        }
    }
    buffer[pos] = '\0';
    
    int result = fs_write_file(editor.filename, buffer, pos);
    if (result == FS_SUCCESS) {
        editor.modified = false;
        console_puts("File saved successfully.\n");
        return 0;
    } else {
        console_puts("Error saving file.\n");
        return -1;
    }
}

void editor_show_help(void) {
    console_puts("\033[2J\033[H"); // clear screen
    console_puts("\n=== ChipOS Editor Help ===\n\n");
    
    console_puts("NAVIGATION:\n");
    console_puts("  Arrow Keys    - Move cursor in any direction\n");
    console_puts("  h/j/k/l       - Vim-style left/down/up/right\n");
    console_puts("  (Works in all modes)\n\n");
    
    console_puts("COMMAND MODE (default):\n");
    console_puts("  i             - Insert at cursor\n");
    console_puts("  a             - Append after cursor\n");
    console_puts("  A             - Append at end of line\n");
    console_puts("  o             - Open new line below\n");
    console_puts("  x             - Delete character under cursor\n");
    console_puts("  :w            - Save file\n");
    console_puts("  :q            - Quit (if no changes)\n");
    console_puts("  :q!           - Quit without saving\n");
    console_puts("  :wq           - Save and quit\n");
    console_puts("  :h or :help   - Show this help\n\n");
    
    console_puts("INSERT MODE:\n");
    console_puts("  Type normally - Insert text\n");
    console_puts("  ESC           - Return to command mode\n");
    console_puts("  Enter         - New line\n");
    console_puts("  Backspace/Del - Delete previous character\n");
    console_puts("  Arrow Keys    - Move cursor (still works!)\n\n");
    
    console_puts("READ-ONLY MODE:\n");
    console_puts("  Arrow Keys    - Navigate\n");
    console_puts("  h/j/k/l       - Vim navigation\n");
    console_puts("  :q            - Quit\n");
    console_puts("  :h            - Show help\n\n");
    
    console_puts("STATUS BAR INFO:\n");
    console_puts("  Shows: File | Language | Line/Total | [MODIFIED] or [READ-ONLY]\n");
    console_puts("  Current Mode: COMMAND or INSERT\n");
    console_puts("  Cursor Position: Line:Column\n\n");
    
    console_puts("SUPPORTED LANGUAGES:\n");
    console_puts("  - C/C++ (syntax highlighting)\n");
    console_puts("  - Verilog/SystemVerilog (syntax highlighting)\n");
    console_puts("  - RISC-V Assembly (syntax highlighting)\n");
    console_puts("  - Plain Text\n\n");
    
    console_puts("TIPS:\n");
    console_puts("  - Arrow keys work in ALL modes (command & insert)\n");
    console_puts("  - ESC always returns to command mode\n");
    console_puts("  - Syntax highlighting auto-detects from file extension\n");
    console_puts("  - View scrolls automatically when cursor moves off screen\n\n");
}

void editor_insert_char(char c) {
    int line_len = strlen(editor.lines[editor.cursor_line]);
    
    if (line_len < MAX_LINE_LENGTH - 1) {
        // shift characters to the right
        for (int i = line_len; i > editor.cursor_col; i--) {
            editor.lines[editor.cursor_line][i] = editor.lines[editor.cursor_line][i-1];
        }
        editor.lines[editor.cursor_line][editor.cursor_col] = c;
        editor.lines[editor.cursor_line][line_len + 1] = '\0';
        editor.cursor_col++;
        editor.modified = true;
    }
}

void editor_delete_char(void) {
    if (editor.cursor_col > 0) {
        int line_len = strlen(editor.lines[editor.cursor_line]);
        // shift characters to the left
        for (int i = editor.cursor_col - 1; i < line_len; i++) {
            editor.lines[editor.cursor_line][i] = editor.lines[editor.cursor_line][i+1];
        }
        editor.cursor_col--;
        editor.modified = true;
    }
}

void editor_new_line(void) {
    if (editor.line_count < MAX_LINES - 1) {
        // shift lines down
        for (int i = editor.line_count; i > editor.cursor_line + 1; i--) {
            strcpy(editor.lines[i], editor.lines[i-1]);
        }
        
        // split current line
        editor.line_count++;
        
        // copy text after cursor to new line
        strcpy(editor.lines[editor.cursor_line + 1], 
               editor.lines[editor.cursor_line] + editor.cursor_col);
        
        // truncate current line at cursor
        editor.lines[editor.cursor_line][editor.cursor_col] = '\0';
        
        // move cursor to start of new line
        editor.cursor_line++;
        editor.cursor_col = 0;
        editor.modified = true;
    }
}

int editor_start(const char* filename) {
    // initialize editor state
    memset(&editor, 0, sizeof(editor_state_t));
    strcpy(editor.filename, filename);
    editor.language = detect_language(filename);
    editor.cursor_line = 0;
    editor.cursor_col = 0;
    editor.view_start_line = 0;
    editor.view_height = 20;
    editor.modified = false;
    editor.insert_mode = false;
    
     char choice = '1';
     
    // check if file exists and handle accordingly
    char buffer[4096];
    int bytes_read = fs_read_file(filename, buffer, sizeof(buffer) - 1);
    
    if (bytes_read >= 0) {
        // file exists - ask user what to do
        console_puts("\n=== File Exists ===\n");
        console_puts("File '");
        console_puts(filename);
        console_puts("' already exists.\n\n");
        console_puts("Choose your editing mode:\n");
        console_puts("1. Edit existing content (modify the file)\n");
        console_puts("2. Append to end (add new content at the end)\n");
        console_puts("3. Overwrite completely (start fresh, lose existing content)\n");
        console_puts("4. View only (read-only mode)\n");
        console_puts("\nChoice (1-4): ");
        
        char choice = console_getchar();
        console_putchar(choice);
        console_puts("\n\n");
        
        switch (choice) {
            case '1':
                // load existing content for editing
                console_puts("Loading existing content for editing...\n");
                editor_load_file(filename);
                break;
                
            case '2':
                // load existing content, position cursor at end for appending
                console_puts("Loading file for appending...\n");
                editor_load_file(filename);
                // move cursor to end of last line
                editor.cursor_line = editor.line_count - 1;
                editor.cursor_col = strlen(editor.lines[editor.cursor_line]);
                // add a new line if the last line isn't empty
                if (strlen(editor.lines[editor.cursor_line]) > 0) {
                    editor_new_line();
                }
                editor.insert_mode = true; // start in insert mode for appending
                console_puts("Positioned at end of file. You're now in INSERT mode.\n");
                break;
                
            case '3':
                // start fresh (don't load existing content)
                console_puts("Starting with empty file (existing content will be overwritten when saved)...\n");
                editor.line_count = 1;
                editor.lines[0][0] = '\0';
                editor.modified = true; // mark as modified since we're discarding content
                break;
                
            case '4':
                // view-only mode
                console_puts("Opening in read-only mode...\n");
                editor_load_file(filename);
                break;
                
            default:
                console_puts("Invalid choice. Loading existing content for editing...\n");
                editor_load_file(filename);
                break;
        }
        
        console_puts("Press any key to start editing...");
        console_getchar();
        
    } else {
        // new file
        console_puts("Creating new file: ");
        console_puts(filename);
        console_puts("\n");
        editor.line_count = 1;
        editor.lines[0][0] = '\0';
    }
    
    // main editor loop 
char input;
bool running = true;
bool read_only = (bytes_read >= 0 && choice == '4');

while (running) {
    // clear screen and show editor interface
    
    console_puts("\033[2J\033[H"); // clear screen and move cursor to top
    
    // show status bar
    console_puts("\n--- ChipOS ---\n");
    console_puts("File: ");
    console_puts(editor.filename);
    console_puts(" | Language: ");
    
    switch (editor.language) {
        case LANG_C: console_puts("C/C++"); break;
        case LANG_VERILOG: console_puts("Verilog"); break;
        case LANG_SYSTEMVERILOG: console_puts("SystemVerilog"); break;
        case LANG_RISCV_ASM: console_puts("RISC-V Assembly"); break;
        default: console_puts("Plain Text"); break;
    }
    
    console_puts(" | Line: ");
    console_put_dec(editor.cursor_line + 1);
    console_puts("/");
    console_put_dec(editor.line_count);
    
    if (read_only) {
        console_puts(" [READ-ONLY]");
    } else if (editor.modified) {
        console_puts(" [MODIFIED]");
    }
    
    console_puts("\n");
    
    if (read_only) {
        console_puts("Commands: :q (quit) :h (help) ARROW KEYS (navigate)\n");
    } else {
        console_puts("Commands: :w (save) :q (quit) :h (help) i (insert) ESC (command) ARROW KEYS\n");
    }
    console_puts("========================================\n");
    
    // display file content
    int end_line = (editor.view_start_line + editor.view_height < editor.line_count) 
                  ? editor.view_start_line + editor.view_height 
                  : editor.line_count;
    
    for (int i = editor.view_start_line; i < end_line; i++) {
        // show cursor indicator
        if (i == editor.cursor_line) {
            console_puts(">");
        } else {
            console_puts(" ");
        }
        editor_display_line(i, editor.lines[i]);
    }
    
    // show current mode and cursor position
    console_puts("\n");
    if (read_only) {
        console_puts("Mode: VIEW-ONLY");
    } else {
        console_puts("Mode: ");
        console_puts(editor.insert_mode ? "INSERT" : "COMMAND");
    }
    console_puts(" | Cursor: ");
    console_put_dec(editor.cursor_line + 1);
    console_puts(":");
    console_put_dec(editor.cursor_col + 1);
    console_puts(" > ");
    
    // get user input
    input = console_getchar();
    
    // handle ESC sequences (arrow keys) - works in ALL modes
    if (input == 27) { // ESC key
        char seq1 = console_getchar();
        if (seq1 == '[') {
            char seq2 = console_getchar();
            
            // handle arrow keys
            switch (seq2) {
                case 'A': // UP arrow
                    if (editor.cursor_line > 0) {
                        editor.cursor_line--;
                        int line_len = strlen(editor.lines[editor.cursor_line]);
                        if (editor.cursor_col > line_len) {
                            editor.cursor_col = line_len;
                        }
                        // adjust view window
                        if (editor.cursor_line < editor.view_start_line) {
                            editor.view_start_line = editor.cursor_line;
                        }
                    }
                    continue; // skip rest of input handling
                    
                case 'B': // DOWN arrow  
                    if (editor.cursor_line < editor.line_count - 1) {
                        editor.cursor_line++;
                        int line_len = strlen(editor.lines[editor.cursor_line]);
                        if (editor.cursor_col > line_len) {
                            editor.cursor_col = line_len;
                        }
                        // adjust view window
                        if (editor.cursor_line >= editor.view_start_line + editor.view_height) {
                            editor.view_start_line = editor.cursor_line - editor.view_height + 1;
                        }
                    }
                    continue;
                    
                case 'C': // RIGHT arrow
                    {
                        int line_len = strlen(editor.lines[editor.cursor_line]);
                        if (editor.cursor_col < line_len) {
                            editor.cursor_col++;
                        }
                    }
                    continue;
                    
                case 'D': // LEFT arrow
                    if (editor.cursor_col > 0) {
                        editor.cursor_col--;
                    }
                    continue;
            }
        }
        
        // Handle ESC in insert mode only
        if (!read_only && editor.insert_mode) {
            editor.insert_mode = false;
            console_puts("\n[Switched to COMMAND mode]");
        }
        continue; 
    }
    
    //  MODE-SPECIFIC INPUT HANDLING
    if (read_only) {
        // read-only mode - only allow commands and quit
        switch (input) {
            case 'j': // move down 
                if (editor.cursor_line < editor.line_count - 1) {
                    editor.cursor_line++;
                    int line_len = strlen(editor.lines[editor.cursor_line]);
                    if (editor.cursor_col > line_len) {
                        editor.cursor_col = line_len;
                    }
                    // adjust view window
                    if (editor.cursor_line >= editor.view_start_line + editor.view_height) {
                        editor.view_start_line = editor.cursor_line - editor.view_height + 1;
                    }
                }
                break;
                
            case 'k': // move up 
                if (editor.cursor_line > 0) {
                    editor.cursor_line--;
                    int line_len = strlen(editor.lines[editor.cursor_line]);
                    if (editor.cursor_col > line_len) {
                        editor.cursor_col = line_len;
                    }
                    // Adjust view window
                    if (editor.cursor_line < editor.view_start_line) {
                        editor.view_start_line = editor.cursor_line;
                    }
                }
                break;
                
            case 'h': // move left 
                if (editor.cursor_col > 0) {
                    editor.cursor_col--;
                }
                break;
                
            case 'l': // Move right 
                {
                    int line_len = strlen(editor.lines[editor.cursor_line]);
                    if (editor.cursor_col < line_len) {
                        editor.cursor_col++;
                    }
                }
                break;
                
            case ':': // command mode
                console_puts("\nCommand: :");
                char cmd[16];
                int cmd_idx = 0;
                char cmd_char;
                while ((cmd_char = console_getchar()) != '\n' && cmd_char != '\r' && cmd_idx < 15) {
                    console_putchar(cmd_char);
                    cmd[cmd_idx++] = cmd_char;
                }
                cmd[cmd_idx] = '\0';
                
                if (strcmp(cmd, "q") == 0) {
                    running = false;
                } else if (strcmp(cmd, "h") == 0) {
                    editor_show_help();
                    console_puts("\nPress any key to continue...");
                    console_getchar();
                } else {
                    console_puts("\nRead-only mode. Available commands: :q, :h\n");
                    console_puts("Press any key to continue...");
                    console_getchar();
                }
                break;
        }
        
    } else if (editor.insert_mode) {
        // insert mode - handle text input
        if (input == '\n' || input == '\r') {
            editor_new_line();
        } else if (input == 127 || input == 8) { // backspace/delete
            editor_delete_char();
        } else if (input >= 32 && input <= 126) { // printable characters
            editor_insert_char(input);
        }
        // ignore other control characters
        
    } else {
        // command mode - handle navigation and commands
        switch (input) {
            case 'i': // enter insert mode
                editor.insert_mode = true;
                break;
                
            case 'a': // append (insert at end of line)
                editor.cursor_col = strlen(editor.lines[editor.cursor_line]);
                editor.insert_mode = true;
                break;
                
            case 'A': // append at end of line
                editor.cursor_col = strlen(editor.lines[editor.cursor_line]);
                editor.insert_mode = true;
                break;
                
            case 'o': // open new line below
                editor.cursor_col = strlen(editor.lines[editor.cursor_line]);
                editor_new_line();
                editor.insert_mode = true;
                break;
                
            case 'j': // move down
                if (editor.cursor_line < editor.line_count - 1) {
                    editor.cursor_line++;
                    int line_len = strlen(editor.lines[editor.cursor_line]);
                    if (editor.cursor_col > line_len) {
                        editor.cursor_col = line_len;
                    }
                    if (editor.cursor_line >= editor.view_start_line + editor.view_height) {
                        editor.view_start_line = editor.cursor_line - editor.view_height + 1;
                    }
                }
                break;
                
            case 'k': // move up 
                if (editor.cursor_line > 0) {
                    editor.cursor_line--;
                    int line_len = strlen(editor.lines[editor.cursor_line]);
                    if (editor.cursor_col > line_len) {
                        editor.cursor_col = line_len;
                    }
                    if (editor.cursor_line < editor.view_start_line) {
                        editor.view_start_line = editor.cursor_line;
                    }
                }
                break;
                
            case 'h': // move left
                if (editor.cursor_col > 0) {
                    editor.cursor_col--;
                }
                break;
                
            case 'l': // move right 
                {
                    int line_len = strlen(editor.lines[editor.cursor_line]);
                    if (editor.cursor_col < line_len) {
                        editor.cursor_col++;
                    }
                }
                break;
                
            case 'x': // delete character under cursor
                {
                    int line_len = strlen(editor.lines[editor.cursor_line]);
                    if (editor.cursor_col < line_len) {
                        // Shift characters left
                        for (int i = editor.cursor_col; i < line_len; i++) {
                            editor.lines[editor.cursor_line][i] = editor.lines[editor.cursor_line][i+1];
                        }
                        editor.modified = true;
                    }
                }
                break;
                
            case ':': // command line mode
                console_puts("\nCommand: :");
                char cmd[16];
                int cmd_idx = 0;
                char cmd_char;
                while ((cmd_char = console_getchar()) != '\n' && cmd_char != '\r' && cmd_idx < 15) {
                    console_putchar(cmd_char);
                    cmd[cmd_idx++] = cmd_char;
                }
                cmd[cmd_idx] = '\0';
                
                if (strcmp(cmd, "w") == 0) {
                    if (editor_save_file() == 0) {
                        console_puts("Press any key to continue...");
                        console_getchar();
                    }
                } else if (strcmp(cmd, "q") == 0) {
                    if (editor.modified) {
                        console_puts("\nFile has unsaved changes!");
                        console_puts("\nUse :q! to quit without saving, or :wq to save and quit.");
                        console_puts("\nPress any key to continue...");
                        console_getchar();
                    } else {
                        running = false;
                    }
                } else if (strcmp(cmd, "q!") == 0) {
                    console_puts("\nQuitting without saving...");
                    running = false;
                } else if (strcmp(cmd, "wq") == 0) {
                    if (editor_save_file() == 0) {
                        console_puts("File saved. Exiting...");
                        running = false;
                    } else {
                        console_puts("Error saving file. Press any key to continue...");
                        console_getchar();
                    }
                } else if (strcmp(cmd, "h") == 0 || strcmp(cmd, "help") == 0) {
                    editor_show_help();
                    console_puts("\nPress any key to continue...");
                    console_getchar();
                } else {
                    console_puts("\nUnknown command: ");
                    console_puts(cmd);
                    console_puts("\nType :h for help.");
                    console_puts("\nPress any key to continue...");
                    console_getchar();
                }
                break;
                
            default:
                // show brief help for unknown keys
                console_puts("\nUnknown key. Press :h for help or 'i' to insert text.");
                console_puts("\nPress any key to continue...");
                console_getchar();
                break;
        }
    }
}

console_puts("\nEditor closed.\n");
return 0;

     }
