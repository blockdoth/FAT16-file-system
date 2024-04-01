#undef DEBUG_FAT16
#include "api.h"

#define MAX_NOT_NESTED_FILES 10
#define MAX_NESTED_FILES 3

void setupFormattedVolume(){
    RawVolume* raw_volume = mount_volume(RAM_DISK,  GiB/4);
    fs_format(raw_volume, FAT16, DRIVE_R);
}

void formatVolumeTest(){
    RawVolume* raw_volume = mount_volume(RAM_DISK,  GiB/4);
    assert_int_equals(fs_format(raw_volume, FAT16, DRIVE_R), FS_SUCCES);
}


void makeFilesFlat(){
    setupFormattedVolume();
    char* prefix = "#R|";
    char* paths[MAX_NOT_NESTED_FILES];
    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        char name[20];
        strcpy(name, prefix);
        strcpy(name + 3, randomString(10));
        name[13] = '\0';
        paths[i] = strdup(name);
        uint32_t fileSize = rand() % (512 * 5); // BytesPerSector * SectorsPerCluster * Magic
        char* file = randomString(fileSize);
        fs_create_file(name,file, fileSize);
        free(file);
    }

    char* tree = fs_get_string("#R|");
    printf("%s\n",tree);
    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        assert_true(fs_file_exists(paths[i]) == FS_SUCCES);
        free(paths[i]);
    }
}


void makeDirsFlat(){
    setupFormattedVolume();
    char* prefix = "#R|";
    char* paths[MAX_NOT_NESTED_FILES];
    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        char name[20];
        strcpy(name, prefix);
        strcpy(name + 3, randomString(10));
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
}

void makeDirsNested(){
    setupFormattedVolume();
    char path[200] = "#R";
    int writePointer = 2;
    char* paths[MAX_NOT_NESTED_FILES];
    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        path[writePointer++] = '|';
        strcpy(&path[writePointer], randomString(10));
        writePointer += 10;
        path[writePointer + 1] = '\0';
        fs_create_dir(path);
        paths[i] = strdup(path);
    }

    char* tree = fs_get_string("#R|");
    printf("%s\n",tree);
    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
        assert_true(fs_dir_exists(paths[i]) == FS_SUCCES);
        free(paths[i]);
    }
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
            strcpy(&path[writePointer], randomString(10));
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
}



//void makeMultipleNestedDirsAndFiles(){
//    setupFormattedVolume();
//    char path[1000] = "#R";
//    int writePointer = 2;
//    char* paths[MAX_NOT_NESTED_FILES * MAX_NESTED_FILES];
//    int path_pointer = 0;
//    for (int i = 0; i < MAX_NOT_NESTED_FILES; i++) {
//        path[writePointer++] = '|';
//        for (int j = 0; j < MAX_NESTED_FILES; j++) {
//            uint32_t fileSize = rand() % (512 * 3); // BytesPerSector * SectorsPerCluster * Magic
//            char* file = randomString(fileSize);
//            strcpy(&path[writePointer], randomString(10));
//            path[writePointer + 11] = '\0';
//            fs_create_file(path,file, fileSize);
//            //path[writePointer + 10] = '!';
//            paths[path_pointer++] = strdup(path);
//            fs_create_dir(path);
//            free(file);
//        }
//        writePointer += 10;
//    }
//
//    char* tree = fs_get_string("#R|");
//    printf("%s\n",tree);
//    for (int i = 0; i < MAX_NESTED_FILES * MAX_NOT_NESTED_FILES; i++) {
//        assert_true(fs_file_exists(paths[i]) == FS_SUCCES);
//        free(paths[i]);
//    }
//}



void register_api_tests(){
    register_test(formatVolumeTest);
    register_test(makeFilesFlat);
    register_test(makeDirsFlat);
    register_test(makeDirsNested);
    register_test(makeMultipleNestedDirs);
    //register_test(makeMultipleNestedDirsAndFiles);

}