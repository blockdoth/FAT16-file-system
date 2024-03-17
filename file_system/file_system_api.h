#ifndef FILE_SYSTEM_FILE_SYSTEM_API_H
#define FILE_SYSTEM_FILE_SYSTEM_API_H
#include "volume/volume.h"


typedef enum FILESYSTEM_TYPE{
    FAT16
} FILESYSTEM_TYPE;


bool fs_format(RawVolume* raw_volume, FILESYSTEM_TYPE filesystem);
void fs_destroy();

bool fs_file_exists(char* path);
bool fs_create_file(char* path, void* file_data, uint32_t file_size);
uint32_t fs_update_file(char* path, void* data, uint32_t new_file_size);
void* fs_read_file(char* path);

bool fs_dir_exists(char* path);
bool fs_create_dir(char* path);

#endif //FILE_SYSTEM_FILE_SYSTEM_API_H
