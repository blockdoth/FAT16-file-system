#ifndef FILE_SYSTEM_FILE_API_H
#define FILE_SYSTEM_FILE_API_H

#include "../common_types.h"

#endif //FILE_SYSTEM_FILE_API_H

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

typedef struct {
    bool free;
    DFILE* stat;
    PERMS perms;
    char* path;
    void* file;
} FiletableEntry;

#define FILETABLE_SIZE 10

static FiletableEntry* fileTable[FILETABLE_SIZE];


FD dfopen(char* path, PERMS perms);
void dfclose(FD fd);
void dfflush(FD fd);

void dfrawread(FD fd, void* buffer, uint32_t size, uint32_t offset); //
char dfgetc(FD fd);
char* dfgetstr(FD fd, uint32_t n); // Gets a string of length n or until '\0' when n = -1

void dfrawwrite(FD fd, void* src, uint32_t size, uint32_t offset);
void dfprintf(FD fd, char* str, uint32_t size);

void initFileTable();
void destroyFileTable();
FD addFileTableEntry(void* file, char* path);