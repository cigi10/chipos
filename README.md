# ChipOS

A minimal RISC-V operating system with integrated development tools for embedded systems and computer architecture education. ChipOS provides syntax-aware text editing, file management, and a complete shell environment that runs directly on RISC-V hardware or QEMU.

Built as a microkernel with modular components, ChipOS offers a practical platform for learning OS development while providing useful tools for hardware design workflows.

## Features

### Development Environment
- **Dual text editors**: 
  - `edit` - Basic editor with insert mode and save/quit commands
  - `code` - VIM-style modal editor with advanced navigation
- **Multi-language syntax highlighting** for C/C++, Verilog/SystemVerilog, and RISC-V assembly
- **Interactive shell** with command history and tab completion
- **File system operations** with hierarchical directory support

### System Components
- **Memory Management** - Dynamic heap allocation with usage tracking and leak detection
- **Filesystem** - In-memory hierarchical filesystem with metadata management
- **UART Driver** - Serial communication for hardware interfacing
- **Debug Support** - Built-in system introspection and memory diagnostics

### Hardware Integration
- **RISC-V Native** - Optimized for RV64IMAC instruction set
- **Bare Metal** - No external dependencies, fully self-contained kernel
- **QEMU Compatible** - Runs seamlessly on qemu-system-riscv64
- **Lightweight** - Complete system runs in ~64KB of RAM

## Quick Start

### Requirements
- RISC-V GNU Toolchain (`riscv64-unknown-elf-gcc`)
- QEMU with RISC-V support (`qemu-system-riscv64`)
- GNU Make

### Installation
```bash
# install toolchain (Ubuntu/Debian)
sudo apt install gcc-riscv64-linux-gnu qemu-system-misc

# clone and build
git clone https://github.com/cigi10/chipos.git
cd chipos
make clean && make

# run in QEMU
make run
# or manually: qemu-system-riscv64 -machine virt -bios none -kernel build/kernel.bin -nographic
```

## Usage Guide

### Basic Workflow
```bash
# file system navigation
ls                    # list current directory
mkdir hardware        # create project directory
cd hardware
pwd                   # show current path

# file operations
touch cpu.v           # create new Verilog file
code cpu.v            # edit with VIM-style editor
cat cpu.v             # view with syntax highlighting
rm old_file.c         # remove file
```

### Text Editors

**Basic Editor (`edit`)**
- Simple insert mode editing
- `:w` to save, `:q` to quit, `:wq` to save and quit
- Arrow key navigation

**VIM-style Editor (`code`)**
- Modal editing (normal/insert modes) 
- Standard VIM keybindings for navigation
- `:w`, `:q`, `:wq` commands
- Advanced cursor movement

## Command Reference

### File Operations
| Command | Description | Example |
|---------|-------------|---------|
| `ls [dir]` | List directory contents | `ls /projects` |
| `cd <dir>` | Change directory | `cd src` |
| `pwd` | Print working directory | `pwd` |
| `mkdir <dir>` | Create directory | `mkdir tests` |
| `rmdir <dir>` | Remove empty directory | `rmdir old` |
| `rm <file>` | Delete file | `rm temp.txt` |
| `touch <file>` | Create empty file | `touch main.c` |
| `cat <file>` | Display file with highlighting | `cat program.v` |
| `edit <file>` | Open in basic editor | `edit config.h` |
| `code <file>` | Open in VIM editor | `code algorithm.c` |

### System Commands
| Command | Description | Example |
|---------|-------------|---------|
| `about` | Show system information | `about` |
| `mem` | Display memory usage | `mem` |
| `calc <expr>` | Evaluate expression | `calc 16 * 1024` |
| `clear` | Clear screen | `clear` |
| `echo <text>` | Print text | `echo "Hello World"` |
| `colortest` | Test terminal colors | `colortest` |

## Syntax Highlighting

### C/C++ (`.c`, `.h`, `.cpp`)
- **Keywords**: `if`, `while`, `for`, `struct`, `typedef`, `return`
- **Types**: `int`, `char`, `void`, `unsigned`, `static`
- **Preprocessor**: `#include`, `#define`, `#ifdef`
- **Literals**: Strings, characters, numbers
- **Comments**: `//` and `/* */` style

### Verilog/SystemVerilog (`.v`, `.sv`)
- **Keywords**: `module`, `endmodule`, `always`, `assign`
- **Types**: `wire`, `reg`, `input`, `output`, `logic`
- **Constructs**: `begin`/`end`, `case`/`endcase`
- **Operators**: Bitwise, logical, arithmetic
- **Comments**: `//` and `/* */` style

### RISC-V Assembly (`.s`, `.asm`)
- **Instructions**: `add`, `sub`, `lw`, `sw`, `beq`, `jal`
- **Registers**: Both numeric (`x0`-`x31`) and ABI names (`zero`, `ra`, `sp`)
- **Labels**: Jump targets and function names
- **Directives**: `.text`, `.data`, `.global`
- **Immediates**: Hex (`0x1000`), decimal, binary (`0b1010`)

