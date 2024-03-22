#include "file_system.h"

FormattedVolume* drives[maxDriveCount];

FS_STATUS_CODE fs_format(RawVolume *raw_volume, FILESYSTEM_TYPE filesystem, DriveID driveID) {
    switch (filesystem) {
        case FAT16:
            drives[driveID] =  formatFAT16Volume(raw_volume);
            return FS_SUCCES;
        default:
    }
    return FS_FORMAT_NOT_SUPPORTED;
}

void fs_destroy(DriveID driveID) {
    FormattedVolume* currentDrive = drives[driveID];
    currentDrive->rawVolume->destroy(currentDrive->rawVolume);
    free(currentDrive->info);
    free(currentDrive);
}


FS_STATUS_CODE fs_create_file(char* path, void* file_data, uint32_t file_size){
    if(!checkValidPath(path)){
        return FS_INVALID_PATH;
    }
    Path resolvedPath = parsePath(path);
    FileMetadata fileMetadata = initFile(path, file_size);
    FormattedVolume* currentDrive = drives[resolvedPath.driveId];
    return currentDrive->createFile(currentDrive, resolvedPath, &fileMetadata, file_data);
}


FS_STATUS_CODE fs_create_dir(char* path){
    if(!checkValidPath(path)){
        return FS_INVALID_PATH;
    }
    Path resolvedPath = parsePath(path);
    FileMetadata fileMetadata = initFile(path, 0);
    fileMetadata.directory = 1;
    FormattedVolume* currentDrive = drives[resolvedPath.driveId];
    return currentDrive->createDir(currentDrive, resolvedPath,  &fileMetadata);
}

void* fs_read_file(char* path){
    if(!checkValidPath(path)){
        return NULL;
    }
    Path resolvedPath = parsePath(path);
    FormattedVolume* currentDrive = drives[resolvedPath.driveId];
    return currentDrive->readFile(currentDrive, resolvedPath);
}

void* fs_read_file_section(char* path, uint32_t offset, uint32_t size){
    if(!checkValidPath(path)){
        return NULL;
    }
    Path resolvedPath = parsePath(path);
    FormattedVolume* currentDrive = drives[resolvedPath.driveId];
    return currentDrive->readFileSection(currentDrive, resolvedPath, offset, size);
}

FS_STATUS_CODE fs_file_exists(char* path){
    if(!checkValidPath(path)){
        return FS_INVALID_PATH;
    }
    Path resolvedPath = parsePath(path);
    FormattedVolume* currentDrive = drives[resolvedPath.driveId];
    return currentDrive->checkFile(currentDrive,resolvedPath);
}
FS_STATUS_CODE fs_dir_exists(char* path){
    if(!checkValidPath(path)){
        return FS_INVALID_PATH;
    }
    Path resolvedPath = parsePath(path);
    FormattedVolume* currentDrive = drives[resolvedPath.driveId];
    return currentDrive->checkDir(currentDrive,resolvedPath);
}
uint32_t fs_update_file(char* path, void* data, uint32_t new_file_size){
    if(!checkValidPath(path)){
        return FS_INVALID_PATH;
    }
    Path resolvedPath = parsePath(path);
    FormattedVolume* currentDrive = drives[resolvedPath.driveId];
    return currentDrive->updateFile(currentDrive, resolvedPath, data, new_file_size);
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

FS_STATUS_CODE fs_delete_dir(char* path, bool recursive){
    if(!checkValidPath(path)){
        return FS_INVALID_PATH;
    }
    Path resolvedPath = parsePath(path);
    FormattedVolume* currentDrive = drives[resolvedPath.driveId];
    return currentDrive->deleteDir(currentDrive, resolvedPath);
}

FS_STATUS_CODE fs_delete_file(char* path){
    if(!checkValidPath(path)){
        return FS_INVALID_PATH;
    }
    Path resolvedPath = parsePath(path);
    FormattedVolume* currentDrive = drives[resolvedPath.driveId];
    return currentDrive->deleteFile(currentDrive, resolvedPath);
}


FS_STATUS_CODE checkValidPath(char* path){
    if(path[0] != '#' || path[0] == '\0'){
        return FS_INVALID_PATH;
    }
    path++;
    switch (*path) {
        case 'D':
        case 'R':
        case 'B':
            return FS_SUCCES;
        default:
            return FS_INVALID_PATH;
    }
}

char* fs_get_string(char* path){
    Path resolvedPath = parsePath(path);
    FormattedVolume* currentDrive = drives[resolvedPath.driveId];
    return currentDrive->toString(currentDrive,resolvedPath);
}

Path parsePath(char* path){
//    if(*path == '\0'){
//        char** resolvedPath = (char**) malloc(sizeof(char *));
//        *resolvedPath = "";
//        return (Path) {resolvedPath, 0};
//    }
    path++; // Skip '#'
    DriveID driveId;
    switch (*path) {
        case 'D':
            driveId = DRIVE_D;
            break;
        case 'R':
            driveId = DRIVE_R;
            break;
        case 'B':
            driveId = DRIVE_B;
            break;
        default:
            // Shouldnt ever be reached
            driveId = INVALID_DRIVE_ID;
    }
    path+=2;
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

    return (Path){resolvedPath,depth, driveId};
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

