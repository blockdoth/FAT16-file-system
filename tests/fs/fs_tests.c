#undef DEBUG_FAT16
#include "fs_tests.h"

#define MAX_NOT_NESTED_FILES 10
#define MAX_NESTED_FILES 3
#define SECTOR_SIZE 512
#define SECTORS_PER_CLUSTER 64
#define CLUSTER_COUNT 100


void setupFormattedVolume(){
    RawVolume* raw_volume = mount_volume(RAM_DISK,  GiB/4);
    fs_format(raw_volume, (FormatSpecifier){FAT16,{SECTOR_SIZE,SECTORS_PER_CLUSTER}}, DRIVE_R);
}

void formatVolumeTest(){
    RawVolume* raw_volume = mount_volume(RAM_DISK,  GiB/4);
    FS_STATUS_CODE statusCode = fs_format(raw_volume, (FormatSpecifier){FAT16,{SECTOR_SIZE,SECTORS_PER_CLUSTER}}, DRIVE_R);
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
        uint32_t fileSize = rand() % (SECTOR_SIZE * 5);
        char* file = randomString(fileSize);
        fs_create_file(name,file, fileSize);
        free(file);
    }

//    char* tree = fs_get_string("#R");
//    printf("%s\n",tree);
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

    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        assert_true(fs_is_dir(paths[i]) == true);
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

//    char* tree = fs_get_string("#R|");
//    printf("%s\n",tree);
    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        assert_true(fs_is_dir(paths[i]) == true);
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

//    char* tree = fs_get_string("#R|");
//    printf("%s\n",tree);
    for (int i = 0; i < MAX_NESTED_FILES * MAX_NOT_NESTED_FILES; i++) {
        assert_true(fs_is_dir(paths[i]) == true);
        assert_true(fs_dir_exists(paths[i]) == FS_SUCCES);
        free(paths[i]);
    }
    fs_destroy(DRIVE_R);
}


void deleteFilesFlat(){
    setupFormattedVolume();
    char* prefix = "#R|";
    char* paths[MAX_NOT_NESTED_FILES];
    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        char name[100];
        strcpy(name, prefix);
        char* fileName = randomString(10);
        strcpy(name + 3, fileName);
        free(fileName);
        name[13] = '\0';
        paths[i] = strdup(name);
        uint32_t fileSize = rand() % (SECTOR_SIZE * 5); // BytesPerSector * SectorsPerCluster * Magic
        char* file = randomString(fileSize);
        fs_create_file(name,file, fileSize);
        free(file);
    }

    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        fs_delete_file(paths[i]);
        assert_true(fs_file_exists(paths[i]) == FS_FILE_NOT_FOUND);
        free(paths[i]);
    }
    fs_destroy(DRIVE_R);
}


void deleteDirsFlat(){
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

    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        fs_delete_file(paths[i]);
        assert_true(fs_file_exists(paths[i]) == FS_DIRECTORY_NOT_FOUND);
        free(paths[i]);
    }
    fs_destroy(DRIVE_R);
}

void deleteDirsNested(){
    setupFormattedVolume();
    char path[200] = "#R";
    int writePointer = 2;
    char* paths[MAX_NOT_NESTED_FILES];
    for (int i = MAX_NOT_NESTED_FILES - 1; i >= 0; i--) {
        path[writePointer++] = '|';
        char* fileName = randomString(10);
        strcpy(&path[writePointer], fileName);
        free(fileName);
        writePointer += 10;
        path[writePointer] = '\0';
        fs_create_dir(path);
        paths[i] = strdup(path);
    }

    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        fs_delete_file(paths[i]);
        assert_true(fs_file_exists(paths[i]) == FS_DIRECTORY_NOT_FOUND);
        free(paths[i]);
    }
    fs_destroy(DRIVE_R);
}


void deleteMultipleNestedDirs(){
    setupFormattedVolume();
    char path[500] = "#R";
    int writePointer = 2;
    char* paths[MAX_NOT_NESTED_FILES * MAX_NESTED_FILES];
    int path_pointer = 0;
    for (int i = MAX_NOT_NESTED_FILES - 1; i >= 0; i--) {
        path[writePointer++] = '|';
        for (int j = MAX_NESTED_FILES; j > 0 ; j--) {
            char* fileName = randomString(10);
            strcpy(&path[writePointer], fileName);
            free(fileName);
            path[writePointer + 11] = '\0';
            fs_create_dir(path);
            paths[path_pointer++] = strdup(path);
        }
        writePointer += 10;
    }

    for (int i = 0; i < MAX_NESTED_FILES * MAX_NOT_NESTED_FILES; i++) {
        fs_delete_file(paths[i]);
        assert_true(fs_file_exists(paths[i]) == FS_DIRECTORY_NOT_FOUND);
        free(paths[i]);
    }
    fs_destroy(DRIVE_R);
}



