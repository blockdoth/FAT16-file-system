#include "flashdisk.h"


bool flashdisk_init(RawVolume* self, uint32_t volume_size){
    // TODO implement
    return false;
}
void flashdisk_destroy(RawVolume* self){
    // TODO implement
}

bool flashdisk_write(RawVolume* self, void* sourceAddress, volume_ptr destinationAddress, uint32_t size){
    if(bounds_check(self, destinationAddress, size)){
        return false;
    }
    // TODO implement
    return false;
}
void* flashdisk_read(RawVolume* self, volume_ptr sourceAddress, uint32_t size){
    if(bounds_check(self, sourceAddress, size)){
        return NULL;
    }
    // TODO implement
    return NULL;
}

RawVolume* prep_flashdisk() {
    RawVolume* volume = (RawVolume*)malloc(sizeof(RawVolume));
    if(volume == NULL){
        return NULL;
    }
    volume->volumeData = 0;
    volume->volumeSize = 0;
    volume->init = flashdisk_init;
    volume->destroy = flashdisk_destroy;
    volume->write= flashdisk_write;
    volume->read = flashdisk_read;
    return volume;
}