#include <string.h>
#include <malloc.h>
#include "file_system.h"
#include "file_system_implementations/FAT16.h"





// Supported operations

// │ Operation │  Files  │ Directories  │
// ├───────────┼─────────┼──────────────┤
// │ Finding   │   [    ]   │     [ ]      │
// │ Reading   │   [X]   │     [ ]      │
// │ Writing   │   [X]   │     [ ]      │
// │ Updating  │   [ ]   │     [ ]      │


// UNIX based API

// Files

// Directories




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
    FileMetadata fileMetadata = convertMetadata(systemFile);
    return formatted_volume->writeFile(formatted_volume, &fileMetadata, file_data);
}
void* fs_read_file(system_file_metadata* systemFile){
    FileMetadata fileMetadata = convertMetadata(systemFile);
    return formatted_volume->read(formatted_volume, &fileMetadata);
}


bool fs_create_dir(system_file_metadata* systemFile){
    FileMetadata fileMetadata = convertMetadata(systemFile);
    fileMetadata.directory = 1;

    return formatted_volume->writeDir(formatted_volume, &fileMetadata, systemFile->path);
}


FileMetadata convertMetadata(system_file_metadata* systemFile){
    return (FileMetadata){
            systemFile->name,
            systemFile->read_only,
            systemFile->hidden,
            0,
            0,
            0,
            systemFile->archive,
            strlen(systemFile->name) > 10 ? 1: 0,
            getCurrentTimeMs(),
            getCurrentTime(),
            getCurrentDate(),
            getCurrentDate(),
            getCurrentTime(),
            getCurrentDate(),
            systemFile->fileSize,
    };
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

