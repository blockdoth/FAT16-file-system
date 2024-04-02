#undef DEBUG_FAT16
#include "fs_tests.h"

#define MAX_NOT_NESTED_FILES 10
#define MAX_NESTED_FILES 3
#define SECTOR_SIZE 512
#define SECTORS_PER_CLUSTER 64
#define CLUSTER_COUNT 100

void setupFormattedVolume(){
    RawVolume* raw_volume = mount_volume(RAM_DISK,  GiB/4);
    fs_format(raw_volume, (FormatSpecifier){FAT16,{512,64}}, DRIVE_R);
}

void formatVolumeTest(){
    RawVolume* raw_volume = mount_volume(RAM_DISK,  GiB/4);
    FS_STATUS_CODE statusCode = fs_format(raw_volume, (FormatSpecifier){FAT16,{512,64}}, DRIVE_R);
    assert_int_equals(statusCode, FS_SUCCES);
    fs_destroy(DRIVE_R);
}

void makeFilesFlat(){
    setupFormattedVolume();
    char* prefix = "#R|";
    char* paths[MAX_NOT_NESTED_FILES];
    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        char name[20];
        strcpy(name, prefix);
        char* fileName = randomString(10);
        strcpy(name + 3, fileName);
        free(fileName);
        name[13] = '\0';
        paths[i] = strdup(name);
        uint32_t fileSize = rand() % (512 * 5); // BytesPerSector * SectorsPerCluster * Magic
        char* file = randomString(fileSize);
        fs_create_file(name,file, fileSize);
        free(file);
    }

    char* tree = fs_get_string("#R");
    printf("%s\n",tree);
    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        assert_true(fs_file_exists(paths[i]) == FS_SUCCES);
        free(paths[i]);
    }
    fs_destroy(DRIVE_R);
}


void makeDirsFlat(){
    setupFormattedVolume();
    char* prefix = "#R|";
    char* paths[MAX_NOT_NESTED_FILES];
    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        char name[20];
        strcpy(name, prefix);
        char* fileName = randomString(10);
        strcpy(name + 3, fileName);
        free(fileName);
        name[13] = '\0';
        paths[i] = strdup(name);
        fs_create_dir(name);
    }

    char* tree = fs_get_string("#R|");
    printf("%s\n",tree);
    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        assert_true(fs_dir_exists(paths[i]) == FS_SUCCES);
        free(paths[i]);
    }
    fs_destroy(DRIVE_R);
}

void makeDirsNested(){
    setupFormattedVolume();
    char path[200] = "#R";
    int writePointer = 2;
    char* paths[MAX_NOT_NESTED_FILES];
    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        path[writePointer++] = '|';
        char* fileName = randomString(10);
        strcpy(&path[writePointer], fileName);
        free(fileName);
        writePointer += 10;
        path[writePointer] = '\0';
        fs_create_dir(path);
        paths[i] = strdup(path);
    }

    char* tree = fs_get_string("#R|");
    printf("%s\n",tree);
    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        assert_true(fs_dir_exists(paths[i]) == FS_SUCCES);
        free(paths[i]);
    }
    fs_destroy(DRIVE_R);
}


void makeMultipleNestedDirs(){
    setupFormattedVolume();
    char path[500] = "#R";
    int writePointer = 2;
    char* paths[MAX_NOT_NESTED_FILES * MAX_NESTED_FILES];
    int path_pointer = 0;
    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        path[writePointer++] = '|';
        for (int j = 0; j < MAX_NESTED_FILES; j++) {
            char* fileName = randomString(10);
            strcpy(&path[writePointer], fileName);
            free(fileName);
            path[writePointer + 11] = '\0';
            fs_create_dir(path);
            paths[path_pointer++] = strdup(path);
        }
        writePointer += 10;
    }

    char* tree = fs_get_string("#R|");
    printf("%s\n",tree);
    for (int i = 0; i < MAX_NESTED_FILES * MAX_NOT_NESTED_FILES; i++) {
        assert_true(fs_dir_exists(paths[i]) == FS_SUCCES);
        free(paths[i]);
    }
    fs_destroy(DRIVE_R);
}

