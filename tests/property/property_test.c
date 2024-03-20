#include "../../file_system/file_system_api.h"
#include "property_test.h"


uint32_t dirCount = 0;
char** createDirs(uint32_t depth, char* basePath){
    if(depth > MAX_DIR_DEPTH){
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

void register_property_tests(){
    register_test(make_volume);
}

