#include "api.h"
#undef DEBUG_FAT16


void formatVolumeTest(){
    RawVolume* raw_volume = mount_volume(RAM_DISK,  GiB/4);

    assert_int_equals(fs_format(raw_volume, FAT16), FS_SUCCES);

}


void register_api_tests(){
    register_test(formatVolumeTest);
    register_test(makeFilesFlat);
    register_test(makeFilesNested);
    register_test(makeDirsNested);
    register_test(makeDirsFlat);

}