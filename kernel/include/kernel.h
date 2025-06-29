#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"

// Kernel version information
#define KERNEL_NAME "ChipOS"
#define KERNEL_VERSION_MAJOR 0
#define KERNEL_VERSION_MINOR 1
#define KERNEL_VERSION_PATCH 0

// Kernel configuration
#define KERNEL_STACK_SIZE (4 * 1024)  // 4KB stack
#define MAX_INTERRUPTS 32

// RISC-V specific definitions
#define MSTATUS_MIE  (1 << 3)   // Machine Interrupt Enable
#define MSTATUS_MPIE (1 << 7)   // Machine Previous Interrupt Enable

// CSR (Control and Status Register) access macros
#define CSR_READ(csr) ({                    \
    unsigned long __v;                      \
    asm volatile ("csrr %0, " #csr          \
                  : "=r" (__v) :            \
                  : "memory");              \
    __v;                                    \
})

#define CSR_WRITE(csr, val) ({              \
    asm volatile ("csrw " #csr ", %0"       \
                  : : "rK" (val)            \
                  : "memory");              \
})

#define CSR_SET(csr, val) ({                \
    unsigned long __v = (unsigned long)(val); \
    asm volatile ("csrs " #csr ", %0"       \
                  : : "rK" (__v)            \
                  : "memory");              \
})

#define CSR_CLEAR(csr, val) ({              \
    unsigned long __v = (unsigned long)(val); \
    asm volatile ("csrc " #csr ", %0"       \
                  : : "rK" (__v)            \
                  : "memory");              \
})

// Function prototypes
void kernel_main(void);
void kernel_panic(const char* message);
void kernel_print_banner(void);

// Interrupt handling (stub for now)
typedef void (*interrupt_handler_t)(void);
void register_interrupt_handler(int irq, interrupt_handler_t handler);

// System information structure
typedef struct {
    uint32_t hart_id;
    uint32_t misa;
    uint64_t memory_size;
    const char* cpu_model;
} system_info_t;

extern system_info_t g_system_info;

// Utility macros
#define ALIGN_UP(addr, align) (((addr) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(addr, align) ((addr) & ~((align) - 1))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Debugging macros
#ifdef DEBUG
#define KERNEL_DEBUG(fmt, ...) console_printf("[DEBUG] " fmt, ##__VA_ARGS__)
#else
#define KERNEL_DEBUG(fmt, ...)
#endif

// Assert macro
#define KERNEL_ASSERT(condition) do {       \
    if (!(condition)) {                     \
        kernel_panic("Assertion failed: "  \
                    #condition " at "       \
                    __FILE__ ":"            \
                    __stringify(__LINE__)); \
    }                                       \
} while(0)

#define __stringify_1(x) #x
#define __stringify(x) __stringify_1(x)

#endif