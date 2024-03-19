#include <time.h>
#include "system_test.h"
#include "../../file_system/file_system.h"

#define GiB 1073741823

#define MAX_DIR_DEPTH 20
#define MAX_NAME_LENGTH 10
#define MIN_NAME_LENGTH 3
#define MAX_FILES 20
#define MAX_FILESIZE 1000
#define MIN_FILESIZE 10
#define MAX_SUBFOLDERS 5
#define MAX_DEPTH 10


uint32_t dirCount = 0;
char** createDirs(uint32_t depth, char* basePath){
    if(depth > MAX_DEPTH){
        return &basePath;
    }
    char path[1000]; // Adjust size as per your need
    //memset(path, 0, sizeof(path));
    strcpy(path, basePath);
    uint32_t basePathLen = strlen(basePath);
    if(basePathLen == 0){
        strcat(path, "#");
    }else{
        strcat(path, "|");

    }
    uint32_t offset = strlen(path);
    uint32_t subDirCount = rand() % MAX_SUBFOLDERS;
    //printf("Creating %u subdirs at depth %u\n", subDirCount, depth);
    char** resultPaths = (char **) malloc(subDirCount * sizeof(char**));
    char** childPaths;
    for (int i = 0; i < subDirCount; i++) {
        char* name = randomString(MAX_NAME_LENGTH);
        strcpy(path + offset, name);
        //printf("%s\n", path);
        fs_create_dir(path);
        dirCount++;
        resultPaths[i] = path;
        childPaths = createDirs( depth + 1, path);
        if(childPaths != NULL){
            uint32_t childPathsCount = sizeof(childPaths) / sizeof(char**);
            uint32_t newSize = (subDirCount + childPathsCount) * sizeof(char **);
            resultPaths = realloc(resultPaths, newSize);
//            for (int i = 0; i < subDirCount; i++) {
//                printf("%s", resultPaths[i]);
//            }

            for (int i = 0; i < childPathsCount; ++i) {
                resultPaths[subDirCount + i] = childPaths[i];

            }
        }
    }


    return resultPaths;
}

void make_volume(){
    srand(time(NULL));

    RawVolume* raw_volume = mount_volume(RAM_DISK,  GiB);
    fs_format(raw_volume, FAT16);

    void* filesData[MAX_FILES];
    for (int i = 0; i < MAX_FILES; ++i) {
        filesData[i] = randomString(rand() % (MAX_FILESIZE - MIN_FILESIZE) + MIN_FILESIZE);
    }


    char** names[MAX_SUBFOLDERS];
    for (int i = 0; i < MAX_SUBFOLDERS; ++i) {
        names[i] = randomString(MAX_NAME_LENGTH);
    }

    char** path = createDirs(0, "");
    uint32_t pathCount = sizeof(path) / sizeof (char **);
    char* tree = fs_get_string("#");
    printf("%s", tree);
    printf("FS contains %u directories\n", dirCount);

//    for (int i = 0; i < pathCount; ++i) {
//        printf("%s\n", path[i]);
//    }
//    for (int i = 0; i < MAX_FILES; ++i) {
//        free(filesData[i]);
//    }
//    for (int i = 0; i < MAX_SUBFOLDERS; ++i) {
//        free(names[i]);
//    }
}

void register_system_tests(){
    register_test(make_volume);
}


char* randomString(uint32_t length){
    char* chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    uint8_t charsLength = 52;
    char* randomName = (char*) malloc(length);
    for (int i = 0; i < length; i++) {
        randomName[i] = chars[rand() % charsLength];
    }
    return randomName;
}

char* generatePath() {
    uint8_t depth = rand() % MAX_DIR_DEPTH + 1;
    uint8_t* nameLengths = (uint8_t*) malloc(depth);
    uint8_t totalLength = 1;
    for (int i = 0; i < depth; i++) {
        nameLengths[i] = rand() % (MAX_NAME_LENGTH - MIN_NAME_LENGTH + 1) + MIN_NAME_LENGTH;
        totalLength += nameLengths[i];
    }
    char* path = (char*)malloc(totalLength);
    strcpy(path, "");
    uint8_t offset = 1;
    for (int i = 0; i < depth; ++i) {
        uint8_t len = nameLengths[i];
        char* name = randomString(len);
        strncpy(path + offset, name, len);
        //free(name); // Causes segfault sometimes, idk why
        offset += len;
        strcpy(path + offset++, "|");
    }
    free(nameLengths);
    path[totalLength - 1] = '\0';
    return path;
}