## Architecture

```
ChipOS/
├── boot/               # boot sequence and initialization
│   ├── boot.s          # RISC-V assembly bootstrap
│   └── linker.ld       # memory layout and linking script
├── kernel/             # core kernel components
│   ├── drivers/        # hardware abstraction layer
│   ├── editor/         # text editing subsystem
│   ├── fs/             # filesystem implementation
│   ├── include/        # kernel header files
│   ├── memory/         # memory management
│   ├── shell/          # interactive shell
│   │   ├── shell.c     # command interpreter
│   │   └── shell.h     # shell interface definitions
│   └── kernel.c        # main kernel entry point
├── lib/                # utility library
│   ├── string.c        # string manipulation functions
│   └── string.h        # string library headers
├── build/              # build artifacts
│   ├── *.o             # object files (boot, console, editor, etc.)
│   ├── kernel.elf      # ELF executable with debug symbols
│   └── kernel.bin      # final stripped kernel binary
├── .vscode/            # VS Code configuration
├── Makefile            # build system
└── README.md           
```

## Technical Details

## Technical Details

### Memory Layout
| Region | Purpose | Address Range |
|--------|---------|---------------|
| Bootloader | Assembly initialization | `0x80000000–0x80000FFF` |
| Kernel Text | Code and data sections | `0x80001000–0x8001FFFF` |
| Heap | Dynamic allocations | `0x80020000–...` (grows up) |
| Stack | Kernel stack | Top-down from high RAM |
| File Data | In-memory file storage | Custom allocation region |

- **Heap**: Basic `kmalloc()` allocator with bump pointer
- **Stack**: Fixed size (4KB-16KB) per execution context

### Boot Sequence
1. **Power-On Reset** - Execution begins at `boot/boot.s`
2. **Stack Setup** - Initialize stack pointer and clear BSS section  
3. **Kernel Main** - Control passes to `kernel_main()` in C
4. **Subsystem Init** - Initialize console/UART, memory manager, filesystem, shell, editor
5. **Shell Start** - Begin interactive command loop

### Filesystem Internals
ChipOS uses an in-memory filesystem (RAMFS-like) with:
- **File Table**: Flat array of file entries in RAM
- **File Structure**: `name`, `size`, `start_block`, `flags`, `used`
- **Operations**: `fs_create`, `fs_delete`, `fs_read`, `fs_write`
- **Limitations**: No nested directories yet, ~64 file limit (configurable)

### Editor Implementation

**Basic Editor (`edit`)**
- Line-based editing with in-memory buffer (`char editor_buffer[MAX_SIZE]`)
- Commands: `:w` (save), `:q` (quit), `:wq` (save and quit)
- Syntax highlighting via file extension detection

**VIM-style Editor (`code`)**  
- Modal editing with cursor navigation
- Arrow key movement and insert mode
- Same syntax highlighting engine as basic editor

**Syntax Highlighting Engine**
- Detects language by file extension
- Applies ANSI color codes for keywords, comments, strings
- Supports nested constructs (comments within code blocks)

### Build Configuration
- **Target**: RISC-V 64-bit (RV64IMAC)
- **ABI**: LP64
- **Code Model**: `medany` (position-independent)
- **Optimization**: `-O2` with debug symbols
- **Freestanding**: No standard library dependencies

## Development Guide

### Building and Running
```bash
# clean build
make clean && make

# run in QEMU
qemu-system-riscv64 -machine virt -bios none -kernel build/kernel.bin -nographic
```

### Sample Workflows

**Creating a C Program**
```bash
touch main.c
code main.c          # write your C code
cat main.c           # view with syntax highlighting
```

**Working with Verilog**
```bash
touch alu.v
edit alu.v           # create Verilog module
cat alu.v            # see keyword highlighting
```

**RISC-V Assembly**
```bash
touch program.s
code program.s       # write assembly code
cat program.s        # view with instruction highlighting
```

### Performance Characteristics
| Metric | Value |
|--------|-------|
| Boot time | ~10ms in QEMU |
| RAM usage | ~128KB static + dynamic heap |
| File system | Max 64 entries (configurable) |
| Editor buffer | ~4KB per file |

### Debugging Tips
| Issue | Solution |
|-------|---------|
| File not opening | Check if `used == 1` and name is valid |
| Editor crashes | Check buffer overflows or null pointers |
| Shell unresponsive | Confirm `uart_read` isn't blocked |
| QEMU freeze | Check for infinite loops or panic triggers |

Use `console_println()` for debug tracing throughout the code.

- **No Persistent Storage**: Files don't survive reboot
- **Single Process**: No multitasking or process isolation
- **Limited Memory**: Fixed heap size, no virtual memory
- **Basic Networking**: No network stack implementation
- **QEMU Only**: Primarily tested on QEMU, hardware support limited