void makeDeleteMakeFilesFlat(){
    setupFormattedVolume();
    char* prefix = "#R|";
    char* paths[MAX_NOT_NESTED_FILES];
    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        char name[100];
        strcpy(name, prefix);
        char* fileName = randomString(10);
        strcpy(name + 3, fileName);
        free(fileName);
        name[13] = '\0';
        paths[i] = strdup(name);
        uint32_t fileSize = rand() % (SECTOR_SIZE * 5); // BytesPerSector * SectorsPerCluster * Magic
        char* file = randomString(fileSize);
        fs_create_file(name,file, fileSize);
        free(file);
    }

    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        fs_delete_file(paths[i]);
        assert_true(fs_file_exists(paths[i]) == FS_FILE_NOT_FOUND);
        uint32_t fileSize = rand() % (SECTOR_SIZE * 5); // BytesPerSector * SectorsPerCluster * Magic
        char* file = randomString(fileSize);
        fs_create_file(paths[i],file,fileSize);
        free(file);
    }

    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        assert_true(fs_file_exists(paths[i]) == FS_SUCCES);
        free(paths[i]);
    }
    fs_destroy(DRIVE_R);
}


void smallReadWrite(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE;
    char* data = randomString(dataSize);
    fs_create_file("#R|small", data, dataSize);
    char* returnedData = fs_read_file("#R|small");
    assert_mem_equals(returnedData, data, dataSize);
    free(returnedData);
    free(data);
    fs_destroy(DRIVE_R);
}

void biggerReadWrite(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE * SECTORS_PER_CLUSTER;
    char* data = randomString(dataSize);
    fs_create_file("#R|small", data, dataSize);
    char* returnedData = fs_read_file("#R|small");
    assert_mem_equals(returnedData, data, dataSize);
    free(returnedData);
    free(data);
    fs_destroy(DRIVE_R);
}

void bigReadWrite(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE * SECTORS_PER_CLUSTER * CLUSTER_COUNT;
    char* data = randomString(dataSize);
    fs_create_file("#R|small", data, dataSize);
    char* returnedData = fs_read_file("#R|small");
    memCompare(returnedData, data, dataSize);
    assert_mem_equals(returnedData, data, dataSize);
    free(returnedData);
    free(data);
    fs_destroy(DRIVE_R);
}


void smallReadWriteSection(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE;
    uint32_t offset = 10;
    uint32_t chunkSize = 100;
    char* data = randomString(dataSize);
    fs_create_file("#R|small", data, dataSize);
    char* returnedData = fs_read_file_section("#R|small", offset, chunkSize);
    assert_mem_equals(returnedData,data + offset, chunkSize);
    free(returnedData);
    free(data);
    fs_destroy(DRIVE_R);
}

void biggerReadWriteSection(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE * SECTORS_PER_CLUSTER;
    uint32_t offset = SECTOR_SIZE * 30;
    uint32_t chunkSize = SECTOR_SIZE * 2 + 10;
    char* data = randomString(dataSize);
    fs_create_file("#R|small", data, dataSize);
    char* returnedData = fs_read_file_section("#R|small", offset, chunkSize);
    assert_mem_equals(returnedData, data + offset, chunkSize);
    free(returnedData);
    free(data);
    fs_destroy(DRIVE_R);
}

void bigReadWriteSection(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE * SECTORS_PER_CLUSTER * CLUSTER_COUNT;
    uint32_t offset = SECTOR_SIZE * SECTORS_PER_CLUSTER * 30;
    uint32_t chunkSize = SECTOR_SIZE * SECTORS_PER_CLUSTER * 2 + SECTOR_SIZE * 13 + 5;
    char* data = randomString(dataSize);
    fs_create_file("#R|small", data, dataSize);
    char* returnedData = fs_read_file_section("#R|small", offset, chunkSize);
    assert_mem_equals(returnedData, data + offset, chunkSize);
    free(returnedData);
    free(data);
    fs_destroy(DRIVE_R);
}

void smallExpand(){
    setupFormattedVolume();
    uint32_t dataSize = 10;
    char* initialData = randomString(dataSize);
    char* expandedData = randomString(dataSize);
    initialData = realloc(initialData,2 * dataSize + 1);
    fs_create_file("#R|small", initialData, dataSize);
    fs_expand_file("#R|small", expandedData, dataSize);
    char* returnedData = fs_read_file("#R|small");
    strcat(initialData, expandedData);
    assert_mem_equals(returnedData, initialData, 2 * dataSize);
    free(returnedData);
    free(initialData);
    free(expandedData);
    fs_destroy(DRIVE_R);
}

