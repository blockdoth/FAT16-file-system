#undef DEBUG_FAT16
#include "api.h"


void setupFormattedVolume(){
    RawVolume* raw_volume = mount_volume(RAM_DISK,  GiB/4);
    fs_format(raw_volume, FAT16, DRIVE_R);
}

void formatVolumeTest(){
    RawVolume* raw_volume = mount_volume(RAM_DISK,  GiB/4);
    assert_int_equals(fs_format(raw_volume, FAT16, DRIVE_R), FS_SUCCES);
}


#define MAX_FILES 10
void makeFilesFlat(){
    setupFormattedVolume();

    char* prefix = "#R|";
    char* paths[MAX_FILES];

    for (int i = 0; i < MAX_FILES; i++) {
        char name[20];
        strcpy(name, prefix);
        strcpy(name + 3, randomString(11));
        name[14] = '\0';
        paths[i] = strdup(name);
        uint32_t fileSize = rand() % (512 * 5); // BytesPerSector * SectorsPerCluster * Magic
        char* file = randomString(fileSize);
        fs_create_file(name,file, fileSize);
        free(file);
    }

    char* tree = fs_get_string("#R|");
    printf("%s",tree);
    for (int i = 0; i < MAX_FILES; i++) {
        printf("%s", paths[i]);
        assert_true(fs_file_exists(paths[i]) == FS_SUCCES);
        free(paths[i]);
    }


}

void makeFilesNested(){
    setupFormattedVolume();

}

void makeDirsNested(){
    setupFormattedVolume();

}

void makeDirsFlat(){
    setupFormattedVolume();

}


void register_api_tests(){
    register_test(formatVolumeTest);
    register_test(makeFilesFlat);
//    register_test(makeFilesNested);
//    register_test(makeDirsNested);
//    register_test(makeDirsFlat);

}