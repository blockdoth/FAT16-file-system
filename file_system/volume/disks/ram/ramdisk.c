#include "ramdisk.h"
#include "../disk_debug.h"

bool ramdisk_init(RawVolume* self, uint32_t volume_size){
    self->volumeData = malloc(volume_size);
    if(self->volumeData == NULL){
        printf("ERROR:\t Failed to allocate ram disk");
        return false;
    }
    self->volumeSize = volume_size;
    initDebugLog(volume_size);
    return true;
}
void ramdisk_destroy(RawVolume* self){
    initDebugLog(self->volumeSize);
    free(self->volumeData);
    free(self);
}

bool ramdisk_write(RawVolume* self, void* sourceAddress, volume_ptr destinationAddress, uint32_t size){
    if(bounds_check(self, destinationAddress, size)){
        return false;
    }
    uint32_t* destination = (uint32_t*) (self->volumeData + destinationAddress);
    if(sourceAddress == NULL){
        clearDebugLog(destinationAddress, size);
        memset(destination,0, size);
    }else{
        writeDebugLog((char*) sourceAddress, destinationAddress, size);
        memcpy(destination,sourceAddress,size);
    }
    return true;
}
void* ramdisk_read(RawVolume* self, volume_ptr sourceAddress, uint32_t size){
    if(bounds_check(self, sourceAddress, size)){
        return NULL;
    }
    uint32_t* source = (uint32_t*) (self->volumeData + sourceAddress);

    void* chunk = malloc(size);
    if(chunk == NULL){
        printf("ERROR:\t Failed to allocate memory for read\n");
    }

    memcpy(chunk,source,size);

    readDebugLog(sourceAddress, size, (char*) chunk);
    return chunk;
}

RawVolume* prep_ramdisk() {
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