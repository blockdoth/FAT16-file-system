#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "volume/volume.h"
#include "file_system_api.h"
#include "formats/FAT16/FAT16.h"

#include <string.h>
#include <malloc.h>


typedef struct FATVolumeInfo {
    sector_ptr FAT1Start;
    sector_ptr FAT2Start;
    sector_ptr dataSectionStart;
    sector_ptr rootSectionStart;
    uint32_t FATEntryCount;
    uint32_t FATTableSectorCount;
    uint32_t rootSectorCount;
    uint32_t totalSectorCount;
    uint64_t totalAddressableSize;
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t bytesPerCluster;
} FATVolumeInfo;

typedef struct Path {
    char** path;
    uint8_t depth;
    DriveID driveId;
} Path;


typedef union {
    FATVolumeInfo FAT16;
} VolumeInfo;

typedef struct {
    sector_ptr sectorPtr;
    uint16_t age;
    void* sector;
} FAT16CacheEntry;

typedef struct {
    FAT16CacheEntry* cache;
    uint32_t size;
    uint32_t cacheHits;
    uint32_t cacheMisses;
} FAT16Cache;

typedef union{
    FAT16Cache FAT16;
} Cache;

typedef struct FormattedVolume {
    RawVolume* rawVolume;
    VolumeInfo* info;
    Cache cache;
    FS_STATUS_CODE (*createFile)(struct FormattedVolume* self, Path* path, FileMetadata* fileMetadata, void* fileData);
    FS_STATUS_CODE (*createDir)(struct FormattedVolume* self, Path* path, FileMetadata* fileMetadata);
    void* (*readFile)(struct FormattedVolume* self, Path* path);
    void* (*readFileSection)(struct FormattedVolume* self, Path* path, uint32_t offset, uint32_t size);
    FS_STATUS_CODE (*checkFile)(struct FormattedVolume* self, Path* path);
    FS_STATUS_CODE (*checkDir)(struct FormattedVolume* self, Path* path);
    uint32_t (*updateFile)(struct FormattedVolume* self, Path* path, void* fileData, uint32_t dataSize, uint32_t offset);
    uint32_t (*expandFile)(struct FormattedVolume* self, Path* path, void* fileData, uint32_t dataSize);
    FS_STATUS_CODE (*deleteDir)(struct FormattedVolume *self, Path* path);
    FS_STATUS_CODE (*deleteFile)(struct FormattedVolume* self, Path* path);
    FS_STATUS_CODE (*isDir)(struct FormattedVolume* self, Path* path);
    char* (*toString)(struct FormattedVolume* self);
    FileMetadata* (*getMetadata)(struct FormattedVolume* self, Path* path);
    FS_STATUS_CODE (*renameFile)(struct FormattedVolume* self, Path* path, char* newName);
    FS_STATUS_CODE (*destroy)(struct FormattedVolume* self);
} FormattedVolume;


uint16_t getCurrentTimeMs();
uint16_t getCurrentTime();
uint16_t getCurrentDate();

FileMetadata* initFile(char* path, uint32_t file_size);
void destroyMetaData(FileMetadata* metadata);

char* extractName(char* path);
FS_STATUS_CODE checkValidPath(char* path);
Path* parsePath(char* path);
void destroyPath(Path* path);
DriveID parseDriveId(char* id);
// FILE api

FormattedVolume *formatFAT16Volume(RawVolume *volume, FAT16Config fat16Config);
FS_STATUS_CODE FAT16WriteFile(FormattedVolume* self, Path* path, FileMetadata* fileMetadata, void* fileData);
FS_STATUS_CODE FAT16WriteDir(FormattedVolume* self, Path* path, FileMetadata* fileMetadata);
void* FAT16ReadFile(FormattedVolume* self, Path* path);
void* FAT16ReadFileSection(FormattedVolume* self, Path* path, uint32_t offset, uint32_t chunkSize);
FS_STATUS_CODE FAT16CheckFile(FormattedVolume* self, Path* path);
FS_STATUS_CODE FAT16CheckDir(FormattedVolume* self, Path* path);
uint32_t FAT16UpdateFile(FormattedVolume* self, Path* path, void* newData, uint32_t updatedDataSize, uint32_t offset);
uint32_t FAT16ExpandFile(FormattedVolume* self, Path* path, void* newData, uint32_t newDataSize);

FS_STATUS_CODE FAT16DeleteFile(FormattedVolume* self, Path* path);
FS_STATUS_CODE FAT16DeleteDir(FormattedVolume *self, Path* path);
FileMetadata* FAT16GetMetadata(FormattedVolume *self, Path* path);
FS_STATUS_CODE FAT16Rename(FormattedVolume *self, Path* path, char* newName);
char* FAT16ToTreeString(FormattedVolume* self);
FS_STATUS_CODE FAT16IsDir(FormattedVolume* self, Path* path);
FS_STATUS_CODE FAT16Destroy(FormattedVolume* self);



#endif
