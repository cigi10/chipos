#include "fs.h"
#include "../drivers/console.h"
#include "../../lib/string.h"

filesystem_t fs;
static uint32_t system_time = 0;

int fs_init(void) {
    // clear the entire filesystem structure
    memset(&fs, 0, sizeof(filesystem_t));
    
    console_puts("[DEBUG] Initializing filesystem...\n");
    
    // initialize root directory (ID 0)
    fs.files[0].name[0] = '/';
    fs.files[0].name[1] = '\0';
    fs.files[0].type = FILE_TYPE_DIRECTORY;
    fs.files[0].size = 0;
    fs.files[0].permissions = PERM_READ | PERM_WRITE | PERM_EXEC;
    fs.files[0].parent_id = 0;  // Root is its own parent
    fs.files[0].created_time = system_time++;
    fs.files[0].modified_time = system_time;
    fs.files[0].data_offset = 0;
    
    // initialize filesystem metadata
    fs.file_count = 1;          // Start with 1 (root directory)
    fs.current_dir = 0;         // Start in root directory
    fs.root_dir = 0;            // Root directory ID
    fs.next_file_id = 1;        // Next available file ID
    strcpy(fs.current_path, "/");
    fs.data_usage = 0;
    
    console_puts("[DEBUG] Root directory created (ID: 0)\n");
    console_puts("[DEBUG] Initial file_count: 1\n");
    console_puts("[DEBUG] Initial next_file_id: 1\n");
    
    // create initial system directories
    console_puts("[DEBUG] Creating initial directories...\n");
    
    int result = fs_make_directory("home");
    console_puts("[DEBUG] fs_make_directory(\"home\") result: ");
    char buf[16];
    itoa(result, buf, 10);
    console_puts(buf);
    console_puts(", file_count now: ");
    itoa(fs.file_count, buf, 10);
    console_puts(buf);
    console_puts("\n");
    
    result = fs_make_directory("bin");
    console_puts("[DEBUG] fs_make_directory(\"bin\") result: ");
    itoa(result, buf, 10);
    console_puts(buf);
    console_puts(", file_count now: ");
    itoa(fs.file_count, buf, 10);
    console_puts(buf);
    console_puts("\n");
    
    result = fs_make_directory("etc");
    console_puts("[DEBUG] fs_make_directory(\"etc\") result: ");
    itoa(result, buf, 10);
    console_puts(buf);
    console_puts(", file_count now: ");
    itoa(fs.file_count, buf, 10);
    console_puts(buf);
    console_puts("\n");
    
    console_puts("[DEBUG] fs_init completed. Final file_count: ");
    itoa(fs.file_count, buf, 10);
    console_puts(buf);
    console_puts(", next_file_id: ");
    itoa(fs.next_file_id, buf, 10);
    console_puts(buf);
    console_puts("\n");
    
    // DEBUG: show what directories were actually created
    console_puts("[DEBUG] Directory table after init:\n");
    for (uint32_t i = 0; i < fs.file_count; i++) {
        console_puts("[DEBUG] ID ");
        itoa(i, buf, 10);
        console_puts(buf);
        console_puts(": name='");
        console_puts(fs.files[i].name);
        console_puts("', parent=");
        itoa(fs.files[i].parent_id, buf, 10);
        console_puts(buf);
        console_puts(", type=");
        console_puts((fs.files[i].type == FILE_TYPE_DIRECTORY) ? "DIR" : "FILE");
        console_puts("\n");
    }
    
    return FS_SUCCESS;
}