void smallReadWrite(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE;
    char* data = randomString(dataSize);
    fs_create_file("#R|small", data, dataSize);
    char* returnedFile = fs_read_file("#R|small");
    assert_mem_equals(returnedFile, data, dataSize);
    free(returnedFile);
    free(data);
    fs_destroy(DRIVE_R);
}

void biggerReadWrite(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE * SECTORS_PER_CLUSTER * CLUSTER_COUNT;
    char* data = randomString(dataSize);
    fs_create_file("#R|small", data, dataSize);
    char* returnedFile = fs_read_file("#R|small");
    assert_mem_equals(returnedFile, data, dataSize);
    free(returnedFile);
    free(data);
    fs_destroy(DRIVE_R);
}

void bigReadWrite(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE * SECTORS_PER_CLUSTER * CLUSTER_COUNT;
    char* data = randomString(dataSize);
    fs_create_file("#R|small", data, dataSize);
    char* returnedFile = fs_read_file("#R|small");
    memCompare(returnedFile, data, dataSize);
    assert_mem_equals(returnedFile, data, dataSize);
    free(returnedFile);
    free(data);
    fs_destroy(DRIVE_R);
}


void smallExpand(){
    setupFormattedVolume();
    uint32_t dataSize = 10;
    char* initialData = randomString(dataSize);
    char* expandedData = randomString(dataSize);
    initialData = realloc(initialData,2 * dataSize);
    fs_create_file("#R|small", initialData, dataSize);
    fs_expand_file("#R|small", expandedData, dataSize);
    char* returnedFile = fs_read_file("#R|small");
    strcat(initialData, expandedData);
    assert_mem_equals(returnedFile, initialData, 2 * dataSize);
    free(returnedFile);
    free(initialData);
    free(expandedData);
    fs_destroy(DRIVE_R);
}

void biggerExpand(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE * SECTORS_PER_CLUSTER / 2;
    char* initialData = randomString(dataSize);
    char* expandedData = randomString(dataSize);
    initialData = realloc(initialData,2 * dataSize);
    strcat(initialData, expandedData);
    fs_create_file("#R|small", initialData, dataSize);
    fs_expand_file("#R|small", expandedData, dataSize);
    char* returnedFile = fs_read_file("#R|small");
    memCompare(returnedFile, initialData, 2 * dataSize);
    assert_mem_equals(returnedFile, initialData, 2 * dataSize);
    free(returnedFile);
    free(initialData);
    free(expandedData);
    fs_destroy(DRIVE_R);
}

void bigExpand(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE * SECTORS_PER_CLUSTER * 2;
    char* initialData = randomString(dataSize);
    char* expandedData = randomString(dataSize);
    initialData = realloc(initialData,2 * dataSize);
    strcat(initialData, expandedData);
    fs_create_file("#R|small", initialData, dataSize);
    fs_expand_file("#R|small", expandedData, dataSize);
    char* returnedFile = fs_read_file("#R|small");
    memCompare(returnedFile, initialData, 2 * dataSize);
    assert_mem_equals(returnedFile, initialData, 2 * dataSize);
    free(returnedFile);
    free(initialData);
    free(expandedData);
    fs_destroy(DRIVE_R);
}


void register_api_tests(){
    register_test(bigExpand);
    register_test(biggerExpand);
    register_test(smallExpand);
    register_test(bigReadWrite);
    register_test(biggerReadWrite);
    register_test(smallReadWrite);
    register_test(makeMultipleNestedDirs);
    register_test(makeDirsNested);
    register_test(makeDirsFlat);
    register_test(makeFilesFlat);
    register_test(formatVolumeTest);


}