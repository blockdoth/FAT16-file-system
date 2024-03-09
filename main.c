#include <stdio.h>
#include "file_system/volume_management/volume.h"
#include "file_system/volume_management/volume_test.h"
#include "file_system/file_system.h"



int main() {
    printf("Mounting ramdisk\n");
    RawVolume* raw_volume = mount_volume(RAM_DISK, 41943040);
    if(test_volume(raw_volume)){
        return 1;
    }

    FormattedVolume * fs = format_volume(raw_volume, FAT16);



    return 0;
}