void biggerExpand(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE * SECTORS_PER_CLUSTER / 2;
    char* initialData = randomString(dataSize);
    char* expandedData = randomString(dataSize);
    initialData = realloc(initialData,2 * dataSize + 1);
    strcat(initialData, expandedData);
    fs_create_file("#R|small", initialData, dataSize);
    fs_expand_file("#R|small", expandedData, dataSize);
    char* returnedData = fs_read_file("#R|small");
    memCompare(returnedData, initialData, 2 * dataSize);
    assert_mem_equals(returnedData, initialData, 2 * dataSize);
    free(returnedData);
    free(initialData);
    free(expandedData);
    fs_destroy(DRIVE_R);
}

void bigExpand(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE * SECTORS_PER_CLUSTER * 10;
    char* initialData = randomString(dataSize);
    char* expandedData = randomString(dataSize);
    initialData = realloc(initialData,2 * dataSize + 1);
    strcat(initialData, expandedData);
    fs_create_file("#R|small", initialData, dataSize);
    fs_expand_file("#R|small", expandedData, dataSize);
    char* returnedData = fs_read_file("#R|small");
    memCompare(returnedData, initialData, 2 * dataSize);
    assert_mem_equals(returnedData, initialData, 2 * dataSize);
    free(returnedData);
    free(initialData);
    free(expandedData);
    fs_destroy(DRIVE_R);
}

void expandAroundFile(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE * SECTORS_PER_CLUSTER * 2;
    char* initialData = randomString(dataSize);
    char* expandedData = randomString(dataSize);
    char* randomData = randomString(dataSize);
    initialData = realloc(initialData,3 * dataSize + 1);
    strcat(initialData, expandedData);
    strcat(initialData, expandedData);
    fs_create_file("#R|small", initialData, dataSize);
    fs_create_file("#R|randomA", randomData, dataSize);
    fs_expand_file("#R|small", expandedData, dataSize);
    fs_create_file("#R|randomB", randomData, dataSize);
    fs_expand_file("#R|small", expandedData, dataSize);
    char* returnedDataRandomA = fs_read_file("#R|randomA");
    char* returnedData = fs_read_file("#R|small");
    char* returnedDataRandomB = fs_read_file("#R|randomB");
    assert_mem_equals(returnedData, initialData, 3 * dataSize);
    assert_mem_equals(returnedDataRandomA, randomData, dataSize);
    assert_mem_equals(returnedDataRandomB, randomData, dataSize);
    free(returnedData);
    free(randomData);
    free(returnedDataRandomA);
    free(returnedDataRandomB);
    free(initialData);
    free(expandedData);
    fs_destroy(DRIVE_R);
}


void smallUpdateNoOffsetSameSize(){
    setupFormattedVolume();
    uint32_t dataSize = 10;
    char* initialData = randomString(dataSize);
    char* updatedData = randomString(dataSize);
    fs_create_file("#R|small", initialData, dataSize);
    fs_update_file("#R|small", updatedData, dataSize, 0);
    char* returnedData = fs_read_file("#R|small");
    assert_mem_equals(returnedData, updatedData, dataSize);
    free(returnedData);
    free(initialData);
    free(updatedData);
    fs_destroy(DRIVE_R);
}

void smallUpdateNoOffsetIncreaseSize(){
    setupFormattedVolume();
    uint32_t dataSize = 10;
    char* initialData = randomString(dataSize);
    char* updatedData = randomString(2 * dataSize);
    fs_create_file("#R|small", initialData, dataSize);
    fs_update_file("#R|small", updatedData, 2 * dataSize, 0);
    char* returnedData = fs_read_file("#R|small");
    assert_mem_equals(returnedData, updatedData, 2 * dataSize);
    free(returnedData);
    free(initialData);
    free(updatedData);
    fs_destroy(DRIVE_R);
}

void smallUpdateOffsetSameSize(){
    setupFormattedVolume();
    uint32_t dataSize = 10;
    uint32_t offset = 5;
    char* initialData = randomString(dataSize);
    char* updatedData = randomString(dataSize - offset);
    fs_create_file("#R|small", initialData, dataSize);
    fs_update_file("#R|small", updatedData, dataSize, offset);
    char* returnedData = fs_read_file("#R|small");
    mergeData(initialData, updatedData, dataSize, offset);
    memCompare(returnedData, initialData, dataSize);
    assert_mem_equals(returnedData, initialData, dataSize);
    free(returnedData);
    free(initialData);
    free(updatedData);
    fs_destroy(DRIVE_R);
}


void biggerUpdateNoOffsetSameSize(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE * SECTORS_PER_CLUSTER;
    char* initialData = randomString(dataSize);
    char* updatedData = randomString(dataSize);
    fs_create_file("#R|small", initialData, dataSize);
    fs_update_file("#R|small", updatedData, dataSize, 0);
    char* returnedData = fs_read_file("#R|small");
    assert_mem_equals(returnedData, updatedData, dataSize);
    free(returnedData);
    free(initialData);
    free(updatedData);
    fs_destroy(DRIVE_R);
}

