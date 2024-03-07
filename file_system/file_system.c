#include "../include/file_system.h"



Volume* format_volume(raw_volume* raw_volume, FILESYSTEM_TYPE filesystem){
    Volume* formatted_volume;
    switch (filesystem) {
        case FAT16:
            formatted_volume = format_FAT16_volume(raw_volume);
            break;
        default:
    }
    return formatted_volume;
}

Volume* format_FAT16_volume(raw_volume *raw_volume) {
    if(check_FAT16_formattible(raw_volume)){
        return nullptr;
    }

    Volume *formatted_volume = {};


    return formatted_volume;
}

bool check_FAT16_formattible(raw_volume *raw_volume) {
    return raw_volume->size < FAT16_MINIMUM_SIZE;
}

