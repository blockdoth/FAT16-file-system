#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "volume_management/volume.h"


typedef enum FILESYSTEM_TYPE{
    FAT16
} FILESYSTEM_TYPE;


typedef struct FormattedVolume {
    RawVolume* volume;
    bool (*write)(struct FormattedVolume* self, void* src_addr, volume_ptr dest_addr, uint32_t data_size);
    void* (*read)(struct FormattedVolume* self, volume_ptr src_addr, uint32_t data_size);
} FormattedVolume;

FormattedVolume* format_volume(RawVolume* raw_volume, FILESYSTEM_TYPE filesystem);
FormattedVolume* formatFAT16Volume(RawVolume *volume);

bool check_FAT16_formattible(RawVolume *raw_volume);


#endif