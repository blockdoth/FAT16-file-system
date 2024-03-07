#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "volume.h"

#define FAT16_MINIMUM_SIZE 1024

typedef enum FILESYSTEM_TYPE{
    FAT16
} FILESYSTEM_TYPE;


typedef struct Volume {

} Volume;

Volume* format_volume(raw_volume* raw_volume, FILESYSTEM_TYPE filesystem);
Volume* format_FAT16_volume(raw_volume *pVolume);

bool check_FAT16_formattible(raw_volume *raw_volume);


#endif