#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "volume/volume.h"
#include "file_system_api.h"
#include "formats/FAT16/FAT16.h"

#include <string.h>
#include <malloc.h>

typedef struct FileMetadata {
    char* name;
    uint8_t read_only : 1;
    uint8_t hidden : 1;
    uint8_t system : 1;
    uint8_t volume_id : 1;
    uint8_t directory : 1;
    uint8_t archive : 1;
    uint8_t long_name : 1;
    uint8_t creationTimeTenth;
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t lastAccessedDate;
    uint16_t timeOfLastWrite;
    uint16_t dateOfLastWrite;
    uint32_t fileSize;
} FileMetadata;

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

typedef struct FormattedVolume {
    RawVolume* rawVolume;
    VolumeInfo* info;
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
    bool (*isDir)(struct FormattedVolume* self, Path* path);
    char* (*toString)(struct FormattedVolume* self);
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
char* FAT16ToTreeString(FormattedVolume* self);
bool FAT16IsDir(FormattedVolume* self, Path* path);
FS_STATUS_CODE FAT16Destroy(FormattedVolume* self);



#endif
