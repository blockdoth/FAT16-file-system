#include "system_test.h"
#include "../../file_system/file_system.h"

#define GiB 1073741823

void make_volume(){
    RawVolume* raw_volume = mount_volume(RAM_DISK,  GiB);
    fs_format(raw_volume, FAT16);


}

void register_system_tests(){
    register_test(make_volume);
}