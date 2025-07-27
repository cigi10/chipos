#include "string.h"
#include "../kernel/include/types.h"

void* memset(void* ptr, int value, unsigned long num) {
    unsigned char* p = (unsigned char*)ptr;
    for (unsigned long i = 0; i < num; i++) {
        p[i] = (unsigned char)value;
    }
    return ptr;
}

void* memcpy(void* dest, const void* src, unsigned long num) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    for (unsigned long i = 0; i < num; i++) {
        d[i] = s[i];
    }
    return dest;
}

unsigned long strlen(const char* str) {
    unsigned long len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
    return original_dest;
}

char* strcat(char* dest, const char* src) {
    char* original_dest = dest;
    // find the end of dest
    while (*dest != '\0') {
        dest++;
    }
    // copy src to the end of dest
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
    return original_dest;
}

char* strstr(const char* haystack, const char* needle) {
    if (*needle == '\0') {
        return (char*)haystack;
    }
    
    while (*haystack != '\0') {
        const char* h = haystack;
        const char* n = needle;
        
        while (*h == *n && *n != '\0') {
            h++;
            n++;
        }
        
        if (*n == '\0') {
            return (char*)haystack;
        }
        
        haystack++;
    }
    
    return (void*)0;
}
char* strrchr(const char* str, int c) {
    char* last_occurrence = (void*)0;
    
    while (*str != '\0') {
        if (*str == (char)c) {
            last_occurrence = (char*)str;
        }
        str++;
    }
    
    if (c == '\0') {
        return (char*)str;
    }
    
    return last_occurrence;
}

char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == c) {
            return (char*)str;
        }
        str++;
    }
    return (c == '\0') ? (char*)str : NULL;
}

int strncmp(const char* str1, const char* str2, unsigned long n) {
    while (n && *str1 && (*str1 == *str2)) {
        ++str1;
        ++str2;
        --n;
    }
    if (n == 0) {
        return 0;
    } else {
        return (*(unsigned char*)str1 - *(unsigned char*)str2);
    }
}

char* strncpy(char* dest, const char* src, size_t n) {
    char* ret = dest;
    while (n-- && (*dest++ = *src++));
    while (n--) *dest++ = '\0';
    return ret;
}

char* itoa(int value, char* str, int base) {
    char* ptr = str;
    char* ptr1 = str;
    char tmp_char;
    int tmp_value;

    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return str;
    }

    while (value != 0) {
        tmp_value = value % base;
        *ptr++ = (tmp_value < 10) ? (tmp_value + '0') : (tmp_value - 10 + 'a');
        value /= base;
    }

    *ptr-- = '\0';
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
    return str;
}

char* strtok(char* str, const char* delim) {
    static char* next = NULL;
    if (str) next = str;
    if (!next) return NULL;

    while (*next && strchr(delim, *next)) next++;
    if (!*next) return NULL;

    char* start = next;

    while (*next && !strchr(delim, *next)) next++;
    if (*next) {
        *next = '\0';
        next++;
    } else {
        next = NULL;
    }

    return start;
}
