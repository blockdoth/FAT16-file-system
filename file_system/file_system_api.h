#ifndef FILE_SYSTEM_API_H
#define FILE_SYSTEM_API_H
#include "volume/volume.h"


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


FS_STATUS_CODE fs_format(RawVolume *raw_volume, FormatSpecifier formatSpecifier, DriveID driveID);
void fs_destroy(DriveID driveID);

FS_STATUS_CODE fs_file_exists(char* path);
FS_STATUS_CODE fs_create_file(char* path, void* file_data, uint32_t file_size);
uint32_t fs_update_file(char* path, void* data, uint32_t new_file_size, uint32_t offset);
uint32_t fs_expand_file(char* path, void* newData, uint32_t newDataSize);
void* fs_read_file(char* path);
void* fs_read_file_section(char* path, uint32_t offset, uint32_t size);
FS_STATUS_CODE fs_delete_file(char* path);
FS_STATUS_CODE fs_dir_exists(char* path);
char* fs_get_string(char* path);
FS_STATUS_CODE fs_create_dir(char* path);
FS_STATUS_CODE fs_delete_dir(char* path, bool recursive);

#endif //FILE_SYSTEM_API_H