static int find_file_in_dir(uint32_t dir_id, const char* name) {
    for (uint32_t i = 0; i < fs.file_count; i++) {
        if (fs.files[i].parent_id == dir_id && strcmp(fs.files[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int fs_create_file(const char* name, file_type_t type) {
    if (fs.file_count >= MAX_FILES) return FS_ERROR_NO_SPACE;
    if (find_file_in_dir(fs.current_dir, name) >= 0) return FS_ERROR_ALREADY_EXISTS;

    uint32_t new_index = fs.file_count;
    strncpy(fs.files[new_index].name, name, MAX_FILENAME - 1);
    fs.files[new_index].name[MAX_FILENAME - 1] = '\0';
    fs.files[new_index].type = type;
    fs.files[new_index].size = 0;
    fs.files[new_index].permissions = (type == FILE_TYPE_DIRECTORY)
        ? (PERM_READ | PERM_WRITE | PERM_EXEC)
        : (PERM_READ | PERM_WRITE);
    fs.files[new_index].parent_id = fs.current_dir;
    fs.files[new_index].created_time = system_time++;
    fs.files[new_index].modified_time = system_time;
    fs.files[new_index].data_offset = fs.data_usage;

    fs.file_count++;
    return FS_SUCCESS;
}

int fs_write_file(const char* name, const void* data, uint32_t size) {
    int file_id = find_file_in_dir(fs.current_dir, name);
    if (file_id < 0) {
        if (fs_create_file(name, FILE_TYPE_REGULAR) != FS_SUCCESS) return FS_ERROR_NO_SPACE;
        file_id = find_file_in_dir(fs.current_dir, name);
    }

    if (fs.files[file_id].type != FILE_TYPE_REGULAR) return FS_ERROR_INVALID_PATH;
    if (size > MAX_FILE_SIZE) size = MAX_FILE_SIZE;

    memcpy(&fs.data_storage[fs.files[file_id].data_offset], data, size);
    fs.files[file_id].size = size;
    fs.files[file_id].modified_time = system_time++;

    if (fs.files[file_id].data_offset + size > fs.data_usage) {
        fs.data_usage = fs.files[file_id].data_offset + size;
    }

    return FS_SUCCESS;
}

int fs_read_file(const char* name, void* buffer, uint32_t size) {
    int file_id = find_file_in_dir(fs.current_dir, name);
    if (file_id < 0 || fs.files[file_id].type != FILE_TYPE_REGULAR) return FS_ERROR_INVALID_PATH;

    uint32_t read_size = (size < fs.files[file_id].size) ? size : fs.files[file_id].size;
    memcpy(buffer, &fs.data_storage[fs.files[file_id].data_offset], read_size);
    return read_size;
}

void fs_list_directory(int dir_id, bool long_listing) {
    if (dir_id < 0 || fs.files[dir_id].type != FILE_TYPE_DIRECTORY) {
        console_puts("ls: not a directory\n");
        return;
    }

    if (long_listing) {
        console_puts("Type  Size     Name        ParentID\n");
        console_puts("----  -------- ----------- --------\n");
    }

    for (uint32_t i = 0; i < fs.file_count; i++) {
        file_entry_t* f = &fs.files[i];
        if (f->parent_id == (uint32_t)dir_id) {
            if (long_listing) {
                char size_buf[16];
                char parent_buf[16];

                itoa(f->size, size_buf, 16);
                itoa(f->parent_id, parent_buf, 16);

                // print type
                console_putc((f->type == FILE_TYPE_DIRECTORY) ? 'd' : 'f');
                console_puts("     0x");
                
                // print size
                console_puts(size_buf);
                console_puts(" ");
                
                // print name
                console_puts(f->name);
                
                // align spacing manually if needed (optional)
                console_puts(" 0x");
                
                // print parent ID
                console_puts(parent_buf);
                console_puts("\n");
            } else {
                console_puts(f->name);
                console_puts("\n");
            }
        }
    }
}

static void rebuild_current_path() {
    if (fs.current_dir == fs.root_dir) {
        strcpy(fs.current_path, "/");
        return;
    }

    char stack[32][MAX_FILENAME];
    int depth = 0;
    uint32_t curr = fs.current_dir;

    // traverse up to root, pushing names onto stack
    while (curr != fs.root_dir && depth < 32) {
        strcpy(stack[depth], fs.files[curr].name);
        curr = fs.files[curr].parent_id;
        depth++;
    }

    // start path with root
    strcpy(fs.current_path, "/");

    // rebuild path from stack
    for (int i = depth - 1; i >= 0; i--) {
        strcat(fs.current_path, stack[i]);
        if (i != 0) strcat(fs.current_path, "/");
    }

    // fallback in case something went wrong
    if (fs.current_path[0] == '\0') {
        strcpy(fs.current_path, "/");
    }
}

int fs_change_directory(const char* path) {
    console_puts("[DEBUG] fs_change_directory called with path: ");
    console_puts(path);
    console_puts("\n");

    if (!path || strlen(path) == 0) {
        console_puts("[DEBUG] Invalid path (empty or NULL)\n");
        return FS_ERROR_INVALID_PATH;
    }

    if (strcmp(path, ".") == 0) {
        console_puts("[DEBUG] Staying in the same directory\n");
        return FS_SUCCESS;
    }

    if (strcmp(path, "..") == 0) {
        uint32_t parent = fs.files[fs.current_dir].parent_id;
        console_puts("[DEBUG] Going up: ");
        char buf[16];
        itoa(fs.current_dir, buf, 16);
        console_puts("from 0x");
        console_puts(buf);
        itoa(parent, buf, 16);
        console_puts(" -> 0x");
        console_puts(buf);
        console_puts("\n");

        fs.current_dir = parent;

        rebuild_current_path();

        return FS_SUCCESS;
    }

    int target_id = find_file_in_dir(fs.current_dir, path);
    console_puts("[DEBUG] find_file_in_dir returned: 0x");
    char buf[16];
    itoa(target_id, buf, 16);
    console_puts(buf);
    console_puts("\n");

    if (target_id < 0) {
        console_puts("[DEBUG] Directory not found\n");
        return FS_ERROR_NOT_FOUND;
    }

    if (fs.files[target_id].type != FILE_TYPE_DIRECTORY) {
        console_puts("[DEBUG] Target is not a directory\n");
        return FS_ERROR_NOT_DIRECTORY;
    }

    console_puts("[DEBUG] Changing current_dir: 0x");
    itoa(fs.current_dir, buf, 16);
    console_puts(buf);
    console_puts(" -> 0x");
    itoa(target_id, buf, 16);
    console_puts(buf);
    console_puts("\n");

    fs.current_dir = target_id;

    rebuild_current_path();

    return FS_SUCCESS;
}

int fs_make_directory(const char* name) {
    console_puts("[DEBUG] fs_make_directory called for: ");
    console_puts(name);
    console_puts("\n");
    
    // check for valid name
    if (!name || strlen(name) == 0 || strlen(name) >= MAX_FILENAME) {
        console_puts("[DEBUG] Invalid directory name\n");
        return FS_ERROR_INVALID_NAME;
    }
    
    // check if directory already exists in current directory
    int existing = find_file_in_dir(fs.current_dir, name);
    if (existing >= 0) {
        console_puts("[DEBUG] Directory already exists with ID: ");
        char buf[16];
        itoa(existing, buf, 16);
        console_puts("0x");
        console_puts(buf);
        console_puts("\n");
        return FS_ERROR_ALREADY_EXISTS;
    }
    
    // check space in file table
    if (fs.file_count >= MAX_FILES) {
        console_puts("[DEBUG] No space left in file table\n");
        return FS_ERROR_NO_SPACE;
    }
    
    // check if next_file_id is valid
    if (fs.next_file_id >= MAX_FILES) {
        console_puts("[DEBUG] No more file IDs available\n");
        return FS_ERROR_NO_SPACE;
    }
    
    int new_id = fs.file_count; 

    console_puts("[DEBUG] Assigning new directory ID: ");
    char buf[16];
    itoa(new_id, buf, 10);
    console_puts(buf);
    console_puts(" (file_count=");
    itoa(fs.file_count, buf, 10);
    console_puts(buf);
    console_puts(")\n");
    
    // fill directory metadata
    file_entry_t* entry = &fs.files[new_id];
    entry->type = FILE_TYPE_DIRECTORY;
    entry->size = 0;
    entry->permissions = PERM_READ | PERM_WRITE | PERM_EXEC;
    entry->parent_id = fs.current_dir;
    entry->created_time = system_time++;
    entry->modified_time = system_time;
    entry->data_offset = 0;
    strcpy(entry->name, name);
    
    // update filesystem counters
    fs.file_count++;
    fs.next_file_id++;
    
    // print debug info
    console_puts("[DEBUG] Directory created successfully\n");
    console_puts("[DEBUG] New directory ID: 0x");
    itoa(new_id, buf, 16);
    console_puts(buf);
    console_puts(" (parent: 0x");
    itoa(fs.current_dir, buf, 16);
    console_puts(buf);
    console_puts(")\n");
    console_puts("[DEBUG] Updated file_count: ");
    itoa(fs.file_count, buf, 10);
    console_puts(buf);
    console_puts(", next_file_id: ");
    itoa(fs.next_file_id, buf, 10);
    console_puts(buf);
    console_puts("\n");
    
    return FS_SUCCESS;
}

int fs_delete_file(const char* name) {
    int file_id = find_file_in_dir(fs.current_dir, name);
    if (file_id < 0) return FS_ERROR_NOT_FOUND;

    uint32_t old_last_id = fs.file_count - 1;

    if (file_id < (int)old_last_id) {
        for (uint32_t i = 0; i < fs.file_count; i++) {
            if (fs.files[i].parent_id == old_last_id) {
                fs.files[i].parent_id = file_id;
            }
        }
        if (fs.current_dir == old_last_id) {
            fs.current_dir = file_id;
        }
        fs.files[file_id] = fs.files[old_last_id];
    }

    fs.file_count--;
    return FS_SUCCESS;
}

int fs_remove_directory(const char* name) {
    int dir_id = find_file_in_dir(fs.current_dir, name);
    if (dir_id < 0) return FS_ERROR_NOT_FOUND;
    if (fs.files[dir_id].type != FILE_TYPE_DIRECTORY) return FS_ERROR_NOT_DIRECTORY;

    for (uint32_t i = 0; i < fs.file_count; i++) {
        if (fs.files[i].parent_id == (uint32_t)dir_id) return FS_ERROR_NOT_EMPTY;
    }

    uint32_t old_last_id = fs.file_count - 1;
    if (dir_id < (int)old_last_id) {
        for (uint32_t i = 0; i < fs.file_count; i++) {
            if (fs.files[i].parent_id == old_last_id) {
                fs.files[i].parent_id = dir_id;
            }
        }
        if (fs.current_dir == old_last_id) {
            fs.current_dir = dir_id;
        }
        fs.files[dir_id] = fs.files[old_last_id];
    }

    fs.file_count--;
    return FS_SUCCESS;
}

int fs_get_current_path(char* buffer, uint32_t size) {
    if (!buffer || size < 1) return FS_ERROR_INVALID_PATH;

    size_t len = strlen(fs.current_path) + 1;
    if (len > size) return FS_ERROR_NO_SPACE;

    strcpy(buffer, fs.current_path);
    return FS_SUCCESS;
}



char* fs_get_current_path_ptr(void) {
    return fs.current_path;
}

file_entry_t* fs_get_file(const char* name) {
    int file_id = find_file_in_dir(fs.current_dir, name);
    if (file_id < 0) return NULL;
    return &fs.files[file_id];
}

int fs_copy_file(const char* src, const char* dest) {
    char buffer[MAX_FILE_SIZE];
    int bytes_read = fs_read_file(src, buffer, MAX_FILE_SIZE);
    if (bytes_read < 0) return bytes_read;
    return fs_write_file(dest, buffer, bytes_read);
}

int fs_move_file(const char* src, const char* dest) {
    file_entry_t* src_file = fs_get_file(src);
    if (!src_file) return FS_ERROR_NOT_FOUND;
    if (fs_get_file(dest)) return FS_ERROR_ALREADY_EXISTS;

    int ret = fs_create_file(dest, src_file->type);
    if (ret != FS_SUCCESS) return ret;

    uint8_t buffer[MAX_FILE_SIZE];
    uint32_t bytes_read = fs_read_file(src, buffer, src_file->size);
    if (bytes_read != src_file->size) {
        fs_delete_file(dest);
        return FS_ERROR_PERMISSION_DENIED;
    }

    if (fs_write_file(dest, buffer, src_file->size) != FS_SUCCESS) {
        fs_delete_file(dest);
        return FS_ERROR_PERMISSION_DENIED;
    }

    return fs_delete_file(src);
}

int fs_find_file(const char* pattern) {
    console_puts("Find results:\n");
    int found = 0;

    for (uint32_t i = 0; i < fs.file_count; i++) {
        if (strstr(fs.files[i].name, pattern)) {
            console_puts(fs.files[i].name);
            console_puts("\n");
            found++;
        }
    }

    if (!found) console_puts("No files found matching pattern.\n");
    return found;
}

int fs_grep_file(const char* filename, const char* pattern) {
    char buffer[MAX_FILE_SIZE];
    int bytes_read = fs_read_file(filename, buffer, MAX_FILE_SIZE - 1);
    if (bytes_read < 0) return bytes_read;

    buffer[bytes_read] = '\0';
    if (strstr(buffer, pattern)) {
        console_puts("Pattern found in ");
        console_puts(filename);
        console_puts("\n");
        return 1;
    }

    return 0;
}

int fs_touch_file(const char* name) {
    int file_id = find_file_in_dir(fs.current_dir, name);
    if (file_id >= 0) {
        fs.files[file_id].modified_time = system_time++;
        return FS_SUCCESS;
    }

    if (fs_create_file(name, FILE_TYPE_REGULAR) != FS_SUCCESS) return FS_ERROR_NO_SPACE;
    return fs_write_file(name, "", 0);
}

int fs_resolve_path(const char* path) {
    if (!path || path[0] == '\0') return -1;

    uint32_t dir = (path[0] == '/') ? fs.root_dir : fs.current_dir;
    char temp_path[MAX_PATH];
    strncpy(temp_path, path, MAX_PATH - 1);
    temp_path[MAX_PATH - 1] = '\0';

    char* token = strtok(temp_path, "/");
    while (token != NULL) {
        int next = find_file_in_dir(dir, token);
        if (next < 0 || fs.files[next].type != FILE_TYPE_DIRECTORY) {
            return -1;
        }
        dir = next;
        token = strtok(NULL, "/");
    }

    return (int)dir;
}


int fs_getcwd(char* buffer, uint32_t size) {
    if (!buffer || size < 1) return FS_ERROR_INVALID_PATH;

    // Special case: root directory
    if (fs.current_dir == fs.root_dir) {
        if (size < 2) return FS_ERROR_NO_SPACE;  
        strcpy(buffer, "/");
        return FS_SUCCESS;
    }

    char path_stack[32][MAX_FILENAME];
    int depth = 0;
    uint32_t current = fs.current_dir;

    while (current != fs.root_dir && depth < 32) {
        strncpy(path_stack[depth], fs.files[current].name, MAX_FILENAME);
        path_stack[depth][MAX_FILENAME - 1] = '\0';
        depth++;
        current = fs.files[current].parent_id;

        if (current == fs.current_dir) break;
    }

    buffer[0] = '\0';
    for (int i = depth - 1; i >= 0; i--) {
        strcat(buffer, "/");
        strcat(buffer, path_stack[i]);
    }

    if (strlen(buffer) + 1 > size) return FS_ERROR_NO_SPACE;

    return FS_SUCCESS;
}

uint32_t fs_get_current_dir_id(void) {
    return fs.current_dir;
}
