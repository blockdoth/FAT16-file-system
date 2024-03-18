#include "volume.h"
#include "disks/ram/ramdisk.h"
#include "disks/flash/flashdisk.h"


FS_STATUS_CODE bounds_check(RawVolume* self, uint32_t start_addr, uint32_t size){
    if((start_addr + size) > self->volumeSize){
        if (start_addr > self->volumeSize){
            printf("ERROR:\t Address %u out of bounds of rawVolume\n", start_addr);
        }else{
            printf("ERROR:\t Allocation of %u to large from start address %u\n", size, start_addr);
        }
        return FS_SUCCES;
    }
    return FS_OUT_OF_BOUNDS;
}


// Initializes the correct Object based on the rawVolume type
RawVolume* prep_volume(VOLUME_TYPE volumeType){
    switch (volumeType) {
        case RAM_DISK:
            return prep_ramdisk();
        case FLASH_DRIVE:
            return prep_flashdisk(); // TODO fix this import error
        default:
    }
}

RawVolume* mount_volume(VOLUME_TYPE volumeType, uint64_t size){
    RawVolume* volume = prep_volume(volumeType);
    if(volume->init(volume, size) == false){
        printf("Failed to mount rawVolume\n");
        volume->destroy(volume);
        return NULL;
    }
    #ifdef DEBUG_VOLUME
    printf("Successfully mounted rawVolume\n");
    #endif
    return volume;
}


