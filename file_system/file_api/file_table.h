#ifndef FILE_SYSTEM_FILE_TABLE_H
#define FILE_SYSTEM_FILE_TABLE_H
#include "file_api.h"

typedef struct {
    bool free;
    uint16_t bufferedWrites;
    DFILE* stat;
    PERMS perms;
    char* path;
    void* file;
} FiletableEntry;

#define FILETABLE_SIZE 10
#define MAX_BUFFERED_WRITES 5
static FiletableEntry* fileTable[FILETABLE_SIZE];

// File table management
void initFileTable();
void destroyFileTable();
FD addFileTableEntry(void* file, char* path);

void writeFile(FiletableEntry* entry);
void updateDiffRange(FiletableEntry* entry, uint32_t leftRange, uint32_t rightRange);
void resizeFile(FiletableEntry* entry, uint32_t newSize);

#endif //FILE_SYSTEM_FILE_TABLE_H
