#ifndef FILE_SYSTEM_VIRTUAL_FILES_SYSTEM_H
#define FILE_SYSTEM_VIRTUAL_FILES_SYSTEM_H
#include "stdint.h"
#include "common_types.h"
#include "malloc.h"
#include "string.h"

typedef enum FILESYSTEM_TYPE{
    FAT16
} FILESYSTEM_TYPE;

typedef struct Path {
    char** path;
    uint8_t depth;
    DriveID driveId;
} Path;

typedef struct {
    char* name;
    bool read_only;
    bool hidden;
    bool stale;
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t lastAccessedDate;
    uint16_t timeOfLastWrite;
    uint16_t dateOfLastWrite;
    uint32_t fileSize;
    struct VirtualDir* parentDir;
} VirtualFile;

typedef struct {
    char* name;
    bool read_only;
    bool hidden;
    bool stale;
    uint32_t creationTime;
    uint16_t creationDate;
    uint16_t lastAccessedDate;
    uint16_t timeOfLastWrite;
    uint16_t dateOfLastWrite;
    FILESYSTEM_TYPE filesystem;
    struct VirtualDir* parentDir;
    struct VirtualDir** childDirs;
    VirtualFile** childFiles;
} VirtualDir;

FS_STATUS_CODE checkValidPath(char* path);
Path parsePath(char* path);
void destroyPath(Path path);

#endif //FILE_SYSTEM_VIRTUAL_FILES_SYSTEM_H
