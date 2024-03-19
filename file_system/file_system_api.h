#ifndef FILE_SYSTEM_FILE_SYSTEM_API_H
#define FILE_SYSTEM_FILE_SYSTEM_API_H
#include "volume/volume.h"


typedef enum FILESYSTEM_TYPE{
    FAT16
} FILESYSTEM_TYPE;


FS_STATUS_CODE fs_format(RawVolume* raw_volume, FILESYSTEM_TYPE filesystem);
void fs_destroy();

FS_STATUS_CODE fs_file_exists(char* path);
FS_STATUS_CODE fs_create_file(char* path, void* file_data, uint32_t file_size);
uint32_t fs_update_file(char* path, void* data, uint32_t new_file_size);
void* fs_read_file(char* path);
void* fs_read_file_section(char* path, uint32_t offset, uint32_t size);
FS_STATUS_CODE fs_delete_file(char* path);
FS_STATUS_CODE fs_dir_exists(char* path);
char* fs_get_string(char* path);
FS_STATUS_CODE fs_create_dir(char* path);
FS_STATUS_CODE fs_delete_dir(char* path, bool recursive);

#endif //FILE_SYSTEM_FILE_SYSTEM_API_H
