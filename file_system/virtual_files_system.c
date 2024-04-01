#include "virtual_files_system.h"




FS_STATUS_CODE fs_file_exists(char* path){
    if(!checkValidPath(path)) return FS_INVALID_PATH;
    Path resolvedPath = parsePath(path);

}

FS_STATUS_CODE fs_create_file(char* path, void* file_data, uint32_t file_size);


uint32_t fs_update_file(char* path, void* data, uint32_t new_file_size);


void* fs_read_file(char* path);

void* fs_read_file_section(char* path, uint32_t offset, uint32_t size);

FS_STATUS_CODE fs_delete_file(char* path);

FS_STATUS_CODE fs_dir_exists(char* path);

char* fs_get_string(char* path);

FS_STATUS_CODE fs_create_dir(char* path);

FS_STATUS_CODE fs_delete_dir(char* path, bool recursive);





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