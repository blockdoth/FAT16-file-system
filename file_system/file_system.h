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

typedef struct FileIdentifier {
    char* identifier;
}FileIdentifier;
typedef struct FATVolumeInfo {
    volume_ptr FAT1Start;
    volume_ptr FAT2Start;
    volume_ptr dataSectionStart;
    volume_ptr rootSectionStart;
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
} Path;

typedef struct FormattedVolume {
    RawVolume* rawVolume;
    FATVolumeInfo* volumeInfo;
    bool (*createFile)(struct FormattedVolume* self, Path path, FileMetadata* fileMetadata, void* fileData);
    bool (*createDir)(struct FormattedVolume* self, Path path, FileMetadata* fileMetadata);
    void* (*readFile)(struct FormattedVolume* self, Path path);
    bool (*checkFile)(struct FormattedVolume* self, Path path);
    bool (*checkDir)(struct FormattedVolume* self, Path path);
    uint32_t (*updateFile)(struct FormattedVolume* self, Path path, void* fileData, uint32_t dataSize);
} FormattedVolume;




uint16_t getCurrentTimeMs();
uint16_t getCurrentTime();
uint16_t getCurrentDate();

FileMetadata initFile(char* path, uint32_t file_size);
char* extractName(char* path);
Path parsePath(char* path);
void destroyPath(Path path);
// FILE api

FormattedVolume* formatFAT16Volume(RawVolume *volume);
bool FAT16WriteFile(FormattedVolume* self, Path path, FileMetadata* fileMetadata, void* fileData);
bool FAT16WriteDir(FormattedVolume* self, Path path, FileMetadata* fileMetadata);
void* FAT16ReadFile(FormattedVolume* self, Path path);
bool FAT16CheckFile(FormattedVolume* self, Path path);
bool FAT16CheckDir(FormattedVolume* self, Path path);
uint32_t FAT16UpdateFile(FormattedVolume* self, Path path, void* fileData, uint32_t dataSize);

// Supported operations

// │ Operation │  Files  │ Directories  │
// ├───────────┼─────────┼──────────────┤
// │ Finding   │   [ ]   │     [ ]      │
// │ Reading   │   [X]   │     [X]      │
// │ Writing   │   [X]   │     [X]      │
// │ Updating  │   [ ]   │     [ ]      │




#endif
