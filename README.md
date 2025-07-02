# ChipOS

A specialized RISC-V operating system designed for hardware development and embedded systems education. ChipOS provides an integrated development environment with multi-language syntax highlighting, making it perfect for ECE students and hardware engineers.

## Features

### Development Environment
- **Multi-language syntax highlighting** for C/C++, Verilog/SystemVerilog, and RISC-V Assembly
- **Built-in text editor** with language-aware features and syntax coloring
- **Interactive shell** with comprehensive command set
- **File management** with directory operations and file manipulation

### System Components
- **Memory Management** - Dynamic allocation with heap tracking
- **Filesystem** - Hierarchical directory structure with full CRUD operations
- **UART Communication** - Serial input/output for hardware interaction
- **Debug Support** - Extensive debugging output and system introspection

### Hardware Focus
- **RISC-V Native** - Built specifically for RISC-V architecture
- **Hardware Development Workflow** - Optimized for digital design and embedded programming
- **Educational Tools** - Perfect for learning computer architecture and OS development

## System Requirements

- **Architecture**: RISC-V 64-bit
- **Emulator**: QEMU RISC-V system emulation
- **Toolchain**: RISC-V GNU toolchain
- **Build System**: GNU Make

## Building ChipOS

```bash
# Clone the repository
git clone https://github.com/yourusername/chipos.git
cd chipos

# Build the OS
make clean && make

# Run in QEMU
qemu-system-riscv64 -machine virt -bios none -kernel build/kernel.bin -nographic
```

## Quick Start

After booting ChipOS, you'll see the interactive shell. Try these commands:

```bash
# Explore the filesystem
ls
mkdir myproject
cd myproject

# Create and edit files
touch program.c
edit program.c
# (Write some C code with syntax highlighting)

# View files with syntax highlighting
cat program.c

# System information
about
mem
```

## Supported Languages

### C/C++ (.c, .h)
- Keyword highlighting for control structures, data types
- Comment and string literal highlighting
- Preprocessor directive support

### Verilog/SystemVerilog (.v, .sv)
- Module, always, and wire keyword highlighting
- Port and signal highlighting
- Comment support

### RISC-V Assembly (.s, .asm)
- Instruction highlighting (arithmetic, logic, memory, control)
- Register name highlighting (both x and ABI names)
- Label and comment support
- Number format support (hex, decimal, binary)

## 🎮 Shell Commands

### File Operations
- `ls [dir]` - List directory contents
- `cd <dir>` - Change directory  
- `pwd` - Print working directory
- `mkdir <dir>` - Create directory
- `rmdir <dir>` - Remove directory
- `rm <file>` - Remove file
- `touch <file>` - Create empty file
- `cat <file>` - Display file with syntax highlighting

### System Commands
- `about` - Show system information
- `mem` - Show memory usage
- `calc <expr>` - Simple calculator
- `clear` - Clear screen
- `echo <text>` - Echo text
- `colortest` - Test color support

### Editor Commands
- `:w` - Save file
- `:q` - Quit without saving
- `:wq` - Save and quit
- Navigation with arrow keys
- Insert mode for editing

## Architecture

ChipOS is structured as a microkernel with the following components:

```
ChipOS/
├── boot/           # Boot loader and linker scripts
├── kernel/         # Core kernel components
│   ├── drivers/    # Hardware drivers (UART, console)
│   ├── memory/     # Memory management
│   ├── shell/      # Interactive shell
│   ├── editor/     # Text editor with syntax highlighting
│   └── fs/         # Filesystem implementation
├── lib/            # Standard library functions
└── build/          # Build artifacts
```

## Educational Value

ChipOS demonstrates key operating system concepts:

- **Boot Process** - From assembly bootstrap to C kernel
- **Memory Management** - Heap allocation and memory tracking
- **I/O Systems** - UART driver implementation
- **File Systems** - Hierarchical directory structures
- **User Interface** - Shell and editor implementation
- **Hardware Abstraction** - RISC-V specific optimizations

## Technical Details

- **Language**: C with RISC-V assembly bootstrap
- **Architecture**: RISC-V 64-bit (RV64IMAC)
- **Memory Model**: medany code model
- **Build Flags**: Freestanding environment, no standard library
- **Debugging**: Extensive debug output for development

## Contributing

Contributions are welcome! Areas for improvement:

- Additional language support
- Compiler/assembler integration
- Network stack implementation
- Graphics mode support
- Hardware driver expansion

## License

This project is open source. See LICENSE file for details.

## Acknowledgments

Built for educational purposes and hardware development workflows. Special thanks to the RISC-V community and open-source toolchain maintainers.

