#ifndef FILE_SYSTEM_FILE_API_H
#define FILE_SYSTEM_FILE_API_H

#include "../common_types.h"
#include "../file_system_api/file_system_api.h"

typedef enum {
    STDIN,
    STDOUT,
    STDERR,
} FD_STREAM;

typedef enum {
    READ,
    WRITE, //TODO expand
    NOTHING
} PERMS;

typedef enum {
    UINT16,
    FLOAT,
} NUMFORMAT;

typedef struct {
    uint32_t file_descriptor;
    uint32_t pos;
    uint32_t size;
    uint32_t leftDiffRange;
    uint32_t rightDiffRange;
} DFILE;

typedef uint16_t FD;

// File state management
FD dfopen(char* path, PERMS perms);
void dfclose(FD fd);
void dfflush(FD fd);

// Reading data from a file
void dfrawread(FD fd, void* buffer, uint32_t size, uint32_t offset); //
char dfgetc(FD fd);
char* dfgetstr(FD fd, uint32_t length); // Gets a string of a stated length or until '\0' when length = -1

// Writing data to a file
void dfrawwrite(FD fd, void* src, uint32_t size, uint32_t offset);
void dfcatc(FD fd, char c);
void dfcatstr(FD fd, char* str, uint32_t length);

// Should print to stdout //TODO
void dfprintf(char* str, uint32_t size);
#endif //FILE_SYSTEM_FILE_API_H

