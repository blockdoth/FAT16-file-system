#include "file_system.h"
#include "file_system_implementations/FAT16.h"
#include "stdio.h"

FormattedVolume* formatted_volume;

bool format_volume(RawVolume* raw_volume, FILESYSTEM_TYPE filesystem){
    switch (filesystem) {
        case FAT16:
            formatted_volume = formatFAT16Volume(raw_volume);
            return true;
        default:
    }
    return false;
}


bool fs_create_file(system_file_metadata* systemFile, void* file_data){
    FileMetadata fileMetadata= {
            systemFile->name,
            systemFile->read_only,
            systemFile->hidden,
            systemFile->system,
            systemFile->volume_id,
            systemFile->directory,
            systemFile->archive,
            systemFile->long_name,
            getCurrentTimeMs(),
            getCurrentTime(),
            getCurrentDate(),
            getCurrentDate(),
            getCurrentTime(),
            getCurrentDate(),
            systemFile->fileSize,
    };
    formatted_volume->write(formatted_volume, &fileMetadata, file_data);
}
bool fs_read_file(system_file_metadata* systemFile){
    FileMetadata fileMetadata= {
            systemFile->name,
            systemFile->read_only,
            systemFile->hidden,
            systemFile->system,
            systemFile->volume_id,
            systemFile->directory,
            systemFile->archive,
            systemFile->long_name,
            getCurrentTimeMs(),
            getCurrentTime(),
            getCurrentDate(),
            getCurrentDate(),
            getCurrentTime(),
            getCurrentDate(),
            systemFile->fileSize,
    };
    return formatted_volume->read(formatted_volume, &fileMetadata);
}


uint16_t getCurrentTimeMs(){
    return 0; //TODO
}

uint16_t getCurrentTime(){
    return 0; //TODO
}

uint16_t getCurrentDate(){
    return 0; //TODO
}

