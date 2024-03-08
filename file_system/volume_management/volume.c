#include "volume.h"
#include <assert.h>
#include <string.h>
#include <malloc.h>


RawVolume* mount_volume(VOLUME_TYPE volumeType, uint64_t size){
    RawVolume* volume = prep_volume(volumeType);
    if(volume->init(volume, size)){
        printf("Failed to mount volume\n");
    }
    printf("Successfully mounted volume\n");
    return volume;
}

bool bounds_check(RawVolume* self, uint32_t start_addr, uint32_t size){
    if((start_addr + size) > self->volumeSize){
        if (start_addr > self->volumeSize){
            printf("ERROR:\t Address %u out of bounds of volume\n", start_addr);
        }else{
            printf("ERROR:\t Allocation of %u to large from start address %u\n", size, start_addr);
        }
        return 1;
    }
    return 0;
}

RawVolume* prep_volume(VOLUME_TYPE volumeType){
    switch (volumeType) {
        case RAM_DISK:
            return prep_ram_disk();
        case FLASH_DRIVE:
            return prep_flash_drive();
        default:
    }
}

// === RAM setup ===

RawVolume* prep_ram_disk() {
    RawVolume* volume = (RawVolume*)malloc(sizeof(RawVolume));
    if(volume == NULL){
        return NULL;
    }
    volume->volumeData = 0;
    volume->volumeSize = 0;
    volume->init = ramdisk_init;
    volume->destroy = ramdisk_destroy;
    volume->write= ramdisk_write;
    volume->read = ramdisk_read;
    return volume;
}

bool ramdisk_init(RawVolume* self, uint32_t volume_size){
    self->volumeData = malloc(volume_size);
    if(self->volumeData == NULL){
        printf("ERROR:\t Failed to allocate ram disk");
        return 1;
    }
    self->volumeSize = volume_size;
    INIT_DEBUG_LOG(volume_size);
    return 0;
}
void ramdisk_destroy(RawVolume* self){
    DESTROY_DEBUG_LOG(self->volumeSize);
    free(self->volumeData);
    free(self);
}

bool ramdisk_write(RawVolume* self, void* sourceAddress, volume_ptr destinationAddress, uint32_t size){
    if(bounds_check(self, destinationAddress, size)){
        return 1;
    }
    uint32_t* destination = (uint32_t*) (self->volumeData + destinationAddress);
    if(sourceAddress == NULL){
        CLEAR_DEBUG_LOG(destinationAddress, size);
        memset(destination,0, size);
    }else{
        WRITE_DEBUG_LOG((char*) sourceAddress, destinationAddress, size);
        memcpy(destination,sourceAddress,size);
    }
    return 0;
}
void* ramdisk_read(RawVolume* self, volume_ptr sourceAddress, uint32_t size){
    if(bounds_check(self, sourceAddress, size)){
        return NULL;
    }
    void* chunk = malloc(size);
    if(chunk == NULL){
        printf("ERROR:\t Failed to allocate memory for read\n");
    }

    uint32_t* source = (uint32_t*) (self->volumeData + sourceAddress);
    memcpy(chunk,source,size);

    READ_DEBUG_LOG(sourceAddress, size, (char*) chunk);
    return chunk;
}

// === FLASH drive setup ===
RawVolume* prep_flash_drive(){
    assert(0 && "TODO");
}

#undef DEBUG