#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "volume_management/volume.h"


typedef enum FILESYSTEM_TYPE{
    FAT16
} FILESYSTEM_TYPE;

//TODO Try to move this in FAT16.h
typedef struct FATVolumeInfo {
    volume_ptr FAT1Address;
    volume_ptr FAT2Address;
    uint32_t FATClusters;
    volume_ptr rootSectorAddress;
    uint32_t rootSectorCount;
    volume_ptr dataSectorAddress;
    uint64_t totalAddressableSize;
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;

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
    uint8_t read_only : 1;
    uint8_t hidden : 1;
    uint8_t system : 1;
    uint8_t volume_id : 1;
    uint8_t directory : 1;
    uint8_t archive : 1;
    uint8_t long_name : 1;
    uint32_t fileSize;
} system_file;

typedef struct FormattedVolume {
    RawVolume* rawVolume;
    FATVolumeInfo* volumeInfo;
    bool (*write)( struct FormattedVolume* self, FileMetadata* fileMetadata);
    void* (*read)( struct FormattedVolume* self, FileIdentifier* fileIdentifier);
} FormattedVolume;

bool format_volume(RawVolume* raw_volume, FILESYSTEM_TYPE filesystem);

bool fs_create_file(system_file* systemFile);


uint16_t getCurrentTimeMs();
uint16_t getCurrentTime();
uint16_t getCurrentDate();
#endif
