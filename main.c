#include <stdio.h>
#include "include/volume.h"
#include "include/volume_test.h"
#include "include/file_system.h"



int main() {
    printf("Mounting ramdisk\n");
    raw_volume* raw_volume = mount_volume(RAM_DISK, 1024);
    if(test_volume(raw_volume)){
        return 1;
    }

    Volume* fs = format_volume(raw_volume, FAT16);



    return 0;
}
