#include "file_system.h"


FormattedVolume* formatted_volume;

bool fs_format(RawVolume* raw_volume, FILESYSTEM_TYPE filesystem){
    switch (filesystem) {
        case FAT16:
            formatted_volume = formatFAT16Volume(raw_volume);
            return true;
        default:
    }
    return false;
}

void fs_destroy() {
    formatted_volume->rawVolume->destroy(formatted_volume->rawVolume);
    free(formatted_volume->volumeInfo);
    free(formatted_volume);
}


bool fs_create_file(char* path, void* file_data, uint32_t file_size){
    Path resolvedPath = parsePath(path);
    FileMetadata fileMetadata = initFile(path, file_size);
    return formatted_volume->createFile(formatted_volume, resolvedPath, &fileMetadata, file_data);
}


bool fs_create_dir(char* path){
    Path resolvedPath = parsePath(path);
    FileMetadata fileMetadata = initFile(path, 0);
    fileMetadata.directory = 1;
    return formatted_volume->createDir(formatted_volume, resolvedPath,  &fileMetadata);
}

void* fs_read_file(char* path){
    Path resolvedPath = parsePath(path);
    return formatted_volume->readFile(formatted_volume, resolvedPath);
}

bool fs_file_exists(char* path){
    Path resolvedPath = parsePath(path);
    return formatted_volume->checkFile(formatted_volume,resolvedPath);
}
bool fs_dir_exists(char* path){
    Path resolvedPath = parsePath(path);
    return formatted_volume->checkDir(formatted_volume,resolvedPath);
}
uint32_t fs_update_file(char* path, void* data, uint32_t new_file_size){
    Path resolvedPath = parsePath(path);
    return formatted_volume->updateFile(formatted_volume, resolvedPath, data, new_file_size);
}


FileMetadata initFile(char* path, uint32_t file_size){
    char* name = extractName(path);
    FileMetadata fileMetadata = {
            name,
            0,
            0,
            0,
            0,
            0,
            0,
            strlen(name) > 10 ? 1: 0,
            getCurrentTimeMs(),
            getCurrentTime(),
            getCurrentDate(),
            getCurrentDate(),
            getCurrentTime(),
            getCurrentDate(),
            file_size,
    };

    return fileMetadata;
}


Path parsePath(char* path){
//    if(*path == '\0'){
//        char** resolvedPath = (char**) malloc(sizeof(char *));
//        *resolvedPath = "";
//        return (Path) {resolvedPath, 0};
//    }
    path++; // Skip '#'
    char* tempPath = path;
    uint16_t depth = 0;
    while(*tempPath != '\0'){
        if(*tempPath++ == '|'){
            depth++;
        }
    }
    char** resolvedPath = (char**) malloc((depth + 1) * sizeof(char*));

    int i = 0;
    tempPath = path;
    char* start = path;
    // Parsing the path and extracting substrings separated by '|'
    while (*tempPath++ != '\0') {
        if (*tempPath == '|' || *tempPath == '\0') {
            uint32_t strlen = tempPath - start;
            resolvedPath[i] = (char*)malloc((strlen + 1) * sizeof(char));
            strncpy(resolvedPath[i], start, strlen);
            resolvedPath[i][strlen] = '\0'; // Null-terminate the string
            start = tempPath + 1;
            i++;
        }
        //tempPath++;
    }
    return (Path){
            resolvedPath,
            depth
    };
}

void destroyPath(Path path){
//    for (int i = 0; i <= path.depth; ++i) {
//        free(path.path[i]);
//    }
}

// Figure this one out yourself nerds

char* extractName(char* path){
    while (*++path != '\0') {}
    char* endFileName = path;
    while(*(--path - 1) != '#' && *(path - 1) != '|'){}
    char* fileName = (char*)malloc((endFileName - path) * sizeof(char*));
    strcpy(fileName, path);
    return fileName;
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

