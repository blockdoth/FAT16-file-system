#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "volume_management/volume.h"


typedef enum FILESYSTEM_TYPE{
    FAT16
} FILESYSTEM_TYPE;

//TODO Try to move this in FAT16.h
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

typedef struct system_file {
    char* name;
    char* path;
    uint32_t fileSize;
    uint8_t read_only : 1;
    uint8_t hidden : 1;
    uint8_t archive : 1;
} system_file_metadata;

typedef struct FormattedVolume {
    RawVolume* rawVolume;
    FATVolumeInfo* volumeInfo;
    bool (*writeFile)(struct FormattedVolume* self, FileMetadata* fileMetadata, void* fileData, char* path);
    bool (*writeDir)(struct FormattedVolume* self, FileMetadata* fileMetadata, char* path);
    bool (*findFile)(struct FormattedVolume* self, FileMetadata* fileMetadata);
    void* (*read)( struct FormattedVolume* self, FileMetadata* fileIdentifier, char* path);
} FormattedVolume;

typedef struct Path {
    char** path;
    uint8_t depth;
} Path;


typedef void* system_file_data;

bool format_volume(RawVolume* raw_volume, FILESYSTEM_TYPE filesystem);

bool fs_create_file(system_file_metadata* systemFile, void* file_data);
bool fs_create_dir(system_file_metadata* systemFile);
void* fs_read_file(system_file_metadata* systemFile);


uint16_t getCurrentTimeMs();
uint16_t getCurrentTime();
uint16_t getCurrentDate();

FileMetadata convertMetadata(system_file_metadata* systemFile);

// FILE api




#endif
