#ifndef FILE_SYSTEM_API_H
#define FILE_SYSTEM_API_H
#include "volume/volume.h"

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


typedef enum FILESYSTEM_TYPE{
    FAT16
} FILESYSTEM_TYPE;

typedef struct {
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
} FAT16Config;

typedef union {
    FAT16Config fat16Config;
} FormatConfig;

typedef struct {
    FILESYSTEM_TYPE filesystemType;
    FormatConfig formatConfig;
} FormatSpecifier;


// === Filesystem lifecycle ===
FS_STATUS_CODE fs_format(RawVolume* raw_volume, FormatSpecifier formatSpecifier, DriveID drive_id);
void fs_destroy(DriveID drive_id);

// === Files ===
FS_STATUS_CODE fs_create_file(char* path, void* data, uint32_t size);
void* fs_read_file(char* path);
void* fs_read_file_section(char* path, uint32_t offset, uint32_t size);
uint32_t fs_expand_file(char* path, void* new_data, uint32_t new_size);
uint32_t fs_update_file(char* path, void* new_data, uint32_t new_size, uint32_t offset);
FS_STATUS_CODE fs_delete_file(char* path);
FS_STATUS_CODE fs_file_exists(char* path);

// === Dirs ===
FS_STATUS_CODE fs_create_dir(char* path);
FS_STATUS_CODE fs_delete_dir(char* path);
FS_STATUS_CODE fs_dir_exists(char* path);
FS_STATUS_CODE fs_is_dir(char* path);

// === Metadata ===
FileMetadata* fs_get_metadata(char* path);
FS_STATUS_CODE fs_rename(char* path, char* new_name);

// === Misc ===
char* fs_tree(char* path);

#endif //FILE_SYSTEM_API_H
