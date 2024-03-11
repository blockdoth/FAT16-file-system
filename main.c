#include <stdio.h>
#include "file_system/volume_management/volume.h"
#include "file_system/volume_management/volume_test.h"
#include "file_system/file_system.h"
#include "file_system/file_system_implementations/FAT16.h"

#define GiB 1073741824 // Bytes





int main() {
    printf("Mounting ramdisk\n");
    RawVolume* raw_volume = mount_volume(RAM_DISK,  GiB - 1);
    if(test_volume(raw_volume)){
        return 1;
    }

    format_volume(raw_volume, FAT16);

    system_file_data file =  "Hello World!";
    uint32_t file_length = 12;

    system_file_metadata file_metadata = {
            "file.txt",
            0,
            1,
            0,
            0,
            0,
            0,
            0,
            file_length,
    };


    fs_create_file(&file_metadata, file);
    fs_read_file(&file_metadata);


    return 0;
}
