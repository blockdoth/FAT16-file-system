#include "file_system.h"

FormattedVolume* drives[maxDriveCount];

FS_STATUS_CODE fs_format(RawVolume *raw_volume, FormatSpecifier formatSpecifier, DriveID driveID) {
    switch (formatSpecifier.filesystemType) {
        case FAT16:
            drives[driveID] = formatFAT16Volume(raw_volume, formatSpecifier.formatConfig.fat16Config);
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
    Path* resolvedPath = parsePath(path);
    FileMetadata* fileMetadata = initFile(path, file_size);
    FormattedVolume* currentDrive = drives[resolvedPath->driveId];
    FS_STATUS_CODE statusCode = currentDrive->createFile(currentDrive, resolvedPath, fileMetadata, file_data);
    destroyPath(resolvedPath);
    destroyMetaData(fileMetadata);
    return statusCode;
}


FS_STATUS_CODE fs_create_dir(char* path){
    if(!checkValidPath(path)){
        return FS_INVALID_PATH;
    }
    Path* resolvedPath = parsePath(path);
    FileMetadata* fileMetadata = initFile(path, 0);
    fileMetadata->directory = 1;
    FormattedVolume* currentDrive = drives[resolvedPath->driveId];
    FS_STATUS_CODE statusCode = currentDrive->createDir(currentDrive, resolvedPath,  fileMetadata);
    destroyPath(resolvedPath);
    destroyMetaData(fileMetadata);
    return statusCode;
}

void* fs_read_file(char* path){
    if(!checkValidPath(path)){
        return NULL;
    }
    Path* resolvedPath = parsePath(path);
    FormattedVolume* currentDrive = drives[resolvedPath->driveId];
    void* file = currentDrive->readFile(currentDrive, resolvedPath);
    destroyPath(resolvedPath);
    return file;
}

void* fs_read_file_section(char* path, uint32_t offset, uint32_t size){
    if(!checkValidPath(path)){
        return NULL;
    }
    Path* resolvedPath = parsePath(path);
    FormattedVolume* currentDrive = drives[resolvedPath->driveId];
    void* file = currentDrive->readFileSection(currentDrive, resolvedPath, offset, size);
    destroyPath(resolvedPath);
    return file;
}

FS_STATUS_CODE fs_file_exists(char* path){
    if(!checkValidPath(path)){
        return FS_INVALID_PATH;
    }
    Path* resolvedPath = parsePath(path);
    FormattedVolume* currentDrive = drives[resolvedPath->driveId];
    FS_STATUS_CODE statusCode = currentDrive->checkFile(currentDrive,resolvedPath);
    destroyPath(resolvedPath);
    return statusCode;
}
FS_STATUS_CODE fs_dir_exists(char* path){
    if(!checkValidPath(path)){
        return FS_INVALID_PATH;
    }
    Path* resolvedPath = parsePath(path);
    FormattedVolume* currentDrive = drives[resolvedPath->driveId];
    FS_STATUS_CODE statusCode = currentDrive->checkDir(currentDrive,resolvedPath);
    destroyPath(resolvedPath);
    return statusCode;
}

uint32_t fs_update_file(char* path, void* data, uint32_t new_file_size, uint32_t offset){
    if(!checkValidPath(path)){
        return FS_INVALID_PATH;
    }
    Path* resolvedPath = parsePath(path);
    FormattedVolume* currentDrive = drives[resolvedPath->driveId];
    uint32_t newFileSize = currentDrive->updateFile(currentDrive, resolvedPath, data, new_file_size, offset);
    destroyPath(resolvedPath);
    return newFileSize;
}

uint32_t fs_expand_file(char* path, void* newData, uint32_t newDataSize){
    if(!checkValidPath(path)){
        return FS_INVALID_PATH;
    }
    Path* resolvedPath = parsePath(path);
    FormattedVolume* currentDrive = drives[resolvedPath->driveId];
    uint32_t newFileSize = currentDrive->expandFile(currentDrive, resolvedPath, newData, newDataSize);
    destroyPath(resolvedPath);
    return newFileSize;
}


FileMetadata* initFile(char* path, uint32_t file_size){
    char* name = extractName(path);
    FileMetadata* fileMetadata = (FileMetadata*) malloc(sizeof(FileMetadata));
    fileMetadata->name = name;
    fileMetadata->fileSize = file_size;
    fileMetadata->read_only = 0;
    fileMetadata->hidden = 0;
    fileMetadata->system = 0;
    fileMetadata->volume_id = 0;
    fileMetadata->directory = 0;
    fileMetadata->archive = 0;
    fileMetadata->long_name = strlen(name) > 10 ? 1: 0;
    fileMetadata->creationTimeTenth = getCurrentTimeMs();
    fileMetadata->creationTime = getCurrentTime();
    fileMetadata->creationDate = getCurrentDate();
    fileMetadata->lastAccessedDate = getCurrentDate();
    fileMetadata->timeOfLastWrite = getCurrentTime();
    fileMetadata->dateOfLastWrite = getCurrentDate();
    return fileMetadata;
}

FS_STATUS_CODE fs_delete_dir(char* path, bool recursive){
    if(!checkValidPath(path)){
        return FS_INVALID_PATH;
    }
    Path* resolvedPath = parsePath(path);
    FormattedVolume* currentDrive = drives[resolvedPath->driveId];
    FS_STATUS_CODE statusCode = currentDrive->deleteDir(currentDrive, resolvedPath);
    destroyPath(resolvedPath);
    return statusCode;
}

FS_STATUS_CODE fs_delete_file(char* path){
    if(!checkValidPath(path)){
        return FS_INVALID_PATH;
    }
    Path* resolvedPath = parsePath(path);
    FormattedVolume* currentDrive = drives[resolvedPath->driveId];
    FS_STATUS_CODE statusCode = currentDrive->deleteFile(currentDrive, resolvedPath);
    destroyPath(resolvedPath);
    return statusCode;
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
    if(!checkValidPath(path)){
        return "";
    }
    DriveID driveId = parseDriveId(++path);
    FormattedVolume* currentDrive = drives[driveId];
    char* string = currentDrive->toString(currentDrive);
    return string;
}

DriveID parseDriveId(char* id){
    switch (*id) {
        case 'D':
            return DRIVE_D;
        case 'R':
            return DRIVE_R;
        case 'B':
            return DRIVE_B;
        default:
            // Shouldnt ever be reached
            return INVALID_DRIVE_ID;
    }
}

Path* parsePath(char* path){
//    if(*path == '\0'){
//        char** resolvedPaths = (char**) malloc(sizeof(char *));
//        *resolvedPaths = "";
//        return (Path) {resolvedPaths, 0};
//    }
    path++; // Skip '#'
    DriveID driveId = parseDriveId(path);
    path+=2;
    char* tempPath = path;
    uint16_t depth = 0;
    while(*tempPath != '\0'){
        if(*tempPath++ == '|'){
            depth++;
        }
    }
    char** resolvedPaths = (char**) malloc((depth + 1) * sizeof(char*));
    int i = 0;
    tempPath = path;
    char* start = path;
    // Parsing the path and extracting substrings separated by '|'
    while (*tempPath++ != '\0') {
        if (*tempPath == '|' || *tempPath == '\0') {
            uint32_t strlen = tempPath - start;
            resolvedPaths[i] = (char*)malloc((strlen + 1) * sizeof(char));
            strncpy(resolvedPaths[i], start, strlen);
            resolvedPaths[i][strlen] = '\0'; // Null-terminate the string
            start = tempPath + 1;
            i++;
        }
        //tempPath++;
    }
    Path* resolvedPath = (Path*) malloc(sizeof(Path));
    resolvedPath->driveId = driveId;
    resolvedPath->path = resolvedPaths;
    resolvedPath->depth = depth;
    return resolvedPath;
}

void destroyPath(Path* path){
    for (int i = 0; i < path->depth + 1; ++i) {
        free(path->path[i]);
    }
    free(path);
}

void destroyMetaData(FileMetadata* metadata){
    free(metadata->name);
    free(metadata);
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

