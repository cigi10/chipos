#ifndef FS_H
#define FS_H

#include "../include/types.h"

// Constants
#define MAX_FILENAME 64
#define MAX_FILES 256
#define MAX_DIRS 64
#define MAX_FILE_SIZE 4096
#define MAX_PATH_LENGTH 256
#define MAX_PATH 256


// File types
typedef enum {
    FILE_TYPE_REGULAR,
    FILE_TYPE_DIRECTORY,
    FILE_TYPE_DEVICE
} file_type_t;

// Permission flags
#define PERM_READ 0x01
#define PERM_WRITE 0x02
#define PERM_EXEC 0x04

// File entry structure
typedef struct {
    char name[MAX_FILENAME];
    file_type_t type;
    uint32_t size;
    uint32_t permissions;
    uint32_t parent_id;
    uint32_t created_time;
    uint32_t modified_time;
    uint32_t data_offset;
} file_entry_t;

// Directory entry structure
typedef struct {
    uint32_t file_id;
    char name[MAX_FILENAME];
} dir_entry_t;

// Main filesystem structure
typedef struct {
    file_entry_t files[MAX_FILES];
    uint32_t file_count;
    uint32_t current_dir;
    uint32_t root_dir;
    char current_path[MAX_PATH_LENGTH];
    uint8_t data_storage[MAX_FILES * MAX_FILE_SIZE];
    uint32_t data_usage;
    uint32_t next_file_id;
} filesystem_t;

// Error codes - FIXED: All unique values
#define FS_SUCCESS 0
#define FS_ERROR_NOT_FOUND -1
#define FS_ERROR_INVALID_NAME -2
#define FS_ERROR_NO_SPACE -3
#define FS_ERROR_ALREADY_EXISTS -4
#define FS_ERROR_INVALID_PATH -5
#define FS_ERROR_NOT_DIRECTORY -6
#define FS_ERROR_NOT_EMPTY -7
#define FS_ERROR_PERMISSION_DENIED -8

// Helper function to convert error codes to strings
static inline const char* fs_error_string(int error_code) {
    switch (error_code) {
        case FS_SUCCESS: return "Success";
        case FS_ERROR_NOT_FOUND: return "File or directory not found";
        case FS_ERROR_INVALID_NAME: return "Invalid file or directory name";
        case FS_ERROR_NO_SPACE: return "No space left";
        case FS_ERROR_ALREADY_EXISTS: return "File or directory already exists";
        case FS_ERROR_INVALID_PATH: return "Invalid path";
        case FS_ERROR_NOT_DIRECTORY: return "Not a directory";
        case FS_ERROR_NOT_EMPTY: return "Directory not empty";
        case FS_ERROR_PERMISSION_DENIED: return "Permission denied";
        default: return "Unknown error";
    }
}

// Core file system functions
int fs_init(void);
int fs_create_file(const char* name, file_type_t type);
int fs_delete_file(const char* name);
int fs_write_file(const char* name, const void* data, uint32_t size);
int fs_read_file(const char* name, void* buffer, uint32_t size);
int fs_touch_file(const char* name);
file_entry_t* fs_get_file(const char* name);

// Directory operations
void fs_list_directory(int dir_id, bool long_listing);
int fs_make_directory(const char* name);
int fs_remove_directory(const char* name);

// Navigation functions
int fs_change_directory(const char* path);
char* fs_get_current_path_ptr(void);
int fs_get_current_path(char* buffer, uint32_t size);
int fs_resolve_path(const char* path);
uint32_t fs_get_current_dir_id(void);

// File operations
int fs_copy_file(const char* src, const char* dest);
int fs_move_file(const char* src, const char* dest);
int fs_find_file(const char* pattern);
int fs_grep_file(const char* filename, const char* pattern);
int fs_getcwd(char* buffer, uint32_t size);


// Global filesystem instance
extern filesystem_t fs;

#endif // FS_H