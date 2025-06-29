#ifndef STRING_H
#define STRING_H

#include <stddef.h> // For size_t

/* Memory functions */
void* memset(void* ptr, int value, size_t num);
void* memcpy(void* dest, const void* src, size_t num);
int memcmp(const void* ptr1, const void* ptr2, size_t num);

/* String functions */
size_t strlen(const char* str);
int strcmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t n);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);  // Added this critical function
char* strcat(char* dest, const char* src);
char* strncat(char* dest, const char* src, size_t n);
char* strstr(const char* haystack, const char* needle);
char* strchr(const char* str, int c);
char* strrchr(const char* str, int c);
char* strtok(char* str, const char* delim);

/* Additional useful functions */
char* strtok(char* str, const char* delim);
size_t strspn(const char* str1, const char* str2);
size_t strcspn(const char* str1, const char* str2);



char* itoa(int value, char* str, int base); 
#endif // STRING_H