void biggerUpdateNoOffsetIncreaseSize(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE * SECTORS_PER_CLUSTER / 2;
    char* initialData = randomString(dataSize);
    char* updatedData = randomString(2 * dataSize);
    fs_create_file("#R|small", initialData, dataSize);
    fs_update_file("#R|small", updatedData, 2 * dataSize, 0);
    char* returnedData = fs_read_file("#R|small");
    assert_mem_equals(returnedData, updatedData, 2 * dataSize);
    free(returnedData);
    free(initialData);
    free(updatedData);
    fs_destroy(DRIVE_R);
}

void biggerUpdateOffsetSameSize(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE * SECTORS_PER_CLUSTER;
    uint32_t offset = SECTOR_SIZE * 15;
    char* initialData = randomString(dataSize);
    char* updatedData = randomString(dataSize - offset);
    fs_create_file("#R|small", initialData, dataSize);
    fs_update_file("#R|small", updatedData, dataSize - offset, offset);
    char* returnedData = fs_read_file("#R|small");
    mergeData(initialData, updatedData, dataSize, offset);
    memCompare(returnedData, initialData, dataSize);
    assert_mem_equals(returnedData, initialData, dataSize);
    free(returnedData);
    free(initialData);
    free(updatedData);
    fs_destroy(DRIVE_R);
}


void bigUpdateNoOffsetSameSize(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE * SECTORS_PER_CLUSTER * 10;
    char* initialData = randomString(dataSize);
    char* updatedData = randomString(dataSize);
    fs_create_file("#R|small", initialData, dataSize);
    fs_update_file("#R|small", updatedData, dataSize, 0);
    char* returnedData = fs_read_file("#R|small");
    assert_mem_equals(returnedData, updatedData, dataSize);
    free(returnedData);
    free(initialData);
    free(updatedData);
    fs_destroy(DRIVE_R);
}

void bigUpdateNoOffsetIncreaseSize(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE * SECTORS_PER_CLUSTER * 10;
    char* initialData = randomString(dataSize);
    char* updatedData = randomString(2 * dataSize);
    fs_create_file("#R|small", initialData, dataSize);
    fs_update_file("#R|small", updatedData, 2 * dataSize, 0);
    char* returnedData = fs_read_file("#R|small");
    assert_mem_equals(returnedData, updatedData, 2 * dataSize);
    free(returnedData);
    free(initialData);
    free(updatedData);
    fs_destroy(DRIVE_R);
}

void bigUpdateOffsetSameSize(){
    setupFormattedVolume();
    uint32_t dataSize = SECTOR_SIZE * SECTORS_PER_CLUSTER * 10;
    uint32_t offset = SECTOR_SIZE * SECTORS_PER_CLUSTER * 5;
    char* initialData = randomString(dataSize);
    char* updatedData = randomString(dataSize - offset);
    fs_create_file("#R|small", initialData, dataSize);
    fs_update_file("#R|small", updatedData, dataSize - offset, offset);
    char* returnedData = fs_read_file("#R|small");
    mergeData(initialData, updatedData, dataSize, offset);
    memCompare(returnedData, initialData, dataSize);
    assert_mem_equals(returnedData, initialData, dataSize);
    free(returnedData);
    free(initialData);
    free(updatedData);
    fs_destroy(DRIVE_R);
}

void register_api_tests(){
    register_test(bigUpdateOffsetSameSize);
    register_test(bigUpdateNoOffsetIncreaseSize);
    register_test(bigUpdateNoOffsetSameSize);
    register_test(biggerUpdateOffsetSameSize);
    register_test(biggerUpdateNoOffsetIncreaseSize);
    register_test(biggerUpdateNoOffsetSameSize);
    register_test(smallUpdateOffsetSameSize);
    register_test(smallUpdateNoOffsetIncreaseSize);
    register_test(smallUpdateNoOffsetSameSize);
    register_test(expandAroundFile);
    register_test(bigExpand);
    register_test(biggerExpand);
    register_test(smallExpand);
    register_test(bigReadWriteSection);
    register_test(biggerReadWriteSection);
    register_test(smallReadWriteSection);
    register_test(bigReadWrite);
    register_test(biggerReadWrite);
    register_test(smallReadWrite);
    register_test(makeDeleteMakeFilesFlat);
    register_test(deleteMultipleNestedDirs);
    register_test(deleteDirsNested);
    register_test(deleteDirsFlat);
    register_test(deleteFilesFlat);
    register_test(makeMultipleNestedDirs);
    register_test(makeDirsNested);
    register_test(makeDirsFlat);
    register_test(makeFilesFlat);
    register_test(formatVolumeTest);


}