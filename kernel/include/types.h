#ifndef TYPES_H
#define TYPES_H
#define _STDINT_H
#define _STDINT_GCC_H

typedef enum { bool_false = 0, bool_true = 1 } bool;
#define false bool_false
#define true bool_true
  
typedef long unsigned int uint64_t; 
typedef long int int64_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;

// architecture-specific types
#if __riscv_xlen == 64
typedef uint64_t size_t;
typedef uint64_t uintptr_t;
typedef int64_t  intptr_t;
#else
typedef uint32_t size_t;
typedef uint32_t uintptr_t;
typedef int32_t  intptr_t;
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#endif 
