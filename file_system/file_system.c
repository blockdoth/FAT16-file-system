#include "file_system.h"



FormattedVolume * format_volume(RawVolume* raw_volume, FILESYSTEM_TYPE filesystem){
    FormattedVolume* formatted_volume;
    switch (filesystem) {
        case FAT16:
            formatted_volume = format_FAT16_volume(raw_volume);
            break;
        default:
    }
    return formatted_volume;
}




